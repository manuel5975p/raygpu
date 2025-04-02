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

#include "webgpu/webgpu.h"
#include <algorithm>
#include <raygpu.h>
#include <unordered_set>
#include <webgpu/webgpu_cpp.h>
#include <wgpustate.inc>
#include <cstring>
#include <cassert>
#include <iostream>
#include <internals.hpp>
#ifndef __EMSCRIPTEN__
#include <spirv_reflect.h>
#endif



/*WGPUBindGroupLayout bindGroupLayoutFromUniformTypes(const UniformDescriptor* uniforms, uint32_t uniformCount){
    std::vector<WGPUBindGroupLayoutEntry> blayouts(uniformCount);
    WGPUBindGroupLayoutDescriptor bglayoutdesc{};
    std::memset(blayouts.data(), 0, blayouts.size() * sizeof(WGPUBindGroupLayoutEntry));
    for(size_t i = 0;i < uniformCount;i++){
        blayouts[i].binding = i;
        switch(uniforms[i].type){
            case uniform_buffer:
                blayouts[i].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
                blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                blayouts[i].buffer.minBindingSize = uniforms[i].minBindingSize;
            break;
            case storage_buffer:{
                blayouts[i].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
                blayouts[i].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case texture2d:
                blayouts[i].visibility = WGPUShaderStage_Fragment;
                blayouts[i].texture.sampleType = WGPUTextureSampleType_Float;
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case sampler:
                blayouts[i].visibility = WGPUShaderStage_Fragment;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            default:break;
        }
    }
    bglayoutdesc.entryCount = uniformCount;
    bglayoutdesc.entries = blayouts.data();
    return wgpuDeviceCreateBindGroupLayout((WGPUDevice)GetDevice(), &bglayoutdesc);
}*/


extern "C" DescribedPipeline* LoadPipelineForVAO(const char* shaderSource, VertexArray* vao){
    ShaderSources sources zeroinit;
    sources.sourceCount = 1;
    sources.language = sourceTypeWGSL;

    sources.sources[0].stageMask = (ShaderStageMask)(ShaderStageMask_Vertex | ShaderStageMask_Fragment);
    sources.sources[0].data = shaderSource;
    sources.sources[0].sizeInBytes = std::strlen(shaderSource);

    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(sources);
    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    DescribedPipeline* pl = LoadPipelineForVAOEx(sources, vao, values.data(), values.size(), GetDefaultSettings());

    return pl;
}

DescribedPipeline* LoadPipelineForVAOEx(const char* shaderSource, VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}
extern "C" DescribedPipeline* LoadPipeline(const char* shaderSource){
    ShaderSources sources zeroinit;
    sources.language = sourceTypeWGSL;
    auto& src = sources.sources[sources.sourceCount++];
    src.data = shaderSource;
    src.sizeInBytes = std::strlen(shaderSource);
    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(sources);

    std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>> attribs = getAttributes(sources);
    
    std::vector<AttributeAndResidence> allAttribsInOneBuffer;
    allAttribsInOneBuffer.reserve(attribs.size());
    uint32_t offset = 0;
    for(const auto& [name, attr] : attribs){
        const auto& [format, location] = attr;
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
    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, allAttribsInOneBuffer.data(), allAttribsInOneBuffer.size(), values.data(), values.size(), GetDefaultSettings());
    
    return pl;
}
DescribedShaderModule LoadShaderModuleSPIRV(ShaderSources sourcesSpirv){
    DescribedShaderModule ret zeroinit;
    #ifndef __EMSCRIPTEN__
    for(uint32_t i = 0;i < sourcesSpirv.sourceCount;i++){
        WGPUShaderModuleDescriptor shaderDesc zeroinit;
        WGPUShaderModuleSPIRVDescriptor shaderCodeDesc zeroinit;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceSPIRV;

        shaderCodeDesc.code = (const uint32_t*)sourcesSpirv.sources[i].data;
        shaderCodeDesc.codeSize = sourcesSpirv.sources[i].sizeInBytes / sizeof(uint32_t);

        shaderDesc.nextInChain = &shaderCodeDesc.chain;
        WGPUShaderModule sh = wgpuDeviceCreateShaderModule((WGPUDevice)GetDevice(), &shaderDesc);
        

        spv_reflect::ShaderModule spv_mod(sourcesSpirv.sources[i].sizeInBytes, (const uint32_t*)sourcesSpirv.sources[i].data);
        uint32_t entryPointCount = spv_mod.GetEntryPointCount();
        for(uint32_t i = 0;i < entryPointCount;i++){
            auto epStage = spv_mod.GetEntryPointShaderStage(i);
            ShaderStage stage = [](SpvReflectShaderStageFlagBits epStage){
                switch(epStage){
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                        return ShaderStage_Vertex;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                        return ShaderStage_Fragment;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
                        return ShaderStage_Compute;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                        return ShaderStage_Geometry;
                    default:
                        TRACELOG(LOG_FATAL, "Unknown shader stage: %d", (int)epStage);
                        return ShaderStage_Vertex;
                }
            }(epStage);

            ret.stages[stage].module = sh;
            ret.reflectionInfo.ep[stage].stage = stage;
            std::memset(ret.reflectionInfo.ep[stage].name, 0, sizeof(ret.reflectionInfo.ep[stage].name));
            uint32_t eplength = std::strlen(spv_mod.GetEntryPointName(i));
            rassert(eplength < 15, "Entry point name must be < 15 chars");
            std::copy(spv_mod.GetEntryPointName(i), spv_mod.GetEntryPointName(i) + eplength, ret.reflectionInfo.ep[stage].name);
        }
    }
    #endif
    return ret;
}

