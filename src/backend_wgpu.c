// begin file src/backend_wgpu.c

#include <stdint.h>
#include <raygpu.h>
#ifdef SUPPORT_VULKAN_BACKEND
#include <wgvk.h>
#else
#include <webgpu/webgpu.h>
#endif
#include <internals.h>
#include <wgpustate.inc>
#define Matrix spvMatrix
#if SUPPORT_WGPU_BACKEND == 1
    #include <spirv_reflect.c>
#else
    #include <spirv_reflect.h>
#endif
#undef Matrix
wgpustate g_wgpustate = {0};

WGPUQueue GetQueue() { return g_wgpustate.queue; }

void *GetSurface() { return (WGPUSurface)g_renderstate.mainWindow->surface.surface; }

inline WGPUVertexFormat f16format(uint32_t s) {
    switch (s) {
    case 1:
        return WGPUVertexFormat_Float16;
    case 2:
        return WGPUVertexFormat_Float16x2;
        // case 3:return WGPUVertexFormat_Float16x3;
    case 4:
        return WGPUVertexFormat_Float16x4;
    default:
        abort();
    }
    rg_unreachable();
}
inline WGPUVertexFormat f32format(uint32_t s) {
    switch (s) {
    case 1:
        return WGPUVertexFormat_Float32;
    case 2:
        return WGPUVertexFormat_Float32x2;
    case 3:
        return WGPUVertexFormat_Float32x3;
    case 4:
        return WGPUVertexFormat_Float32x4;
    default:
        abort();
    }
    rg_unreachable();
}
void BindComputePipeline(DescribedComputePipeline *pipeline) {
    wgpuComputePassEncoderSetPipeline((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder,
                                      (WGPUComputePipeline)pipeline->pipeline);
    wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder,
                                       0,
                                       (WGPUBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup),
                                       0,
                                       0);
}
void CopyBufferToBuffer(DescribedBuffer *source, DescribedBuffer *dest, size_t count) {
    wgpuCommandEncoderCopyBufferToBuffer((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder,
                                         (WGPUBuffer)source->buffer,
                                         0,
                                         (WGPUBuffer)dest->buffer,
                                         0,
                                         count);
}
WGPUBuffer intermediary = 0;
void CopyTextureToTexture(Texture source, Texture dest) {
    size_t rowBytes = RoundUpToNextMultipleOf256(source.width) * GetPixelSizeInBytes(source.format);
    WGPUBufferDescriptor bdesc = {
        .size = rowBytes * source.height,
        .usage =  WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc
    };

    if (!intermediary) {
        intermediary = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bdesc);
    } else if (wgpuBufferGetSize(intermediary) < bdesc.size) {
        wgpuBufferRelease(intermediary);
        intermediary = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bdesc);
    }

    const WGPUTexelCopyTextureInfo src = {
        .texture = (WGPUTexture)source.id,
        .aspect = WGPUTextureAspect_All,
        .mipLevel = 0,
        .origin = {0, 0, 0},
    };

    const WGPUTexelCopyBufferInfo bdst = {
        .buffer = intermediary,
        .layout.rowsPerImage = source.height,
        .layout.bytesPerRow = rowBytes,
        .layout.offset = 0,
    };

    const WGPUTexelCopyTextureInfo tdst = {
        .texture = (WGPUTexture)dest.id,
        .aspect = WGPUTextureAspect_All,
        .mipLevel = 0,
        .origin = {0, 0, 0},
    };

    const WGPUExtent3D copySize = {
        .width = source.width,
        .height = source.height,
        .depthOrArrayLayers = 1,
    };

    wgpuCommandEncoderCopyTextureToBuffer((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &src, &bdst, &copySize);
    wgpuCommandEncoderCopyBufferToTexture((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &bdst, &tdst, &copySize);

    // Doesnt work unfortunately:
    // wgpuCommandEncoderCopyTextureToTexture(g_renderstate.computepass.cmdEncoder, &src, &dst, &copySize);

    // wgpuBufferRelease(intermediary);
}

void DispatchCompute(uint32_t x, uint32_t y, uint32_t z) {
    wgpuComputePassEncoderDispatchWorkgroups((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, x, y, z);
}
void ComputepassEndOnlyComputing(cwoid) {
    wgpuComputePassEncoderEnd((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder);
    g_renderstate.computepass.cpEncoder = NULL;
}
void BeginComputepassEx(DescribedComputepass *computePass) {
    computePass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), NULL);
    WGPUComputePassDescriptor desc = {0};
    desc.label = STRVIEW("ComputePass");
    g_renderstate.computepass.cpEncoder =
        wgpuCommandEncoderBeginComputePass((WGPUCommandEncoder)g_renderstate.computepass.cmdEncoder, &desc);
}
void UpdateTexture(Texture tex, void *data) {
    const WGPUTexelCopyTextureInfo destination = {
        .texture = (WGPUTexture)tex.id,
        .aspect = WGPUTextureAspect_All,
        .mipLevel = 0,
        .origin = {0, 0, 0},
    };
    
    const WGPUTexelCopyBufferLayout source = {
        .offset = 0,
        .bytesPerRow = GetPixelSizeInBytes(tex.format) * tex.width,
        .rowsPerImage = tex.height,
    };

    const WGPUExtent3D writeSize = { 
        .depthOrArrayLayers = 1,
        .width = tex.width,
        .height = tex.height,
    };
    wgpuQueueWriteTexture(GetQueue(),
                          &destination,
                          data,
                          (uint64_t)tex.width * (uint64_t)tex.height * (uint64_t)GetPixelSizeInBytes(tex.format),
                          &source,
                          &writeSize);
}
RGAPI Texture3D LoadTexture3DPro(
    uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, RGTextureUsage usage, uint32_t sampleCount) {
    Texture3D ret zeroinit;
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.sampleCount = sampleCount;
    ret.format = format;
    WGPUTextureDescriptor tDesc = {
        .usage = usage,
        .dimension = WGPUTextureDimension_3D,
        .size = {width, height, depth},
        .format = toWGPUPixelFormat(format),
        .mipLevelCount = 1,
        .sampleCount = sampleCount,
        .viewFormatCount = 1,
        .viewFormats = &tDesc.format,
    };
    assert(tDesc.size.width > 0);
    assert(tDesc.size.height > 0);

    WGPUTextureViewDescriptor textureViewDesc zeroinit;
    textureViewDesc.aspect =
        ((format == PIXELFORMAT_DEPTH_24_PLUS || format == PIXELFORMAT_DEPTH_32_FLOAT) ? WGPUTextureAspect_DepthOnly
                                                                                       : WGPUTextureAspect_All);
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
void EndComputepassEx(DescribedComputepass *computePass) {
    if (computePass->cpEncoder) {
        wgpuComputePassEncoderEnd((WGPUComputePassEncoder)computePass->cpEncoder);
        wgpuComputePassEncoderRelease((WGPUComputePassEncoder)computePass->cpEncoder);
        computePass->cpEncoder = 0;
    }

    // TODO
    g_renderstate.activeComputepass = NULL;

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {0};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)computePass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)computePass->cmdEncoder);
}

void UnloadTexture(Texture tex) {
    for (uint32_t i = 0; i < tex.mipmaps; i++) {
        if (tex.mipViews[i]) {
            wgpuTextureViewRelease((WGPUTextureView)tex.mipViews[i]);
            tex.mipViews[i] = NULL;
        }
    }
    if (tex.view) {
        wgpuTextureViewRelease((WGPUTextureView)tex.view);
        tex.view = NULL;
    }
    if (tex.id) {
        wgpuTextureRelease((WGPUTexture)tex.id);
        tex.id = NULL;
    }
}
void PresentSurface(FullSurface *fsurface) {
    wgpuSurfacePresent((WGPUSurface)fsurface->surface); 
}

static inline uint64_t bgEntryHash(WGPUBindGroupEntry bge) {
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}

void BindShaderWithSettings(Shader shader, PrimitiveType drawMode, RenderSettings settings) {
    ShaderImpl *impl = allocatedShaderIDs_shc + shader.id;
    impl->state.primitiveType = drawMode;
    impl->state.settings = settings;
    WGPURenderPipeline activePipeline = PipelineHashMap_getOrCreate(&impl->pipelineCache, &impl->state, &impl->shaderModule, &impl->bglayout, &impl->layout);
    wgpuRenderPassEncoderSetPipeline(g_renderstate.activeRenderpass->rpEncoder, activePipeline);
    wgpuRenderPassEncoderSetBindGroup(g_renderstate.activeRenderpass->rpEncoder, 0, UpdateAndGetNativeBindGroup(&impl->bindGroup), 0, NULL);
}

void BindShader(Shader shader, PrimitiveType drawMode) {
    BindShaderWithSettings(shader, drawMode, g_renderstate.currentSettings);
}
void BindPipeline(DescribedPipeline *pipeline) {
    wgpuRenderPassEncoderSetPipeline((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder,
                                     (WGPURenderPipeline)pipeline->activePipeline);
}

void ResizeBuffer(DescribedBuffer *buffer, size_t newSize) {
    if (newSize == buffer->size)
        return;

    DescribedBuffer newbuffer = {0};
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
WGPURenderPassDepthStencilAttachment *defaultDSA(WGPUTextureView depth) {
    WGPURenderPassDepthStencilAttachment *dsa =
        (WGPURenderPassDepthStencilAttachment *)calloc(1, sizeof(WGPURenderPassDepthStencilAttachment));
    // The view of the depth texture
    dsa->view = depth;
    // dsa.depthSlice = 0;
    //  The initial value of the depth buffer, meaning "far"
    dsa->depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    dsa->depthLoadOp = WGPULoadOp_Load;
    dsa->depthStoreOp = WGPUStoreOp_Store;
    // we could turn off writing to the depth buffer globally here
    dsa->depthReadOnly = false;
    // Stencil setup, mandatory but unused
    dsa->stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
    dsa.stencilLoadOp = WGPULoadOp_Load;
    dsa.stencilStoreOp = WGPUStoreOp_Store;
#else
    dsa->stencilLoadOp = WGPULoadOp_Undefined;
    dsa->stencilStoreOp = WGPUStoreOp_Undefined;
#endif
    dsa->stencilReadOnly = true;
    return dsa;
}
void UnloadSampler(DescribedSampler sampler) { wgpuSamplerRelease((WGPUSampler)sampler.sampler); }
void ResizeBufferAndConserve(DescribedBuffer *buffer, size_t newSize) {
    if (newSize == buffer->size)
        return;

    size_t smaller = std_min_u64(newSize, buffer->size);
    DescribedBuffer newbuffer = {0};

    newbuffer.usage = buffer->usage;
    newbuffer.size = newSize;
    WGPUBufferDescriptor desc zeroinit;
    desc.usage = newbuffer.usage;
    desc.size = newbuffer.size;
    desc.mappedAtCreation = false;
    newbuffer.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &desc);
    WGPUCommandEncoderDescriptor edesc = {0};
    WGPUCommandBufferDescriptor bdesc = {0};
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &edesc);
    wgpuCommandEncoderCopyBufferToBuffer(enc, (WGPUBuffer)buffer->buffer, 0, (WGPUBuffer)newbuffer.buffer, 0, smaller);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, &bdesc);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    *buffer = newbuffer;
}

static inline WGPUStorageTextureAccess toStorageTextureAccess(access_type acc) {
    switch (acc) {
    case readonly:
        return WGPUStorageTextureAccess_ReadOnly;
    case readwrite:
        return WGPUStorageTextureAccess_ReadWrite;
    case writeonly:
        return WGPUStorageTextureAccess_WriteOnly;
    default:
        rg_unreachable();
    }
    return WGPUStorageTextureAccess_Force32;
}
static inline WGPUBufferBindingType toStorageBufferAccess(access_type acc) {
    switch (acc) {
    case readonly:
        return WGPUBufferBindingType_ReadOnlyStorage;
    case readwrite:
        return WGPUBufferBindingType_Storage;
    case writeonly:
        return WGPUBufferBindingType_Storage;
    default:
        rg_unreachable();
    }
    return WGPUBufferBindingType_Force32;
}
static inline WGPUTextureFormat toStorageTextureFormat(format_or_sample_type fmt) {
    switch (fmt) {
    case format_r32float:
        return WGPUTextureFormat_R32Float;
    case format_r32uint:
        return WGPUTextureFormat_R32Uint;
    case format_rgba8unorm:
        return WGPUTextureFormat_RGBA8Unorm;
    case format_rgba32float:
        return WGPUTextureFormat_RGBA32Float;
    default:
        rg_unreachable();
    }
    return WGPUTextureFormat_Force32;
}
static inline WGPUTextureSampleType toTextureSampleType(format_or_sample_type fmt) {
    switch (fmt) {
    case sample_f32:
        return WGPUTextureSampleType_Float;
    case sample_u32:
        return WGPUTextureSampleType_Uint;
    default:
        return WGPUTextureSampleType_Float; // rg_unreachable();
    }
    return WGPUTextureSampleType_Force32;
}

WGPUBindGroupLayoutEntry toWGPUBindGroupLayoutEntry(const ResourceTypeDescriptor* rtd, RGShaderStage shaderStage){
    
    WGPUBindGroupLayoutEntry ret = {0};

    ret.binding = rtd->location;
    ret.visibility = RG_to_WGPU_ShaderStage(shaderStage);

    switch (rtd->type) {
        default:
            rg_unreachable();
        case uniform_buffer:
            ret.visibility = shaderStage;
            ret.buffer.type = WGPUBufferBindingType_Uniform;
            ret.buffer.minBindingSize = rtd->minBindingSize;
            break;
        case storage_buffer: 
            ret.visibility = shaderStage;
            ret.buffer.type = toStorageBufferAccess(rtd->access);
            ret.buffer.minBindingSize = 0;
            break;
        case texture2d:
            ret.visibility = shaderStage;
            ret.texture.sampleType = toTextureSampleType(rtd->fstype);
            ret.texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
        case texture2d_array:
            ret.storageTexture.access = toStorageTextureAccess(rtd->access);
            ret.visibility = shaderStage;
            ret.storageTexture.format = toStorageTextureFormat(rtd->fstype);
            ret.storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
        case texture_sampler:
            ret.visibility = shaderStage;
            ret.sampler.type = WGPUSamplerBindingType_Filtering;
            break;
        case texture3d:
            ret.visibility = shaderStage;
            ret.texture.sampleType = toTextureSampleType(rtd->fstype);
            ret.texture.viewDimension = WGPUTextureViewDimension_3D;
            break;
        case storage_texture2d:
            ret.storageTexture.access = toStorageTextureAccess(rtd->access);
            ret.visibility = shaderStage;
            ret.storageTexture.format = toStorageTextureFormat(rtd->fstype);
            ret.storageTexture.viewDimension = WGPUTextureViewDimension_2D;
            break;
        case storage_texture2d_array:
            ret.storageTexture.access = toStorageTextureAccess(rtd->access);
            ret.visibility = shaderStage;
            ret.storageTexture.format = toStorageTextureFormat(rtd->fstype);
            ret.storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
        case storage_texture3d:
            ret.storageTexture.access = toStorageTextureAccess(rtd->access);
            ret.visibility = shaderStage;
            ret.storageTexture.format = toStorageTextureFormat(rtd->fstype);
            ret.storageTexture.viewDimension = WGPUTextureViewDimension_3D;
            break;
        }
    return ret;
}

DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor *uniforms, uint32_t uniformCount, bool compute) {
    DescribedBindGroupLayout ret = {0};
    WGPUShaderStage visible;
    WGPUShaderStage vfragmentOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Fragment;
    WGPUShaderStage vvertexOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Vertex;
    if (compute) {
        visible = WGPUShaderStage_Compute;
    } else {
        visible = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    }

    
    WGPUBindGroupLayoutEntry* blayouts = (WGPUBindGroupLayoutEntry*)VLAStack_alloc(&g_vlastack, (size_t)uniformCount *  sizeof(WGPUBindGroupLayoutEntry));
    memset(blayouts, 0, (size_t)uniformCount * sizeof(WGPUBindGroupLayoutEntry));
    
    
    for(uint32_t i = 0;i < uniformCount;i++){
        blayouts[i] = toWGPUBindGroupLayoutEntry(uniforms + i, compute ? RGShaderStage_Compute : (RGShaderStage_Vertex | RGShaderStage_Fragment));
    }
    const WGPUBindGroupLayoutDescriptor bglayoutdesc = {
        .entryCount = uniformCount,
        .entries = blayouts,
    };

    ret.entries = (ResourceTypeDescriptor *)RL_CALLOC(uniformCount, sizeof(ResourceTypeDescriptor));
    if (uniformCount > 0) {
        memcpy(ret.entries, uniforms, uniformCount * sizeof(ResourceTypeDescriptor));
    }
    ret.layout = wgpuDeviceCreateBindGroupLayout((WGPUDevice)GetDevice(), &bglayoutdesc);

    VLAStack_free(&g_vlastack, blayouts);
    return ret;
}

RGAPI FullSurface CompleteSurface(void *nsurface, int width, int height) {
    FullSurface ret = {0};
    ret.surface = (WGPUSurface)nsurface;
    negotiateSurfaceFormatAndPresentMode(nsurface);
    WGPUSurfaceCapabilities capa = {0};

    wgpuSurfaceGetCapabilities(ret.surface, GetAdapter(), &capa);

    WGPUPresentMode presentMode = WGPUPresentMode_Undefined;
    WGPUPresentMode thm = g_renderstate.throttled_PresentMode;
    WGPUPresentMode um = g_renderstate.unthrottled_PresentMode;
    if (g_renderstate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT) {
        presentMode = (((g_renderstate.unthrottled_PresentMode == WGPUPresentMode_Mailbox) ? um : thm));
    } else if (g_renderstate.windowFlags & FLAG_VSYNC_HINT) {
        presentMode = thm;
    } else {
        presentMode = um;
    }
    const char *presentModeName;
    switch (presentMode) {
    case WGPUPresentMode_Undefined:
        presentModeName = "WGPUPresentMode_Undefined";
        break;
    case WGPUPresentMode_Fifo:
        presentModeName = "WGPUPresentMode_Fifo";
        break;
    case WGPUPresentMode_FifoRelaxed:
        presentModeName = "WGPUPresentMode_FifoRelaxed";
        break;
    case WGPUPresentMode_Immediate:
        presentModeName = "WGPUPresentMode_Immediate";
        break;
    case WGPUPresentMode_Mailbox:
        presentModeName = "WGPUPresentMode_Mailbox";
        break;
    default:
        presentModeName = "Not a WGPUPresentMode enum";
        break;
    }
    TRACELOG(LOG_INFO, "Initialized surface with %s", presentModeName);

    const PixelFormat format = g_renderstate.frameBufferFormat;
    WGPUSurfaceConfiguration config = {
        .device = (WGPUDevice)GetDevice(),
        .format = toWGPUPixelFormat(format),
        .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc,
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .viewFormatCount = 1,
        .viewFormats = &config.format,
        .alphaMode = WGPUCompositeAlphaMode_Opaque,
        .presentMode = presentMode,
    };

    ret.presentMode = WGPU_to_RG_PresentMode(config.presentMode);
    ret.width = config.width;
    ret.height = config.height;
    ret.format = format;

    ret.renderTarget = LoadRenderTexture(width, height);
    wgpuSurfaceConfigure((WGPUSurface)ret.surface, &config);
    return ret;
}

StagingBuffer GenStagingBuffer(size_t size, RGBufferUsage usage) {
    StagingBuffer ret = {0};
    WGPUBufferDescriptor descriptor1 = {
        .nextInChain = NULL,
        .label = {0},
        .usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc,
        .size = size,
        .mappedAtCreation = true
    };
    WGPUBufferDescriptor descriptor2 = {
        .nextInChain = NULL,
        .label = {0},
        .usage = usage,
        .size = size,
        .mappedAtCreation = false
    };

    ret.gpuUsable.buffer = wgpuDeviceCreateBuffer(GetDevice(), &descriptor1);
    ret.mappable.buffer = wgpuDeviceCreateBuffer(GetDevice(), &descriptor2);
    ret.map = wgpuBufferGetMappedRange(ret.mappable.buffer, 0, size);
    return ret;
}
void RecreateStagingBuffer(StagingBuffer *buffer) {
    wgpuBufferRelease((WGPUBuffer)buffer->gpuUsable.buffer);
    WGPUBufferDescriptor gpudesc = {
        .size = buffer->gpuUsable.size,
        .usage = buffer->gpuUsable.usage,
    };
    buffer->gpuUsable.buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &gpudesc);
}
static void emptyfunction_123(WGPUMapAsyncStatus status, WGPUStringView message, void *userdata1, void *userdata2) {}
void UpdateStagingBuffer(StagingBuffer *buffer) {
    wgpuBufferUnmap((WGPUBuffer)buffer->mappable.buffer);
    WGPUCommandEncoderDescriptor arg = {0};
    WGPUCommandBufferDescriptor arg2 = {0};
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &arg);

    wgpuCommandEncoderCopyBufferToBuffer(
        enc, (WGPUBuffer)buffer->mappable.buffer, 0, (WGPUBuffer)buffer->gpuUsable.buffer, 0, buffer->mappable.size);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, &arg2);
    wgpuQueueSubmit(GetQueue(), 1, &buf);

    WGPUBufferMapCallbackInfo callbackInfo = {
        .mode = WGPUCallbackMode_WaitAnyOnly,
        .callback = emptyfunction_123,
        .userdata1 = NULL,
        .userdata2 = NULL,
    };
    WGPUFuture future = wgpuBufferMapAsync(
        buffer->mappable.buffer, WGPUMapMode_Write, 0, wgpuBufferGetSize(buffer->mappable.buffer), callbackInfo);

    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    WGPUFutureWaitInfo winfo = {future, 0};
    wgpuInstanceWaitAny((WGPUInstance)GetInstance(), 1, &winfo, UINT64_MAX);
    buffer->map = (vertex *)wgpuBufferGetMappedRange((WGPUBuffer)buffer->mappable.buffer, 0, buffer->mappable.size);
}
void UnloadStagingBuffer(StagingBuffer *buf) {
    wgpuBufferRelease((WGPUBuffer)buf->gpuUsable.buffer);
    wgpuBufferUnmap((WGPUBuffer)buf->mappable.buffer);
    wgpuBufferRelease((WGPUBuffer)buf->mappable.buffer);
}

const char mipmapComputerSource2[] = 
"@group(0) @binding(0) var previousMipLevel: texture_2d<f32>;\n"
"@group(0) @binding(1) var nextMipLevel: texture_storage_2d<rgba8unorm, write>;\n"
"@compute @workgroup_size(8, 8)\n"
"fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {\n"
"    let offset = vec2<u32>(0, 1);\n"
"    \n"
"    let color = (\n"
"        textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +\n"
"        textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +\n"
"        textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +\n"
"        textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)\n"
"    ) * 0.25;\n"
"    textureStore(nextMipLevel, id.xy, color);\n"
"}\n";
DescribedComputePipeline *mipmap__cpl = 0;
void GenTextureMipmaps(Texture2D *tex) {
    if(mipmap__cpl == NULL){
        mipmap__cpl = LoadComputePipeline(mipmapComputerSource2);
    }
    BeginComputepass();

    for (int i = 0; i < tex->mipmaps - 1; i++) {
        SetBindgroupTextureView(&mipmap__cpl->bindGroup, 0, (WGPUTextureView)tex->mipViews[i]);
        SetBindgroupTextureView(&mipmap__cpl->bindGroup, 1, (WGPUTextureView)tex->mipViews[i + 1]);
        if (i == 0) {
            BindComputePipeline(mipmap__cpl);
        }
        ComputePassSetBindGroup(&g_renderstate.computepass, 0, &mipmap__cpl->bindGroup);
        uint32_t divisor = (1 << i) * 8;
        DispatchCompute((tex->width + divisor - 1) & -(divisor) / 8, (tex->height + divisor - 1) & -(divisor) / 8, 1);
    }
    EndComputepass();
}

void UpdateBindGroup(DescribedBindGroup *bg) {
    // std::cout << "Updating bindgroup with " << bg->desc.entryCount << " entries" << std::endl;
    // std::cout << "Updating bindgroup with " << bg->desc.entries[1].binding << " entries" << std::endl;
    if (bg->needsUpdate) {
        WGPUBindGroupDescriptor desc zeroinit;
        WGPUBindGroupEntry* aswgpu = (WGPUBindGroupEntry*)RL_CALLOC(bg->entryCount, sizeof(WGPUBindGroupEntry));
        for (uint32_t i = 0; i < bg->entryCount; i++) {
            WGPUBindGroupEntry* to = aswgpu + i;
            const ResourceDescriptor* from = bg->entries + i;
            to->binding = from->binding;
            to->buffer = from->buffer;
            to->offset = from->offset;
            to->size = from->size;
            to->sampler = from->sampler;
            to->textureView = from->textureView;
        }

        desc.entries = aswgpu;
        desc.entryCount = bg->entryCount;
        desc.layout = bg->layout->layout;
        bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &desc);
        bg->needsUpdate = false;
        RL_FREE(aswgpu);
    }
}

static inline uint64_t bgEntryHashRD(const ResourceDescriptor bge) {
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}

void UpdateBindGroupEntry(DescribedBindGroup *bg, size_t index, ResourceDescriptor entry) {
    if (index >= bg->entryCount) {
        TRACELOG(LOG_WARNING, "Trying to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->entryCount);
        // return;
    }
    WGPUBuffer newpuffer = entry.buffer;
    WGPUTextureView newtexture = entry.textureView;
    if (newtexture && bg->entries[index].textureView == newtexture) {
        // return;
    }
    uint64_t oldHash = bg->descriptorHash;
    
    bg->descriptorHash ^= bgEntryHashRD(bg->entries[index]);
    if (entry.buffer) {
        wgpuBufferAddRef((WGPUBuffer)entry.buffer);
    }
    if (entry.textureView) {
        wgpuTextureViewAddRef((WGPUTextureView)entry.textureView);
    }

    if (bg->entries[index].buffer) {
        wgpuBufferRelease((WGPUBuffer)bg->entries[index].buffer);
        bg->entries[index].buffer = 0;
    }
    if (bg->entries[index].textureView) {
        wgpuTextureViewRelease((WGPUTextureView)bg->entries[index].textureView);
        bg->entries[index].textureView = 0;
    }

    bg->entries[index] = entry;
    bg->descriptorHash ^= bgEntryHashRD(bg->entries[index]);

    // TODO don't release and recreate here or find something better
    if (true /*|| donotcache*/) {
        if (bg->bindGroup)
            wgpuBindGroupRelease((WGPUBindGroup)bg->bindGroup);
        bg->bindGroup = NULL;
    }
    // else if(!bg->needsUpdate && bg->bindGroup){
    //     g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //     bg->bindGroup = NULL;
    // }
    bg->needsUpdate = true;

    // bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &(bg->desc));
}

void GetNewTexture(FullSurface *fsurface) {
    if (fsurface->headless) {
        return;
    } else {
        WGPUSurfaceTexture surfaceTexture = {0};

        wgpuSurfaceGetCurrentTexture((WGPUSurface)fsurface->surface, &surfaceTexture);
        if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
            TRACELOG(LOG_DEBUG, "wgpuSurfaceGetCurrentTexture called with SuccessOptimal");
            TRACELOG(LOG_DEBUG, "Acquired texture is %u x %u", wgpuTextureGetWidth(surfaceTexture.texture), wgpuTextureGetHeight(surfaceTexture.texture));

        } else if (surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
            TRACELOG(LOG_DEBUG, "wgpuSurfaceGetCurrentTexture called with SuccessSubptimal");
        } else {
            TRACELOG(LOG_DEBUG, "wgpuSurfaceGetCurrentTexture called with error");
        }
        // TODO: some better surface recovery handling, doesn't seem to be an issue for now however
        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal) {
            WGPUTextureFormat viewFormat = toWGPUPixelFormat(fsurface->format);
            WGPUSurfaceConfiguration sconf = {
                .nextInChain = NULL,
                .device = GetDevice(),
                .format = toWGPUPixelFormat(fsurface->format),
                .usage = WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment,
                .width = fsurface->width,
                .height = fsurface->height,
                .viewFormatCount = 1,
                .viewFormats = &viewFormat,
                .alphaMode = WGPUCompositeAlphaMode_Opaque,
                .presentMode = RG_to_WGPU_PresentMode(fsurface->presentMode),
            };
            wgpuSurfaceConfigure((WGPUSurface)fsurface->surface, &sconf);
            wgpuSurfaceGetCurrentTexture((WGPUSurface)fsurface->surface, &surfaceTexture);
        }
        // rassert(surfaceTexture.status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal, "WGPUSurface did not return
        // optimal, instead: %d", surfaceTexture.status);
        if (fsurface->renderTarget.texture.id) {
            wgpuTextureRelease(fsurface->renderTarget.texture.id);
        }
        fsurface->renderTarget.texture.id = surfaceTexture.texture;
        fsurface->renderTarget.texture.width = wgpuTextureGetWidth(surfaceTexture.texture);
        fsurface->renderTarget.texture.height = wgpuTextureGetHeight(surfaceTexture.texture);
        
        if(fsurface->renderTarget.depth.width != fsurface->renderTarget.texture.width || fsurface->renderTarget.depth.height != fsurface->renderTarget.texture.height){
            TRACELOG(LOG_WARNING, "Unforeseen rescale to %u x %u\n", fsurface->renderTarget.texture.width, fsurface->renderTarget.texture.height);
            UnloadTexture(fsurface->renderTarget.depth);
            fsurface->renderTarget.depth = LoadDepthTexture(fsurface->renderTarget.texture.width, fsurface->renderTarget.texture.height);
        }

        if (fsurface->renderTarget.texture.view) {
            wgpuTextureViewRelease(fsurface->renderTarget.texture.view);
        }
        WGPUTextureViewDescriptor tvDesc = {
            .label ={
                .data = "SurfaceTextureView",
                .length = sizeof("SurfaceTextureView") - 1,
            },
            .format = wgpuTextureGetFormat(surfaceTexture.texture),
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
            .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc
        };
        fsurface->renderTarget.texture.view = wgpuTextureCreateView(surfaceTexture.texture, NULL);
    }
}
struct {
    WGPUBool requested;
    WGPULimits limits;
} limitsToBeRequested;

WGPUBackendType requestedBackend = DEFAULT_BACKEND;
WGPUAdapterType requestedAdapterType = WGPUAdapterType_Unknown;

