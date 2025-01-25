#include "vulkan_internals.hpp"
void wgvkReleaseCommandBuffer(CommmandBufferHandle commandBuffer) { vkFreeCommandBuffers(g_vulkanstate.device, commandBuffer->pool, 1, &commandBuffer->buffer); }
void wgvkReleaseRenderPassEncoder(RenderPassEncoderHandle rpenc) {
    --rpenc->refCount;
    if (rpenc->refCount == 0) {
        for (auto x : rpenc->referencedBuffers) {
            wgvkReleaseBuffer(x);
        }
        for (auto x : rpenc->referencedDescriptorSets) {
            wgvkReleaseDescriptorSet(x);
        }
        std::free(rpenc);
    }
}
void wgvkReleaseBuffer(BufferHandle buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vkDestroyBuffer(g_vulkanstate.device, buffer->buffer, nullptr);
        vkFreeMemory(g_vulkanstate.device, buffer->memory, nullptr);
    }
}
void wgvkReleaseDescriptorSet(DescriptorSetHandle dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        vkFreeDescriptorSets(g_vulkanstate.device, dshandle->pool, 1, &dshandle->set);
        vkDestroyDescriptorPool(g_vulkanstate.device, dshandle->pool, nullptr);
    }
}

// Implementation of RenderpassEncoderDraw
void wgvkRenderpassEncoderDraw(RenderPassEncoderHandle rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Record the draw command into the command buffer
    vkCmdDraw(rpe->cmdBuffer, vertices, instances, firstvertex, firstinstance);
}

// Implementation of RenderpassEncoderDrawIndexed
void wgvkRenderpassEncoderDrawIndexed(RenderPassEncoderHandle rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Assuming vertexOffset is 0. Modify if you have a different offset.
    int32_t vertexOffset = 0;

    // Record the indexed draw command into the command buffer
    vkCmdDrawIndexed(rpe->cmdBuffer, indices, instances, firstindex, vertexOffset, firstinstance);
}
void wgvkRenderPassEncoderBindPipeline(RenderPassEncoderHandle rpe, DescribedPipeline *pipeline) { 
    rpe->lastLayout = (VkPipelineLayout)pipeline->layout.layout; 
    vkCmdBindPipeline(rpe->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pipeline->quartet.pipeline_TriangleList);
}
// Implementation of RenderPassDescriptorBindDescriptorSet
void wgvkRenderPassEncoderBindDescriptorSet(RenderPassEncoderHandle rpe, uint32_t group, DescriptorSetHandle dset) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(dset != nullptr && "DescriptorSetHandle is null");
    assert(rpe->lastLayout != VK_NULL_HANDLE && "Pipeline layout is not set");

    // Bind the descriptor set to the command buffer
    vkCmdBindDescriptorSets(rpe->cmdBuffer,                  // Command buffer
                            VK_PIPELINE_BIND_POINT_GRAPHICS, // Pipeline bind point
                            rpe->lastLayout,                 // Pipeline layout
                            group,                           // First set
                            1,                               // Descriptor set count
                            &(dset->set),                    // Pointer to descriptor set
                            0,                               // Dynamic offset count
                            nullptr                          // Pointer to dynamic offsets
    );
    if(!rpe->referencedDescriptorSets.contains(dset)){
        ++dset->refCount;
        rpe->referencedDescriptorSets.insert(dset);
    }
}
void wgvkRenderPassEncoderBindVertexBuffer(RenderPassEncoderHandle rpe, uint32_t binding, BufferHandle buffer, VkDeviceSize offset) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the vertex buffer to the command buffer at the specified binding point
    vkCmdBindVertexBuffers(rpe->cmdBuffer, binding, 1, &(buffer->buffer), &offset);

    if(!rpe->referencedBuffers.contains(buffer)){
        ++buffer->refCount;
        rpe->referencedBuffers.insert(buffer);
    }
}
void wgvkRenderPassEncoderBindIndexBuffer(RenderPassEncoderHandle rpe, BufferHandle buffer, VkDeviceSize offset, VkIndexType indexType) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the index buffer to the command buffer
    vkCmdBindIndexBuffer(rpe->cmdBuffer, buffer->buffer, offset, indexType);

    if(!rpe->referencedBuffers.contains(buffer)){
        ++buffer->refCount;
        rpe->referencedBuffers.insert(buffer);
    }
}
