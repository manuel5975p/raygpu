#define VK_NO_PROTOTYPES
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_error.h>
#include <format>
#include <raygpu.h>
#include <internals.hpp>
#include <renderstate.hpp>
#if SUPPORT_VULKAN_BACKEND == 1
#include <external/volk.h>
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#include "backend_vulkan/vulkan_internals.hpp"
#elif SUPPORT_WGPU_BACKEND == 1 || defined(__EMSCRIPTEN__)
#include "sdl3webgpu.h"
#endif

constexpr uint32_t max_format_count = 16;

typedef struct SurfaceAndSwapchainSupport{
    PixelFormat supportedFormats[max_format_count];
    uint32_t supportedFormatCount;
    PresentMode supportedPresentModes[4];
    uint32_t supportedPresentModeCount;

    uint32_t presentQueueIndex; // Vulkan only
}SurfaceAndSwapchainSupport;

uint32_t GetPresentQueueIndex(void* instanceHandle, void* adapterHandle){
    #if SUPPORT_VULKAN_BACKEND == 1
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, queueFamilies.data());
    for(uint32_t index = 0;index < queueFamilyCount;index++){
        bool ps = SDL_Vulkan_GetPresentationSupport((VkInstance)instanceHandle, (VkPhysicalDevice)adapterHandle, index);
        if(ps){
            return index;
        }
    }
    #endif
    return ~0;
}
bool alreadyInited = false;
void Initialize_SDL3(){
    if(!alreadyInited){
        #ifdef __EMSCRIPTEN__
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
        #endif
        alreadyInited = true;
        SDL_InitFlags initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
        SDL_Init(initFlags);
    }
}

extern "C" void* CreateSurfaceForWindow_SDL3(void* windowHandle){
    #if SUPPORT_VULKAN_BACKEND == 1
    WGVKSurface retp = callocnew(WGVKSurfaceImpl);
    SDL_Vulkan_CreateSurface((SDL_Window*)windowHandle, g_vulkanstate.instance->instance, nullptr, &retp->surface);
    return retp;
    #else
    return SDL3_GetWGPUSurface((WGPUInstance)GetInstance(), (SDL_Window*)windowHandle);
    #endif
}
SubWindow OpenSubWindow_SDL3(uint32_t width, uint32_t height, const char* title){
    SubWindow ret zeroinit;
    ret.type = windowType_sdl3;
    //WGPUInstance inst = (WGPUInstance)GetInstance();
    //WGPUSurfaceCapabilities capabilities zeroinit;
    SDL_WindowFlags windowFlags = 0;
    #if SUPPORT_VULKAN_BACKEND == 1
    windowFlags |= SDL_WINDOW_VULKAN;
    #endif
    ret.handle = SDL_CreateWindow(title, width, height, windowFlags);
    #if SUPPORT_VULKAN_BACKEND == 1
    //WGVKSurface vSurface = callocnew(WGVKSurfaceImpl);
    //bool surfaceCreated = SDL_Vulkan_CreateSurface((SDL_Window*)ret.handle, g_vulkanstate.instance, nullptr, &vSurface->surface);
    //SurfaceConfiguration config{};
    //config.width = width;
    //config.height = height;
    //config.format = BGRA8;
    //config.presentMode = PresentMode_Fifo;
    //config.device = g_vulkanstate.device;
    //wgvkSurfaceConfigure(vSurface, &config);
    //FullSurface fsurface zeroinit;
    //fsurface.surfaceConfig = config;
    //fsurface.surface = vSurface;
    //fsurface.renderTarget.depth = LoadDepthTexture(width, height);
    //ret.surface = fsurface;
    //g_renderstate.createdSubwindows[ret.handle] = ret;
    //#else
    //WGPUSurface surface = SDL3_GetWGPUSurface(inst, (SDL_Window*)ret.handle);
    //
    //ret.surface = CreateSurface(surface, width, height);
    #endif
    g_renderstate.createdSubwindows[ret.handle] = ret;
    g_renderstate.input_map[(GLFWwindow*)ret.handle];
    //setup((GLFWwindow*)ret.handle);
    return ret;
}


