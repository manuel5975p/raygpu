#include <raygpu.h>
#include <stdio.h>
#include <assert.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Camera3D cam;
Mesh cube;
DescribedPipeline* pl;
Texture checkers;
float angle;
void mainloop(void){
    BeginDrawing();
    angle += GetFrameTime();
    cam.position = (Vector3){sinf(angle) * 10.f, 5.0f, cosf(angle) * 10.f};
    ClearBackground(BLACK);
    //TODO: Swapping the next two causes a problem since the BindGroup is lazily updated only at BindPipeline
    //EDIT: It's not due to lazy update; DrawArrays and DrawArraysFixed did not check for a pending Bindgroup Update
    BeginPipelineMode(pl);
    UseTexture(checkers);
    BeginMode3D(cam);
    BindPipelineVertexArray(pl, cube.vao);
    DrawArraysIndexed(WGPUPrimitiveTopology_TriangleList, *cube.ibo, 36);
    EndMode3D();
    EndPipelineMode();
    DrawFPS(0, 0);
    if(IsKeyPressed(KEY_U)){
        ToggleFullscreen();
    }
    EndDrawing();
}
int main(cwoid){
    InitWindow(1200, 800, "VAO");
    
    cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 45.0f
    };
    cube = GenMeshCube(3.f,3.f,3.f);
    //assert(cube.ibo.buffer == 0);
    pl = Relayout(DefaultPipeline(), cube.vao);
    checkers = LoadTextureFromImage(GenImageChecker(RED, DARKBLUE, 100, 100, 4));
    angle = 0.0f;

    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
