#include <raygpu.h>

int main(void){
    InitWindow(800, 600, "WebGPU window");
    SetTargetFPS(60);
    bool flag = 0;
    Image img = GenImageChecker(RED, GREEN, 100, 100, 10);
    Texture tex = LoadTextureFromImage(img);
    float angle = 0.0f;
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){130, 130, 130, 255});
        DrawTexturePro(tex, CLITERAL(Rectangle){0,0,100,100}, CLITERAL(Rectangle){100,150,100,100}, CLITERAL(Vector2){50,50}, angle, WHITE);
        DrawTexturePro(tex, CLITERAL(Rectangle){0,0,20,20}, CLITERAL(Rectangle){300,150,100,100}, CLITERAL(Vector2){50,50}, angle / 2.0f, WHITE);
        DrawFPS(10, 10);
        angle += GetFrameTime() * 100;
        EndDrawing();
    }
    
}
