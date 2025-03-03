#include "enum_translation.h"
#include <raygpu.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>
#include <dawn/dawn_proc_table.h>
#include <wgpustate.inc>
#include <unordered_set>
#include <internals.hpp>
wgpustate g_wgpustate{};
struct webgpu_cxx_state{
    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
    //wgpu::Surface surface{};
    //full_renderstate* rstate = nullptr;
    //Texture depthTexture{};
};
WGPUQueue GetQueue(){
    return g_wgpustate.queue.Get();
}

void* GetSurface(){
    return (WGPUSurface)g_renderstate.mainWindow->surface.surface;
}
wgpu::Instance& GetCXXInstance(){
    return g_wgpustate.instance;
}
wgpu::Adapter&  GetCXXAdapter (){
    return g_wgpustate.adapter;
}
wgpu::Device&   GetCXXDevice  (){
    return g_wgpustate.device;
}
wgpu::Queue&    GetCXXQueue   (){
    return g_wgpustate.queue;
}
wgpu::Surface&  GetCXXSurface (){
    return *((wgpu::Surface*)&g_renderstate.mainWindow->surface.surface);
}
inline WGPUVertexFormat f16format(uint32_t s){
    switch(s){
        case 1:return WGPUVertexFormat_Float16  ;
        case 2:return WGPUVertexFormat_Float16x2;
      //case 3:return WGPUVertexFormat_Float16x3;
        case 4:return WGPUVertexFormat_Float16x4;
        default: abort();
    }
    rg_unreachable();
}
inline WGPUVertexFormat f32format(uint32_t s){
    switch(s){
        case 1:return WGPUVertexFormat_Float32  ;
        case 2:return WGPUVertexFormat_Float32x2;
        case 3:return WGPUVertexFormat_Float32x3;
        case 4:return WGPUVertexFormat_Float32x4;
        default: abort();
    }
    rg_unreachable();
}
extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    wgpuComputePassEncoderSetPipeline ((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, (WGPUComputePipeline)pipeline->pipeline);
    wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, 0, (WGPUBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup), 0, 0);
}
extern "C" void CopyBufferToBuffer(DescribedBuffer* source, DescribedBuffer* dest, size_t count){
    wgpuCommandEncoderCopyBufferToBuffer((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, (WGPUBuffer)source->buffer, 0, (WGPUBuffer)dest->buffer, 0, count);
}
WGPUBuffer intermediary = 0;
extern "C" void CopyTextureToTexture(Texture source, Texture dest){
    size_t rowBytes = RoundUpToNextMultipleOf256(source.width * GetPixelSizeInBytes(source.format));
    WGPUBufferDescriptor bdesc zeroinit;
    bdesc.size = rowBytes * source.height;
    bdesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
    if(!intermediary){
        intermediary = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bdesc);
    }
    else if(wgpuBufferGetSize(intermediary) < bdesc.size){
        wgpuBufferRelease(intermediary);
        intermediary = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bdesc);
    }
    
    WGPUImageCopyTexture src zeroinit;
    src.texture = (WGPUTexture)source.id;
    src.aspect = WGPUTextureAspect_All;
    src.mipLevel = 0;
    src.origin = WGPUOrigin3D{0, 0, 0};
    
    WGPUImageCopyBuffer bdst zeroinit;
    bdst.buffer = intermediary;
    bdst.layout.rowsPerImage = source.height;
    bdst.layout.bytesPerRow = rowBytes;
    bdst.layout.offset = 0;

    WGPUImageCopyTexture tdst zeroinit;
    tdst.texture = (WGPUTexture)dest.id;
    tdst.aspect = WGPUTextureAspect_All;
    tdst.mipLevel = 0;
    tdst.origin = WGPUOrigin3D{0, 0, 0};

    WGPUExtent3D copySize zeroinit;
    copySize.width = source.width;
    copySize.height = source.height;
    copySize.depthOrArrayLayers = 1;
    
    wgpuCommandEncoderCopyTextureToBuffer((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &src, &bdst, &copySize);
    wgpuCommandEncoderCopyBufferToTexture((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &bdst, &tdst, &copySize);
    
    //Doesnt work unfortunately:
    //wgpuCommandEncoderCopyTextureToTexture(g_renderstate.computepass.cmdEncoder, &src, &dst, &copySize);
    
    //wgpuBufferRelease(intermediary);
}

void DispatchCompute(uint32_t x, uint32_t y, uint32_t z){
    wgpuComputePassEncoderDispatchWorkgroups((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, x, y, z);
}
void ComputepassEndOnlyComputing(cwoid){
    wgpuComputePassEncoderEnd((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder);
    g_renderstate.computepass.cpEncoder = nullptr;
}
void BeginComputepassEx(DescribedComputepass* computePass){
    computePass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), nullptr);
    WGPUComputePassDescriptor desc{};
    desc.label = STRVIEW("ComputePass");
    g_renderstate.computepass.cpEncoder = wgpuCommandEncoderBeginComputePass((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &desc);
}
void UpdateTexture(Texture tex, void* data){
    WGPUImageCopyTexture destination{};
    destination.texture = (WGPUTexture)tex.id;
    destination.aspect = WGPUTextureAspect_All;
    destination.mipLevel = 0;
    destination.origin = WGPUOrigin3D{0,0,0};

    WGPUTextureDataLayout source{};
    source.offset = 0;
    source.bytesPerRow = GetPixelSizeInBytes(tex.format) * tex.width;
    source.rowsPerImage = tex.height;
    WGPUExtent3D writeSize{};
    writeSize.depthOrArrayLayers = 1;
    writeSize.width = tex.width;
    writeSize.height = tex.height;
    wgpuQueueWriteTexture(GetQueue(), &destination, data, tex.width * tex.height * GetPixelSizeInBytes(tex.format), &source, &writeSize);
}
extern "C" Texture3D LoadTexture3DPro(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, WGPUTextureUsage usage, uint32_t sampleCount){
    Texture3D ret zeroinit;
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.sampleCount = sampleCount;
    ret.format = format;

    WGPUTextureDescriptor tDesc zeroinit;
    tDesc.dimension = WGPUTextureDimension_3D;
    tDesc.size = {width, height, depth};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = sampleCount;
    tDesc.format = (WGPUTextureFormat)format;
    tDesc.usage  = usage;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;
    
    WGPUTextureViewDescriptor textureViewDesc zeroinit;
    textureViewDesc.aspect = ((format == Depth24 || format == Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_3D;
    textureViewDesc.format = tDesc.format;

    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &textureViewDesc);
    
    return ret;
}
void EndComputepassEx(DescribedComputepass* computePass){
    if(computePass->cpEncoder){
        wgpuComputePassEncoderEnd((WGPUComputePassEncoder)computePass->cpEncoder);
        wgpuComputePassEncoderRelease((WGPUComputePassEncoder)computePass->cpEncoder);
        computePass->cpEncoder = 0;
    }
    
    //TODO
    g_renderstate.activeComputepass = nullptr;

    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)computePass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)computePass->cmdEncoder);
}

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


extern "C" void UnloadTexture(Texture tex){
    for(uint32_t i = 0;i < tex.mipmaps;i++){
        if(tex.mipViews[i]){
            wgpuTextureViewRelease((WGPUTextureView)tex.mipViews[i]);
            tex.mipViews[i] = nullptr;
        }
    }
    if(tex.view){
        wgpuTextureViewRelease((WGPUTextureView)tex.view);
        tex.view = nullptr;
    }
    if(tex.id){
        wgpuTextureRelease((WGPUTexture)tex.id);
        tex.id = nullptr;
    }
}
void PresentSurface(FullSurface* fsurface){
    wgpuSurfacePresent((WGPUSurface)fsurface->surface);
}


