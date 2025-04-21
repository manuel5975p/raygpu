#ifdef __EMSCRIPTEN__
#define RGFW_WASM
#endif
#include <external/RGFW.h>
#include <webgpu/webgpu.h> // Make sure this is included
#include <stdio.h>         // For fprintf, potentially NULL

#if defined(RGFW_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11) // Check for Cocoa macOS
    // Import Cocoa and Metal/QuartzCore headers for macOS
    #import <Cocoa/Cocoa.h>
    #import <QuartzCore/CAMetalLayer.h>
    #include <objc/runtime.h> // For object_getInstanceVariable if needed
    #include <objc/message.h> // For objc_msgSend
#elif defined(RGFW_UNIX) // Linux/Other Unix (Wayland/X11)
    #if defined(RGFW_WAYLAND)
        #include <wayland-client.h>
    #endif
    #if defined(RGFW_X11)
        #include <X11/Xlib.h>
    #endif
#endif

WGPUSurface RGFW_GetWGPUSurface(WGPUInstance instance, RGFW_window* window) {
    if (!instance || !window) {
        fprintf(stderr, "Error: Invalid WGPUInstance or RGFW_window pointer.\n");
        return NULL;
    }

    WGPUSurfaceDescriptor surfaceDesc = {0};
    // The nextInChain will point to the platform-specific source struct

#if defined(RGFW_WINDOWS)
    // --- Windows Implementation ---
    WGPUSurfaceSourceWindowsHWND fromHwnd = {0};
    fromHwnd.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
    fromHwnd.hwnd = window->src.window; // Get HWND from RGFW window source
    if (!fromHwnd.hwnd) {
        fprintf(stderr, "RGFW Error: HWND is NULL for Windows window.\n");
        return NULL;
    }
    fromHwnd.hinstance = GetModuleHandle(NULL); // Get current process HINSTANCE

    surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromHwnd.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);

#elif defined(RGFW_MACOS) && !defined(RGFW_MACOS_X11) // Exclude X11 on Mac builds
    // --- macOS (Cocoa) Implementation ---
    NSView* nsView = (NSView*)window->src.view;
    if (!nsView) {
        fprintf(stderr, "RGFW Error: NSView is NULL for macOS window.\n");
        return NULL;
    }

    // Ensure the view is layer-backed before trying to get/set the layer
    // Note: RGFW might already do this depending on its configuration.
    // This call is generally safe to make even if already layer-backed.
    ((void (*)(id, SEL, BOOL))objc_msgSend)((id)nsView, sel_registerName("setWantsLayer:"), YES);

    id layer = ((id (*)(id, SEL))objc_msgSend)((id)nsView, sel_registerName("layer"));

    // Check if the layer exists and is already a CAMetalLayer
    if (layer && [layer isKindOfClass:[CAMetalLayer class]]) {
        // Layer exists and is the correct type, proceed
    } else if (!layer) {
        // Layer doesn't exist, create a CAMetalLayer and set it
        CAMetalLayer* metalLayer = [CAMetalLayer layer];
        if (!metalLayer) {
             fprintf(stderr, "RGFW Error: Failed to create CAMetalLayer.\n");
             return NULL;
        }
        ((void (*)(id, SEL, id))objc_msgSend)((id)nsView, sel_registerName("setLayer:"), metalLayer);
        layer = metalLayer; // Use the newly created layer
        // printf("Info: Created and assigned CAMetalLayer for NSView.\n");
    } else {
        // Layer exists but is NOT a CAMetalLayer - this is an issue.
        // The view's layer needs to be explicitly set to CAMetalLayer for WebGPU.
        // This might require changes in how RGFW initializes the view when WebGPU is intended.
        fprintf(stderr, "RGFW Error: NSView's existing layer is not a CAMetalLayer. Cannot create WebGPU surface.\n");
        return NULL;
    }

    // At this point, 'layer' should be a valid CAMetalLayer*
    WGPUSurfaceSourceMetalLayer fromMetal = {0};
    fromMetal.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
    fromMetal.layer = (__bridge CAMetalLayer*)layer; // Use __bridge for ARC compatibility if mixing C/Obj-C

    surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromMetal.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);


#elif defined(RGFW_UNIX)
    // --- Unix (Wayland/X11) Implementation ---

    #if defined(RGFW_WAYLAND)
    // Check if Wayland is actively being used (if both X11 and Wayland are compiled)
    if (RGFW_usingWayland()) {
        if (window->src.wl_display && window->src.surface) {
            WGPUSurfaceSourceWaylandSurface fromWl = {0};
            fromWl.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
            fromWl.display = window->src.wl_display; // Get wl_display from RGFW
            fromWl.surface = window->src.surface;   // Get wl_surface from RGFW

            surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromWl.chain;
            return wgpuInstanceCreateSurface(instance, &surfaceDesc);
        }
        fprintf(stderr, "RGFW Info: Using Wayland, but wl_display or wl_surface is NULL.\n");
        return NULL; // Cannot create Wayland surface without handles
    }
    #endif // RGFW_WAYLAND

    #if defined(RGFW_X11)
    // Fallback to X11 if Wayland isn't used or not compiled
    // (or if RGFW_usingWayland() returned false)
    {
        if (window->src.display && window->src.window) {
            WGPUSurfaceSourceXlibWindow fromXlib = {0};
            fromXlib.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
            fromXlib.display = window->src.display; // Get Display* from RGFW
            fromXlib.window = window->src.window;   // Get Window from RGFW

            surfaceDesc.nextInChain = (WGPUChainedStruct*)&fromXlib.chain;
            return wgpuInstanceCreateSurface(instance, &surfaceDesc);
        }
        fprintf(stderr, "RGFW Info: Using X11 (or fallback), but Display* or Window is NULL.\n");
        return NULL; // Cannot create X11 surface without handles
    }
    #endif // RGFW_X11

    // If RGFW_UNIX is defined but neither RGFW_WAYLAND nor RGFW_X11 resulted in a surface
    fprintf(stderr, "RGFW Error: RGFW_UNIX defined, but no Wayland or X11 surface could be created.\n");
    return NULL;

#elif defined(__EMSCRIPTEN__)
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc = {0};
    canvasDesc.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasDesc.selector = (WGPUStringView){.data = "#canvas", .length = 7};
    
    surfaceDesc.nextInChain = &canvasDesc.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
#else
    // --- Other/Unsupported Platforms ---
    #warning "RGFW_GetWGPUSurface: Platform not explicitly supported (only Windows, macOS/Cocoa, Wayland, X11 implemented)."
    fprintf(stderr, "RGFW Error: Platform not supported by RGFW_GetWGPUSurface.\n");
    return NULL;
#endif
}