void UnloadShaderModule(DescribedShaderModule mod){
    std::unordered_set<WGPUShaderModule> freed;
    for(size_t i = 0;i < ShaderStage_EnumCount;i++){
        if(mod.stages[i].module){
            if(!freed.contains((WGPUShaderModule)mod.stages[i].module)){
                freed.insert((WGPUShaderModule)mod.stages[i].module);
                wgpuShaderModuleRelease((WGPUShaderModule)mod.stages[i].module);
            }
        }
    }
}

WGPURenderPipeline createSingleRenderPipe(const ModifiablePipelineState &mst, const DescribedShaderModule &shaderModule, const DescribedBindGroupLayout &bglayout, const DescribedPipelineLayout &pllayout){
    WGPURenderPipelineDescriptor pipelineDesc zeroinit;
    const RenderSettings& settings = mst.settings; 
    pipelineDesc.multisample.count = settings.sampleCount ? settings.sampleCount : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = (WGPUPipelineLayout)pllayout.layout;

    WGPUVertexState   vertexState   zeroinit;
    WGPUFragmentState fragmentState zeroinit;
    WGPUBlendState    blendState    zeroinit;

    vertexState.module = (WGPUShaderModule)shaderModule.stages[ShaderStage_Vertex].module;

    VertexBufferLayoutSet vlayout_complete = getBufferLayoutRepresentation(mst.vertexAttributes.data(), mst.vertexAttributes.size());
    vertexState.bufferCount = vlayout_complete.number_of_buffers;

    std::vector<WGPUVertexBufferLayout> layouts_converted;
    for(uint32_t i = 0;i < vlayout_complete.number_of_buffers;i++){
        layouts_converted.push_back(WGPUVertexBufferLayout{
            .nextInChain    = nullptr,
            .stepMode       = (WGPUVertexStepMode)vlayout_complete.layouts[i].stepMode,
            .arrayStride    = vlayout_complete.layouts[i].arrayStride,
            .attributeCount = vlayout_complete.layouts[i].attributeCount,
            //TODO: this relies on the fact that VertexAttribute and WGPUVertexAttribute are exactly compatible
            .attributes     = (WGPUVertexAttribute*)vlayout_complete.layouts[i].attributes,
        });
    }
    vertexState.buffers = layouts_converted.data();
    vertexState.constantCount = 0;
    vertexState.entryPoint = WGPUStringView{shaderModule.reflectionInfo.ep[ShaderStage_Vertex].name, std::strlen(shaderModule.reflectionInfo.ep[ShaderStage_Vertex].name)};
    pipelineDesc.vertex = vertexState;


    
    fragmentState.module = (WGPUShaderModule)shaderModule.stages[ShaderStage_Fragment].module;
    fragmentState.entryPoint = WGPUStringView{shaderModule.reflectionInfo.ep[ShaderStage_Fragment].name, std::strlen(shaderModule.reflectionInfo.ep[ShaderStage_Fragment].name)};
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    blendState.color.srcFactor = (WGPUBlendFactor   )settings.blendState.color.srcFactor;
    blendState.color.dstFactor = (WGPUBlendFactor   )settings.blendState.color.dstFactor;
    blendState.color.operation = (WGPUBlendOperation)settings.blendState.color.operation;
    blendState.alpha.srcFactor = (WGPUBlendFactor   )settings.blendState.alpha.srcFactor;
    blendState.alpha.dstFactor = (WGPUBlendFactor   )settings.blendState.alpha.dstFactor;
    blendState.alpha.operation = (WGPUBlendOperation)settings.blendState.alpha.operation;
    WGPUColorTargetState colorTarget{};

    colorTarget.format = (WGPUTextureFormat)g_renderstate.frameBufferFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    // We setup a depth buffer state for the render pipeline
    WGPUDepthStencilState depthStencilState{};
    if(settings.depthTest){
        // Keep a fragment only if its depth is lower than the previously blended one
        // Each time a fragment is blended into the target, we update the value of the Z-buffer
        // Store the format in a variable as later parts of the code depend on it
        // Deactivate the stencil alltogether
        WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth32Float;

        depthStencilState.depthCompare = (WGPUCompareFunction)settings.depthCompare;
        depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
        depthStencilState.format = depthTextureFormat;
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;
        depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
        depthStencilState.stencilBack.compare = WGPUCompareFunction_Always;
    }
    pipelineDesc.depthStencil = settings.depthTest ? &depthStencilState : nullptr;
    
    pipelineDesc.primitive.frontFace = (WGPUFrontFace)settings.frontFace;
    pipelineDesc.primitive.cullMode = settings.faceCull ? WGPUCullMode_Back : WGPUCullMode_None;
    pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    
    pipelineDesc.primitive.topology = toWebGPUPrimitive(mst.primitiveType);
    return wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
}

