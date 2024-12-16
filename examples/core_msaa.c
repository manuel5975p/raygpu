#include <raygpu.h>

int main(void){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "WebGPU window");
    bool flag = 0;
    const int textWidth = MeasureText("Hello MSAA Enjoyer", 100);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){130, 130, 130, 255});
        DrawText("Hello MSAA Enjoyer", GetScreenWidth() / 2 - textWidth / 2, GetScreenHeight() / 2 - 50, 100, (Color){210, 210, 210,255});
        DrawCircleSector(GetMousePosition(), 100.0f, 0.0f, 360.0f, 128, WHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }
}
