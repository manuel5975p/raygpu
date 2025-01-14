/**
 * This is an extension of SDL2 for WebGPU, abstracting away the details of
 * OS-specific operations.
 * 
 * This file is part of the "Learn WebGPU for C++" book.
 *   https://eliemichel.github.io/LearnWebGPU
 * 
 * Most of this code comes from the wgpu-native triangle example:
 *   https://github.com/gfx-rs/wgpu-native/blob/master/examples/triangle/main.c
 * 
 * MIT License
 * Copyright (c) 2022-2023 Elie Michel and the wgpu-native authors
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sdl2webgpu.h"

#include <webgpu/webgpu.h>

#if defined(SDL_VIDEO_DRIVER_COCOA)
#  include <Cocoa/Cocoa.h>
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
#  include <UIKit/UIKit.h>
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#  include <Metal/Metal.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

WGPUSurface SDL_GetWGPUSurface(WGPUInstance instance, SDL_Window* window) {
    SDL_SysWMinfo windowWMInfo;
    SDL_VERSION(&windowWMInfo.version)
    SDL_GetWindowWMInfo(window, &windowWMInfo);

#if defined(SDL_VIDEO_DRIVER_COCOA)
    {
        id metal_layer = NULL;
        NSWindow* ns_window = windowWMInfo.info.cocoa.window;
        [ns_window.contentView setWantsLayer : YES];
        metal_layer = [CAMetalLayer layer];
        [ns_window.contentView setLayer : metal_layer];

#  ifdef WEBGPU_BACKEND_DAWN
        WGPUSurfaceSourceMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
#  else
        WGPUSurfaceDescriptorFromMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceDescriptorFromMetalLayer;
#  endif
        fromMetalLayer.chain.next = NULL;
        fromMetalLayer.layer = metal_layer;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){NULL, 0};

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_VIDEO_DRIVER_UIKIT)
    {
        UIWindow* ui_window = windowWMInfo.info.uikit.window;
        UIView* ui_view = ui_window.rootViewController.view;
        CAMetalLayer* metal_layer = [CAMetalLayer new];
        metal_layer.opaque = true;
        metal_layer.frame = ui_view.frame;
        metal_layer.drawableSize = ui_view.frame.size;

        [ui_view.layer addSublayer: metal_layer];

#  ifdef WEBGPU_BACKEND_DAWN
        WGPUSurfaceSourceMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceSourceMetalLayer;
#  else
        WGPUSurfaceDescriptorFromMetalLayer fromMetalLayer;
        fromMetalLayer.chain.sType = WGPUSType_SurfaceDescriptorFromMetalLayer;
#  endif
        fromMetalLayer.chain.next = NULL;
        fromMetalLayer.layer = metal_layer;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromMetalLayer.chain;
        surfaceDescriptor.label = (WGPUStringView){NULL, 0};

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_VIDEO_DRIVER_X11)
    {
        Display* x11_display = windowWMInfo.info.x11.display;
        Window x11_window = windowWMInfo.info.x11.window;

#  ifdef WEBGPU_BACKEND_DAWN
        WGPUSurfaceSourceXlibWindow fromXlibWindow;
        fromXlibWindow.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
#  else
        WGPUSurfaceDescriptorFromXlibWindow fromXlibWindow;
        fromXlibWindow.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
#  endif
        fromXlibWindow.chain.next = NULL;
        fromXlibWindow.display = x11_display;
        fromXlibWindow.window = x11_window;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromXlibWindow.chain;
        surfaceDescriptor.label = (WGPUStringView){NULL, 0};

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_VIDEO_DRIVER_WAYLAND)
    {
        struct wl_display* wayland_display = windowWMInfo.info.wl.display;
        struct wl_surface* wayland_surface = windowWMInfo.info.wl.surface;

#  ifdef WEBGPU_BACKEND_DAWN
        WGPUSurfaceSourceWaylandSurface fromWaylandSurface;
        fromWaylandSurface.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
#  else
        WGPUSurfaceDescriptorFromWaylandSurface fromWaylandSurface;
        fromWaylandSurface.chain.sType = WGPUSType_SurfaceDescriptorFromWaylandSurface;
#  endif
        fromWaylandSurface.chain.next = NULL;
        fromWaylandSurface.display = wayland_display;
        fromWaylandSurface.surface = wayland_surface;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromWaylandSurface.chain;
        surfaceDescriptor.label = (WGPUStringView){NULL, 0};

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_VIDEO_DRIVER_WINDOWS)
    {
        HWND hwnd = windowWMInfo.info.win.window;
        HINSTANCE hinstance = GetModuleHandle(NULL);

#  ifdef WEBGPU_BACKEND_DAWN
        WGPUSurfaceSourceWindowsHWND fromWindowsHWND;
        fromWindowsHWND.chain.sType = WGPUSType_SurfaceSourceWindowsHWND;
#  else
        WGPUSurfaceDescriptorFromWindowsHWND fromWindowsHWND;
        fromWindowsHWND.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
#  endif
        fromWindowsHWND.chain.next = NULL;
        fromWindowsHWND.hinstance = hinstance;
        fromWindowsHWND.hwnd = hwnd;

        WGPUSurfaceDescriptor surfaceDescriptor;
        surfaceDescriptor.nextInChain = &fromWindowsHWND.chain;
        surfaceDescriptor.label = (WGPUStringView){NULL, 0};

        return wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    }
#elif defined(SDL_VIDEO_DRIVER_EMSCRIPTEN)
    {
        WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc = {0};
        canvasDesc.selector = (WGPUStringView){.data = "#canvas", .length = 7};

        WGPUSurfaceDescriptor surfaceDesc = {0};
        surfaceDesc.nextInChain = &canvasDesc.chain;
        return wgpuInstanceCreateSurface(instance, &surfaceDesc);
    }  
#else
    // TODO: See SDL_syswm.h for other possible enum values!
#error "Unsupported WGPU_TARGET"
#endif
}

