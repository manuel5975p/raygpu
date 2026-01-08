// Test for SetWindowTitle and SetSubWindowTitle functions

#include <raygpu.h>
#include <stdio.h>

void DrawToWindow(SubWindow window, const char* text, int windowWidth, Color bgColor, Color textColor) {
    if (window == NULL) {
        BeginDrawing();
    } else {
        BeginWindowMode(window);
    }
    
    ClearBackground(bgColor);
    int textWidth = MeasureText(text, 30);
    DrawText(text, (windowWidth - textWidth) / 2, 180, 30, textColor);
    
    if (window == NULL) {
        EndDrawing();
    } else {
        EndWindowMode();
    }
}

void UpdateWindowTitle(SubWindow window, const char* libraryName, int seconds) {
    char titleBuffer[256];
    snprintf(titleBuffer, sizeof(titleBuffer), "%s - %d", libraryName, seconds);
    
    if (window == NULL) {
        SetWindowTitle(titleBuffer);
    } else {
        SetSubWindowTitle(window, titleBuffer);
    }
}

int main(void) {
    // Initialize main window with resizable flag
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Main Window");
    
    // Create platform-specific sub-windows
    SubWindow glfwWindow = OpenSubWindow_GLFW(600, 400, "GLFW");
    SubWindow rgfwWindow = OpenSubWindow_RGFW(600, 400, "RGFW");
    SubWindow sdl3Window = OpenSubWindow_SDL3(600, 400, "SDL3");
    
    double lastUpdateTime = 0.0;
    
    while (!WindowShouldClose()) {
        double currentTime = GetTime();
        
        // Update titles every second
        if (currentTime - lastUpdateTime >= 1.0) {
            lastUpdateTime = currentTime;
            int seconds = (int)currentTime;
            
            // Update all window titles
            UpdateWindowTitle(NULL, "Main", seconds);
            UpdateWindowTitle(glfwWindow, "GLFW", seconds);
            UpdateWindowTitle(rgfwWindow, "RGFW", seconds);
            UpdateWindowTitle(sdl3Window, "SDL3", seconds);
        }
        
        // Draw on all windows
        DrawToWindow(NULL, "Main Window", 800, RAYWHITE, DARKGRAY);
        DrawToWindow(glfwWindow, "GLFW Window", 600, SKYBLUE, DARKBLUE);
        DrawToWindow(rgfwWindow, "RGFW Window", 600, LIME, DARKGREEN);
        DrawToWindow(sdl3Window, "SDL3 Window", 600, ORANGE, RED);
    }
    
    return 0;
}
