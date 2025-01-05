#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
VertexArray* vao;
DescribedBuffer vbo;
DescribedPipeline* pipeline;
void mainloop(cwoid){
    BeginDrawing();
    BeginPipelineMode(pipeline);
    BindVertexArray(pipeline, vao);
    DrawArrays(WGPUPrimitiveTopology_TriangleList, 3);
    EndPipelineMode();
    DrawFPS(0,0);
    EndDrawing();
}
int main(void){
    InitWindow(800, 600, "The Render Pipeline");
    const char* resourceDirectoryPath = FindDirectory("resources", 3);
    char* shaderSource = LoadFileText(TextFormat("%s/simple_shader.wgsl", resourceDirectoryPath));
    
    float vertices[6] = {
        0,0,
        1,0,
        0,1
    };
    vbo = GenBuffer(vertices, sizeof(vertices)); 
    vao = LoadVertexArray();
    VertexAttribPointer(vao, &vbo, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    EnableVertexAttribArray(vao, 0);
    pipeline = LoadPipeline(shaderSource);

    float uniformData[4] = {0,0,0,0};
    SetTargetFPS(0);
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