extern "C" SubWindow InitWindow_SDL3(uint32_t width, uint32_t height, const char *title) {
    Initialize_SDL3();
    TRACELOG(LOG_INFO, "INITED SDL3!!!!!!!!!!!!!!!!!");
    SubWindow ret{};
    ret.type = windowType_sdl3;
    //SDL_SetHint(SDL_HINT_TRACKPAD_IS_TOUCH_ONLY, "1");
    SDL_WindowFlags windowFlags zeroinit;
    #if SUPPORT_VULKAN_BACKEND == 1 && !defined(__EMSCRIPTEN__)
    windowFlags |= SDL_WINDOW_VULKAN;
    #endif
    SDL_Window *window = SDL_CreateWindow(title, width, height, windowFlags);
    SDL_SetWindowResizable(window, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE));
    if(g_renderstate.windowFlags & FLAG_FULLSCREEN_MODE)
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    
    ret.handle = window;

    g_renderstate.createdSubwindows[ret.handle] = ret;
    g_renderstate.window = (GLFWwindow*)ret.handle;
    g_renderstate.mainWindow = &g_renderstate.createdSubwindows[ret.handle];
    SDL_StartTextInput((SDL_Window*)ret.handle);
    return ret;
}

SurfaceAndSwapchainSupport QuerySurfaceAndSwapchainSupport(void* instanceHandle, void* adapterHandle, void* surfaceHandle){
    SurfaceAndSwapchainSupport ret zeroinit;

    if(surfaceHandle == 0){
        return ret;
    }
    
    #if SUPPORT_VULKAN_BACKEND == 1
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    VkSurfaceKHR surface = (VkSurfaceKHR)surfaceHandle; 
    
    ret.presentQueueIndex = GetPresentQueueIndex(instanceHandle, adapterHandle);
    VkSurfaceCapabilitiesKHR capabilities zeroinit;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &capabilities);
    uint32_t formatCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, nullptr);
    
    if(formatCount != 0) {
        ret.supportedFormatCount = formatCount;
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, formats.data());
        for(uint32_t i = 0;i < formatCount;i++){
            ret.supportedFormats[i] = fromVulkanPixelFormat(formats[i].format);
        }
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, presentModes.data());
    
    #endif
    return ret;
}


void negotiateSurfaceFormatAndPresentMode(void* nsurface){
    //TODO
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

uint32_t GetMonitorWidth_SDL3(cwoid){
    Initialize_SDL3();
    int displayCount = 0;
    SDL_GetDisplays(&displayCount);
    SDL_Point zeropoint{};
    SDL_DisplayID id = SDL_GetDisplayForPoint(&zeropoint);
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(id);
    if(mode == nullptr){
        const char* errormessage = SDL_GetError();
        TRACELOG(LOG_WARNING, errormessage);
    }
    return mode->w;
}

uint32_t GetMonitorHeight_SDL3(cwoid){
    Initialize_SDL3();
    SDL_Point zeropoint{};
    SDL_DisplayID id = SDL_GetDisplayForPoint(&zeropoint);
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(id);
    return mode->h;
}

void ResizeCallback(SDL_Window* window, int width, int height){

    //TRACELOG(LOG_INFO, "SDL3's ResizeCallback called with %d x %d", width, height);
    ResizeSurface(&g_renderstate.createdSubwindows[window].surface, width, height);
    if((void*)window == (void*)g_renderstate.window){
        g_renderstate.mainWindowRenderTarget = g_renderstate.createdSubwindows[window].surface.renderTarget;
    }
    Matrix newcamera = ScreenMatrix(width, height);
}

static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode){
    if ((sdlScancode >= 0) && (sdlScancode < SCANCODE_MAPPED_NUM)){
        return mapScancodeToKey[sdlScancode];
    }

    return KEY_NULL; // No equivalent key in Raylib
}

void PenAxisCallback(SDL_Window* window, SDL_PenID penID, SDL_PenAxis axis, float value){
    g_renderstate.input_map[window].penStates[penID].axes[axis] = value;
    if(axis == SDL_PEN_AXIS_PRESSURE){
        std::cout << value << std::endl;
    }
}
void PenMotionCallback(SDL_Window* window, SDL_PenID penID, float x, float y){
    g_renderstate.input_map[window].penStates[penID].position = Vector2{x,y };
}
void FingerMotionCallback(SDL_Window* window, SDL_FingerID finger, float x, float y){
    
    std::cout << std::format("Finger {}: {},{}", finger, x, y) << std::endl;
}
void MouseButtonCallback(SDL_Window* window, int button, int action){
    if(action == 1){
        g_renderstate.input_map[window].mouseButtonDown[button] = 1;
    }
    else if(action == 0){
        g_renderstate.input_map[window].mouseButtonDown[button] = 0;
    }
}
void MousePositionCallback(SDL_Window* window, double x, double y){
    g_renderstate.input_map[window].mousePos = Vector2{float(x), float(y)};
}

