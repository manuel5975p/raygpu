#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

#include <raygpu.h>

int main(int argc, char* argv[]) {
    //auto spirv = glsl_to_spirv(LoadFileText("../resources/simple.vert"), LoadFileText("../resources/simple.frag"));
    //std::cerr << spirv[0] << ", " << spirv[1] << std::endl;
    SetTraceLogFile(stderr);
    InitWindow(800, 800, "Window");
    const char* resDir = FindDirectory("resources", 3);
    TRACELOG(LOG_INFO, resDir);
    //wgpu::ShaderModuleSPIRVDescriptor shaderCodeDesc{};
    //shaderCodeDesc.nextInChain = nullptr;
    //shaderCodeDesc.sType = wgpu::SType::ShaderSourceSPIRV;
    //shaderCodeDesc.code = (uint32_t*)spirv.data();
    //shaderCodeDesc.codeSize = spirv.size();
    //wgpu::ShaderModuleDescriptor shaderDesc{};
    //shaderDesc.nextInChain = &shaderCodeDesc;
    //WGPUStringView strv = STRVIEW("wtf");
    //wgpu::ShaderModule sh = GetCXXDevice().CreateShaderModule(&shaderDesc);
    //std::cout << sh.Get() << "\n";
    float vertexdata[9] = {
        0, 0, 0,
        1, 0, 0,
        0, 1, 0,
    };
    
    DescribedBuffer* vb = GenBuffer(vertexdata, sizeof(vertexdata));
    VertexArray* va = LoadVertexArray();
    VertexAttribPointer(va, vb, 0, WGPUVertexFormat_Float32x3, 0, WGPUVertexStepMode_Vertex);
    DescribedPipeline* pl = LoadPipelineGLSL(LoadFileText(TextFormat("%s/simple.vert", resDir)), LoadFileText(TextFormat("%s/simple.frag", resDir)));
    Image checkerimg = GenImageChecker(BLACK, WHITE, 20, 20, 20);
    Texture checkertex = LoadTextureFromImage(checkerimg);
    SetPipelineTexture(pl, 0, checkertex);
    SetPipelineSampler(pl, 1, LoadSampler(repeat, nearest));
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RED);
        BeginPipelineMode(pl);
        BindVertexArray(va);
        DrawArrays(WGPUPrimitiveTopology_TriangleList, 3);
        EndPipelineMode();
        EndDrawing();
    }
    // Finalize glslang process
    return 0;
}
