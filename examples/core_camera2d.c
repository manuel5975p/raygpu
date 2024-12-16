#include <raygpu.h>
#include <math.h>
int main(void){
    InitWindow(1000, 1000, "Camera2D Example");
    Camera2D cam = CLITERAL(Camera2D){
        .offset = CLITERAL(Vector2){500, 500},
        .target = CLITERAL(Vector2){0, 0},
        .rotation = 0.0f,
        .zoom = 10.f,
    };
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        DrawFPS(0,0);
        BeginMode2D(cam);
        DrawRectangleV(CLITERAL(Vector2){-90.0f,-40.0f},   CLITERAL(Vector2){180.0f,80.0f}, (Color) { 80, 80, 80, 255 });
        DrawText("This is in worldspace", -50, 5, 10, GREEN);
        DrawRectangleV(CLITERAL(Vector2){-4.0f,-4.0f},   CLITERAL(Vector2){8.0f,8.0f}, BLUE);
        DrawRectangleV(CLITERAL(Vector2){-0.4f,-0.4f},   CLITERAL(Vector2){0.8f,0.8f}, RED);
        DrawRectangleV(CLITERAL(Vector2){-0.04f,-0.04f}, CLITERAL(Vector2){0.08f,0.08f}, WHITE);
        EndMode2D();
        ////DrawText("This is in screenspace", 10, 10, 30, GREEN);
        cam.zoom *= expf(GetMouseWheelMove() * 0.1f);
        EndDrawing();
    }
    return 0;
}

