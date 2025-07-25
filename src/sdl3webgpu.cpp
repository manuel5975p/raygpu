#include "sdl3webgpu.h"
#include "SDL3/SDL_properties.h"
#include "SDL3/SDL_video.h"

#include <webgpu/webgpu.h>
#include <string>
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
    #include <Cocoa/Cocoa.h>
    #include <Foundation/Foundation.h>
    #include <QuartzCore/CAMetalLayer.h>
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
    #include <UIKit/UIKit.h>
    #include <Foundation/Foundation.h>
    #include <QuartzCore/CAMetalLayer.h>
    #include <Metal/Metal.h>
#endif

WGPUSurface SDL3_GetWGPUSurface(WGPUInstance instance, SDL_Window* window) {
    //#if defined(SDL_VIDEO_DRIVER_X11)
    std::string drv = SDL_GetCurrentVideoDriver();
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
    WGPUSurfaceSourceWindowsHWND fromHwnd{};
    fromHwnd.hwnd = hwndPointer;
    fromHwnd.hinstance = hwndPointer;
    WGPUSurfaceDescriptor surfaceDescriptor{};
    surfaceDescriptor.nextInChain = &fromHwnd.chain;
    return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
#else
    if (drv == "x11") {
        Display *xdisplay = (Display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow) {
            WGPUSurfaceSourceXlibWindow fromXlibWindow{};
            fromXlibWindow.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
            fromXlibWindow.chain.next = NULL;
            fromXlibWindow.display = xdisplay;
            fromXlibWindow.window = xwindow;
            WGPUSurfaceDescriptor surfaceDescriptor{};
            surfaceDescriptor.nextInChain = &fromXlibWindow.chain;
            return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
        }
    }
    else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        struct wl_display *display = (struct wl_display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        struct wl_surface *surface = (struct wl_surface *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (display && surface) {
            WGPUSurfaceSourceWaylandSurface fromWl{};
            fromWl.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
            fromWl.chain.next = NULL;
            fromWl.display = display;
            fromWl.surface = surface;
            WGPUSurfaceDescriptor surfaceDescriptor{};
            surfaceDescriptor.nextInChain = &fromWl.chain;
            return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
        }
    }
#endif
    
    //#endif
    return nullptr;
}
