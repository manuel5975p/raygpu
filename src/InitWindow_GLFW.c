// begin file src/InitWindow_GLFW.cpp
#include <raygpu.h>
#define GLFW_INCLUDE_NONE
#if SUPPORT_VULKAN_BACKEND == 1
    #include <external/volk.h>
    #include <renderstate.hpp>
#endif
#if SUPPORT_WGPU_BACKEND == 1
    #include "wgpustate.inc"
#endif
#include <internals.hpp>
#include <GLFW/glfw3.h>
#include "glfw3webgpu.h"

int emscripten_to_glfw_key(const char* emsc);

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
// Configurable scaling factors
const float PIXEL_SCALE = 1.0f;
const float LINE_SCALE = 20.0f;  // Adjust based on typical line height
const float PAGE_SCALE = 800.0f; // Adjust based on typical page height
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
#if SUPPORT_VULKAN_BACKEND == 1
#include <wgvk_structs_impl.h>
#endif
void setupGLFWCallbacks(GLFWwindow* window);
void ResizeCallback(GLFWwindow* window, int width, int height){
    
    TRACELOG(LOG_INFO, "GLFW's ResizeCallback called with %d x %d", width, height);

    if (width == 0 || height == 0) {
        g_renderstate.minimized = true;
        return;
    }
    else {
        ResizeSurface(&CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->surface, width, height);
        if((void*)window == (void*)g_renderstate.window){
            g_renderstate.mainWindowRenderTarget = CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->surface.renderTarget;
        }
        Matrix newcamera = ScreenMatrix(width, height);
        g_renderstate.minimized = false;
    }
}
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.scrollThisFrame.x += (float)xoffset;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.scrollThisFrame.y += (float)yoffset;
}

#ifdef __EMSCRIPTEN__


EM_BOOL EmscriptenWheelCallback(int eventType, const EmscriptenWheelEvent* wheelEvent, void *userData) {
    // Calculate scaling based on deltaMode
    float scaleX = calculateScrollScale(wheelEvent->deltaMode);
    float scaleY = calculateScrollScale(wheelEvent->deltaMode);
    
    // Optionally clamp the delta values to prevent excessive scrolling
    double deltaX = std_clamp_f64(wheelEvent->deltaX * scaleX, -100.0, 100.0) / 100.0f;
    double deltaY = std_clamp_f64(wheelEvent->deltaY * scaleY, -100.0, 100.0) / 100.0f;
    
    
    // Invoke the original scroll callback with scaled deltas
    //auto originalCallback = reinterpret_cast<decltype(scrollCallback)*>(userData);
    ScrollCallback(NULL, deltaX, deltaY);
    
    return EM_TRUE; // Indicate that the event was handled
};

#endif

void cpcallback(GLFWwindow* window, double x, double y){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mousePos = CLITERAL(Vector2){(float)x, (float)y};
}

#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenMouseCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    cpcallback(NULL, mouseEvent->targetX, mouseEvent->targetY);
    return true;
};
#endif
void clickcallback(GLFWwindow* window, int button, int action, int mods){
    if(action == GLFW_PRESS){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mouseButtonDown[button] = 1;
    }
    else if(action == GLFW_RELEASE){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mouseButtonDown[button] = 0;
    }
}
#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenMousedownClickCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    clickcallback(NULL, mouseEvent->button, GLFW_PRESS, 0);
    return true;
};
EM_BOOL EmscriptenMouseupClickCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
    clickcallback(NULL, mouseEvent->button, GLFW_RELEASE, 0);
    return true;
};
#endif

#ifndef __EMSCRIPTEN__

#else
#endif


//#ifndef __EMSCRIPTEN__
void CursorEnterCallback(GLFWwindow* window, int entered){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.cursorInWindow = entered;
}
//#endif

void glfwKeyCallback (GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.keydown[key] = 1;
    }else if(action == GLFW_RELEASE){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.keydown[key] = 0;
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
    int key = emscripten_to_glfw_key(keyEvent->code);
    if(key != GLFW_KEY_UNKNOWN){
        glfwKeyCallback(g_renderstate.window, key, key, GLFW_PRESS, modifier);
    }
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
    int key = emscripten_to_glfw_key(keyEvent->code);
    if(key != GLFW_KEY_UNKNOWN){
        glfwKeyCallback(g_renderstate.window, key, key, GLFW_RELEASE, modifier);
    }
    return 1;
}
#endif// __EMSCRIPTEN__
static void CharCallback_glfw_tp(GLFWwindow* w, unsigned int cp){
    CharCallback((void*)w, cp);
}
void setupGLFWCallbacks(GLFWwindow* window){
    glfwSetWindowSizeCallback(window, ResizeCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);
    glfwSetCursorPosCallback(window, cpcallback);
    glfwSetCharCallback(window, CharCallback_glfw_tp);
    glfwSetCursorEnterCallback(window, CursorEnterCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, clickcallback);
    #ifdef __EMSCRIPTEN__
    emscripten_set_mousedown_callback("#canvas", NULL, 1, EmscriptenMousedownClickCallback);
    emscripten_set_mouseup_callback("#canvas",   NULL, 1, EmscriptenMouseupClickCallback);
    emscripten_set_mousemove_callback("#canvas", NULL, 1, EmscriptenMouseCallback);
    emscripten_set_wheel_callback("#canvas",     NULL, 1, EmscriptenWheelCallback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenKeydownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, EmscriptenKeyupCallback);
    #endif
}

