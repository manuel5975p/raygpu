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
#include <regex>
bool glslang_initialized = false;
std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char *vs, const char *fs) {

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
    program.link(messages);
    //if (!program.link(messages)) {
    //    std::cerr << "GLSL Linking Failed:\n" << program.getInfoLog() << std::endl;
    //}

    // Retrieve the intermediate representation
    //program.buildReflection();
    //int uniformCount = program.getNumUniformVariables();

    //std::cout << uniformCount << "\n";
    //glslang::TIntermediate *vertexIntermediate   = shader.getIntermediate();
    //glslang::TIntermediate *fragmentIntermediate = fragshader.getIntermediate();

    glslang::TIntermediate *vertexIntermediate = program.getIntermediate(EShLanguage(EShLangVertex));
    glslang::TIntermediate *fragmentIntermediate = program.getIntermediate(EShLanguage(EShLangFragment));
    if (!vertexIntermediate) {
        std::cerr << "Failed to get intermediate representation for vertex" << std::endl;
    }
    if (!fragmentIntermediate) {
        std::cerr << "Failed to get intermediate representation for fragment" << std::endl;
    }
    

    // Convert to SPIR-V
    std::vector<uint32_t> spirvV, spirvF;
    glslang::SpvOptions opt;
    opt.disableOptimizer = false;
    glslang::GlslangToSpv(*vertexIntermediate, spirvV, &opt);
    glslang::GlslangToSpv(*fragmentIntermediate, spirvF, &opt);

    return {spirvV, spirvF};
}
extern "C" DescribedPipeline* LoadPipelineGLSL(const char* vs, const char* fs){
    auto [spirvV, spirvF] = glsl_to_spirv(vs, fs);
    //std::cout.write((char*)spirv.data(), spirv.size() * sizeof(uint32_t));
    //std::cout.flush();
    //return nullptr;
    tint::Slice<uint32_t> slv(spirvV.data(), spirvV.size());
    tint::Slice<uint32_t> slF(spirvV.data(), spirvV.size());
    tint::wgsl::writer::ProgramOptions prgoptions{};
    prgoptions.allowed_features.extensions.insert(tint::wgsl::Extension::kClipDistances);
    //options.allowed_features.features;
    auto resultV = tint::spirv::reader::Read(spirvV);
    auto resultF = tint::spirv::reader::Read(spirvF);
    //std::cout << resultV << "\n";
    tint::ast::transform::Renamer ren;
    
    resultF.Symbols().Foreach([](tint::Symbol s){
        //std::cout << s.value() << "\n";
    });
    
    for(auto semnode : resultF.SemNodes().Objects()){
        //std::cout << semnode << "\n";
    }
    tint::ast::transform::DataMap imput{};
    tint::ast::transform::Renamer::Config dat;

    imput.Add(dat);
    tint::ast::transform::DataMap ouput{};
    ren.Apply(resultF, imput, ouput);
    auto wgsl_from_prog_resultV = tint::wgsl::writer::Generate(resultV, tint::wgsl::writer::Options{});
    auto wgsl_from_prog_resultF = tint::wgsl::writer::Generate(resultF, tint::wgsl::writer::Options{});
    //std::cout << spirvV.size() << "\n";
    //std::cout << resultV.Diagnostics() << "\n";
    //std::cout << wgsl_from_prog_resultF->wgsl << "\n";
    //std::exit(0);

    std::regex pattern("main");

    std::string replacementV = "vs_main";
    std::string replacementF = "fs_main";

    std::string sourceV = std::regex_replace(wgsl_from_prog_resultV->wgsl, pattern, replacementV);
    std::string sourceF = std::regex_replace(wgsl_from_prog_resultF->wgsl, pattern, replacementF);
    //std::cout << sourceF << "\n";
    //resultF.Foreach([](tint::Symbol s){
    //    std::cout << s.Name() << "\n";
    //});
    //auto parse = tint::spirv::reader::Parse(sl);
    //std::cout << result.Diagnostics() << "\n";
    //auto mod = LoadShaderModuleFromSPIRV(spirv.data(), spirv.size() * sizeof(uint32_t));
    //UniformDescriptor ud[2] = {
    //    UniformDescriptor{
    //        .type = uniform_type::texture2d,
    //        .minBindingSize = 0,
    //        .location = 0,
    //        .access = readonly,
    //        .fstype = sample_f32
    //    },
    //    UniformDescriptor{
    //        .type = uniform_type::sampler,
    //        .minBindingSize = 0,
    //        .location = 1,
    //        .access = readonly,  //ignore
    //        .fstype = sample_f32 //ignore
    //    }
    //};
    //AttributeAndResidence attr[1] = {
    //    AttributeAndResidence{
    //        .attr = WGPUVertexAttribute{.format = WGPUVertexFormat_Float32x3, .offset = 0, .shaderLocation = 0},
    //        .bufferSlot = 0,
    //        .stepMode = WGPUVertexStepMode_Vertex,
    //        .enabled = true
    //    }
    //};
    //LoadPipelineMod(mod, attr, 1, ud, 2, GetDefaultSettings());
    //LoadPipelineEx
    //}
    //return nullptr;
    std::cout << sourceV << "\n\n\n" << sourceF << "\n\n\n";

    std::string composed = sourceV + "\n\n" + sourceF;
    //std::cout << wgsl_from_prog_resultF->wgsl << "\n";
    return LoadPipeline(composed.c_str());
}
#else
//int main(){
//    return 0;
//}
#endif