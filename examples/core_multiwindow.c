#include <raygpu.h>

int main(void){
    const int screenWidth = 800;
    const int screenHeight = 600;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Primary Window");

    int secondWidth = 400, secondHeight = 400;
    SubWindow second = OpenSubWindow(secondWidth, secondHeight, "Secondary Window");
    SubWindow third = OpenSubWindow(secondWidth, secondHeight, "Third Window");
    RenderTexture rtex = LoadRenderTexture(800, 800);
    while(!WindowShouldClose()){
        
        //BeginRenderpass();
        //EndRenderpass();
        

        
        
        BeginWindowMode(third);
        ClearBackground(RED);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, GREEN);
        EndWindowMode();
        BeginWindowMode(second);
        BeginTextureMode(rtex);
        ClearBackground(GREEN);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, WHITE);
        EndTextureMode();
        DrawTexturePro(rtex.color, (Rectangle){0,0,800,800}, (Rectangle){300,300,100,100}, (Vector2){0,0}, 0.0f, WHITE);
        EndWindowMode();
        BeginDrawing();
        
        ClearBackground(DARKBLUE);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, RED);
        EndDrawing();
    }
}
