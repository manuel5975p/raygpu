#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#if SUPPORT_VULKAN_BACKEND == 1
const char text[] = "Hello Vulkan enjoyer";
const char title[] = "Vulkan Window";
#elif SUPPORT_WGPU_BACKEND
const char text[] = "Hello WebGPU enjoyer";
const char title[] = "WebGPU Window";
#endif
SubWindow sdlwin;
void mainloop(void){
    BeginDrawing();
    //ClearBackground((Color){230, 230, 230, 0});
    ClearBackground(BLANK);
    int fontsize = GetScreenWidth() < GetScreenHeight() ? GetScreenWidth() : GetScreenHeight();
    fontsize /= 20;
    DrawText(text, GetScreenWidth() / 2 - MeasureText(text, fontsize) / 2, 300, fontsize, (Color){190, 190, 190,255});
    
    DrawFPS(5, 5);
    if(IsKeyPressed(KEY_U)){
        TRACELOG(LOG_INFO, "Key U pressed");
        ToggleFullscreen();
    }
    if(IsKeyPressed(KEY_E)){
        DisableDepthTest();
    }
    //drawCurrentBatch();
    
    EndDrawing();
    if((GetFrameCount() & 0x7ff) == 0){
        printf("FPS: %u\n", GetFPS());
    }
    
}
int main(void){
    //RequestAdapterType(SOFTWARE_RENDERER);
    InitWindow(800, 600, title);

    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
    
}
