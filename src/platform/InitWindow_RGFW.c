// begin file src/InitWindow_RGFW.c
#define RGFW_IMPLEMENTATION
#define RGFW_USE_XDL

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#if RAYGPU_USE_X11 == 1 
    #ifndef VK_KHR_xlib_surface
        #define VK_KHR_xlib_surface 1
    #endif
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
#endif

#if defined(__APPLE__)

    //#define VK_KHR_mvk_surface 1
    #define VK_MVK_macos_surface 1
    #include <vulkan/vulkan_macos.h>
#endif

#define Font rlFont
    #include <raygpu.h>
#undef Font

enum RGFW_key_alt {
    alt_RGFW_keyNULL = 0,
    alt_RGFW_escape = '\033',
    alt_RGFW_backtick = '`',
    alt_RGFW_0 = '0',
    alt_RGFW_1 = '1',
    alt_RGFW_2 = '2',
    alt_RGFW_3 = '3',
    alt_RGFW_4 = '4',
    alt_RGFW_5 = '5',
    alt_RGFW_6 = '6',
    alt_RGFW_7 = '7',
    alt_RGFW_8 = '8',
    alt_RGFW_9 = '9',
    alt_RGFW_minus = '-',
    alt_RGFW_equals = '=',
    alt_RGFW_backSpace = '\b',
    alt_RGFW_tab = '\t',
    alt_RGFW_space = ' ',
    alt_RGFW_a = 'a',
    alt_RGFW_b = 'b',
    alt_RGFW_c = 'c',
    alt_RGFW_d = 'd',
    alt_RGFW_e = 'e',
    alt_RGFW_f = 'f',
    alt_RGFW_g = 'g',
    alt_RGFW_h = 'h',
    alt_RGFW_i = 'i',
    alt_RGFW_j = 'j',
    alt_RGFW_k = 'k',
    alt_RGFW_l = 'l',
    alt_RGFW_m = 'm',
    alt_RGFW_n = 'n',
    alt_RGFW_o = 'o',
    alt_RGFW_p = 'p',
    alt_RGFW_q = 'q',
    alt_RGFW_r = 'r',
    alt_RGFW_s = 's',
    alt_RGFW_t = 't',
    alt_RGFW_u = 'u',
    alt_RGFW_v = 'v',
    alt_RGFW_w = 'w',
    alt_RGFW_x = 'x',
    alt_RGFW_y = 'y',
    alt_RGFW_z = 'z',
    alt_RGFW_period = '.',
    alt_RGFW_comma = ',',
    alt_RGFW_slash = '/',
    alt_RGFW_bracket = '[',
    alt_RGFW_closeBracket = ']',
    alt_RGFW_semicolon = ';',
    alt_RGFW_apostrophe = '\'',
    alt_RGFW_backSlash = '\\',
    alt_RGFW_return = '\n',
    alt_RGFW_enter = alt_RGFW_return,
    alt_RGFW_delete = '\177', /* 127 */
    alt_RGFW_F1,
    alt_RGFW_F2,
    alt_RGFW_F3,
    alt_RGFW_F4,
    alt_RGFW_F5,
    alt_RGFW_F6,
    alt_RGFW_F7,
    alt_RGFW_F8,
    alt_RGFW_F9,
    alt_RGFW_F10,
    alt_RGFW_F11,
    alt_RGFW_F12,
    alt_RGFW_F13,
    alt_RGFW_F14,
    alt_RGFW_F15,
    alt_RGFW_F16,
    alt_RGFW_F17,
    alt_RGFW_F18,
    alt_RGFW_F19,
    alt_RGFW_F20,
    alt_RGFW_F21,
    alt_RGFW_F22,
    alt_RGFW_F23,
    alt_RGFW_F24,
    alt_RGFW_F25,
    alt_RGFW_capsLock,
    alt_RGFW_shiftL,
    alt_RGFW_controlL,
    alt_RGFW_altL,
    alt_RGFW_superL,
    alt_RGFW_shiftR,
    alt_RGFW_controlR,
    alt_RGFW_altR,
    alt_RGFW_superR,
    alt_RGFW_up,
    alt_RGFW_down,
    alt_RGFW_left,
    alt_RGFW_right,
    alt_RGFW_insert,
    alt_RGFW_menu,
    alt_RGFW_end,
    alt_RGFW_home,
    alt_RGFW_pageUp,
    alt_RGFW_pageDown,
    alt_RGFW_numLock,
    alt_RGFW_kpSlash,
    alt_RGFW_kpMultiply,
    alt_RGFW_kpPlus,
    alt_RGFW_kpMinus,
    alt_RGFW_kpEqual,
    alt_RGFW_kp1,
    alt_RGFW_kp2,
    alt_RGFW_kp3,
    alt_RGFW_kp4,
    alt_RGFW_kp5,
    alt_RGFW_kp6,
    alt_RGFW_kp7,
    alt_RGFW_kp8,
    alt_RGFW_kp9,
    alt_RGFW_kp0,
    alt_RGFW_kpPeriod,
    alt_RGFW_kpReturn,
    alt_RGFW_scrollLock,
    alt_RGFW_printScreen,
    alt_RGFW_pause,
    alt_RGFW_world1,
    alt_RGFW_world2,
    alt_RGFW_keyLast = 256 /* padding for alignment ~(175 by default) */
};

