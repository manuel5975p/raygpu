#include <raygpu.h>

int main(){
    InitWindow(800, 600, "Main Window");
    SubWindow sdl3Window = InitWindow_SDL3(800, 600, "SDL3 Window");
    SubWindow rgfwWindow = InitWindow_SDL3(800, 600, "RGFW Window");
    
    while(!WindowShouldClose()){
        BeginWindowMode(sdl3Window);
        ClearBackground(RAYWHITE);
        DrawText("SDL3 Window", 50, 50, 30, BLACK);
        EndWindowMode();

        BeginWindowMode(rgfwWindow);
        ClearBackground(RAYWHITE);
        DrawText("RGFW Window", 50, 50, 30, BLACK);
        EndWindowMode();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Main Window", 50, 50, 30, BLACK);
        EndDrawing();
    }

}
