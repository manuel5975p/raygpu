#include <raygpu.h>
#define RAYGUI_IMPLEMENTATION
#include <external/raygui.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
Texture tex;
Texture texSrgb;
Color rgbavalues{20,160,110,255};
void mainloop(){
    unsigned char data[400] = {};
    for(int i = 0;i < 100;i++){
        reinterpret_cast<Color*>(data)[i] = rgbavalues;
    }
    UpdateTexture(tex, data);
    UpdateTexture(texSrgb, data);
    BeginDrawing();
    
    ClearBackground(BLACK);
    DrawTexturePro(tex    , CLITERAL(Rectangle){0,0,10,10}, CLITERAL(Rectangle){0,300,1000,200}, CLITERAL(Vector2){0,0}, 0, WHITE);
    DrawTexturePro(texSrgb, CLITERAL(Rectangle){0,0,10,10}, CLITERAL(Rectangle){0,500,1000,200}, CLITERAL(Vector2){0,0}, 0, WHITE);
    float r = rgbavalues.r,g = rgbavalues.g,b = rgbavalues.b;
    GuiSlider(Rectangle{100,10,300,50}, "0", "255",  &r, 0, 255);
    GuiSlider(Rectangle{100,60,300,50}, "0", "255",  &g, 0, 255);
    GuiSlider(Rectangle{100,110,300,50}, "0", "255",  &b, 0, 255);
    DrawText("VK_FORMAT_R8G8B8A8_UNORM", 10, 450, 50, WHITE);
    DrawText("VK_FORMAT_R8G8B8A8_SRGB", 10, 650, 50, WHITE);
    rgbavalues.r = r;
    rgbavalues.g = g;
    rgbavalues.b = b;
    EndDrawing();
}
int main(cwoid){
    InitWindow(1000, 1000, "Textures");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    tex = LoadTextureEx(10, 10, RGBA8, false);
    texSrgb = LoadTextureEx(10, 10, RGBA8_Srgb, false);
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}