void ScrollCallback(SDL_Window* window, double xoffset, double yoffset){
    g_renderstate.input_map[window].scrollThisFrame.x += xoffset;
    g_renderstate.input_map[window].scrollThisFrame.y += yoffset;
}

void KeyUpCallback (SDL_Window* window, int key, int scancode, int mods){
    g_renderstate.input_map[window].keydown[key] = 0;
}
void KeyDownCallback (SDL_Window* window, int key, int scancode, int mods){
    g_renderstate.input_map[window].keydown[key] = 1;
    //if(action == GLFW_PRESS){
    //    g_renderstate.input_map[window].keydown[key] = 1;
    //}else if(action == GLFW_RELEASE){
    //    g_renderstate.input_map[window].keydown[key] = 0;
    //}
    //if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
    //    EndGIFRecording();
    //    glfwSetWindowShouldClose(window, true);
    //}
}
extern "C" void PollEvents_SDL3() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:{
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            g_renderstate.closeFlag = true;
        }break;
        case SDL_EVENT_KEY_DOWN:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            KeyDownCallback(window, ConvertScancodeToKey(event.key.scancode), event.key.scancode, event.key.mod);
            if(event.key.scancode == SDL_SCANCODE_ESCAPE){
                g_renderstate.closeFlag = true;
            }
        }break;
        case SDL_EVENT_KEY_UP:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            KeyUpCallback(window, ConvertScancodeToKey(event.key.scancode), event.key.scancode, event.key.mod);
        }break;
        case SDL_EVENT_WINDOW_RESIZED: {
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            int newWidth = event.window.data1;
            int newHeight = event.window.data2;
            ResizeCallback(window, newWidth, newHeight);
            //if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            //    int newWidth = event.window.data1;
            //    int newHeight = event.window.data2;
            //    ResizeCallback(window, newWidth, newHeight);
            //} else if (event.window.event == SDL_WINDOWEVENT_ENTER) {
            //    CursorEnterCallback(window, true);
            //} else if (event.window.event == SDL_WINDOWEVENT_LEAVE) {
            //    CursorEnterCallback(window, false);
            //} else if(event.window.event == SDL_WINDOWEVENT_CLOSE){
            //    g_renderstate.closeFlag = true;
            //}
            // Handle other window events if necessary
        } break;
        //case SDL_MULTIGESTURE:{
        //    //SDL_Window *window = SDL_GetWindowFromID(event.tfinger.windowID);
        //    if(lastTouched)
        //        GestureCallback(lastTouched, event.mgesture.dDist, event.mgesture.dTheta);
        //}break;
        //case SDL_FINGERDOWN:{
        //    lastTouched = SDL_GetWindowFromID(event.tfinger.windowID);
        //    int w, h;
        //    SDL_GetWindowSize(lastTouched, &w, &h);
        //    FingerDownCallback(lastTouched, event.tfinger.fingerId, event.tfinger.x * w, event.tfinger.y * h);
        //}break;
        //case SDL_FINGERUP:{
        //    lastTouched = SDL_GetWindowFromID(event.tfinger.windowID);
        //    int w, h;
        //    SDL_GetWindowSize(lastTouched, &w, &h);
        //    FingerUpCallback(lastTouched, event.tfinger.fingerId, event.tfinger.x * w, event.tfinger.y * h);
        //}break;
        case SDL_EVENT_FINGER_MOTION:{
            SDL_Window* lastTouched = SDL_GetWindowFromID(event.tfinger.windowID);
            int w, h;
            SDL_GetWindowSize(lastTouched, &w, &h);
            FingerMotionCallback(lastTouched, event.tfinger.fingerID, event.tfinger.x * w, event.tfinger.y * h);
        }break;
        case SDL_EVENT_MOUSE_WHEEL: {
            SDL_Window *window = SDL_GetWindowFromID(event.wheel.windowID);
            // Note: SDL's yoffset is positive when scrolling up, negative when scrolling down
            ScrollCallback(window, event.wheel.x, event.wheel.y);
        } break;
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Window *window = SDL_GetWindowFromID(event.motion.windowID);
            MousePositionCallback(window, event.motion.x, event.motion.y);
        } break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Window *window = SDL_GetWindowFromID(event.button.windowID);
            Uint8 state = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;

            int btn = event.button.button - 1;
            if (btn == 2) btn = 1;
            else if (btn == 1) btn = 2;

            MouseButtonCallback(window, btn, state);
        }
        break;
        case SDL_EVENT_PEN_AXIS:{
            SDL_Window *window = SDL_GetWindowFromID(event.paxis.windowID);
            PenAxisCallback(window, event.paxis.which, event.paxis.axis, event.paxis.value);
        }break;
        case SDL_EVENT_PEN_MOTION:{
            SDL_Window *window = SDL_GetWindowFromID(event.pmotion.windowID);
            PenMotionCallback(window, event.pmotion.which, event.pmotion.x, event.pmotion.y);
            //std::cout << event.pmotion.x << "\n";
        }break;