extern "C" DescribedPipeline* LoadPipelineForVAOEx(ShaderSources sources, VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    //detectShaderLanguage()
    
    DescribedShaderModule module = LoadShaderModule(sources);
    
    DescribedPipeline* pl = LoadPipelineMod(module, vao->attributes.data(), vao->attributes.size(), uniforms, uniformCount, settings);
    //DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}
extern "C" DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    
    ShaderSources sources zeroinit;
    sources.sourceCount = 1;
    sources.language = sourceTypeWGSL;
    sources.sources[0].data = shaderSource;
    sources.sources[0].sizeInBytes = std::strlen(shaderSource);
    sources.sources[0].stageMask = ShaderStageMask(ShaderStageMask_Vertex | ShaderStageMask_Fragment);
    DescribedShaderModule mod = LoadShaderModuleWGSL(sources);

    return LoadPipelineMod(mod, attribs, attribCount, uniforms, uniformCount, settings);
}
extern "C" DescribedPipeline* LoadPipelineMod(DescribedShaderModule mod, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){

    DescribedPipeline* retp = callocnewpp(DescribedPipeline);
    
    retp->state.settings = settings;
    DescribedPipeline& ret = *retp;
    ret.shaderModule = mod;

    ret.state.vertexAttributes;    


    ret.bglayout = LoadBindGroupLayout(uniforms, uniformCount, false);

    std::vector<ResourceDescriptor> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = ResourceDescriptor{};
        bge[i].binding = uniforms[i].location;
    }
    ret.bindGroup = LoadBindGroup(&ret.bglayout, bge.data(), bge.size());
    WGPUPipelineLayoutDescriptor layout_descriptor zeroinit;

    layout_descriptor.bindGroupLayoutCount = 1;
    layout_descriptor.bindGroupLayouts = (WGPUBindGroupLayout*) (&ret.bglayout.layout);
    ret.layout.layout = wgpuDeviceCreatePipelineLayout((WGPUDevice)GetDevice(), &layout_descriptor);
    
    UpdatePipeline(retp);
    //TRACELOG(LOG_INFO, "Pipeline with %d attributes and %d uniforms was loaded", attribCount, uniformCount);
    return retp;
}

std::vector<AttributeAndResidence> GetAttributesAndResidences(const VertexArray* va){
    return va->attributes;
}

WGPUBuffer cloneBuffer(WGPUBuffer b, WGPUBufferUsage usage){
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), nullptr);
    WGPUBufferDescriptor retd{};
    retd.usage = usage;
    retd.size = wgpuBufferGetSize(b);
    WGPUBuffer ret = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &retd);
    wgpuCommandEncoderCopyBufferToBuffer(enc, b, 0, ret, 0, retd.size);
    WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(enc, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &buffer);
    return ret;
}

