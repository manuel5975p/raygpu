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


#include <memory>
#include <map>
#include <unordered_map>
#include <bit>
#include <fstream>
#include <external/gl_corearb.h>
#if SUPPORT_GLSL_PARSER == 1
//#include <SPIRV/GlslangToSpv.h>
#include "glslang/Public/ResourceLimits.h"

#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/localintermediate.h>
#include <glslang/Public/resource_limits_c.h>
#include <spirv_reflect.h>
#include <raygpu.h>
#include <internals.hpp>

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
#include <bitset>

#if SUPPORT_GLSL_PARSER == 1

#if SUPPORT_VULKAN_BACKEND == 1 && defined(VULKAN_ENABLE_RAYTRACING) && VULKAN_ENABLE_RAYTRACING == 1
constexpr auto defaultSpirvVersion = glslang::EShTargetSpv_1_4;
#else
constexpr auto defaultSpirvVersion = glslang::EShTargetSpv_1_3;
#endif

extern "C" const char vertexSourceGLSL[];
extern "C" const char fragmentSourceGLSL[];
const TBuiltInResource DefaultTBuiltInResource_RG = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
    /* .maxMeshOutputVerticesEXT = */ 256,
    /* .maxMeshOutputPrimitivesEXT = */ 256,
    /* .maxMeshWorkGroupSizeX_EXT = */ 128,
    /* .maxMeshWorkGroupSizeY_EXT = */ 128,
    /* .maxMeshWorkGroupSizeZ_EXT = */ 128,
    /* .maxTaskWorkGroupSizeX_EXT = */ 128,
    /* .maxTaskWorkGroupSizeY_EXT = */ 128,
    /* .maxTaskWorkGroupSizeZ_EXT = */ 128,
    /* .maxMeshViewCountEXT = */ 4,
    /* .maxDualSourceDrawBuffersEXT = */ 1,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};
#endif

