// begin file src/InitWindow_SDL3.c
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_video.h"
#include "macros_and_constants.h"
#define VK_NO_PROTOTYPES
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_error.h>
#include <raygpu.h>
#include "../internal_include/internals.h"
#include "../internal_include/renderstate.h"
#if SUPPORT_VULKAN_BACKEND == 1
    #include <external/volk.h>
    #include <vulkan/vulkan.h>
    #include <SDL3/SDL_vulkan.h>
    #include <wgvk_structs_impl.h>
#elif SUPPORT_WGPU_BACKEND == 1 || defined(__EMSCRIPTEN__)

#endif
#include "sdl3webgpu.h"

#define max_format_count 16

typedef struct SurfaceAndSwapchainSupport{
    PixelFormat supportedFormats[max_format_count];
    uint32_t supportedFormatCount;
    WGPUPresentMode supportedPresentModes[4];
    uint32_t supportedPresentModeCount;

    uint32_t presentQueueIndex; // Vulkan only
}SurfaceAndSwapchainSupport;

uint32_t GetPresentQueueIndex(void* instanceHandle, void* adapterHandle){
    #if SUPPORT_VULKAN_BACKEND == 1
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[64];
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, queueFamilies);
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

void Initialize_SDL3() {
    if (!alreadyInited) {
        #ifdef __EMSCRIPTEN__
        SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");
        #endif

        SDL_InitFlags initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD;
        
        // CHECK THE RETURN VALUE HERE
        if (!SDL_Init(initFlags)) {
            TRACELOG(LOG_ERROR, "SDL_Init FAILED: %s", SDL_GetError());
            // Do not set alreadyInited = true if it failed, so we can try again or exit
            return; 
        }

        alreadyInited = true;
        TRACELOG(LOG_INFO, "SDL_Init() succeeded");
    }
}
RGAPI WGPUSurface CreateSurfaceForWindow_SDL3(void* windowHandle){
    WGPUSurface surface = SDL3_GetWGPUSurface((WGPUInstance)GetInstance(), (SDL_Window*)windowHandle);
    int px, py;
    int rx, ry;
    
    SDL_GetWindowSizeInPixels((SDL_Window*)windowHandle, &px, &py);
    SDL_GetWindowSize((SDL_Window*)windowHandle, &rx, &ry);
    TRACELOG(LOG_INFO, "[SDL_Window] Pixel size: %d, %d", px, py);
    TRACELOG(LOG_INFO, "[SDL_Window] Render size: %d, %d", rx, ry);
    double scaleFactor = (double)px / rx;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, windowHandle)->scaleFactor = scaleFactor;
    
    return surface;
}

