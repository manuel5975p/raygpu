#define RGFW_IMPLEMENTATION
#define RGFW_USE_XDL
#define RGFW_VULKAN

#define Font rlFont
    #include <raygpu.h>
#undef Font

#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#include <external/RGFW.h>
#include <internals.hpp>
#if SUPPORT_VULKAN_BACKEND == 1
    #include "backend_vulkan/vulkan_internals.hpp"
#endif

VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice pd, uint32_t i){
    return RGFW_getVKPresentationSupport(instance, pd, i);
}

//void keyfunc(RGFW_window* win, RGFW_key key, unsigned char keyChar, RGFW_keymod keyMod, RGFW_bool pressed) {
//    if (key == RGFW_escape && pressed) {
//        RGFW_window_setShouldClose(win, 1);
//    }
//}
//int main() {
//    RGFW_window* win = RGFW_createWindow("a window", RGFW_RECT(0, 0, 800, 600), 0);
//    
//    RGFW_setKeyCallback(keyfunc); // you can use callbacks like this if you want
//    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
//        while (RGFW_window_checkEvent(win)) {  // or RGFW_window_checkEvents(); if you only want callbacks
//            // you can either check the current event yourself
//            if (win->event.type == RGFW_quit) break;
//            
//            if (win->event.type == RGFW_mouseButtonPressed && win->event.button == RGFW_mouseLeft) {
//                printf("You clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
//            }
//
//            // or use the existing functions
//            if (RGFW_isMousePressed(win, RGFW_mouseRight)) {
//                printf("The right mouse button was clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
//            }
//        }
//
//        RGFW_window_swapBuffers(win);
//    }
//
//    RGFW_window_close(win);
//    return 0;
//}

void setupRGFWCallbacks(RGFW_window* window){

}
extern "C" void* CreateSurfaceForWindow_RGFW(void* windowHandle){
    
    #if SUPPORT_VULKAN_BACKEND == 1
    WGVKSurface retp = callocnew(WGVKSurfaceImpl);
    RGFW_window_createVKSurface((RGFW_window*)windowHandle, g_vulkanstate.instance, &retp->surface);
    return retp;
    #endif
}

SubWindow InitWindow_RGFW(int width, int height, const char* title){
    SubWindow ret{};
    ret.type = windowType_rgfw;
    ret.handle = RGFW_createWindow(title, RGFW_rect{0, 0, width, height}, RGFW_windowCenter);
    
    setupRGFWCallbacks((RGFW_window*)ret.handle);
    return ret;
}