extern "C" RenderPipelineQuartet GetPipelinesForLayout(DescribedPipeline* pl, const std::vector<AttributeAndResidence>& attribs){
    uint32_t attribCount = attribs.size();
    auto it = pl->createdPipelines->pipelines.find(attribs);
    if(it != pl->createdPipelines->pipelines.end()){
        //TRACELOG(LOG_INFO, "Reusing cached pipeline triplet");
        return it->second;
    }
    TRACELOG(LOG_DEBUG, "Creating new pipeline triplet");
    VertexBufferLayoutSet layoutset = getBufferLayoutRepresentation(attribs.data(), attribs.size());
    pl->vertexLayout = layoutset;
    

    WGPURenderPipelineDescriptor pipelineDesc zeroinit;
    const RenderSettings& settings = pl->settings; 
    pipelineDesc.multisample.count = pl->settings.sampleCount ? pl->settings.sampleCount : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = (WGPUPipelineLayout)pl->layout.layout;

    WGPUVertexState vertexState{};
    WGPUFragmentState fragmentState{};
    WGPUBlendState blendState{};

    vertexState.module = (WGPUShaderModule)pl->sh.stages[ShaderStage_Vertex].module;

    VertexBufferLayoutSet& vlayout_complete = pl->vertexLayout;
    vertexState.bufferCount = vlayout_complete.number_of_buffers;

    std::vector<WGPUVertexBufferLayout> layouts_converted;
    for(uint32_t i = 0;i < vlayout_complete.number_of_buffers;i++){
        layouts_converted.push_back(WGPUVertexBufferLayout{
            .nextInChain = nullptr,
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


    
    fragmentState.module = (WGPUShaderModule)pl->sh.stages[ShaderStage_Fragment].module;
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
    RenderPipelineQuartet quartet;
    //if(attribCount != 0){
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_PointList;
    quartet.pipeline_PointList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    quartet.pipeline_TriangleList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_LineList;
    quartet.pipeline_LineList = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
    quartet.pipeline_TriangleStrip = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);
    pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

    pl->createdPipelines->pipelines[attribs] = quartet;
    return quartet;
}


void PostPresentSurface(){}
extern "C" void BindPipeline(DescribedPipeline* pipeline, PrimitiveType drawMode){
    switch(drawMode){
        case RL_TRIANGLES:
        //std::cout << "Binding: " <<  pipeline->pipeline << "\n";
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_TriangleList);
        break;
        case RL_TRIANGLE_STRIP:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_TriangleStrip);
        break;
        case RL_LINES:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_LineList);
        break;
        case RL_POINTS:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_PointList);
        break;
        default:
            assert(false && "Unsupported Drawmode");
            abort();
    }
    //pipeline->lastUsedAs = drawMode;
    wgpuRenderPassEncoderSetBindGroup ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, 0, (WGPUBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup), 0, 0);
}
void ResizeBuffer(DescribedBuffer* buffer, size_t newSize){
    if(newSize == buffer->size)return;

    DescribedBuffer newbuffer{};
    newbuffer.usage = buffer->usage;
    newbuffer.size = newSize;
    WGPUBufferDescriptor desc zeroinit;
    desc.usage = newbuffer.usage;
    desc.size = newbuffer.size;
    desc.mappedAtCreation = false;

    newbuffer.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &desc);
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    *buffer = newbuffer;
}
WGPURenderPassDepthStencilAttachment* defaultDSA(WGPUTextureView depth){
    WGPURenderPassDepthStencilAttachment* dsa = (WGPURenderPassDepthStencilAttachment*)calloc(1, sizeof(WGPURenderPassDepthStencilAttachment));
    // The view of the depth texture
    dsa->view = depth;
    //dsa.depthSlice = 0;
    // The initial value of the depth buffer, meaning "far"
    dsa->depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    dsa->depthLoadOp = WGPULoadOp_Load;
    dsa->depthStoreOp = WGPUStoreOp_Store;
    // we could turn off writing to the depth buffer globally here
    dsa->depthReadOnly = false;
    // Stencil setup, mandatory but unused
    dsa->stencilClearValue = 0;
    #ifdef WEBGPU_BACKEND_WGPU
    dsa.stencilLoadOp =  WGPULoadOp_Load;
    dsa.stencilStoreOp = WGPUStoreOp_Store;
    #else
    dsa->stencilLoadOp = WGPULoadOp_Undefined;
    dsa->stencilStoreOp = WGPUStoreOp_Undefined;
    #endif
    dsa->stencilReadOnly = true;
    return dsa;
}
void UnloadSampler(DescribedSampler sampler){
    wgpuSamplerRelease((WGPUSampler)sampler.sampler);
}
void ResizeBufferAndConserve(DescribedBuffer* buffer, size_t newSize){
    if(newSize == buffer->size)return;

    size_t smaller = std::min<uint32_t>(newSize, buffer->size);
    DescribedBuffer newbuffer{};

    newbuffer.usage = buffer->usage;
    newbuffer.size = newSize;
    WGPUBufferDescriptor desc zeroinit;
    desc.usage = newbuffer.usage;
    desc.size = newbuffer.size;
    desc.mappedAtCreation = false;
    newbuffer.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &desc);
    WGPUCommandEncoderDescriptor edesc{};
    WGPUCommandBufferDescriptor bdesc{};
    auto enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &edesc);
    wgpuCommandEncoderCopyBufferToBuffer(enc, (WGPUBuffer)buffer->buffer, 0, (WGPUBuffer)newbuffer.buffer, 0, smaller);
    auto buf = wgpuCommandEncoderFinish(enc, &bdesc);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    *buffer = newbuffer;
}
DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, bool compute){
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

    
    WGPUBindGroupLayoutEntry* blayouts = (WGPUBindGroupLayoutEntry*)calloc(uniformCount, sizeof(WGPUBindGroupLayoutEntry));
    WGPUBindGroupLayoutDescriptor bglayoutdesc{};

    for(size_t i = 0;i < uniformCount;i++){
        blayouts[i].binding = uniforms[i].location;
        switch(uniforms[i].type){
            default:
                rg_unreachable();
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
            case texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
            case texture_sampler:
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
            case storage_texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
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

    ret.entries = (ResourceTypeDescriptor*)std::calloc(uniformCount, sizeof(ResourceTypeDescriptor));
    std::memcpy(ret.entries, uniforms, uniformCount * sizeof(ResourceTypeDescriptor));
    ret.layout = wgpuDeviceCreateBindGroupLayout((WGPUDevice)GetDevice(), &bglayoutdesc);

    std::free(blayouts);
    return ret;
}




extern "C" FullSurface CreateSurface(void* nsurface, uint32_t width, uint32_t height){
    FullSurface ret{};
    ret.surface = (WGPUSurface)nsurface;
    negotiateSurfaceFormatAndPresentMode(nsurface);
    WGPUSurfaceCapabilities capa{};
    WGPUAdapter adapter = (WGPUAdapter)GetAdapter();

    wgpuSurfaceGetCapabilities((WGPUSurface)ret.surface, adapter, &capa);
    WGPUSurfaceConfiguration config{};
    WGPUPresentMode thm = (WGPUPresentMode)g_renderstate.throttled_PresentMode;
    WGPUPresentMode um = (WGPUPresentMode)g_renderstate.unthrottled_PresentMode;
    if (g_renderstate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT) {
        config.presentMode = (WGPUPresentMode)(((g_renderstate.unthrottled_PresentMode == PresentMode_Mailbox) ? um : thm));
    } else if (g_renderstate.windowFlags & FLAG_VSYNC_HINT) {
        config.presentMode = (WGPUPresentMode)thm;
    } else {
        config.presentMode = (WGPUPresentMode)um;
    }
    TRACELOG(LOG_INFO, "Initialized surface with %s", presentModeSpellingTable.at(config.presentMode).c_str());
    config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = (WGPUTextureFormat)g_renderstate.frameBufferFormat;
    config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    config.viewFormats = &config.format;
    config.viewFormatCount = 1;
    config.device = (WGPUDevice)GetDevice();

    ret.surfaceConfig.presentMode = (PresentMode)config.presentMode;
    ret.surfaceConfig.device = (void*)config.device;
    ret.surfaceConfig.width = config.width;
    ret.surfaceConfig.height = config.width;
    ret.surfaceConfig.format = (PixelFormat)config.format;
    
    ret.renderTarget = LoadRenderTexture(width, height);
    wgpuSurfaceConfigure((WGPUSurface)ret.surface, &config);
    return ret;
}










extern "C" StagingBuffer GenStagingBuffer(size_t size, BufferUsage usage){
    StagingBuffer ret{};
    WGPUBufferDescriptor descriptor1 = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = WGPUStringView{},
        .usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc,
        .size = size,
        .mappedAtCreation = true
    };
    WGPUBufferDescriptor descriptor2 = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = WGPUStringView{},
        .usage = usage,
        .size = size,
        .mappedAtCreation = false
    };
    ret.gpuUsable.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor1);
    ret.mappable.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor2);
    ret.map = wgpuBufferGetMappedRange((WGPUBuffer)ret.mappable.buffer, 0, size);
    return ret;
}
void RecreateStagingBuffer(StagingBuffer* buffer){
    wgpuBufferRelease((WGPUBuffer)buffer->gpuUsable.buffer);
    WGPUBufferDescriptor gpudesc{};
    gpudesc.size = buffer->gpuUsable.size;
    gpudesc.usage = buffer->gpuUsable.usage;
    buffer->gpuUsable.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &gpudesc);
}

void UpdateStagingBuffer(StagingBuffer* buffer){
    wgpuBufferUnmap((WGPUBuffer)buffer->mappable.buffer);
    WGPUCommandEncoderDescriptor arg{};
    WGPUCommandBufferDescriptor arg2{};
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &arg);

    wgpuCommandEncoderCopyBufferToBuffer(enc, (WGPUBuffer)buffer->mappable.buffer, 0, (WGPUBuffer)buffer->gpuUsable.buffer, 0,buffer->mappable.size);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, &arg2);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    
    wgpu::Buffer b((WGPUBuffer)buffer->mappable.buffer);
    WGPUFuture f = b.MapAsync(wgpu::MapMode::Write, 0, b.GetSize(), wgpu::CallbackMode::WaitAnyOnly, [](wgpu::MapAsyncStatus status, wgpu::StringView message){});
    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    WGPUFutureWaitInfo winfo{f, 0};
    wgpuInstanceWaitAny((WGPUInstance)GetInstance(), 1, &winfo, UINT64_MAX);
    buffer->mappable.buffer = b.MoveToCHandle();
    buffer->map = (vertex*)wgpuBufferGetMappedRange((WGPUBuffer)buffer->mappable.buffer, 0, buffer->mappable.size);
}
void UnloadStagingBuffer(StagingBuffer* buf){
    wgpuBufferRelease((WGPUBuffer)buf->gpuUsable.buffer);
    wgpuBufferUnmap  ((WGPUBuffer)buf->mappable.buffer);
    wgpuBufferRelease((WGPUBuffer)buf->mappable.buffer);
}

constexpr char mipmapComputerSource2[] = R"(
    @group(0) @binding(0) var previousMipLevel: texture_2d<f32>;
    @group(0) @binding(1) var nextMipLevel: texture_storage_2d<rgba8unorm, write>;
    
    @compute @workgroup_size(8, 8)
    fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
        let offset = vec2<u32>(0, 1);
        
        let color = (
            textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +
            textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +
            textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +
            textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)
        ) * 0.25;
        textureStore(nextMipLevel, id.xy, color);
    }
    )";
void GenTextureMipmaps(Texture2D* tex){
    static DescribedComputePipeline* cpl = LoadComputePipeline(mipmapComputerSource2);
    BeginComputepass();
    
    for(int i = 0;i < tex->mipmaps - 1;i++){
        SetBindgroupTextureView(&cpl->bindGroup, 0, (WGPUTextureView)tex->mipViews[i    ]);
        SetBindgroupTextureView(&cpl->bindGroup, 1, (WGPUTextureView)tex->mipViews[i + 1]);
        if(i == 0){
            BindComputePipeline(cpl);
        }
        ComputePassSetBindGroup(&g_renderstate.computepass, 0, &cpl->bindGroup);
        uint32_t divisor = (1 << i) * 8;
        DispatchCompute((tex->width + divisor - 1) & -(divisor) / 8, (tex->height + divisor - 1) & -(divisor) / 8, 1);
    }
    EndComputepass();
}









extern "C" void UpdateBindGroup(DescribedBindGroup* bg){
    //std::cout << "Updating bindgroup with " << bg->desc.entryCount << " entries" << std::endl;
    //std::cout << "Updating bindgroup with " << bg->desc.entries[1].binding << " entries" << std::endl;
    if(bg->needsUpdate){
        WGPUBindGroupDescriptor desc zeroinit;
        std::vector<WGPUBindGroupEntry> aswgpu(bg->entryCount);
        for(uint32_t i = 0;i < bg->entryCount;i++){
            aswgpu[i].binding = bg->entries[i].binding;
            aswgpu[i].buffer = (WGPUBuffer)bg->entries[i].buffer;
            aswgpu[i].offset = bg->entries[i].offset;
            aswgpu[i].size = bg->entries[i].size;
            aswgpu[i].sampler = (WGPUSampler)bg->entries[i].sampler;
            aswgpu[i].textureView = (WGPUTextureView)bg->entries[i].textureView;
        }
        desc.entries = aswgpu.data();
        desc.entryCount = aswgpu.size();
        desc.layout = (WGPUBindGroupLayout)bg->layout->layout;
        bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &desc);
        bg->needsUpdate = false;
    }
}

