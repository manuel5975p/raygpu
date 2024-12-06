#include <raygpu.h>
#include <wgpustate.inc>

#include <cstring>
#include <iostream>
WGPUBindGroupLayout bindGroupLayoutFromUniformTypes(const UniformDescriptor* uniforms, uint32_t uniformCount){
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
}

DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount){
    DescribedBindGroupLayout ret{};
    
    WGPUBindGroupLayoutEntry* blayouts = (WGPUBindGroupLayoutEntry*)malloc(uniformCount * sizeof(WGPUBindGroupLayoutEntry));
    WGPUBindGroupLayoutDescriptor& bglayoutdesc = ret.descriptor;
    std::memset(blayouts, 0, uniformCount * sizeof(WGPUBindGroupLayoutEntry));
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
    bglayoutdesc.entries = blayouts;
    ret.entries = blayouts;
    ret.layout = wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &bglayoutdesc);
    return ret;
}



extern "C" DescribedPipeline LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount){
    
    WGPUShaderModule shader = LoadShaderFromMemory(shaderSource);

    DescribedPipeline ret;

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
    
    for(size_t i = 0;i < number_of_buffers;i++){
        ret.vbLayouts[i].attributes = buffer_to_attributes[i].data();
        ret.vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        ret.vbLayouts[i].arrayStride = strides[i];
        ret.vbLayouts[i].stepMode = WGPUVertexStepMode_Vertex;
    }

    ret.bglayout = LoadBindGroupLayout(uniforms, uniformCount);
    std::vector<WGPUBindGroupEntry> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = WGPUBindGroupEntry{};
        bge[i].binding = i;
    }
    ret.bindGroup = LoadBindGroup(&ret, bge.data(), bge.size());
    WGPUPipelineLayout playout;
    WGPUPipelineLayoutDescriptor pldesc{};
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = &ret.bglayout.layout;    
    playout = wgpuDeviceCreatePipelineLayout(g_wgpustate.device, &pldesc);
    
    WGPURenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = playout;
    
    WGPUVertexState vertexState{};
    vertexState.module = shader;
    vertexState.bufferCount = number_of_buffers;
    vertexState.buffers = ret.vbLayouts;
    vertexState.constantCount = 0;
    vertexState.entryPoint = STRVIEW("vs_main");
    pipelineDesc.vertex = vertexState;

    WGPUFragmentState fragmentState{};

    pipelineDesc.fragment = &fragmentState;
    fragmentState.module = shader;
    fragmentState.entryPoint = STRVIEW("fs_main");
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    WGPUBlendState blendState{};
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;
    WGPUColorTargetState colorTarget{};
    colorTarget.format = g_wgpustate.frameBufferFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    // We setup a depth buffer state for the render pipeline
    WGPUDepthStencilState depthStencilState{};
    // Keep a fragment only if its depth is lower than the previously blended one
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    // Each time a fragment is blended into the target, we update the value of the Z-buffer
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
    // Store the format in a variable as later parts of the code depend on it
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
    depthStencilState.format = depthTextureFormat;
    // Deactivate the stencil alltogether
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
    depthStencilState.stencilBack.compare = WGPUCompareFunction_Always;
    pipelineDesc.depthStencil = &depthStencilState;
    ret.pipeline = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &pipelineDesc);
    
    return ret;
}

extern "C" DescribedBindGroup LoadBindGroup(const DescribedPipeline* pipeline, const WGPUBindGroupEntry* entries, size_t entryCount){
    DescribedBindGroup ret zeroinit;
    WGPUBindGroupEntry* rentries = (WGPUBindGroupEntry*) calloc(entryCount, sizeof(WGPUBindGroupEntry));
    memcpy(rentries, entries, entryCount * sizeof(WGPUBindGroupEntry));
    ret.entries = rentries;
    ret.desc.layout = pipeline->bglayout.layout;
    ret.desc.entries = ret.entries;
    ret.desc.entryCount = entryCount;
    ret.needsUpdate = true;
    //ret.bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &ret.desc);
    return ret;
}
extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, WGPUBindGroupEntry entry){
    bg->entries[index] = entry;

    //TODO don't release and recreate here or find something better lol
    
    if(!bg->needsUpdate && bg->bindGroup)wgpuBindGroupRelease(bg->bindGroup);
    bg->needsUpdate = true;
    //bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
}

extern "C" void UpdateBindGroup(DescribedBindGroup* bg){
    bg->desc.nextInChain = nullptr;
    //std::cout << "Updating bindgroup with " << bg->desc.entryCount << " entries" << std::endl;
    //std::cout << "Updating bindgroup with " << bg->desc.entries[1].binding << " entries" << std::endl;
    if(bg->needsUpdate){
        bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
        bg->needsUpdate = false;
    }
}
WGPUBindGroup GetWGPUBindGroup(DescribedBindGroup* bg){
    if(bg->needsUpdate){
        bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
        bg->needsUpdate = false;
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