SubWindow OpenSubWindow_SDL3_NoSurface(int width, int height, const char* title){
    SubWindow ret = callocnew(RGWindowImpl);
    ret->type = windowType_sdl3;
    ret->handle = SDL_CreateWindow(title, width, height, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_SetWindowResizable((SDL_Window*)ret->handle, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE));
    int ret_width, ret_height;
    SDL_GetWindowSize(ret->handle, &ret_width, &ret_height);
    ret->width = ret_width;
    ret->height = ret_height;
    CreatedWindowMap_put(&g_renderstate.createdSubwindows, ret->handle, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    return ret;
}

SubWindow OpenSubWindow_SDL3(int width, int height, const char* title){
    SubWindow sw = OpenSubWindow_SDL3_NoSurface(width, height, title);
    CreateAndSetSurfaceForWindow(sw);
    return sw;
}

RGAPI SubWindow InitWindow_SDL3(int width, int height, const char *title) {
    const char *driverHint = NULL;

    #if RAYGPU_USE_WAYLAND == 1 && RAYGPU_USE_X11 == 1
        // Both enabled: Try Wayland first, fallback to X11
        driverHint = "wayland,x11";
    #elif RAYGPU_USE_WAYLAND == 1
        // Only Wayland enabled: Force Wayland (will fail if not available)
        driverHint = "wayland";
    #elif RAYGPU_USE_X11 == 1
        // Only X11 enabled: Force X11
        driverHint = "x11";
    #endif

    if (driverHint) {
        if (!SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, driverHint, SDL_HINT_OVERRIDE)) {
            TRACELOG(LOG_DEBUG, "Failed to set video driver hint to: %s", driverHint);
        } else {
            TRACELOG(LOG_DEBUG, "Successfully set video driver hint to: %s", driverHint);
        }
    }
    Initialize_SDL3();
    TRACELOG(LOG_INFO, "SDL Successfully inited. Some info:");
    int numDrivers = SDL_GetNumVideoDrivers();
    for (int i = 0; i < numDrivers; i++) {
        TRACELOG(LOG_INFO, "  Video driver %d: %s", i, SDL_GetVideoDriver(i));
    }
    TRACELOG(LOG_INFO, "  Current video driver: %s", SDL_GetCurrentVideoDriver());
    SubWindow ret = callocnew(RGWindowImpl);
    ret->scaleFactor = 1.0;
    ret->type = windowType_sdl3;
    SDL_WindowFlags windowFlags = 0;
    
    windowFlags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window *window = SDL_CreateWindow(title, width, height, windowFlags);
    
    rassert(window != NULL, "SDL_CreateWindow returned NULL");
    SDL_SetWindowResizable(window, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE));
    if(g_renderstate.windowFlags & FLAG_FULLSCREEN_MODE){
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    
    ret->handle = window;

    CreatedWindowMap_put(&g_renderstate.createdSubwindows, ret->handle, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    g_renderstate.window = (GLFWwindow*)ret->handle;
    g_renderstate.mainWindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    SDL_StartTextInput((SDL_Window*)ret->handle);
    return ret;
}

SurfaceAndSwapchainSupport QuerySurfaceAndSwapchainSupport(void* instanceHandle, void* adapterHandle, void* surfaceHandle){
    SurfaceAndSwapchainSupport ret = {0};

    if(surfaceHandle == 0){
        return ret;
    }
    
    #if SUPPORT_VULKAN_BACKEND == 1
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    VkSurfaceKHR surface = (VkSurfaceKHR)surfaceHandle; 
    
    ret.presentQueueIndex = GetPresentQueueIndex(instanceHandle, adapterHandle);
    VkSurfaceCapabilitiesKHR capabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &capabilities);
    uint32_t formatCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, NULL);
    
    if(formatCount != 0) {
        ret.supportedFormatCount = formatCount;
        VkSurfaceFormatKHR formats[128] = {0};
        vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, formats);
        for(uint32_t i = 0;i < formatCount;i++){
            ret.supportedFormats[i] = fromWGPUPixelFormat(fromVulkanPixelFormat(formats[i].format));
        }
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, NULL);
    VkPresentModeKHR presentModes[32];
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, presentModes);
    
    #endif
    return ret;
}


//void negotiateSurfaceFormatAndPresentMode(void* nsurface){
// TODO
//}
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

int GetMonitorWidth_SDL3(cwoid){
    Initialize_SDL3();
    int displayCount = 0;
    SDL_GetDisplays(&displayCount);
    SDL_Point zeropoint = {0};
    SDL_DisplayID id = SDL_GetDisplayForPoint(&zeropoint);
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(id);
    if(mode == NULL){
        const char* errormessage = SDL_GetError();
        TRACELOG(LOG_WARNING, errormessage);
    }
    return mode->w;
}

int GetMonitorHeight_SDL3(cwoid){
    Initialize_SDL3();
    SDL_Point zeropoint = {0};
    SDL_DisplayID id = SDL_GetDisplayForPoint(&zeropoint);
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(id);
    return mode->h;
}

void ResizeCallback_SDL3(SDL_Window* window, int width, int height){
    //TRACELOG(LOG_INFO, "SDL3's ResizeCallback called with %d x %d", width, height);
    RGWindowImpl* rgwindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, window);
    if(rgwindow){
        rgwindow->width = width;
        rgwindow->height = height;
        ResizeSurface(&rgwindow->surface, (int)(width * rgwindow->scaleFactor), (int)(height * rgwindow->scaleFactor));
        if((void*)window == (void*)g_renderstate.window){
            g_renderstate.mainWindowRenderTarget = rgwindow->surface.renderTarget;
        }
        Matrix newcamera = ScreenMatrix((int)(width * rgwindow->scaleFactor), (int)(height * rgwindow->scaleFactor));
    }
}

static KeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode){
    if ((sdlScancode >= 0) && (sdlScancode < SCANCODE_MAPPED_NUM)){
        return mapScancodeToKey[sdlScancode];
    }

    return KEY_NULL; // No equivalent key in Raylib
}