const int keyMappingRGFW_(int RGFWKey){
    switch(RGFWKey){
        default:
        case alt_RGFW_keyNULL:return KEY_NULL;
        case alt_RGFW_return:return KEY_ENTER;
        case alt_RGFW_apostrophe:return KEY_APOSTROPHE;
        case alt_RGFW_comma:return KEY_COMMA;
        case alt_RGFW_minus:return KEY_MINUS;
        case alt_RGFW_period:return KEY_PERIOD;
        case alt_RGFW_slash:return KEY_SLASH;
        case alt_RGFW_escape:return KEY_ESCAPE;
        case alt_RGFW_F1:return KEY_F1;
        case alt_RGFW_F2:return KEY_F2;
        case alt_RGFW_F3:return KEY_F3;
        case alt_RGFW_F4:return KEY_F4;
        case alt_RGFW_F5:return KEY_F5;
        case alt_RGFW_F6:return KEY_F6;
        case alt_RGFW_F7:return KEY_F7;
        case alt_RGFW_F8:return KEY_F8;
        case alt_RGFW_F9:return KEY_F9;
        case alt_RGFW_F10:return KEY_F10;
        case alt_RGFW_F11:return KEY_F11;
        case alt_RGFW_F12:return KEY_F12;
        case alt_RGFW_backtick:return KEY_GRAVE;
        case alt_RGFW_0:return KEY_ZERO;
        case alt_RGFW_1:return KEY_ONE;
        case alt_RGFW_2:return KEY_TWO;
        case alt_RGFW_3:return KEY_THREE;
        case alt_RGFW_4:return KEY_FOUR;
        case alt_RGFW_5:return KEY_FIVE;
        case alt_RGFW_6:return KEY_SIX;
        case alt_RGFW_7:return KEY_SEVEN;
        case alt_RGFW_8:return KEY_EIGHT;
        case alt_RGFW_9:return KEY_NINE;
        case alt_RGFW_equals:return KEY_EQUAL;
        case alt_RGFW_backSpace:return KEY_BACKSPACE;
        case alt_RGFW_tab:return KEY_TAB;
        case alt_RGFW_capsLock:return KEY_CAPS_LOCK;
        case alt_RGFW_shiftL:return KEY_LEFT_SHIFT;
        case alt_RGFW_controlL:return KEY_LEFT_CONTROL;
        case alt_RGFW_altL:return KEY_LEFT_ALT;
        case alt_RGFW_superL:return KEY_LEFT_SUPER;
        case alt_RGFW_shiftR:return KEY_RIGHT_SHIFT;
        case alt_RGFW_altR:return KEY_RIGHT_ALT;
        case alt_RGFW_space:return KEY_SPACE;
        case alt_RGFW_a:return KEY_A;
        case alt_RGFW_b:return KEY_B;
        case alt_RGFW_c:return KEY_C;
        case alt_RGFW_d:return KEY_D;
        case alt_RGFW_e:return KEY_E;
        case alt_RGFW_f:return KEY_F;
        case alt_RGFW_g:return KEY_G;
        case alt_RGFW_h:return KEY_H;
        case alt_RGFW_i:return KEY_I;
        case alt_RGFW_j:return KEY_J;
        case alt_RGFW_k:return KEY_K;
        case alt_RGFW_l:return KEY_L;
        case alt_RGFW_m:return KEY_M;
        case alt_RGFW_n:return KEY_N;
        case alt_RGFW_o:return KEY_O;
        case alt_RGFW_p:return KEY_P;
        case alt_RGFW_q:return KEY_Q;
        case alt_RGFW_r:return KEY_R;
        case alt_RGFW_s:return KEY_S;
        case alt_RGFW_t:return KEY_T;
        case alt_RGFW_u:return KEY_U;
        case alt_RGFW_v:return KEY_V;
        case alt_RGFW_w:return KEY_W;
        case alt_RGFW_x:return KEY_X;
        case alt_RGFW_y:return KEY_Y;
        case alt_RGFW_z:return KEY_Z;
        case alt_RGFW_bracket:return KEY_LEFT_BRACKET;
        case alt_RGFW_backSlash:return KEY_BACKSLASH;
        case alt_RGFW_closeBracket:return KEY_RIGHT_BRACKET;
        case alt_RGFW_semicolon:return KEY_SEMICOLON;
        case alt_RGFW_insert:return KEY_INSERT;
        case alt_RGFW_home:return KEY_HOME;
        case alt_RGFW_pageUp:return KEY_PAGE_UP;
        case alt_RGFW_delete:return KEY_DELETE;
        case alt_RGFW_end:return KEY_END;
        case alt_RGFW_pageDown:return KEY_PAGE_DOWN;
        case alt_RGFW_right:return KEY_RIGHT;
        case alt_RGFW_left:return KEY_LEFT;
        case alt_RGFW_down:return KEY_DOWN;
        case alt_RGFW_up:return KEY_UP;
        case alt_RGFW_numLock:return KEY_NUM_LOCK;
        case alt_RGFW_kpSlash:return KEY_KP_DIVIDE;
        case alt_RGFW_kpMultiply:return KEY_KP_MULTIPLY;
        case alt_RGFW_kpMinus:return KEY_KP_SUBTRACT;
        case alt_RGFW_kpReturn:return KEY_KP_ENTER;
        case alt_RGFW_kp1:return KEY_KP_1;
        case alt_RGFW_kp2:return KEY_KP_2;
        case alt_RGFW_kp3:return KEY_KP_3;
        case alt_RGFW_kp4:return KEY_KP_4;
        case alt_RGFW_kp5:return KEY_KP_5;
        case alt_RGFW_kp6:return KEY_KP_6;
        case alt_RGFW_kp7:return KEY_KP_7;
        case alt_RGFW_kp8:return KEY_KP_8;
        case alt_RGFW_kp9:return KEY_KP_9;
        case alt_RGFW_kp0:return KEY_KP_0;
        case alt_RGFW_kpPeriod:return KEY_KP_DECIMAL;
        case alt_RGFW_scrollLock:return KEY_SCROLL_LOCK;
    }
};
#define RGFWDEF
#if RAYGPU_USE_X11 == 1
    #define RGFW_USE_XDL
