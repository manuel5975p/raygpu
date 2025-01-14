#include <raygpu.h>
float angle = 0;
float zoom = 100;
void mainloop(void){
    uint64_t now = NanoTime();
    while(NanoTime() - now < 15000000);
    BeginDrawing();
    angle += RAD2DEG * GetGesturePinchAngle();
    zoom *= GetGesturePinchZoom();
    DrawRectanglePro((Rectangle){400, 400, zoom, zoom}, (Vector2){zoom / 2,zoom / 2}, angle, WHITE);
    for(int i = 0;i < GetTouchPointCount();i++){
        DrawCircleV(GetTouchPosition(i), 40.0f, WHITE);
    }
    EndDrawing();
}
int main(void){
    SetConfigFlags(FLAG_VSYNC_LOWLATENCY_HINT);
    InitWindow(800, 800, "Touch and Gestures Input");
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
