#include <raygpu.h>

int main(void){
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Primary Window");

    int secondWidth = 400, secondHeight = 400;
    SubWindow second = OpenSubWindow(secondWidth, secondHeight, "Secondary Window");
    SubWindow third = OpenSubWindow(secondWidth, secondHeight, "Third Window");
    
    while(!WindowShouldClose()){
        
        BeginWindowMode(second);
        ClearBackground(DARKBLUE);
        DrawCircle(200, 200, 50.0f, WHITE);
        EndWindowMode();
        
        BeginWindowMode(third);
        ClearBackground(RED);
        DrawCircle(200, 200, 50.0f, GREEN);
        EndWindowMode();
        
        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawCircle(200, 200, 50.0f, RED);
        EndDrawing();
    }
}