void PenAxisCallback(SDL_Window* window, SDL_PenID penID, SDL_PenAxis axis, float value){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.penStates[penID].value.axes[axis] = value;
}
void PenMotionCallback(SDL_Window* window, SDL_PenID penID, float x, float y){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.penStates[penID].value.position = CLITERAL(Vector2){x,y };
}
void FingerMotionCallback(SDL_Window* window, SDL_FingerID finger, float x, float y){
    window_input_state* ips = &CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state;
    if(ips->touchPointsCount < TOUCH_MAX - 1){
        TouchPoint insrt = {
            .pos = {x, y},
            .id = (int64_t)finger
        };
        ips->touchPoints[ips->touchPointsCount++] = insrt;
    }
}
void MouseButtonCallback(SDL_Window* window, int button, int action){
    if(action == 1){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mouseButtonDown[button] = 1;
    }
    else if(action == 0){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mouseButtonDown[button] = 0;
    }
}
void MousePositionCallback_SDL3(SDL_Window* window, double x, double y){
    double scale = CreatedWindowMap_get(&g_renderstate.createdSubwindows,window)->scaleFactor;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.mousePos = CLITERAL(Vector2){(float)(x * scale), (float)(y * scale)};
}

void ScrollCallback_SDL3(SDL_Window* window, double xoffset, double yoffset){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.scrollThisFrame.x += (float)xoffset;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.scrollThisFrame.y += (float)yoffset;
}

