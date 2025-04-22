#define RGFW_IMPLEMENTATION
#define RGFW_USE_XDL


#define Font rlFont
    #include <raygpu.h>
#undef Font
#include <array>
enum struct RGFW_key_alt {
	RGFW_keyNULL = 0,
	RGFW_escape = '\033',
	RGFW_backtick = '`',
	RGFW_0 = '0',
	RGFW_1 = '1',
	RGFW_2 = '2',
	RGFW_3 = '3',
	RGFW_4 = '4',
	RGFW_5 = '5',
	RGFW_6 = '6',
	RGFW_7 = '7',
	RGFW_8 = '8',
	RGFW_9 = '9',

	RGFW_minus = '-',
	RGFW_equals = '=',
	RGFW_backSpace = '\b',
	RGFW_tab = '\t',
	RGFW_space = ' ',

	RGFW_a = 'a',
	RGFW_b = 'b',
	RGFW_c = 'c',
	RGFW_d = 'd',
	RGFW_e = 'e',
	RGFW_f = 'f',
	RGFW_g = 'g',
	RGFW_h = 'h',
	RGFW_i = 'i',
	RGFW_j = 'j',
	RGFW_k = 'k',
	RGFW_l = 'l',
	RGFW_m = 'm',
	RGFW_n = 'n',
	RGFW_o = 'o',
	RGFW_p = 'p',
	RGFW_q = 'q',
	RGFW_r = 'r',
	RGFW_s = 's',
	RGFW_t = 't',
	RGFW_u = 'u',
	RGFW_v = 'v',
	RGFW_w = 'w',
	RGFW_x = 'x',
	RGFW_y = 'y',
	RGFW_z = 'z',

	RGFW_period = '.',
	RGFW_comma = ',',
	RGFW_slash = '/',
	RGFW_bracket = '{',
	RGFW_closeBracket = '}',
	RGFW_semicolon = ';',
	RGFW_apostrophe = '\'',
	RGFW_backSlash = '\\',
	RGFW_return = '\n',

	RGFW_delete = '\177', /* 127 */

	RGFW_F1,
	RGFW_F2,
	RGFW_F3,
	RGFW_F4,
	RGFW_F5,
	RGFW_F6,
	RGFW_F7,
	RGFW_F8,
	RGFW_F9,
	RGFW_F10,
	RGFW_F11,
	RGFW_F12,

	RGFW_capsLock,
	RGFW_shiftL,
	RGFW_controlL,
	RGFW_altL,
	RGFW_superL,
	RGFW_shiftR,
	RGFW_controlR,
	RGFW_altR,
	RGFW_superR,
	RGFW_up,
	RGFW_down,
	RGFW_left,
	RGFW_right,

	RGFW_insert,
	RGFW_end,
	RGFW_home,
	RGFW_pageUp,
	RGFW_pageDown,

	RGFW_numLock,
	RGFW_KP_Slash,
	RGFW_multiply,
	RGFW_KP_Minus,
	RGFW_KP_1,
	RGFW_KP_2,
	RGFW_KP_3,
	RGFW_KP_4,
	RGFW_KP_5,
	RGFW_KP_6,
	RGFW_KP_7,
	RGFW_KP_8,
	RGFW_KP_9,
	RGFW_KP_0,
	RGFW_KP_Period,
	RGFW_KP_Return,
	RGFW_scrollLock,
	RGFW_keyLast
};

