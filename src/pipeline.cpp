#include <algorithm>
#include <raygpu.h>
#include <webgpu/webgpu_cpp.h>
#include <wgpustate.inc>
#include <cstring>
#include <cassert>
#include <iostream>

typedef struct StringToUniformMap{
    std::unordered_map<std::string, UniformDescriptor> uniforms;
    UniformDescriptor operator[](const std::string& v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const std::string& v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
    }
    UniformDescriptor operator[](const char* v)const noexcept{
        return uniforms.find(v)->second;
    }
    uint32_t GetLocation(const char* v)const noexcept{
        auto it = uniforms.find(v);
        if(it == uniforms.end())
            return LOCATION_NOT_FOUND;
        return it->second.location;
        
    }
}StringToUniformMap;
inline WGPUStorageTextureAccess toStorageTextureAccess(access_type acc){
    switch(acc){
        case access_type::readonly:return WGPUStorageTextureAccess_ReadOnly;
        case access_type::readwrite:return WGPUStorageTextureAccess_ReadWrite;
        case access_type::writeonly:return WGPUStorageTextureAccess_WriteOnly;
        default: TRACELOG(LOG_FATAL, "Invalid enum type");
    }
    return WGPUStorageTextureAccess_Force32;
}
inline WGPUBufferBindingType toStorageBufferAccess(access_type acc){
    switch(acc){
        case access_type::readonly: return WGPUBufferBindingType_ReadOnlyStorage;
        case access_type::readwrite:return WGPUBufferBindingType_Storage;
        case access_type::writeonly:return WGPUBufferBindingType_Storage;
        default: TRACELOG(LOG_FATAL, "Invalid enum type");
    }
    return WGPUBufferBindingType_Force32;
}
inline WGPUTextureFormat toStorageTextureFormat(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::format_r32float: return WGPUTextureFormat_R32Float;
        case format_or_sample_type::format_r32uint: return WGPUTextureFormat_R32Uint;
        case format_or_sample_type::format_rgba8unorm: return WGPUTextureFormat_RGBA8Unorm;
        case format_or_sample_type::format_rgba32float: return WGPUTextureFormat_RGBA32Float;
        default: TRACELOG(LOG_FATAL, "Invalid enum type");
    }
    return WGPUTextureFormat_Force32;
}
inline WGPUTextureSampleType toTextureSampleType(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::sample_f32: return WGPUTextureSampleType_Float;
        case format_or_sample_type::sample_u32: return WGPUTextureSampleType_Uint;
        default: TRACELOG(LOG_FATAL, "Invalid enum type");
    }
    return WGPUTextureSampleType_Force32;
}




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
    return wgpuDeviceCreateBindGroupLayout(GetDevice(), &bglayoutdesc);
}*/

DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount, bool compute){
    DescribedBindGroupLayout ret{};
    WGPUShaderStage visible;
    WGPUShaderStage vfragmentOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Fragment;
    WGPUShaderStage vvertexOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Vertex;
    if(compute){
        visible = WGPUShaderStage_Compute;
    }
    else{
        visible = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    }
    WGPUBindGroupLayoutEntry* blayouts = (WGPUBindGroupLayoutEntry*)malloc(uniformCount * sizeof(WGPUBindGroupLayoutEntry));
    WGPUBindGroupLayoutDescriptor& bglayoutdesc = ret.descriptor;
    std::memset(blayouts, 0, uniformCount * sizeof(WGPUBindGroupLayoutEntry));
    for(size_t i = 0;i < uniformCount;i++){
        blayouts[i].binding = uniforms[i].location;
        switch(uniforms[i].type){
            case uniform_buffer:
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                blayouts[i].buffer.minBindingSize = uniforms[i].minBindingSize;
            break;
            case storage_buffer:{
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = toStorageBufferAccess(uniforms[i].access);
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case texture2d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(uniforms[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case sampler:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            case texture3d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(uniforms[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_3D;
            break;
            case storage_texture2d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case storage_texture3d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_3D;
            break;
        }
    }
    bglayoutdesc.entryCount = uniformCount;
    bglayoutdesc.entries = blayouts;
    ret.entries = blayouts;
    ret.layout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &bglayoutdesc);
    return ret;
}
extern "C" DescribedPipeline* LoadPipelineForVAO(const char* shaderSource, VertexArray* vao){
    std::unordered_map<std::string, UniformDescriptor> bindings = getBindings(shaderSource);
    std::vector<UniformDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const UniformDescriptor& x, const UniformDescriptor& y){
        return x.location < y.location;
    });
    DescribedPipeline* pl = LoadPipelineForVAOEx(shaderSource, vao, values.data(), values.size(), GetDefaultSettings());

    return pl;
}
uint32_t GetUniformLocation(DescribedPipeline* pl, const char* uniformName){
    return pl->sh.uniformLocations->GetLocation(uniformName);
}
uint32_t GetUniformLocationCompute(DescribedComputePipeline* pl, const char* uniformName){
    return pl->shaderModule.uniformLocations->GetLocation(uniformName);
}
DescribedPipeline* LoadPipelineForVAOEx(const char* shaderSource, VertexArray* vao, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}
extern "C" DescribedPipeline* LoadPipeline(const char* shaderSource){
    std::unordered_map<std::string, UniformDescriptor> bindings = getBindings(shaderSource);

    std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> attribs = getAttributes(shaderSource);
    
    std::vector<AttributeAndResidence> allAttribsInOneBuffer;
    allAttribsInOneBuffer.reserve(attribs.size());
    uint32_t offset = 0;
    for(const auto& [name, attr] : attribs){
        const auto& [format, location] = attr;
        allAttribsInOneBuffer.push_back(AttributeAndResidence{
            .attr = WGPUVertexAttribute{
                .format = format,
                .offset = offset,
                .shaderLocation = location},
            .bufferSlot = 0,
            .stepMode = WGPUVertexStepMode_Vertex,
            .enabled = true}
        );
        offset += attributeSize(format);
    }
    std::vector<UniformDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const UniformDescriptor& x, const UniformDescriptor& y){
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
    WGPUShaderModule sh = wgpuDeviceCreateShaderModule(GetDevice(), &shaderDesc);
    DescribedShaderModule ret zeroinit;
    ret.shaderModule = sh;
    ret.source = calloc(codeSizeInBytes / sizeof(uint32_t), sizeof(uint32_t));
    std::memcpy(const_cast<void*>(ret.source), shaderCodeSPIRV, codeSizeInBytes);
    ret.sourceLengthInBytes = codeSizeInBytes;
    ret.uniformLocations = callocnew(StringToUniformMap);
    new (ret.uniformLocations) StringToUniformMap;
    return ret;
}
DescribedShaderModule LoadShaderModuleFromMemory(const char* shaderSourceWGSL) {
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};

    size_t sourceSize = strlen(shaderSourceWGSL);

    std::unordered_map<std::string, UniformDescriptor> bindings = getBindings(shaderSourceWGSL);
    
    std::vector<UniformDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const UniformDescriptor& x, const UniformDescriptor& y){
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
    WGPUShaderModule mod = wgpuDeviceCreateShaderModule(GetDevice(), &shaderDesc);
    DescribedShaderModule ret zeroinit;
    ret.sourceType = sourceTypeWGSL;
    ret.uniformLocations = callocnew(StringToUniformMap);
    new (ret.uniformLocations) StringToUniformMap;
    for(const auto& [x,y] : bindings){
        ret.uniformLocations->uniforms[x] = y;
    }
    void* source = std::calloc(sourceSize + 1, 1);
    std::memcpy(source, shaderSourceWGSL, sourceSize);
    ret.shaderModule = mod;
    ret.sourceLengthInBytes = sourceSize;
    ret.source = source;
    return ret;
}
void UnloadShaderModule(DescribedShaderModule mod){
    //hmmmmmmmmmmmmmmmmmmmmmmmmmm
    //const shouldn't exist!!1!
    free(const_cast<void*>(mod.source));
}
extern "C" DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedShaderModule mod = LoadShaderModuleFromMemory(shaderSource);
    return LoadPipelineMod(mod, attribs, attribCount, uniforms, uniformCount, settings);
}
extern "C" DescribedPipeline* LoadPipelineMod(DescribedShaderModule mod, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){

    DescribedPipeline* retp = callocnew(DescribedPipeline);
    retp->createdPipelines = callocnew(VertexStateToPipelineMap);
    new (retp->createdPipelines) VertexStateToPipelineMap;
    
    retp->settings = settings;
    DescribedPipeline& ret = *retp;
    ret.sh = mod;

    uint32_t maxslot = 0;
    for(size_t i = 0;i < attribCount;i++){
        maxslot = std::max(maxslot, attribs[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    std::vector<std::vector<WGPUVertexAttribute>> buffer_to_attributes(number_of_buffers);
    ret.vbLayouts = new WGPUVertexBufferLayout[number_of_buffers];
    std::vector<uint32_t> strides(number_of_buffers, 0);
    std::vector<uint32_t> attrIndex(number_of_buffers, 0);
    for(size_t i = 0;i < attribCount;i++){
        buffer_to_attributes[attribs[i].bufferSlot].push_back(attribs[i].attr);
        strides[attribs[i].bufferSlot] += attributeSize(attribs[i].attr.format);
    }
    ret.attributePool = (WGPUVertexAttribute*) calloc(attribCount, sizeof(WGPUVertexAttribute));
    //std::cout << "Allocating " << attribCount << " attributes\n";
    uint32_t poolOffset = 0;
    std::vector<uint32_t> attributeOffsets(number_of_buffers, 0);

    for(size_t i = 0;i < number_of_buffers;i++){
        attributeOffsets[i] = poolOffset;
        memcpy(ret.attributePool + poolOffset, buffer_to_attributes[i].data(), buffer_to_attributes[i].size() * sizeof(WGPUVertexAttribute));
        poolOffset += buffer_to_attributes[i].size();
    }
    

    for(size_t i = 0;i < number_of_buffers;i++){
        ret.vbLayouts[i].attributes = ret.attributePool + attributeOffsets[i];
        ret.vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        ret.vbLayouts[i].arrayStride = strides[i];
        ret.vbLayouts[i].stepMode = WGPUVertexStepMode_Vertex;
    }

    ret.bglayout = LoadBindGroupLayout(uniforms, uniformCount, false);
    std::vector<WGPUBindGroupEntry> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = WGPUBindGroupEntry{};
        bge[i].binding = uniforms[i].location;
    }
    ret.bindGroup = LoadBindGroup(&ret.bglayout, bge.data(), bge.size());
    ret.layout.descriptor = WGPUPipelineLayoutDescriptor{};
    ret.layout.descriptor.bindGroupLayoutCount = 1;
    ret.layout.descriptor.bindGroupLayouts = &ret.bglayout.layout;
    ret.layout.layout = wgpuDeviceCreatePipelineLayout(GetDevice(), &ret.layout.descriptor);
    
    WGPURenderPipelineDescriptor& pipelineDesc = ret.descriptor;
    pipelineDesc.multisample.count = settings.sampleCount ? settings.sampleCount : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = ret.layout.layout;
    
    WGPUVertexState vertexState{};
    vertexState.module = ret.sh.shaderModule;
    vertexState.bufferCount = number_of_buffers;
    vertexState.buffers = ret.vbLayouts;
    vertexState.constantCount = 0;
    vertexState.entryPoint = STRVIEW("vs_main");
    pipelineDesc.vertex = vertexState;

    ret.fragmentState = callocnew(WGPUFragmentState);

    pipelineDesc.fragment = ret.fragmentState;
    ret.fragmentState->module = ret.sh.shaderModule;
    ret.fragmentState->entryPoint = STRVIEW("fs_main");
    ret.fragmentState->constantCount = 0;
    ret.fragmentState->constants = nullptr;
    ret.blendState = new WGPUBlendState{};
    ret.blendState->color.srcFactor = settings.blendFactorSrcColor;
    ret.blendState->color.dstFactor = settings.blendFactorDstColor;
    ret.blendState->color.operation = settings.blendOperationColor;

    ret.blendState->alpha.srcFactor = settings.blendFactorSrcAlpha;
    ret.blendState->alpha.dstFactor = settings.blendFactorDstAlpha;
    ret.blendState->alpha.operation = settings.blendOperationAlpha;
    ret.colorTarget = callocnew(WGPUColorTargetState);
    ret.colorTarget->format = g_wgpustate.frameBufferFormat;
    ret.colorTarget->blend = ret.blendState;
    ret.colorTarget->writeMask = WGPUColorWriteMask_All;
    ret.fragmentState->targetCount = 1;
    ret.fragmentState->targets = ret.colorTarget;
    pipelineDesc.fragment = ret.fragmentState;
    // We setup a depth buffer state for the render pipeline
    ret.depthStencilState = nullptr;
    if(settings.depthTest){
        ret.depthStencilState = callocnew(WGPUDepthStencilState);
        // Keep a fragment only if its depth is lower than the previously blended one
        ret.depthStencilState->depthCompare = settings.depthCompare;
        // Each time a fragment is blended into the target, we update the value of the Z-buffer
        ret.depthStencilState->depthWriteEnabled = WGPUOptionalBool_True;
        // Store the format in a variable as later parts of the code depend on it
        WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
        ret.depthStencilState->format = depthTextureFormat;
        // Deactivate the stencil alltogether
        ret.depthStencilState->stencilReadMask = 0;
        ret.depthStencilState->stencilWriteMask = 0;
        ret.depthStencilState->stencilFront.compare = WGPUCompareFunction_Always;
        ret.depthStencilState->stencilBack.compare = WGPUCompareFunction_Always;
    }
    pipelineDesc.depthStencil = settings.depthTest ? ret.depthStencilState : nullptr;
    ret.descriptor.primitive.frontFace = settings.frontFace;
    ret.descriptor.primitive.cullMode = settings.faceCull ? WGPUCullMode_Back : WGPUCullMode_None;
    if(attribCount != 0){
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        ret.pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &ret.descriptor);
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
        ret.pipeline_LineList = wgpuDeviceCreateRenderPipeline(GetDevice(), &ret.descriptor);
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
        ret.descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
        ret.pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline(GetDevice(), &ret.descriptor);
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
        ret.descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    }
    //TRACELOG(LOG_INFO, "Pipeline with %d attributes and %d uniforms was loaded", attribCount, uniformCount);
    return retp;
}
typedef struct VertexArray{
    std::vector<AttributeAndResidence> attributes;
    std::vector<std::pair<DescribedBuffer*, WGPUVertexStepMode>> buffers;
}VertexArray;

std::vector<AttributeAndResidence> GetAttributesAndResidences(const VertexArray* va){
    return va->attributes;
}
PipelineTriplet GetPipelinesForLayout(DescribedPipeline* pipeline, const std::vector<AttributeAndResidence>& attribs){
    uint32_t attribCount = attribs.size();
    auto it = pipeline->createdPipelines->pipelines.find(attribs);
    if(it != pipeline->createdPipelines->pipelines.end()){
        //TRACELOG(LOG_INFO, "Reusing cached pipeline triplet");
        return it->second;
    }
    TRACELOG(LOG_DEBUG, "Creating new pipeline triplet");
    uint32_t maxslot = 0;
    for(size_t i = 0;i < attribCount;i++){
        maxslot = std::max(maxslot, attribs[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    pipeline->vbLayouts = (WGPUVertexBufferLayout*) calloc(number_of_buffers, sizeof(WGPUVertexBufferLayout)); 
    pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;
    std::vector<WGPUVertexStepMode> stepmodes(number_of_buffers, WGPUVertexStepMode_Undefined);

    for(size_t i = 0;i < attribs.size();i++){
        if(stepmodes[attribs[i].bufferSlot] == WGPUVertexStepMode_Undefined){
            stepmodes[attribs[i].bufferSlot] = attribs[i].stepMode;
        }
        else if(stepmodes[attribs[i].bufferSlot] != attribs[i].stepMode){
            TRACELOG(LOG_ERROR, "Mixed stepmodes");
            return PipelineTriplet{};
        }
    }
    pipeline->descriptor.vertex.bufferCount = number_of_buffers;
    //auto& attribs = va->attributes;
    std::vector<std::vector<WGPUVertexAttribute>> buffer_to_attributes(number_of_buffers);
    std::vector<uint32_t> strides  (number_of_buffers, 0);
    std::vector<uint32_t> attrIndex(number_of_buffers, 0);
    for(size_t i = 0;i < attribCount;i++){
        buffer_to_attributes[attribs[i].bufferSlot].push_back(attribs[i].attr);
        strides[attribs[i].bufferSlot] += attributeSize(attribs[i].attr.format);
    }
    
    for(size_t i = 0;i < number_of_buffers;i++){
        pipeline->vbLayouts[i].attributes = buffer_to_attributes[i].data();
        pipeline->vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        pipeline->vbLayouts[i].arrayStride = strides[i];
        pipeline->vbLayouts[i].stepMode = attribs[i].stepMode;
    }
    PipelineTriplet ret{};
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    ret.pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
    ret.pipeline_LineList = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
    ret.pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipeline->createdPipelines->pipelines[attribs] = ret;
    return ret;
}
extern "C" void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va){
    
    auto pltriptle = GetPipelinesForLayout(pipeline, va->attributes);
    //pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;

    //if(pipeline->pipeline)
    //    wgpuRenderPipelineRelease(pipeline->pipeline);
    //if(pipeline->pipeline_TriangleStrip)
    //    wgpuRenderPipelineRelease(pipeline->pipeline_TriangleStrip);
    //if(pipeline->pipeline_LineList)
    //    wgpuRenderPipelineRelease(pipeline->pipeline_LineList);

    pipeline->pipeline = pltriptle.pipeline;
    pipeline->pipeline_LineList = pltriptle.pipeline_LineList;
    pipeline->pipeline_TriangleStrip = pltriptle.pipeline_TriangleStrip;
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    //pipeline->pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
    //pipeline->pipeline_LineList = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    //pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
    //pipeline->pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    //pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    //pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
}
WGPUBuffer cloneBuffer(WGPUBuffer b, WGPUBufferUsage usage){
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    WGPUBufferDescriptor retd{};
    retd.usage = usage;
    retd.size = wgpuBufferGetSize(b);
    WGPUBuffer ret = wgpuDeviceCreateBuffer(GetDevice(), &retd);
    wgpuCommandEncoderCopyBufferToBuffer(enc, b, 0, ret, 0, retd.size);
    WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(enc, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &buffer);
    return ret;
}
DescribedPipeline* ClonePipeline(const DescribedPipeline* _pipeline){
    DescribedPipeline* pipeline = callocnew(DescribedPipeline);
    pipeline->createdPipelines = callocnew(VertexStateToPipelineMap);
    new (pipeline->createdPipelines) VertexStateToPipelineMap;
    pipeline->descriptor = _pipeline->descriptor;
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipeline->settings = _pipeline->settings;
    pipeline->bglayout.descriptor = _pipeline->bglayout.descriptor;
    pipeline->lastUsedAs = _pipeline->lastUsedAs;
    pipeline->bglayout.entries = (WGPUBindGroupLayoutEntry*)calloc(_pipeline->bglayout.descriptor.entryCount, sizeof(WGPUBindGroupLayoutEntry));
    memcpy(pipeline->bglayout.entries, _pipeline->bglayout.descriptor.entries, _pipeline->bglayout.descriptor.entryCount * sizeof(WGPUBindGroupLayoutEntry));
    pipeline->bglayout.descriptor.entries = pipeline->bglayout.entries;
    pipeline->descriptor.vertex = _pipeline->descriptor.vertex;
    pipeline->vbLayouts = (WGPUVertexBufferLayout*)calloc(_pipeline->descriptor.vertex.bufferCount, sizeof(WGPUVertexBufferLayout));
    memcpy(pipeline->vbLayouts, _pipeline->vbLayouts, _pipeline->descriptor.vertex.bufferCount * sizeof(WGPUVertexBufferLayout));
    
    pipeline->descriptor.vertex.buffers  = pipeline->vbLayouts;
    pipeline->bindGroup.desc = _pipeline->bindGroup.desc;
    pipeline->bindGroup.entries = (WGPUBindGroupEntry*)calloc(_pipeline->bindGroup.desc.entryCount, sizeof(WGPUBindGroupEntry));
    memcpy(pipeline->bindGroup.entries, _pipeline->bindGroup.entries, _pipeline->bindGroup.desc.entryCount * sizeof(WGPUBindGroupEntry));

    for(uint32_t i = 0;i < _pipeline->bindGroup.desc.entryCount;i++){
        if(_pipeline->bindGroup.releaseOnClear & (1ull << i)){
            if(_pipeline->bindGroup.entries[i].buffer){
                if(_pipeline->bglayout.entries[i].buffer.type == WGPUBufferBindingType_Uniform){
                    pipeline->bindGroup.entries[i].buffer = cloneBuffer(pipeline->bindGroup.entries[i].buffer, WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Uniform);
                }
                else if(_pipeline->bglayout.entries[i].buffer.type == WGPUBufferBindingType_Storage){
                    pipeline->bindGroup.entries[i].buffer = cloneBuffer(pipeline->bindGroup.entries[i].buffer, WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Storage);
                }
                else if(_pipeline->bglayout.entries[i].buffer.type == WGPUBufferBindingType_ReadOnlyStorage){
                    pipeline->bindGroup.entries[i].buffer = cloneBuffer(pipeline->bindGroup.entries[i].buffer, WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc | WGPUBufferUsage_Storage);
                }
            }
        }
    }
    pipeline->bindGroup.desc.entries = pipeline->bindGroup.entries;

    pipeline->blendState        = callocnew(WGPUBlendState);
    pipeline->fragmentState     = callocnew(WGPUFragmentState);
    pipeline->colorTarget       = callocnew(WGPUColorTargetState);
    pipeline->depthStencilState = callocnew(WGPUDepthStencilState);
    //TODO: this incurs lifetime problem, so do other things in this
    pipeline->sh = _pipeline->sh;
    memcpy(pipeline->blendState, _pipeline->blendState, sizeof(WGPUBlendState));
    memcpy(pipeline->fragmentState, _pipeline->fragmentState, sizeof(WGPUFragmentState));
    memcpy(pipeline->colorTarget, _pipeline->colorTarget, sizeof(WGPUColorTargetState));
    memcpy(pipeline->depthStencilState, _pipeline->depthStencilState, sizeof(WGPUDepthStencilState));
    pipeline->descriptor.vertex.module = pipeline->sh.shaderModule;
    pipeline->fragmentState->module = pipeline->sh.shaderModule;
    pipeline->colorTarget->blend = pipeline->blendState;
    pipeline->fragmentState->targets = pipeline->colorTarget;
    pipeline->bglayout.layout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &pipeline->bglayout.descriptor);
    pipeline->layout.descriptor = _pipeline->layout.descriptor;
    pipeline->layout.descriptor.bindGroupLayouts = &pipeline->bglayout.layout;
    pipeline->layout.layout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pipeline->layout.descriptor);
    pipeline->bindGroup.desc.entries = pipeline->bindGroup.entries;
    pipeline->bindGroup.needsUpdate = true;
    //pipeline->bindGroup.bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &pipeline->bindGroup.desc);
    pipeline->descriptor.depthStencil = pipeline->settings.depthTest ? pipeline->depthStencilState : nullptr;
    pipeline->descriptor.fragment = pipeline->fragmentState;
    pipeline->descriptor.layout = pipeline->layout.layout;
    pipeline->pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);


    pipeline->sh.uniformLocations = callocnew(StringToUniformMap);
    new(pipeline->sh.uniformLocations) StringToUniformMap(*_pipeline->sh.uniformLocations);
    return pipeline;
}
DescribedPipeline* ClonePipelineWithSettings(const DescribedPipeline* pl, RenderSettings settings){
    DescribedPipeline* cloned = ClonePipeline(pl);
    cloned->settings = settings;
    cloned->descriptor.multisample.count = settings.sampleCount;
    cloned->blendState->color.srcFactor = settings.blendFactorSrcColor;
    cloned->blendState->color.dstFactor = settings.blendFactorDstColor;
    cloned->blendState->color.operation = settings.blendOperationColor;

    cloned->blendState->alpha.srcFactor = settings.blendFactorSrcAlpha;
    cloned->blendState->alpha.dstFactor = settings.blendFactorDstAlpha;
    cloned->blendState->alpha.operation = settings.blendOperationAlpha;
    cloned->descriptor.primitive.cullMode = settings.faceCull ? WGPUCullMode_Back : WGPUCullMode_None;
    cloned->descriptor.primitive.frontFace = settings.frontFace;
    cloned->descriptor.depthStencil = settings.depthTest ? cloned->depthStencilState : nullptr;
    cloned->depthStencilState->depthCompare = settings.depthCompare;
    wgpuRenderPipelineRelease(cloned->pipeline);
    cloned->pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &cloned->descriptor);
    return cloned;
}
DescribedComputePipeline* LoadComputePipeline(const char* shaderCode){
    auto bindmap = getBindings(shaderCode);
    std::vector<UniformDescriptor> udesc;
    for(auto& [x,y] : bindmap){
        udesc.push_back(y);
    }
    std::sort(udesc.begin(), udesc.end(), [](const UniformDescriptor& x, const UniformDescriptor& y){
        return x.location < y.location;
    });
    return LoadComputePipelineEx(shaderCode, udesc.data(), udesc.size());
    
}
DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const UniformDescriptor* uniforms, uint32_t uniformCount){
    auto bindmap = getBindings(shaderCode);
    DescribedComputePipeline* ret = callocnew(DescribedComputePipeline);
    WGPUComputePipelineDescriptor& desc = ret->desc;
    WGPUPipelineLayoutDescriptor pldesc{};
    pldesc.bindGroupLayoutCount = 1;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, true);
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = &ret->bglayout.layout;
    WGPUPipelineLayout playout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pldesc);
    ret->shaderModule = LoadShaderModuleFromMemory(shaderCode);
    desc.compute.module = ret->shaderModule.shaderModule;
    desc.compute.entryPoint = STRVIEW("compute_main");
    desc.layout = playout;
    ret->pipeline = wgpuDeviceCreateComputePipeline(GetDevice(), &ret->desc);
    std::vector<WGPUBindGroupEntry> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = WGPUBindGroupEntry{};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge.data(), bge.size());
    return ret;
}
Texture GetDefaultTexture(cwoid){
    return g_wgpustate.whitePixel;
}
RenderSettings GetDefaultSettings(){
    RenderSettings ret zeroinit;
    ret.faceCull = 1;
    ret.frontFace = WGPUFrontFace_CCW;
    ret.depthTest = 1;
    ret.depthCompare = WGPUCompareFunction_LessEqual;
    ret.sampleCount = (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1;

    ret.blendFactorSrcAlpha = WGPUBlendFactor_One;
    ret.blendFactorDstAlpha = WGPUBlendFactor_OneMinusSrcAlpha;
    ret.blendOperationAlpha = WGPUBlendOperation_Add;

    ret.blendFactorSrcColor = WGPUBlendFactor_SrcAlpha;
    ret.blendFactorDstColor = WGPUBlendFactor_OneMinusSrcAlpha;
    ret.blendOperationColor = WGPUBlendOperation_Add;
    
    return ret;
}
DescribedPipeline* DefaultPipeline(){
    return g_wgpustate.rstate->defaultPipeline;
}
extern "C" void UnloadPipeline(DescribedPipeline* pl){
    
    wgpuPipelineLayoutRelease(pl->layout.layout);
    UnloadBindGroup(&pl->bindGroup);
    UnloadBindGroupLayout(&pl->bglayout);
    wgpuRenderPipelineRelease(pl->pipeline);
    wgpuRenderPipelineRelease(pl->pipeline_LineList);
    wgpuRenderPipelineRelease(pl->pipeline_TriangleStrip);
    free(pl->attributePool);
    UnloadShaderModule(pl->sh);
    free(pl->blendState);
    free(pl->colorTarget);
    free(pl->depthStencilState);
}
uint64_t bgEntryHash(const WGPUBindGroupEntry& bge){
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}

extern "C" DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const WGPUBindGroupEntry* entries, size_t entryCount){
    DescribedBindGroup ret zeroinit;
    ret.entries = (WGPUBindGroupEntry*) calloc(entryCount, sizeof(WGPUBindGroupEntry));
    memcpy(ret.entries, entries, entryCount * sizeof(WGPUBindGroupEntry));
    ret.desc.layout = bglayout->layout;
    ret.desc.entries = ret.entries;
    ret.desc.entryCount = entryCount;
    ret.needsUpdate = true;
    ret.descriptorHash = 0;
    for(uint32_t i = 0;i < ret.desc.entryCount;i++){
        ret.descriptorHash ^= bgEntryHash(ret.entries[i]);
    }
    //ret.bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &ret.desc);
    return ret;
}
extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, WGPUBindGroupEntry entry){
    if(index >= bg->desc.entryCount){
        TRACELOG(LOG_WARNING, "Trying to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->desc.entryCount);
        //return;
    }
    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);
    //bool donotcache = false;
    if(bg->releaseOnClear & (1 << index)){
        //donotcache = true;
        if(bg->entries[index].buffer){
            wgpuBufferRelease(bg->entries[index].buffer);
        }
        else if(bg->entries[index].textureView){
            //Todo: currently not the case anyway, but this is nadinöf
            wgpuTextureViewRelease(bg->entries[index].textureView);
        }
        else if(bg->entries[index].sampler){
            wgpuSamplerRelease(bg->entries[index].sampler);
        }
        bg->releaseOnClear &= ~(1 << index);
    }
    bg->entries[index] = entry;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);

    //TODO don't release and recreate here or find something better
    if(true /*|| donotcache*/){
        if(bg->bindGroup)
            wgpuBindGroupRelease(bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    //else if(!bg->needsUpdate && bg->bindGroup){
    //    g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //    bg->bindGroup = nullptr;
    //}
    bg->needsUpdate = true;
    
    //bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
}
DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao){
    DescribedPipeline* klon = ClonePipeline(pl);
    PreparePipeline(klon, vao);
    return klon;
}

