#include "vulkan_internals.hpp"

void ReleaseCommandBuffer(CommmandBufferHandle commandBuffer) { vkFreeCommandBuffers(g_vulkanstate.device, commandBuffer->pool, 1, &commandBuffer->buffer); }
void ReleaseRenderPassEncoder(RenderPassEncoderHandle rpenc) {
    --rpenc->refCount;
    if (rpenc->refCount == 0) {
        for (auto x : rpenc->referencedBuffers) {
            ReleaseBuffer(x);
        }
        for (auto x : rpenc->referencedDescriptorSets) {
            ReleaseDescriptorSet(x);
        }
    }
}
void ReleaseBuffer(BufferHandle buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vkDestroyBuffer(g_vulkanstate.device, buffer->buffer, nullptr);
        vkFreeMemory(g_vulkanstate.device, buffer->memory, nullptr);
    }
}
void ReleaseDescriptorSet(DescriptorSetHandle dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        vkFreeDescriptorSets(g_vulkanstate.device, dshandle->pool, 1, &dshandle->set);
        vkDestroyDescriptorPool(g_vulkanstate.device, dshandle->pool, nullptr);
    }
}

// Implementation of RenderpassEncoderDraw
void RenderpassEncoderDraw(RenderPassEncoderHandle rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Record the draw command into the command buffer
    vkCmdDraw(rpe->cmdBuffer, vertices, instances, firstvertex, firstinstance);
}

// Implementation of RenderpassEncoderDrawIndexed
void RenderpassEncoderDrawIndexed(RenderPassEncoderHandle rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Assuming vertexOffset is 0. Modify if you have a different offset.
    int32_t vertexOffset = 0;

    // Record the indexed draw command into the command buffer
    vkCmdDrawIndexed(rpe->cmdBuffer, indices, instances, firstindex, vertexOffset, firstinstance);
}
void RenderPassDescriptorBindPipeline(RenderPassEncoderHandle rpe, uint32_t group, DescribedPipeline *pipeline) { rpe->lastLayout = (VkPipelineLayout)pipeline->layout.layout; }
// Implementation of RenderPassDescriptorBindDescriptorSet
void RenderPassDescriptorBindDescriptorSet(RenderPassEncoderHandle rpe, uint32_t group, DescriptorSetHandle dset) {
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

    ++dset->refCount;
    // Track the referenced descriptor set
    rpe->referencedDescriptorSets.push_back(dset);
}
void RenderPassDescriptorBindVertexBuffer(RenderPassEncoderHandle rpe, 
                                          uint32_t binding, 
                                          BufferHandle buffer, 
                                          VkDeviceSize offset) 
{
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the vertex buffer to the command buffer at the specified binding point
    vkCmdBindVertexBuffers(rpe->cmdBuffer, binding, 1, &(buffer->buffer), &offset);

    // Increment the reference count since rpe now holds a reference to buffer
    ++buffer->refCount;

    // Track the referenced buffer
    rpe->referencedBuffers.push_back(buffer);
}
void RenderPassDescriptorBindIndexBuffer(RenderPassEncoderHandle rpe, 
                                         BufferHandle buffer, 
                                         VkDeviceSize offset, 
                                         VkIndexType indexType) 
{
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the index buffer to the command buffer
    vkCmdBindIndexBuffer(rpe->cmdBuffer, buffer->buffer, offset, indexType);

    // Increment the reference count since rpe now holds a reference to buffer
    ++buffer->refCount;

    // Track the referenced buffer
    rpe->referencedBuffers.push_back(buffer);
}