int GetMonitorWidth_GLFW(cwoid){
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == NULL){
        return 0;
    }
    return mode->width;
}
int GetMonitorHeight_GLFW(cwoid){
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == NULL){
        return 0;
    }
    return mode->height;
}
void ShowCursor_GLFW(GLFWwindow* window){
    glfwSetInputMode(g_renderstate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
void DisableCursorGLFW(GLFWwindow* window){
    #if !defined(__EMSCRIPTEN__) && !defined(DAWN_USE_WAYLAND) && defined(GLFW_CURSOR_CAPTURED)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    #endif
}
void PollEvents_SDL(cwoid);

void PollEvents_GLFW(cwoid){
    glfwPollEvents();
}
int GetCurrentMonitor_GLFW(GLFWwindow* window){
    int index = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount >= 1)
    {
        if (glfwGetWindowMonitor(window) != NULL){
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
        if (g_renderstate.windowFlags & FLAG_FULLSCREEN_MODE) enterFullscreen = false;
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
        g_renderstate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
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
        g_renderstate.windowFlags |= FLAG_FULLSCREEN_MODE;
    }
    //TRACELOG(LOG_DEBUG, "Tagu fullscreen");
    #else //Other than emscripten
    GLFWmonitor* monitor = glfwGetWindowMonitor(g_renderstate.window);
    if(monitor){
        //We need to exit fullscreen
        g_renderstate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        glfwSetWindowMonitor(
            g_renderstate.window,
            NULL,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.x,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.y,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.width,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.height, GLFW_DONT_CARE);
    }
    else{
        //We need to enter fullscreen
        int xpos = 0, ypos = 0;
        int xs, ys;
        #ifndef __EMSCRIPTEN__
        if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND)
            glfwGetWindowPos(g_renderstate.window, &xpos, &ypos);
        #endif
        glfwGetWindowSize(g_renderstate.window, &xs, &ys);
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition = CLITERAL(Rectangle){(float)xpos, (float)ypos, (float)xs, (float)ys};
        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor_GLFW(g_renderstate.window);
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        // Use current monitor, so we correctly get the display the window is on
        GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;
        const GLFWvidmode* vm = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(g_renderstate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    }

    #endif
}
WGPUSurface CreateSurfaceForWindow_GLFW(void* windowHandle){
    #if SUPPORT_VULKAN_BACKEND == 1
    WGPUSurface retp = callocnew(WGPUSurfaceImpl);
    glfwCreateWindowSurface(((WGPUInstance)GetInstance())->instance, (GLFWwindow*)windowHandle, NULL, &retp->surface);
    float xscale, yscale;
    glfwGetWindowContentScale((GLFWwindow*)windowHandle, &xscale, &yscale);
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, windowHandle)->scaleFactor = xscale;
    return retp;
    #elif defined(__EMSCRIPTEN__)
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector fromCanvasHTMLSelector = {
        .chain = {
            .sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector
        },
        .selector = (WGPUStringView){ "#canvas", WGPU_STRLEN }
    };
    WGPUSurfaceDescriptor surfaceDesc = {.nextInChain = &fromCanvasHTMLSelector.chain};
    
    WGPUInstance instance = (WGPUInstance)GetInstance();
    if(instance == NULL){
        abort();
    }
    return wgpuInstanceCreateSurface(instance, &surfaceDesc);
    #else
    float xscale, yscale;
    glfwGetWindowContentScale((GLFWwindow*)windowHandle, &xscale, &yscale);
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, windowHandle)->scaleFactor = xscale;
    WGPUSurface wsurfaceHandle = glfwCreateWindowWGPUSurface((WGPUInstance)GetInstance(), (GLFWwindow*)windowHandle);
    return wsurfaceHandle;
    #endif
}
void SetWindowShouldClose_GLFW(GLFWwindow* window){
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}
static void glfw_e_c_(int code, const char* message) {
    fprintf(stderr, "GLFW error: %d - %s", code, message);
}
SubWindow InitWindow_GLFW(int width, int height, const char* title){
    SubWindow ret = callocnew(RGWindowImpl);
    ret->scaleFactor = 1.0f;
    ret->type = windowType_glfw;
    
    void* window = NULL;
    if (!glfwInit()) {
        abort();
    }

    GLFWmonitor* mon = NULL;
    glfwSetErrorCallback(glfw_e_c_);


    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    #ifndef __EMSCRIPTEN__        // Create the test window with no client API.
        glfwWindowHint(GLFW_RESIZABLE, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE) ? GLFW_TRUE : GLFW_FALSE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 144);

        if(g_renderstate.windowFlags & FLAG_FULLSCREEN_MODE){
            mon = glfwGetPrimaryMonitor();
            //std::cout <<glfwGetVideoMode(mon)->refreshRate << std::endl;
            //abort();
        }
    #endif
    // glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    // glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_TRUE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    #endif
    window = (void*)glfwCreateWindow(width, height, title, mon, NULL);
    //if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND)
    //    glfwSetWindowPos((GLFWwindow*)window, 200, 1900);
    
    int wposx = 0, wposy = 0;
    #ifndef __EMSCRIPTEN__
    if(glfwGetPlatform() != GLFW_PLATFORM_WAYLAND)
        glfwGetWindowPos((GLFWwindow*)window, &wposx, &wposy);
    #endif
    ret->handle = window;
    CreatedWindowMap_put(&g_renderstate.createdSubwindows, window, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);

    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state = CLITERAL(window_input_state){0};
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.windowPosition = CLITERAL(Rectangle){
        (float)wposx,
        (float)wposy,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
    ret->handle = (void*)window;
    //ret.surface = GetSurface();
    //ret.surface.renderTarget = g_renderstate.mainWindowRenderTarget;
    
    CreatedWindowMap_get(&g_renderstate.createdSubwindows,ret->handle)->input_state = CLITERAL(window_input_state){0};
    setupGLFWCallbacks((GLFWwindow*)ret->handle);
    return ret;
}
SubWindow OpenSubWindow_GLFW(int width, int height, const char* title){
    SubWindow ret = (SubWindow)callocnew(RGWindowImpl);
    #ifndef __EMSCRIPTEN__
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    ret->handle = glfwCreateWindow(width, height, title, NULL, NULL);
    CreatedWindowMap_put(&g_renderstate.createdSubwindows, ret->handle, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    CreatedWindowMap_get(&g_renderstate.createdSubwindows,ret->handle)->input_state = CLITERAL(window_input_state){0};
    setupGLFWCallbacks((GLFWwindow*)ret->handle);
    #endif
    return ret;
}
bool WindowShouldClose_GLFW(GLFWwindow* win){
    return glfwWindowShouldClose(win);
}
void CloseSubWindow_GLFW(SubWindow subWindow){
    CreatedWindowMap_erase(&g_renderstate.createdSubwindows, subWindow->handle);
    glfwWindowShouldClose((GLFWwindow*)subWindow->handle);
    glfwSetWindowShouldClose((GLFWwindow*)subWindow->handle, GLFW_TRUE);
}

int emscripten_to_glfw_key(const char *key_name) {
    if (!key_name) return GLFW_KEY_UNKNOWN;

    // Alphabet: "KeyA".."KeyZ"
    if (strncmp(key_name, "Key", 3) == 0 && key_name[3] >= 'A' && key_name[3] <= 'Z' && key_name[4] == '\0') {
        return GLFW_KEY_A + (key_name[3] - 'A');
    }

    // Digits: "Digit0".."Digit9"
    if (strncmp(key_name, "Digit", 5) == 0 && key_name[5] >= '0' && key_name[5] <= '9' && key_name[6] == '\0') {
        return GLFW_KEY_0 + (key_name[5] - '0');
    }

    // Function keys: "F1".."F12"
    if (key_name[0] == 'F' && key_name[1] >= '1' && key_name[1] <= '9') {
        int fn = 0;
        if (sscanf(key_name + 1, "%d", &fn) == 1 && fn >= 1 && fn <= 12) {
            return GLFW_KEY_F1 + (fn - 1);
        }
    }

    // Arrows
    if (strcmp(key_name, "ArrowUp") == 0) return GLFW_KEY_UP;
    if (strcmp(key_name, "ArrowDown") == 0) return GLFW_KEY_DOWN;
    if (strcmp(key_name, "ArrowLeft") == 0) return GLFW_KEY_LEFT;
    if (strcmp(key_name, "ArrowRight") == 0) return GLFW_KEY_RIGHT;

    // Numpad
    if (strncmp(key_name, "Numpad", 6) == 0) {
        const char *suffix = key_name + 6;
        if (suffix[0] >= '0' && suffix[0] <= '9' && suffix[1] == '\0') {
            return GLFW_KEY_KP_0 + (suffix[0] - '0');
        }
        if (strcmp(suffix, "Decimal") == 0) return GLFW_KEY_KP_DECIMAL;
        if (strcmp(suffix, "Divide") == 0) return GLFW_KEY_KP_DIVIDE;
        if (strcmp(suffix, "Multiply") == 0) return GLFW_KEY_KP_MULTIPLY;
        if (strcmp(suffix, "Subtract") == 0) return GLFW_KEY_KP_SUBTRACT;
        if (strcmp(suffix, "Add") == 0) return GLFW_KEY_KP_ADD;
        if (strcmp(suffix, "Enter") == 0) return GLFW_KEY_KP_ENTER;
        if (strcmp(suffix, "Equal") == 0) return GLFW_KEY_KP_EQUAL;
    }

    // Control keys & symbols
    if (strcmp(key_name, "Enter") == 0) return GLFW_KEY_ENTER;
    if (strcmp(key_name, "Escape") == 0) return GLFW_KEY_ESCAPE;
    if (strcmp(key_name, "Space") == 0) return GLFW_KEY_SPACE;
    if (strcmp(key_name, "Tab") == 0) return GLFW_KEY_TAB;
    if (strcmp(key_name, "ShiftLeft") == 0) return GLFW_KEY_LEFT_SHIFT;
    if (strcmp(key_name, "ShiftRight") == 0) return GLFW_KEY_RIGHT_SHIFT;
    if (strcmp(key_name, "ControlLeft") == 0) return GLFW_KEY_LEFT_CONTROL;
    if (strcmp(key_name, "ControlRight") == 0) return GLFW_KEY_RIGHT_CONTROL;
    if (strcmp(key_name, "AltLeft") == 0) return GLFW_KEY_LEFT_ALT;
    if (strcmp(key_name, "AltRight") == 0) return GLFW_KEY_RIGHT_ALT;
    if (strcmp(key_name, "CapsLock") == 0) return GLFW_KEY_CAPS_LOCK;
    if (strcmp(key_name, "Backspace") == 0) return GLFW_KEY_BACKSPACE;
    if (strcmp(key_name, "Delete") == 0) return GLFW_KEY_DELETE;
    if (strcmp(key_name, "Insert") == 0) return GLFW_KEY_INSERT;
    if (strcmp(key_name, "Home") == 0) return GLFW_KEY_HOME;
    if (strcmp(key_name, "End") == 0) return GLFW_KEY_END;
    if (strcmp(key_name, "PageUp") == 0) return GLFW_KEY_PAGE_UP;
    if (strcmp(key_name, "PageDown") == 0) return GLFW_KEY_PAGE_DOWN;

    if (strcmp(key_name, "Backquote") == 0) return GLFW_KEY_GRAVE_ACCENT;
    if (strcmp(key_name, "Minus") == 0) return GLFW_KEY_MINUS;
    if (strcmp(key_name, "Equal") == 0) return GLFW_KEY_EQUAL;
    if (strcmp(key_name, "BracketLeft") == 0) return GLFW_KEY_LEFT_BRACKET;
    if (strcmp(key_name, "BracketRight") == 0) return GLFW_KEY_RIGHT_BRACKET;
    if (strcmp(key_name, "Backslash") == 0) return GLFW_KEY_BACKSLASH;
    if (strcmp(key_name, "Semicolon") == 0) return GLFW_KEY_SEMICOLON;
    if (strcmp(key_name, "Quote") == 0) return GLFW_KEY_APOSTROPHE;
    if (strcmp(key_name, "Comma") == 0) return GLFW_KEY_COMMA;
    if (strcmp(key_name, "Period") == 0) return GLFW_KEY_PERIOD;
    if (strcmp(key_name, "Slash") == 0) return GLFW_KEY_SLASH;

    if (strcmp(key_name, "PrintScreen") == 0) return GLFW_KEY_PRINT_SCREEN;
    if (strcmp(key_name, "ScrollLock") == 0) return GLFW_KEY_SCROLL_LOCK;
    if (strcmp(key_name, "Pause") == 0) return GLFW_KEY_PAUSE;
    if (strcmp(key_name, "ContextMenu") == 0) return GLFW_KEY_MENU;

    if (strcmp(key_name, "IntlBackslash") == 0) return GLFW_KEY_UNKNOWN;

    return GLFW_KEY_UNKNOWN;
}

// end file src/InitWindow_GLFW.cpp