#include <raygpu.h>

int main() {
    InitWindow(600, 600, "Hello!");

    RenderTexture tex  = LoadRenderTexture(100, 100);
    
    BeginTextureMode(tex);
    ClearBackground(RED);
    EndTextureMode();

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        DrawTextureV(tex.texture, (Vector2){10, 10}, WHITE);
        DrawCircleV(GetMousePosition(),   100, BLACK);
        EndDrawing();
    }
}