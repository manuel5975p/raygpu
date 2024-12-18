#include <raygpu.h>
#include <string>
int main(cwoid){
    InitWindow(400, 300, "VAO");
    
    Camera3D cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };

    std::string resourceDirectoryPath = FindDirectory("resources", 3);
    Model churchModel = LoadModel((resourceDirectoryPath + "/church.obj").c_str());
    std::cout << churchModel.meshCount << std::endl;
    Mesh churchMesh = churchModel.meshes[0];
    //UploadMesh(&cube, true);

    DescribedPipeline* pl = Relayout(DefaultPipeline(), churchMesh.vao);
    DescribedPipeline* plorig = DefaultPipeline();
    //PreparePipeline(pl, cube.vao);
    WGPUBindGroupEntry camentry zeroinit;
    camentry.binding = 0;
    Matrix mat = GetCameraMatrix3D(cam, 1.5);
    camentry.buffer = GenUniformBuffer(&mat, sizeof(Matrix)).buffer;
    camentry.size = sizeof(Matrix);
    UpdateBindGroupEntry(&pl->bindGroup, 0, camentry);
    Texture checkers = LoadTextureFromImage(GenImageChecker(RED, DARKBLUE, 100, 100, 4));
    float angle = 0.0f;
    while(!WindowShouldClose()){
        BeginDrawing();
        angle += GetFrameTime();
        cam.position = Vector3{std::sin(angle) * 35.f, 20.0f, std::cos(angle) * 35.f};
        cam.target = Vector3{0, 10.0f, 0};
        ClearBackground(BLANK);
        
        
        //TODO: Swapping the next two causes a problem since the BindGroup is lazily updated only at BindPipeline
        //EDIT: It's not due to lazy update; DrawArrays and DrawArraysFixed did not check for a pending Bindgroup Update
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        UseNoTexture();
        BeginMode3D(cam);
        for(float x = -20;x <= 20;x += 4){
            DrawMesh(churchMesh, Material{}, MatrixTranslate(0,0,x) * MatrixScale(0.2f,0.2f,0.2f));
        }
        EndMode3D();
        EndPipelineMode();
        //BeginPipelineMode(plorig, WGPUPrimitiveTopology_TriangleList);
        DrawFPS(0, 0);

        //EndPipelineMode();
        EndDrawing();
    }
}
