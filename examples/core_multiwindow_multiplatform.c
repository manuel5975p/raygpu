#include <raygpu.h>

int main(){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Main Window");
    SubWindow sdl3Window = OpenSubWindow_SDL3(800, 600, "SDL3 SubWindow");
    SubWindow glfwWindow = OpenSubWindow_GLFW(800, 600, "GLFW SubWindow");
    SubWindow rgfwWindow = OpenSubWindow_RGFW(800, 600, "RGFW SubWindow");
    while(!WindowShouldClose()){
        BeginWindowMode(sdl3Window);
        ClearBackground(RAYWHITE);
        DrawText("SDL3 SubWindow", 50, 50, 30, BLACK);
        DrawCircle(GetMouseX(), GetMouseY(), 50, RED);
        EndWindowMode();

        BeginWindowMode(glfwWindow);
        ClearBackground(RAYWHITE);
        DrawText("GLFW SubWindow", 50, 50, 30, BLACK);
        DrawCircle(GetMouseX(), GetMouseY(), 50, RED);
        EndWindowMode();

        BeginWindowMode(rgfwWindow);
        ClearBackground(RAYWHITE);
        DrawText("RGFW SubWindow", 50, 50, 30, BLACK);
        DrawCircle(GetMouseX(), GetMouseY(), 50, RED);
        EndWindowMode();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Main Window", 50, 50, 30, BLACK);
        DrawCircle(GetMouseX(), GetMouseY(), 50, RED);
        EndDrawing();
    }
}