void setlimit(WGPULimits* limits, LimitType limit, uint64_t value) {
    switch (limit) {
    case maxImmediateSize:
        break; // TODO
    case maxTextureDimension1D:
        limits->maxTextureDimension1D = value;
    case maxTextureDimension2D:
        limits->maxTextureDimension2D = value;
    case maxTextureDimension3D:
        limits->maxTextureDimension3D = value;
    case maxTextureArrayLayers:
        limits->maxTextureArrayLayers = value;
    case maxBindGroups:
        limits->maxBindGroups = value;
    case maxBindGroupsPlusVertexBuffers:
        limits->maxBindGroupsPlusVertexBuffers = value;
    case maxBindingsPerBindGroup:
        limits->maxBindingsPerBindGroup = value;
    case maxDynamicUniformBuffersPerPipelineLayout:
        limits->maxDynamicUniformBuffersPerPipelineLayout = value;
    case maxDynamicStorageBuffersPerPipelineLayout:
        limits->maxDynamicStorageBuffersPerPipelineLayout = value;
    case maxSampledTexturesPerShaderStage:
        limits->maxSampledTexturesPerShaderStage = value;
    case maxSamplersPerShaderStage:
        limits->maxSamplersPerShaderStage = value;
    case maxStorageBuffersPerShaderStage:
        limits->maxStorageBuffersPerShaderStage = value;
    case maxStorageTexturesPerShaderStage:
        limits->maxStorageTexturesPerShaderStage = value;
    case maxUniformBuffersPerShaderStage:
        limits->maxUniformBuffersPerShaderStage = value;
    case maxUniformBufferBindingSize:
        limits->maxUniformBufferBindingSize = value;
    case maxStorageBufferBindingSize:
        limits->maxStorageBufferBindingSize = value;
    case minUniformBufferOffsetAlignment:
        limits->minUniformBufferOffsetAlignment = value;
    case minStorageBufferOffsetAlignment:
        limits->minStorageBufferOffsetAlignment = value;
    case maxVertexBuffers:
        limits->maxVertexBuffers = value;
    case maxBufferSize:
        limits->maxBufferSize = value;
    case maxVertexAttributes:
        limits->maxVertexAttributes = value;
    case maxVertexBufferArrayStride:
        limits->maxVertexBufferArrayStride = value;
    case maxInterStageShaderVariables:
        limits->maxInterStageShaderVariables = value;
    case maxColorAttachments:
        limits->maxColorAttachments = value;
    case maxColorAttachmentBytesPerSample:
        limits->maxColorAttachmentBytesPerSample = value;
    case maxComputeWorkgroupStorageSize:
        limits->maxComputeWorkgroupStorageSize = value;
    case maxComputeInvocationsPerWorkgroup:
        limits->maxComputeInvocationsPerWorkgroup = value;
    case maxComputeWorkgroupSizeX:
        limits->maxComputeWorkgroupSizeX = value;
    case maxComputeWorkgroupSizeY:
        limits->maxComputeWorkgroupSizeY = value;
    case maxComputeWorkgroupSizeZ:
        limits->maxComputeWorkgroupSizeZ = value;
    case maxComputeWorkgroupsPerDimension:
        limits->maxComputeWorkgroupsPerDimension = value;
        // case maxImmediateSize: limits.maxImmediateSize = value;
    }
}
void RequestLimit(LimitType limit, uint64_t value) {
    limitsToBeRequested.requested = (WGPUBool)1;

    setlimit(&limitsToBeRequested.limits, limit, value);
}
void RequestAdapterType(AdapterType type) {
    switch (type) {
    case SOFTWARE_RENDERER:
        requestedAdapterType = WGPUAdapterType_CPU;
        break;
    case INTEGRATED_GPU:
        requestedAdapterType = WGPUAdapterType_IntegratedGPU;
        break;
    case DISCRETE_GPU:
        requestedAdapterType = WGPUAdapterType_DiscreteGPU;
        break;
    }
}
void DummySubmitOnQueue() {
#if SUPPORT_VULKAN_BACKEND == 1
    wgpuDeviceTick(GetDevice());
#endif
}
void RequestBackend(BackendType backend) {
    switch (backend) {
    case BackendType_Undefined:
        requestedBackend = WGPUBackendType_Undefined;
    case BackendType_Null:
        requestedBackend = WGPUBackendType_Null;
    case BackendType_WebGPU:
        requestedBackend = WGPUBackendType_WebGPU;
    case BackendType_D3D11:
        requestedBackend = WGPUBackendType_D3D11;
    case BackendType_D3D12:
        requestedBackend = WGPUBackendType_D3D12;
    case BackendType_Metal:
        requestedBackend = WGPUBackendType_Metal;
    case BackendType_Vulkan:
        requestedBackend = WGPUBackendType_Vulkan;
    case BackendType_OpenGL:
        requestedBackend = WGPUBackendType_OpenGL;
    case BackendType_OpenGLES:
        requestedBackend = WGPUBackendType_OpenGLES;
    default:
        abort();
    }
}
void requestAdapterCallback(
    WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void *_userdata1, void *_userdata2) {
    wgpustate *st = (wgpustate *)_userdata1;
    if (status == WGPURequestAdapterStatus_Success) {
        st->adapter = adapter;
    } else {
        char tmp[2048] = {};
        memcpy(tmp, message.data, std_min_u64(message.length, (size_t)2047));
        TRACELOG(LOG_FATAL, "Failed to get an adapter: %s\n", tmp);
    }
}
void requestDeviceCallback(
    WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void *_userdata1, void *_userdata2) {
    wgpustate *st = (wgpustate *)_userdata1;
    if (status == WGPURequestDeviceStatus_Success) {
        st->device = device;
    } else {
        char tmp[2048] = {};
        memcpy(tmp, message.data, std_min_u64(message.length, (size_t)2047));
        TRACELOG(LOG_FATAL, "Failed to create device: %s\n", tmp);
    }
}

//inline std::string WGPUStringViewToString(WGPUStringView view) {
//    if (view.data == NULL) {
//        return "";
//    }
//    if (view.length == WGPU_STRLEN) {
//        // Use the constructor that takes a null-terminated C-string.
//        return std::string(view.data);
//    } else {
//        // Use the constructor that takes a pointer and a specific length.
//        return std::string(view.data, view.length);
//    }
//}

char* NullTerminatedStringFromView_(WGPUStringView view){
    size_t length = view.length == WGPU_STRLEN ? strlen(view.data) : view.length;
    if(length == 0){
        return NULL;
    }
    char* retData = (char*)RL_MALLOC(length + 1);
    memcpy(retData, view.data, length);
    retData[length] = '\0';
    return retData;
}
int ueccount_ = 0;
void uncapturedErrorCallback(
    const WGPUDevice *device, WGPUErrorType type, WGPUStringView message, void *_userdata1, void *_userdata2) {
    if(ueccount_++ >= 5)return;
    const char *errorTypeName = "";
    switch (type) {
    case WGPUErrorType_Validation:
        errorTypeName = "Validation";
        break;
    case WGPUErrorType_OutOfMemory:
        errorTypeName = "Out of memory";
        break;
    case WGPUErrorType_Unknown:
        errorTypeName = "Unknown";
        break;
    case WGPUErrorType_NoError:
        errorTypeName = "No Error";
        break;
    case WGPUErrorType_Internal:
        errorTypeName = "Internal";
        break;
    default:
        break;
        // abort();
        // rg_unreachable();
    }
    char* snprintfCompatibleMessage = NullTerminatedStringFromView_(message);
    TRACELOG(LOG_ERROR, "%s error: %s", errorTypeName, snprintfCompatibleMessage);
    RL_FREE(snprintfCompatibleMessage);
    
    //assert(false);
    //rg_trap();
};

static inline bool bcompat(WGPUBackendType backend) {
    switch (backend) {
    case WGPUBackendType_D3D12:
    case WGPUBackendType_Metal:
    case WGPUBackendType_Vulkan:
    case WGPUBackendType_WebGPU:
    case WGPUBackendType_Null:
        return false;
    case WGPUBackendType_D3D11:
    case WGPUBackendType_OpenGL:
    case WGPUBackendType_OpenGLES:
        return true;
    case WGPUBackendType_Undefined:
    default:
        rg_unreachable();
        // return false;
    }
};

void bw_deviceLostCallback(const WGPUDevice *device, WGPUDeviceLostReason reason, WGPUStringView message, void *userdata1, void *userdata2) {
    const char *reasonName = "";
    switch (reason) {
    case WGPUDeviceLostReason_Unknown:
        reasonName = "Unknown";
        break;
    case WGPUDeviceLostReason_Destroyed:
        reasonName = "Destroyed";
        break;
    case WGPUDeviceLostReason_FailedCreation:
        reasonName = "FailedCreation";
        break;
    default:
        rg_unreachable();
    }
    char messages[65536] = {0};

    if (message.length == WGPU_STRLEN) {
        size_t len = strlen(message.data);
        if(len < sizeof(messages) - 1){
            memcpy(messages, message.data, len);
            messages[len] = '\0';
        }
    } else {
        size_t len = message.length;
        if(len < sizeof(messages) - 1){
            memcpy(messages, message.data, len);
            messages[len] = '\0';
        }
    }
    TRACELOG(LOG_FATAL, "Device lost because of %s: %s", reasonName, messages);
};

static void onAdapter(
    WGPURequestAdapterStatus status,
    WGPUAdapter adapter,
    WGPUStringView message,
    void* userdata1,
    void* userdata2)
{
    wgpustate* state = (wgpustate*)userdata1;
    if (status == WGPURequestAdapterStatus_Success) {
        state->adapter = adapter;
        TRACELOG(LOG_DEBUG, "Adapter OK: %p\n", adapter);
    } else {
        TRACELOG(LOG_ERROR, "Adapter failed: %.*s\n", (int)message.length, message.data);
    }
}
static void onDevice(
    WGPURequestDeviceStatus status,
    WGPUDevice device,
    WGPUStringView message,
    void* userdata1,
    void* userdata2)
{
    wgpustate* app = (wgpustate*)userdata1;
    if (status == WGPURequestDeviceStatus_Success) {
        app->device = device;
        TRACELOG(LOG_DEBUG, "Device OK: %p\n", device);
    } else {
        TRACELOG(LOG_ERROR, "Device failed: %.*s\n", (int)message.length, message.data);
    }
}


static bool initResumeEntry      (InitContext_Impl _ctx);
static bool InitBackend_DoTheRest(InitContext_Impl _ctx);


InitContext_Impl g_init_context_;

#if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)

static bool kickDeviceRequest(double t, void* ctx){
    (void)t; (void)ctx;
    initResumeEntry(g_init_context_);
    return false;
}
static bool kickFinalizeAndContinue(double t, void* ctx){
    (void)t; (void)ctx;
    InitBackend_DoTheRest(g_init_context_);
    return false;
}

#endif

static bool animationFrameWaitForAdapter(double time, void* _ctx){
    if(((struct wgpustate*)g_init_context_.wgpustate)->adapter){
        // Adapter ready. request frame to finish init and call continuation point
        #if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
        emscripten_request_animation_frame_loop(kickDeviceRequest, NULL);
        #else
        initResumeEntry(g_init_context_);
        #endif
        return false;
    }
    else{
        return true; // Keep calling it
    }
 }

 static bool animationFrameWaitForDevice(double time, void* _ctx){
    if(((struct wgpustate*)g_init_context_.wgpustate)->device){
        // Device ready. request frame to finish init and call continuation point
        #if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
        emscripten_request_animation_frame_loop(kickFinalizeAndContinue, NULL);
        #else
        InitBackend_DoTheRest(g_init_context_);
        #endif
        return false;
    }
    else{
        return true; // Keep calling it
    }
}
static bool initAdapterAndDevice(InitContext_Impl _ctx){
    InitContext_Impl* ctx = &_ctx;

    WGPURequestAdapterOptions requestAdapterOptions = {
        .backendType = requestedBackend,
        .forceFallbackAdapter = (requestedAdapterType == WGPUAdapterType_CPU),
        .featureLevel = bcompat(requestedBackend) ? WGPUFeatureLevel_Compatibility : WGPUFeatureLevel_Core
    };
    switch (requestedAdapterType) {
        case WGPUAdapterType_CPU:
            requestAdapterOptions.forceFallbackAdapter = true;
            break;
        case WGPUAdapterType_DiscreteGPU: /* fallthrough */
        case WGPUAdapterType_IntegratedGPU:
            requestAdapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;
            break;
        default:
            break;
    }

    WGPURequestAdapterCallbackInfo requestAdapterCallbackInfo = {
        .callback  = onAdapter,
    #if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
        .mode      = WGPUCallbackMode_AllowSpontaneous,
    #else
        .mode      = WGPUCallbackMode_WaitAnyOnly,
    #endif
        .userdata1 = ctx->wgpustate,
    };

    wgpustate* state = (wgpustate*)(ctx->wgpustate);

    WGPUFuture arfuture = wgpuInstanceRequestAdapter(state->instance, &requestAdapterOptions, requestAdapterCallbackInfo);

#if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
    (void)arfuture;
    g_init_context_ = _ctx;
    emscripten_request_animation_frame_loop(animationFrameWaitForAdapter, NULL);
    return true;
#else
    WGPUFutureWaitInfo arFutureWaitInfo = {
        .future    = arfuture,
        .completed = 0
    };
    WGPUWaitStatus arWaitStatus = wgpuInstanceWaitAny(state->instance, 1, &arFutureWaitInfo, UINT32_MAX);
    (void)arWaitStatus; // status is logged in callbacks if needed
    initResumeEntry(_ctx);
    return true;
#endif
}


static bool initResumeEntry(InitContext_Impl _ctx){
    InitContext_Impl* ctx = &_ctx;

    WGPUFeatureName fnames[2] = {
        WGPUFeatureName_ClipDistances,
        WGPUFeatureName_Float32Filterable,
    };

    WGPUDeviceDescriptor deviceDesc = {
    #ifndef __EMSCRIPTEN__
        .requiredFeatureCount = 2,
        .requiredFeatures = fnames,
    #endif
        .deviceLostCallbackInfo = {
            .callback = bw_deviceLostCallback,
            .mode = WGPUCallbackMode_AllowSpontaneous
        },
        .uncapturedErrorCallbackInfo = {
            .callback = uncapturedErrorCallback,
        }
    };

    WGPURequestDeviceCallbackInfo rdCallback = {
    #if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
        .mode = WGPUCallbackMode_AllowSpontaneous,
    #else
        .mode = WGPUCallbackMode_WaitAnyOnly,
    #endif
        .callback = onDevice,
        .userdata1 = ctx->wgpustate
    };

    wgpustate* state = (wgpustate*)(ctx->wgpustate);
    WGPUFuture rdFuture = wgpuAdapterRequestDevice(state->adapter, &deviceDesc, rdCallback);

#if defined(__EMSCRIPTEN__) && !defined(ASSUME_EM_ASYNCIFY)
    (void)rdFuture;
    g_init_context_ = _ctx;
    emscripten_request_animation_frame_loop(animationFrameWaitForDevice, NULL);
    return true;
#else
    WGPUFutureWaitInfo rdFutureWaitInfo = { .future = rdFuture, .completed = 0 };
    (void)wgpuInstanceWaitAny(state->instance, 1, &rdFutureWaitInfo, UINT32_MAX);
    InitBackend_DoTheRest(_ctx);
    return true;
#endif
}


