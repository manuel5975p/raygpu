#define Font FontDifferentName
#include <raygpu.h>
#undef Font
#include <SDL2/SDL.h>

#include <cstdlib>
#include <memory>
#include <utility>
#include <iostream>
#include <sdl2webgpu.h>

//#include "dawn/common/Log.h"
#include "dawn/common/Platform.h"
#include "webgpu/webgpu_glfw.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#if defined(DAWN_USE_X11)
//#define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(DAWN_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#if !defined(__EMSCRIPTEN__)
//#include "GLFW/glfw3native.h"
#endif
#include <X11/Xlib.h>
#include <X11/X.h>


std::pair<Display*, Window> sdlGetWindowHandle(SDL_Window* window){
#if defined(SDL_PLATFORM_WIN32)
    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
    if (hwnd) {
        ...
    }
#elif defined(SDL_PLATFORM_MACOS)
    NSWindow *nswindow = (__bridge NSWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    if (nswindow) {
        ...
    }
#elif defined(SDL_PLATFORM_LINUX)
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
        Display *xdisplay = (Display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        Window xwindow = (Window)SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
        if (xdisplay && xwindow) {
            return {xdisplay, xwindow};
        }
    } else if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        struct wl_display *display = (struct wl_display *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        struct wl_surface *surface = (struct wl_surface *)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        if (display && surface) {
            
        }
    }
#elif defined(SDL_PLATFORM_IOS)
    SDL_PropertiesID props = SDL_GetWindowProperties(window);
    UIWindow *uiwindow = (__bridge UIWindow *)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
    if (uiwindow) {
        GLuint framebuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_FRAMEBUFFER_NUMBER, 0);
        GLuint colorbuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RENDERBUFFER_NUMBER, 0);
        GLuint resolveFramebuffer = (GLuint)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_UIKIT_OPENGL_RESOLVE_FRAMEBUFFER_NUMBER, 0);
        ...
    }
#endif
    return {};
}















// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


namespace wgpu::glfw {



// SetupWindowAndGetSurfaceDescriptorCocoa defined in GLFWUtils_metal.mm
std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)>
SetupWindowAndGetSurfaceDescriptorCocoa(SDL_Window* window);

std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct*)>
SetupWindowAndGetSurfaceDescriptor(SDL_Window* window) {
    //if (glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API) {
    //    //std::cerr << "GL context was created on the window. Disable context creation by "
    //    //                    "setting the GLFW_CLIENT_API hint to GLFW_NO_API.";
    //    return {nullptr, [](wgpu::ChainedStruct*) {}};
    //}
#if DAWN_PLATFORM_IS(WINDOWS)
    wgpu::SurfaceSourceWindowsHWND* desc = new wgpu::SurfaceSourceWindowsHWND();
    desc->hwnd = glfwGetWin32Window(window);
    desc->hinstance = GetModuleHandle(nullptr);
    return {desc, [](wgpu::ChainedStruct* desc) {
                delete reinterpret_cast<wgpu::SurfaceSourceWindowsHWND*>(desc);
            }};
#elif defined(DAWN_ENABLE_BACKEND_METAL)
    return SetupWindowAndGetSurfaceDescriptorCocoa(window);
#elif defined(DAWN_USE_WAYLAND) || defined(DAWN_USE_X11)
#if defined(GLFW_PLATFORM_WAYLAND) && defined(DAWN_USE_WAYLAND)
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        wgpu::SurfaceSourceWaylandSurface* desc = new wgpu::SurfaceSourceWaylandSurface();
        desc->display = glfwGetWaylandDisplay();
        desc->surface = glfwGetWaylandWindow(window);
        return {desc, [](wgpu::ChainedStruct* desc) {
                    delete reinterpret_cast<wgpu::SurfaceSourceWaylandSurface*>(desc);
                }};
    } else  // NOLINT(readability/braces)
#endif
#if defined(DAWN_USE_X11)
    {
        wgpu::SurfaceSourceXlibWindow* desc = new wgpu::SurfaceSourceXlibWindow();
        std::tie(desc->display, desc->window) = sdlGetWindowHandle(window);
        return {desc, [](wgpu::ChainedStruct* desc) {
                    delete reinterpret_cast<wgpu::SurfaceSourceXlibWindow*>(desc);
                }};
    }
#else
    {
        return {nullptr, [](wgpu::ChainedStruct*) {}};
    }
#endif
#else
    return {nullptr, [](wgpu::ChainedStruct*) {}};
#endif
}
wgpu::Surface CreateSurfaceForWindow(const wgpu::Instance& instance, SDL_Window* window) {
    auto chainedDescriptor = SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();
    wgpu::Surface surface = instance.CreateSurface(&descriptor);

    return surface;
}
}  // namespace wgpu::glfw



void InitWindow_SDL3(){
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SDLFanschter", 0, 0, 500, 500, 0);
    WGPUSurface csurf = SDL_GetWGPUSurface(GetInstance(), window);
    wgpu::Surface surf(csurf);
    WGPUSurfaceCapabilities capa;
    WGPUAdapter adapter = GetAdapter();
    //std::cout << surf.Get() << "\n";
    //surf.GetCapabilities(GetCXXAdapter(), &capa);
    wgpuSurfaceGetCapabilities(csurf, adapter, &capa);
    std::cout << capa.presentModeCount << "\n";
    WGPUSurfaceConfiguration config{};
    config.presentMode = WGPUPresentMode_Fifo;
    config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = WGPUTextureFormat_BGRA8Unorm;
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = 500;
    config.height = 500;
    config.viewFormats = &config.format;
    config.viewFormatCount = 1;
    config.device = GetDevice();
    wgpuSurfaceConfigure(csurf, &config);
    for(;;){
        WGPUSurfaceTexture stex;
        wgpuSurfaceGetCurrentTexture(csurf, &stex);
        wgpuSurfacePresent(csurf);
    }
    //surf.Configure(&config);
    //wgpu::SurfaceTexture tex;
    //surf.Present();
    //std::cout << SDL_GetCurrentVideoDriver() << "\n";
}
