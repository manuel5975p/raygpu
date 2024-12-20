#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int textWidth;
void mainloop(void){
    BeginDrawing();
    ClearBackground((Color){130, 130, 130, 255});
    DrawText("Hello MSAA Enjoyer", GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 50, 100, (Color){210, 210, 210,255});
    DrawCircleSector(GetMousePosition(), 100.0f, 0.0f, 360.0f, 128, WHITE);
    DrawFPS(10, 10);
    EndDrawing();
}
int main(void){
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "WebGPU window");
    textWidth = MeasureText("Hello MSAA Enjoyer", 100);
    
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