extern std::unordered_map<uint32_t, std::string> uniformTypeNames;
bool glslang_initialized = false;
std::vector<uint32_t> glsl_to_spirv_single(const char* cs, EShLanguage stage){
    glslang::TShader shader(stage);
    shader.setEnvInput (glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, glslang::EShTargetVulkan_1_4);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_4);
    shader.setEnvTarget(glslang::EShTargetSpv, defaultSpirvVersion);
    shader.setStrings(&cs, 1);

    auto stageToString = [](EShLanguage stage){
        switch(stage){
            case EShLangVertex: return "EShLangVertex";
            case EShLangTessControl: return "EShLangTessControl";
            case EShLangTessEvaluation: return "EShLangTessEvaluation";
            case EShLangGeometry: return "EShLangGeometry";
            case EShLangFragment: return "EShLangFragment";
            case EShLangCompute: return "EShLangCompute";
            case EShLangRayGen: return "EShLangRayGen";
            case EShLangIntersect: return "EShLangIntersect";
            case EShLangAnyHit: return "EShLangAnyHit";
            case EShLangClosestHit: return "EShLangClosestHit";
            case EShLangMiss: return "EShLangMiss";
            case EShLangCallable: return "EShLangCallable";
            case EShLangTask: return "EShLangTask";
            case EShLangMesh: return "EShLangMesh";
            default: return "??unknown ShaderStage??";
        }
    };
    TBuiltInResource Resources zeroinit;
    Resources.maxComputeWorkGroupSizeX = 1024;
    Resources.maxComputeWorkGroupSizeY = 1024;
    Resources.maxComputeWorkGroupSizeZ = 1024;
    Resources.maxCombinedTextureImageUnits = 8;

    Resources.limits.generalUniformIndexing = true;
    Resources.limits.generalVariableIndexing = true;
    Resources.maxDrawBuffers = MAX_COLOR_ATTACHMENTS;

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    if(!shader.parse(&Resources, glslang::EShTargetVulkan_1_4, ECoreProfile, false, false, messages)){
        TRACELOG(LOG_ERROR, "%s GLSL Parsing Failed: %s", stageToString(stage), shader.getInfoLog());
    }
    glslang::TProgram program;
    program.addShader(&shader);
    if(!program.link(messages)){
        TRACELOG(LOG_ERROR, "Error linkin shader: %s", program.getInfoLog());
    }
    glslang::TIntermediate* intermediate = program.getIntermediate(stage);
    std::vector<uint32_t> output;
    glslang::GlslangToSpv(*intermediate, output);
    return output;
}
std::vector<uint32_t> glsl_to_spirv(const char *cs){
    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }
    return glsl_to_spirv_single(cs, EShLangCompute);   
}

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
EShLanguage ShaderStageToGlslanguage(ShaderStage stage){
    switch(stage){
        case ShaderStage_Vertex: return EShLangVertex; 
        case ShaderStage_TessControl: return EShLangTessControl; 
        case ShaderStage_TessEvaluation: return EShLangTessEvaluation; 
        case ShaderStage_Geometry: return EShLangGeometry; 
        case ShaderStage_Fragment: return EShLangFragment; 
        case ShaderStage_Compute: return EShLangCompute; 
        case ShaderStage_RayGen: return EShLangRayGen; 
        case ShaderStage_Intersect: return EShLangIntersect; 
        case ShaderStage_AnyHit: return EShLangAnyHit; 
        case ShaderStage_ClosestHit: return EShLangClosestHit; 
        case ShaderStage_Miss: return EShLangMiss; 
        case ShaderStage_Callable: return EShLangCallable; 
        case ShaderStage_Task: return EShLangTask; 
        case ShaderStage_Mesh: return EShLangMesh; 
        default: rg_unreachable();     
    }
}
VertexFormat fromGLVertexFormat(uint32_t glType){
    switch(glType){
        default: 
            rassert(false, "unsupported gl vertex format");
            return VertexFormat(~0);
        case GL_INT:      return VertexFormat_Sint32;
        case GL_INT_VEC2: return VertexFormat_Sint32x2;
        case GL_INT_VEC3: return VertexFormat_Sint32x3;
        case GL_INT_VEC4: return VertexFormat_Sint32x4;

        case GL_UNSIGNED_INT:      return VertexFormat_Uint32;
        case GL_UNSIGNED_INT_VEC2: return VertexFormat_Uint32x2;
        case GL_UNSIGNED_INT_VEC3: return VertexFormat_Uint32x3;
        case GL_UNSIGNED_INT_VEC4: return VertexFormat_Uint32x4;

        case GL_FLOAT:      return VertexFormat_Float32;
        case GL_FLOAT_VEC2: return VertexFormat_Float32x2;
        case GL_FLOAT_VEC3: return VertexFormat_Float32x3;
        case GL_FLOAT_VEC4: return VertexFormat_Float32x4;
    }
    rg_unreachable();
};
InOutAttributeInfo getAttributesGLSL(ShaderSources sources){
    const int glslVersion = 460;
    InOutAttributeInfo ret;
    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }

    std::vector<std::pair<EShLanguage, std::unique_ptr<glslang::TShader>>> shaders;
    std::vector<const char*> stageSources;
    for(uint32_t i = 0;i < sources.sourceCount;i++){
        if(std::bitset<sizeof(ShaderStageMask) * CHAR_BIT>(+sources.sources[i].stageMask).count() != 1){
            TRACELOG(LOG_ERROR, "Only single stages are supported for GLSL");
        }
        ShaderStage stage = (ShaderStage)std::countr_zero(uint32_t(sources.sources[i].stageMask));
        shaders.emplace_back(ShaderStageToGlslanguage(stage), std::make_unique<glslang::TShader>(ShaderStageToGlslanguage(stage)));
        shaders.back().second->setEnvTarget(glslang::EshTargetSpv, defaultSpirvVersion);
    }

    const TBuiltInResource* Resources = &DefaultTBuiltInResource_RG;
    
    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    // Parse the shader
    
    for(size_t i = 0;i < shaders.size();i++){
        auto& [language, shader] = shaders[i];
        const char* dptr = (const char*)sources.sources[i].data;
        shader->setStrings(reinterpret_cast<char const*const*const>(&(sources.sources[i].data)), 1);
        shader->setAutoMapLocations(false);
        shader->setAutoMapBindings (false);
        if(!shader->parse(Resources,  glslVersion, ECoreProfile, false, false, messages)){
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
        //TRACELOG(LOG_INFO, "Program linked successfully");
    }
    program.buildReflection();
    std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>>& retInputs = ret.vertexAttributes;
    uint32_t attributeCount = program.getNumLiveAttributes();

    int pouts = program.getNumPipeOutputs();
    ret.attachments.resize(pouts);
    for(int i = 0;i < pouts;i++){
        int vectorsize = -1;
        format_or_sample_type sample_type = format_or_sample_type::we_dont_know;

        if(program.getPipeOutput(i).getType()->isVector()){
            vectorsize = program.getPipeOutput(i).getType()->getVectorSize();
            glslang::TBasicType ebt = program.getPipeOutput(i).getType()->getBasicType();
            sample_type = ebt == glslang::EbtFloat ? format_or_sample_type::sample_f32 : format_or_sample_type::sample_u32;
        }
        else if(program.getPipeOutput(0).getType()->isScalar()){
            glslang::TBasicType ebt = program.getPipeOutput(i).getType()->getBasicType();
            sample_type = ebt == glslang::EbtFloat ? format_or_sample_type::sample_f32 : format_or_sample_type::sample_u32;
            vectorsize = 1;
        }
        glslang::TString tstr = program.getPipeOutput(0).getType()->getCompleteString();
        const char* tstr_cstr = tstr.c_str();
        rassert(vectorsize != -1, "Could not make sense of this type: %s", tstr_cstr);
        
        ret.attachments[i] = {};
    }
    for(int32_t i = 0;i < attributeCount;i++){
        int glattrib = program.getAttributeType(i);
        std::string attribname = program.getAttributeName(i);
        if(program.getAttributeTType(i)->getQualifier().hasLocation())
        {
            uint32_t location = program.getAttributeTType(i)->getQualifier().layoutLocation;
            
            VertexFormat type = fromGLVertexFormat(glattrib);
            if(!attribname.starts_with("gl_")){
                retInputs[attribname] = {type, location};
            }
        }

    }
    return ret;
}
namespace glslang{
    struct aggregateTraverser: TIntermTraverser{
        std::vector<std::string> textureNames;
        virtual void visitSymbol(TIntermSymbol* sym)override{
            if(sym->getType().isTexture())
                textureNames.emplace_back(sym->getName());
        }
    };
    template<typename callable>
    struct testTraverser : TIntermTraverser{
        std::unordered_map<std::string, format_or_sample_type> sampleTypes;

