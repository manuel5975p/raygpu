#include "vulkan_internals.hpp"

void ReleaseCommandBuffer(CommmandBufferHandle commandBuffer){
    vkFreeCommandBuffers(g_vulkanstate.device, commandBuffer->pool, 1, &commandBuffer->buffer);
}
void ReleaseRenderPassEncoder(RenderPassEncoderHandle rpenc){
    --rpenc ->refCount;
    if(rpenc->refCount == 0){
        for(auto x : rpenc->referencedBuffers){
            ReleaseBuffer(x);
        }
        for(auto x : rpenc->referencedDescriptorSets){
            ReleaseDescriptorSet(x);
        }
    }
}
void ReleaseBuffer(BufferHandle buffer){
    --buffer->refCount;
    if(buffer->refCount == 0){
        vkDestroyBuffer(g_vulkanstate.device, buffer->buffer, nullptr);
        vkFreeMemory(g_vulkanstate.device, buffer->memory, nullptr);
    }
}
void ReleaseDescriptorSet(DescriptorSetHandle dshandle){
    --dshandle->refCount;
    if(dshandle->refCount == 0){
        vkFreeDescriptorSets(g_vulkanstate.device, dshandle->pool, 1, &dshandle->set);
        vkDestroyDescriptorPool(g_vulkanstate.device, dshandle->pool, nullptr);
    }
}