#include <raygpu.h>
float angle = 0;
float zoom = 100;
void mainloop(void){
    BeginDrawing();
    angle += RAD2DEG * GetTouchRotate();
    zoom *= GetTouchZoom();
    DrawRectanglePro((Rectangle){400, 400, zoom, zoom}, (Vector2){zoom / 2,zoom / 2}, angle, WHITE);
    EndDrawing();
}
int main(void){
    InitWindow(800, 800, "Touch and Gestures Input");
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