void InitBackend(InitContext_Impl _ctx) {
    g_wgpustate = (wgpustate){0};
    InitContext_Impl* ctx = &_ctx;
    wgpustate *state = &g_wgpustate;
    WGPUInstanceFeatureName instanceFeatures[2] = {
        WGPUInstanceFeatureName_TimedWaitAny,
        WGPUInstanceFeatureName_ShaderSourceSPIRV
    };
    WGPUInstanceDescriptor idesc = {
        #if !defined(__EMSCRIPTEN__) || defined(ASSUME_EM_ASYNCIFY) 
        .requiredFeatureCount = 1,
        .requiredFeatures = instanceFeatures
        #endif
    };

    state->instance = wgpuCreateInstance(&idesc);
    _ctx.wgpustate = (void*)state;
    initAdapterAndDevice(_ctx);

}
static bool InitBackend_DoTheRest(InitContext_Impl _ctx){
    InitContext_Impl* ctx = &_ctx;

    wgpustate* state = (wgpustate*)(ctx->wgpustate);
    if (state->adapter == NULL) {
        TRACELOG(LOG_FATAL, "Adapter is null\n");
        return false;
    }

    WGPUAdapterInfo info = (WGPUAdapterInfo){0};
    wgpuAdapterGetInfo(state->adapter, &info);

    char* deviceName  = NullTerminatedStringFromView_(info.device);
    char* architecture= NullTerminatedStringFromView_(info.architecture);
    char* description = NullTerminatedStringFromView_(info.description);
    char* vendor      = NullTerminatedStringFromView_(info.vendor);

    const char *adapterTypeString =
        (info.adapterType == WGPUAdapterType_CPU) ? "CPU" :
        (info.adapterType == WGPUAdapterType_IntegratedGPU ? "Integrated GPU" : "Dedicated GPU");

    const char* backendString = "?invalid WGPUBackendType";
    switch(info.backendType){
        case WGPUBackendType_D3D11:   backendString = "DirectX 11";    break;
        case WGPUBackendType_D3D12:   backendString = "DirectX 12";    break;
        case WGPUBackendType_Vulkan:  backendString = "Vulkan";        break;
        case WGPUBackendType_OpenGL:  backendString = "Desktop OpenGL";break;
        case WGPUBackendType_OpenGLES:backendString = "OpenGL ES";     break;
        case WGPUBackendType_Metal:   backendString = "Metal";         break;
        case WGPUBackendType_WebGPU:  backendString = "WebGPU";        break;
        default: break;
    }

    TRACELOG(LOG_INFO, "Using adapter %s %s", vendor, deviceName);
    TRACELOG(LOG_INFO, "Using adapter %s", info.device.data);
    TRACELOG(LOG_INFO, "Adapter description: %s", description);
    TRACELOG(LOG_INFO, "Adapter architecture: %s", architecture);
    TRACELOG(LOG_INFO, "%s renderer running on %s", backendString, adapterTypeString);

    RL_FREE(deviceName);
    RL_FREE(architecture);
    RL_FREE(description);
    RL_FREE(vendor);

    WGPULimits adapterLimits = (WGPULimits){0};
    WGPULimits slimits       = (WGPULimits){0};
    wgpuAdapterGetLimits(state->adapter, &adapterLimits);
    wgpuDeviceGetLimits(state->device, &slimits);

    TraceLog(LOG_INFO, "Adapter could support %u bindings per bindgroup", (unsigned)adapterLimits.maxBindingsPerBindGroup);
    TraceLog(LOG_INFO, "Adapter could support %u bindgroups", (unsigned)adapterLimits.maxBindGroups);
    TraceLog(LOG_INFO, "Adapter could support buffers up to %llu megabytes", (unsigned long long)(adapterLimits.maxBufferSize / 1000000ull));
    TraceLog(LOG_INFO, "Adapter could support textures up to %u x %u", (unsigned)adapterLimits.maxTextureDimension2D, (unsigned)adapterLimits.maxTextureDimension2D);
    TraceLog(LOG_INFO, "Adapter could support %u VBO slots", (unsigned)adapterLimits.maxVertexBuffers);

    TraceLog(LOG_INFO, "Device supports %u bindings per bindgroup", (unsigned)slimits.maxBindingsPerBindGroup);
    TraceLog(LOG_INFO, "Device supports %u bindgroups", (unsigned)slimits.maxBindGroups);
    TraceLog(LOG_INFO, "Device supports buffers up to %llu megabytes", (unsigned long long)(slimits.maxBufferSize / 1000000ull));
    TraceLog(LOG_INFO, "Device supports textures up to %u x %u", (unsigned)slimits.maxTextureDimension2D, (unsigned)slimits.maxTextureDimension2D);
    TraceLog(LOG_INFO, "Device supports %u VBO slots", (unsigned)slimits.maxVertexBuffers);

    state->queue = wgpuDeviceGetQueue(state->device);
    ctx->continuationPoint(_ctx);
    return true;
}


const char *TextureFormatName(WGPUTextureFormat fmt);
bool negotiateSurfaceFormatAndPresentMode_called = false;
void negotiateSurfaceFormatAndPresentMode(const void *SurfaceHandle) {
    const WGPUSurface surf = (WGPUSurface)SurfaceHandle;
    if (negotiateSurfaceFormatAndPresentMode_called)
        return;
    negotiateSurfaceFormatAndPresentMode_called = true;
    WGPUSurfaceCapabilities capabilities;
    wgpuSurfaceGetCapabilities(surf, (WGPUAdapter)GetAdapter(), &capabilities);
    {
        // std::string presentModeString;
        // for(uint32_t i = 0;i < capabilities.presentModeCount;i++){
        //     presentModeString += presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[i]).c_str();
        //     if(i < capabilities.presentModeCount - 1){
        //         presentModeString += ", ";
        //     }
        // }
        // TRACELOG(LOG_INFO, "Supported present modes: %s", presentModeString.c_str());
    }
    if (capabilities.presentModeCount == 0) {
        TRACELOG(LOG_ERROR, "No presentation modes supported! This surface is most likely invalid");
    } else if (capabilities.presentModeCount == 1) {
        // TRACELOG(LOG_INFO, "Only %s supported",
        // presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[0]).c_str());
        g_renderstate.unthrottled_PresentMode = capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode = capabilities.presentModes[0];
    } else if (capabilities.presentModeCount > 1) {
        g_renderstate.unthrottled_PresentMode = capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode   = capabilities.presentModes[0];

        unsigned i;
        int hasFifo = 0, hasFifoRelaxed = 0, hasMailbox = 0, hasImmediate = 0;

        for (i = 0; i < capabilities.presentModeCount; i++) {
            switch (capabilities.presentModes[i]) {
                case WGPUPresentMode_Fifo:        hasFifo = 1; break;
                case WGPUPresentMode_FifoRelaxed: hasFifoRelaxed = 1; break;
                case WGPUPresentMode_Mailbox:     hasMailbox = 1; break;
                case WGPUPresentMode_Immediate:   hasImmediate = 1; break;
                default: break;
            }
        }

        if (hasFifo)
            g_renderstate.throttled_PresentMode = WGPUPresentMode_Fifo;
        else if (hasFifoRelaxed)
            g_renderstate.throttled_PresentMode = WGPUPresentMode_FifoRelaxed;

        if (hasMailbox)
            g_renderstate.unthrottled_PresentMode = WGPUPresentMode_Mailbox;
        else if (hasImmediate)
            g_renderstate.unthrottled_PresentMode = WGPUPresentMode_Immediate;
    }

    {
        char formatsString[8192];
        char* insert = formatsString;
        for (uint32_t i = 0; i < capabilities.formatCount; i++) {
            insert += snprintf(insert, 8192, "%s, ", TextureFormatName(capabilities.formats[i]));
        }
        TRACELOG(LOG_INFO, "Supported surface formats: %s", formatsString);
    }

    WGPUTextureFormat selectedFormat = capabilities.formats[0];
    int format_index = 0;
    for (format_index = 0; format_index < capabilities.formatCount; format_index++) {
        if (capabilities.formats[format_index] == WGPUTextureFormat_RGBA16Float) {
            selectedFormat = (capabilities.formats[format_index]);
            goto found;
        }
    }
    for (format_index = 0; format_index < capabilities.formatCount; format_index++) {
        if (capabilities.formats[format_index] ==
            WGPUTextureFormat_BGRA8Unorm /*|| capabilities.formats[format_index] == WGPUTextureFormat_RGBA8Unorm*/) {
            selectedFormat = (capabilities.formats[format_index]);
            goto found;
        }
    }
found:
    g_renderstate.frameBufferFormat = fromWGPUPixelFormat(selectedFormat);
    if (format_index == capabilities.formatCount) {
        TRACELOG(LOG_WARNING, "No RGBA8 / BGRA8 Unorm framebuffer format found, colors might be off");
        g_renderstate.frameBufferFormat = fromWGPUPixelFormat(selectedFormat);
    }

    TRACELOG(LOG_INFO, "Selected surface format %s", TextureFormatName(toWGPUPixelFormat(g_renderstate.frameBufferFormat)));

    // TRACELOG(LOG_INFO, "Selected present mode %s",
    // presentModeSpellingTable.at((WGPUPresentMode)g_renderstate.throttled_PresentMode).c_str());
}

DescribedBuffer *GenBufferEx(const void *data, size_t size, RGBufferUsage usage) {
    DescribedBuffer *ret = callocnew(DescribedBuffer);
    WGPUBufferDescriptor descriptor = {0};
    descriptor.size = size;
    descriptor.mappedAtCreation = false;
    descriptor.usage = usage;
    ret->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor);
    ret->size = size;
    ret->usage = usage;
    if (data != NULL) {
        wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), (WGPUBuffer)ret->buffer, 0, data, size);
    }
    return ret;
}
void *GetInstance() { return g_wgpustate.instance; }
WGPUDevice GetDevice() { return g_wgpustate.device; }
WGPUAdapter GetAdapter() { return g_wgpustate.adapter; }
void ComputePassSetBindGroup(DescribedComputepass *drp, uint32_t group, DescribedBindGroup *bindgroup) {
    wgpuComputePassEncoderSetBindGroup(
        (WGPUComputePassEncoder)drp->cpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, NULL);
}
// WGPUBuffer GetMatrixBuffer(){
//     wgpuQueueWriteBuffer(GetQueue(), g_renderstate.matrixStack[g_renderstate.stackPosition].second, 0,
//     &g_renderstate.matrixStack[g_renderstate.stackPosition].first, sizeof(Matrix)); return
//     g_renderstate.matrixStack[g_renderstate.stackPosition].second;
// }
Texture2DArray LoadTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, PixelFormat format) {
    const WGPUTextureDescriptor tDesc = {
        .usage = WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst,
        .dimension = WGPUTextureDimension_2D,
        .size = {width, height, layerCount},
        .format = toWGPUPixelFormat(format),
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 1,
        .viewFormats = &tDesc.format,
    };
    assert(tDesc.size.width > 0);
    assert(tDesc.size.height > 0);
    const WGPUTextureViewDescriptor vDesc = {
        .format = tDesc.format,
        .dimension = WGPUTextureViewDimension_2DArray,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = layerCount,
        .aspect = WGPUTextureAspect_All,
        .usage = tDesc.usage,
    };

    WGPUTexture id = wgpuDeviceCreateTexture(GetDevice(), &tDesc);
    WGPUTextureView view = wgpuTextureCreateView(id, &vDesc);

    Texture2DArray ret = {.id = id, .view = view, .layerCount = layerCount, .format = format, .sampleCount = 1};
    return ret;
}
Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, RGTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps) {
    WGPUTextureDescriptor tDesc = {
        .usage = usage,
        .dimension = WGPUTextureDimension_2D,
        .size = {width, height, 1u},
        .format = toWGPUPixelFormat(format),
        .mipLevelCount = mipmaps,
        .sampleCount = sampleCount,
        .viewFormatCount = 1,
        .viewFormats = &tDesc.format,
    };
    assert(tDesc.size.width > 0);
    assert(tDesc.size.height > 0);

    WGPUTextureViewDescriptor textureViewDesc = {0};
    char potlabel[128];
    if (format == PIXELFORMAT_DEPTH_24_PLUS) {
        int len = snprintf(potlabel, 128, "Depftex %d x %d", width, height);
        textureViewDesc.label.data = potlabel;
        textureViewDesc.label.length = len;
    }
    textureViewDesc.usage = usage;
    textureViewDesc.aspect =
        ((format == PIXELFORMAT_DEPTH_24_PLUS || format == PIXELFORMAT_DEPTH_32_FLOAT) ? WGPUTextureAspect_DepthOnly
                                                                                       : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = mipmaps;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    Texture ret zeroinit;
    ret.id = wgpuDeviceCreateTexture(GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.sampleCount = sampleCount;
    ret.mipmaps = mipmaps;
    if (mipmaps > 1) {
        for (uint32_t i = 0; i < mipmaps; i++) {
            textureViewDesc.baseMipLevel = i;
            textureViewDesc.mipLevelCount = 1;
            ret.mipViews[i] = wgpuTextureCreateView(ret.id, &textureViewDesc);
        }
    }
    return ret;
}
DescribedSampler LoadSamplerEx(TextureWrap amode, TextureFilter fmode, TextureFilter mipmapFilter, float maxAnisotropy) {
    DescribedSampler ret = {
        .magFilter = fmode,
        .minFilter = fmode,
        .mipmapFilter = fmode,
        .compare = RGCompareFunction_Undefined,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 10.0f,
        .maxAnisotropy = maxAnisotropy,
        .addressModeU = amode,
        .addressModeV = amode,
        .addressModeW = amode,
    };

    const WGPUSamplerDescriptor sdesc = {
        .magFilter = toWGPUFilterMode(fmode),
        .minFilter = toWGPUFilterMode(fmode),
        .mipmapFilter = toWGPUMipmapFilterMode(fmode),
        .compare = WGPUCompareFunction_Undefined,
        .lodMinClamp = 0.0f,
        .lodMaxClamp = 10.0f,
        .maxAnisotropy = (uint16_t)maxAnisotropy,
        .addressModeU = toWGPUAddressMode(amode),
        .addressModeV = toWGPUAddressMode(amode),
        .addressModeW = toWGPUAddressMode(amode),
    };

    ret.sampler = wgpuDeviceCreateSampler(GetDevice(), &sdesc);
    return ret;
}
void SetBindgroupUniformBufferData(DescribedBindGroup *bg, uint32_t index, const void *data, size_t size) {
    
    const WGPUBufferDescriptor bufferDesc = {
        .size = size,
        .usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform,
        .mappedAtCreation = false,
    };
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), uniformBuffer, 0, data, size);
    const ResourceDescriptor entry = {
        .binding = index,
        .buffer = uniformBuffer,
        .size = size,
    };

    UpdateBindGroupEntry(bg, index, entry);
    wgpuBufferRelease(uniformBuffer);
    // bg->releaseOnClear |= (1 << index);
}

