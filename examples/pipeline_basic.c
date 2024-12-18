#include <raygpu.h>
#include <stdio.h>

int main(void){
    InitWindow(800, 600, "The Render Pipeline");
    const char* resourceDirectoryPath = FindDirectory("resources", 3);
    puts(resourceDirectoryPath); 
    char dirpath[1024] = {0};
    strcpy(dirpath, resourceDirectoryPath);
    strcat(dirpath, "/simple_shader.wgsl");
    char* shaderSource = LoadFileText(dirpath);
    AttributeAndResidence attributes[1] = {
        (AttributeAndResidence){
            .attr = (WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x2, 
                                          .offset = 0, 
                                          .shaderLocation = 0},
            
            .bufferSlot = 0, 
            .stepMode = WGPUVertexStepMode_Vertex
        }
    };
    UniformDescriptor uniforms[1] = {
        (UniformDescriptor){
            .minBindingSize = 16,
            .type = uniform_buffer
        }
    };
    RenderSettings settings = {0};
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    //settings.optionalDepthTexture = GetDepthTexture().view;
    DescribedPipeline* pipeline = LoadPipelineEx(shaderSource, attributes, 1,  uniforms, 1, settings);
    
    float vertices[6] = {
        0,0,
        1,0,
        0,1
    };
    DescribedBuffer vbo = GenBuffer(vertices, sizeof(vertices)); 
    
    VertexArray* vao = LoadVertexArray();
    VertexAttribPointer(vao, &vbo, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    EnableVertexAttribArray(vao, 0);
    float uniformData[4] = {0,0,0,0};
    SetTargetFPS(3000);
    while(!WindowShouldClose()){
        BeginDrawing();
        BeginPipelineMode(pipeline, WGPUPrimitiveTopology_TriangleList);
        SetUniformBufferData(0, uniformData, 16);
        BindVertexArray(pipeline, vao);
        DrawArrays(3);
        EndPipelineMode();
        DrawFPS(0,0);
        EndDrawing();
    }
}
