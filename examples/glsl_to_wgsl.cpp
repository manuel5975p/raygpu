/*
 * MIT License
 * 
 * Copyright (c) 2025 @manuel5975p
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


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