//
        //    uint8_t forGLFW = event.button.button - 1;
        //    if(forGLFW == 2) forGLFW = 1;
        //    else if(forGLFW == 1) forGLFW = 2;
        //    MouseButtonCallback(window, forGLFW, state);
        //} break;
//
        case SDL_EVENT_TEXT_INPUT: {
            SDL_Window *window = SDL_GetWindowFromID(event.text.windowID);
            int cpsize = 0;
            unsigned int codePoint = (unsigned int)GetCodepoint(event.text.text, &cpsize);
            CharCallback(window, codePoint);
        } break;
        case SDL_EVENT_TEXT_EDITING:{
        }break;
        //case SDL_TEXTEDITING: {
        //    // Handle text editing if necessary
        //    // Typically used for input method editors (IMEs)
        //} break;

            // Handle other events as needed

        default:
            break;
        }
    }
}
void ToggleFullscreen_SDL3(cwoid){
    bool alreadyFullscreen = SDL_GetWindowFlags((SDL_Window*)g_renderstate.window) & SDL_WINDOW_FULLSCREEN;
    if(alreadyFullscreen){
        //We need to exit fullscreen
        g_renderstate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        //SDL_SetWindowResizable((SDL_Window*)g_renderstate.window, SDL_FALSE);
        SDL_SetWindowFullscreen((SDL_Window*)g_renderstate.window, 0);

        SDL_SetWindowSize((SDL_Window*)g_renderstate.window, g_renderstate.input_map[g_renderstate.window].windowPosition.width, g_renderstate.input_map[g_renderstate.window].windowPosition.height);
        //SDL_SetWindowSize((SDL_Window*)g_renderstate.window, g_renderstate.input_map[g_renderstate.window].windowPosition.width, g_renderstate.input_map[g_renderstate.window].windowPosition.height);
        //SDL_SetWindowSize((SDL_Window*)g_renderstate.window, g_renderstate.input_map[g_renderstate.window].windowPosition.width, g_renderstate.input_map[g_renderstate.window].windowPosition.height);
        //SDL_SetWindowSize((SDL_Window*)g_renderstate.window, g_renderstate.input_map[g_renderstate.window].windowPosition.width, g_renderstate.input_map[g_renderstate.window].windowPosition.height);
        //TRACELOG(LOG_WARNING, "Setting the size to  %d x %d", (int)g_renderstate.input_map[g_renderstate.window].windowPosition.width, (int)g_renderstate.input_map[g_renderstate.window].windowPosition.height);
    }
    else{
        //We need to enter fullscreen
        int xpos = 0, ypos = 0;
        int xs, ys;
        #ifndef DAWN_USE_WAYLAND
        SDL_GetWindowPosition((SDL_Window*)g_renderstate.window, &xpos, &ypos);
        #endif
        SDL_GetWindowSize((SDL_Window*)g_renderstate.window, &xs, &ys);
        g_renderstate.input_map[g_renderstate.window].windowPosition = Rectangle{float(xpos), float(ypos), float(xs), float(ys)};
        SDL_SetWindowSize((SDL_Window*)g_renderstate.window, GetMonitorWidth_SDL3(), GetMonitorHeight_SDL3());
        SDL_SetWindowFullscreen((SDL_Window*)g_renderstate.window, SDL_WINDOW_FULLSCREEN);
        //int monitorCount = 0;
        //int monitorIndex = GetCurrentMonitor_GLFW(g_renderstate.window);
        //GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
        //// Use current monitor, so we correctly get the display the window is on
        //GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;
        //auto vm = glfwGetVideoMode(monitor);
        //glfwSetWindowMonitor(g_renderstate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    }
}

