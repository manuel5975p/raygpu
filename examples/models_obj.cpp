#include <raygpu.h>
#include <string>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
Material churchMat{};
std::string resourceDirectoryPath;
Model churchModel;
Texture cdiffuse;
Mesh churchMesh;
float angle = 0;
Camera3D cam;
std::vector<Matrix> trfs;

constexpr size_t instanceCount = 10000;
constexpr size_t rootInstanceCount = 100;

void mainloop(){
    BeginDrawing();
    angle += GetFrameTime();
    cam.position = Vector3{std::sin(angle) * 45.f, 30.0f, std::cos(angle) * 45.f};
    cam.target = Vector3{0, 10.0f, 0};
    ClearBackground(BLANK);
    //BeginPipelineMode(pl);
    BeginMode3D(cam);
    DrawMeshInstanced(churchMesh, churchModel.materials[churchModel.meshMaterial[0]], trfs.data(), instanceCount);
    EndMode3D();
    //EndPipelineMode();
    DrawFPS(0, 0);
    DrawText(TextFormat("Drawing %llu triangles", (unsigned long long)(instanceCount * churchMesh.triangleCount)), 0, 100, 40, WHITE);
    EndDrawing();
}
int main(cwoid){
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);
    //SetConfigFlags(FLAG_VSYNC_HINT);
    
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920, 1080, "VAO");
    churchMat = LoadMaterialDefault();
    SetTargetFPS(0);
    cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 50.0f
    };
    
    

    resourceDirectoryPath = FindDirectory("resources", 3);
    churchModel = LoadModel((resourceDirectoryPath + "/church.obj").c_str());

    churchMat = LoadMaterialDefault();
    
    //cdiffuse = LoadTextureFromImage(LoadImage((resourceDirectoryPath + "/church_diffuse.png").c_str()));
    //churchMat.maps[MATERIAL_MAP_DIFFUSE].texture = cdiffuse;
    
    churchMesh = churchModel.meshes[0];
    //UploadMesh(&cube, true);

    //DescribedPipeline* pl = Relayout(DefaultPipeline(), churchMesh.vao);
    
    
    trfs = std::vector<Matrix>(instanceCount);
    for(int i = 0;i < rootInstanceCount;i++){
        for(int j = 0;j < rootInstanceCount;j++){
            trfs[i * rootInstanceCount + j] = MatrixTranslate(10.0f * i - rootInstanceCount * 5.0f, 0, 10.0f * j - rootInstanceCount * 5.0f) * MatrixScale(0.5,0.5,0.5);
        }
    }
    
    using mainloop_type = decltype(mainloop);
    auto mainloopCaller = [](void* x){
        (*((decltype(mainloop)*)x))();
    };
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