        testTraverser(callable c) : m_callable(std::move(c)){}
        callable m_callable;
        virtual void visitSymbol(TIntermSymbol* sym)           {if(false)std::cout << sym->getCompleteString() << " visited.\n"; }
        virtual void visitConstantUnion(TIntermConstantUnion*) { }
        virtual bool visitBinary(TVisit, TIntermBinary*)       { return true; }
        virtual bool visitUnary(TVisit, TIntermUnary* unary)   { return true;   }
        virtual bool visitSelection(TVisit, TIntermSelection* sel) { if(false)  std::cout << sel->getCompleteString() << "\n";return true; }
        virtual bool visitAggregate(TVisit, TIntermAggregate* agg) {
            if(agg->isSampling()){
                aggregateTraverser agt{};
                agg->traverse(&agt);
                m_callable(agg->getType(), agt.textureNames);
                format_or_sample_type type = agg->getType().isFloatingDomain() ? format_or_sample_type::sample_f32 : format_or_sample_type::sample_u32;
                for(const std::string& x : agt.textureNames){
                    sampleTypes.emplace(x, type);
                }
                return false;
            }
            return true; 
        }
        virtual bool visitLoop(TVisit, TIntermLoop*)           { return true; }
        virtual bool visitBranch(TVisit, TIntermBranch*)       { return true; }
        virtual bool visitSwitch(TVisit, TIntermSwitch*)       { return true; }
    };
}

