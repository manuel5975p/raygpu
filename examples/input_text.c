#include <raygpu.h>
#include <stdio.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
char buffer[100] = {0};

bool editable = true;

void mainloop(cwoid){
    BeginDrawing();
    GuiCheckBox((Rectangle){100,100,70,70}, "Editable", &editable);
    GuiTextBox((Rectangle){100,200,400,70}, buffer, 100, editable);
    //for(int i = 0;i < 10;i++)
    //    DrawText("48762938476293874598ljfasdhgfakjhfaskjdsgjksafdgjaskdjfdsagjkfasdghkajs48762938476293874598ljfasdhgfakjhfaskjdsgjksafdgjaskdjfdsagjkfasdghkajs48762938476293874598ljfasdhgfakjhfaskjdsgjksafdgjaskdjfdsagjkfasdghkajs48762938476293874598ljfasdhgfakjhfaskjdsgjksafdgjaskdjfdsagjkfasdghkajs", 0, 500 + 10*i, 5, WHITE);
    DrawFPS(5, 5);
    EndDrawing();
    //TRACELOG(LOG_WARNING, "%d", GetFPS());
}
int main(void){
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1000, 700, "Text input");
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else 
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}
