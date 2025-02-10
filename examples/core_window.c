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
    ClearBackground((Color){230, 23, 230,255});
    DrawText("Hello Vulkan enjoyer", 100, 300, 50, (Color){190, 190, 190,255});
    DrawFPS(5, 5);
    //if(IsKeyPressed(KEY_U)){
    //    ToggleFullscreen();
    //}
    drawCurrentBatch();
    UseNoTexture();
    rlVertex2f(0, 0);
    rlVertex2f(100, 0);
    rlVertex2f(0, 100);
    rlVertex2f(0, 200);
    drawCurrentBatch();
    EndDrawing();
    
}
int main(void){
    //RequestLimit(maxBufferSize, 1 << 13);
    //RequestBackend(WGPUBackendType_Vulkan);
    //SetConfigFlags(FLAG_VSYNC_HINT);
    //SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Vulkan window");
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
