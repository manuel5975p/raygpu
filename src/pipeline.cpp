#include <raygpu.h>
#include <wgpustate.inc>
#include <cstring>
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
    return wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &bglayoutdesc);
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
                blayouts[i].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case storage_write_buffer:{
                blayouts[i].visibility = vvertexOnly;
                blayouts[i].buffer.type = WGPUBufferBindingType_Storage;
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case texture2d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = WGPUTextureSampleType_Float;
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case sampler:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            default:break;
        }
    }
    bglayoutdesc.entryCount = uniformCount;
    bglayoutdesc.entries = blayouts;
    ret.entries = blayouts;
    ret.layout = wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &bglayoutdesc);
    return ret;
}
DescribedPipeline* LoadPipelineForVAO(const char* shaderSource, VertexArray* vao, RenderSettings settings){
    std::unordered_map<std::string, UniformDescriptor> bindings = getBindings(shaderSource);
    std::vector<UniformDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    DescribedPipeline* pl = LoadPipelineForVAOEx(shaderSource, vao, values.data(), values.size(), settings);
    pl->uniformLocations = callocnew(StringToUniformMap);
    new (pl->uniformLocations) StringToUniformMap{};
    for(const auto& [x,y] : bindings){
        (*pl->uniformLocations).uniforms[x] = y;
    }
    return pl;
}
uint32_t GetUniformLocation(DescribedPipeline* pl, const char* uniformName){
    return pl->uniformLocations->GetLocation(uniformName);
}
DescribedPipeline* LoadPipelineForVAOEx(const char* shaderSource, VertexArray* vao, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}
DescribedPipeline* LoadPipeline(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, RenderSettings settings){
    std::unordered_map<std::string, UniformDescriptor> bindings = getBindings(shaderSource);
    std::vector<UniformDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    DescribedPipeline* pl = LoadPipelineEx(shaderSource, attribs, attribCount, values.data(), values.size(), settings);
    pl->uniformLocations = callocnew(StringToUniformMap);
    new (pl->uniformLocations) StringToUniformMap{};
    for(const auto& [x,y] : bindings){
        (*pl->uniformLocations).uniforms[x] = y;
    }
    return pl;
}
extern "C" DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){

    DescribedPipeline* retp = callocnew(DescribedPipeline);
    retp->settings = settings;
    DescribedPipeline& ret = *retp;
    ret.sh = LoadShaderFromMemory(shaderSource);

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
    ret.layout.layout = wgpuDeviceCreatePipelineLayout(g_wgpustate.device, &ret.layout.descriptor);
    
    WGPURenderPipelineDescriptor& pipelineDesc = ret.descriptor;
    pipelineDesc.multisample.count = settings.sampleCount_onlyApplicableIfMoreThanOne ? settings.sampleCount_onlyApplicableIfMoreThanOne : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = ret.layout.layout;
    
    WGPUVertexState vertexState{};
    vertexState.module = ret.sh;
    vertexState.bufferCount = number_of_buffers;
    vertexState.buffers = ret.vbLayouts;
    vertexState.constantCount = 0;
    vertexState.entryPoint = STRVIEW("vs_main");
    pipelineDesc.vertex = vertexState;

    ret.fragmentState = callocnew(WGPUFragmentState);

    pipelineDesc.fragment = ret.fragmentState;
    ret.fragmentState->module = ret.sh;
    ret.fragmentState->entryPoint = STRVIEW("fs_main");
    ret.fragmentState->constantCount = 0;
    ret.fragmentState->constants = nullptr;
    ret.blendState = new WGPUBlendState{};
    ret.blendState->color.srcFactor = WGPUBlendFactor_SrcAlpha;
    ret.blendState->color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    ret.blendState->color.operation = WGPUBlendOperation_Add;

    ret.blendState->alpha.srcFactor = WGPUBlendFactor_One;
    ret.blendState->alpha.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    ret.blendState->alpha.operation = WGPUBlendOperation_Add;
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
        ret.pipeline = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &ret.descriptor);
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
        ret.pipeline_LineList = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &ret.descriptor);
        ret.descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
        ret.descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
        ret.pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &ret.descriptor);
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