inline uint64_t bgEntryHash(const ResourceDescriptor& bge){
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}


extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry){
    if(index >= bg->entryCount){
        TRACELOG(LOG_WARNING, "Trying to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->entryCount);
        //return;
    }
    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);
    if(entry.buffer){
        wgpuBufferAddRef((WGPUBuffer)entry.buffer);
    }
    if(entry.textureView){
        wgpuTextureViewAddRef((WGPUTextureView)entry.textureView);
    }

    if(bg->entries[index].buffer){
        wgpuBufferRelease((WGPUBuffer)bg->entries[index].buffer);
    }
    if(bg->entries[index].textureView){
        wgpuTextureViewRelease((WGPUTextureView)bg->entries[index].textureView);
    }
    
    //bool donotcache = false;
    //if(bg->releaseOnClear & (1 << index)){
    //    //donotcache = true;
    //    if(bg->entries[index].buffer){
    //        wgpuBufferRelease((WGPUBuffer)bg->entries[index].buffer);
    //    }
    //    else if(bg->entries[index].textureView){
    //        //Todo: currently not the case anyway, but this is nadinöf
    //        wgpuTextureViewRelease((WGPUTextureView)bg->entries[index].textureView);
    //    }
    //    else if(bg->entries[index].sampler){
    //        wgpuSamplerRelease((WGPUSampler)bg->entries[index].sampler);
    //    }
    //    bg->releaseOnClear &= ~(1 << index);
    //}
    bg->entries[index] = entry;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);

    //TODO don't release and recreate here or find something better
    if(true /*|| donotcache*/){
        if(bg->bindGroup)
            wgpuBindGroupRelease((WGPUBindGroup)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    //else if(!bg->needsUpdate && bg->bindGroup){
    //    g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //    bg->bindGroup = nullptr;
    //}
    bg->needsUpdate = true;
    
    //bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &(bg->desc));
}


extern "C" void GetNewTexture(FullSurface* fsurface){
    if(fsurface->surface == 0){
        return;
    }
    else{
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture((WGPUSurface)fsurface->surface, &surfaceTexture);
        fsurface->renderTarget.texture.id = surfaceTexture.texture;
        fsurface->renderTarget.texture.width = wgpuTextureGetWidth(surfaceTexture.texture);
        fsurface->renderTarget.texture.height = wgpuTextureGetHeight(surfaceTexture.texture);
        fsurface->renderTarget.texture.view = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
    }
}
std::optional<wgpu::Limits> limitsToBeRequested = std::nullopt;
WGPUBackendType requestedBackend = DEFAULT_BACKEND;
WGPUAdapterType requestedAdapterType = WGPUAdapterType_Unknown;
void setlimit(wgpu::Limits& limits, LimitType limit, uint64_t value){
    switch(limit){
        case maxTextureDimension1D:limits.maxTextureDimension1D = value;break;
        case maxTextureDimension2D:limits.maxTextureDimension2D = value;break;
        case maxTextureDimension3D:limits.maxTextureDimension3D = value;break;
        case maxTextureArrayLayers:limits.maxTextureArrayLayers = value;break;
        case maxBindGroups:limits.maxBindGroups = value;break;
        case maxBindGroupsPlusVertexBuffers:limits.maxBindGroupsPlusVertexBuffers = value;break;
        case maxBindingsPerBindGroup:limits.maxBindingsPerBindGroup = value;break;
        case maxDynamicUniformBuffersPerPipelineLayout:limits.maxDynamicUniformBuffersPerPipelineLayout = value;break;
        case maxDynamicStorageBuffersPerPipelineLayout:limits.maxDynamicStorageBuffersPerPipelineLayout = value;break;
        case maxSampledTexturesPerShaderStage:limits.maxSampledTexturesPerShaderStage = value;break;
        case maxSamplersPerShaderStage:limits.maxSamplersPerShaderStage = value;break;
        case maxStorageBuffersPerShaderStage:limits.maxStorageBuffersPerShaderStage = value;break;
        case maxStorageTexturesPerShaderStage:limits.maxStorageTexturesPerShaderStage = value;break;
        case maxUniformBuffersPerShaderStage:limits.maxUniformBuffersPerShaderStage = value;break;
        case maxUniformBufferBindingSize:limits.maxUniformBufferBindingSize = value;break;
        case maxStorageBufferBindingSize:limits.maxStorageBufferBindingSize = value;break;
        case minUniformBufferOffsetAlignment:limits.minUniformBufferOffsetAlignment = value;break;
        case minStorageBufferOffsetAlignment:limits.minStorageBufferOffsetAlignment = value;break;
        case maxVertexBuffers:limits.maxVertexBuffers = value;break;
        case maxBufferSize:limits.maxBufferSize = value;break;
        case maxVertexAttributes:limits.maxVertexAttributes = value;break;
        case maxVertexBufferArrayStride:limits.maxVertexBufferArrayStride = value;break;
        case maxInterStageShaderComponents:limits.maxInterStageShaderComponents = value;break;
        case maxInterStageShaderVariables:limits.maxInterStageShaderVariables = value;break;
        case maxColorAttachments:limits.maxColorAttachments = value;break;
        case maxColorAttachmentBytesPerSample:limits.maxColorAttachmentBytesPerSample = value;break;
        case maxComputeWorkgroupStorageSize:limits.maxComputeWorkgroupStorageSize = value;break;
        case maxComputeInvocationsPerWorkgroup:limits.maxComputeInvocationsPerWorkgroup = value;break;
        case maxComputeWorkgroupSizeX:limits.maxComputeWorkgroupSizeX = value;break;
        case maxComputeWorkgroupSizeY:limits.maxComputeWorkgroupSizeY = value;break;
        case maxComputeWorkgroupSizeZ:limits.maxComputeWorkgroupSizeZ = value;break;
        case maxComputeWorkgroupsPerDimension:limits.maxComputeWorkgroupsPerDimension = value;break;
        case maxStorageBuffersInVertexStage:limits.maxStorageBuffersInVertexStage = value;break;
        case maxStorageTexturesInVertexStage:limits.maxStorageTexturesInVertexStage = value;break;
        case maxStorageBuffersInFragmentStage:limits.maxStorageBuffersInFragmentStage = value;break;
        case maxStorageTexturesInFragmentStage:limits.maxStorageTexturesInFragmentStage = value;break;
    }
}
extern "C" void RequestLimit(LimitType limit, uint64_t value){
    if(!limitsToBeRequested.has_value()){
        limitsToBeRequested = wgpu::Limits();
    }
    setlimit(limitsToBeRequested.value(), limit, value);
}
extern "C" void RequestAdapterType(WGPUAdapterType type){
    requestedAdapterType = type;
}
extern "C" void RequestBackend(BackendType backend){
    switch(backend){
        case BackendType_Undefined:requestedBackend = WGPUBackendType_Undefined; 
        case BackendType_Null:requestedBackend = WGPUBackendType_Null; 
        case BackendType_WebGPU:requestedBackend = WGPUBackendType_WebGPU; 
        case BackendType_D3D11:requestedBackend = WGPUBackendType_D3D11; 
        case BackendType_D3D12:requestedBackend = WGPUBackendType_D3D12; 
        case BackendType_Metal:requestedBackend = WGPUBackendType_Metal; 
        case BackendType_Vulkan:requestedBackend = WGPUBackendType_Vulkan; 
        case BackendType_OpenGL:requestedBackend = WGPUBackendType_OpenGL; 
        case BackendType_OpenGLES:requestedBackend = WGPUBackendType_OpenGLES; 
        default:abort();
    }
}
void InitBackend(){
    wgpustate* sample = &g_wgpustate;
    // Create the toggles descriptor if not using emscripten.
    wgpu::ChainedStruct* togglesChain = nullptr;
    wgpu::SType type;
#ifndef __EMSCRIPTEN__
    std::vector<const char*> enableToggleNames{};
    std::vector<const char*> disabledToggleNames{};

    wgpu::DawnTogglesDescriptor toggles = {};
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    togglesChain = &toggles;
#endif  // __EMSCRIPTEN__

    // Setup base adapter options with toggles.
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = togglesChain;
    auto backendType = (wgpu::BackendType)requestedBackend;
    auto adapterType = (wgpu::AdapterType)requestedAdapterType;
    adapterOptions.backendType = backendType;
    if (backendType != wgpu::BackendType::Undefined) {
        auto bcompat = [](wgpu::BackendType backend) {
            switch (backend) {
                case wgpu::BackendType::D3D12:
                case wgpu::BackendType::Metal:
                case wgpu::BackendType::Vulkan:
                case wgpu::BackendType::WebGPU:
                case wgpu::BackendType::Null:
                    return false;
                case wgpu::BackendType::D3D11:
                case wgpu::BackendType::OpenGL:
                case wgpu::BackendType::OpenGLES:
                    return true;
                case wgpu::BackendType::Undefined:
                default:
                    rg_unreachable();
                    //return false;
            }
        };
        adapterOptions.featureLevel = bcompat(backendType) ?  wgpu::FeatureLevel::Compatibility : wgpu::FeatureLevel::Core;

    }

    switch (adapterType) {
        case wgpu::AdapterType::CPU:
            adapterOptions.forceFallbackAdapter = true;
            break;
        case wgpu::AdapterType::DiscreteGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::IntegratedGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::Unknown:
            break;
    }

#ifndef __EMSCRIPTEN__
    
    //dawnProcSetProcs(&dawn::native::GetProcs());

    // Create the instance with the toggles
    wgpu::InstanceDescriptor instanceDescriptor = {};
    instanceDescriptor.nextInChain = togglesChain;
    instanceDescriptor.capabilities.timedWaitAnyEnable = true;
    
    sample->instance = wgpu::CreateInstance(&instanceDescriptor);
#else
    // Create the instance
    sample->instance = wgpu::CreateInstance(nullptr);
#endif  // __EMSCRIPTEN__

    //wgpu::WGSLFeatureName wgslfeatures[8];
    //sample->instance.EnumerateWGSLLanguageFeatures(wgslfeatures);
    //std::cout << sample->instance.HasWGSLLanguageFeature(wgpu::WGSLFeatureName::ReadonlyAndReadwriteStorageTextures);
    //exit(0);
    // Synchronously create the adapter
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    char tmp[2048] = {};
                    std::memcpy(tmp, message.data, std::min(message.length, (size_t)2047));
                    TRACELOG(LOG_FATAL, "Failed to get an adapter: %s\n", tmp);
                    return;
                }
                sample->adapter = std::move(adapter);
            }),
        UINT64_MAX);
    if (sample->adapter == nullptr) {
        std::cerr << "Adapter is null\n";
        abort();
    }
    wgpu::AdapterInfo info;
    sample->adapter.GetInfo(&info);
    
    std::vector<char> deviceNameCstr(info.device.length + 1, '\0');
    std::vector<char> architectureCstr(info.architecture.length + 1, '\0');
    std::vector<char> deviceDescCstr(info.description.length + 1, '\0');
    std::vector<char> vendorC2str(info.vendor.length + 1, '\0');

    memcpy(deviceNameCstr.data(), info.device.data, info.device.length);
    memcpy(deviceDescCstr.data(), info.description.data, info.description.length);
    memcpy(architectureCstr.data(), info.architecture.data, info.architecture.length);
    memcpy(vendorC2str.data(), info.vendor.data, info.vendor.length);
    
    const char* adapterTypeString = info.adapterType == wgpu::AdapterType::CPU ? "CPU" : (info.adapterType == wgpu::AdapterType::IntegratedGPU ? "Integrated GPU" : "Dedicated GPU");
    const char* backendString = backendTypeSpellingTable.at((WGPUBackendType)info.backendType).c_str();

    TRACELOG(LOG_INFO, "Using adapter %s %s", vendorC2str.data(), deviceNameCstr.data());
    TRACELOG(LOG_INFO, "Adapter description: %s", deviceDescCstr.data());
    TRACELOG(LOG_INFO, "Adapter architecture: %s", architectureCstr.data());
    TRACELOG(LOG_INFO, "%s renderer running on %s", backendString, adapterTypeString);
    // Create device descriptor with callbacks and toggles
    {
        wgpu::SupportedFeatures features;
        sample->adapter.GetFeatures(&features);
        std::string featuresString;
        for(size_t i = 0; i < features.featureCount;i++){
            featuresString += featureSpellingTable.contains((WGPUFeatureName)features.features[i]) ? featureSpellingTable.at((WGPUFeatureName)features.features[i]) : "<unknown feature>";
            if(i < features.featureCount - 1)featuresString += ", ";
        }
        TRACELOG(LOG_INFO, "Features supported: %s ", featuresString.c_str());
    }
    wgpu::SupportedLimits adapterLimits;
    sample->adapter.GetLimits(&adapterLimits);
    
    {
        TraceLog(LOG_INFO, "Platform could support %u bindings per bindgroup",    (unsigned)adapterLimits.limits.maxBindingsPerBindGroup);
        TraceLog(LOG_INFO, "Platform could support %u bindgroups",                (unsigned)adapterLimits.limits.maxBindGroups);
        TraceLog(LOG_INFO, "Platform could support buffers up to %llu megabytes", (unsigned long long)adapterLimits.limits.maxBufferSize / (1000000ull));
        TraceLog(LOG_INFO, "Platform could support textures up to %u x %u",       (unsigned)adapterLimits.limits.maxTextureDimension2D, (unsigned)adapterLimits.limits.maxTextureDimension2D);
        TraceLog(LOG_INFO, "Platform could support %u VBO slots",                 (unsigned)adapterLimits.limits.maxVertexBuffers);
    }
    
    wgpu::DeviceDescriptor deviceDesc = {};
    #ifndef __EMSCRIPTEN__ //y tho
    wgpu::FeatureName fnames[2] = {
        wgpu::FeatureName::ClipDistances,
        wgpu::FeatureName::Float32Filterable,
    };
    deviceDesc.requiredFeatures = fnames;
    deviceDesc.requiredFeatureCount = 2;
    #endif
    deviceDesc.nextInChain = togglesChain;
    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            const char* reasonName = "";
            switch (reason) {
                case wgpu::DeviceLostReason::Unknown:
                    reasonName = "Unknown";
                    break;
                case wgpu::DeviceLostReason::Destroyed:
                    reasonName = "Destroyed";
                    break;
                case wgpu::DeviceLostReason::InstanceDropped:
                    reasonName = "InstanceDropped";
                    break;
                case wgpu::DeviceLostReason::FailedCreation:
                    reasonName = "FailedCreation";
                    break;
                default:
                    rg_unreachable();
            }
            std::cerr << "Device lost because of " << reasonName << ": " << std::string(message.data, message.length) << std::endl;
        });
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
            const char* errorTypeName = "";
            switch (type) {
                case wgpu::ErrorType::Validation:
                    errorTypeName = "Validation";
                    break;
                case wgpu::ErrorType::OutOfMemory:
                    errorTypeName = "Out of memory";
                    break;
                case wgpu::ErrorType::Unknown:
                    errorTypeName = "Unknown";
                    break;
                case wgpu::ErrorType::NoError:
                    errorTypeName = "No Error";
                    break;
                default:
                    rg_unreachable();
            }
            std::cerr << errorTypeName << " error: " << std::string(message.data, message.length);
            __builtin_trap();
        });

    
    if(!limitsToBeRequested.has_value()){
        limitsToBeRequested = wgpu::Limits();
    }
    
    
    // Synchronously create the device
    wgpu::RequiredLimits reqLimits;



    if(limitsToBeRequested.has_value()){
        limitsToBeRequested->maxStorageBuffersInVertexStage = adapterLimits.limits.maxStorageBuffersInVertexStage;
        limitsToBeRequested->maxStorageBuffersInFragmentStage = adapterLimits.limits.maxStorageBuffersInFragmentStage;
        limitsToBeRequested->maxStorageBuffersPerShaderStage = adapterLimits.limits.maxStorageBuffersPerShaderStage;

        reqLimits.limits = limitsToBeRequested.value();
        deviceDesc.requiredLimits = &reqLimits;
    }
    else{
        deviceDesc.requiredLimits = nullptr;
    }
    sample->instance.WaitAny(
        sample->adapter.RequestDevice(
            &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                if (status != wgpu::RequestDeviceStatus::Success) {
                    std::cerr << "Failed to get an device: " << std::string(message.data, message.length);
                    return;
                }
                sample->device = std::move(device);
                sample->queue = sample->device.GetQueue();
            }),

        UINT64_MAX);
    if (sample->device == nullptr) {
        TRACELOG(LOG_FATAL, "Device is null");
    }
    wgpu::SupportedLimits slimits;
    
    sample->device.GetLimits(&slimits);
    TraceLog(LOG_INFO, "Device supports %u bindings per bindgroup", (unsigned)slimits.limits.maxBindingsPerBindGroup);
    TraceLog(LOG_INFO, "Device supports %u bindgroups", (unsigned)slimits.limits.maxBindGroups);
    TraceLog(LOG_INFO, "Device supports buffers up to %llu megabytes", (unsigned long long)slimits.limits.maxBufferSize / (1000000ull));
    TraceLog(LOG_INFO, "Device supports textures up to %u x %u", (unsigned)slimits.limits.maxTextureDimension2D, (unsigned)slimits.limits.maxTextureDimension2D);
    TraceLog(LOG_INFO, "Device supports %u VBO slots", (unsigned)slimits.limits.maxVertexBuffers);

}


