#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
SubWindow sdlwin;
void mainloop(void){
    //BeginWindowMode(sdlwin);
    //ClearBackground(RED);
    //DrawRectangle(GetMouseX(), GetMouseY(), 100, 100, BLUE);
    //EndWindowMode();

    BeginDrawing();
    ClearBackground((Color){230, 230, 230,255});
    DrawText("Hello WebGPU enjoyer", 100, 300, 50, (Color){190, 190, 190,255});
    DrawFPS(5, 5);
    if(IsKeyPressed(KEY_U)){
        ToggleFullscreen();
    }
    EndDrawing();
    
}
int main(void){
    RequestLimit(maxBufferSize, 1 << 13);
    RequestBackend(WGPUBackendType_Vulkan);
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "WebGPU window");
    //SetTargetFPS(0);
    //sdlwin = OpenSubWindow_GLFW(400, 400, "SDL fenschter");

    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    
    //CopyTextureToTexture
    #else
    //requestAnimationFrameLoopWithJSPI(mainloop, 0, 0);
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
    
}