const std::array<short, 512> keyMappingRGFW_ = [](){
    std::array<short, 512> ret;
    ret[(int)RGFW_key_alt::RGFW_keyNULL] = KEY_NULL;
    ret[(int)RGFW_key_alt::RGFW_return] = KEY_ENTER;
    ret[(int)RGFW_key_alt::RGFW_return] = KEY_ENTER;
    ret[(int)RGFW_key_alt::RGFW_apostrophe] = KEY_APOSTROPHE;
    ret[(int)RGFW_key_alt::RGFW_comma] = KEY_COMMA;
    ret[(int)RGFW_key_alt::RGFW_minus] = KEY_MINUS;
    ret[(int)RGFW_key_alt::RGFW_period] = KEY_PERIOD;
    ret[(int)RGFW_key_alt::RGFW_slash] = KEY_SLASH;
    ret[(int)RGFW_key_alt::RGFW_escape] = KEY_ESCAPE;
    ret[(int)RGFW_key_alt::RGFW_F1] = KEY_F1;
    ret[(int)RGFW_key_alt::RGFW_F2] = KEY_F2;
    ret[(int)RGFW_key_alt::RGFW_F3] = KEY_F3;
    ret[(int)RGFW_key_alt::RGFW_F4] = KEY_F4;
    ret[(int)RGFW_key_alt::RGFW_F5] = KEY_F5;
    ret[(int)RGFW_key_alt::RGFW_F6] = KEY_F6;
    ret[(int)RGFW_key_alt::RGFW_F7] = KEY_F7;
    ret[(int)RGFW_key_alt::RGFW_F8] = KEY_F8;
    ret[(int)RGFW_key_alt::RGFW_F9] = KEY_F9;
    ret[(int)RGFW_key_alt::RGFW_F10] = KEY_F10;
    ret[(int)RGFW_key_alt::RGFW_F11] = KEY_F11;
    ret[(int)RGFW_key_alt::RGFW_F12] = KEY_F12;
    ret[(int)RGFW_key_alt::RGFW_backtick] = KEY_GRAVE;
    ret[(int)RGFW_key_alt::RGFW_0] = KEY_ZERO;
    ret[(int)RGFW_key_alt::RGFW_1] = KEY_ONE;
    ret[(int)RGFW_key_alt::RGFW_2] = KEY_TWO;
    ret[(int)RGFW_key_alt::RGFW_3] = KEY_THREE;
    ret[(int)RGFW_key_alt::RGFW_4] = KEY_FOUR;
    ret[(int)RGFW_key_alt::RGFW_5] = KEY_FIVE;
    ret[(int)RGFW_key_alt::RGFW_6] = KEY_SIX;
    ret[(int)RGFW_key_alt::RGFW_7] = KEY_SEVEN;
    ret[(int)RGFW_key_alt::RGFW_8] = KEY_EIGHT;
    ret[(int)RGFW_key_alt::RGFW_9] = KEY_NINE;
    ret[(int)RGFW_key_alt::RGFW_equals] = KEY_EQUAL;
    ret[(int)RGFW_key_alt::RGFW_backSpace] = KEY_BACKSPACE;
    ret[(int)RGFW_key_alt::RGFW_tab] = KEY_TAB;
    ret[(int)RGFW_key_alt::RGFW_capsLock] = KEY_CAPS_LOCK;
    ret[(int)RGFW_key_alt::RGFW_shiftL] = KEY_LEFT_SHIFT;
    ret[(int)RGFW_key_alt::RGFW_controlL] = KEY_LEFT_CONTROL;
    ret[(int)RGFW_key_alt::RGFW_altL] = KEY_LEFT_ALT;
    ret[(int)RGFW_key_alt::RGFW_superL] = KEY_LEFT_SUPER;
    #ifndef RGFW_MACO
    ret[(int)RGFW_key_alt::RGFW_shiftR] = KEY_RIGHT_SHIFT;
    ret[(int)RGFW_key_alt::RGFW_altR] = KEY_RIGHT_ALT;
    #endif
    ret[(int)RGFW_key_alt::RGFW_space] = KEY_SPACE;
    ret[(int)RGFW_key_alt::RGFW_a] = KEY_A;
    ret[(int)RGFW_key_alt::RGFW_b] = KEY_B;
    ret[(int)RGFW_key_alt::RGFW_c] = KEY_C;
    ret[(int)RGFW_key_alt::RGFW_d] = KEY_D;
    ret[(int)RGFW_key_alt::RGFW_e] = KEY_E;
    ret[(int)RGFW_key_alt::RGFW_f] = KEY_F;
    ret[(int)RGFW_key_alt::RGFW_g] = KEY_G;
    ret[(int)RGFW_key_alt::RGFW_h] = KEY_H;
    ret[(int)RGFW_key_alt::RGFW_i] = KEY_I;
    ret[(int)RGFW_key_alt::RGFW_j] = KEY_J;
    ret[(int)RGFW_key_alt::RGFW_k] = KEY_K;
    ret[(int)RGFW_key_alt::RGFW_l] = KEY_L;
    ret[(int)RGFW_key_alt::RGFW_m] = KEY_M;
    ret[(int)RGFW_key_alt::RGFW_n] = KEY_N;
    ret[(int)RGFW_key_alt::RGFW_o] = KEY_O;
    ret[(int)RGFW_key_alt::RGFW_p] = KEY_P;
    ret[(int)RGFW_key_alt::RGFW_q] = KEY_Q;
    ret[(int)RGFW_key_alt::RGFW_r] = KEY_R;
    ret[(int)RGFW_key_alt::RGFW_s] = KEY_S;
    ret[(int)RGFW_key_alt::RGFW_t] = KEY_T;
    ret[(int)RGFW_key_alt::RGFW_u] = KEY_U;
    ret[(int)RGFW_key_alt::RGFW_v] = KEY_V;
    ret[(int)RGFW_key_alt::RGFW_w] = KEY_W;
    ret[(int)RGFW_key_alt::RGFW_x] = KEY_X;
    ret[(int)RGFW_key_alt::RGFW_y] = KEY_Y;
    ret[(int)RGFW_key_alt::RGFW_z] = KEY_Z;
    ret[(int)RGFW_key_alt::RGFW_bracket] = KEY_LEFT_BRACKET;
    ret[(int)RGFW_key_alt::RGFW_backSlash] = KEY_BACKSLASH;
    ret[(int)RGFW_key_alt::RGFW_closeBracket] = KEY_RIGHT_BRACKET;
    ret[(int)RGFW_key_alt::RGFW_semicolon] = KEY_SEMICOLON;
    ret[(int)RGFW_key_alt::RGFW_insert] = KEY_INSERT;
    ret[(int)RGFW_key_alt::RGFW_home] = KEY_HOME;
    ret[(int)RGFW_key_alt::RGFW_pageUp] = KEY_PAGE_UP;
    ret[(int)RGFW_key_alt::RGFW_delete] = KEY_DELETE;
    ret[(int)RGFW_key_alt::RGFW_end] = KEY_END;
    ret[(int)RGFW_key_alt::RGFW_pageDown] = KEY_PAGE_DOWN;
    ret[(int)RGFW_key_alt::RGFW_right] = KEY_RIGHT;
    ret[(int)RGFW_key_alt::RGFW_left] = KEY_LEFT;
    ret[(int)RGFW_key_alt::RGFW_down] = KEY_DOWN;
    ret[(int)RGFW_key_alt::RGFW_up] = KEY_UP;
    ret[(int)RGFW_key_alt::RGFW_numLock] = KEY_NUM_LOCK;
    ret[(int)RGFW_key_alt::RGFW_KP_Slash] = KEY_KP_DIVIDE;
    ret[(int)RGFW_key_alt::RGFW_multiply] = KEY_KP_MULTIPLY;
    ret[(int)RGFW_key_alt::RGFW_KP_Minus] = KEY_KP_SUBTRACT;
    ret[(int)RGFW_key_alt::RGFW_KP_Return] = KEY_KP_ENTER;
    ret[(int)RGFW_key_alt::RGFW_KP_1] = KEY_KP_1;
    ret[(int)RGFW_key_alt::RGFW_KP_2] = KEY_KP_2;
    ret[(int)RGFW_key_alt::RGFW_KP_3] = KEY_KP_3;
    ret[(int)RGFW_key_alt::RGFW_KP_4] = KEY_KP_4;
    ret[(int)RGFW_key_alt::RGFW_KP_5] = KEY_KP_5;
    ret[(int)RGFW_key_alt::RGFW_KP_6] = KEY_KP_6;
    ret[(int)RGFW_key_alt::RGFW_KP_7] = KEY_KP_7;
    ret[(int)RGFW_key_alt::RGFW_KP_8] = KEY_KP_8;
    ret[(int)RGFW_key_alt::RGFW_KP_9] = KEY_KP_9;
    ret[(int)RGFW_key_alt::RGFW_KP_0] = KEY_KP_0;
    ret[(int)RGFW_key_alt::RGFW_KP_Period] = KEY_KP_DECIMAL;
    ret[(int)RGFW_key_alt::RGFW_scrollLock] = KEY_SCROLL_LOCK;
    return ret;
}();
#if SUPPORT_VULKAN_BACKEND == 1
    #define RGFW_VULKAN
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
#else
    #ifdef __EMSCRIPTEN__
        #define RGFW_WASM
    #endif