std::unordered_map<std::string, ResourceTypeDescriptor> getBindingsGLSL(ShaderSources sources){
    const int glslVersion = 460;
    
    
    if (!glslang_initialized){
        glslang::InitializeProcess();
        glslang_initialized = true;
    }
    
    std::vector<std::pair<EShLanguage, std::unique_ptr<glslang::TShader>>> shaders;
    std::vector<const char*> stageSources;
    for(uint32_t i = 0;i < sources.sourceCount;i++){
        if(std::bitset<sizeof(ShaderStageMask) * CHAR_BIT>(+sources.sources[i].stageMask).count() == 0){
            TRACELOG(LOG_FATAL, "Empty shader stage");
        }
        else if(std::bitset<sizeof(ShaderStageMask) * CHAR_BIT>(+sources.sources[i].stageMask).count() > 1){
            std::cout << sources.sources[i].stageMask << "\n";
            TRACELOG(LOG_FATAL, "Only single stages are supported for GLSL");
        }
        ShaderStage stage = (ShaderStage)std::countr_zero(uint32_t(sources.sources[i].stageMask));
        shaders.emplace_back(ShaderStageToGlslanguage(stage), std::make_unique<glslang::TShader>(ShaderStageToGlslanguage(stage)));
        shaders.back().second->setEnvTarget(glslang::EShTargetSpv, defaultSpirvVersion);
    }

    TBuiltInResource Resources = DefaultTBuiltInResource_RG;

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);
    // Parse the shader
    
    for(size_t i = 0;i < shaders.size();i++){
        auto& [language, shader] = shaders[i];
        shader->setStrings(reinterpret_cast<char const*const*const>(&sources.sources[i].data), 1);
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
            case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                return acceleration_structure;
            break;
            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                return combined_image_sampler;
            default:
                rassert(false, "Unsupported");
                rg_unreachable();
        }
    };

    std::unordered_map<std::string, ResourceTypeDescriptor> ret;
    for(const auto& [lang, shader] : shaders){
        glslang::TIntermediate* intermediate = program.getIntermediate(lang);
        std::vector<uint32_t> stageSpirv;
        //glslang::SpvOptions options{.generateDebugInfo = true};
        glslang::GlslangToSpv(*intermediate, stageSpirv);
        auto marker = [](const glslang::TType& tt, const std::vector<std::string>& textureNames){

        };
        glslang::testTraverser<decltype(marker)> traverser(marker);
        intermediate->getTreeRoot()->traverse(&traverser);

        spv_reflect::ShaderModule mod(stageSpirv);
        uint32_t count = 0;
        SpvReflectResult result = spvReflectEnumerateDescriptorSets(&mod.GetShaderModule(), &count, NULL);
        rassert(result == SPV_REFLECT_RESULT_SUCCESS, "spvReflectEnumerateDescriptorSets failed");
      
        std::vector<SpvReflectDescriptorSet*> sets(count);
        result = spvReflectEnumerateDescriptorSets(&mod.GetShaderModule(), &count, sets.data());
        std::vector<SpvReflectInterfaceVariable*> vars;
        uint32_t varcount = 0;
        spvReflectEnumerateInputVariables(&mod.GetShaderModule(), &varcount, nullptr);
        vars.resize(varcount);
        spvReflectEnumerateInputVariables(&mod.GetShaderModule(), &varcount, vars.data());
        
        for(auto set : sets){
        
            for(uint32_t i = 0;i < set->binding_count;i++){
                auto binding = set->bindings[i];
                //std::cout << set->bindings[i]->name << ": " <<  set->bindings[i]->descriptor_type << "\n";
                
                if(set->bindings[i]->descriptor_type != SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER && set->bindings[i]->descriptor_type != SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER){
                    ResourceTypeDescriptor insert zeroinit;
                    auto& binding = set->bindings[i];
                    insert.fstype = traverser.sampleTypes[set->bindings[i]->name];
                    insert.type = spvdsToResourceType(set->bindings[i]->descriptor_type);
                    insert.location = set->bindings[i]->binding;
                    ret[set->bindings[i]->name] = insert;
                    auto& inserted = *ret.find(set->bindings[i]->name);
                    inserted.second.visibility = ShaderStageMask(inserted.second.visibility | ShaderStageMask(1u << lang));
                }

                //TRACELOG(LOG_WARNING, "Parsed uniforms %s at binding %u", set->bindings[i]->name, (unsigned)set->bindings[i]->binding);// << ": " <<  set->bindings[i]->descriptor_type << "\n";
                
            }
        }
        //std::cout << std::endl;
    }
    program.buildReflection();
    for(int i = 0;i < program.getNumUniformBlocks();i++){
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
        ret[name] = insert;
        auto& inserted = *ret.find(name);
        inserted.second.visibility = ShaderStageMask(inserted.second.visibility | ShaderStageMask(program.getUniformBlock(i).stages));
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
ShaderSources glsl_to_spirv(ShaderSources sources){
    ShaderSources ret zeroinit;
    rassert(sources.language == sourceTypeGLSL, "Must be GLSL here");
    ret.sourceCount = sources.sourceCount;
    ret.language = sourceTypeSPIRV;

    for(uint32_t i = 0;i < sources.sourceCount;i++){
        ShaderStage stage = (ShaderStage)std::countr_zero((uint32_t)sources.sources[i].stageMask);
        std::vector<uint32_t> stageToSpirv = glsl_to_spirv_single((const char*)sources.sources[i].data, ShaderStageToGlslanguage(stage));
        uint32_t* odata = (uint32_t*)std::calloc(stageToSpirv.size(), sizeof(uint32_t));
        std::copy(stageToSpirv.begin(), stageToSpirv.end(), odata);
        ret.sources[i].data = odata;
        ret.sources[i].sizeInBytes = stageToSpirv.size() * sizeof(uint32_t);
        ret.sources[i].stageMask = sources.sources[i].stageMask;
    }
    return ret;
}

DescribedShaderModule LoadShaderModuleGLSL(ShaderSources sourcesGLSL){
    ShaderSources spirv = glsl_to_spirv(sourcesGLSL);
    DescribedShaderModule ret = LoadShaderModuleSPIRV(spirv);
    ret.reflectionInfo.uniforms = callocnewpp(StringToUniformMap);
    ret.reflectionInfo.attributes = callocnewpp(StringToAttributeMap);
    ret.reflectionInfo.uniforms->uniforms = getBindingsGLSL(sourcesGLSL);
    
    InOutAttributeInfo attribs = getAttributesGLSL(sourcesGLSL);
    ret.reflectionInfo.attributes->attributes = attribs.vertexAttributes;
    ret.reflectionInfo.colorAttachmentCount = attribs.attachments.size();
    return ret;
}
DescribedPipeline* LoadPipelineGLSL(const char* vs, const char* fs){
    ShaderSources glslSources zeroinit;
    glslSources.language = sourceTypeGLSL;
    
    glslSources.sourceCount = 2;
    glslSources.sources[0].data = vs ? vs : vertexSourceGLSL;
    glslSources.sources[0].sizeInBytes = std::strlen((const char*)glslSources.sources[0].data);
    glslSources.sources[0].stageMask = ShaderStageMask_Vertex;

    glslSources.sources[1].data = fs ? fs : fragmentSourceGLSL;
    glslSources.sources[1].sizeInBytes = std::strlen((const char*)glslSources.sources[1].data);
    glslSources.sources[1].stageMask = ShaderStageMask_Fragment;

    DescribedShaderModule shaderModule = LoadShaderModuleGLSL(glslSources);

    std::vector<ResourceTypeDescriptor> flat;
    flat.reserve(shaderModule.reflectionInfo.uniforms->uniforms.size());
    for(const auto& [x, y] : shaderModule.reflectionInfo.uniforms->uniforms){
        flat.push_back(y);
    }
    std::sort(flat.begin(), flat.end(), [](const ResourceTypeDescriptor& a, const ResourceTypeDescriptor& b){
        return a.location < b.location;
    });

    std::vector<std::pair<VertexFormat, unsigned int>> flatAttributes;
    flatAttributes.reserve(shaderModule.reflectionInfo.attributes->attributes.size());
    for(const auto& [x, y] : shaderModule.reflectionInfo.attributes->attributes){
        flatAttributes.push_back(y);
    }
    std::sort(flatAttributes.begin(), flatAttributes.end(), [](const std::pair<VertexFormat, unsigned int>& a, const std::pair<VertexFormat, unsigned int>& b){
        return a.second < b.second;
    });
    std::vector<AttributeAndResidence> allAttribsInOneBuffer;

    allAttribsInOneBuffer.reserve(flatAttributes.size());
    uint32_t offset = 0;
    
    for(const auto& [format, location] : flatAttributes){
        allAttribsInOneBuffer.push_back(AttributeAndResidence{
            .attr = VertexAttribute{
                .nextInChain = nullptr,
                .format = format,
                .offset = offset,
                .shaderLocation = location
            },
            .bufferSlot = 0,
            .stepMode = VertexStepMode_Vertex,
            .enabled = true}
        );
        offset += attributeSize(format);
    }
    return LoadPipelineMod(shaderModule, allAttribsInOneBuffer.data(), allAttribsInOneBuffer.size(), flat.data(), flat.size(), GetDefaultSettings());
}
extern "C" DescribedPipeline* rlLoadShaderCode(char const* vs, char const* fs){
    return LoadPipelineGLSL(vs, fs);
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