//DescribedPipeline* ClonePipelineWithSettings(const DescribedPipeline* pl, RenderSettings settings){
//    DescribedPipeline* cloned = ClonePipeline(pl);
//
//    cloned->settings = settings;
//    UpdatePipeline(cloned);
//    return cloned;
//}
DescribedComputePipeline* LoadComputePipeline(const char* shaderCode){
    ShaderSources sources zeroinit;
    sources.sourceCount = 1;
    sources.language = sourceTypeWGSL;

    sources.sources[0].data = shaderCode;
    sources.sources[0].sizeInBytes = std::strlen(shaderCode);
    sources.sources[0].stageMask = ShaderStageMask_Compute;

    auto bindmap = getBindings(sources);
    std::vector<ResourceTypeDescriptor> udesc;
    for(auto& [x,y] : bindmap){
        udesc.push_back(y);
    }
    std::sort(udesc.begin(), udesc.end(), [](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    return LoadComputePipelineEx(shaderCode, udesc.data(), udesc.size());
    
}
DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount){
    ShaderSources sources zeroinit;
    sources.sourceCount = 1;
    sources.language = sourceTypeWGSL;
    sources.sources[0].data = shaderCode;
    sources.sources[0].sizeInBytes = std::strlen(shaderCode);
    sources.sources[0].stageMask = ShaderStageMask_Compute;

    auto bindmap = getBindings(sources);
    DescribedComputePipeline* ret = callocnew(DescribedComputePipeline);
    WGPUComputePipelineDescriptor desc{};
    WGPUPipelineLayoutDescriptor pldesc{};
    pldesc.bindGroupLayoutCount = 1;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, true);
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = (WGPUBindGroupLayout*) &ret->bglayout.layout;
    WGPUPipelineLayout playout = wgpuDeviceCreatePipelineLayout((WGPUDevice)GetDevice(), &pldesc);
    ret->shaderModule = LoadShaderModuleWGSL(sources);
    desc.compute.module = (WGPUShaderModule) ret->shaderModule.stages[ShaderStage_Compute].module;

    desc.compute.entryPoint = WGPUStringView{ret->shaderModule.reflectionInfo.ep[ShaderStage_Compute].name, std::strlen(ret->shaderModule.reflectionInfo.ep[ShaderStage_Compute].name)};
    desc.layout = playout;
    WGPUDevice device = (WGPUDevice)GetDevice();
    ret->pipeline = wgpuDeviceCreateComputePipeline((WGPUDevice)GetDevice(), &desc);
    std::vector<ResourceDescriptor> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = ResourceDescriptor{};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge.data(), bge.size());
    return ret;
}
Shader LoadShaderFromMemory(const char *vertexSource, const char *fragmentSource){
    Shader shader zeroinit;
    
    #if SUPPORT_GLSL_PARSER == 1

    //shader.id = LoadPipelineGLSL(vertexSource, fragmentSource);
    shader.locs = (int*)std::calloc(RL_MAX_SHADER_LOCATIONS, sizeof(int));
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) {
        shader.locs[i] = LOCATION_NOT_FOUND;
    }
    //shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    //shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    //shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    //shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    //shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    //shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    //shader.locs[SHADER_LOC_VERTEX_BONEIDS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS);
    //shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);
    //shader.locs[SHADER_LOC_VERTEX_INSTANCE_TX] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX);

    // Get handles to GLSL uniform locations (vertex shader)
    shader.locs[SHADER_LOC_MATRIX_MVP] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
    shader.locs[SHADER_LOC_MATRIX_VIEW] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
    shader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);
    shader.locs[SHADER_LOC_BONE_MATRICES] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES);

    // Get handles to GLSL uniform locations (fragment shader)
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
    shader.locs[SHADER_LOC_MAP_DIFFUSE]   = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // SHADER_LOC_MAP_ALBEDO
    shader.locs[SHADER_LOC_MAP_SPECULAR]  = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
    shader.locs[SHADER_LOC_MAP_NORMAL]    = GetUniformLocation(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    #else
    TRACELOG(LOG_ERROR, "GLSL Shader requested but compiled without GLSL support");
    TRACELOG(LOG_ERROR, "Configure CMake with -DSUPPORT_GLSL_PARSER=ON");
    #endif
    return shader;
}

extern "C" void UnloadPipeline(DescribedPipeline* pl){
    
    //MASSIVE TODO

    /*wgpuPipelineLayoutRelease(pl->layout.layout);
    UnloadBindGroup(&pl->bindGroup);
    UnloadBindGroupLayout(&pl->bglayout);
    wgpuRenderPipelineRelease(pl->pipeline);
    wgpuRenderPipelineRelease(pl->pipeline_LineList);
    wgpuRenderPipelineRelease(pl->pipeline_TriangleStrip);
    free(pl->attributePool);
    UnloadShaderModule(pl->sh);
    free(pl->blendState);
    free(pl->colorTarget);
    free(pl->depthStencilState);*/
}







extern "C" void UnloadBindGroup(DescribedBindGroup* bg){
    free(bg->entries);
    wgpuBindGroupRelease((WGPUBindGroup)bg->bindGroup);
}
extern "C" void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout){
    free(bglayout->entries);
    wgpuBindGroupLayoutRelease((WGPUBindGroupLayout)bglayout->layout);
}
void UniformAccessor::operator=(DescribedBuffer* buf){
    SetBindgroupStorageBuffer(bindgroup, index, buf);
}
UniformAccessor DescribedComputePipeline::operator[](const char* uniformName){

    auto it = shaderModule.reflectionInfo.uniforms->uniforms.find(uniformName);
    if(it == shaderModule.reflectionInfo.uniforms->uniforms.end()){
        TRACELOG(LOG_ERROR, "Accessing nonexistent uniform %s", uniformName);
        return UniformAccessor{.index = LOCATION_NOT_FOUND, .bindgroup = nullptr};
    }
    uint32_t location = it->second.location;
    return UniformAccessor{.index = location, .bindgroup = &this->bindGroup};
}
