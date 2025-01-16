#include <raygpu.h>

int main(void){
    InitWindow(1280, 720, "Pipes");

    RenderSettings settings = GetDefaultSettings();
    settings.depthTest = false;
    DescribedPipeline* depthless = ClonePipelineWithSettings(DefaultPipeline(), settings);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGREEN);
        BeginPipelineMode(depthless);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, WHITE);
        EndPipelineMode();
        EndDrawing();
    }
}
