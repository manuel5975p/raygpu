#include <raygpu.h>
#include <GLFW/glfw3.h>
#include <wgpustate.inc>
#include "GLFW/glfw3.h"
#include "webgpu/webgpu_glfw.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
// Configurable scaling factors
constexpr float PIXEL_SCALE = 1.0f;
constexpr float LINE_SCALE = 20.0f;  // Adjust based on typical line height
constexpr float PAGE_SCALE = 800.0f; // Adjust based on typical page height
// Function to calculate scaling based on deltaMode
float calculateScrollScale(int deltaMode) {
    switch(deltaMode) {
        case DOM_DELTA_PIXEL:
            return PIXEL_SCALE;
        case DOM_DELTA_LINE:
            return LINE_SCALE;
        case DOM_DELTA_PAGE:
            return PAGE_SCALE;
        default:
            return PIXEL_SCALE; // Fallback to pixel scale
    }
}
#endif  // 
extern wgpustate g_wgpustate;
void setupGLFWCallbacks(GLFWwindow* window);
void ResizeCallback(GLFWwindow* window, int width, int height){
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
    
    if(window == g_wgpustate.window){
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
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    g_wgpustate.input_map[window].scrollThisFrame.x += xoffset;
    g_wgpustate.input_map[window].scrollThisFrame.y += yoffset;
}


#ifdef __EMSCRIPTEN__


EM_BOOL EmscriptenWheelCallback(int eventType, const EmscriptenWheelEvent* wheelEvent, void *userData) {
    // Calculate scaling based on deltaMode
    float scaleX = calculateScrollScale(wheelEvent->deltaMode);
    float scaleY = calculateScrollScale(wheelEvent->deltaMode);
    
    // Optionally clamp the delta values to prevent excessive scrolling
    double deltaX = std::clamp(wheelEvent->deltaX * scaleX, -100.0, 100.0) / 100.0f;
    double deltaY = std::clamp(wheelEvent->deltaY * scaleY, -100.0, 100.0) / 100.0f;
    
    
    // Invoke the original scroll callback with scaled deltas
    //auto originalCallback = reinterpret_cast<decltype(scrollCallback)*>(userData);
    ScrollCallback(nullptr, deltaX, deltaY);
    
    return EM_TRUE; // Indicate that the event was handled
};

#endif

void cpcallback(GLFWwindow* window, double x, double y){
    g_wgpustate.input_map[window].mousePos = Vector2{float(x), float(y)};
}

#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    cpcallback(nullptr, mouseEvent->targetX, mouseEvent->targetY);
    return true;
};
#endif
void clickcallback(GLFWwindow* window, int button, int action, int mods){
    if(action == GLFW_PRESS){
        g_wgpustate.input_map[window].mouseButtonDown[button] = 1;
    }
    else if(action == GLFW_RELEASE){
        g_wgpustate.input_map[window].mouseButtonDown[button] = 0;
    }
}
#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenMousedownClickCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    clickcallback(nullptr, mouseEvent->button, GLFW_PRESS, 0);
    return true;
};
EM_BOOL EmscriptenMouseupClickCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    clickcallback(nullptr, mouseEvent->button, GLFW_RELEASE, 0);
    return true;
};
#endif

#ifndef __EMSCRIPTEN__

#else
#endif


//#ifndef __EMSCRIPTEN__
void CharCallback(GLFWwindow* window, unsigned int codePoint){
    g_wgpustate.input_map[window].charQueue.push_back((int)codePoint);
}
void CursorEnterCallback(GLFWwindow* window, int entered){
    g_wgpustate.input_map[window].cursorInWindow = entered;
}
//#endif
extern const std::unordered_map<std::string, int> emscriptenToGLFWKeyMap;
void glfwKeyCallback (GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        g_wgpustate.input_map[window].keydown[key] = 1;
    }else if(action == GLFW_RELEASE){
        g_wgpustate.input_map[window].keydown[key] = 0;
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        EndGIFRecording();
        
        glfwSetWindowShouldClose(window, true);
    }
}
#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenKeydownCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData){
    if(keyEvent->repeat)return 0;
    uint32_t modifier = 0;
    if(keyEvent->ctrlKey)
        modifier |= GLFW_MOD_CONTROL;
    if(keyEvent->shiftKey)
        modifier |= GLFW_MOD_SHIFT;
    if(keyEvent->altKey)
        modifier |= GLFW_MOD_ALT;
    glfwKeyCallback(g_wgpustate.window, emscriptenToGLFWKeyMap.at(keyEvent->code), emscriptenToGLFWKeyMap.at(keyEvent->code), GLFW_PRESS, modifier);
    //__builtin_dump_struct(keyEvent, printf);
    //printf("Pressed %u\n", keyEvent->which);
    return 0;
}
EM_BOOL EmscriptenKeyupCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData){
    if(keyEvent->repeat)return 1;
    uint32_t modifier = 0;
    if(keyEvent->ctrlKey)
        modifier |= GLFW_MOD_CONTROL;
    if(keyEvent->shiftKey)
        modifier |= GLFW_MOD_SHIFT;
    if(keyEvent->altKey)
        modifier |= GLFW_MOD_ALT;
    glfwKeyCallback(g_wgpustate.window, emscriptenToGLFWKeyMap.at(keyEvent->code), emscriptenToGLFWKeyMap.at(keyEvent->code), GLFW_RELEASE, modifier);
    //printf("Released %u\n", keyEvent->which);
    return 1;
}
#endif// __EMSCRIPTEN__

