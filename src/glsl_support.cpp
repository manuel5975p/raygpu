#if SUPPORT_GLSL_PARSER == 1
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#include <raygpu.h>
#include <tint/tint.h>
#include <tint/lang/core/ir/module.h>
#include <tint/lang/spirv/reader/parser/parser.h>
#include <tint/lang/spirv/reader/reader.h>
#include <tint/lang/wgsl/reader/reader.h>
#include <tint/lang/wgsl/writer/writer.h>
#include <tint/lang/glsl/writer/writer.h>
#include <tint/lang/core/type/reference.h>

bool glslang_initialized = false;
std::vector<uint32_t> glsl_to_spirv(const char *vs, const char *fs) {

    if (!glslang_initialized)
        glslang::InitializeProcess();
    

    glslang::TShader shader(EShLangVertex);
    glslang::TShader fragshader(EShLangFragment);
    const char *shaderStrings[2];
    shaderStrings[0] = vs;//shaderCode.c_str();
    shaderStrings[1] = fs;//fragCode.c_str();
    shader.setStrings(shaderStrings, 1);
    fragshader.setStrings(shaderStrings + 1, 1);
    int ver = 430;
    // Set shader version and other options
    shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, ver);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);

    fragshader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, ver);
    fragshader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    fragshader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    // Define resources (required for some shaders)
    TBuiltInResource Resources = {};
    // Initialize Resources with default values or customize as needed
    // For simplicity, we'll use the default initialization here

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    // Parse the shader
    shader.setAutoMapLocations(false);
    if (!shader.parse(&Resources, ver, ECoreProfile, false, false, messages)) {
        TRACELOG(LOG_ERROR, "Vertex GLSL Parsing Failed: %s", shader.getInfoLog());
    }
    fragshader.setAutoMapLocations(false);
    if (!fragshader.parse(&Resources, ver, ECoreProfile, false, false, messages)) {
        TRACELOG(LOG_ERROR, "Fragment GLSL Parsing Failed: %s", fragshader.getInfoLog());
    }

    // Link the program
    glslang::TProgram program;
    program.addShader(&shader);
    program.addShader(&fragshader);

    if (!program.link(messages)) {
        std::cerr << "GLSL Linking Failed:\n" << program.getInfoLog() << std::endl;
    }

    // Retrieve the intermediate representation
    program.buildReflection();
    int uniformCount = program.getNumUniformVariables();

    //std::cout << uniformCount << "\n";

    glslang::TIntermediate *intermediate = program.getIntermediate(EShLangFragment);
    if (!intermediate) {
        std::cerr << "Failed to get intermediate representation." << std::endl;
    }
    

    // Convert to SPIR-V
    std::vector<uint32_t> spirv;
    glslang::SpvOptions opt;
    opt.disableOptimizer = false;
    glslang::GlslangToSpv(*intermediate, spirv, &opt);

    return spirv;
}
extern "C" DescribedPipeline* LoadPipelineGLSL(const char* vs, const char* fs){
    std::vector<uint32_t> spirv = glsl_to_spirv(vs, fs);
    //std::cout.write((char*)spirv.data(), spirv.size() * sizeof(uint32_t));
    //std::cout.flush();
    //return nullptr;
    tint::Slice<uint32_t> sl(spirv.data(), spirv.size());
    tint::spirv::reader::Options options{};
    auto result = tint::spirv::reader::ReadIR(spirv);
    //auto parse = tint::spirv::reader::Parse(sl);
    //std::cout << result.Diagnostics() << "\n";
    wgpu::ShaderModuleSPIRVDescriptor shaderCodeDesc{};
    shaderCodeDesc.nextInChain = nullptr;
    shaderCodeDesc.sType = wgpu::SType::ShaderSourceSPIRV;
    shaderCodeDesc.code = (uint32_t*)spirv.data();
    shaderCodeDesc.codeSize = spirv.size();
    wgpu::ShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc;
    WGPUStringView strv = STRVIEW("wtf");
    wgpu::ShaderModule sh = GetCXXDevice().CreateShaderModule(&shaderDesc);
    
    auto wgslres = tint::wgsl::writer::WgslFromIR(result.Get(), tint::wgsl::writer::ProgramOptions{});
    std::cerr << wgslres.Get().wgsl << "\n";
    //for(size_t i = 0;i < result->functions.Length();i++){
    //    std::cout << (int)result->functions[i]->IsEntryPoint() << "\n";
    //}
    return nullptr;
}
#else
//int main(){
//    return 0;
//}
#endif