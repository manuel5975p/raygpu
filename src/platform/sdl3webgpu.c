// begin file src/sdl3webgpu.c
#define Font rlFont
#include <raygpu.h>
#undef Font

#include "sdl3webgpu.h"
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

#include <webgpu/webgpu.h>

#ifdef __APPLE__
    #define SDL_VIDEO_DRIVER_COCOA
#endif
#ifndef WEBGPU_BACKEND_DAWN
    #define WEBGPU_BACKEND_DAWN 1
#endif
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
//#include <SDL3/SDL_properties.h>
    #if RAYGPU_USE_X11 == 1
#include <X11/Xlib.h>
#endif
#if defined(SDL_VIDEO_DRIVER_COCOA)
    #ifndef APPLE_PLATFORM_HEADERS_INCLUDED
    #define APPLE_PLATFORM_HEADERS_INCLUDED
        #include <Cocoa/Cocoa.h>
        #include <Foundation/Foundation.h>
        #include <QuartzCore/CAMetalLayer.h>
    #endif
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
    #include <UIKit/UIKit.h>
    #include <Foundation/Foundation.h>
    #include <QuartzCore/CAMetalLayer.h>
    #include <Metal/Metal.h>
#endif

WGPUSurface SDL3_GetWGPUSurface(WGPUInstance instance, SDL_Window* window) {

#ifdef __EMSCRIPTEN__

    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector canvasDesc = {0};
    canvasDesc.chain.sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector;
    canvasDesc.selector = (WGPUStringView){.data = "#canvas", .length = 7};

    WGPUSurfaceDescriptor surfaceDesc = {0};
    surfaceDesc.nextInChain = &canvasDesc.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
#elif defined(ANDROID)
    void* awindow = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
    //void* asurface = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_SURFACE_POINTER, NULL);
    WGPUSurfaceSourceAndroidNativeWindow fromAndroidWindow{};
    fromAndroidWindow.chain.sType = WGPUSType_SurfaceSourceAndroidNativeWindow;
    fromAndroidWindow.chain.next = NULL;
    fromAndroidWindow.window = awindow;
    WGPUSurfaceDescriptor surfaceDescriptor{};
    surfaceDescriptor.nextInChain = &fromAndroidWindow.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#elif defined(_WIN32)
    void* hwndPointer = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    void* instancePointer = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL);
    WGPUSurfaceSourceWindowsHWND fromHwnd = {
        .chain = {
            .sType = WGPUSType_SurfaceSourceWindowsHWND,
        },
        .hinstance = hwndPointer,
        .hwnd = hwndPointer,
    };
    WGPUSurfaceDescriptor surfaceDescriptor = {
        .nextInChain = &fromHwnd.chain
    };
    return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#elif !defined(__APPLE__)
    #if RAYGPU_USE_X11 == 1
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        Display *xdisplay = (Display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow) {
            WGPUSurfaceSourceXlibWindow fromXlibWindow = {
                .chain.sType = WGPUSType_SurfaceSourceXlibWindow,
                .chain.next = NULL,
                .display = xdisplay,
                .window = xwindow,
            };

            const WGPUSurfaceDescriptor surfaceDescriptor = {
                .nextInChain = &fromXlibWindow.chain
            };

            return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
        }
    }
    else 
    #endif
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        struct wl_display* display = (struct wl_display*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        struct wl_surface* surface = (struct wl_surface*)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (display && surface) {

            WGPUSurfaceColorManagement cmanagement = {
                .chain = {
                    .sType = WGPUSType_SurfaceColorManagement
                },
                .colorSpace = WGPUPredefinedColorSpace_SRGB,
                .toneMappingMode = WGPUToneMappingMode_Extended,
            };
            WGPUSurfaceSourceWaylandSurface fromWl = {
                .chain = {
                    .sType = WGPUSType_SurfaceSourceWaylandSurface,
                    .next = NULL//&cmanagement.chain
                },
                .display = display,
                .surface = surface,
            };
            const WGPUSurfaceDescriptor surfaceDescriptor = {
                .nextInChain = &fromWl.chain
            };

            return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
        }
    }
    #elif defined(SDL_VIDEO_DRIVER_COCOA)
    {
        id metal_layer = NULL;
        NSWindow *ns_window = (__bridge NSWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
        if (!ns_window) return NULL;
        [ns_window.contentView setWantsLayer : YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer : metal_layer];
        CGSize viewSize = ns_window.contentView.bounds.size;
        CGSize drawableSize;
        
        CGFloat scale = ns_window.backingScaleFactor;
        drawableSize.width = viewSize.width * scale;
        drawableSize.height = viewSize.height * scale;
        CAMetalLayer* ml = (CAMetalLayer*)metal_layer;
        ml.drawableSize = drawableSize;

        // TRACELOG(LOG_INFO, "Scale factor: %f", ns_window.backingScaleFactor);
        // TRACELOG(LOG_INFO, "Drawable_size: %d, %d", drawableSize.width, drawableSize.height);

        WGPUSurfaceColorManagement cmanagement = {
                .chain = {
                    .sType = WGPUSType_SurfaceColorManagement
                },
                .colorSpace = WGPUPredefinedColorSpace_DisplayP3,
                .toneMappingMode = WGPUToneMappingMode_Standard,
        };
        WGPUSurfaceSourceMetalLayer fromMetalLayer = {0};
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
        fromMetalLayer.chain.next = NULL;//&cmanagement.chain;
        fromMetalLayer.layer = ml;

        WGPUSurfaceDescriptor surfaceDescriptor = {0};
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
    #endif
    
    //#endif
    return NULL;
}



// end file src/sdl3webgpu.c