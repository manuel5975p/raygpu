#define RGFW_IMPLEMENTATION
#define RGFW_USE_XDL
#define RGFW_VULKAN
#include <external/RGFW.h>
#include <raygpu.h>

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


SubWindow InitWindow_RGFW(int width, int height, const char* title){
    SubWindow ret{};
    ret.type = windowType_rgfw;
    void* window = nullptr;
    
    GLFWmonitor* mon = nullptr;

    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error: " << code << " - " << message;
    });


    #ifndef __EMSCRIPTEN__


        // Create the test window with no client API.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, (g_renderstate.windowFlags & FLAG_WINDOW_RESIZABLE) ? GLFW_TRUE : GLFW_FALSE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 144);

        if(g_renderstate.windowFlags & FLAG_FULLSCREEN_MODE){
            mon = glfwGetPrimaryMonitor();
            //std::cout <<glfwGetVideoMode(mon)->refreshRate << std::endl;
            //abort();
        }
    #endif
        
    #ifndef __EMSCRIPTEN__
        window = (void*)glfwCreateWindow(width, height, title, mon, nullptr);
        //glfwSetWindowPos((GLFWwindow*)window, 200, 1900);
        if (!window) {
            abort();
        }
        

        // Create the surface.
        #if SUPPORT_WGPU_BACKEND == 1
        wgpu::Surface rs = wgpu::glfw::CreateSurfaceForWindow((WGPUInstance)GetInstance(), (GLFWwindow*)window);
        WGPUSurface wsurfaceHandle = rs.MoveToCHandle();
        //ret.surface = CreateSurface(wsurfaceHandle, width, height);
        #elif SUPPORT_VULKAN_BACKEND == 1
        WGVKSurfaceConfiguration config{};
        config.format = BGRA8;
        config.presentMode = PresentMode_Fifo;
        config.width = width;
        config.height = height;
        //ret.surface = LoadSurface((GLFWwindow*)window, config);
        #endif
        //negotiateSurfaceFormatAndPresentMode(wsurfaceHandle);
    #else
        // Create the surface.
        wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "#canvas";

        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &canvasDesc;
        g_renderstate.surface = g_wgpustate.instance.CreateSurface(&surfaceDesc);
        negotiateSurfaceFormatAndPresentMode(g_wgpustate.surface);
        ret.surface.surface = g_wgpustate.surface.Get();
        window = glfwCreateWindow(width, height, title, mon, nullptr);
        g_renderstate.window = (GLFWwindow*)window;
    #endif
    

    
    int wposx = 0, wposy = 0;
    #ifndef DAWN_USE_WAYLAND
    glfwGetWindowPos((GLFWwindow*)window, &wposx, &wposy);
    #endif
    g_renderstate.input_map[window].windowPosition = Rectangle{
        (float)wposx,
        (float)wposy,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
    ret.handle = (void*)window;
    //ret.surface = GetSurface();
    //ret.surface.renderTarget = g_renderstate.mainWindowRenderTarget;
    g_renderstate.createdSubwindows[window] = ret;
    g_renderstate.input_map[ret.handle] = window_input_state{};
    setupGLFWCallbacks((GLFWwindow*)ret.handle);
    return ret;
}