extern "C" void UpdateBindGroup(DescribedBindGroup* bg){
    bg->desc.nextInChain = nullptr;
    //std::cout << "Updating bindgroup with " << bg->desc.entryCount << " entries" << std::endl;
    //std::cout << "Updating bindgroup with " << bg->desc.entries[1].binding << " entries" << std::endl;
    if(bg->needsUpdate){
        auto it = g_wgpustate.bindGroupPool.find(bg->descriptorHash);
        if(it != g_wgpustate.bindGroupPool.end()){
            bg->bindGroup = it->second;
        }
        else{
            //TRACELOG(LOG_WARNING, "a ooo, create bind grupp");
            //__builtin_dump_struct(&(bg->desc), printf);
            //for(size_t i = 0;i < bg->desc.entryCount;i++)
            //    __builtin_dump_struct(&(bg->desc.entries[i]), printf);
            bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
        }
        bg->needsUpdate = false;
    }
}
WGPUBindGroup GetWGPUBindGroup(DescribedBindGroup* bg){
    if(bg->needsUpdate){
        UpdateBindGroup(bg);
        //bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
        //bg->needsUpdate = false;
    }
    return bg->bindGroup;
}
extern "C" void UnloadBindGroup(DescribedBindGroup* bg){
    free(bg->entries);
    wgpuBindGroupRelease(bg->bindGroup);
}
extern "C" void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout){
    free(bglayout->entries);
    wgpuBindGroupLayoutRelease(bglayout->layout);
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