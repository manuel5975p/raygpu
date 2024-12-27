#include <raygpu.h>
#include <string>
#include <iostream>

Model mod;
int main(){
    InitWindow(1200, 800, "glTF Model Loading");
    std::string resourceDirectoryPath = FindDirectory("resources", 3);
    Model carmodel = LoadModel((resourceDirectoryPath + "/old_car_new.glb").c_str());
    Model churchModel = LoadModel((resourceDirectoryPath + "/church.obj").c_str());
    Mesh churchMesh = churchModel.meshes[0];
    Camera3D cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){10,20,25},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };
    UploadMesh(&carmodel.meshes[0], false);
    UploadMesh(&carmodel.meshes[1], false);
    Mesh carmesh = carmodel.meshes[0];
    Texture checker = LoadTextureFromImage(GenImageChecker(WHITE, BLACK, 100, 100, 10));
    Texture card = LoadTextureFromImage(LoadImage((resourceDirectoryPath + "/old_car_d.png").c_str()));

    std::cout << carmodel.materialCount << "\n";
    DescribedPipeline* pl = Relayout(DefaultPipeline(), carmesh.vao);
    SetPipelineTexture(pl, 1, card);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        DrawMesh(carmesh, Material{}, MatrixScale(1, 1, 1));
        EndPipelineMode();
        EndMode3D();
        DrawFPS(5, 5);
        EndDrawing();
    }
}