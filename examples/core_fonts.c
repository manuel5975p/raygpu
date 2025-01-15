#include <raygpu.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
Font tele;
void mainloop(void){
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextEx(tele, "Custom Text", (Vector2){10,120}, 50, 1.0f, WHITE);
    DrawFPS(10, 10);
    EndDrawing();
}
int main(void){
    InitWindow(800, 600, "Fonts Example");

    int dsize = 0;
    int deflsize = 0;

    //unsigned char* tgrd = DecodeDataBase64((uint8_t*)telegrama_render, &dsize);
    //unsigned char* decomp = DecompressData(tgrd, dsize, &deflsize);
    //tele = LoadFontFromMemory(".ttf", decomp, deflsize, 200, 0, 1000);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
    return 0;
}