bool negotiateSurfaceFormatAndPresentMode_called = false;
extern "C" void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle){
    const WGPUSurface surf = (WGPUSurface)SurfaceHandle;
    if(negotiateSurfaceFormatAndPresentMode_called)return;
    negotiateSurfaceFormatAndPresentMode_called = true;
    WGPUSurfaceCapabilities capabilities;
    wgpuSurfaceGetCapabilities(surf, (WGPUAdapter)GetAdapter(), &capabilities);
    {
        std::string presentModeString;
        for(uint32_t i = 0;i < capabilities.presentModeCount;i++){
            presentModeString += presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[i]).c_str();
            if(i < capabilities.presentModeCount - 1){
                presentModeString += ", ";
            }
        }
        TRACELOG(LOG_INFO, "Supported present modes: %s", presentModeString.c_str());
    }
    if(capabilities.presentModeCount == 0){
        TRACELOG(LOG_ERROR, "No presentation modes supported! This surface is most likely invalid");
    }
    else if(capabilities.presentModeCount == 1){
        TRACELOG(LOG_INFO, "Only %s supported", presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[0]).c_str());
        g_renderstate.unthrottled_PresentMode = (PresentMode)capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode = (PresentMode)capabilities.presentModes[0];
    }
    else if(capabilities.presentModeCount > 1){
        g_renderstate.unthrottled_PresentMode = (PresentMode)capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode = (PresentMode)capabilities.presentModes[0];
        std::unordered_set<WGPUPresentMode> pmset(capabilities.presentModes, capabilities.presentModes + capabilities.presentModeCount);
        if(pmset.find(WGPUPresentMode_Fifo) != pmset.end()){
            g_renderstate.throttled_PresentMode = PresentMode_Fifo;
        }
        else if(pmset.find(WGPUPresentMode_FifoRelaxed) != pmset.end()){
            g_renderstate.throttled_PresentMode = PresentMode_FifoRelaxed;
        }
        if(pmset.find(WGPUPresentMode_Mailbox) != pmset.end()){
            g_renderstate.unthrottled_PresentMode = PresentMode_Mailbox;
        }
        else if(pmset.find(WGPUPresentMode_Immediate) != pmset.end()){
            g_renderstate.unthrottled_PresentMode = PresentMode_Immediate;
        }
    }
    
    {
        std::string formatsString;
        for(uint32_t i = 0;i < capabilities.formatCount;i++){
            formatsString += textureFormatSpellingTable.at((WGPUTextureFormat)capabilities.formats[i]).c_str();
            if(i < capabilities.formatCount - 1){
                formatsString += ", ";
            }
        }
        TRACELOG(LOG_INFO, "Supported surface formats: %s", formatsString.c_str());
    }

    WGPUTextureFormat selectedFormat = WGPUTextureFormat_Undefined;
    int format_index = 0;
    for(format_index = 0;format_index < capabilities.formatCount;format_index++){
        if(capabilities.formats[format_index] == WGPUTextureFormat_RGBA16Float){
            selectedFormat = (capabilities.formats[format_index]);
            break;
        }
        if(capabilities.formats[format_index] == WGPUTextureFormat_BGRA8Unorm ||
           capabilities.formats[format_index] == WGPUTextureFormat_RGBA8Unorm){
            selectedFormat = (capabilities.formats[format_index]);
            break;
        }
    }
    g_renderstate.frameBufferFormat = (PixelFormat)selectedFormat;
    if(format_index == capabilities.formatCount){
        TRACELOG(LOG_WARNING, "No RGBA8 / BGRA8 Unorm framebuffer format found, colors might be off"); 
        g_renderstate.frameBufferFormat = (PixelFormat)selectedFormat;
    }
    
    TRACELOG(LOG_INFO, "Selected surface format %s", textureFormatSpellingTable.at((WGPUTextureFormat)g_renderstate.frameBufferFormat).c_str());
    
    //TRACELOG(LOG_INFO, "Selected present mode %s", presentModeSpellingTable.at((WGPUPresentMode)g_wgpustate.presentMode).c_str());
}

