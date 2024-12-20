#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int textWidth;
Texture tex;
void mainloop(void){
    BeginDrawing();
    ClearBackground((Color){130, 130, 130, 255});
    //UseNoTexture();
    for(int i = 0;i < 32;i++){
        DrawLine(0, i * 50, i * 50, 0, BLACK);
    }
    int wh = GetScreenWidth() / 2;
    int hh = GetScreenHeight() / 2;
    DrawText("Hello MSAA Enjoyer", wh - textWidth / 2, hh / 2, 50, WHITE);
    //DrawTexturePro(tex,(Rectangle){0,0,100,100}, (Rectangle){0,0,100,100},(Vector2){0,0},0, (Color){210, 210, 210,255});
    //for(int i = 0;i < 32;i++){
    //    DrawLine(0, i * 50, i * 50, 0, BLACK);
    //}
    //DrawTexturePro(tex,(Rectangle){0,0,100,100}, (Rectangle){100,0,100,100},(Vector2){0,0},0, (Color){210, 210, 210,255});
    DrawCircleSector(GetMousePosition(), 100.0f, 0.0f, 360.0f, 128, WHITE);
    DrawFPS(10, 10);
    EndDrawing();
}
int main(void){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(800, 600, "WebGPU window");
    textWidth = MeasureText("Hello MSAA Enjoyer", 50);
    tex = LoadTextureFromImage(GenImageChecker(RED, GREEN, 100, 100, 10));
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
