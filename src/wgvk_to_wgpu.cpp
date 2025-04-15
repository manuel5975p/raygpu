#include <webgpu/webgpu.h>
#include <wgvk.h>
#include <vector>
#include <raygpu.h>

static inline WGPUStorageTextureAccess toStorageTextureAccess(access_type acc){
    switch(acc){
        case access_type::readonly:return WGPUStorageTextureAccess_ReadOnly;
        case access_type::readwrite:return WGPUStorageTextureAccess_ReadWrite;
        case access_type::writeonly:return WGPUStorageTextureAccess_WriteOnly;
        default: rg_unreachable();
    }
    return WGPUStorageTextureAccess_Force32;
}
static inline WGPUBufferBindingType toStorageBufferAccess(access_type acc){
    switch(acc){
        case access_type::readonly: return WGPUBufferBindingType_ReadOnlyStorage;
        case access_type::readwrite:return WGPUBufferBindingType_Storage;
        case access_type::writeonly:return WGPUBufferBindingType_Storage;
        default: rg_unreachable();
    }
    return WGPUBufferBindingType_Force32;
}
static inline WGPUTextureFormat toStorageTextureFormat(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::format_r32float: return WGPUTextureFormat_R32Float;
        case format_or_sample_type::format_r32uint: return WGPUTextureFormat_R32Uint;
        case format_or_sample_type::format_rgba8unorm: return WGPUTextureFormat_RGBA8Unorm;
        case format_or_sample_type::format_rgba32float: return WGPUTextureFormat_RGBA32Float;
        default: rg_unreachable();
    }
    return WGPUTextureFormat_Force32;
}
static inline WGPUTextureSampleType toTextureSampleType(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::sample_f32: return WGPUTextureSampleType_Float;
        case format_or_sample_type::sample_u32: return WGPUTextureSampleType_Uint;
        default: rg_unreachable();
    }
    return WGPUTextureSampleType_Force32;
}
void wgvkQueueWriteTexture(WGVKQueue queue, WGVKTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGVKTexelCopyBufferLayout const * dataLayout, WGVKExtent3D const * writeSize){
    wgpuQueueWriteTexture(queue, destination, data, dataSize, dataLayout, writeSize);
}
void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities){
    wgpuSurfaceGetCapabilities(wgvkSurface, adapter, capabilities);
}
void wgvkSurfaceConfigure(WGVKSurface surface, const WGVKSurfaceConfiguration* config){
    wgpuSurfaceConfigure(surface, config);
}
//void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    //wgpuCommandEncoderTransitionTextureLayout(encoder, texture, from, to);
//}
WGVKTexture wgvkDeviceCreateTexture(WGVKDevice device, const WGVKTextureDescriptor* descriptor){
    return wgpuDeviceCreateTexture(device, descriptor);
}
WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor){
    return wgpuTextureCreateView(texture, descriptor);
}
WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const WGVKBufferDescriptor* desc){
    return wgpuDeviceCreateBuffer(device, desc);
}
void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, const void* data, size_t size){
    wgpuQueueWriteBuffer(cSelf, buffer, bufferOffset, data, size);
}
void wgvkBufferMap(WGVKBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data){
    WGPUBufferMapCallbackInfo callbackInfo zeroinit;
    callbackInfo.mode = WGPUCallbackMode_WaitAnyOnly;
    bool mapOk = false;
    auto capturing_lambda = [&data, &mapOk](WGPUMapAsyncStatus mapStatus, WGPUStringView label, void* userdata){
        if(mapStatus == WGPUMapAsyncStatus_Success){
            mapOk = true;
        }
    };
    auto noncapturing_lambda = [](WGPUMapAsyncStatus mapStatus, WGPUStringView label, void* userdata1, void* userdata2){
        reinterpret_cast<decltype(capturing_lambda)*>(userdata1)->operator()(mapStatus, label, userdata2);
    };
    callbackInfo.callback = noncapturing_lambda;
    WGPUFuture mapFuture = wgpuBufferMapAsync(buffer, mapmode, offset, size, callbackInfo);
    WGPUFutureWaitInfo info zeroinit;
    info.future = mapFuture;
    WGPUWaitStatus waitStatus = wgpuInstanceWaitAny(WGPUInstance{}, 1, &info, 1 << 30);
    if(waitStatus == WGPUWaitStatus_Success && mapOk){
        *data = wgpuBufferGetMappedRange(buffer, offset, size);
    }
    else{
        *data = nullptr;
    }
}
void wgvkBufferUnmap(WGVKBuffer buffer){
    wgpuBufferUnmap(buffer);
}
size_t wgvkBufferGetSize(WGVKBuffer buffer){
    return wgpuBufferGetSize(buffer);
}
WGVKBindGroupLayout wgvkDeviceCreateBindGroupLayout(WGVKDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount){
    WGPUShaderStage vfragmentOnly = WGPUShaderStage_Fragment;
    WGPUShaderStage visible = visible = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute;
    
    std::vector<WGPUBindGroupLayoutEntry> blayouts(entryCount);
    for(size_t i = 0;i < entryCount;i++){
        blayouts[i].binding = entries[i].location;
        switch(entries[i].type){
            default:
                rg_unreachable();
            case uniform_buffer:
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                blayouts[i].buffer.minBindingSize = entries[i].minBindingSize;
            break;
            case storage_buffer:{
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = toStorageBufferAccess(entries[i].access);
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case texture2d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(entries[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(entries[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(entries[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
            case texture_sampler:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            case texture3d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(entries[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_3D;
            break;
            case storage_texture2d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(entries[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(entries[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case storage_texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(entries[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(entries[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
            case storage_texture3d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(entries[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(entries[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_3D;
            break;
        }
    }
    WGPUBindGroupLayoutDescriptor bgld zeroinit;
    bgld.entries = blayouts.data();
    bgld.entryCount = blayouts.size();

    return wgpuDeviceCreateBindGroupLayout(device, &bgld);
}
WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc){
    return wgpuDeviceCreateBindGroup(device, bgdesc);
}
void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup bindGroup, const WGVKBindGroupDescriptor* bgdesc){
    //wgpuWriteBindGroup(device, bindGroup, bgdesc);
}
//void wgvkQueueTransitionLayout(WGVKQueue cSelf, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    //wgpuQueueTransitionLayout(cSelf, texture, from, to);
//}
WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* cdesc){
    return wgpuDeviceCreateCommandEncoder(device, cdesc);
}
WGVKRenderPassEncoder wgvkCommandEncoderBeginRenderPass(WGVKCommandEncoder enc, const WGVKRenderPassDescriptor* rpdesc){
    return wgpuCommandEncoderBeginRenderPass(enc, rpdesc);
}
void wgvkRenderPassEncoderEnd(WGVKRenderPassEncoder renderPassEncoder){
    wgpuRenderPassEncoderEnd(renderPassEncoder);
}
WGVKCommandBuffer wgvkCommandEncoderFinish(WGVKCommandEncoder commandEncoder){
    return wgpuCommandEncoderFinish(commandEncoder, nullptr);
}
void wgvkQueueSubmit(WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers){
    wgpuQueueSubmit(queue, commandCount, buffers);
}
void wgvkCommandEncoderCopyBufferToBuffer(WGVKCommandEncoder commandEncoder, WGVKBuffer source, uint64_t sourceOffset, WGVKBuffer destination, uint64_t destinationOffset, uint64_t size){
    wgpuCommandEncoderCopyBufferToBuffer(commandEncoder, source, sourceOffset, destination, destinationOffset, size);
}
void wgvkCommandEncoderCopyBufferToTexture(WGVKCommandEncoder commandEncoder, WGVKTexelCopyBufferInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize){
    wgpuCommandEncoderCopyBufferToTexture(commandEncoder, source, destination, copySize);
}
void wgvkCommandEncoderCopyTextureToBuffer(WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyBufferInfo const * destination, WGVKExtent3D const * copySize){
    wgpuCommandEncoderCopyTextureToBuffer(commandEncoder, source, destination, copySize);
}
void wgvkCommandEncoderCopyTextureToTexture(WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize){
    wgpuCommandEncoderCopyTextureToTexture(commandEncoder, source, destination, copySize);
}
void wgvkRenderpassEncoderDraw(WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance){
    wgpuRenderPassEncoderDraw(rpe, vertices, instances, firstvertex, firstinstance);
}
void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance){
    //TODO basevertex?
    wgpuRenderPassEncoderDrawIndexed(rpe, indices, instances, firstindex, 0, firstinstance);
}
void wgvkRenderPassEncoderSetBindGroup(WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset){
    wgpuRenderPassEncoderSetBindGroup(rpe, group, dset, 0, nullptr);
}
void wgvkRenderPassEncoderBindPipeline(WGVKRenderPassEncoder rpe, struct DescribedPipeline* pipeline){
    wgpuRenderPassEncoderSetPipeline(rpe, (WGPURenderPipeline)pipeline); //UUUUUUH this assumes that the WGVKRenderPipeline is the first member of the DescribedPipeline
}
WGVKInstance wgvkCreateInstance(const WGVKInstanceDescriptor *descriptor){
    return wgpuCreateInstance(descriptor);
}
WGVKFuture wgvkInstanceRequestAdapter(WGVKInstance instance, const WGVKRequestAdapterOptions* options, WGVKRequestAdapterCallbackInfo callbackInfo){
    return wgpuInstanceRequestAdapter(instance, options, callbackInfo);
}
WGVKDevice wgpuAdapterCreateDevice(WGPUAdapter adapter, const WGVKDeviceDescriptor *descriptor){

}
//void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, VkPipeline pipeline, VkPipelineLayout layout){
    //TODO 
    //wgpuRenderPassEncoderSetPipeline(rpe, pipeline, layout);
//}
//void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, IndexFormat indexType){
//    wgpuRenderPassEncoderSetIndexBuffer(rpe, buffer, toWebGPUIndexFormat(indexType), offset, wgpuBufferGetSize(buffer));
//}
//void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset){
//    wgpuRenderPassEncoderSetVertexBuffer(rpe, binding, buffer, offset, wgpuBufferGetSize(buffer));
//}
void wgvkComputePassEncoderSetPipeline(WGVKComputePassEncoder cpe, WGVKComputePipeline computePipeline){
    wgpuComputePassEncoderSetPipeline(cpe, computePipeline);
}
void wgvkComputePassEncoderSetBindGroup(WGVKComputePassEncoder cpe, uint32_t groupIndex, WGVKBindGroup bindGroup){
    wgpuComputePassEncoderSetBindGroup(cpe, groupIndex, bindGroup, 0, nullptr);
}
void wgvkComputePassEncoderDispatchWorkgroups(WGVKComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z){
    wgpuComputePassEncoderDispatchWorkgroups(cpe, x, y, z);
}
void wgvkReleaseComputePassEncoder(WGVKComputePassEncoder cpenc){
    wgpuComputePassEncoderRelease(cpenc);
}
WGVKComputePassEncoder wgvkCommandEncoderBeginComputePass(WGVKCommandEncoder enc){
    return wgpuCommandEncoderBeginComputePass(enc, nullptr);
}
void wgvkCommandEncoderEndComputePass(WGVKComputePassEncoder commandEncoder){
    wgpuComputePassEncoderEnd(commandEncoder);
}
void wgvkTextureAddRef(WGVKTexture texture){
    wgpuTextureAddRef(texture);
}
void wgvkTextureViewAddRef(WGVKTextureView textureView){
    wgpuTextureViewAddRef(textureView);
}
void wgvkBufferAddRef(WGVKBuffer buffer){
    wgpuBufferAddRef(buffer);
}
void wgvkBindGroupAddRef(WGVKBindGroup bindGroup){
    wgpuBindGroupAddRef(bindGroup);
}
void wgvkReleaseCommandEncoder(WGVKCommandEncoder commandBuffer){
    wgpuCommandEncoderRelease(commandBuffer);
}
WGVKCommandEncoder wgvkResetCommandBuffer(WGVKCommandBuffer commandEncoder){
    return nullptr;
}
void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer){
    wgpuCommandBufferRelease(commandBuffer);
}
void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc){
    wgpuRenderPassEncoderRelease(rpenc);
}
void wgvkBufferRelease(WGVKBuffer buffer){
    wgpuBufferRelease(buffer);
}
void wgvkBindGroupRelease(WGVKBindGroup bindGroup){
    wgpuBindGroupRelease(bindGroup);
}
void wgvkReleaseBindGroupLayout(WGVKBindGroupLayout bglayout){
    wgpuBindGroupLayoutRelease(bglayout);
}
void wgvkReleaseTexture(WGVKTexture texture){
    wgpuTextureRelease(texture);
}
void wgvkReleaseTextureView(WGVKTextureView view){
    wgpuTextureViewRelease(view);
}