#endif
//#define RGFW_NO_DPI
#if SUPPORT_VULKAN_BACKEND == 1
#ifdef _WIN32
#define Rectangle w__Rectangle
#define LoadImage w__LoadImage
#define DrawText w__DrawText
#define DrawTextEx w__DrawTextEx
#define ShowCursor w__ShowCursor
#define AdapterType w__AdapterType
#include <windows.h>
#define VK_KHR_win32_surface 1
#include <vulkan/vulkan_win32.h>
#endif
#include <external/volk.h>
#define RGFW_VULKAN
#include <external/RGFW.h>
#ifdef _WIN32
#undef AdapterType
#undef ShowCursor
#undef LoadImage
#undef DrawTextEx
#undef DrawText
#undef Rectangle
#endif
#else
    #ifdef __EMSCRIPTEN__
        #define RGFW_WASM
    #endif
#endif


#include <internals.h>
#include <renderstate.h>
#if SUPPORT_VULKAN_BACKEND == 1
    #include <wgvk_structs_impl.h>
#endif
#if SUPPORT_VULKAN_BACKEND == 1
VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice pd, uint32_t i){
    return RGFW_getPresentationSupport_Vulkan(instance, pd, i);
}
#endif

void keyfunc_rgfw(RGFW_window* window, u8 key, u8 keyChar, RGFW_keymod keyMod, RGFW_bool repeat, RGFW_bool pressed) {
    if(repeat && pressed){
        return;
    }
    KeyboardKey kii = keyMappingRGFW_(key);
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->input_state.keydown[kii] = pressed ? 1 : 0;
    
    if(key == alt_RGFW_escape && pressed){
        CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->closeRequestedFlag = true;
    }
}

void mouseMotionfunc_rgfw(RGFW_window* win, i32 x, i32 y, float vecX, float vecY){
    float scaleFactor = CreatedWindowMap_get(&g_renderstate.createdSubwindows, win)->scaleFactor;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, win)->input_state.mousePos = CLITERAL(Vector2){(float)x * scaleFactor, (float)y * scaleFactor};
}

void windowQuitfunc_rgfw(RGFW_window* window){
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->closeRequestedFlag = true;
}

void windowResizedfunc_rgfw(RGFW_window* window, i32 w, i32 h){
    FullSurface* const surface = &CreatedWindowMap_get(&g_renderstate.createdSubwindows, window)->surface;
    ResizeSurface(surface, w, h);
    if((void*)window == (void*)g_renderstate.window){
        g_renderstate.mainWindowRenderTarget = surface->renderTarget;
    }
    Matrix newcamera = ScreenMatrix(w, h);
}

