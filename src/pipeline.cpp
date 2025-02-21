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

#include <algorithm>
#include <raygpu.h>
#include <webgpu/webgpu_cpp.h>
#include <wgpustate.inc>
#include <cstring>
#include <cassert>
#include <iostream>
#include <internals.hpp>
typedef struct StringToUniformMap{
    std::unordered_map<std::string, ResourceTypeDescriptor> uniforms;
    ResourceTypeDescriptor operator[](const std::string& v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const std::string& v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
    }
    ResourceTypeDescriptor operator[](const char* v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const char* v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
        
    }
}StringToUniformMap;




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
    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(shaderSource);
    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    DescribedPipeline* pl = LoadPipelineForVAOEx(shaderSource, vao, values.data(), values.size(), GetDefaultSettings());

    return pl;
}
uint32_t GetUniformLocation(DescribedPipeline* pl, const char* uniformName){
    return LOCATION_NOT_FOUND;
    return pl->sh.uniformLocations->GetLocation(uniformName);
}
uint32_t GetUniformLocationCompute(DescribedComputePipeline* pl, const char* uniformName){
    return pl->shaderModule.uniformLocations->GetLocation(uniformName);
}
DescribedPipeline* LoadPipelineForVAOEx(const char* shaderSource, VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}
extern "C" DescribedPipeline* LoadPipeline(const char* shaderSource){
    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(shaderSource);

    std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>> attribs = getAttributes(shaderSource);
    
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
DescribedShaderModule LoadShaderModuleFromSPIRV(const uint32_t* shaderCodeSPIRV, size_t codeSizeInBytes){

    WGPUShaderModuleSPIRVDescriptor shaderCodeDesc zeroinit;
    
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceSPIRV;

    shaderCodeDesc.code = (uint32_t*)shaderCodeSPIRV;
    shaderCodeDesc.codeSize = codeSizeInBytes / sizeof(uint32_t);
    WGPUShaderModuleDescriptor shaderDesc zeroinit;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    WGPUStringView strv = STRVIEW("spirv_shader");
    WGPUShaderModule sh = wgpuDeviceCreateShaderModule((WGPUDevice)GetDevice(), &shaderDesc);
    DescribedShaderModule ret zeroinit;
    ret.vertexModule = sh;
    ret.fragmentModule = sh;
    ret.source = calloc(codeSizeInBytes / sizeof(uint32_t), sizeof(uint32_t));
    std::memcpy(const_cast<void*>(ret.source), shaderCodeSPIRV, codeSizeInBytes);
    ret.sourceLengthInBytes = codeSizeInBytes;
    ret.uniformLocations = callocnew(StringToUniformMap);
    new (ret.uniformLocations) StringToUniformMap;
    return ret;
}
DescribedShaderModule LoadShaderModuleFromMemoryWGSL2(const char* shaderSourceWGSLVertex, const char* shaderSourceWGSLFragment){
    abort();
    WGPUShaderModuleDescriptor shaderDesc zeroinit;

    WGPUShaderModuleWGSLDescriptor shaderCodeDesc1 zeroinit;
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc2 zeroinit;
    WGPUShaderModuleSPIRVDescriptor shaderCodeDesc3 zeroinit;

    shaderDesc.nextInChain = &shaderCodeDesc1.chain;
    return {};
}



DescribedShaderModule LoadShaderModuleFromMemory(const char* shaderSourceWGSL) {
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};

    size_t sourceSize = strlen(shaderSourceWGSL);

    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(shaderSourceWGSL);
    
    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    shaderCodeDesc.code = WGPUStringView{shaderSourceWGSL, sourceSize};
    WGPUShaderModuleDescriptor shaderDesc zeroinit;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    #ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    #endif
    WGPUShaderModule mod = wgpuDeviceCreateShaderModule((WGPUDevice)GetDevice(), &shaderDesc);
    DescribedShaderModule ret zeroinit;
    ret.sourceType = sourceTypeWGSL;
    ret.uniformLocations = callocnew(StringToUniformMap);
    new (ret.uniformLocations) StringToUniformMap;
    for(const auto& [x,y] : bindings){
        ret.uniformLocations->uniforms[x] = y;
    }
    void* source = std::calloc(sourceSize + 1, 1);
    std::memcpy(source, shaderSourceWGSL, sourceSize);
    ret.vertexModule = mod;
    ret.fragmentModule = mod;
    ret.computeModule = mod;
    ret.sourceLengthInBytes = sourceSize;
    ret.source = source;
    return ret;
}
void UnloadShaderModule(DescribedShaderModule mod){
    //hmmmmmmmmmmmmmmmmmmmmmmmmmm
    //const shouldn't exist!!1!
    std::free(const_cast<void*>(mod.source));
    if(mod.vertexModule) wgpuShaderModuleRelease((WGPUShaderModule)mod.vertexModule);
    if(mod.fragmentModule && mod.fragmentModule != mod.vertexModule){
        wgpuShaderModuleRelease((WGPUShaderModule)mod.fragmentModule);
    }
    if(mod.computeModule && mod.computeModule != mod.vertexModule && mod.computeModule != mod.fragmentModule){
        wgpuShaderModuleRelease((WGPUShaderModule)mod.computeModule);
    }
}
extern "C" void UpdatePipeline(DescribedPipeline* pl){
    WGPURenderPipelineDescriptor pipelineDesc zeroinit;
    const RenderSettings& settings = pl->settings; 
    pipelineDesc.multisample.count = pl->settings.sampleCount ? pl->settings.sampleCount : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = (WGPUPipelineLayout)pl->layout.layout;

    WGPUVertexState vertexState{};
    WGPUFragmentState fragmentState{};
    WGPUBlendState blendState{};

    vertexState.module = (WGPUShaderModule)pl->sh.vertexModule;

    VertexBufferLayoutSet& vlayout_complete = pl->vertexLayout;
    vertexState.bufferCount = vlayout_complete.number_of_buffers;

    std::vector<WGPUVertexBufferLayout> layouts_converted;
    for(uint32_t i = 0;i < vlayout_complete.number_of_buffers;i++){
        layouts_converted.push_back(WGPUVertexBufferLayout{
            .nextInChain    = nullptr,
            .arrayStride    = vlayout_complete.layouts[i].arrayStride,
            .stepMode       = (WGPUVertexStepMode)vlayout_complete.layouts[i].stepMode,
            .attributeCount = vlayout_complete.layouts[i].attributeCount,
            //TODO: this relies on the fact that VertexAttribute and WGPUVertexAttribute are exactly compatible
            .attributes     = (WGPUVertexAttribute*)vlayout_complete.layouts[i].attributes,
        });
    }
    vertexState.buffers = layouts_converted.data();
    vertexState.constantCount = 0;
    vertexState.entryPoint = STRVIEW("vs_main");
    pipelineDesc.vertex = vertexState;


    
    fragmentState.module = (WGPUShaderModule)pl->sh.fragmentModule;
    fragmentState.entryPoint = STRVIEW("fs_main");
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    blendState.color.srcFactor = (WGPUBlendFactor   )settings.blendFactorSrcColor;
    blendState.color.dstFactor = (WGPUBlendFactor   )settings.blendFactorDstColor;
    blendState.color.operation = (WGPUBlendOperation)settings.blendOperationColor;
    blendState.alpha.srcFactor = (WGPUBlendFactor   )settings.blendFactorSrcAlpha;
    blendState.alpha.dstFactor = (WGPUBlendFactor   )settings.blendFactorDstAlpha;
    blendState.alpha.operation = (WGPUBlendOperation)settings.blendOperationAlpha;
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
    if(pl->vertexLayout.layouts->attributeCount != 0){
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_PointList;
        pl->quartet.pipeline_PointList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        pl->quartet.pipeline_TriangleList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_LineList;
        pl->quartet.pipeline_LineList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
        pl->quartet.pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
        pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    }
}
extern "C" DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedShaderModule mod = LoadShaderModuleFromMemory(shaderSource);
    return LoadPipelineMod(mod, attribs, attribCount, uniforms, uniformCount, settings);
}
extern "C" DescribedPipeline* LoadPipelineMod(DescribedShaderModule mod, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){

    DescribedPipeline* retp = callocnew(DescribedPipeline);
    retp->createdPipelines = callocnew(VertexStateToPipelineMap);
    new (retp->createdPipelines) VertexStateToPipelineMap;
    
    retp->settings = settings;
    DescribedPipeline& ret = *retp;
    ret.sh = mod;

    ret.vertexLayout = getBufferLayoutRepresentation(attribs, attribCount);    


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
extern "C" void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va){
    
    auto plquart = GetPipelinesForLayout(pipeline, va->attributes);
    //pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;

    //if(pipeline->pipeline)
    //    wgpuRenderPipelineRelease(pipeline->pipeline);
    //if(pipeline->pipeline_TriangleStrip)
    //    wgpuRenderPipelineRelease(pipeline->pipeline_TriangleStrip);
    //if(pipeline->pipeline_LineList)
    //    wgpuRenderPipelineRelease(pipeline->pipeline_LineList);

    pipeline->quartet = plquart;
    //pipeline->pipeline = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
    //pipeline->pipeline_LineList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    //pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
    //pipeline->pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    //pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
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
DescribedPipeline* ClonePipeline(const DescribedPipeline* _pipeline){
    DescribedPipeline* pipeline = callocnew(DescribedPipeline);
    pipeline->createdPipelines = callocnew(VertexStateToPipelineMap);
    new (pipeline->createdPipelines) VertexStateToPipelineMap;
    
    pipeline->settings = _pipeline->settings;
    //TODO: this incurs an erroneous shallow copy of a DescribedBindGroupLayout
    pipeline->layout = _pipeline->layout;
    pipeline->bglayout = LoadBindGroupLayout(_pipeline->bglayout.entries, _pipeline->bglayout.entryCount, false);

    pipeline->bindGroup = LoadBindGroup(&_pipeline->bglayout, _pipeline->bindGroup.entries, _pipeline->bindGroup.entryCount);
    for(uint32_t i = 0;i < _pipeline->bindGroup.entryCount;i++){
        //if(_pipeline->bindGroup.releaseOnClear & (1ull << i)){
        //    if(_pipeline->bindGroup.entries[i].buffer){
        //        if(_pipeline->bglayout.entries[i].type == uniform_buffer){
        //            pipeline->bindGroup.entries[i].buffer = cloneBuffer((WGPUBuffer)pipeline->bindGroup.entries[i].buffer, WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Uniform);
        //        }
        //        else if(_pipeline->bglayout.entries[i].type == storage_buffer){
        //            pipeline->bindGroup.entries[i].buffer = cloneBuffer((WGPUBuffer)pipeline->bindGroup.entries[i].buffer, WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Storage);
        //        }
        //    }
        //}
    }
    pipeline->vertexLayout = VertexBufferLayoutSet zeroinit;
    
    pipeline->vertexLayout.layouts = (VertexBufferLayout*)std::calloc(_pipeline->vertexLayout.number_of_buffers, sizeof(VertexBufferLayout));
    pipeline->vertexLayout.number_of_buffers = _pipeline->vertexLayout.number_of_buffers;
    uint32_t attributeCount = 0;
    for(size_t i = 0;i < pipeline->vertexLayout.number_of_buffers;i++){
        attributeCount += _pipeline->vertexLayout.layouts[i].attributeCount;
    }
    pipeline->vertexLayout.attributePool = (VertexAttribute*)std::calloc(attributeCount, sizeof(VertexAttribute));
    for(size_t i = 0;i < _pipeline->vertexLayout.number_of_buffers;i++){
        pipeline->vertexLayout.layouts[i].attributes = pipeline->vertexLayout.attributePool + (_pipeline->vertexLayout.layouts[i].attributes - _pipeline->vertexLayout.attributePool);
    }
    //TODO: this incurs lifetime problem, so do other things in this
    pipeline->sh = _pipeline->sh;

    UpdatePipeline(pipeline);
    
    //hm
    pipeline->sh.uniformLocations = callocnew(StringToUniformMap);
    new(pipeline->sh.uniformLocations) StringToUniformMap(*_pipeline->sh.uniformLocations);
    return pipeline;
}
DescribedPipeline* ClonePipelineWithSettings(const DescribedPipeline* pl, RenderSettings settings){
    DescribedPipeline* cloned = ClonePipeline(pl);

    cloned->settings = settings;
    UpdatePipeline(cloned);
    return cloned;
}
DescribedComputePipeline* LoadComputePipeline(const char* shaderCode){
    auto bindmap = getBindings(shaderCode);
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
    auto bindmap = getBindings(shaderCode);
    DescribedComputePipeline* ret = callocnew(DescribedComputePipeline);
    WGPUComputePipelineDescriptor desc{};
    WGPUPipelineLayoutDescriptor pldesc{};
    pldesc.bindGroupLayoutCount = 1;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, true);
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = (WGPUBindGroupLayout*) &ret->bglayout.layout;
    WGPUPipelineLayout playout = wgpuDeviceCreatePipelineLayout((WGPUDevice)GetDevice(), &pldesc);
    ret->shaderModule = LoadShaderModuleFromMemory(shaderCode);
    desc.compute.module = (WGPUShaderModule)ret->shaderModule.computeModule;
    desc.compute.entryPoint = STRVIEW("compute_main");
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

    shader.id = LoadPipelineGLSL(vertexSource, fragmentSource);
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
Texture GetDefaultTexture(cwoid){
    return g_renderstate.whitePixel;
}
RenderSettings GetDefaultSettings(){
    RenderSettings ret zeroinit;
    ret.faceCull = 1;
    ret.frontFace = FrontFace_CCW;
    ret.depthTest = 1;
    ret.depthCompare = CompareFunction_LessEqual;
    ret.sampleCount = (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1;

    ret.blendFactorSrcAlpha = BlendFactor_One;
    ret.blendFactorDstAlpha = BlendFactor_OneMinusSrcAlpha;
    ret.blendOperationAlpha = BlendOperation_Add;

    ret.blendFactorSrcColor = BlendFactor_SrcAlpha;
    ret.blendFactorDstColor = BlendFactor_OneMinusSrcAlpha;
    ret.blendOperationColor = BlendOperation_Add;
    
    return ret;
}
DescribedPipeline* DefaultPipeline(){
    return g_renderstate.defaultPipeline;
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



DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao){
    DescribedPipeline* klon = ClonePipeline(pl);
    PreparePipeline(klon, vao);
    return klon;
}


NativeBindgroupHandle UpdateAndGetNativeBindGroup(DescribedBindGroup* bg){
    if(bg->needsUpdate){
        UpdateBindGroup(bg);
        //bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &(bg->desc));
        bg->needsUpdate = false;
    }
    return bg->bindGroup;
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
    auto it = shaderModule.uniformLocations->uniforms.find(uniformName);
    if(it == shaderModule.uniformLocations->uniforms.end()){
        TRACELOG(LOG_ERROR, "Accessing nonexistent uniform %s", uniformName);
        return UniformAccessor{.index = LOCATION_NOT_FOUND, .bindgroup = nullptr};
    }
    uint32_t location = it->second.location;
    return UniformAccessor{.index = location, .bindgroup = &this->bindGroup};
}