#endif
#include <external/RGFW.h>
#include <internals.hpp>

#include <renderstate.hpp>
#if SUPPORT_VULKAN_BACKEND == 1
    #include "backend_vulkan/vulkan_internals.hpp"
#endif
#if SUPPORT_VULKAN_BACKEND == 1
VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice pd, uint32_t i){
    return RGFW_getVKPresentationSupport(instance, pd, i);
}
#endif
void keyfunc_rgfw(RGFW_window* window, RGFW_key key, unsigned char keyChar, RGFW_keymod keyMod, RGFW_bool pressed) {
    
    KeyboardKey kii = (KeyboardKey)keyMappingRGFW_[key];
    g_renderstate.input_map[window].keydown[kii] = pressed ? 1 : 0;
}

void PollEvents_RGFW(){
    for(const auto& [handle, subwindow] : g_renderstate.createdSubwindows){
        if(subwindow.type == windowType_rgfw){
            RGFW_window_checkEvents((RGFW_window*)handle, 1);
        }
    }
}
void setupRGFWCallbacks(RGFW_window* window){
    RGFW_setKeyCallback(keyfunc_rgfw);
}
extern "C" void* CreateSurfaceForWindow_RGFW(void* windowHandle){
    
    #if SUPPORT_VULKAN_BACKEND == 1
    WGVKSurface retp = callocnew(WGVKSurfaceImpl);
    RGFW_window_createVKSurface((RGFW_window*)windowHandle, g_vulkanstate.instance->instance, &retp->surface);
    return retp;
    #else
    WGPUSurface surf = (WGPUSurface)RGFW_GetWGPUSurface(GetInstance(), (RGFW_window*) windowHandle);
    return surf;
    #endif
}

SubWindow InitWindow_RGFW(int width, int height, const char* title){
    SubWindow ret{};
    ret.type = windowType_rgfw;
    ret.handle = RGFW_createWindow(title, RGFW_rect{0, 0, width, height}, RGFW_windowCenter);
    
    setupRGFWCallbacks((RGFW_window*)ret.handle);
    return ret;
}
