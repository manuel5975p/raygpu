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





#if SUPPORT_GLSL_PARSER == 1
//#include <SPIRV/GlslangToSpv.h>
#include <external/gl_corearb.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_reflect.h>
#include <raygpu.h>
#include <memory>
#include <fstream>
#ifdef GLSL_TO_WGSL
#include <tint/tint.h>
#include <tint/lang/core/ir/module.h>
#include <tint/lang/spirv/reader/parser/parser.h>
#include <tint/lang/spirv/reader/reader.h>
#include <tint/lang/wgsl/reader/reader.h>
#include <tint/lang/wgsl/writer/writer.h>
#include <tint/lang/glsl/writer/writer.h>
#include <tint/lang/core/type/reference.h>
#endif
#include <regex>
extern std::unordered_map<uint32_t, std::string> uniformTypeNames;
bool glslang_initialized = false;
std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char *vs, const char *fs) {

    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }

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

    fragshader.setEnvInput(glslang::EShSourceGlsl, EShLangFragment, glslang::EShClientOpenGL, ver);
    fragshader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    fragshader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    // Define resources (required for some shaders)
    TBuiltInResource Resources = {};

    Resources.limits.generalUniformIndexing = true;
    Resources.limits.generalVariableIndexing = true;
    Resources.maxDrawBuffers = true;

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
std::vector<uint32_t> glsl_to_spirv(const char *cs){
    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }
    glslang::TShader shader(EShLangCompute);
    shader.setEnvInput(glslang::EShSourceGlsl, EShLangCompute, glslang::EShClientOpenGL, 450);
    shader.setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
    shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    TBuiltInResource Resources = {};
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 1024;
    Resources.maxCombinedTextureImageUnits = 8;

    Resources.limits.generalUniformIndexing = true;
    Resources.limits.generalVariableIndexing = true;
    Resources.maxDrawBuffers = true;

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    if(!shader.parse(&Resources,  450, ECoreProfile, false, false, messages)){
        
        TRACELOG(LOG_ERROR, "Compute GLSL Parsing Failed: %s", shader.getInfoLog());
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if(!program.link(messages)){
        TRACELOG(LOG_ERROR, "Error linkin shader: %s", program.getInfoLog());
    }
    glslang::TIntermediate* intermediate = program.getIntermediate(EShLangCompute);
    std::vector<uint32_t> output;
    glslang::GlslangToSpv(*intermediate, output);
    return output;
}
std::unordered_map<std::string, ResourceTypeDescriptor> getBindingsGLSL(ShaderSources sources){
    const int glslVersion = 450;
    
    
    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }
    
    if(sources.vertexAndFragmentSource){
        TRACELOG(LOG_ERROR, "VS and FS in one not supported for GLSL");
    }
    
    std::vector<std::pair<EShLanguage, std::unique_ptr<glslang::TShader>>> shaders;
    std::vector<const char*> stageSources;
    if(sources.vertexSource && sources.fragmentSource){
        shaders.push_back({EShLangVertex,   std::make_unique<glslang::TShader>(EShLangVertex)});
        shaders.push_back({EShLangFragment, std::make_unique<glslang::TShader>(EShLangFragment)});
        stageSources.push_back(sources.vertexSource);
        stageSources.push_back(sources.fragmentSource);
        shaders[0].second->setStrings(stageSources.data(), 1);
        shaders[1].second->setStrings(stageSources.data() + 1, 1);
        shaders[0].second->setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL, glslVersion);
        shaders[0].second->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shaders[0].second->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        shaders[1].second->setEnvInput(glslang::EShSourceGlsl, EShLangFragment, glslang::EShClientOpenGL, glslVersion);
        shaders[1].second->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shaders[1].second->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
    }
    else if(sources.computeSource){
        shaders.push_back({EShLangCompute, std::make_unique<glslang::TShader>(EShLangCompute)});
        stageSources.push_back(sources.computeSource);
        shaders[0].second->setStrings(stageSources.data(), 1);
        shaders[0].second->setEnvInput(glslang::EShSourceGlsl, EShLangCompute, glslang::EShClientOpenGL, glslVersion);
        shaders[0].second->setEnvClient(glslang::EShClientOpenGL, glslang::EShTargetOpenGL_450);
        shaders[0].second->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_3);
        
    }

    
    

    TBuiltInResource Resources = {};
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 1024;
    Resources.maxCombinedTextureImageUnits = 8;

    Resources.limits.generalUniformIndexing = true;
    Resources.limits.generalVariableIndexing = true;
    Resources.maxDrawBuffers = true;

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    // Parse the shader
    
    for(size_t i = 0;i < shaders.size();i++){
        auto& [language, shader] = shaders[i];

        shader->setAutoMapLocations(false);
        shader->setAutoMapBindings (false);
        if(!shader->parse(&Resources,  glslVersion, ECoreProfile, false, false, messages)){
            const char* lang = (language == EShLangCompute ? "Compute" : ((language == EShLangVertex) ? "Vertex" : "Fragment"));
            TRACELOG(LOG_ERROR, "%s GLSL Parsing Failed: %s", lang, shader->getInfoLog());
        }
    }

    // Link the program
    glslang::TProgram program;
    for(size_t i = 0;i < shaders.size();i++){
        auto& [language, shader] = shaders[i];
        program.addShader(shader.get());
    }
    if(!program.link(messages)){
        TRACELOG(LOG_WARNING, "Linking program failed: %s", program.getInfoDebugLog());
    }
    else{
        TRACELOG(LOG_INFO, "Program linked successfully");
    }
    auto spvdsToResourceType = [](SpvReflectDescriptorType spvType){
        switch(spvType){
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                return texture2d;
            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                return storage_texture2d;
            break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                return texture_sampler;
            break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            default:
                assert(false && "Unsupported");
                __builtin_unreachable();
        }
    };

    std::unordered_map<std::string, ResourceTypeDescriptor> ret;
    for(const auto& [lang, shader] : shaders){
        glslang::TIntermediate* intermediate = program.getIntermediate(lang);
        std::vector<uint32_t> stageSpirv;
        //glslang::SpvOptions options{.generateDebugInfo = true};
        glslang::GlslangToSpv(*intermediate, stageSpirv);
        spv_reflect::ShaderModule mod(stageSpirv);
        uint32_t count = 0;
        SpvReflectResult result = spvReflectEnumerateDescriptorSets(&mod.GetShaderModule(), &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);
      
        std::vector<SpvReflectDescriptorSet*> sets(count);
        result = spvReflectEnumerateDescriptorSets(&mod.GetShaderModule(), &count, sets.data());
        std::vector<SpvReflectInterfaceVariable*> vars;
        uint32_t varcount;
        spvReflectEnumerateInputVariables(&mod.GetShaderModule(), &varcount, nullptr);
        vars.resize(varcount);
        spvReflectEnumerateInputVariables(&mod.GetShaderModule(), &varcount, vars.data());

        for(auto set : sets){
        
            for(uint32_t i = 0;i < set->binding_count;i++){
                auto binding = set->bindings[i];
                //std::cout << set->bindings[i]->name << ": " <<  set->bindings[i]->descriptor_type << "\n";
                if(set->bindings[i]->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER || set->bindings[i]->descriptor_type == SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER){

                }
                else{
                    ResourceTypeDescriptor insert zeroinit;
                    insert.fstype = we_dont_know;
                    insert.type = spvdsToResourceType(set->bindings[i]->descriptor_type);
                    insert.location = set->bindings[i]->binding;
                    ret[set->bindings[i]->name] = insert;
                }
                //std::cout << set->bindings[i]->block. << ": " <<  set->bindings[i]->descriptor_type << "\n";
                
            }
        }
        //std::cout << std::endl;
    }
    program.buildReflection();
    for(uint32_t i = 0;i < program.getNumUniformBlocks();i++){
        std::string name = program.getUniformBlockName(i);
        ResourceTypeDescriptor insert zeroinit;
        //std::cout << program.getUniformBlock(i).getBinding() << ": " << program.getUniformBlockName(i) << " readonly: " << program.getUniformBlock(i).getType()->getStorageQualifierString() << "\n";
        //std::cout << program.getUniformBlock(i).getBinding() << ": " << program.getUniformBlockName(i) << "\n";
        //std::cout << program.getUniformBlockName(i) << ": " << program.getUniformBlock(i).size << "\n";
        insert.location = program.getUniformBlock(i).getBinding();
        insert.minBindingSize = program.getUniformBlock(i).size;
        insert.access = program.getUniformBlock(i).getType()->getQualifier().isWriteOnly() ? writeonly : (program.getUniformBlock(i).getType()->getQualifier().isReadOnly() ? readonly : readwrite);
        
        std::string storageOrUniform = program.getUniformBlock(i).getType()->getStorageQualifierString();
        bool uniform = storageOrUniform.find("uniform") != std::string::npos;
        insert.type = uniform ? uniform_buffer : storage_buffer;
        ret[program.getUniformBlockName(i)] = insert;
    }
