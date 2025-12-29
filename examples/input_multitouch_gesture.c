#include <raygpu.h>
#ifndef _WIN64
float angle = 0;
float zoom = 100;
void mainloop(void){
    uint64_t now = NanoTime();
    while(NanoTime() - now < 15000000);
    BeginDrawing();
    ClearBackground(BLACK);
    //angle += RAD2DEG * GetGesturePinchAngle();
    //zoom *= GetGesturePinchZoom();
    //DrawRectanglePro((Rectangle){400, 400, zoom, zoom}, (Vector2){zoom / 2,zoom / 2}, angle, WHITE);
    for(int i = 0;i < GetTouchPointCount();i++){
        DrawCircleV(GetTouchPosition(i), 40.0f, WHITE);
    }
    DrawLine(GetMouseX(), GetMouseY() - 20, GetMouseX(), GetMouseY() - 5, GREEN);
    DrawLine(GetMouseX(), GetMouseY() + 5, GetMouseX(), GetMouseY() + 20, GREEN);
    DrawLine(GetMouseX() - 20, GetMouseY(), GetMouseX() - 5, GetMouseY(), GREEN);
    DrawLine(GetMouseX() + 5, GetMouseY(), GetMouseX() + 20, GetMouseY(), GREEN);

    EndDrawing();
}
int main(void){
    SetConfigFlags(FLAG_VSYNC_LOWLATENCY_HINT);
    ProgramInfo program = {
        .windowTitle = "Touch Gestures",
        .windowWidth = 800, 
        .windowHeight = 800,
        .renderFunction = mainloop,
    };
    InitProgram(program);
}
#else
int main(void) { return 0; }
#endif