void SetBindgroupStorageBufferData(DescribedBindGroup *bg, uint32_t index, const void *data, size_t size) {
    const WGPUBufferDescriptor bufferDesc = {
        .size = size,
        .usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage,
        .mappedAtCreation = false,
    };
    WGPUBuffer storageBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer((WGPUQueue)GetQueue(), storageBuffer, 0, data, size);
    const ResourceDescriptor entry = {
        .binding = index,
        .buffer = storageBuffer,
        .size = size,
    };

    UpdateBindGroupEntry(bg, index, entry);

    wgpuBufferRelease(storageBuffer);
}
void UnloadBuffer(DescribedBuffer *buffer) {
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    RL_FREE(buffer);
}
Texture LoadTextureFromImage(Image img) {
    Texture ret zeroinit;
    ret.sampleCount = 1;
    Color *altdata = NULL;
    if (img.format == GRAYSCALE) {
        altdata = (Color *)calloc(((size_t)img.width) * img.height, sizeof(Color));
        for (size_t i = 0; i < ((size_t)img.width) * img.height; i++) {
            uint16_t gscv = ((uint16_t *)img.data)[i];
            ((Color *)altdata)[i].r = gscv & 255;
            ((Color *)altdata)[i].g = gscv & 255;
            ((Color *)altdata)[i].b = gscv & 255;
            ((Color *)altdata)[i].a = gscv >> 8;
        }
    }

    WGPUTextureDescriptor tDesc = {
        .nextInChain = NULL,
        .label = {NULL, 0},
        .usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
        .dimension = WGPUTextureDimension_2D,
        .size = {img.width, img.height, 1},
        .format = img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : toWGPUPixelFormat(img.format),
        .mipLevelCount = 1,
        .sampleCount = 1,
        .viewFormatCount = 1,
        .viewFormats = NULL
    };

    assert(tDesc.size.width > 0);
    assert(tDesc.size.height > 0);

    WGPUTextureFormat resulting_tf = img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : toWGPUPixelFormat(img.format);
    tDesc.viewFormats = (WGPUTextureFormat *)&resulting_tf;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &tDesc);
    
    const WGPUTextureViewDescriptor vdesc = {
        .aspect = WGPUTextureAspect_All,
        .format = tDesc.format,
        .dimension = WGPUTextureViewDimension_2D,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
    };

    const WGPUTexelCopyTextureInfo destination = {
        .texture = (WGPUTexture)ret.id,
        .mipLevel = 0,
        .origin = {0, 0, 0},             // equivalent of the offset argument of Queue::writeBuffer
        .aspect = WGPUTextureAspect_All, // only relevant for depth/Stencil textures
    };

    const WGPUTexelCopyBufferLayout source = {
        .offset = 0,
        .bytesPerRow = 4 * img.width,
        .rowsPerImage = img.height,
    };
    wgpuQueueWriteTexture((WGPUQueue)GetQueue(), &destination, altdata ? altdata : img.data, ((size_t)4) * img.width * img.height, &source, &tDesc.size);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vdesc);
    ret.width = img.width;
    ret.height = img.height;
    if (altdata)
        RL_FREE(altdata);
    TRACELOG(LOG_INFO, "Successfully loaded %u x %u texture from image", (unsigned)img.width, (unsigned)img.height);
    return ret;
}
void ResizeSurface(FullSurface *fsurface, int newWidth, int newHeight) {
    fsurface->renderTarget.colorMultisample.width = newWidth;
    fsurface->renderTarget.colorMultisample.height = newHeight;
    fsurface->renderTarget.texture.width = newWidth;
    fsurface->renderTarget.texture.height = newHeight;
    fsurface->renderTarget.depth.width = newWidth;
    fsurface->renderTarget.depth.height = newHeight;
    WGPUTextureFormat format = toWGPUPixelFormat(fsurface->format);
    
    const WGPUSurfaceConfiguration wsconfig = {
        .device = GetDevice(),
        .format = format,
        .usage = RGTextureUsage_CopySrc | RGTextureUsage_RenderAttachment,
        .width = (uint32_t)newWidth,
        .height = (uint32_t)newHeight,
        .viewFormatCount = 1,
        .viewFormats = &format,
        .alphaMode = WGPUCompositeAlphaMode_Opaque,
        .presentMode = RG_to_WGPU_PresentMode(fsurface->presentMode),
    };

    wgpuSurfaceConfigure(fsurface->surface, &wsconfig);
    fsurface->width = newWidth;
    fsurface->height = newHeight;
    UnloadTexture(fsurface->renderTarget.colorMultisample);
    UnloadTexture(fsurface->renderTarget.depth);
    if (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) {
        fsurface->renderTarget.colorMultisample = LoadTexturePro(newWidth, newHeight, fsurface->format, RGTextureUsage_RenderAttachment | RGTextureUsage_CopySrc, 4, 1);
    }
    fsurface->renderTarget.depth = LoadTexturePro(newWidth, newHeight, PIXELFORMAT_DEPTH_32_FLOAT, RGTextureUsage_RenderAttachment | RGTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1);
}

DescribedRenderpass LoadRenderpassEx(RenderSettings settings, bool colorClear, RGColor colorClearValue, bool depthClear, float depthClearValue) {
    DescribedRenderpass ret = {
        .settings = settings,
        .colorClear = colorClearValue,
        .depthClear = depthClearValue,
        .colorLoadOp = colorClear ? RGLoadOp_Clear : RGLoadOp_Load,
        .colorStoreOp = RGStoreOp_Store,
        .depthLoadOp = depthClear ? RGLoadOp_Clear : RGLoadOp_Load,
        .depthStoreOp = RGStoreOp_Store,
    };
    return ret;
}
void BeginRenderpassEx(DescribedRenderpass *renderPass) {
    WGPUCommandEncoderDescriptor desc = {
        .label = STRVIEW("another cmdencoder")
    };

    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &desc);

    WGPURenderPassDescriptor renderPassDesc zeroinit;
    renderPassDesc.colorAttachmentCount = 1;

    WGPURenderPassColorAttachment colorAttachment zeroinit;
    WGPURenderPassDepthStencilAttachment depthAttachment zeroinit;
    if (RenderTexture_stack_cpeek(&g_renderstate.renderTargetStack)->colorMultisample.view) {
        colorAttachment.view = (WGPUTextureView)RenderTexture_stack_cpeek(&g_renderstate.renderTargetStack)->colorMultisample.view;
        colorAttachment.resolveTarget = (WGPUTextureView)RenderTexture_stack_cpeek(&g_renderstate.renderTargetStack)->texture.view;
    } else {
        colorAttachment.view = (WGPUTextureView)RenderTexture_stack_cpeek(&g_renderstate.renderTargetStack)->texture.view;
        colorAttachment.resolveTarget = NULL;
    }

    colorAttachment.loadOp = RG_to_WGPU_LoadOp(renderPass->colorLoadOp);
    colorAttachment.storeOp = RG_to_WGPU_StoreOp(renderPass->colorStoreOp);
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.clearValue = CLITERAL(WGPUColor){
        renderPass->colorClear.r,
        renderPass->colorClear.g,
        renderPass->colorClear.b,
        renderPass->colorClear.a,
    };

    depthAttachment.view = (WGPUTextureView)RenderTexture_stack_cpeek(&g_renderstate.renderTargetStack)->depth.view;
    depthAttachment.depthLoadOp = RG_to_WGPU_LoadOp(renderPass->depthLoadOp);
    depthAttachment.depthStoreOp = RG_to_WGPU_StoreOp(renderPass->depthStoreOp);
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

// WGPUBuffer readtex = NULL;
// volatile bool waitflag = false;

static inline void
onBuffer2Mapped(WGPUMapAsyncStatus status, WGPUStringView message, WGPU_NULLABLE void *userdata1, WGPU_NULLABLE void *userdata2) {
    if (status != WGPUMapAsyncStatus_Success) {
        TRACELOG(LOG_ERROR, "onBuffer2Mapped failed with status: %d", status);
        return;
    }

    Image *udImage = (Image *)userdata1;
    WGPUBuffer bufferToMap = (WGPUBuffer)userdata2;
    uint64_t bufferSize = wgpuBufferGetSize(bufferToMap);
    const void *map = wgpuBufferGetConstMappedRange(bufferToMap, 0, bufferSize);

    udImage->data = RL_CALLOC(bufferSize / 4, 4);
    if (udImage->data) {
        memcpy(udImage->data, map, bufferSize);
    }
    wgpuBufferUnmap(bufferToMap);
    wgpuBufferRelease(bufferToMap);
};

Image LoadImageFromTextureEx(WGPUTexture tex, uint32_t miplevel) {
    WGPUTextureFormat wFormat = wgpuTextureGetFormat(tex);
    size_t formatSize = GetPixelSizeInBytes(fromWGPUPixelFormat(wFormat));
    uint32_t width = wgpuTextureGetWidth(tex);
    uint32_t height = wgpuTextureGetHeight(tex);
    const size_t rowStrideInBytes = RoundUpToNextMultipleOf256(width) * formatSize;

    Image ret = {
        .data = NULL,
        .width = (uint32_t)wgpuTextureGetWidth(tex),
        .height = (uint32_t)wgpuTextureGetHeight(tex),
        .mipmaps = 1,
        .format = fromWGPUPixelFormat(wFormat),
        .rowStrideInBytes = rowStrideInBytes,
    };

    WGPUBufferDescriptor b = {
        .usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst,
        .size = rowStrideInBytes * height,
        .mappedAtCreation = false,
    };
    // Create a LOCAL buffer.
    WGPUBuffer localReadTex = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &b);

    WGPUCommandEncoderDescriptor commandEncoderDesc = {0};
    commandEncoderDesc.label = STRVIEW("Command Encoder for Texture Readback");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), NULL);

    const WGPUTexelCopyTextureInfo tbsource = {
        .texture = tex,
        .mipLevel = miplevel,
        .origin = {0, 0, 0},
        .aspect = WGPUTextureAspect_All,
    };

    const WGPUTexelCopyBufferInfo tbdest = {
        .layout =
            {
                .offset = 0,
                .bytesPerRow = (uint32_t)rowStrideInBytes,
                .rowsPerImage = height,
            },
        .buffer = localReadTex,
    };

    WGPUExtent3D copysize = {width / (1u << miplevel), height / (1u << miplevel), 1};
    wgpuCommandEncoderClearBuffer(encoder, localReadTex, 0, b.size);
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &copysize);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {0};
    cmdBufferDescriptor.label = STRVIEW("Command buffer for Texture Readback");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, NULL);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(command);

    WGPUBufferMapCallbackInfo mapCallbackInfo = {
        .mode = WGPUCallbackMode_WaitAnyOnly,
        .callback = onBuffer2Mapped,
        .userdata1 = &ret,
        .userdata2 = localReadTex,
    };

    WGPUFuture future = wgpuBufferMapAsync(localReadTex, WGPUMapMode_Read, 0, b.size, mapCallbackInfo);
    WGPUFutureWaitInfo fwinfo = {.future = future};

    wgpuInstanceWaitAny((WGPUInstance)GetInstance(), 1, &fwinfo, UINT64_MAX);

    return ret;
}

void BufferData(DescribedBuffer *buffer, const void *data, size_t size) {
    if (buffer->size >= size) {
        wgpuQueueWriteBuffer(GetQueue(), buffer->buffer, 0, data, size);
    } else {
        if (buffer->buffer)
            wgpuBufferRelease(buffer->buffer);
        WGPUBufferDescriptor nbdesc = {0};
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgpuDeviceCreateBuffer(GetDevice(), &nbdesc);
        buffer->size = size;
        wgpuQueueWriteBuffer(GetQueue(), buffer->buffer, 0, data, size);
    }
}
void ResetSyncState() {}

void
RenderPassSetIndexBuffer(DescribedRenderpass *drp, DescribedBuffer *buffer, IndexFormat format, uint64_t offset) {
    wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)drp->rpEncoder, (WGPUBuffer)buffer->buffer, RG_to_WGPU_IndexFormat(format), offset, buffer->size);
}
void RenderPassSetVertexBuffer(DescribedRenderpass *drp, uint32_t slot, DescribedBuffer *buffer, uint64_t offset) {
    wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, buffer->size);
}
void RenderPassSetBindGroup(DescribedRenderpass *drp, uint32_t group, DescribedBindGroup *bindgroup) {
    wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)drp->rpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, NULL);
}
void RenderPassDraw(DescribedRenderpass *drp, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
void RenderPassDrawIndexed(DescribedRenderpass *drp, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance) {
    wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}
void EndRenderpassEx(DescribedRenderpass *renderPass) {
    drawCurrentBatch();
    wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)renderPass->rpEncoder);
    g_renderstate.activeRenderpass = NULL;
    WGPURenderPassEncoder re = renderPass->rpEncoder;
    renderPass->rpEncoder = 0;
    WGPUCommandBufferDescriptor cmdBufferDescriptor  = {0};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)renderPass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit((WGPUQueue)GetQueue(), 1, &command);
    wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)re);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)renderPass->cmdEncoder);
    wgpuCommandBufferRelease(command);
}
void EndRenderpassPro(DescribedRenderpass *rp, bool renderTexture) { EndRenderpassEx(rp); }

RenderTexture LoadRenderTexture(uint32_t width, uint32_t height) {
    RenderTexture ret = {
        .texture = LoadTextureEx(width, height, g_renderstate.frameBufferFormat, true),
            .colorMultisample = {0},
            .depth = LoadTexturePro(width, height, PIXELFORMAT_DEPTH_32_FLOAT, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1)
    };
    if (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) {
        ret.colorMultisample = LoadTexturePro(
            width, height, g_renderstate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4, 1);
    }
    ret.colorAttachmentCount = 1;
    return ret;
}

DescribedShaderModule LoadShaderModuleSPIRV(ShaderSources sourcesSpirv) {
    DescribedShaderModule ret zeroinit;
#ifndef __EMSCRIPTEN__
    for (uint32_t i = 0; i < sourcesSpirv.sourceCount; i++) {

        rassert(sourcesSpirv.sources[i].sizeInBytes / sizeof(uint32_t) < UINT32_MAX,
                "SPIRV too long: %llu bytes",
                (unsigned long long)sourcesSpirv.sources[i].sizeInBytes);
        WGPUShaderSourceSPIRV shaderCodeDesc = {
            .chain = {.sType = WGPUSType_ShaderSourceSPIRV},
            .codeSize = (uint32_t)(sourcesSpirv.sources[i].sizeInBytes / sizeof(uint32_t)),
            .code = (const uint32_t *)sourcesSpirv.sources[i].data,
        };
        rassert(*shaderCodeDesc.code == 0x07230203, "Invalid SPIRV magic");
        WGPUShaderModuleDescriptor shaderDesc = {.nextInChain = &shaderCodeDesc.chain};

        WGPUShaderModule sh = wgpuDeviceCreateShaderModule((WGPUDevice)GetDevice(), &shaderDesc);
        SpvReflectShaderModule spv_mod;
        SpvReflectResult moduleCreateResult = spvReflectCreateShaderModule(sourcesSpirv.sources[i].sizeInBytes, (const uint32_t *)sourcesSpirv.sources[i].data, &spv_mod);
        if(moduleCreateResult == SPV_REFLECT_RESULT_SUCCESS){
            const uint32_t entryPointCount = spv_mod.entry_point_count;
            for (uint32_t i = 0; i < entryPointCount; i++) {
                SpvReflectShaderStageFlagBits epStage = spv_mod.entry_points[i].shader_stage;
                RGShaderStageEnum stage;
                switch (epStage) {
                    case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                        stage = RGShaderStageEnum_Vertex;
                        break;
                    case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                        stage = RGShaderStageEnum_Fragment;
                        break;
                    case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
                        stage = RGShaderStageEnum_Compute;
                        break;
                    case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                        stage =  RGShaderStageEnum_Vertex;
                        break;
                    default:
                        TRACELOG(LOG_FATAL, "Unknown shader stage: %d", (int)epStage);
                        stage = RGShaderStageEnum_Vertex;
                        break;
                }
            
                ret.stages[stage].module = sh;
                ret.reflectionInfo.ep[stage].stage = stage;
                memset(ret.reflectionInfo.ep[stage].name, 0, sizeof(ret.reflectionInfo.ep[stage].name));
                uint32_t eplength = strlen(spv_mod.entry_points[i].name);
                rassert(eplength < 15, "Entry point name must be < 15 chars");
                memcpy(ret.reflectionInfo.ep[stage].name, spv_mod.entry_points[i].name, eplength);
            }
    }
    }
#endif
    return ret;
}

void UnloadShaderModule(DescribedShaderModule mod) {
    WGPUShaderModule freed[RGShaderStageEnum_EnumCount + 1];
    int freedCount = 0;

    for (size_t i = 0; i < RGShaderStageEnum_EnumCount; i++) {
        WGPUShaderModule m = (WGPUShaderModule)mod.stages[i].module;
        if (!m){
            continue;
        }

        int alreadyFreed = 0;
        for (int j = 0; j < freedCount; j++) {
            if (freed[j] == m) {
                alreadyFreed = 1;
                break;
            }
        }

        if (!alreadyFreed) {
            freed[freedCount++] = m;
            wgpuShaderModuleRelease(m);
        }
    }
}

