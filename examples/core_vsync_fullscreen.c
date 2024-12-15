#include <raygpu.h>
#define RAYGUI_IMPLEMENTATION

#include <raygui.h>

int main(void){
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(), GetMonitorHeight(), "WebGPU window");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    bool flag = 0;
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){130, 130, 130, 255});
        if(GuiButton(CLITERAL(Rectangle){300,300,200,60}, "Button")){
            flag = true;
        }
        if(flag)
            DrawText("The buttone got pressed", 300, 400, 50, CLITERAL(Color){0,255,0,255});
        DrawFPS(0, 0);
        int w = MeasureText("Hello Fullscreen Enjoyer", 100);
        DrawText("Hello Fullscreen Enjoyer", GetScreenWidth() / 2 - w / 2, GetScreenHeight() / 2 - 50, 100, (Color){210, 210, 210,255});
        EndDrawing();
    }
}
