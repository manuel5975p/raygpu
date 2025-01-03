#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
void mainloop(void){
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
    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1920, 1080, "WebGPU window");
    SetTargetFPS(0);
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
    
}