WGPURenderPipeline createSingleRenderPipe(const ModifiablePipelineState* mst,
                                          const DescribedShaderModule* shaderModule,
                                          const DescribedBindGroupLayout* bglayout,
                                          const DescribedPipelineLayout* pllayout) {
    WGPURenderPipelineDescriptor pipelineDesc zeroinit;

    const RenderSettings* settings = &mst->settings;
    pipelineDesc.multisample.count = settings->sampleCount ? settings->sampleCount : 1;
    pipelineDesc.multisample.mask = 0xFFFFFFFF;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;
    pipelineDesc.layout = (WGPUPipelineLayout)pllayout->layout;

    WGPUVertexState vertexState zeroinit;
    WGPUFragmentState fragmentState zeroinit;
    WGPUBlendState blendState zeroinit;

    vertexState.module = (WGPUShaderModule)shaderModule->stages[RGShaderStageEnum_Vertex].module;

    VertexBufferLayoutSet vlayout_complete = getBufferLayoutRepresentation(mst->vertexAttributes, mst->vertexAttributeCount);

    vertexState.bufferCount = vlayout_complete.number_of_buffers;

    WGPUVertexBufferLayout layouts_converted[16] = {0};
    size_t insertIndex = 0;
    WGPUVertexAttribute** wgpuAttributeHeap = (WGPUVertexAttribute**)VLAStack_alloc(&g_vlastack, vlayout_complete.number_of_buffers * sizeof(WGPUVertexAttribute*));
    for (uint32_t i = 0; i < vlayout_complete.number_of_buffers; i++) {
        wgpuAttributeHeap[i] = (WGPUVertexAttribute*)VLAStack_alloc(&g_vlastack, vlayout_complete.layouts[i].attributeCount * sizeof(WGPUVertexAttribute));
        for(uint32_t j = 0;j < vlayout_complete.layouts[i].attributeCount;j++){
            const RGVertexAttribute* attrJ = vlayout_complete.layouts[i].attributes + j;
            wgpuAttributeHeap[i][j] = (WGPUVertexAttribute){
                .format = RG_to_WGPU_VertexFormat(attrJ->format),
                .offset = attrJ->offset,
                .shaderLocation = attrJ->shaderLocation,
            };
        }
        layouts_converted[insertIndex++] = (WGPUVertexBufferLayout){
            .nextInChain = NULL,
            .stepMode = RG_to_WGPU_VertexStepMode(vlayout_complete.layouts[i].stepMode),
            .arrayStride = vlayout_complete.layouts[i].arrayStride,
            .attributeCount = vlayout_complete.layouts[i].attributeCount,
            .attributes = wgpuAttributeHeap[i],
        };
        
    }
    vertexState.buffers = layouts_converted;
    vertexState.constantCount = 0;
    vertexState.entryPoint = CLITERAL(WGPUStringView){shaderModule->reflectionInfo.ep[WGPUShaderStageEnum_Vertex].name, strlen(shaderModule->reflectionInfo.ep[WGPUShaderStageEnum_Vertex].name)};
    pipelineDesc.vertex = vertexState;

    fragmentState.module = shaderModule->stages[WGPUShaderStageEnum_Fragment].module;
    fragmentState.entryPoint = CLITERAL(WGPUStringView){shaderModule->reflectionInfo.ep[WGPUShaderStageEnum_Fragment].name, strlen(shaderModule->reflectionInfo.ep[WGPUShaderStageEnum_Fragment].name)};
    fragmentState.constantCount = 0;
    fragmentState.constants = NULL;

    blendState.color.srcFactor = RG_to_WGPU_BlendFactor   (settings->blendState.color.srcFactor);
    blendState.color.dstFactor = RG_to_WGPU_BlendFactor   (settings->blendState.color.dstFactor);
    blendState.color.operation = RG_to_WGPU_BlendOperation(settings->blendState.color.operation);
    blendState.alpha.srcFactor = RG_to_WGPU_BlendFactor   (settings->blendState.alpha.srcFactor);
    blendState.alpha.dstFactor = RG_to_WGPU_BlendFactor   (settings->blendState.alpha.dstFactor);
    blendState.alpha.operation = RG_to_WGPU_BlendOperation(settings->blendState.alpha.operation);

    const WGPUColorTargetState colorTarget = {
        .format = toWGPUPixelFormat(g_renderstate.frameBufferFormat),
        .blend = &blendState,
        .writeMask = WGPUColorWriteMask_All,
    };

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    // Set up depthStencilState based on settings->depthTest flags, there's no stencil test bool
    WGPUDepthStencilState depthStencilState = {0};
    if (settings->depthTest) {
        // Keep a fragment only if its depth is lower than the previously blended one
        // Each time a fragment is blended into the target, we update the value of the Z-buffer
        // Store the format in a variable as later parts of the code depend on it
        // Deactivate the stencil alltogether
        WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth32Float;

        depthStencilState.depthCompare = RG_to_WGPU_CompareFunction(settings->depthCompare);
        depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
        depthStencilState.format = depthTextureFormat;
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;
        depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
        depthStencilState.stencilBack.compare = WGPUCompareFunction_Always;
    }
    pipelineDesc.depthStencil = settings->depthTest ? &depthStencilState : NULL;

    pipelineDesc.primitive.frontFace = RG_to_WGPU_FrontFace(settings->frontFace);
    pipelineDesc.primitive.cullMode = settings->faceCull ? WGPUCullMode_Back : WGPUCullMode_None;
    //pipelineDesc.primitive.cullMode = WGPUCullMode_None;
    switch (mst->primitiveType) {
        case RL_LINES:
            pipelineDesc.primitive.topology = WGPUPrimitiveTopology_LineList;break;
        case RL_TRIANGLES:
            pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;break;
        case RL_TRIANGLE_STRIP:
            pipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleStrip;break;
        case RL_POINTS:
            pipelineDesc.primitive.topology = WGPUPrimitiveTopology_PointList;break;
        case RL_QUADS:
        default:
            rg_unreachable();
    }
    WGPURenderPipeline ret = wgpuDeviceCreateRenderPipeline((WGPUDevice)GetDevice(), &pipelineDesc);

    for(uint32_t i = vlayout_complete.number_of_buffers;i > 0;i--){
        VLAStack_free(&g_vlastack, wgpuAttributeHeap[i - 1]);
    }
    VLAStack_free(&g_vlastack, (void*)wgpuAttributeHeap);
    return ret;
}

RGAPI Shader LoadPipelineEx(const char *shaderSource, const AttributeAndResidence *attribs, uint32_t attribCount, const ResourceTypeDescriptor *uniforms, uint32_t uniformCount, RenderSettings settings) {
    ShaderSources sources = dualStage(shaderSource, sourceTypeWGSL, RGShaderStageEnum_Vertex, RGShaderStageEnum_Fragment);
    DescribedShaderModule mod = LoadShaderModule(sources);
    return LoadPipelineFromModule(mod, attribs, attribCount, uniforms, uniformCount, settings);
}
RGAPI Shader LoadPipeline(const char *shaderSource) {
    ShaderSources sources = dualStage(shaderSource, sourceTypeWGSL, RGShaderStageEnum_Vertex, RGShaderStageEnum_Fragment);
    InOutAttributeInfo attribs = getAttributesWGSL(sources);
    AttributeAndResidence allAttribsInOneBuffer[MAX_VERTEX_ATTRIBUTES];
    const uint32_t attributeCount = attribs.vertexAttributeCount;
    uint32_t offset = 0;
    for (uint32_t attribIndex = 0; attribIndex < attribs.vertexAttributeCount; attribIndex++) {
        const RGVertexFormat format = attribs.vertexAttributes[attribIndex].format;
        const uint32_t location = attribs.vertexAttributes[attribIndex].location;
        allAttribsInOneBuffer[attribIndex] = CLITERAL(AttributeAndResidence){
            .attr = {
                .format = format,
                .offset = offset,
                .shaderLocation = location
            },
            .bufferSlot = 0,
            .stepMode = RGVertexStepMode_Vertex,
            .enabled = true
        };
        offset += attributeSize(format);
    }

    StringToUniformMap *bindings = getBindingsWGSL(sources);
    ResourceTypeDescriptor *values = (ResourceTypeDescriptor *)RL_CALLOC(bindings->current_size, sizeof(ResourceTypeDescriptor));
    uint32_t insertIndex = 0;
    for (uint32_t i = 0; i < bindings->current_capacity; i++) {
        if (bindings->table[i].key.length != 0) {
            values[insertIndex++] = bindings->table[i].value;
        }
    }

    quickSort_ResourceTypeDescriptor(values, values + bindings->current_size);

    Shader ret = LoadPipelineEx(shaderSource, allAttribsInOneBuffer, attributeCount, values, bindings->current_size, GetDefaultSettings());
    RL_FREE(values);
    StringToUniformMap_free(bindings);
    RL_FREE(bindings);
    return ret;
}

RGAPI Shader LoadPipelineFromModule(DescribedShaderModule mod, const AttributeAndResidence *attribs, uint32_t attribCount, const ResourceTypeDescriptor *uniforms, uint32_t uniformCount, RenderSettings settings) {
    Shader retS = {.id = getNextShaderID_shc()};
    ShaderImpl *ret = GetShaderImpl(retS);
    ret->state.settings = settings;
    ret->state.vertexAttributes = (AttributeAndResidence *)attribs;
    ret->state.vertexAttributeCount = attribCount;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, false);
    ret->shaderModule = mod;
    ret->state.colorAttachmentState.colorAttachmentCount = mod.reflectionInfo.attributes.attachmentCount;

    for(uint32_t i = 0; i < MAX_COLOR_ATTACHMENTS;i++) ret->state.colorAttachmentState.attachmentFormats[i] = PIXELFORMAT_UNCOMPRESSED_B8G8R8A8;

    const WGPUBindGroupLayout bgls[1] = {ret->bglayout.layout};
    const WGPUPipelineLayoutDescriptor pldesc = {
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = bgls
    };

    ret->layout.layout = wgpuDeviceCreatePipelineLayout(GetDevice(), &pldesc);

    ResourceDescriptor* bge = (ResourceDescriptor*)RL_CALLOC(uniformCount, sizeof(ResourceDescriptor));
    for (uint32_t i = 0; i < uniformCount; i++) {
        bge[i] = CLITERAL(ResourceDescriptor){.binding = uniforms[i].location};
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge, uniformCount);
    RL_FREE((void*)bge);
    return retS;
}

WGPUBuffer cloneBuffer(WGPUBuffer b, WGPUBufferUsage usage) {
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), NULL);
    const WGPUBufferDescriptor retd = {
        .usage = usage,
        .size = wgpuBufferGetSize(b),
    };
    WGPUBuffer ret = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &retd);
    wgpuCommandEncoderCopyBufferToBuffer(enc, b, 0, ret, 0, retd.size);
    WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(enc, NULL);
    wgpuQueueSubmit(GetQueue(), 1, &buffer);
    return ret;
}

DescribedComputePipeline *LoadComputePipeline(const char *shaderCode) {
    ShaderSources sources = singleStage(shaderCode, detectShaderLanguageSingle(shaderCode, strlen(shaderCode)), RGShaderStageEnum_Compute);

    StringToUniformMap *bindings = getBindings(sources);

    ResourceTypeDescriptor *udesc = (ResourceTypeDescriptor *)RL_CALLOC(bindings->current_size, sizeof(ResourceTypeDescriptor));
    uint32_t insertIndex = 0;
    for (uint32_t i = 0; i < bindings->current_capacity; i++) {
        StringToUniformMap_kv_pair *entry = bindings->table + i;
        if (bindings->table[i].key.length == 0) {
            continue;
        }
        udesc[insertIndex++] = entry->value;
    }

    quickSort_ResourceTypeDescriptor(udesc, udesc + insertIndex);

    DescribedComputePipeline *ret = LoadComputePipelineEx(shaderCode, udesc, insertIndex);
    RL_FREE(udesc);
    StringToUniformMap_free(bindings);
    RL_FREE(bindings);
    return ret;
}

RGAPI DescribedComputePipeline *
LoadComputePipelineEx(const char *shaderCode, const ResourceTypeDescriptor *uniforms, uint32_t uniformCount) {
    ShaderSources sources = singleStage(shaderCode, detectShaderLanguageSingle(shaderCode, strlen(shaderCode)), RGShaderStageEnum_Compute);

    StringToUniformMap* bindmap = getBindings(sources);
    DescribedComputePipeline *ret = callocnew(DescribedComputePipeline);
    WGPUComputePipelineDescriptor desc  = {0};
    WGPUPipelineLayoutDescriptor pldesc = {0};
    pldesc.bindGroupLayoutCount = 1;
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, true);
    pldesc.bindGroupLayoutCount = 1;
    pldesc.bindGroupLayouts = (WGPUBindGroupLayout *)&ret->bglayout.layout;
    WGPUPipelineLayout playout = wgpuDeviceCreatePipelineLayout((WGPUDevice)GetDevice(), &pldesc);
    ret->shaderModule = LoadShaderModule(sources);
    desc.compute.module = (WGPUShaderModule)ret->shaderModule.stages[WGPUShaderStageEnum_Compute].module;

    desc.compute.entryPoint = CLITERAL(WGPUStringView){ret->shaderModule.reflectionInfo.ep[WGPUShaderStageEnum_Compute].name, strlen(ret->shaderModule.reflectionInfo.ep[WGPUShaderStageEnum_Compute].name)};
    desc.layout = playout;
    WGPUDevice device = (WGPUDevice)GetDevice();
    ret->pipeline = wgpuDeviceCreateComputePipeline((WGPUDevice)GetDevice(), &desc);
    ResourceDescriptor* bge = (ResourceDescriptor*)RL_CALLOC(uniformCount, sizeof(ResourceDescriptor));
    for (uint32_t i = 0; i < uniformCount; i++) {
        bge[i] = CLITERAL(ResourceDescriptor){0};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge, uniformCount);
    RL_FREE((void*)bge);
    return ret;
}

