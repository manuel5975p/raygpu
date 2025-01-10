#include <raygpu.h>

int main(void){
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(800, 600, "WebGPU window");
    
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){230, 230, 230,255});
        DrawText("Hello WebGPU enjoyer", 100, 300, 50, (Color){190, 190, 190,255});
        if(IsKeyPressed(KEY_U)){
            ToggleFullscreen();
        }
        EndDrawing();
        
    }
}