//
    //for(uint32_t i = 0;i < program.getNumUniformVariables();i++){
    //    if(program.getUniform(i).getBinding() != -1){
    //        ResourceTypeDescriptor insert zeroinit;
    //        
    //        insert.location = program.getUniform(i).getBinding();
    //        insert.minBindingSize = 0;
    //        //insert.access = program.getUniformBlock(i).getType()->getQualifier().isWriteOnly() ? writeonly : (program.getUniformBlock(i).getType()->getQualifier().isReadOnly() ? readonly : readwrite);
    //        //std::string storageOrUniform = program.getUniformBlock(i).getType()->getStorageQualifierString();
    //        switch(program.getUniformType(i)){
    //            case GL_SAMPLER_2D:
    //            insert.type = texture_sampler;
    //            break;
    //            case GL_TEXTURE_2D:
    //            insert.type = texture2d;
    //            break;
    //            
    //            default:{
    //                const glslang::TType* oo = program.getUniformTType(i);
    //                bool cs = oo->containsSampler();
    //                std::cout << "not handled: " << program.getUniformType(i) << std::endl;
    //                //abort(); 
    //            }break;
    //        }
    //        ret[program.getUniform(i).name] = insert;
    //    }
    //    //std::cout << program.getUniform(i).getBinding() << ": " << program.getUniform(i).name << " of type " << uniformTypeNames[program.getUniformType(i)] << "\n";
    //}
    //for(uint32_t i = 0;i < program.getNumUniformBlocks();i++){
    //    std::cout << program.getUniformBlock(i).getBinding() << ": " << program.getUniformBlockName(i) << "\n";
    //}
    return ret;
}
#ifdef GLSL_TO_WGSL
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
    tint::Program resultV = tint::spirv::reader::Read(spirvV);
    tint::Program resultF = tint::spirv::reader::Read(spirvF);
    //std::cout << resultV << "\n";
    tint::ast::transform::Renamer ren;
    
    resultF.Symbols().Foreach([](tint::Symbol s){
        //std::cout << s.value() << "\n";
    });
    
    for(auto semnode : resultF.SemNodes().Objects()){
        //std::cout << semnode << "\n";
    }
    tint::ast::transform::DataMap imputV{};
    tint::ast::transform::DataMap imputF{};
    tint::ast::transform::Renamer::Config datF;
    tint::ast::transform::Renamer::Config datV;
    std::vector<std::string> scrambleInFragment;
    for(const tint::ast::Variable* gvar : resultF.AST().GlobalVariables()){
        std::string name = gvar->name->symbol.Name();
        //if(gvar->As<tint::ast::Variable>() && gvar->As<tint::ast::Var>()->declared_address_space){
        //    if(gvar->As<tint::ast::Variable>()->declared_address_space->As<tint::ast::IdentifierExpression>()){
        //        if(gvar->As<tint::ast::Variable>()->declared_address_space->As<tint::ast::IdentifierExpression>()->identifier->symbol.Name() == "private"){
        //            scrambleInFragment.push_back(name);
        //        }
        //    }
        //}
    }
    resultV.Symbols().Foreach([&](tint::Symbol x){
        std::regex m("main");
        std::string repl = "vs_main";
        std::string tname = std::regex_replace(x.Name(), m, repl);
        datV.requested_names.emplace(x.Name(), tname);
    });
    resultF.Symbols().Foreach([&](tint::Symbol x){
        std::regex m("main");
        std::string repl = "fs_main";
        std::string tname = std::regex_replace(x.Name(), m, repl);
        datF.requested_names.emplace(x.Name(), tname);
    });
    
    for(auto& n : scrambleInFragment){
        datF.requested_names[n] = n + "_frag";
    }
    datV.target = tint::ast::transform::Renamer::Target::kAll;
    datF.target = tint::ast::transform::Renamer::Target::kAll;
    imputV.Add<tint::ast::transform::Renamer::Config>(datV);
    imputF.Add<tint::ast::transform::Renamer::Config>(datF);
    tint::ast::transform::DataMap ouput{};
    auto aresultV = ren.Apply(resultV, imputV, ouput);
    auto aresultF = ren.Apply(resultF, imputF, ouput);
    auto wgsl_from_prog_resultV = tint::wgsl::writer::Generate(aresultV.value(), tint::wgsl::writer::Options{});
    auto wgsl_from_prog_resultF = tint::wgsl::writer::Generate(aresultF.value(), tint::wgsl::writer::Options{});
    //std::cout << spirvV.size() << "\n";
    //std::cout << resultV.Diagnostics() << "\n";
    //std::cout << wgsl_from_prog_resultF->wgsl << "\n";
    //std::exit(0);

    std::regex pattern("main");

    std::string replacementV = "vs_main";
    std::string replacementF = "fs_main";

    std::string sourceV = wgsl_from_prog_resultV->wgsl;//std::regex_replace(wgsl_from_prog_resultV->wgsl, pattern, replacementV);
    std::string sourceF = wgsl_from_prog_resultF->wgsl;//std::regex_replace(wgsl_from_prog_resultF->wgsl, pattern, replacementF);
    
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
    //std::cout << sourceV << "\n\n\n" << sourceF << "\n\n\n";

    std::string composed = sourceV + "\n\n" + sourceF;
    //std::cout << wgsl_from_prog_resultF->wgsl << "\n";
    return LoadPipeline(composed.c_str());
}
#endif
#else
//int main(){
//    return 0;
//}
#endif
std::unordered_map<uint32_t, std::string> uniformTypeNames = []{
    std::unordered_map<uint32_t, std::string> map;
    map[GL_FLOAT] = "GL_FLOAT";
    map[GL_FLOAT_VEC2] = "GL_FLOAT_VEC2";
    map[GL_FLOAT_VEC3] = "GL_FLOAT_VEC3";
    map[GL_FLOAT_VEC4] = "GL_FLOAT_VEC4";
    map[GL_DOUBLE] = "GL_DOUBLE";
    map[GL_DOUBLE_VEC2] = "GL_DOUBLE_VEC2";
    map[GL_DOUBLE_VEC3] = "GL_DOUBLE_VEC3";
    map[GL_DOUBLE_VEC4] = "GL_DOUBLE_VEC4";
    map[GL_INT] = "GL_INT";
    map[GL_INT_VEC2] = "GL_INT_VEC2";
    map[GL_INT_VEC3] = "GL_INT_VEC3";
    map[GL_INT_VEC4] = "GL_INT_VEC4";
    map[GL_UNSIGNED_INT] = "GL_UNSIGNED_INT";
    map[GL_UNSIGNED_INT_VEC2] = "GL_UNSIGNED_INT_VEC2";
    map[GL_UNSIGNED_INT_VEC3] = "GL_UNSIGNED_INT_VEC3";
    map[GL_UNSIGNED_INT_VEC4] = "GL_UNSIGNED_INT_VEC4";
    map[GL_BOOL] = "GL_BOOL";
    map[GL_BOOL_VEC2] = "GL_BOOL_VEC2";
    map[GL_BOOL_VEC3] = "GL_BOOL_VEC3";
    map[GL_BOOL_VEC4] = "GL_BOOL_VEC4";
    map[GL_FLOAT_MAT2] = "GL_FLOAT_MAT2";
    map[GL_FLOAT_MAT3] = "GL_FLOAT_MAT3";
    map[GL_FLOAT_MAT4] = "GL_FLOAT_MAT4";
    map[GL_FLOAT_MAT2x3] = "GL_FLOAT_MAT2x3";
    map[GL_FLOAT_MAT2x4] = "GL_FLOAT_MAT2x4";
    map[GL_FLOAT_MAT3x2] = "GL_FLOAT_MAT3x2";
    map[GL_FLOAT_MAT3x4] = "GL_FLOAT_MAT3x4";
    map[GL_FLOAT_MAT4x2] = "GL_FLOAT_MAT4x2";
    map[GL_FLOAT_MAT4x3] = "GL_FLOAT_MAT4x3";
    map[GL_DOUBLE_MAT2] = "GL_DOUBLE_MAT2";
    map[GL_DOUBLE_MAT3] = "GL_DOUBLE_MAT3";
    map[GL_DOUBLE_MAT4] = "GL_DOUBLE_MAT4";
    map[GL_DOUBLE_MAT2x3] = "GL_DOUBLE_MAT2x3";
    map[GL_DOUBLE_MAT2x4] = "GL_DOUBLE_MAT2x4";
    map[GL_DOUBLE_MAT3x2] = "GL_DOUBLE_MAT3x2";
    map[GL_DOUBLE_MAT3x4] = "GL_DOUBLE_MAT3x4";
    map[GL_DOUBLE_MAT4x2] = "GL_DOUBLE_MAT4x2";
    map[GL_DOUBLE_MAT4x3] = "GL_DOUBLE_MAT4x3";
    map[GL_SAMPLER_1D] = "GL_SAMPLER_1D";
    map[GL_SAMPLER_2D] = "GL_SAMPLER_2D";
    map[GL_SAMPLER_3D] = "GL_SAMPLER_3D";
    map[GL_SAMPLER_CUBE] = "GL_SAMPLER_CUBE";
    map[GL_SAMPLER_1D_SHADOW] = "GL_SAMPLER_1D_SHADOW";
    map[GL_SAMPLER_2D_SHADOW] = "GL_SAMPLER_2D_SHADOW";
    map[GL_SAMPLER_1D_ARRAY] = "GL_SAMPLER_1D_ARRAY";
    map[GL_SAMPLER_2D_ARRAY] = "GL_SAMPLER_2D_ARRAY";
    map[GL_SAMPLER_1D_ARRAY_SHADOW] = "GL_SAMPLER_1D_ARRAY_SHADOW";
    map[GL_SAMPLER_2D_ARRAY_SHADOW] = "GL_SAMPLER_2D_ARRAY_SHADOW";
    map[GL_SAMPLER_2D_MULTISAMPLE] = "GL_SAMPLER_2D_MULTISAMPLE";
    map[GL_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_SAMPLER_2D_MULTISAMPLE_ARRAY";
    map[GL_SAMPLER_CUBE_SHADOW] = "GL_SAMPLER_CUBE_SHADOW";
    map[GL_SAMPLER_BUFFER] = "GL_SAMPLER_BUFFER";
    map[GL_SAMPLER_2D_RECT] = "GL_SAMPLER_2D_RECT";
    map[GL_SAMPLER_2D_RECT_SHADOW] = "GL_SAMPLER_2D_RECT_SHADOW";
    map[GL_INT_SAMPLER_1D] = "GL_INT_SAMPLER_1D";
    map[GL_INT_SAMPLER_2D] = "GL_INT_SAMPLER_2D";
    map[GL_INT_SAMPLER_3D] = "GL_INT_SAMPLER_3D";
    map[GL_INT_SAMPLER_CUBE] = "GL_INT_SAMPLER_CUBE";
    map[GL_INT_SAMPLER_1D_ARRAY] = "GL_INT_SAMPLER_1D_ARRAY";
    map[GL_INT_SAMPLER_2D_ARRAY] = "GL_INT_SAMPLER_2D_ARRAY";
    map[GL_INT_SAMPLER_2D_MULTISAMPLE] = "GL_INT_SAMPLER_2D_MULTISAMPLE";
    map[GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    map[GL_INT_SAMPLER_BUFFER] = "GL_INT_SAMPLER_BUFFER";
    map[GL_INT_SAMPLER_2D_RECT] = "GL_INT_SAMPLER_2D_RECT";
    map[GL_UNSIGNED_INT_SAMPLER_1D] = "GL_UNSIGNED_INT_SAMPLER_1D";
    map[GL_UNSIGNED_INT_SAMPLER_2D] = "GL_UNSIGNED_INT_SAMPLER_2D";
    map[GL_UNSIGNED_INT_SAMPLER_3D] = "GL_UNSIGNED_INT_SAMPLER_3D";
    map[GL_UNSIGNED_INT_SAMPLER_CUBE] = "GL_UNSIGNED_INT_SAMPLER_CUBE";
    map[GL_UNSIGNED_INT_SAMPLER_1D_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY";
    map[GL_UNSIGNED_INT_SAMPLER_2D_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY";
    map[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE] = "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE";
    map[GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY] = "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY";
    map[GL_UNSIGNED_INT_SAMPLER_BUFFER] = "GL_UNSIGNED_INT_SAMPLER_BUFFER";
    map[GL_UNSIGNED_INT_SAMPLER_2D_RECT] = "GL_UNSIGNED_INT_SAMPLER_2D_RECT";
    return map;
}();
