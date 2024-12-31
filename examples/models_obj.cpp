#include <raygpu.h>
#include <string>

int main(cwoid){
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);
    SetConfigFlags(FLAG_VSYNC_HINT);
    
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920, 1080, "VAO");
    SetTargetFPS(0);
    Camera3D cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };
    

    std::string resourceDirectoryPath = FindDirectory("resources", 3);
    Model churchModel = LoadModel((resourceDirectoryPath + "/church.obj").c_str());
    Texture cdif = LoadTextureFromImage(LoadImage((resourceDirectoryPath + "/church_diffuse.png").c_str()));
    std::cout << churchModel.meshCount << std::endl;
    Mesh churchMesh = churchModel.meshes[0];
    //UploadMesh(&cube, true);

    DescribedPipeline* pl = Relayout(DefaultPipeline(), churchMesh.vao);
    
    Texture checkers = LoadTextureFromImage(GenImageChecker(RED, DARKBLUE, 100, 100, 4));
    float angle = 0.0f;
    constexpr size_t instanceCount = 2500;
    constexpr size_t rootInstanceCount = 50;
    Matrix trfs[instanceCount];
    for(int i = 0;i < rootInstanceCount;i++){
        for(int j = 0;j < rootInstanceCount;j++){
            trfs[i * rootInstanceCount + j] = MatrixTranslate(10.0f * i - rootInstanceCount * 5.0f, 0, 10.0f * j - rootInstanceCount * 5.0f) * MatrixScale(0.5,0.5,0.5);
        }
    }
    while(!WindowShouldClose()){
        BeginDrawing();
        angle += GetFrameTime();
        cam.position = Vector3{std::sin(angle) * 45.f, 30.0f, std::cos(angle) * 45.f};
        cam.target = Vector3{0, 10.0f, 0};
        ClearBackground(BLANK);
        //BeginPipelineMode(pl);
        UseTexture(cdif);
        BeginMode3D(cam);
        DrawMeshInstanced(churchMesh, Material{}, trfs, instanceCount);
        EndMode3D();
        //EndPipelineMode();
        DrawFPS(0, 0);
        DrawText(TextFormat("Drawing %llu triangles", (unsigned long long)(instanceCount * churchMesh.triangleCount)), 0, 100, 40, WHITE);
        EndDrawing();
    }
}
