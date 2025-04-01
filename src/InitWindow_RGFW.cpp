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
#include <renderstate.hpp>
#if SUPPORT_VULKAN_BACKEND == 1
    #include "backend_vulkan/vulkan_internals.hpp"
#endif

VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice pd, uint32_t i){
    return RGFW_getVKPresentationSupport(instance, pd, i);
}
void keyfunc_rgfw(RGFW_window* window, RGFW_key key, unsigned char keyChar, RGFW_keymod keyMod, RGFW_bool pressed) {
    g_renderstate.input_map[window].keydown[key] = pressed ? 1 : 0;
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