void setupGLFWCallbacks(GLFWwindow* window){
    glfwSetWindowSizeCallback(window, ResizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetCursorPosCallback(window, cpcallback);
    glfwSetCharCallback(window, CharCallback);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, clickcallback);
    #ifdef __EMSCRIPTEN__
    emscripten_set_mousedown_callback("#canvas", nullptr, 1, EmscriptenMousedownClickCallback);
    emscripten_set_mouseup_callback("#canvas",   nullptr, 1, EmscriptenMouseupClickCallback);
    emscripten_set_mousemove_callback("#canvas", nullptr, 1, EmscriptenMouseCallback);
    emscripten_set_wheel_callback("#canvas",     nullptr, 1, EmscriptenWheelCallback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, EmscriptenKeydownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, EmscriptenKeyupCallback);
    #endif
}

uint32_t GetMonitorWidth_GLFW(cwoid){
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        return 0;
    }
    return mode->width;
}
uint32_t GetMonitorHeight_GLFW(cwoid){
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        return 0;
    }
    return mode->height;
}
void ShowCursor_GLFW(GLFWwindow* window){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void HideCursor_GLFW(GLFWwindow* window){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
bool IsCursorHidden_GLFW(GLFWwindow* window){
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
}
void EnableCursor_GLFW(GLFWwindow* window){
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void DisableCursor(GLFWwindow* window){
    
    #if !defined(__EMSCRIPTEN__) && !defined(DAWN_USE_WAYLAND) && defined(GLFW_CURSOR_CAPTURED)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    #endif
}
extern "C" void PollEvents_SDL(cwoid);

extern "C" void PollEvents_GLFW(cwoid){
    glfwPollEvents();
}
int GetCurrentMonitor_GLFW(GLFWwindow* window){
    int index = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount >= 1)
    {
        if (glfwGetWindowMonitor(window) != nullptr){
            // Get the handle of the monitor that the specified window is in full screen on
            monitor = glfwGetWindowMonitor(window);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    index = i;
                    break;
                }
            }
        }
        else
        {
            // In case the window is between two monitors, we use below logic
            // to try to detect the "current monitor" for that window, note that
            // this is probably an overengineered solution for a very side case
            // trying to match SDL behaviour

            int closestDist = 0x7FFFFFFF;

            // Window center position
            int wcx = 0;
            int wcy = 0;
            #ifndef DAWN_USE_WAYLAND
            glfwGetWindowPos(window, &wcx, &wcy);
            #endif
            wcx += (int)GetScreenWidth()/2;
            wcy += (int)GetScreenHeight()/2;

            for (int i = 0; i < monitorCount; i++)
            {
                // Monitor top-left position
                int mx = 0;
                int my = 0;

                monitor = monitors[i];
                glfwGetMonitorPos(monitor, &mx, &my);
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                if (mode)
                {
                    const int right = mx + mode->width - 1;
                    const int bottom = my + mode->height - 1;

                    if ((wcx >= mx) &&
                        (wcx <= right) &&
                        (wcy >= my) &&
                        (wcy <= bottom))
                    {
                        index = i;
                        break;
                    }

                    int xclosest = wcx;
                    if (wcx < mx) xclosest = mx;
                    else if (wcx > right) xclosest = right;

                    int yclosest = wcy;
                    if (wcy < my) yclosest = my;
                    else if (wcy > bottom) yclosest = bottom;

                    int dx = wcx - xclosest;
                    int dy = wcy - yclosest;
                    int dist = (dx*dx) + (dy*dy);
                    if (dist < closestDist)
                    {
                        index = i;
                        closestDist = dist;
                    }
                }
                else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
            }
        }
    }

    return index;
}
void ToggleFullscreen_GLFW(){
    #ifdef __EMSCRIPTEN__
    //platform.ourFullscreen = true;

    bool enterFullscreen = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (g_wgpustate.windowFlags & FLAG_FULLSCREEN_MODE) enterFullscreen = false;
        //else if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) enterFullscreen = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(document.getElementById('canvas').style.width); }, 0);
            if (canvasStyleWidth > canvasWidth) enterFullscreen = false;
            else enterFullscreen = true;
        }

        EM_ASM(document.exitFullscreen(););

        //CORE.Window.fullscreen = false;
        g_wgpustate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        //CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
    }
    else enterFullscreen = true;

    if (enterFullscreen){
        EM_ASM(
            setTimeout(function()
            {
                Module.requestFullscreen(false, false);
            }, 100);
        );
        g_wgpustate.windowFlags |= FLAG_FULLSCREEN_MODE;
    }
    TRACELOG(LOG_DEBUG, "Tagu fullscreen");
    #else //Other than emscripten
    GLFWmonitor* monitor = glfwGetWindowMonitor(g_wgpustate.window);
    if(monitor){
        //We need to exit fullscreen
        g_wgpustate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        glfwSetWindowMonitor(g_wgpustate.window, NULL, g_wgpustate.input_map[g_wgpustate.window].windowPosition.x, g_wgpustate.input_map[g_wgpustate.window].windowPosition.y, g_wgpustate.input_map[g_wgpustate.window].windowPosition.width, g_wgpustate.input_map[g_wgpustate.window].windowPosition.height, GLFW_DONT_CARE);
    }
    else{
        //We need to enter fullscreen
        int xpos = 0, ypos = 0;
        int xs, ys;
        #ifndef DAWN_USE_WAYLAND
        glfwGetWindowPos(g_wgpustate.window, &xpos, &ypos);
        #endif
        glfwGetWindowSize(g_wgpustate.window, &xs, &ys);
        g_wgpustate.input_map[g_wgpustate.window].windowPosition = Rectangle{float(xpos), float(ypos), float(xs), float(ys)};
        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor_GLFW(g_wgpustate.window);
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        // Use current monitor, so we correctly get the display the window is on
        GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;
        auto vm = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(g_wgpustate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    }

    #endif
}
void SetWindowShouldClose_GLFW(GLFWwindow* window){
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

SubWindow InitWindow_GLFW(int width, int height, const char* title){
    SubWindow ret{};
    void* window = nullptr;
    if (!glfwInit()) {
        abort();
    }
    GLFWmonitor* mon = nullptr;

    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error: " << code << " - " << message;
    });


    #ifndef __EMSCRIPTEN__


        // Create the test window with no client API.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, (g_wgpustate.windowFlags & FLAG_WINDOW_RESIZABLE) ? GLFW_TRUE : GLFW_FALSE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 144);

        if(g_wgpustate.windowFlags & FLAG_FULLSCREEN_MODE){
            mon = glfwGetPrimaryMonitor();
            //std::cout <<glfwGetVideoMode(mon)->refreshRate << std::endl;
            //abort();
        }
    #endif
        
    #ifndef __EMSCRIPTEN__
        window = (void*)glfwCreateWindow(width, height, title, mon, nullptr);
        //glfwSetWindowPos(window, 200, 1200);
        if (!window) {
            abort();
        }
        

        // Create the surface.
        wgpu::Surface rs = wgpu::glfw::CreateSurfaceForWindow(GetInstance(), (GLFWwindow*)window);
        negotiateSurfaceFormatAndPresentMode(rs);
        ret.surface = rs.MoveToCHandle();
    #else
        // Create the surface.
        wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "#canvas";

        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &canvasDesc;
        g_wgpustate.surface = g_wgpustate.instance.CreateSurface(&surfaceDesc);
        negotiateSurfaceFormatAndPresentMode(g_wgpustate.surface);
        ret.surface = g_wgpustate.surface.Get();
        window = glfwCreateWindow(width, height, title, mon, nullptr);
        g_wgpustate.window = (GLFWwindow*)window;
    #endif
    WGPUSurfaceConfiguration config{};
    if(g_wgpustate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT){
        config.presentMode = (WGPUPresentMode)(((g_wgpustate.unthrottled_PresentMode == wgpu::PresentMode::Mailbox) ? g_wgpustate.unthrottled_PresentMode : g_wgpustate.throttled_PresentMode));
    }
    else if(g_wgpustate.windowFlags & FLAG_VSYNC_HINT){
        config.presentMode = (WGPUPresentMode)g_wgpustate.throttled_PresentMode;
    }
    else{
        config.presentMode = (WGPUPresentMode)g_wgpustate.unthrottled_PresentMode;
    }
    TRACELOG(LOG_INFO, "Initialized GLFW window with surface %s", presentModeSpellingTable.at(config.presentMode).c_str());
    config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = g_wgpustate.frameBufferFormat;
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    config.viewFormats = &config.format;
    config.viewFormatCount = 1;
    config.device = GetDevice();
    TRACELOG(LOG_INFO, "Configuring surface");
    wgpuSurfaceConfigure(ret.surface, &config);
    int wposx = 0, wposy = 0;
    #ifndef DAWN_USE_WAYLAND
    glfwGetWindowPos((GLFWwindow*)window, &wposx, &wposy);
    #endif
    g_wgpustate.input_map[window].windowPosition = Rectangle{
        (float)wposx,
        (float)wposy,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
    ret.handle = (void*)window;
    //ret.surface = GetSurface();
    ret.frameBuffer = g_wgpustate.mainWindowRenderTarget;
    g_wgpustate.createdSubwindows[window] = ret;
    g_wgpustate.input_map[ret.handle] = window_input_state{};
    setupGLFWCallbacks((GLFWwindow*)ret.handle);
    return ret;
}
extern "C" SubWindow OpenSubWindow_GLFW(uint32_t width, uint32_t height, const char* title){
    SubWindow ret{};
    #ifndef __EMSCRIPTEN__
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, (g_wgpustate.windowFlags & FLAG_WINDOW_RESIZABLE ) ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    ret.handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    WGPUInstance inst = GetInstance();
    wgpu::Surface secondSurface = wgpu::glfw::CreateSurfaceForWindow(inst, (GLFWwindow*)ret.handle);
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
    setupGLFWCallbacks((GLFWwindow*)ret.handle);
    #endif
    return ret;
}
extern "C" bool WindowShouldClose_GLFW(GLFWwindow* win){
    return glfwWindowShouldClose(win);
}
extern "C" void CloseSubWindow_GLFW(SubWindow subWindow){
    g_wgpustate.createdSubwindows.erase(subWindow.handle);
    glfwWindowShouldClose((GLFWwindow*)subWindow.handle);
    glfwSetWindowShouldClose((GLFWwindow*)subWindow.handle, GLFW_TRUE);
}

