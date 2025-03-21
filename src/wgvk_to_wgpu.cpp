#include <webgpu/webgpu.h>
#include <wgvk.h>



void wgvkQueueWriteTexture(WGVKQueue queue, WGVKTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGVKTexelCopyBufferLayout const * dataLayout, WGVKExtent3D const * writeSize){
    wgpuQueueWriteTexture(queue, destination, data, dataSize, dataLayout, writeSize);
}
void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities){
    wgpuSurfaceGetCapabilities(wgvkSurface, adapter, capabilities);
}
void wgvkSurfaceConfigure(WGVKSurface surface, const WGVKSurfaceConfiguration* config){
    wgpuSurfaceConfigure(surface, config);
}
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    //wgpuCommandEncoderTransitionTextureLayout(encoder, texture, from, to);
}
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
    return wgpuDeviceCreateBindGroupLayout(device, entries, entryCount);
}
WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc){
    return wgpuDeviceCreateBindGroup(device, bgdesc);
}
void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup bindGroup, const WGVKBindGroupDescriptor* bgdesc){
    wgpuWriteBindGroup(device, bindGroup, bgdesc);
}
void wgvkQueueTransitionLayout(WGVKQueue cSelf, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    wgpuQueueTransitionLayout(cSelf, texture, from, to);
}
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
    return wgpuCommandEncoderFinish(commandEncoder);
}
void wgvkQueueSubmit(WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers){
    wgpuQueueSubmit(queue, commandCount, buffers);
}
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    wgpuCommandEncoderTransitionTextureLayout(encoder, texture, from, to);
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
    wgpuRenderpassEncoderDraw(rpe, vertices, instances, firstvertex, firstinstance);
}
void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance){
    wgpuRenderpassEncoderDrawIndexed(rpe, indices, instances, firstindex, firstinstance);
}
void wgvkRenderPassEncoderSetBindGroup(WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset){
    wgpuRenderPassEncoderSetBindGroup(rpe, group, dset);
}
void wgvkRenderPassEncoderBindPipeline(WGVKRenderPassEncoder rpe, struct DescribedPipeline* pipeline){
    wgpuRenderPassEncoderBindPipeline(rpe, pipeline);
}
void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, VkPipeline pipeline, VkPipelineLayout layout){
    wgpuRenderPassEncoderSetPipeline(rpe, pipeline, layout);
}
void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, VkIndexType indexType){
    wgpuRenderPassEncoderBindIndexBuffer(rpe, buffer, offset, indexType);
}
void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset){
    wgpuRenderPassEncoderBindVertexBuffer(rpe, binding, buffer, offset);
}
void wgvkComputePassEncoderSetPipeline(WGVKComputePassEncoder cpe, VkPipeline pipeline, VkPipelineLayout layout){
    wgpuComputePassEncoderSetPipeline(cpe, pipeline, layout);
}
void wgvkComputePassEncoderSetBindGroup(WGVKComputePassEncoder cpe, uint32_t groupIndex, WGVKBindGroup bindGroup){
    wgpuComputePassEncoderSetBindGroup(cpe, groupIndex, bindGroup);
}
void wgvkComputePassEncoderDispatchWorkgroups(WGVKComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z){
    wgpuComputePassEncoderDispatchWorkgroups(cpe, x, y, z);
}
void wgvkReleaseComputePassEncoder(WGVKComputePassEncoder cpenc){
    wgpuReleaseComputePassEncoder(cpenc);
}
WGVKComputePassEncoder wgvkCommandEncoderBeginComputePass(WGVKCommandEncoder enc){
    return wgpuCommandEncoderBeginComputePass(enc);
}
void wgvkCommandEncoderEndComputePass(WGVKComputePassEncoder commandEncoder){
    wgpuCommandEncoderEndComputePass(commandEncoder);
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
    wgpuReleaseCommandEncoder(commandBuffer);
}
WGVKCommandEncoder wgvkResetCommandBuffer(WGVKCommandBuffer commandEncoder){
    return wgpuResetCommandBuffer(commandEncoder);
}
void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer){
    wgpuReleaseCommandBuffer(commandBuffer);
}
void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc){
    wgpuReleaseRenderPassEncoder(rpenc);
}
void wgvkReleaseComputePassEncoder(WGVKComputePassEncoder rpenc){
    wgpuReleaseComputePassEncoder(rpenc);
}
void wgvkBufferRelease(WGVKBuffer buffer){
    wgpuBufferRelease(buffer);
}
void wgvkBindGroupRelease(WGVKBindGroup bindGroup){
    wgpuBindGroupRelease(bindGroup);
}
void wgvkReleaseBindGroupLayout(WGVKBindGroupLayout bglayout){
    wgpuReleaseBindGroupLayout(bglayout);
}
void wgvkReleaseTexture(WGVKTexture texture){
    wgpuReleaseTexture(texture);
}
void wgvkReleaseTextureView(WGVKTextureView view){
    wgpuReleaseTextureView(view);
}