#include <raygpu.h>

int main(void){
    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Pipes");

    RenderSettings settings = GetDefaultSettings();
    settings.sampleCount = 4;
    DescribedPipeline* depthless = ClonePipelineWithSettings(DefaultPipeline(), settings);
    RenderTexture multisampled_rt = LoadRenderTextureEx(640, 720,WGPUTextureFormat_BGRA8Unorm, 4);
    
    while(!WindowShouldClose()){

        

        BeginDrawing();
        ClearBackground(BLANK);

        BeginTextureMode(multisampled_rt);
        ClearBackground(BLANK);
        BeginPipelineMode(depthless);
        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, WHITE);
        DrawTriangle(
            (Vector2){GetMousePosition().x, GetMousePosition().y + 100}, 
            (Vector2){GetMousePosition().x - 30, GetMousePosition().y + 300}, 
            (Vector2){GetMousePosition().x + 30, GetMousePosition().y + 300}, 
            GREEN
        );
        EndPipelineMode();
        EndTextureMode();


        DrawCircle(GetMouseX(), GetMouseY(), 50.0f, WHITE);
        DrawTriangle(
            (Vector2){GetMousePosition().x, GetMousePosition().y + 100}, 
            (Vector2){GetMousePosition().x - 30, GetMousePosition().y + 300}, 
            (Vector2){GetMousePosition().x + 30, GetMousePosition().y + 300}, 
            GREEN
        );
        DrawTexture(multisampled_rt.texture, 0, 0, WHITE);
        DrawRectangle(638, 0, 4, 720, RED);
        DrawText("MSAA On", 5, 5, 30, GREEN);
        DrawText("MSAA Off", 645, 5, 30, GREEN);
        DrawFPS(5, 70);
        EndDrawing();
    }
}
