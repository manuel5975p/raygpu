#include <raygpu.h>
#include <stdio.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
char buffer[100] = {0};

bool editable = true;

void mainloop(cwoid){
    BeginDrawing();
    GuiCheckBox((Rectangle){20,100,70,70}, "Editable", &editable);
    GuiTextBox((Rectangle){100,100,400,70}, buffer, 100, true);

    DrawFPS(5, 5);
    EndDrawing();
}
int main(void){
    InitWindow(1000, 700, "Text input");
    
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else 
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}