void PollEvents_RGFW(){
    if(_rgfwGlobal.windowCount){
        RGFW_pollEvents();
        // for(size_t i = 0;i < g_renderstate.createdSubwindows.current_capacity;i++){
        //     RGWindowImpl* pptr = &g_renderstate.createdSubwindows.table[i].value;
        //     if(pptr->type == windowType_rgfw){
        //         RGFW_event event;
        //         while(RGFW_window_checkEvent((RGFW_window*)pptr->handle, &event)); 
        //     }
        // }
    }
}
void mouseNotifyfunc_rgfw(RGFW_window* win, i32 x, i32 y, RGFW_bool entered){
    (void)x;
    (void)y;
    CreatedWindowMap_get(&g_renderstate.createdSubwindows, win)->input_state.cursorInWindow = entered;
} 
void setupRGFWCallbacks(RGFW_window* window){
    RGFW_setKeyCallback(keyfunc_rgfw);
    RGFW_setWindowResizedCallback(windowResizedfunc_rgfw);
    RGFW_setMousePosCallback(mouseMotionfunc_rgfw);
    RGFW_setWindowQuitCallback(windowQuitfunc_rgfw);
    RGFW_setMouseNotifyCallback(mouseNotifyfunc_rgfw);
}

RGAPI WGPUSurface CreateSurfaceForWindow_RGFW(void* windowHandle){
    WGPUInstance instance = GetInstance();
    WGPUSurface surf = (WGPUSurface)RGFW_GetWGPUSurface(instance, (RGFW_window*)windowHandle);
    // The following line is not required because OpenSubwindow and InitWindow for RGFW both initialize it to something.
    // CreatedWindowMap_get(&g_renderstate.createdSubwindows, windowHandle)->scaleFactor = 1;
    return surf;
}

SubWindow OpenSubWindow_RGFW_NoSurface(int width, int height, const char* title){
    SubWindow ret = callocnew(RGWindowImpl);
    ret->type = windowType_rgfw;
    
    ret->handle = RGFW_createWindow(title,0, 0, width, height, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE) ? 0 : RGFW_windowNoResize);
    int ret_width, ret_height;
    RGFW_window_getSize(ret->handle, &ret_width, &ret_height);
    ret->width = ret_width;
    ret->height = ret_height;
    RGFW_monitor monitor = RGFW_window_getMonitor(ret->handle);
    ret->scaleFactor = monitor.pixelRatio;
    CreatedWindowMap_put(&g_renderstate.createdSubwindows, ret->handle, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    setupRGFWCallbacks((RGFW_window*)ret->handle);
    return ret;
}

SubWindow OpenSubWindow_RGFW(int width, int height, const char* title){
    SubWindow sw = OpenSubWindow_RGFW_NoSurface(width, height, title);
    CreateAndSetSurfaceForWindow(sw);
    return sw;
}
RGAPI void CloseSubWindow_RGFW(SubWindow subWindow){
    rassert(subWindow->type == windowType_rgfw, "CloseSubWindow_RGFW called on non-SDL3 window");
    RGFW_window_close((RGFW_window*)subWindow->handle);
}

SubWindow InitWindow_RGFW(int width, int height, const char* title){
    SubWindow ret = callocnew(RGWindowImpl);
    /* Unpacked rect into x, y, w, h. Removed RGFW_windowNoInitAPI (not needed/deprecated) */
    ret->handle = RGFW_createWindow(title, 0, 0, width, height, RGFW_windowNoResize);
    ret->type = windowType_rgfw;
    RGFW_monitor monitor = RGFW_window_getMonitor(ret->handle);
    ret->scaleFactor = monitor.pixelRatio;
    CreatedWindowMap_put(&g_renderstate.createdSubwindows, ret->handle, *ret);
    ret = CreatedWindowMap_get(&g_renderstate.createdSubwindows, ret->handle);
    setupRGFWCallbacks((RGFW_window*)ret->handle);
    return ret;
}

void ShowCursor_RGFW(void* window) {
    RGFW_window_showMouse((RGFW_window*)window, true);
}
void HideCursor_RGFW(void* window) {
    RGFW_window_showMouse((RGFW_window*)window, false);
}
bool IsCursorHidden_RGFW(void* window) {
    return RGFW_window_isMouseHidden((RGFW_window*)window);
}
void EnableCursor_RGFW(void* window) {
    RGFW_window_captureMouse((RGFW_window*)window, false);
    RGFW_window_setRawMouseMode((RGFW_window*)window, false);
    ShowCursor_RGFW((RGFW_window*)window);
}
void DisableCursor_RGFW(void* window) {
    RGFW_window_captureMouse((RGFW_window*)window, true);
    RGFW_window_setRawMouseMode((RGFW_window*)window, true);
    HideCursor_RGFW((RGFW_window*)window);
}

// end file src/InitWindow_RGFW.cpp
