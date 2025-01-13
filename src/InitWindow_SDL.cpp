#define Font FontDifferentName
#include <raygpu.h>
#undef Font
#include <SDL2/SDL.h>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sdl2webgpu.h>
#include <utility>
#include <wgpustate.inc>
extern wgpustate g_wgpustate;
// #include "dawn/common/Log.h"
#ifdef DONT_NEED_THIS_RIGHT_NOW
#include "dawn/common/Platform.h"
#include "webgpu/webgpu_glfw.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#if defined(DAWN_USE_X11)
// #define GLFW_EXPOSE_NATIVE_X11
#endif
#if defined(DAWN_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#if !defined(__EMSCRIPTEN__)
// #include "GLFW/glfw3native.h"
#endif
#include <X11/X.h>
#include <X11/Xlib.h>

std::pair<Display *, Window> sdlGetWindowHandle(SDL_Window *window) {
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
std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct *)> SetupWindowAndGetSurfaceDescriptorCocoa(SDL_Window *window);

std::unique_ptr<wgpu::ChainedStruct, void (*)(wgpu::ChainedStruct *)> SetupWindowAndGetSurfaceDescriptor(SDL_Window *window) {
    // if (glfwGetWindowAttrib(window, GLFW_CLIENT_API) != GLFW_NO_API) {
    //     //std::cerr << "GL context was created on the window. Disable context creation by "
    //     //                    "setting the GLFW_CLIENT_API hint to GLFW_NO_API.";
    //     return {nullptr, [](wgpu::ChainedStruct*) {}};
    // }
#if DAWN_PLATFORM_IS(WINDOWS)
    wgpu::SurfaceSourceWindowsHWND *desc = new wgpu::SurfaceSourceWindowsHWND();
    desc->hwnd = glfwGetWin32Window(window);
    desc->hinstance = GetModuleHandle(nullptr);
    return {desc, [](wgpu::ChainedStruct *desc) { delete reinterpret_cast<wgpu::SurfaceSourceWindowsHWND *>(desc); }};
#elif defined(DAWN_ENABLE_BACKEND_METAL)
    return SetupWindowAndGetSurfaceDescriptorCocoa(window);
#elif defined(DAWN_USE_WAYLAND) || defined(DAWN_USE_X11)
#if defined(GLFW_PLATFORM_WAYLAND) && defined(DAWN_USE_WAYLAND)
    if (glfwGetPlatform() == GLFW_PLATFORM_WAYLAND) {
        wgpu::SurfaceSourceWaylandSurface *desc = new wgpu::SurfaceSourceWaylandSurface();
        desc->display = glfwGetWaylandDisplay();
        desc->surface = glfwGetWaylandWindow(window);
        return {desc, [](wgpu::ChainedStruct *desc) { delete reinterpret_cast<wgpu::SurfaceSourceWaylandSurface *>(desc); }};
    } else // NOLINT(readability/braces)
#endif
#if defined(DAWN_USE_X11)
    {
        wgpu::SurfaceSourceXlibWindow *desc = new wgpu::SurfaceSourceXlibWindow();
        std::tie(desc->display, desc->window) = sdlGetWindowHandle(window);
        return {desc, [](wgpu::ChainedStruct *desc) { delete reinterpret_cast<wgpu::SurfaceSourceXlibWindow *>(desc); }};
    }
#else
    {
        return {nullptr, [](wgpu::ChainedStruct *) {}};
    }
#endif
#else
    return {nullptr, [](wgpu::ChainedStruct *) {}};
#endif
}
wgpu::Surface CreateSurfaceForWindow(const wgpu::Instance &instance, SDL_Window *window) {
    auto chainedDescriptor = SetupWindowAndGetSurfaceDescriptor(window);

    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = chainedDescriptor.get();
    wgpu::Surface surface = instance.CreateSurface(&descriptor);

    return surface;
}
} // namespace wgpu::glfw
#endif
bool initied__sdl = false;
void Initialize_SDL(){
    if(!initied__sdl){
        SDL_SetMainReady();
        SDL_Init(SDL_INIT_VIDEO);
    }
    initied__sdl = true;
}
void negotiateSurfaceFormatAndPresentMode(const wgpu::Surface &surf);
extern "C" SubWindow InitWindow_SDL2(uint32_t width, uint32_t height, const char *title) {
    
    SubWindow ret;
    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    SDL_SetWindowResizable(window, SDL_bool(g_wgpustate.windowFlags & FLAG_WINDOW_RESIZABLE));
    WGPUSurface csurf = SDL_GetWGPUSurface(GetInstance(), window);
    negotiateSurfaceFormatAndPresentMode((const wgpu::Surface &)csurf);
    WGPUSurfaceCapabilities capa;
    WGPUAdapter adapter = GetAdapter();
    // std::cout << surf.Get() << "\n";
    // surf.GetCapabilities(GetCXXAdapter(), &capa);
    wgpuSurfaceGetCapabilities(csurf, adapter, &capa);
    // std::cout << capa.presentModeCount << "\n";
    // for(uint32_t i = 0;i < capa.presentModeCount;i++){
    //     std::cout << "Sdl supports " << textureFormatSpellingTable.at(capa.formats[i]) << "\n";
    // }
    WGPUSurfaceConfiguration config{};
    if (g_wgpustate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT) {
        config.presentMode = (WGPUPresentMode)(((g_wgpustate.unthrottled_PresentMode == wgpu::PresentMode::Mailbox) ? g_wgpustate.unthrottled_PresentMode : g_wgpustate.throttled_PresentMode));
    } else if (g_wgpustate.windowFlags & FLAG_VSYNC_HINT) {
        config.presentMode = (WGPUPresentMode)g_wgpustate.throttled_PresentMode;
    } else {
        config.presentMode = (WGPUPresentMode)g_wgpustate.unthrottled_PresentMode;
    }
    config.presentMode = WGPUPresentMode_Immediate;
    config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = g_wgpustate.frameBufferFormat;
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    config.viewFormats = &config.format;
    config.viewFormatCount = 1;
    config.device = GetDevice();
    wgpuSurfaceConfigure(csurf, &config);
    // surf.Configure(&config);
    // wgpu::SurfaceTexture tex;
    // surf.Present();
    // std::cout << SDL_GetCurrentVideoDriver() << "\n";
    ret.handle = window;
    ret.frameBuffer = LoadRenderTexture(width, height);
    ret.surface = csurf;
    g_wgpustate.createdSubwindows[ret.handle] = ret;
    return ret;
}

