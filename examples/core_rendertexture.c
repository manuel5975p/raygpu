#include <raygpu.h>

int main(void){
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "WebGPU window");
    //SetTargetFPS(60);
    bool flag = 0;
    RenderTexture tex = LoadRenderTexture(300, 300);
    uint32_t framecount = 0;
    //BeginDrawing();
    //
    //EndDrawing();
    while(!WindowShouldClose()){
        BeginDrawing();
        BeginTextureMode(tex);
        ClearBackground(BLANK);
        DrawRectangle(50, 50, 100, 200, GREEN);
        DrawRectangle(50, 50, 10, 200, RED);
        EndTextureMode();
        ClearBackground(BLUE);
        DrawTexturePro(tex.texture, (Rectangle){0,0,300,300}, (Rectangle){100,100,400,400}, (Vector2){0,0}, 0.0f, WHITE);
        //DrawFPS(0, 0);
        EndDrawing();
    }
    
}