extern "C" DescribedBuffer* GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage){
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    WGPUBufferDescriptor descriptor{};
    descriptor.size = size;
    descriptor.mappedAtCreation = false;
    descriptor.usage = usage;
    ret->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor);
    ret->size = size;
    ret->usage = usage;
    if(data != nullptr){
        wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), (WGPUBuffer)ret->buffer, 0, data, size);
    }
    return ret;
}
void* GetInstance(){
    return g_wgpustate.instance.Get();
}
void* GetDevice(){
    return g_wgpustate.device.Get();
}
void* GetAdapter(){
    return g_wgpustate.adapter.Get();
}
extern "C" void ComputePassSetBindGroup(DescribedComputepass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)drp->cpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, nullptr);
}
WGPUBuffer GetMatrixBuffer(){
    wgpuQueueWriteBuffer(GetQueue(), g_renderstate.matrixStack[g_renderstate.stackPosition].second, 0, &g_renderstate.matrixStack[g_renderstate.stackPosition].first, sizeof(Matrix));
    return g_renderstate.matrixStack[g_renderstate.stackPosition].second;
}
extern "C" Texture2DArray LoadTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, PixelFormat format){
    Texture2DArray ret zeroinit;
    ret.sampleCount = 1;
    WGPUTextureDescriptor tDesc zeroinit;
    tDesc.format = toWGPUTextureFormat(format);
    tDesc.size = WGPUExtent3D{width, height, layerCount};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.sampleCount = 1;
    tDesc.mipLevelCount = 1;
    tDesc.usage = WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &tDesc);
    WGPUTextureViewDescriptor vDesc zeroinit;
    vDesc.aspect = WGPUTextureAspect_All;
    vDesc.arrayLayerCount = layerCount;
    vDesc.baseArrayLayer = 0;
    vDesc.mipLevelCount = 1;
    vDesc.baseMipLevel = 0;
    vDesc.dimension = WGPUTextureViewDimension_2DArray;
    vDesc.format = tDesc.format;
    vDesc.usage = tDesc.usage;
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vDesc);
    ret.width = width;
    ret.height = height;
    ret.layerCount = layerCount;
    ret.format = format;
    return ret;
}
extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = mipmaps;
    tDesc.sampleCount = sampleCount;
    tDesc.format = (WGPUTextureFormat)format;
    tDesc.usage  = usage;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;

    WGPUTextureViewDescriptor textureViewDesc{};
    char potlabel[128]; 
    if(format == Depth24){
        int len = snprintf(potlabel, 128, "Depftex %d x %d", width, height);
        textureViewDesc.label.data = potlabel;
        textureViewDesc.label.length = len;
    }
    textureViewDesc.usage = usage;
    textureViewDesc.aspect = ((format == Depth24 || format == Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = mipmaps;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    Texture ret zeroinit;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &textureViewDesc);
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.sampleCount = sampleCount;
    ret.mipmaps = mipmaps;
    if(mipmaps > 1){
        for(uint32_t i = 0;i < mipmaps;i++){
            textureViewDesc.baseMipLevel = i;
            textureViewDesc.mipLevelCount = 1;
            ret.mipViews[i] = wgpuTextureCreateView((WGPUTexture)ret.id, &textureViewDesc);
        }
    }
    return ret;
}
DescribedSampler LoadSamplerEx(addressMode amode, filterMode fmode, filterMode mipmapFilter, float maxAnisotropy){
    DescribedSampler ret zeroinit;
    ret.magFilter    = fmode;
    ret.minFilter    = fmode;
    ret.mipmapFilter = fmode;
    ret.compare      = CompareFunction_Undefined;
    ret.lodMinClamp  = 0.0f;
    ret.lodMaxClamp  = 10.0f;
    ret.maxAnisotropy = maxAnisotropy;
    ret.addressModeU = amode;
    ret.addressModeV = amode;
    ret.addressModeW = amode;

    WGPUSamplerDescriptor sdesc{};
    sdesc.magFilter    = (WGPUFilterMode)fmode;
    sdesc.minFilter    = (WGPUFilterMode)fmode;
    sdesc.mipmapFilter = (WGPUMipmapFilterMode)fmode;
    sdesc.compare      = WGPUCompareFunction_Undefined;
    sdesc.lodMinClamp  = 0.0f;
    sdesc.lodMaxClamp  = 10.0f;
    sdesc.maxAnisotropy = maxAnisotropy;
    sdesc.addressModeU = (WGPUAddressMode)amode;
    sdesc.addressModeV = (WGPUAddressMode)amode;
    sdesc.addressModeW = (WGPUAddressMode)amode;

    ret.sampler = wgpuDeviceCreateSampler((WGPUDevice)GetDevice(), &sdesc);
    return ret;
}
void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgpuBufferRelease(uniformBuffer);
    //bg->releaseOnClear |= (1 << index);
}