void KeyUpCallback_SDL3(SDL_Window* window, int key, int scancode, int mods){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.keydown[key] = 0;
}
void KeyDownCallback_SDL3(SDL_Window* window, int key, int scancode, int mods){
    if(window){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.keydown[key] = 1;
    } else {
        TRACELOG(LOG_WARNING, "keyevent received on stray window");
    }
}
RGAPI void PollEvents_SDL3() {
    SDL_Event event = {0};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:{
        // TODO: Something IDK
        }break;
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:{
            // TODO: Implement this properly for subwindows
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            RGWindowImpl* rgWindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, window);
            rgWindow->closeRequestedFlag = true;
        }break;
        case SDL_EVENT_KEY_DOWN:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            if(window){
                RGWindowImpl* rgWindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, window);
                KeyDownCallback_SDL3(window, ConvertScancodeToKey(event.key.scancode), event.key.scancode, event.key.mod);
                if(event.key.scancode == SDL_SCANCODE_ESCAPE){
                    rgWindow->closeRequestedFlag= true;
                }
            }
            else{
                TRACELOG(LOG_WARNING, "SDL3 Keydown event on nullptr window");
            }
        }break;
        case SDL_EVENT_KEY_UP:{
            SDL_Window *window = SDL_GetWindowFromID(event.key.windowID);
            if(window){
                KeyUpCallback_SDL3(window, ConvertScancodeToKey(event.key.scancode), event.key.scancode, event.key.mod);
            }
            else{
                TRACELOG(LOG_WARNING, "SDL3 Keydown event on nullptr window");
            }
        }break;
        case SDL_EVENT_WINDOW_RESIZED: {
            SDL_Window *window = SDL_GetWindowFromID(event.window.windowID);
            int newWidth = event.window.data1;
            int newHeight = event.window.data2;
            ResizeCallback_SDL3(window, newWidth, newHeight);
        }break;
        case SDL_EVENT_FINGER_MOTION:{
            SDL_Window* lastTouched = SDL_GetWindowFromID(event.tfinger.windowID);
            int w, h;
            SDL_GetWindowSize(lastTouched, &w, &h);
            FingerMotionCallback(lastTouched, event.tfinger.fingerID, event.tfinger.x * (float)w, event.tfinger.y * (float)h);
        }break;
        case SDL_EVENT_MOUSE_WHEEL: {
            SDL_Window *window = SDL_GetWindowFromID(event.wheel.windowID);
            // Note: SDL's yoffset is positive when scrolling up, negative when scrolling down
            ScrollCallback_SDL3(window, event.wheel.x, event.wheel.y);
        }break;
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Window* window = SDL_GetWindowFromID(event.motion.windowID);
            RGWindowImpl* rgwindow = CreatedWindowMap_get(&g_renderstate.createdSubwindows, window);
            if (SDL_GetWindowRelativeMouseMode(window)) {
                rgwindow->input_state.mousePos.x += event.motion.xrel;
                rgwindow->input_state.mousePos.y += event.motion.yrel;
            } else {
                double scale = rgwindow->scaleFactor;
                MousePositionCallback_SDL3(window, event.motion.x, event.motion.y);
            }
        }break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: // [[fallthrough]];
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            SDL_Window *window = SDL_GetWindowFromID(event.button.windowID);
            Uint8 state = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? 1 : 0;

            int btn = event.button.button - 1;
            if (btn == 2) btn = 1;
            else if (btn == 1) btn = 2;

            MouseButtonCallback(window, btn, state);
        }break;
        case SDL_EVENT_PEN_AXIS:{
            SDL_Window *window = SDL_GetWindowFromID(event.paxis.windowID);
            PenAxisCallback(window, event.paxis.which, event.paxis.axis, event.paxis.value);
        }break;
        case SDL_EVENT_PEN_MOTION:{
            SDL_Window *window = SDL_GetWindowFromID(event.pmotion.windowID);
            PenMotionCallback(window, event.pmotion.which, event.pmotion.x, event.pmotion.y);
            //std::cout << event.pmotion.x << "\n";
        }break;
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
        case SDL_EVENT_WINDOW_MOUSE_ENTER: {
            SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
            CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.cursorInWindow = true;
        } break;
        
        case SDL_EVENT_WINDOW_MOUSE_LEAVE: {
            SDL_Window* window = SDL_GetWindowFromID(event.window.windowID);
            CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.cursorInWindow = false;
        } break;
        case SDL_EVENT_JOYSTICK_ADDED:{
        }break;
        case SDL_EVENT_GAMEPAD_ADDED:{
            if(g_renderstate.mainWindow){
                window_input_state* input = &g_renderstate.mainWindow->input_state;
                for(int i = 0; i < GAMEPAD_MAX; i++) {
                    if(!input->gamepads[i].connected) {
                        SDL_Gamepad* gp = SDL_OpenGamepad(event.gdevice.which);
                        if(gp) {
                            input->gamepads[i].handle = gp;
                            input->gamepads[i].connected = true;
                            memset(input->gamepads[i].axis, 0, sizeof(input->gamepads[i].axis));
                            memset(input->gamepads[i].buttons, 0, sizeof(input->gamepads[i].buttons));
                            TRACELOG(LOG_INFO, "Gamepad added at index %d: %s", i, SDL_GetGamepadName(gp));
                        }
                        break;
                    }
                }
            }
        }break;
        case SDL_EVENT_GAMEPAD_REMOVED:{
            if(g_renderstate.mainWindow){
                window_input_state* input = &g_renderstate.mainWindow->input_state;
                SDL_JoystickID id = event.gdevice.which;
                for(int i = 0; i < GAMEPAD_MAX; i++) {
                    if(input->gamepads[i].connected && 
                        SDL_GetGamepadID((SDL_Gamepad*)input->gamepads[i].handle) == id) {
                        SDL_CloseGamepad((SDL_Gamepad*)input->gamepads[i].handle);
                        input->gamepads[i].connected = false;
                        input->gamepads[i].handle = NULL;
                        TRACELOG(LOG_INFO, "Gamepad removed from index %d", i);
                        break;
                    }
                }
            }
        }break;
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:{
            if(g_renderstate.mainWindow){
                window_input_state* input = &g_renderstate.mainWindow->input_state;
                SDL_JoystickID id = event.gbutton.which;
                for(int i = 0; i < GAMEPAD_MAX; i++) {
                    if(input->gamepads[i].connected && 
                        SDL_GetGamepadID((SDL_Gamepad*)input->gamepads[i].handle) == id) {
                        if(event.gbutton.button < GAMEPAD_BUTTON_MAX) {
                            input->gamepads[i].buttons[event.gbutton.button] = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) ? 1 : 0;
                        }
                        break;
                    }
                }
            }
        }break;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:{
            if(g_renderstate.mainWindow){
                window_input_state* input = &g_renderstate.mainWindow->input_state;
                SDL_JoystickID id = event.gaxis.which;
                for(int i = 0; i < GAMEPAD_MAX; i++) {
                    if(input->gamepads[i].connected && 
                       SDL_GetGamepadID((SDL_Gamepad*)input->gamepads[i].handle) == id) {
                        if(event.gaxis.axis < GAMEPAD_AXIS_MAX) {
                            input->gamepads[i].axis[event.gaxis.axis] = (float)event.gaxis.value / 32768.0f;
                        }
                        break;
                    }
                }
            }
        }break;

        default:
            break;
        }
    }
}
RGAPI void CloseSubWindow_SDL3(SubWindow subWindow){
    rassert(subWindow->type == windowType_sdl3, "CloseSubWindow_SDL3 called on non-SDL3 window");
    SDL_DestroyWindow((SDL_Window*)subWindow->handle);
}

