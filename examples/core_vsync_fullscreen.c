#include <raygpu.h>

int main(void){
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(), GetMonitorHeight(), "WebGPU window");
    bool flag = 0;
    const int textWidth = MeasureText("Hello VSync Enjoyer", 100);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){130, 130, 130, 255});
        DrawText("Hello VSync Enjoyer", GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 50, 100, (Color){210, 210, 210,255});
        DrawCircleSector(GetMousePosition(), 100.0f, 0.0f, 360.0f, 128, WHITE);
        DrawFPS(10, 10);
        if(IsKeyPressed(KEY_F)){
            ToggleFullscreenImpl();
        }
        EndDrawing();
    }
}