void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    bufferDesc.mappedAtCreation = false;

    WGPUBuffer storageBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), storageBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = storageBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgpuBufferRelease(storageBuffer);
}
void UnloadBuffer(DescribedBuffer* buffer){
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    free(buffer);
}
Texture LoadTextureFromImage(Image img){
    Texture ret zeroinit;
    ret.sampleCount = 1;
    Color* altdata = nullptr;
    if(img.format == GRAYSCALE){
        altdata = (Color*)calloc(img.width * img.height, sizeof(Color));
        for(size_t i = 0;i < img.width * img.height;i++){
            uint16_t gscv = ((uint16_t*)img.data)[i];
            ((Color*)altdata)[i].r = gscv & 255;
            ((Color*)altdata)[i].g = gscv & 255;
            ((Color*)altdata)[i].b = gscv & 255;
            ((Color*)altdata)[i].a = gscv >> 8;
        }
    }
    WGPUTextureDescriptor desc = {
        nullptr,
        WGPUStringView{nullptr, 0},
        WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
        WGPUTextureDimension_2D,
        WGPUExtent3D{img.width, img.height, 1},
        img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format,
        1,1,1,nullptr
    };
    WGPUTextureFormat resulting_tf = img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format;
    desc.viewFormats = (WGPUTextureFormat*)&resulting_tf;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &desc);
    WGPUTextureViewDescriptor vdesc{};
    vdesc.arrayLayerCount = 0;
    vdesc.aspect = WGPUTextureAspect_All;
    vdesc.format = desc.format;
    vdesc.dimension = WGPUTextureViewDimension_2D;
    vdesc.baseArrayLayer = 0;
    vdesc.arrayLayerCount = 1;
    vdesc.baseMipLevel = 0;
    vdesc.mipLevelCount = 1;
        
    WGPUImageCopyTexture destination{};
    destination.texture = (WGPUTexture)ret.id;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUTextureDataLayout source{};
    source.offset = 0;
    source.bytesPerRow = 4 * img.width;
    source.rowsPerImage = img.height;
    //wgpuQueueWriteTexture()
    wgpuQueueWriteTexture((WGPUQueue)GetQueue(), &destination, altdata ? altdata : img.data, 4 * img.width * img.height, &source, &desc.size);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vdesc);
    ret.width = img.width;
    ret.height = img.height;
    if(altdata)free(altdata);
    return ret;
}
extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t newWidth, uint32_t newHeight){
    fsurface->surfaceConfig.width = newWidth;
    fsurface->surfaceConfig.height = newHeight;
    fsurface->renderTarget.colorMultisample.width = newWidth;
    fsurface->renderTarget.colorMultisample.height = newHeight;
    fsurface->renderTarget.texture.width = newWidth;
    fsurface->renderTarget.texture.height = newHeight;
    fsurface->renderTarget.depth.width = newWidth;
    fsurface->renderTarget.depth.height = newHeight;
    WGPUTextureFormat format = (WGPUTextureFormat)fsurface->surfaceConfig.format;
    WGPUSurfaceConfiguration wsconfig{};
    wsconfig.device = (WGPUDevice)fsurface->surfaceConfig.device;
    wsconfig.width = newWidth;
    wsconfig.height = newHeight;
    wsconfig.format = format;
    wsconfig.viewFormatCount = 1;
    wsconfig.viewFormats = &format;
    wsconfig.alphaMode = WGPUCompositeAlphaMode_Opaque;
    wsconfig.presentMode = (WGPUPresentMode)fsurface->surfaceConfig.presentMode;
    wsconfig.usage = WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment;
    wgpuSurfaceConfigure((WGPUSurface)fsurface->surface, &wsconfig);
    fsurface->surfaceConfig.width = newWidth;
    fsurface->surfaceConfig.height = newHeight;
    //UnloadTexture(fsurface->frameBuffer.texture);
    //fsurface->frameBuffer.texture = Texture zeroinit;
    UnloadTexture(fsurface->renderTarget.colorMultisample);
    UnloadTexture(fsurface->renderTarget.depth);
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT){
        fsurface->renderTarget.colorMultisample = LoadTexturePro(newWidth, newHeight, fsurface->surfaceConfig.format, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4, 1);
    }
    fsurface->renderTarget.depth = LoadTexturePro(newWidth,
                           newHeight, 
                           Depth32, 
                           WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                           (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                           1
    );
}
extern "C" DescribedRenderpass LoadRenderpassEx(RenderSettings settings, bool colorClear, DColor colorClearValue, bool depthClear, float depthClearValue){
    DescribedRenderpass ret{};

    ret.settings = settings;

    ret.colorClear = colorClearValue;
    ret.depthClear = depthClearValue;
    ret.colorLoadOp = colorClear ? LoadOp_Clear : LoadOp_Load;
    ret.colorStoreOp = StoreOp_Store;
    ret.depthLoadOp = depthClear ? LoadOp_Clear : LoadOp_Load;
    ret.depthStoreOp = StoreOp_Store;

    return ret;
}
void BeginRenderpassEx(DescribedRenderpass* renderPass){
    WGPUCommandEncoderDescriptor desc{};
    desc.label = STRVIEW("another cmdencoder");
    
    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &desc);

    WGPURenderPassDescriptor renderPassDesc zeroinit;
    renderPassDesc.colorAttachmentCount = 1;
    
    WGPURenderPassColorAttachment colorAttachment zeroinit;
    WGPURenderPassDepthStencilAttachment depthAttachment zeroinit;
    if(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].colorMultisample.view){
        colorAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].colorMultisample.view;
        colorAttachment.resolveTarget = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.view;
    }
    else{
        colorAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.view;
        colorAttachment.resolveTarget = nullptr;
    }


    
    colorAttachment.loadOp  = (WGPULoadOp)renderPass->colorLoadOp;
    colorAttachment.storeOp = (WGPUStoreOp)renderPass->colorStoreOp;
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.clearValue = WGPUColor{
        renderPass->colorClear.r,
        renderPass->colorClear.g,
        renderPass->colorClear.b,
        renderPass->colorClear.a,
    };

    depthAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].depth.view;
    depthAttachment.depthLoadOp  = (WGPULoadOp)renderPass->depthLoadOp;
    depthAttachment.depthStoreOp = (WGPUStoreOp)renderPass->depthStoreOp;
    depthAttachment.depthClearValue = 1.0f;
    depthAttachment.depthReadOnly = false;
    depthAttachment.stencilLoadOp = WGPULoadOp_Undefined;
    depthAttachment.stencilStoreOp = WGPUStoreOp_Undefined;

    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthAttachment;
    
    
    renderPass->rpEncoder = wgpuCommandEncoderBeginRenderPass((WGPUCommandEncoder)renderPass->cmdEncoder, &renderPassDesc);
    g_renderstate.activeRenderpass = renderPass;
}
wgpu::Buffer readtex zeroinit;
volatile bool waitflag = false;
Image LoadImageFromTextureEx(WGPUTexture tex, uint32_t miplevel){
    
    size_t formatSize = GetPixelSizeInBytes((PixelFormat)wgpuTextureGetFormat(tex));
    uint32_t width = wgpuTextureGetWidth(tex);
    uint32_t height = wgpuTextureGetHeight(tex);
    Image ret {
        nullptr, 
        wgpuTextureGetWidth(tex), 
        wgpuTextureGetHeight(tex), 
        1,
        (PixelFormat)wgpuTextureGetFormat(tex), 
        RoundUpToNextMultipleOf256(formatSize * width), 
    };
    WGPUBufferDescriptor b{};
    b.mappedAtCreation = false;
    
    b.size = RoundUpToNextMultipleOf256(formatSize * width) * height;
    b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    
    readtex = wgpu::Buffer(wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &b));

    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = STRVIEW("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &commandEncoderDesc);
    WGPUImageCopyTexture tbsource{};
    tbsource.texture = tex;
    tbsource.mipLevel = miplevel;
    tbsource.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    tbsource.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUImageCopyBuffer tbdest{};
    tbdest.buffer = readtex.Get();
    tbdest.layout.offset = 0;
    tbdest.layout.bytesPerRow = RoundUpToNextMultipleOf256(formatSize * width);
    tbdest.layout.rowsPerImage = height;

    WGPUExtent3D copysize{width / (1 << miplevel), height / (1 << miplevel), 1};
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &copysize);
    
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit((WGPUQueue)GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    //#ifdef WEBGPU_BACKEND_DAWN
    //wgpuDeviceTick((WGPUDevice)GetDevice());
    //#else
    //#endif
    ret.format = (PixelFormat)ret.format;
    waitflag = true;
    auto onBuffer2Mapped = [&ret](wgpu::MapAsyncStatus status, const char* userdata){
        //std::cout << "Backcalled: " << (int)status << std::endl;
        waitflag = false;
        if(status != wgpu::MapAsyncStatus::Success){
            TRACELOG(LOG_ERROR, "onBuffer2Mapped called back with error!");
        }
        assert(status == wgpu::MapAsyncStatus::Success);

        uint64_t bufferSize = readtex.GetSize();
        const void* map = readtex.GetConstMappedRange(0, bufferSize);
        //fbLoad.data = std::realloc(fbLoad.data , bufferSize);
        ret.data = std::malloc(bufferSize);
        std::memcpy(ret.data, map, bufferSize);
        readtex.Unmap();
        wgpuBufferRelease(readtex.MoveToCHandle());
    };

    
    
    #ifndef __EMSCRIPTEN__
    GetCXXInstance().WaitAny(readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped), 1000000000);
    #else
    GetCXXInstance().WaitAny(readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped), 1000000000);
    //readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::AllowSpontaneous, onBuffer2Mapped);
    
    //while(waitflag){
    //    
    //}
    //readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * width / 256.0) * 256) * height, onBuffer2Mapped, &ibpair);
    //wgpu::Future fut = readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped);
    //WGPUFutureWaitInfo winfo{};
    //winfo.future = fut;
    //wgpuInstanceWaitAny(g_renderstate.instance, 1, &winfo, 1000000000);

    //while(!done){
        //emscripten_sleep(20);
    //}
    //wgpuInstanceWaitAny(g_renderstate.instance, 1, &winfo, 1000000000);
    //emscripten_sleep(20);
    #endif
    //wgpuBufferMapAsyncF(readtex, WGPUMapMode_Read, 0, 4 * tex.width * tex.height, onBuffer2Mapped);
    /*while(ibpair.first->data == nullptr){
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        WGPUCommandBufferDescriptor cmdBufferDescriptor2{};
        cmdBufferDescriptor.label = "Command buffer";
        WGPUCommandEncoderDescriptor commandEncoderDesc2{};
        commandEncoderDesc.label = "Command Encode2";
        WGPUCommandEncoder encoder2 = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
        WGPUCommandBuffer command2 = wgpuCommandEncoderFinish(encoder2, &cmdBufferDescriptor);
        wgpuQueueSubmit(g_renderstate.queue, 1, &command2);
        wgpuCommandBufferRelease(command);
        #ifdef WEBGPU_BACKEND_DAWN
        wgpuDeviceTick(device);
        #endif
    }
    #ifdef WEBGPU_BACKEND_DAWN
    wgpuInstanceProcessEvents(g_renderstate.instance);
    wgpuDeviceTick(device);
    #endif*/
    //readtex.Destroy();
    return ret;
}
extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->size >= size){
        wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), (WGPUBuffer)buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgpuBufferRelease((WGPUBuffer)buffer->buffer);
        WGPUBufferDescriptor nbdesc{};
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &nbdesc);
        buffer->size = size;
        wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), (WGPUBuffer)buffer->buffer, 0, data, size);
    }
}
extern "C" void ResetSyncState(){}

extern "C" void RenderPassSetIndexBuffer(DescribedRenderpass* drp, DescribedBuffer* buffer, IndexFormat format, uint64_t offset){
    wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)drp->rpEncoder, (WGPUBuffer)buffer->buffer, (WGPUIndexFormat)format, offset, buffer->size);
}
extern "C" void RenderPassSetVertexBuffer(DescribedRenderpass* drp, uint32_t slot, DescribedBuffer* buffer, uint64_t offset){
    wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, buffer->size);
}
extern "C" void RenderPassSetBindGroup(DescribedRenderpass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)drp->rpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, nullptr);
}
extern "C" void RenderPassDraw        (DescribedRenderpass* drp, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance){
    wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
extern "C" void RenderPassDrawIndexed (DescribedRenderpass* drp, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance){
    wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}
extern "C" void EndRenderpassEx(DescribedRenderpass* renderPass){
    drawCurrentBatch();
    wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)renderPass->rpEncoder);
    g_renderstate.activeRenderpass = nullptr;
    auto re = renderPass->rpEncoder;
    renderPass->rpEncoder = 0;
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)renderPass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit((WGPUQueue)GetQueue(), 1, &command);
    wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)re);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)renderPass->cmdEncoder);
    wgpuCommandBufferRelease(command);
}
extern "C" void EndRenderpassPro(DescribedRenderpass* rp, bool renderTexture){
    EndRenderpassEx(rp);
}

RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    RenderTexture ret{
        .texture = LoadTextureEx(width, height, (PixelFormat)g_renderstate.frameBufferFormat, true),
        .colorMultisample = Texture{}, 
        .depth = LoadTexturePro(width, height, Depth32, TextureUsage_RenderAttachment | TextureUsage_CopySrc, (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1)
    };
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT){
        ret.colorMultisample = LoadTexturePro(width, height, (PixelFormat)g_renderstate.frameBufferFormat, TextureUsage_RenderAttachment | TextureUsage_CopySrc, 4, 1);
    }
    return ret;
}
const std::unordered_map<WGPUFeatureName, std::string> featureSpellingTable = [](){
    std::unordered_map<WGPUFeatureName, std::string> ret;
    #ifdef __EMSCRIPTEN__
    ret[WGPUFeatureName_DepthClipControl] = "DepthClipControl";
    ret[WGPUFeatureName_Depth32FloatStencil8] = "Depth32FloatStencil8";
    ret[WGPUFeatureName_TimestampQuery] = "TimestampQuery";
    ret[WGPUFeatureName_TextureCompressionBC] = "TextureCompressionBC";
    ret[WGPUFeatureName_TextureCompressionETC2] = "TextureCompressionETC2";
    ret[WGPUFeatureName_TextureCompressionASTC] = "TextureCompressionASTC";
    ret[WGPUFeatureName_IndirectFirstInstance] = "IndirectFirstInstance";
    ret[WGPUFeatureName_ShaderF16] = "ShaderF16";
    ret[WGPUFeatureName_RG11B10UfloatRenderable] = "RG11B10UfloatRenderable";
    ret[WGPUFeatureName_BGRA8UnormStorage] = "BGRA8UnormStorage";
    ret[WGPUFeatureName_Float32Filterable] = "Float32Filterable";
    ret[WGPUFeatureName_Float32Blendable] = "Float32Blendable";
    ret[WGPUFeatureName_Subgroups] = "Subgroups";
    ret[WGPUFeatureName_SubgroupsF16] = "SubgroupsF16";
    ret[WGPUFeatureName_Force32] = "Force32";
    #else
    ret[WGPUFeatureName_DepthClipControl] = "DepthClipControl";
    ret[WGPUFeatureName_Depth32FloatStencil8] = "Depth32FloatStencil8";
    ret[WGPUFeatureName_TimestampQuery] = "TimestampQuery";
    ret[WGPUFeatureName_TextureCompressionBC] = "TextureCompressionBC";
    ret[WGPUFeatureName_TextureCompressionETC2] = "TextureCompressionETC2";
    ret[WGPUFeatureName_TextureCompressionASTC] = "TextureCompressionASTC";
    ret[WGPUFeatureName_IndirectFirstInstance] = "IndirectFirstInstance";
    ret[WGPUFeatureName_ShaderF16] = "ShaderF16";
    ret[WGPUFeatureName_RG11B10UfloatRenderable] = "RG11B10UfloatRenderable";
    ret[WGPUFeatureName_BGRA8UnormStorage] = "BGRA8UnormStorage";
    ret[WGPUFeatureName_Float32Filterable] = "Float32Filterable";
    ret[WGPUFeatureName_Float32Blendable] = "Float32Blendable";
    ret[WGPUFeatureName_Subgroups] = "Subgroups";
    ret[WGPUFeatureName_SubgroupsF16] = "SubgroupsF16";
    ret[WGPUFeatureName_DawnInternalUsages] = "DawnInternalUsages";
    ret[WGPUFeatureName_DawnMultiPlanarFormats] = "DawnMultiPlanarFormats";
    ret[WGPUFeatureName_DawnNative] = "DawnNative";
    ret[WGPUFeatureName_ChromiumExperimentalTimestampQueryInsidePasses] = "ChromiumExperimentalTimestampQueryInsidePasses";
    ret[WGPUFeatureName_ImplicitDeviceSynchronization] = "ImplicitDeviceSynchronization";
    ret[WGPUFeatureName_ChromiumExperimentalImmediateData] = "ChromiumExperimentalImmediateData";
    ret[WGPUFeatureName_TransientAttachments] = "TransientAttachments";
    ret[WGPUFeatureName_MSAARenderToSingleSampled] = "MSAARenderToSingleSampled";
    ret[WGPUFeatureName_DualSourceBlending] = "DualSourceBlending";
    ret[WGPUFeatureName_D3D11MultithreadProtected] = "D3D11MultithreadProtected";
    ret[WGPUFeatureName_ANGLETextureSharing] = "ANGLETextureSharing";
    ret[WGPUFeatureName_PixelLocalStorageCoherent] = "PixelLocalStorageCoherent";
    ret[WGPUFeatureName_PixelLocalStorageNonCoherent] = "PixelLocalStorageNonCoherent";
    ret[WGPUFeatureName_Unorm16TextureFormats] = "Unorm16TextureFormats";
    ret[WGPUFeatureName_Snorm16TextureFormats] = "Snorm16TextureFormats";
    ret[WGPUFeatureName_MultiPlanarFormatExtendedUsages] = "MultiPlanarFormatExtendedUsages";
    ret[WGPUFeatureName_MultiPlanarFormatP010] = "MultiPlanarFormatP010";
    ret[WGPUFeatureName_HostMappedPointer] = "HostMappedPointer";
    ret[WGPUFeatureName_MultiPlanarRenderTargets] = "MultiPlanarRenderTargets";
    ret[WGPUFeatureName_MultiPlanarFormatNv12a] = "MultiPlanarFormatNv12a";
    ret[WGPUFeatureName_FramebufferFetch] = "FramebufferFetch";
    ret[WGPUFeatureName_BufferMapExtendedUsages] = "BufferMapExtendedUsages";
    ret[WGPUFeatureName_AdapterPropertiesMemoryHeaps] = "AdapterPropertiesMemoryHeaps";
    ret[WGPUFeatureName_AdapterPropertiesD3D] = "AdapterPropertiesD3D";
    ret[WGPUFeatureName_AdapterPropertiesVk] = "AdapterPropertiesVk";
    ret[WGPUFeatureName_R8UnormStorage] = "R8UnormStorage";
    ret[WGPUFeatureName_Norm16TextureFormats] = "Norm16TextureFormats";
    ret[WGPUFeatureName_MultiPlanarFormatNv16] = "MultiPlanarFormatNv16";
    ret[WGPUFeatureName_MultiPlanarFormatNv24] = "MultiPlanarFormatNv24";
    ret[WGPUFeatureName_MultiPlanarFormatP210] = "MultiPlanarFormatP210";
    ret[WGPUFeatureName_MultiPlanarFormatP410] = "MultiPlanarFormatP410";
    ret[WGPUFeatureName_SharedTextureMemoryVkDedicatedAllocation] = "SharedTextureMemoryVkDedicatedAllocation";
    ret[WGPUFeatureName_SharedTextureMemoryAHardwareBuffer] = "SharedTextureMemoryAHardwareBuffer";
    ret[WGPUFeatureName_SharedTextureMemoryDmaBuf] = "SharedTextureMemoryDmaBuf";
    ret[WGPUFeatureName_SharedTextureMemoryOpaqueFD] = "SharedTextureMemoryOpaqueFD";
    ret[WGPUFeatureName_SharedTextureMemoryZirconHandle] = "SharedTextureMemoryZirconHandle";
    ret[WGPUFeatureName_SharedTextureMemoryDXGISharedHandle] = "SharedTextureMemoryDXGISharedHandle";
    ret[WGPUFeatureName_SharedTextureMemoryD3D11Texture2D] = "SharedTextureMemoryD3D11Texture2D";
    ret[WGPUFeatureName_SharedTextureMemoryIOSurface] = "SharedTextureMemoryIOSurface";
    ret[WGPUFeatureName_SharedTextureMemoryEGLImage] = "SharedTextureMemoryEGLImage";
    ret[WGPUFeatureName_SharedFenceVkSemaphoreOpaqueFD] = "SharedFenceVkSemaphoreOpaqueFD";
    ret[WGPUFeatureName_SharedFenceSyncFD] = "SharedFenceSyncFD";
    ret[WGPUFeatureName_SharedFenceVkSemaphoreZirconHandle] = "SharedFenceVkSemaphoreZirconHandle";
    ret[WGPUFeatureName_SharedFenceDXGISharedHandle] = "SharedFenceDXGISharedHandle";
    ret[WGPUFeatureName_SharedFenceMTLSharedEvent] = "SharedFenceMTLSharedEvent";
    ret[WGPUFeatureName_SharedBufferMemoryD3D12Resource] = "SharedBufferMemoryD3D12Resource";
    ret[WGPUFeatureName_StaticSamplers] = "StaticSamplers";
    ret[WGPUFeatureName_YCbCrVulkanSamplers] = "YCbCrVulkanSamplers";
    ret[WGPUFeatureName_ShaderModuleCompilationOptions] = "ShaderModuleCompilationOptions";
    ret[WGPUFeatureName_DawnLoadResolveTexture] = "DawnLoadResolveTexture";
    ret[WGPUFeatureName_DawnPartialLoadResolveTexture] = "DawnPartialLoadResolveTexture";
    ret[WGPUFeatureName_MultiDrawIndirect] = "MultiDrawIndirect";
    ret[WGPUFeatureName_ClipDistances] = "ClipDistances";
    ret[WGPUFeatureName_DawnTexelCopyBufferRowAlignment] = "DawnTexelCopyBufferRowAlignment";
    ret[WGPUFeatureName_FlexibleTextureViews] = "FlexibleTextureViews";
    ret[WGPUFeatureName_Force32] = "Force32";
    #endif
    return ret;
}();