extern "C" void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va){
    size_t count = va->buffers.size();
    pipeline->vbLayouts = (WGPUVertexBufferLayout*) calloc(count, sizeof(WGPUVertexBufferLayout));
    
    pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;
    pipeline->descriptor.vertex.bufferCount = va->buffers.size();

    //LoadPipelineEx
    uint32_t attribCount = va->attributes.size();
    auto& attribs = va->attributes;
    uint32_t maxslot = 0;
    for(size_t i = 0;i < attribCount;i++){
        maxslot = std::max(maxslot, attribs[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    std::vector<std::vector<WGPUVertexAttribute>> buffer_to_attributes(number_of_buffers);
    //WGPUVertexBufferLayout* vbLayouts = new WGPUVertexBufferLayout[number_of_buffers];
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
        pipeline->vbLayouts[i].stepMode = va->buffers[i].second;
    }
    
    pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;
    if(pipeline->pipeline)
        wgpuRenderPipelineRelease(pipeline->pipeline);
    if(pipeline->pipeline_TriangleStrip)
        wgpuRenderPipelineRelease(pipeline->pipeline_TriangleStrip);
    if(pipeline->pipeline_LineList)
        wgpuRenderPipelineRelease(pipeline->pipeline_LineList);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline->pipeline = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_LineList;
    pipeline->pipeline_LineList = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
    pipeline->pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &pipeline->descriptor);
    pipeline->descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline->descriptor.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
}
DescribedPipeline* ClonePipeline(const DescribedPipeline* _pipeline){
    DescribedPipeline* pipeline = callocnew(DescribedPipeline);
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

    pipeline->blendState        = callocnew(WGPUBlendState);
    pipeline->fragmentState     = callocnew(WGPUFragmentState);
    pipeline->colorTarget       = callocnew(WGPUColorTargetState);
    pipeline->depthStencilState = callocnew(WGPUDepthStencilState);
    pipeline->sh = _pipeline->sh;
    memcpy(pipeline->blendState, _pipeline->blendState, sizeof(WGPUBlendState));
    memcpy(pipeline->fragmentState, _pipeline->fragmentState, sizeof(WGPUFragmentState));
    memcpy(pipeline->colorTarget, _pipeline->colorTarget, sizeof(WGPUColorTargetState));
    memcpy(pipeline->depthStencilState, _pipeline->depthStencilState, sizeof(WGPUDepthStencilState));
    pipeline->descriptor.vertex.module = pipeline->sh;
    pipeline->fragmentState->module = pipeline->sh;
    pipeline->colorTarget->blend = pipeline->blendState;
    pipeline->fragmentState->targets = pipeline->colorTarget;
    pipeline->bglayout.layout = wgpuDeviceCreateBindGroupLayout(GetDevice(), &pipeline->bglayout.descriptor);
    pipeline->layout.descriptor = _pipeline->layout.descriptor;
    pipeline->layout.descriptor.bindGroupLayouts = &pipeline->bglayout.layout;
    pipeline->layout.layout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pipeline->layout.descriptor);
    pipeline->bindGroup.desc.entries = pipeline->bindGroup.entries;
    pipeline->bindGroup.bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &pipeline->bindGroup.desc);
    pipeline->descriptor.depthStencil = pipeline->settings.depthTest ? pipeline->depthStencilState : nullptr;
    pipeline->descriptor.fragment = pipeline->fragmentState;
    pipeline->descriptor.layout = pipeline->layout.layout;
    pipeline->pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
    return pipeline;
}

DescribedComputePipeline* LoadComputePipeline(const char* shaderCode, const UniformDescriptor* uniforms, uint32_t uniformCount){
    DescribedComputePipeline* ret = callocnew(DescribedComputePipeline);
    WGPUComputePipelineDescriptor& desc = ret->desc;
    WGPUPipelineLayoutDescriptor pldesc{};
    pldesc.bindGroupLayoutCount = 1;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, true);
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = &ret->bglayout.layout;
    WGPUPipelineLayout playout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pldesc);
    desc.compute.module = LoadShaderFromMemory(shaderCode);
    desc.compute.entryPoint = STRVIEW("compute_main");
    desc.layout = playout;
    ret->pipeline = wgpuDeviceCreateComputePipeline(GetDevice(), &ret->desc);
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
    free(pl->uniformLocations);
    free(pl->blendState);
    free(pl->colorTarget);
    free(pl->depthStencilState);
}
uint64_t bgEntryHash(const WGPUBindGroupEntry& bge){
    const uint32_t rotation = (bge.binding) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}

extern "C" DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const WGPUBindGroupEntry* entries, size_t entryCount){
    DescribedBindGroup ret zeroinit;
    WGPUBindGroupEntry* rentries = (WGPUBindGroupEntry*) calloc(entryCount, sizeof(WGPUBindGroupEntry));
    memcpy(rentries, entries, entryCount * sizeof(WGPUBindGroupEntry));
    ret.entries = rentries;
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
        TRACELOG(LOG_WARNING, "Tried to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->desc.entryCount);
        return;
    }
    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);
    bg->entries[index] = entry;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);

    //TODO don't release and recreate here or find something better lol
    
    if(!bg->needsUpdate && bg->bindGroup){
        //wgpuBindGroupRelease(bg->bindGroup);
        g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
        bg->bindGroup = nullptr;
    }
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