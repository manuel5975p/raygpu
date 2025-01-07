#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
// Include glslang headers
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <raygpu.h>
// Function to read shader code from a file
std::string readShaderCode(const std::string& filepath) {
    std::ifstream file(filepath);
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

int main(int argc, char* argv[]) {
    std::vector<uint32_t> spirv;
    // Initialize glslang process
    glslang::InitializeProcess();

    // Path to the GLSL shader
    std::string shaderPath = "../resources/simple.vert";
    std::string fragPath = "../resources/simple.frag";

    {
        // Read GLSL shader code
        std::string shaderCode = readShaderCode(shaderPath);
        std::string fragCode = readShaderCode(fragPath);

        // Create a shader object
        glslang::TShader shader(EShLangVertex);
        glslang::TShader fragshader(EShLangFragment);
        
        const char* shaderStrings[2];
        shaderStrings[0] = shaderCode.c_str();
        shaderStrings[1] = fragCode.c_str();
        shader.setStrings(shaderStrings, 1);
        fragshader.setStrings(shaderStrings + 1, 1);

        // Set shader version and other options
        shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, 450);
        shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        
        fragshader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, 450);
        fragshader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        fragshader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        // Define resources (required for some shaders)
        TBuiltInResource Resources = {};
        // Initialize Resources with default values or customize as needed
        // For simplicity, we'll use the default initialization here

        EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

        // Parse the shader
        if (!shader.parse(&Resources, 450, ECoreProfile, false, false, messages)) {
            std::cerr << "GLSL Parsing Failed:\n" << shader.getInfoLog() << std::endl;
            return -1;
        }
        if (!fragshader.parse(&Resources, 450, ECoreProfile, false, false, messages)) {
            std::cerr << "GLSL Parsing Failed:\n" << shader.getInfoLog() << std::endl;
            return -1;
        }

        // Link the program
        glslang::TProgram program;
        program.addShader(&shader);
        program.addShader(&fragshader);

        if (!program.link(messages)) {
            std::cerr << "GLSL Linking Failed:\n" << program.getInfoLog() << std::endl;
            return -1;
        }

        // Retrieve the intermediate representation
        glslang::TIntermediate* intermediate = program.getIntermediate(EShLangVertex);
        if (!intermediate) {
            std::cerr << "Failed to get intermediate representation." << std::endl;
            return -1;
        }

        // Convert to SPIR-V
        
        glslang::GlslangToSpv(*intermediate, spirv);

        // Write SPIR-V to a binary file
        std::ofstream spirvFile("simple.spv", std::ios::binary);
        spirvFile.write(reinterpret_cast<const char*>(spirv.data()), spirv.size() * sizeof(uint32_t));
        spirvFile.close();

        std::cout << "SPIR-V binary written to simple.spv" << std::endl;

    }
    InitWindow(800, 800, "Window");
    
    wgpu::ShaderModuleSPIRVDescriptor shaderCodeDesc{};
    
    shaderCodeDesc.nextInChain = nullptr;
    shaderCodeDesc.sType = wgpu::SType::ShaderSourceSPIRV;
    shaderCodeDesc.code = (uint32_t*)spirv.data();
    shaderCodeDesc.codeSize = spirv.size();
    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc;
    WGPUStringView strv = STRVIEW("wtf");
    wgpu::ShaderModule sh = GetCXXDevice().CreateShaderModule(&shaderDesc);
    std::cout << sh.Get() << "\n";
    while(!WindowShouldClose()){
        BeginDrawing();
        EndDrawing();
    }
    // Finalize glslang process
    glslang::FinalizeProcess();
    return 0;
}