SubWindow OpenSubWindow_SDL(uint32_t width, uint32_t height, const char* title){
    SubWindow ret;
    ret.handle = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    WGPUInstance inst = GetInstance();
    wgpu::Surface secondSurface = SDL_GetWGPUSurface(inst, (SDL_Window*)ret.handle);
    wgpu::SurfaceCapabilities capabilities;
    secondSurface.GetCapabilities(GetCXXAdapter(), &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = GetCXXDevice();
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    config.format = (wgpu::TextureFormat)g_wgpustate.frameBufferFormat;
    config.presentMode = g_wgpustate.unthrottled_PresentMode;
    config.width = width;
    config.height = height;
    secondSurface.Configure(&config);
    ret.surface = secondSurface.MoveToCHandle();
    ret.frameBuffer = LoadRenderTexture(config.width, config.height);
    g_wgpustate.createdSubwindows[ret.handle] = ret;
    g_wgpustate.input_map[(GLFWwindow*)ret.handle] = window_input_state{};
    //setupGLFWCallbacks((GLFWwindow*)ret.handle);
    return ret;
}


void ResizeCallback(SDL_Window* window, int width, int height){
    //wgpuSurfaceRelease(g_wgpustate.surface);
    //g_wgpustate.surface = wgpu::glfw::CreateSurfaceForWindow(g_wgpustate.instance, window).MoveToCHandle();
    //while(!g_wgpustate.drawmutex.try_lock());
    //g_wgpustate.drawmutex.lock();
    
    TraceLog(LOG_WARNING, "glfwSizeCallback called with %d x %d", width, height);
    wgpu::SurfaceCapabilities capabilities;
    g_wgpustate.surface.GetCapabilities(g_wgpustate.adapter, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    config.device = g_wgpustate.device;
    config.format = (wgpu::TextureFormat)g_wgpustate.frameBufferFormat;
    config.presentMode = (wgpu::PresentMode)(!!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) ? g_wgpustate.throttled_PresentMode : g_wgpustate.unthrottled_PresentMode);
    config.width = width;
    config.height = height;
    g_wgpustate.width = width;
    g_wgpustate.height = height;
    auto& toBeResizedRendertexture = g_wgpustate.createdSubwindows[window].frameBuffer;
    UnloadTexture(toBeResizedRendertexture.colorMultisample);
    if(g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT)
        toBeResizedRendertexture.colorMultisample = LoadTexturePro(width, height, g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 4, 1);
    UnloadTexture(toBeResizedRendertexture.depth);
    toBeResizedRendertexture.depth = LoadTexturePro(width,
                              height, 
                              WGPUTextureFormat_Depth24Plus, 
                              WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                              (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                              1
    );
    toBeResizedRendertexture.texture.width = width;
    toBeResizedRendertexture.texture.height = height;
    wgpuSurfaceConfigure(g_wgpustate.createdSubwindows[window].surface, (WGPUSurfaceConfiguration*)&config);
    
    if((void*)window == (void*)g_wgpustate.window){
        g_wgpustate.mainWindowRenderTarget = toBeResizedRendertexture;
    }
    Matrix newcamera = ScreenMatrix(width, height);
    //BufferData(g_wgpustate.defaultScreenMatrix, &newcamera, sizeof(Matrix));
    //setTargetTextures(g_wgpustate.rstate, g_wgpustate.rstate->color, g_wgpustate.currentDefaultRenderTarget.colorMultisample.view, g_wgpustate.currentDefaultRenderTarget.depth.view);
    //updateRenderPassDesc(g_wgpustate.rstate);
    //TODO wtf is this?
    //g_wgpustate.rstate->renderpass.dsa->view = g_wgpustate.currentDefaultRenderTarget.depth.view;
    //g_wgpustate.drawmutex.unlock();
}

void CharCallback(SDL_Window* window, unsigned int codePoint){
    g_wgpustate.input_map[window].charQueue.push_back((int)codePoint);
}
void CursorEnterCallback(SDL_Window* window, int entered){
    g_wgpustate.input_map[window].cursorInWindow = entered;
}
void MousePositionCallback(SDL_Window* window, double x, double y){
    g_wgpustate.input_map[window].mousePos = Vector2{float(x), float(y)};
}
void KeyUpCallback (SDL_Window* window, int key, int scancode, int mods){
    g_wgpustate.input_map[window].keydown[key] = 0;
    //if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    //    EndGIFRecording();
    //    glfwSetWindowShouldClose(window, true);
    //}
}
void ScrollCallback(SDL_Window* window, double xoffset, double yoffset){
    g_wgpustate.input_map[window].scrollThisFrame.x += xoffset;
    g_wgpustate.input_map[window].scrollThisFrame.y += yoffset;
}
void KeyDownCallback (SDL_Window* window, int key, int scancode, int mods){
    g_wgpustate.input_map[window].keydown[key] = 1;
    //if(action == GLFW_PRESS){
    //    g_wgpustate.input_map[window].keydown[key] = 1;
    //}else if(action == GLFW_RELEASE){
    //    g_wgpustate.input_map[window].keydown[key] = 0;
    //}
    //if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    //    EndGIFRecording();
    //    glfwSetWindowShouldClose(window, true);
    //}
}

void MouseButtonCallback(SDL_Window* window, int button, int action){
    if(action == SDL_PRESSED){
        g_wgpustate.input_map[window].mouseButtonDown[button] = 1;
    }
    else if(action == SDL_RELEASED){
        g_wgpustate.input_map[window].mouseButtonDown[button] = 0;
    }
}

#define SCANCODE_MAPPED_NUM 232
static const KeyboardKey mapScancodeToKey[SCANCODE_MAPPED_NUM] = {
    KEY_NULL,           // SDL_SCANCODE_UNKNOWN
    (KeyboardKey)0,
    (KeyboardKey)0,
    (KeyboardKey)0,
    KEY_A,              // SDL_SCANCODE_A
    KEY_B,              // SDL_SCANCODE_B
    KEY_C,              // SDL_SCANCODE_C
    KEY_D,              // SDL_SCANCODE_D
    KEY_E,              // SDL_SCANCODE_E
    KEY_F,              // SDL_SCANCODE_F
    KEY_G,              // SDL_SCANCODE_G
    KEY_H,              // SDL_SCANCODE_H
    KEY_I,              // SDL_SCANCODE_I
    KEY_J,              // SDL_SCANCODE_J
    KEY_K,              // SDL_SCANCODE_K
    KEY_L,              // SDL_SCANCODE_L
    KEY_M,              // SDL_SCANCODE_M
    KEY_N,              // SDL_SCANCODE_N
    KEY_O,              // SDL_SCANCODE_O
    KEY_P,              // SDL_SCANCODE_P
    KEY_Q,              // SDL_SCANCODE_Q
    KEY_R,              // SDL_SCANCODE_R
    KEY_S,              // SDL_SCANCODE_S
    KEY_T,              // SDL_SCANCODE_T
    KEY_U,              // SDL_SCANCODE_U
    KEY_V,              // SDL_SCANCODE_V
    KEY_W,              // SDL_SCANCODE_W
    KEY_X,              // SDL_SCANCODE_X
    KEY_Y,              // SDL_SCANCODE_Y
    KEY_Z,              // SDL_SCANCODE_Z
    KEY_ONE,            // SDL_SCANCODE_1
    KEY_TWO,            // SDL_SCANCODE_2
    KEY_THREE,          // SDL_SCANCODE_3
    KEY_FOUR,           // SDL_SCANCODE_4
    KEY_FIVE,           // SDL_SCANCODE_5
    KEY_SIX,            // SDL_SCANCODE_6
    KEY_SEVEN,          // SDL_SCANCODE_7
    KEY_EIGHT,          // SDL_SCANCODE_8
    KEY_NINE,           // SDL_SCANCODE_9
    KEY_ZERO,           // SDL_SCANCODE_0
    KEY_ENTER,          // SDL_SCANCODE_RETURN
    KEY_ESCAPE,         // SDL_SCANCODE_ESCAPE
    KEY_BACKSPACE,      // SDL_SCANCODE_BACKSPACE
    KEY_TAB,            // SDL_SCANCODE_TAB
    KEY_SPACE,          // SDL_SCANCODE_SPACE
    KEY_MINUS,          // SDL_SCANCODE_MINUS
    KEY_EQUAL,          // SDL_SCANCODE_EQUALS
    KEY_LEFT_BRACKET,   // SDL_SCANCODE_LEFTBRACKET
    KEY_RIGHT_BRACKET,  // SDL_SCANCODE_RIGHTBRACKET
    KEY_BACKSLASH,      // SDL_SCANCODE_BACKSLASH
    (KeyboardKey)0,                  // SDL_SCANCODE_NONUSHASH
    KEY_SEMICOLON,      // SDL_SCANCODE_SEMICOLON
    KEY_APOSTROPHE,     // SDL_SCANCODE_APOSTROPHE
    KEY_GRAVE,          // SDL_SCANCODE_GRAVE
    KEY_COMMA,          // SDL_SCANCODE_COMMA
    KEY_PERIOD,         // SDL_SCANCODE_PERIOD
    KEY_SLASH,          // SDL_SCANCODE_SLASH
    KEY_CAPS_LOCK,      // SDL_SCANCODE_CAPSLOCK
    KEY_F1,             // SDL_SCANCODE_F1
    KEY_F2,             // SDL_SCANCODE_F2
    KEY_F3,             // SDL_SCANCODE_F3
    KEY_F4,             // SDL_SCANCODE_F4
    KEY_F5,             // SDL_SCANCODE_F5
    KEY_F6,             // SDL_SCANCODE_F6
    KEY_F7,             // SDL_SCANCODE_F7
    KEY_F8,             // SDL_SCANCODE_F8
    KEY_F9,             // SDL_SCANCODE_F9
    KEY_F10,            // SDL_SCANCODE_F10
    KEY_F11,            // SDL_SCANCODE_F11
    KEY_F12,            // SDL_SCANCODE_F12
    KEY_PRINT_SCREEN,   // SDL_SCANCODE_PRINTSCREEN
    KEY_SCROLL_LOCK,    // SDL_SCANCODE_SCROLLLOCK
    KEY_PAUSE,          // SDL_SCANCODE_PAUSE
    KEY_INSERT,         // SDL_SCANCODE_INSERT
    KEY_HOME,           // SDL_SCANCODE_HOME
    KEY_PAGE_UP,        // SDL_SCANCODE_PAGEUP
    KEY_DELETE,         // SDL_SCANCODE_DELETE
    KEY_END,            // SDL_SCANCODE_END
    KEY_PAGE_DOWN,      // SDL_SCANCODE_PAGEDOWN
    KEY_RIGHT,          // SDL_SCANCODE_RIGHT
    KEY_LEFT,           // SDL_SCANCODE_LEFT
    KEY_DOWN,           // SDL_SCANCODE_DOWN
    KEY_UP,             // SDL_SCANCODE_UP
    KEY_NUM_LOCK,       // SDL_SCANCODE_NUMLOCKCLEAR
    KEY_KP_DIVIDE,      // SDL_SCANCODE_KP_DIVIDE
    KEY_KP_MULTIPLY,    // SDL_SCANCODE_KP_MULTIPLY
    KEY_KP_SUBTRACT,    // SDL_SCANCODE_KP_MINUS
    KEY_KP_ADD,         // SDL_SCANCODE_KP_PLUS
    KEY_KP_ENTER,       // SDL_SCANCODE_KP_ENTER
    KEY_KP_1,           // SDL_SCANCODE_KP_1
    KEY_KP_2,           // SDL_SCANCODE_KP_2
    KEY_KP_3,           // SDL_SCANCODE_KP_3
    KEY_KP_4,           // SDL_SCANCODE_KP_4
    KEY_KP_5,           // SDL_SCANCODE_KP_5
    KEY_KP_6,           // SDL_SCANCODE_KP_6
    KEY_KP_7,           // SDL_SCANCODE_KP_7
    KEY_KP_8,           // SDL_SCANCODE_KP_8
    KEY_KP_9,           // SDL_SCANCODE_KP_9
    KEY_KP_0,           // SDL_SCANCODE_KP_0
    KEY_KP_DECIMAL,     // SDL_SCANCODE_KP_PERIOD
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0, (KeyboardKey)0,
    KEY_LEFT_CONTROL,   //SDL_SCANCODE_LCTRL
    KEY_LEFT_SHIFT,     //SDL_SCANCODE_LSHIFT
    KEY_LEFT_ALT,       //SDL_SCANCODE_LALT
    KEY_LEFT_SUPER,     //SDL_SCANCODE_LGUI
    KEY_RIGHT_CONTROL,  //SDL_SCANCODE_RCTRL
    KEY_RIGHT_SHIFT,    //SDL_SCANCODE_RSHIFT
    KEY_RIGHT_ALT,      //SDL_SCANCODE_RALT
    KEY_RIGHT_SUPER     //SDL_SCANCODE_RGUI
};


static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode){
    if ((sdlScancode >= 0) && (sdlScancode < SCANCODE_MAPPED_NUM))
    {
        return mapScancodeToKey[sdlScancode];
    }

    return KEY_NULL; // No equivalent key in Raylib
}
extern "C" void PollEvents_SDL() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        
        switch (event.type) {
        case SDL_QUIT:{
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            g_wgpustate.closeFlag = true;
        }break;
        case SDL_KEYDOWN:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            KeyDownCallback(window, ConvertScancodeToKey(event.key.keysym.scancode), event.key.keysym.scancode, event.key.keysym.mod);
            if(event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
                g_wgpustate.closeFlag = true;
            }
        }break;
        case SDL_KEYUP:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            KeyUpCallback(window, ConvertScancodeToKey(event.key.keysym.scancode), event.key.keysym.scancode, event.key.keysym.mod);
        }break;
        case SDL_WINDOWEVENT: {
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                int newWidth = event.window.data1;
                int newHeight = event.window.data2;
                ResizeCallback(window, newWidth, newHeight);
            } else if (event.window.event == SDL_WINDOWEVENT_ENTER) {
                CursorEnterCallback(window, true);
            } else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
                CursorEnterCallback(window, false);
            } else if(event.window.event == SDL_WINDOWEVENT_CLOSE){
                g_wgpustate.closeFlag = true;
            }
            // Handle other window events if necessary
        } break;

        case SDL_MOUSEWHEEL: {
            SDL_Window *window = SDL_GetWindowFromID(event.wheel.windowID);
            // Note: SDL's yoffset is positive when scrolling up, negative when scrolling down
            ScrollCallback(window, event.wheel.x, event.wheel.y);
        } break;

        case SDL_MOUSEMOTION: {
            SDL_Window *window = SDL_GetWindowFromID(event.motion.windowID);
            MousePositionCallback(window, event.motion.x, event.motion.y);
        } break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            SDL_Window *window = SDL_GetWindowFromID(event.button.windowID);
            Uint8 state = (event.type == SDL_MOUSEBUTTONDOWN) ? SDL_PRESSED : SDL_RELEASED;

            uint8_t forGLFW = event.button.button - 1;
            if (forGLFW == 2) forGLFW = 1;
            else if (forGLFW == 1) forGLFW = 2;
            MouseButtonCallback(window, forGLFW, state);
        } break;

        case SDL_TEXTINPUT: {
            SDL_Window *window = SDL_GetWindowFromID(event.text.windowID);
            unsigned int codePoint = 0;
            // Convert UTF-8 text to Unicode code point
            // This is a simplified example; proper UTF-8 to code point conversion is needed
            if (event.text.text[0] != '\0') {
                codePoint = static_cast<unsigned int>(event.text.text[0]);
            }
            CharCallback(window, codePoint);
        } break;

        case SDL_TEXTEDITING: {
            // Handle text editing if necessary
            // Typically used for input method editors (IMEs)
        } break;

            // Handle other events as needed

        default:
            break;
        }
    }
}
uint32_t GetMonitorWidth_SDL(){
    Initialize_SDL();
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0){
         TRACELOG(LOG_ERROR, "SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
         return 1;
    }
    return dm.w;
}
uint32_t GetMonitorHeight_SDL(){
    Initialize_SDL();
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0){
         TRACELOG(LOG_ERROR, "SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
         return 1;
    }
    return dm.h;
}