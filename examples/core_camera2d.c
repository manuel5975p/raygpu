#include <raygpu.h>
int main(void){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1000, 1000, "Camera2D Example");
    Camera2D cam = (Camera2D){
        .offset = (Vector2){500, 500},
        .target = (Vector2){0, 0},
        .rotation = 90.0f,
        .zoom = 10.f,
    };

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);
        DrawRectangleV((Vector2){-90.0f,-40.0f},   (Vector2){180.0f,80.0f}, (Color) { 80, 80, 80, 255 });
        DrawText("This is in world space", -50, 5, 10, GREEN);
        DrawRectangleV((Vector2){-4.0f,-4.0f},   (Vector2){8.0f,8.0f}, BLUE);
        DrawRectangleV((Vector2){-0.4f,-0.4f},   (Vector2){0.8f,0.8f}, RED);
        DrawRectangleV((Vector2){-0.04f,-0.04f}, (Vector2){0.08f,0.08f}, WHITE);
        EndMode2D();
        DrawText("This is in screen space", 0, 100, 30, RED);
        DrawFPS(0,0);
        cam.zoom = fminf(1e30f, fmaxf(1e-30f, cam.zoom * expf(GetMouseWheelMove() * 0.1f)));
        const char* text = TextFormat("Zoom: %.4e", cam.zoom);
        int tw = MeasureText(text, 50);
        DrawText(text, GetScreenWidth() - tw - 20, 0, 50, WHITE);
        EndDrawing();
    }
    return 0;
}