const std::unordered_map<std::string, int> emscriptenToGLFWKeyMap = {
    // Alphabet Keys
    {"KeyA", GLFW_KEY_A},
    {"KeyB", GLFW_KEY_B},
    {"KeyC", GLFW_KEY_C},
    {"KeyD", GLFW_KEY_D},
    {"KeyE", GLFW_KEY_E},
    {"KeyF", GLFW_KEY_F},
    {"KeyG", GLFW_KEY_G},
    {"KeyH", GLFW_KEY_H},
    {"KeyI", GLFW_KEY_I},
    {"KeyJ", GLFW_KEY_J},
    {"KeyK", GLFW_KEY_K},
    {"KeyL", GLFW_KEY_L},
    {"KeyM", GLFW_KEY_M},
    {"KeyN", GLFW_KEY_N},
    {"KeyO", GLFW_KEY_O},
    {"KeyP", GLFW_KEY_P},
    {"KeyQ", GLFW_KEY_Q},
    {"KeyR", GLFW_KEY_R},
    {"KeyS", GLFW_KEY_S},
    {"KeyT", GLFW_KEY_T},
    {"KeyU", GLFW_KEY_U},
    {"KeyV", GLFW_KEY_V},
    {"KeyW", GLFW_KEY_W},
    {"KeyX", GLFW_KEY_X},
    {"KeyY", GLFW_KEY_Y},
    {"KeyZ", GLFW_KEY_Z},

    // Number Keys
    {"Digit0", GLFW_KEY_0},
    {"Digit1", GLFW_KEY_1},
    {"Digit2", GLFW_KEY_2},
    {"Digit3", GLFW_KEY_3},
    {"Digit4", GLFW_KEY_4},
    {"Digit5", GLFW_KEY_5},
    {"Digit6", GLFW_KEY_6},
    {"Digit7", GLFW_KEY_7},
    {"Digit8", GLFW_KEY_8},
    {"Digit9", GLFW_KEY_9},

    // Function Keys
    {"F1", GLFW_KEY_F1},
    {"F2", GLFW_KEY_F2},
    {"F3", GLFW_KEY_F3},
    {"F4", GLFW_KEY_F4},
    {"F5", GLFW_KEY_F5},
    {"F6", GLFW_KEY_F6},
    {"F7", GLFW_KEY_F7},
    {"F8", GLFW_KEY_F8},
    {"F9", GLFW_KEY_F9},
    {"F10", GLFW_KEY_F10},
    {"F11", GLFW_KEY_F11},
    {"F12", GLFW_KEY_F12},

    // Arrow Keys
    {"ArrowUp", GLFW_KEY_UP},
    {"ArrowDown", GLFW_KEY_DOWN},
    {"ArrowLeft", GLFW_KEY_LEFT},
    {"ArrowRight", GLFW_KEY_RIGHT},

    // Control Keys
    {"Enter", GLFW_KEY_ENTER},
    {"Escape", GLFW_KEY_ESCAPE},
    {"Space", GLFW_KEY_SPACE},
    {"Tab", GLFW_KEY_TAB},
    {"ShiftLeft", GLFW_KEY_LEFT_SHIFT},
    {"ShiftRight", GLFW_KEY_RIGHT_SHIFT},
    {"ControlLeft", GLFW_KEY_LEFT_CONTROL},
    {"ControlRight", GLFW_KEY_RIGHT_CONTROL},
    {"AltLeft", GLFW_KEY_LEFT_ALT},
    {"AltRight", GLFW_KEY_RIGHT_ALT},
    {"CapsLock", GLFW_KEY_CAPS_LOCK},
    {"Backspace", GLFW_KEY_BACKSPACE},
    {"Delete", GLFW_KEY_DELETE},
    {"Insert", GLFW_KEY_INSERT},
    {"Home", GLFW_KEY_HOME},
    {"End", GLFW_KEY_END},
    {"PageUp", GLFW_KEY_PAGE_UP},
    {"PageDown", GLFW_KEY_PAGE_DOWN},

    // Numpad Keys
    {"Numpad0", GLFW_KEY_KP_0},
    {"Numpad1", GLFW_KEY_KP_1},
    {"Numpad2", GLFW_KEY_KP_2},
    {"Numpad3", GLFW_KEY_KP_3},
    {"Numpad4", GLFW_KEY_KP_4},
    {"Numpad5", GLFW_KEY_KP_5},
    {"Numpad6", GLFW_KEY_KP_6},
    {"Numpad7", GLFW_KEY_KP_7},
    {"Numpad8", GLFW_KEY_KP_8},
    {"Numpad9", GLFW_KEY_KP_9},
    {"NumpadDecimal", GLFW_KEY_KP_DECIMAL},
    {"NumpadDivide", GLFW_KEY_KP_DIVIDE},
    {"NumpadMultiply", GLFW_KEY_KP_MULTIPLY},
    {"NumpadSubtract", GLFW_KEY_KP_SUBTRACT},
    {"NumpadAdd", GLFW_KEY_KP_ADD},
    {"NumpadEnter", GLFW_KEY_KP_ENTER},
    {"NumpadEqual", GLFW_KEY_KP_EQUAL},

    // Punctuation and Symbols
    {"Backquote", GLFW_KEY_GRAVE_ACCENT},
    {"Minus", GLFW_KEY_MINUS},
    {"Equal", GLFW_KEY_EQUAL},
    {"BracketLeft", GLFW_KEY_LEFT_BRACKET},
    {"BracketRight", GLFW_KEY_RIGHT_BRACKET},
    {"Backslash", GLFW_KEY_BACKSLASH},
    {"Semicolon", GLFW_KEY_SEMICOLON},
    {"Quote", GLFW_KEY_APOSTROPHE},
    {"Comma", GLFW_KEY_COMMA},
    {"Period", GLFW_KEY_PERIOD},
    {"Slash", GLFW_KEY_SLASH},

    // Additional Keys (Add more as needed)
    {"PrintScreen", GLFW_KEY_PRINT_SCREEN},
    {"ScrollLock", GLFW_KEY_SCROLL_LOCK},
    {"Pause", GLFW_KEY_PAUSE},
    {"ContextMenu", GLFW_KEY_MENU},
    {"IntlBackslash", GLFW_KEY_UNKNOWN}, // Example of an unmapped key
    // ... add other keys as necessary
};