#include <raygpu.h>
#include <msf_gif.h>

int main(){
    InitWindow(640, 480, "Gif Recording");
    int width = 640, height = 480, centisecondsPerFrame = 2, bitDepth = 16;
    MsfGifState gifState = {};
    msf_gif_bgra_flag = true; //optionally, set this flag if your pixels are in BGRA format instead of RGBA
    // msf_gif_alpha_threshold = 128; //optionally, enable transparency (see documentation in header for details)
    msf_gif_begin(&gifState, width, height);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawFPS(5, 5);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, RED);
        EndRenderpass();
        EndDrawing();
    }
    MsfGifResult result = msf_gif_end(&gifState);
    if (result.data) {
        FILE * fp = fopen("MyGif.gif", "wb");
        fwrite(result.data, result.dataSize, 1, fp);
        fclose(fp);
    }
}