void ToggleFullscreen_SDL3(cwoid){
    bool alreadyFullscreen = SDL_GetWindowFlags((SDL_Window*)g_renderstate.window) & SDL_WINDOW_FULLSCREEN;
    if(alreadyFullscreen){
        //We need to exit fullscreen
        g_renderstate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        SDL_SetWindowFullscreen((SDL_Window*)g_renderstate.window, 0);
        SDL_SetWindowSize(
            (SDL_Window*)g_renderstate.window,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.width,
            (int)CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition.height
        );
    }
    else{
        int xpos = 0, ypos = 0;
        int xs, ys;
        #if RAYGPU_USE_WAYLAND != 1
        SDL_GetWindowPosition((SDL_Window*)g_renderstate.window, &xpos, &ypos);
        #endif
        SDL_GetWindowSize((SDL_Window*)g_renderstate.window, &xs, &ys);
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, g_renderstate.window)->input_state.windowPosition = CLITERAL(Rectangle){(float)xpos, ((float)ypos), ((float)xs), ((float)ys)};
        SDL_SetWindowSize((SDL_Window*)g_renderstate.window, GetMonitorWidth_SDL3(), GetMonitorHeight_SDL3());
        SDL_SetWindowFullscreen((SDL_Window*)g_renderstate.window, SDL_WINDOW_FULLSCREEN);
    }
}

void ShowCursor_SDL3(void* window){
    SDL_ShowCursor();
}
void HideCursor_SDL3(void* window){
    SDL_HideCursor();
}
bool IsCursorHidden_SDL3(void* window){
    return !SDL_CursorVisible() || SDL_GetWindowRelativeMouseMode((SDL_Window*)window);
}
void EnableCursor_SDL3(void* window){
    SDL_SetWindowRelativeMouseMode((SDL_Window*)window, false);
    SDL_ShowCursor();
}
void DisableCursor_SDL3(void* window){
    SDL_SetWindowRelativeMouseMode((SDL_Window*)window, true);
}

// Gamepad input functions
bool IsGamepadAvailable_SDL3(int gamepad) {
    if (gamepad < 0 || gamepad >= GAMEPAD_MAX) return false;
    if (!g_renderstate.mainWindow) return false;
    return g_renderstate.mainWindow->input_state.gamepads[gamepad].connected;
}

const char *GetGamepadName_SDL3(int gamepad) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return NULL;
    SDL_Gamepad *gp = (SDL_Gamepad *)g_renderstate.mainWindow->input_state.gamepads[gamepad].handle;
    return SDL_GetGamepadName(gp);
}

bool IsGamepadButtonPressed_SDL3(int gamepad, int button) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return false;
    if (button < 0 || button >= GAMEPAD_BUTTON_MAX) return false;
    GamepadState *state = &g_renderstate.mainWindow->input_state.gamepads[gamepad];
    return state->buttons[button] && !state->buttonsPrevious[button];
}

bool IsGamepadButtonDown_SDL3(int gamepad, int button) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return false;
    if (button < 0 || button >= GAMEPAD_BUTTON_MAX) return false;
    return g_renderstate.mainWindow->input_state.gamepads[gamepad].buttons[button];
}

bool IsGamepadButtonReleased_SDL3(int gamepad, int button) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return false;
    if (button < 0 || button >= GAMEPAD_BUTTON_MAX) return false;
    GamepadState *state = &g_renderstate.mainWindow->input_state.gamepads[gamepad];
    return !state->buttons[button] && state->buttonsPrevious[button];
}

int GetGamepadButtonPressed_SDL3(void) {
    if (!g_renderstate.mainWindow) return GAMEPAD_BUTTON_UNKNOWN;
    for (int gp = 0; gp < GAMEPAD_MAX; gp++) {
        if (!g_renderstate.mainWindow->input_state.gamepads[gp].connected) continue;
        GamepadState *state = &g_renderstate.mainWindow->input_state.gamepads[gp];
        for (int btn = 0; btn < GAMEPAD_BUTTON_MAX; btn++) {
            if (state->buttons[btn] && !state->buttonsPrevious[btn]) {
                return btn;
            }
        }
    }
    return GAMEPAD_BUTTON_UNKNOWN;
}

int GetGamepadAxisCount_SDL3(int gamepad) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return 0;
    return GAMEPAD_AXIS_MAX;
}

float GetGamepadAxisMovement_SDL3(int gamepad, int axis) {
    if (!IsGamepadAvailable_SDL3(gamepad)) return 0.0f;
    if (axis < 0 || axis >= GAMEPAD_AXIS_MAX) return 0.0f;
    return g_renderstate.mainWindow->input_state.gamepads[gamepad].axis[axis];
}

// end file src/InitWindow_SDL3.c