const std::unordered_map<WGPUBackendType, std::string> backendTypeSpellingTable = [](){
    std::unordered_map<WGPUBackendType, std::string> map;
    map[WGPUBackendType_Undefined] = "Undefined";
    map[WGPUBackendType_Null] = "Null";
    map[WGPUBackendType_WebGPU] = "WebGPU";
    map[WGPUBackendType_D3D11] = "D3D11";
    map[WGPUBackendType_D3D12] = "D3D12";
    map[WGPUBackendType_Metal] = "Metal";
    map[WGPUBackendType_Vulkan] = "Vulkan";
    map[WGPUBackendType_OpenGL] = "OpenGL";
    map[WGPUBackendType_OpenGLES] = "OpenGLES";
    return map;
}();
const std::unordered_map<WGPUPresentMode, std::string> presentModeSpellingTable = [](){
    std::unordered_map<WGPUPresentMode, std::string> map;
    map[WGPUPresentMode_Fifo] = "WGPUPresentMode_Fifo";
    map[WGPUPresentMode_FifoRelaxed] = "WGPUPresentMode_FifoRelaxed";
    map[WGPUPresentMode_Immediate] = "WGPUPresentMode_Immediate";
    map[WGPUPresentMode_Mailbox] = "WGPUPresentMode_Mailbox";
    map[WGPUPresentMode_Force32] = "WGPUPresentMode_Force32";
    return map;
}();
const std::unordered_map<WGPUTextureFormat, std::string> textureFormatSpellingTable = [](){
    std::unordered_map<WGPUTextureFormat, std::string> map;
    map[WGPUTextureFormat_Undefined] = "WGPUTextureFormat_Undefined";
    map[WGPUTextureFormat_R8Unorm] = "WGPUTextureFormat_R8Unorm";
    map[WGPUTextureFormat_R8Snorm] = "WGPUTextureFormat_R8Snorm";
    map[WGPUTextureFormat_R8Uint] = "WGPUTextureFormat_R8Uint";
    map[WGPUTextureFormat_R8Sint] = "WGPUTextureFormat_R8Sint";
    map[WGPUTextureFormat_R16Uint] = "WGPUTextureFormat_R16Uint";
    map[WGPUTextureFormat_R16Sint] = "WGPUTextureFormat_R16Sint";
    map[WGPUTextureFormat_R16Float] = "WGPUTextureFormat_R16Float";
    map[WGPUTextureFormat_RG8Unorm] = "WGPUTextureFormat_RG8Unorm";
    map[WGPUTextureFormat_RG8Snorm] = "WGPUTextureFormat_RG8Snorm";
    map[WGPUTextureFormat_RG8Uint] = "WGPUTextureFormat_RG8Uint";
    map[WGPUTextureFormat_RG8Sint] = "WGPUTextureFormat_RG8Sint";
    map[WGPUTextureFormat_R32Float] = "WGPUTextureFormat_R32Float";
    map[WGPUTextureFormat_R32Uint] = "WGPUTextureFormat_R32Uint";
    map[WGPUTextureFormat_R32Sint] = "WGPUTextureFormat_R32Sint";
    map[WGPUTextureFormat_RG16Uint] = "WGPUTextureFormat_RG16Uint";
    map[WGPUTextureFormat_RG16Sint] = "WGPUTextureFormat_RG16Sint";
    map[WGPUTextureFormat_RG16Float] = "WGPUTextureFormat_RG16Float";
    map[WGPUTextureFormat_RGBA8Unorm] = "WGPUTextureFormat_RGBA8Unorm";
    map[WGPUTextureFormat_RGBA8UnormSrgb] = "WGPUTextureFormat_RGBA8UnormSrgb";
    map[WGPUTextureFormat_RGBA8Snorm] = "WGPUTextureFormat_RGBA8Snorm";
    map[WGPUTextureFormat_RGBA8Uint] = "WGPUTextureFormat_RGBA8Uint";
    map[WGPUTextureFormat_RGBA8Sint] = "WGPUTextureFormat_RGBA8Sint";
    map[WGPUTextureFormat_BGRA8Unorm] = "WGPUTextureFormat_BGRA8Unorm";
    map[WGPUTextureFormat_BGRA8UnormSrgb] = "WGPUTextureFormat_BGRA8UnormSrgb";
    map[WGPUTextureFormat_RGB10A2Uint] = "WGPUTextureFormat_RGB10A2Uint";
    map[WGPUTextureFormat_RGB10A2Unorm] = "WGPUTextureFormat_RGB10A2Unorm";
    map[WGPUTextureFormat_RG11B10Ufloat] = "WGPUTextureFormat_RG11B10Ufloat";
    map[WGPUTextureFormat_RGB9E5Ufloat] = "WGPUTextureFormat_RGB9E5Ufloat";
    map[WGPUTextureFormat_RG32Float] = "WGPUTextureFormat_RG32Float";
    map[WGPUTextureFormat_RG32Uint] = "WGPUTextureFormat_RG32Uint";
    map[WGPUTextureFormat_RG32Sint] = "WGPUTextureFormat_RG32Sint";
    map[WGPUTextureFormat_RGBA16Uint] = "WGPUTextureFormat_RGBA16Uint";
    map[WGPUTextureFormat_RGBA16Sint] = "WGPUTextureFormat_RGBA16Sint";
    map[WGPUTextureFormat_RGBA16Float] = "WGPUTextureFormat_RGBA16Float";
    map[WGPUTextureFormat_RGBA32Float] = "WGPUTextureFormat_RGBA32Float";
    map[WGPUTextureFormat_RGBA32Uint] = "WGPUTextureFormat_RGBA32Uint";
    map[WGPUTextureFormat_RGBA32Sint] = "WGPUTextureFormat_RGBA32Sint";
    map[WGPUTextureFormat_Stencil8] = "WGPUTextureFormat_Stencil8";
    map[WGPUTextureFormat_Depth16Unorm] = "WGPUTextureFormat_Depth16Unorm";
    map[WGPUTextureFormat_Depth24Plus] = "WGPUTextureFormat_Depth24Plus";
    map[WGPUTextureFormat_Depth24PlusStencil8] = "WGPUTextureFormat_Depth24PlusStencil8";
    map[WGPUTextureFormat_Depth32Float] = "WGPUTextureFormat_Depth32Float";
    map[WGPUTextureFormat_Depth32FloatStencil8] = "WGPUTextureFormat_Depth32FloatStencil8";
    map[WGPUTextureFormat_BC1RGBAUnorm] = "WGPUTextureFormat_BC1RGBAUnorm";
    map[WGPUTextureFormat_BC1RGBAUnormSrgb] = "WGPUTextureFormat_BC1RGBAUnormSrgb";
    map[WGPUTextureFormat_BC2RGBAUnorm] = "WGPUTextureFormat_BC2RGBAUnorm";
    map[WGPUTextureFormat_BC2RGBAUnormSrgb] = "WGPUTextureFormat_BC2RGBAUnormSrgb";
    map[WGPUTextureFormat_BC3RGBAUnorm] = "WGPUTextureFormat_BC3RGBAUnorm";
    map[WGPUTextureFormat_BC3RGBAUnormSrgb] = "WGPUTextureFormat_BC3RGBAUnormSrgb";
    map[WGPUTextureFormat_BC4RUnorm] = "WGPUTextureFormat_BC4RUnorm";
    map[WGPUTextureFormat_BC4RSnorm] = "WGPUTextureFormat_BC4RSnorm";
    map[WGPUTextureFormat_BC5RGUnorm] = "WGPUTextureFormat_BC5RGUnorm";
    map[WGPUTextureFormat_BC5RGSnorm] = "WGPUTextureFormat_BC5RGSnorm";
    map[WGPUTextureFormat_BC6HRGBUfloat] = "WGPUTextureFormat_BC6HRGBUfloat";
    map[WGPUTextureFormat_BC6HRGBFloat] = "WGPUTextureFormat_BC6HRGBFloat";
    map[WGPUTextureFormat_BC7RGBAUnorm] = "WGPUTextureFormat_BC7RGBAUnorm";
    map[WGPUTextureFormat_BC7RGBAUnormSrgb] = "WGPUTextureFormat_BC7RGBAUnormSrgb";
    map[WGPUTextureFormat_ETC2RGB8Unorm] = "WGPUTextureFormat_ETC2RGB8Unorm";
    map[WGPUTextureFormat_ETC2RGB8UnormSrgb] = "WGPUTextureFormat_ETC2RGB8UnormSrgb";
    map[WGPUTextureFormat_ETC2RGB8A1Unorm] = "WGPUTextureFormat_ETC2RGB8A1Unorm";
    map[WGPUTextureFormat_ETC2RGB8A1UnormSrgb] = "WGPUTextureFormat_ETC2RGB8A1UnormSrgb";
    map[WGPUTextureFormat_ETC2RGBA8Unorm] = "WGPUTextureFormat_ETC2RGBA8Unorm";
    map[WGPUTextureFormat_ETC2RGBA8UnormSrgb] = "WGPUTextureFormat_ETC2RGBA8UnormSrgb";
    map[WGPUTextureFormat_EACR11Unorm] = "WGPUTextureFormat_EACR11Unorm";
    map[WGPUTextureFormat_EACR11Snorm] = "WGPUTextureFormat_EACR11Snorm";
    map[WGPUTextureFormat_EACRG11Unorm] = "WGPUTextureFormat_EACRG11Unorm";
    map[WGPUTextureFormat_EACRG11Snorm] = "WGPUTextureFormat_EACRG11Snorm";
    map[WGPUTextureFormat_ASTC4x4Unorm] = "WGPUTextureFormat_ASTC4x4Unorm";
    map[WGPUTextureFormat_ASTC4x4UnormSrgb] = "WGPUTextureFormat_ASTC4x4UnormSrgb";
    map[WGPUTextureFormat_ASTC5x4Unorm] = "WGPUTextureFormat_ASTC5x4Unorm";
    map[WGPUTextureFormat_ASTC5x4UnormSrgb] = "WGPUTextureFormat_ASTC5x4UnormSrgb";
    map[WGPUTextureFormat_ASTC5x5Unorm] = "WGPUTextureFormat_ASTC5x5Unorm";
    map[WGPUTextureFormat_ASTC5x5UnormSrgb] = "WGPUTextureFormat_ASTC5x5UnormSrgb";
    map[WGPUTextureFormat_ASTC6x5Unorm] = "WGPUTextureFormat_ASTC6x5Unorm";
    map[WGPUTextureFormat_ASTC6x5UnormSrgb] = "WGPUTextureFormat_ASTC6x5UnormSrgb";
    map[WGPUTextureFormat_ASTC6x6Unorm] = "WGPUTextureFormat_ASTC6x6Unorm";
    map[WGPUTextureFormat_ASTC6x6UnormSrgb] = "WGPUTextureFormat_ASTC6x6UnormSrgb";
    map[WGPUTextureFormat_ASTC8x5Unorm] = "WGPUTextureFormat_ASTC8x5Unorm";
    map[WGPUTextureFormat_ASTC8x5UnormSrgb] = "WGPUTextureFormat_ASTC8x5UnormSrgb";
    map[WGPUTextureFormat_ASTC8x6Unorm] = "WGPUTextureFormat_ASTC8x6Unorm";
    map[WGPUTextureFormat_ASTC8x6UnormSrgb] = "WGPUTextureFormat_ASTC8x6UnormSrgb";
    map[WGPUTextureFormat_ASTC8x8Unorm] = "WGPUTextureFormat_ASTC8x8Unorm";
    map[WGPUTextureFormat_ASTC8x8UnormSrgb] = "WGPUTextureFormat_ASTC8x8UnormSrgb";
    map[WGPUTextureFormat_ASTC10x5Unorm] = "WGPUTextureFormat_ASTC10x5Unorm";
    map[WGPUTextureFormat_ASTC10x5UnormSrgb] = "WGPUTextureFormat_ASTC10x5UnormSrgb";
    map[WGPUTextureFormat_ASTC10x6Unorm] = "WGPUTextureFormat_ASTC10x6Unorm";
    map[WGPUTextureFormat_ASTC10x6UnormSrgb] = "WGPUTextureFormat_ASTC10x6UnormSrgb";
    map[WGPUTextureFormat_ASTC10x8Unorm] = "WGPUTextureFormat_ASTC10x8Unorm";
    map[WGPUTextureFormat_ASTC10x8UnormSrgb] = "WGPUTextureFormat_ASTC10x8UnormSrgb";
    map[WGPUTextureFormat_ASTC10x10Unorm] = "WGPUTextureFormat_ASTC10x10Unorm";
    map[WGPUTextureFormat_ASTC10x10UnormSrgb] = "WGPUTextureFormat_ASTC10x10UnormSrgb";
    map[WGPUTextureFormat_ASTC12x10Unorm] = "WGPUTextureFormat_ASTC12x10Unorm";
    map[WGPUTextureFormat_ASTC12x10UnormSrgb] = "WGPUTextureFormat_ASTC12x10UnormSrgb";
    map[WGPUTextureFormat_ASTC12x12Unorm] = "WGPUTextureFormat_ASTC12x12Unorm";
    map[WGPUTextureFormat_ASTC12x12UnormSrgb] = "WGPUTextureFormat_ASTC12x12UnormSrgb";
    #ifndef __EMSCRIPTEN__ //why??
    map[WGPUTextureFormat_R16Unorm] = "WGPUTextureFormat_R16Unorm";
    map[WGPUTextureFormat_RG16Unorm] = "WGPUTextureFormat_RG16Unorm";
    map[WGPUTextureFormat_RGBA16Unorm] = "WGPUTextureFormat_RGBA16Unorm";
    map[WGPUTextureFormat_R16Snorm] = "WGPUTextureFormat_R16Snorm";
    map[WGPUTextureFormat_RG16Snorm] = "WGPUTextureFormat_RG16Snorm";
    map[WGPUTextureFormat_RGBA16Snorm] = "WGPUTextureFormat_RGBA16Snorm";
    map[WGPUTextureFormat_R8BG8Biplanar420Unorm] = "WGPUTextureFormat_R8BG8Biplanar420Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm";
    map[WGPUTextureFormat_R8BG8A8Triplanar420Unorm] = "WGPUTextureFormat_R8BG8A8Triplanar420Unorm";
    map[WGPUTextureFormat_R8BG8Biplanar422Unorm] = "WGPUTextureFormat_R8BG8Biplanar422Unorm";
    map[WGPUTextureFormat_R8BG8Biplanar444Unorm] = "WGPUTextureFormat_R8BG8Biplanar444Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm";
    map[WGPUTextureFormat_External] = "WGPUTextureFormat_External";
    map[WGPUTextureFormat_Force32] = "WGPUTextureFormat_Force32";
    #endif
    return map;
}();
const char* TextureFormatName(WGPUTextureFormat fmt){
    auto it = textureFormatSpellingTable.find(fmt);
    if(it == textureFormatSpellingTable.end()){
        return "?? Unknown WGPUTextureFormat value ??";
    }
    return it->second.c_str();
}
