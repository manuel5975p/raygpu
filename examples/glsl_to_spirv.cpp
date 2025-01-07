#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

#include <raygpu.h>
// Function to read shader code from a file
std::vector<uint32_t> glsl_to_spirv(const char *vs, const char *fs);

int main(int argc, char* argv[]) {
    //auto spirv = glsl_to_spirv(LoadFileText("../resources/simple.vert"), LoadFileText("../resources/simple.frag"));
    //std::cerr << spirv[0] << ", " << spirv[1] << std::endl;
    SetTraceLogFile(stderr);
    InitWindow(800, 800, "Window");
    
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
    DescribedPipeline* pl = LoadPipelineGLSL(LoadFileText("../../resources/simple.vert"), LoadFileText("../../resources/simple.frag"));
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();
    }
    // Finalize glslang process
    return 0;
}
