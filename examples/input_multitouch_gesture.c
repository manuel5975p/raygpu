#include <raygpu.h>
float angle = 0;
float zoom = 100;
int main(void){
    InitWindow(800, 800, "Touch and Gestures Input");

    while(!WindowShouldClose()){
        BeginDrawing();
        angle += RAD2DEG * GetTouchRotate();
        zoom *= GetTouchZoom();
        DrawRectanglePro((Rectangle){400, 400, zoom, zoom}, (Vector2){zoom / 2,zoom / 2}, angle, WHITE);
        EndDrawing();
    }
}
