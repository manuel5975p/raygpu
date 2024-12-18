#include <raygpu.h>
#include <stdio.h>
int main(cwoid){
    InitWindow(1200, 800, "VAO");
    AttributeAndResidence attributes[4] = {
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x2, 
                                          .offset = 0, 
                                          .shaderLocation = 0},
            
            .bufferSlot = 0, 
            .stepMode = WGPUVertexStepMode_Vertex
        },
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x3, 
                                          .offset = 0, 
                                          .shaderLocation = 1},
            
            .bufferSlot = 1,
            .stepMode = WGPUVertexStepMode_Vertex
        },
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x3, 
                                          .offset = 0, 
                                          .shaderLocation = 2},
            
            .bufferSlot = 2,
            .stepMode = WGPUVertexStepMode_Vertex
        },
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x4, 
                                          .offset = 0, 
                                          .shaderLocation = 3},
            
            .bufferSlot = 3,
            .stepMode = WGPUVertexStepMode_Vertex
        }
    };
    
    Camera3D cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };
    Mesh cube = GenMeshCube(3.f,3.f,3.f);
    //UploadMesh(&cube, true);

    DescribedPipeline* plorig = ClonePipeline(DefaultPipeline());
    DescribedPipeline* pl = DefaultPipeline();
    PreparePipeline(pl, cube.vao);
    Texture checkers = LoadTextureFromImage(GenImageChecker(RED, DARKBLUE, 100, 100, 4));
    float angle = 0.0f;
    while(!WindowShouldClose()){
        BeginDrawing();
        angle += GetFrameTime();
        cam.position = (Vector3){sinf(angle) * 10.f, 5.0f, cosf(angle) * 10.f};
        ClearBackground(BLACK);
        UseTexture(checkers);
        //TODO: Swapping those causes a problem since the BindGroup is lazily updated only at BindPipeline
        BeginMode3D(cam);
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        BindVertexArray(pl, cube.vao);
        DrawArraysIndexed(cube.ibo, 36);
        EndMode3D();
        EndPipelineMode();
        BeginPipelineMode(plorig, WGPUPrimitiveTopology_TriangleList);
        DrawFPS(0, 0);
        EndPipelineMode();
        EndDrawing();
    }
}
