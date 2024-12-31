#include <raygpu.h>
#include <string>
#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Model mod;
float angle = 0;
Mesh carmesh    ;
Texture checker ;
Texture card    ;
Model carmodel;
Camera3D cam;
DescribedPipeline* pl;
void mainloop(){
    
    BeginDrawing();
    ClearBackground(DARKBROWN);
    //DrawCircle(GetMouseX(), GetMouseY(), 100.0f, WHITE);
    BeginMode3D(cam);
    //UseNoTexture();
    //BeginPipelineMode(pl);
    UseTexture(card);
    //if(angle > -0.1)
    DrawMesh(carmesh, Material{}, MatrixRotate(Vector3{0, 1, 0}, angle));
    //EndPipelineMode();
    EndMode3D();
    Matrix mat = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    //SetUniformBufferData(0, &mat, 64);
    //UseNoTexture();
    //UpdateBindGroup(&GetActivePipeline()->bindGroup);
    //std::cout << vboptr - vboptr_base << "\n";
    //DrawCircle(GetMouseX() + 100, GetMouseY(), 50.0f, WHITE);
    //DrawCircle(GetMouseX() + 200, GetMouseY(), 50.0f, WHITE);
    EndRenderpass();
    BeginRenderpass();
    DrawCircle(GetMouseX(), GetMouseY(), 50.0f, WHITE);
    DrawFPS(5, 5);
    if(IsKeyPressed(KEY_U)){
        ToggleFullscreen();
    }
    EndDrawing();
    angle -= 1.f;
}
int main(){
    TRACELOG(LOG_INFO, "Hello");
    const char* ptr = FindDirectory("resources", 3);
    std::string resourceDirectoryPath = ptr ? ptr : "";
    TRACELOG(LOG_INFO, "Directory path: %s", resourceDirectoryPath.c_str());

    InitWindow(1200, 800, "glTF Model Loading");
    SetTargetFPS(0);
    
    //return 0;
    carmodel = LoadModel((resourceDirectoryPath + "/old_car_new.glb").c_str());
    cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){10,20,25},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };
    UploadMesh(&carmodel.meshes[0], false);
    UploadMesh(&carmodel.meshes[1], false);
    carmesh    = carmodel.meshes[0];
    checker = LoadTextureFromImage(GenImageChecker(WHITE, BLACK, 100, 100, 10));
    card    = LoadTextureFromImage(LoadImage((resourceDirectoryPath + "/old_car_d.png").c_str()));

    //pl = Relayout(DefaultPipeline(), carmesh.vao);
    //SetPipelineTexture(pl, 1, card);
    ClearBackground(BLUE);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}