Shader LoadShaderFromMemoryOld(const char *vertexSource, const char *fragmentSource) {
    Shader shader zeroinit;

#if SUPPORT_GLSL_PARSER == 1

    // shader.id = LoadPipelineGLSL(vertexSource, fragmentSource);
    shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));
    for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) {
        shader.locs[i] = LOCATION_NOT_FOUND;
    }
    // shader.locs[SHADER_LOC_VERTEX_POSITION] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    // shader.locs[SHADER_LOC_VERTEX_TEXCOORD01] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    // shader.locs[SHADER_LOC_VERTEX_TEXCOORD02] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    // shader.locs[SHADER_LOC_VERTEX_NORMAL] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    // shader.locs[SHADER_LOC_VERTEX_TANGENT] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    // shader.locs[SHADER_LOC_VERTEX_COLOR] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    // shader.locs[SHADER_LOC_VERTEX_BONEIDS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS);
    // shader.locs[SHADER_LOC_VERTEX_BONEWEIGHTS] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS);
    // shader.locs[SHADER_LOC_VERTEX_INSTANCE_TX] = rlGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX);

    // Get handles to GLSL uniform locations (vertex shader)

    shader.locs[SHADER_LOC_MATRIX_MVP] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
    shader.locs[SHADER_LOC_MATRIX_VIEW] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
    shader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
    shader.locs[SHADER_LOC_MATRIX_NORMAL] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);
    shader.locs[SHADER_LOC_BONE_MATRICES] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES);

    // Get handles to GLSL uniform locations (fragment shader)
    shader.locs[SHADER_LOC_COLOR_DIFFUSE] = GetUniformLocation(shader, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
    shader.locs[SHADER_LOC_MAP_DIFFUSE] =
        GetUniformLocation(shader, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0); // SHADER_LOC_MAP_ALBEDO
    shader.locs[SHADER_LOC_MAP_SPECULAR] =
        GetUniformLocation(shader, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // SHADER_LOC_MAP_METALNESS
    shader.locs[SHADER_LOC_MAP_NORMAL] = GetUniformLocation(shader, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
#else
    TRACELOG(LOG_ERROR, "GLSL Shader requested but compiled without GLSL support");
    TRACELOG(LOG_ERROR, "Configure CMake with -DSUPPORT_GLSL_PARSER=ON");
#endif
    return shader;
}

void UnloadPipeline(DescribedPipeline *pl) {

    // MASSIVE TODO

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


#ifndef __EMSCRIPTEN__
static int spv_is_builtin(const SpvReflectInterfaceVariable *v) {
    return (v->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) != 0;
}

static void spv_fill_vertex_inputs(InOutAttributeInfo *out, SpvReflectShaderModule *mod) {
    uint32_t n = 0;
    if (spvReflectEnumerateInputVariables(mod, &n, NULL) != SPV_REFLECT_RESULT_SUCCESS || n == 0)
        return;
    SpvReflectInterfaceVariable **vars = (SpvReflectInterfaceVariable **)malloc(n * sizeof(*vars));
    if (!vars)
        return;
    if (spvReflectEnumerateInputVariables(mod, &n, vars) != SPV_REFLECT_RESULT_SUCCESS) {
        free((void*)vars);
        return;
    }

    uint32_t i;
    for (i = 0; i < n && out->vertexAttributeCount < MAX_VERTEX_ATTRIBUTES; ++i) {
        const SpvReflectInterfaceVariable *v = vars[i];
        if (spv_is_builtin(v))
            continue;

        ReflectionVertexAttribute *a = &out->vertexAttributes[out->vertexAttributeCount++];
        a->location = v->location;

        /* Map numeric shape to a WGPUVertexFormat.
           Prefer float formats. No Float16x3 in WebGPU, so upcast to x4. */
        const uint32_t comps = v->numeric.vector.component_count ? v->numeric.vector.component_count : 1;
        const uint32_t width = v->numeric.scalar.width;

        if (width == 16) {
            a->format = (comps == 1)   ? RGVertexFormat_Float16
                        : (comps == 2) ? RGVertexFormat_Float16x2
                                       : RGVertexFormat_Float16x4;
        } else {
            a->format = (comps == 1)   ? RGVertexFormat_Float32
                        : (comps == 2) ? RGVertexFormat_Float32x2
                        : (comps == 3) ? RGVertexFormat_Float32x3
                                       : RGVertexFormat_Float32x4;
        }
    }
    free((void*)vars);
}

static void spv_fill_fragment_outputs(InOutAttributeInfo *out, SpvReflectShaderModule *mod) {
    uint32_t n = 0;
    if (spvReflectEnumerateOutputVariables(mod, &n, NULL) != SPV_REFLECT_RESULT_SUCCESS || n == 0)
        return;
    SpvReflectInterfaceVariable **vars = (SpvReflectInterfaceVariable **)malloc(n * sizeof(*vars));
    if (!vars)
        return;
    if (spvReflectEnumerateOutputVariables(mod, &n, vars) != SPV_REFLECT_RESULT_SUCCESS) {
        free((void*)vars);
        return;
    }

    uint32_t i;
    for (i = 0; i < n && out->attachmentCount < MAX_COLOR_ATTACHMENTS; ++i) {
        const SpvReflectInterfaceVariable *v = vars[i];
        if (spv_is_builtin(v))
            continue;
        ReflectionFragmentOutput *o = &out->attachments[out->attachmentCount++];
        switch (v->format) {
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
            o->number_of_components = 1;
            o->type = sample_f32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            o->number_of_components = 2;
            o->type = sample_f32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            o->number_of_components = 3;
            o->type = sample_f32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            o->number_of_components = 4;
            o->type = sample_f32;
            break;
        case SPV_REFLECT_FORMAT_R32_UINT:
            o->number_of_components = 1;
            o->type = sample_u32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_UINT:
            o->number_of_components = 2;
            o->type = sample_u32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:
            o->number_of_components = 3;
            o->type = sample_u32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
            o->number_of_components = 4;
            o->type = sample_u32;
            break;
        case SPV_REFLECT_FORMAT_R32_SINT:
            o->number_of_components = 1;
            o->type = sample_i32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SINT:
            o->number_of_components = 2;
            o->type = sample_i32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:
            o->number_of_components = 3;
            o->type = sample_i32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
            o->number_of_components = 4;
            o->type = sample_i32;
            break;
        default:
            break;
        }
    }
    free((void *)vars);
}
#endif /* __EMSCRIPTEN__ */

// ------------------------------
// Implementations requested
// ------------------------------

InOutAttributeInfo getAttributesSPIRV(ShaderSources sources) {
    InOutAttributeInfo info;
    memset(&info, 0, sizeof(info));

#ifndef __EMSCRIPTEN__
    /* Walk all provided SPIR-V stages. */
    uint32_t i;
    for (i = 0; i < sources.sourceCount; ++i) {
        const ShaderStageSource *s = &sources.sources[i];
        if (!s->data || s->sizeInBytes < 4)
            continue;

        /* Require SPIR-V magic. */
        if (*(const uint32_t *)s->data != 0x07230203u)
            continue;

        SpvReflectShaderModule mod;
        if (spvReflectCreateShaderModule((size_t)s->sizeInBytes, s->data, &mod) != SPV_REFLECT_RESULT_SUCCESS) {
            continue;
        }

        if (mod.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
            spv_fill_vertex_inputs(&info, &mod);
        } else if (mod.shader_stage == SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT) {
            spv_fill_fragment_outputs(&info, &mod);
        }

        spvReflectDestroyShaderModule(&mod);
    }
#endif
    return info;
}

static inline WGPUShaderStage spv_stage_to_wgpu_mask(SpvReflectShaderStageFlagBits s) {
    switch (s) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:return WGPUShaderStage_Vertex;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:return WGPUShaderStage_Fragment;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:return WGPUShaderStage_Compute;
        #if SUPPORT_VULKAN_BACKEND == 1
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:return WGPUShaderStage_Geometry;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:return WGPUShaderStage_TessControl;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:return WGPUShaderStage_TessEvaluation;
        case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:return WGPUShaderStage_RayGen;
        case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR:return WGPUShaderStage_AnyHit;
        case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return WGPUShaderStage_ClosestHit;
        case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:return WGPUShaderStage_Miss;
        case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR:return WGPUShaderStage_Intersect;
        case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR:return WGPUShaderStage_Callable;
        case SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT:return WGPUShaderStage_Task;
        case SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT:return WGPUShaderStage_Mesh;
        #endif
        default:return (WGPUShaderStage)0;
    }
}

static inline uniform_type uniform_type_from_binding(const SpvReflectDescriptorBinding *b) {
    switch (b->descriptor_type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return uniform_buffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return storage_buffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
        return texture_sampler; // sampler only
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        // Single descriptor holds both. Pick texture2d/3d based on dim; app must bind compatibly.
        switch (b->image.dim) {
        case SpvDim3D:
            return texture3d;
        default:
            return b->image.arrayed ? texture2d_array : texture2d;
        }
    case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        switch (b->image.dim) {
        case SpvDim3D:
            return texture3d;
        default:
            return b->image.arrayed ? texture2d_array : texture2d;
        }
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        switch (b->image.dim) {
        case SpvDim3D:
            return storage_texture3d;
        default:
            return b->image.arrayed ? storage_texture2d_array : storage_texture2d;
        }
    default:
        return uniform_buffer; // safe default
    }
}

static inline access_type access_from_binding(const SpvReflectDescriptorBinding *b) {
    switch (b->descriptor_type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        return readwrite;
    default:
        return readonly;
    }
}

static inline format_or_sample_type fstype_from_binding(const SpvReflectDescriptorBinding *b) {
    // sampled textures -> sample type
    if (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
        b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        // SPIR-V does not carry signedness in descriptor. Default to float.
        return sample_f32;
    }
    // storage images -> concrete image format
    if (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
        switch (b->image.image_format) {
        case SpvImageFormatR32f:
            return format_r32float;
        case SpvImageFormatR32ui:
            return format_r32uint;
        case SpvImageFormatRgba8:
            return format_rgba8unorm;
        case SpvImageFormatRgba32f:
            return format_rgba32float;
        default:
            return format_r32float; // fallback
        }
    }
    return sample_f32;
}

static inline uint32_t min_size_from_binding(const SpvReflectDescriptorBinding *b) {
    switch (b->descriptor_type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return (b->block.size <= UINT32_MAX) ? (uint32_t)b->block.size : UINT32_MAX;
    default:
        return 0u;
    }
}

static inline void make_binding_key(const SpvReflectDescriptorBinding *b, BindingIdentifier *out_key) {
    memset(out_key, 0, sizeof(*out_key));
    const char *src = (b->name && b->name[0] != '\0') ? b->name : NULL;
    char tmp[64];
    if (!src) {
        /* synthesize a stable name if the compiler stripped it */
        int n = snprintf(tmp, sizeof(tmp), "set%u_binding%u", b->set, b->binding);
        if (n < 0)
            tmp[0] = '\0';
        src = tmp;
    }
    size_t len = strlen(src);
    if (len > MAX_BINDING_NAME_LENGTH)
        len = MAX_BINDING_NAME_LENGTH;
    memcpy(out_key->name, src, len);
    out_key->name[len] = '\0';
    out_key->length = (uint32_t)len;
}

StringToUniformMap *getBindingsSPIRV(ShaderSources sources) {
    StringToUniformMap *map = (StringToUniformMap *)RL_CALLOC(1, sizeof(StringToUniformMap));
    if (!map)
        return NULL;
    StringToUniformMap_init(map);

    for (uint32_t si = 0; si < sources.sourceCount; ++si) {
        const ShaderStageSource *s = &sources.sources[si];
        if (!s->data || s->sizeInBytes < 4)
            continue;
        if (*((const uint32_t *)s->data) != 0x07230203u)
            continue; // SPIR-V magic

        SpvReflectShaderModule mod;
        if (spvReflectCreateShaderModule((size_t)s->sizeInBytes, s->data, &mod) != SPV_REFLECT_RESULT_SUCCESS) {
            continue;
        }

        WGPUShaderStage stage_mask = spv_stage_to_wgpu_mask(mod.shader_stage);

        uint32_t count = 0;
        if (spvReflectEnumerateDescriptorBindings(&mod, &count, NULL) != SPV_REFLECT_RESULT_SUCCESS) {
            spvReflectDestroyShaderModule(&mod);
            continue;
        }
        if (count == 0) {
            spvReflectDestroyShaderModule(&mod);
            continue;
        }

        SpvReflectDescriptorBinding **binds = (SpvReflectDescriptorBinding **)malloc(count * sizeof(*binds));
        if (!binds) {
            spvReflectDestroyShaderModule(&mod);
            continue;
        }
        if (spvReflectEnumerateDescriptorBindings(&mod, &count, binds) != SPV_REFLECT_RESULT_SUCCESS) {
            free((void*)binds);
            spvReflectDestroyShaderModule(&mod);
            continue;
        }

        for (uint32_t i = 0; i < count; ++i) {
            const SpvReflectDescriptorBinding *b = binds[i];

            BindingIdentifier key;
            make_binding_key(b, &key);

            ResourceTypeDescriptor *existing = StringToUniformMap_get(map, key);
            if (existing) {
                existing->visibility |= stage_mask;
                uint32_t msz = min_size_from_binding(b);
                if (msz > existing->minBindingSize)
                    existing->minBindingSize = msz;
                /* If same name used for different bindings across stages, keep first type. */
                continue;
            }

            ResourceTypeDescriptor rtd;
            memset(&rtd, 0, sizeof(rtd));
            rtd.type = uniform_type_from_binding(b);
            rtd.minBindingSize = min_size_from_binding(b);
            rtd.location = b->binding; // binding index inside its set/group 0 hint
            rtd.access = access_from_binding(b);
            rtd.fstype = fstype_from_binding(b);
            rtd.visibility = stage_mask;

            /* Insert */
            (void)StringToUniformMap_put(map, key, rtd);
        }

        free((void*)binds);
        spvReflectDestroyShaderModule(&mod);
    }

    return map;
}

EntryPointSet getEntryPointsSPIRV(const uint32_t *shaderSourceSPIRV, uint32_t wordCount) {
    EntryPointSet eps;
    memset(&eps, 0, sizeof(eps));
    if (!shaderSourceSPIRV || wordCount == 0)
        return eps;

    SpvReflectShaderModule mod = {};
    spvReflectCreateShaderModule((size_t)wordCount * sizeof(uint32_t), shaderSourceSPIRV, &mod);

    const uint32_t ep_count = mod.entry_point_count;
    for (uint32_t i = 0; i < ep_count; ++i) {
        const SpvReflectEntryPoint *spvEP = spvReflectGetEntryPoint(&mod, mod.entry_points[i].name);
        const SpvReflectShaderStageFlagBits st = spvEP->shader_stage;

        int idx = -1;
        switch (st) {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
            idx = WGPUShaderStageEnum_Vertex;
            break;
        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
            idx = WGPUShaderStageEnum_Fragment;
            break;
        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
            idx = WGPUShaderStageEnum_Compute;
            break;
        #if SUPPORT_VULKAN_BACKEND == 1
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            idx = WGPUShaderStageEnum_TessControl;
            break;
        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            idx = WGPUShaderStageEnum_TessEvaluation;
            break;
        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
            idx = WGPUShaderStageEnum_Geometry;
            break;

        case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:
            idx = WGPUShaderStageEnum_RayGen;
            break;
        case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR:
            idx = WGPUShaderStageEnum_Intersect;
            break;
        case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR:
            idx = WGPUShaderStageEnum_AnyHit;
            break;
        case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
            idx = WGPUShaderStageEnum_ClosestHit;
            break;
        case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:
            idx = WGPUShaderStageEnum_Miss;
            break;
        case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR:
            idx = WGPUShaderStageEnum_Callable;
            break;

        case SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT:
            idx = WGPUShaderStageEnum_Task;
            break;
        case SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT:
            idx = WGPUShaderStageEnum_Mesh;
            break;
        #endif
        default:
            break;
        }

        if (idx >= 0) {
            const char *name = mod.entry_points[i].name;
            if (name && eps.names[idx][0] == '\0') {
                size_t n = strlen(name);
                if (n > (size_t)MAX_SHADER_ENTRYPOINT_NAME_LENGTH)
                    n = (size_t)MAX_SHADER_ENTRYPOINT_NAME_LENGTH;
                memcpy(eps.names[idx], name, n);
                eps.names[idx][n] = '\0';
            }
        }
    }
    return eps;
}

void UnloadBindGroup(DescribedBindGroup *bg) {
    free(bg->entries);
    wgpuBindGroupRelease((WGPUBindGroup)bg->bindGroup);
}
void UnloadBindGroupLayout(DescribedBindGroupLayout *bglayout) {
    free(bglayout->entries);
    wgpuBindGroupLayoutRelease((WGPUBindGroupLayout)bglayout->layout);
}

const char* WGPUFeatureToString (WGPUFeatureName feature) {
    switch(feature){
    case WGPUFeatureName_DepthClipControl: return "WGPUFeatureName_DepthClipControl";
    case WGPUFeatureName_Depth32FloatStencil8: return "WGPUFeatureName_Depth32FloatStencil8";
    case WGPUFeatureName_TimestampQuery: return "WGPUFeatureName_TimestampQuery";
    case WGPUFeatureName_TextureCompressionBC: return "WGPUFeatureName_TextureCompressionBC";
    case WGPUFeatureName_TextureCompressionBCSliced3D: return "WGPUFeatureName_TextureCompressionBCSliced3D";
    case WGPUFeatureName_TextureCompressionETC2: return "WGPUFeatureName_TextureCompressionETC2";
    case WGPUFeatureName_TextureCompressionASTC: return "WGPUFeatureName_TextureCompressionASTC";
    case WGPUFeatureName_TextureCompressionASTCSliced3D: return "WGPUFeatureName_TextureCompressionASTCSliced3D";
    case WGPUFeatureName_IndirectFirstInstance: return "WGPUFeatureName_IndirectFirstInstance";
    case WGPUFeatureName_ShaderF16: return "WGPUFeatureName_ShaderF16";
#ifndef __EMSCRIPTEN__
    case WGPUFeatureName_RG11B10UfloatRenderable: return "WGPUFeatureName_RG11B10UfloatRenderable";
    case WGPUFeatureName_BGRA8UnormStorage: return "WGPUFeatureName_BGRA8UnormStorage";
    case WGPUFeatureName_Float32Filterable: return "WGPUFeatureName_Float32Filterable";
    case WGPUFeatureName_Float32Blendable: return "WGPUFeatureName_Float32Blendable";
    case WGPUFeatureName_ClipDistances: return "WGPUFeatureName_ClipDistances";
    case WGPUFeatureName_DualSourceBlending: return "WGPUFeatureName_DualSourceBlending";
    case WGPUFeatureName_Subgroups: return "WGPUFeatureName_Subgroups";
    case WGPUFeatureName_CoreFeaturesAndLimits: return "WGPUFeatureName_CoreFeaturesAndLimits";
#endif
    default: return "?unknown feature?";
    }
};


const char* PresentModeName(WGPUPresentMode mode) {
    switch(mode){
        case WGPUPresentMode_Fifo: return  "WGPUPresentMode_Fifo";
        case WGPUPresentMode_FifoRelaxed: return  "WGPUPresentMode_FifoRelaxed";
        case WGPUPresentMode_Immediate: return  "WGPUPresentMode_Immediate";
        case WGPUPresentMode_Mailbox: return "WGPUPresentMode_Mailbox";
        default: return "?Unknown WGPUPresentMode?";
    }
};

const char* TextureFormatName(WGPUTextureFormat fmt) {
    switch (fmt) {
    case WGPUTextureFormat_Undefined:
        return "WGPUTextureFormat_Undefined";
    case WGPUTextureFormat_R8Unorm:
        return "WGPUTextureFormat_R8Unorm";
    case WGPUTextureFormat_R8Snorm:
        return "WGPUTextureFormat_R8Snorm";
    case WGPUTextureFormat_R8Uint:
        return "WGPUTextureFormat_R8Uint";
    case WGPUTextureFormat_R8Sint:
        return "WGPUTextureFormat_R8Sint";
    case WGPUTextureFormat_R16Uint:
        return "WGPUTextureFormat_R16Uint";
    case WGPUTextureFormat_R16Sint:
        return "WGPUTextureFormat_R16Sint";
    case WGPUTextureFormat_R16Float:
        return "WGPUTextureFormat_R16Float";
    case WGPUTextureFormat_RG8Unorm:
        return "WGPUTextureFormat_RG8Unorm";
    case WGPUTextureFormat_RG8Snorm:
        return "WGPUTextureFormat_RG8Snorm";
    case WGPUTextureFormat_RG8Uint:
        return "WGPUTextureFormat_RG8Uint";
    case WGPUTextureFormat_RG8Sint:
        return "WGPUTextureFormat_RG8Sint";
    case WGPUTextureFormat_R32Float:
        return "WGPUTextureFormat_R32Float";
    case WGPUTextureFormat_R32Uint:
        return "WGPUTextureFormat_R32Uint";
    case WGPUTextureFormat_R32Sint:
        return "WGPUTextureFormat_R32Sint";
    case WGPUTextureFormat_RG16Uint:
        return "WGPUTextureFormat_RG16Uint";
    case WGPUTextureFormat_RG16Sint:
        return "WGPUTextureFormat_RG16Sint";
    case WGPUTextureFormat_RG16Float:
        return "WGPUTextureFormat_RG16Float";
    case WGPUTextureFormat_RGBA8Unorm:
        return "WGPUTextureFormat_RGBA8Unorm";
    case WGPUTextureFormat_RGBA8UnormSrgb:
        return "WGPUTextureFormat_RGBA8UnormSrgb";
    case WGPUTextureFormat_RGBA8Snorm:
        return "WGPUTextureFormat_RGBA8Snorm";
    case WGPUTextureFormat_RGBA8Uint:
        return "WGPUTextureFormat_RGBA8Uint";
    case WGPUTextureFormat_RGBA8Sint:
        return "WGPUTextureFormat_RGBA8Sint";
    case WGPUTextureFormat_BGRA8Unorm:
        return "WGPUTextureFormat_BGRA8Unorm";
    case WGPUTextureFormat_BGRA8UnormSrgb:
        return "WGPUTextureFormat_BGRA8UnormSrgb";
    case WGPUTextureFormat_RGB10A2Uint:
        return "WGPUTextureFormat_RGB10A2Uint";
    case WGPUTextureFormat_RGB10A2Unorm:
        return "WGPUTextureFormat_RGB10A2Unorm";
    case WGPUTextureFormat_RG11B10Ufloat:
        return "WGPUTextureFormat_RG11B10Ufloat";
    case WGPUTextureFormat_RGB9E5Ufloat:
        return "WGPUTextureFormat_RGB9E5Ufloat";
    case WGPUTextureFormat_RG32Float:
        return "WGPUTextureFormat_RG32Float";
    case WGPUTextureFormat_RG32Uint:
        return "WGPUTextureFormat_RG32Uint";
    case WGPUTextureFormat_RG32Sint:
        return "WGPUTextureFormat_RG32Sint";
    case WGPUTextureFormat_RGBA16Uint:
        return "WGPUTextureFormat_RGBA16Uint";
    case WGPUTextureFormat_RGBA16Sint:
        return "WGPUTextureFormat_RGBA16Sint";
    case WGPUTextureFormat_RGBA16Float:
        return "WGPUTextureFormat_RGBA16Float";
    case WGPUTextureFormat_RGBA32Float:
        return "WGPUTextureFormat_RGBA32Float";
    case WGPUTextureFormat_RGBA32Uint:
        return "WGPUTextureFormat_RGBA32Uint";
    case WGPUTextureFormat_RGBA32Sint:
        return "WGPUTextureFormat_RGBA32Sint";
    case WGPUTextureFormat_Stencil8:
        return "WGPUTextureFormat_Stencil8";
    case WGPUTextureFormat_Depth16Unorm:
        return "WGPUTextureFormat_Depth16Unorm";
    case WGPUTextureFormat_Depth24Plus:
        return "WGPUTextureFormat_Depth24Plus";
    case WGPUTextureFormat_Depth24PlusStencil8:
        return "WGPUTextureFormat_Depth24PlusStencil8";
    case WGPUTextureFormat_Depth32Float:
        return "WGPUTextureFormat_Depth32Float";
    case WGPUTextureFormat_Depth32FloatStencil8:
        return "WGPUTextureFormat_Depth32FloatStencil8";
    case WGPUTextureFormat_BC1RGBAUnorm:
        return "WGPUTextureFormat_BC1RGBAUnorm";
    case WGPUTextureFormat_BC1RGBAUnormSrgb:
        return "WGPUTextureFormat_BC1RGBAUnormSrgb";
    case WGPUTextureFormat_BC2RGBAUnorm:
        return "WGPUTextureFormat_BC2RGBAUnorm";
    case WGPUTextureFormat_BC2RGBAUnormSrgb:
        return "WGPUTextureFormat_BC2RGBAUnormSrgb";
    case WGPUTextureFormat_BC3RGBAUnorm:
        return "WGPUTextureFormat_BC3RGBAUnorm";
    case WGPUTextureFormat_BC3RGBAUnormSrgb:
        return "WGPUTextureFormat_BC3RGBAUnormSrgb";
    case WGPUTextureFormat_BC4RUnorm:
        return "WGPUTextureFormat_BC4RUnorm";
    case WGPUTextureFormat_BC4RSnorm:
        return "WGPUTextureFormat_BC4RSnorm";
    case WGPUTextureFormat_BC5RGUnorm:
        return "WGPUTextureFormat_BC5RGUnorm";
    case WGPUTextureFormat_BC5RGSnorm:
        return "WGPUTextureFormat_BC5RGSnorm";
    case WGPUTextureFormat_BC6HRGBUfloat:
        return "WGPUTextureFormat_BC6HRGBUfloat";
    case WGPUTextureFormat_BC6HRGBFloat:
        return "WGPUTextureFormat_BC6HRGBFloat";
    case WGPUTextureFormat_BC7RGBAUnorm:
        return "WGPUTextureFormat_BC7RGBAUnorm";
    case WGPUTextureFormat_BC7RGBAUnormSrgb:
        return "WGPUTextureFormat_BC7RGBAUnormSrgb";
    case WGPUTextureFormat_ETC2RGB8Unorm:
        return "WGPUTextureFormat_ETC2RGB8Unorm";
    case WGPUTextureFormat_ETC2RGB8UnormSrgb:
        return "WGPUTextureFormat_ETC2RGB8UnormSrgb";
    case WGPUTextureFormat_ETC2RGB8A1Unorm:
        return "WGPUTextureFormat_ETC2RGB8A1Unorm";
    case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
        return "WGPUTextureFormat_ETC2RGB8A1UnormSrgb";
    case WGPUTextureFormat_ETC2RGBA8Unorm:
        return "WGPUTextureFormat_ETC2RGBA8Unorm";
    case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
        return "WGPUTextureFormat_ETC2RGBA8UnormSrgb";
    case WGPUTextureFormat_EACR11Unorm:
        return "WGPUTextureFormat_EACR11Unorm";
    case WGPUTextureFormat_EACR11Snorm:
        return "WGPUTextureFormat_EACR11Snorm";
    case WGPUTextureFormat_EACRG11Unorm:
        return "WGPUTextureFormat_EACRG11Unorm";
    case WGPUTextureFormat_EACRG11Snorm:
        return "WGPUTextureFormat_EACRG11Snorm";
    case WGPUTextureFormat_ASTC4x4Unorm:
        return "WGPUTextureFormat_ASTC4x4Unorm";
    case WGPUTextureFormat_ASTC4x4UnormSrgb:
        return "WGPUTextureFormat_ASTC4x4UnormSrgb";
    case WGPUTextureFormat_ASTC5x4Unorm:
        return "WGPUTextureFormat_ASTC5x4Unorm";
    case WGPUTextureFormat_ASTC5x4UnormSrgb:
        return "WGPUTextureFormat_ASTC5x4UnormSrgb";
    case WGPUTextureFormat_ASTC5x5Unorm:
        return "WGPUTextureFormat_ASTC5x5Unorm";
    case WGPUTextureFormat_ASTC5x5UnormSrgb:
        return "WGPUTextureFormat_ASTC5x5UnormSrgb";
    case WGPUTextureFormat_ASTC6x5Unorm:
        return "WGPUTextureFormat_ASTC6x5Unorm";
    case WGPUTextureFormat_ASTC6x5UnormSrgb:
        return "WGPUTextureFormat_ASTC6x5UnormSrgb";
    case WGPUTextureFormat_ASTC6x6Unorm:
        return "WGPUTextureFormat_ASTC6x6Unorm";
    case WGPUTextureFormat_ASTC6x6UnormSrgb:
        return "WGPUTextureFormat_ASTC6x6UnormSrgb";
    case WGPUTextureFormat_ASTC8x5Unorm:
        return "WGPUTextureFormat_ASTC8x5Unorm";
    case WGPUTextureFormat_ASTC8x5UnormSrgb:
        return "WGPUTextureFormat_ASTC8x5UnormSrgb";
    case WGPUTextureFormat_ASTC8x6Unorm:
        return "WGPUTextureFormat_ASTC8x6Unorm";
    case WGPUTextureFormat_ASTC8x6UnormSrgb:
        return "WGPUTextureFormat_ASTC8x6UnormSrgb";
    case WGPUTextureFormat_ASTC8x8Unorm:
        return "WGPUTextureFormat_ASTC8x8Unorm";
    case WGPUTextureFormat_ASTC8x8UnormSrgb:
        return "WGPUTextureFormat_ASTC8x8UnormSrgb";
    case WGPUTextureFormat_ASTC10x5Unorm:
        return "WGPUTextureFormat_ASTC10x5Unorm";
    case WGPUTextureFormat_ASTC10x5UnormSrgb:
        return "WGPUTextureFormat_ASTC10x5UnormSrgb";
    case WGPUTextureFormat_ASTC10x6Unorm:
        return "WGPUTextureFormat_ASTC10x6Unorm";
    case WGPUTextureFormat_ASTC10x6UnormSrgb:
        return "WGPUTextureFormat_ASTC10x6UnormSrgb";
    case WGPUTextureFormat_ASTC10x8Unorm:
        return "WGPUTextureFormat_ASTC10x8Unorm";
    case WGPUTextureFormat_ASTC10x8UnormSrgb:
        return "WGPUTextureFormat_ASTC10x8UnormSrgb";
    case WGPUTextureFormat_ASTC10x10Unorm:
        return "WGPUTextureFormat_ASTC10x10Unorm";
    case WGPUTextureFormat_ASTC10x10UnormSrgb:
        return "WGPUTextureFormat_ASTC10x10UnormSrgb";
    case WGPUTextureFormat_ASTC12x10Unorm:
        return "WGPUTextureFormat_ASTC12x10Unorm";
    case WGPUTextureFormat_ASTC12x10UnormSrgb:
        return "WGPUTextureFormat_ASTC12x10UnormSrgb";
    case WGPUTextureFormat_ASTC12x12Unorm:
        return "WGPUTextureFormat_ASTC12x12Unorm";
    case WGPUTextureFormat_ASTC12x12UnormSrgb:
        return "WGPUTextureFormat_ASTC12x12UnormSrgb";
#if !defined(__EMSCRIPTEN__) // why??
    case WGPUTextureFormat_R16Unorm:
        return "WGPUTextureFormat_R16Unorm";
    case WGPUTextureFormat_RG16Unorm:
        return "WGPUTextureFormat_RG16Unorm";
    case WGPUTextureFormat_RGBA16Unorm:
        return "WGPUTextureFormat_RGBA16Unorm";
    case WGPUTextureFormat_R16Snorm:
        return "WGPUTextureFormat_R16Snorm";
    case WGPUTextureFormat_RG16Snorm:
        return "WGPUTextureFormat_RG16Snorm";
    case WGPUTextureFormat_RGBA16Snorm:
        return "WGPUTextureFormat_RGBA16Snorm";
    case WGPUTextureFormat_R8BG8Biplanar420Unorm:
        return "WGPUTextureFormat_R8BG8Biplanar420Unorm";
    case WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm:
        return "WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm";
    case WGPUTextureFormat_R8BG8A8Triplanar420Unorm:
        return "WGPUTextureFormat_R8BG8A8Triplanar420Unorm";
    case WGPUTextureFormat_R8BG8Biplanar422Unorm:
        return "WGPUTextureFormat_R8BG8Biplanar422Unorm";
    case WGPUTextureFormat_R8BG8Biplanar444Unorm:
        return "WGPUTextureFormat_R8BG8Biplanar444Unorm";
    case WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm:
        return "WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm";
    case WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm:
        return "WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm";
    case WGPUTextureFormat_External:
        return "WGPUTextureFormat_External";
    case WGPUTextureFormat_Force32:
        return "WGPUTextureFormat_Force32";
#endif
    default:
        return "?? Unknown WGPUTextureFormat value ??";
    }
}



// end file src/backend_wgpu.c
