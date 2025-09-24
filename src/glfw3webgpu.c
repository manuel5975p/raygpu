#include "glfw3webgpu.h"

#include <webgpu/webgpu.h>

#include <GLFW/glfw3.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) 
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#if defined(RAYGPU_USE_X11)
#define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(RAYGPU_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#if !defined(__EMSCRIPTEN__)
#include "GLFW/glfw3native.h"
#endif

#ifdef GLFW_EXPOSE_NATIVE_COCOA
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#endif

WGPUSurface glfwCreateWindowWGPUSurface(WGPUInstance instance, GLFWwindow* window) {
#ifndef __EMSCRIPTEN__
    switch (glfwGetPlatform()) {
#else
    // glfwGetPlatform is not available in older versions of emscripten
    switch (-1) {
#endif

#ifdef GLFW_EXPOSE_NATIVE_X11
    case GLFW_PLATFORM_X11: {
        Display* x11_display = glfwGetX11Display();
        Window x11_window = glfwGetX11Window(window);

        WGPUSurfaceSourceXlibWindow fromXlibWindow;
        fromXlibWindow.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
        fromXlibWindow.chain.next = NULL;
        fromXlibWindow.display = x11_display;
        fromXlibWindow.window = x11_window;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromXlibWindow.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#endif // GLFW_EXPOSE_NATIVE_X11

#ifdef GLFW_EXPOSE_NATIVE_WAYLAND
    case GLFW_PLATFORM_WAYLAND: {
        struct wl_display* wayland_display = glfwGetWaylandDisplay();
        struct wl_surface* wayland_surface = glfwGetWaylandWindow(window);

        WGPUSurfaceSourceWaylandSurface fromWaylandSurface;
        fromWaylandSurface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
        fromWaylandSurface.chain.next = NULL;
        fromWaylandSurface.display = wayland_display;
        fromWaylandSurface.surface = wayland_surface;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromWaylandSurface.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#endif // GLFW_EXPOSE_NATIVE_WAYLAND

#ifdef GLFW_EXPOSE_NATIVE_COCOA
    case GLFW_PLATFORM_COCOA: {
        id metal_layer = [CAMetalLayer layer];
        NSWindow* ns_window = glfwGetCocoaWindow(window);
        [ns_window.contentView setWantsLayer : YES] ;
        [ns_window.contentView setLayer : metal_layer] ;

        WGPUSurfaceSourceMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
        fromMetalLayer.chain.next = NULL;
        fromMetalLayer.layer = metal_layer;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#endif // GLFW_EXPOSE_NATIVE_COCOA

#ifdef GLFW_EXPOSE_NATIVE_WIN32
    case GLFW_PLATFORM_WIN32: {
        HWND hwnd = glfwGetWin32Window(window);
        HINSTANCE hinstance = GetModuleHandle(NULL);

        WGPUSurfaceSourceWindowsHWND fromWindowsHWND;
        fromWindowsHWND.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
        fromWindowsHWND.chain.next = NULL;
        fromWindowsHWND.hinstance = hinstance;
        fromWindowsHWND.hwnd = hwnd;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromWindowsHWND.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#endif // GLFW_EXPOSE_NATIVE_WIN32

#ifdef GLFW_EXPOSE_NATIVE_EMSCRIPTEN
    case GLFW_PLATFORM_EMSCRIPTEN: {
#  ifdef WEBGPU_BACKEND_EMDAWNWEBGPU
        WGPUEmscriptenSurfaceSourceCanvasHTMLSelector fromCanvasHTMLSelector;
        fromCanvasHTMLSelector.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
        fromCanvasHTMLSelector.selector = (WGPUStringView){ "canvas", WGPU_STRLEN };
#  else
        WGPUSurfaceDescriptorFromCanvasHTMLSelector fromCanvasHTMLSelector;
        fromCanvasHTMLSelector.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
        fromCanvasHTMLSelector.selector = "canvas";
#  endif
        fromCanvasHTMLSelector.chain.next = NULL;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
#  ifdef WEBGPU_BACKEND_EMDAWNWEBGPU
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };
#  else
        surfaceDescriptor.label = NULL;
#  endif
        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#endif // GLFW_EXPOSE_NATIVE_EMSCRIPTEN

    default:
        // Unsupported platform
        return NULL;
    }
}
