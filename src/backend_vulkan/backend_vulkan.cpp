#include "vulkan_internals.hpp"

void BufferData_Vk(DescribedBuffer* buffer, void* data, size_t size){
    if(buffer->size >= size){
        BufferHandle handle = (BufferHandle)buffer->buffer;
        void* udata = 0;
        VkResult mapresult = vkMapMemory(g_vulkanstate.device, handle->memory, 0, size, 0, &udata);
        if(mapresult == VK_SUCCESS){
            std::memcpy(data, udata, size);
            vkUnmapMemory(g_vulkanstate.device, handle->memory);
        }
        
        //vkBindBufferMemory()
    }
}

void drawCurrentBatch(){
    size_t vertexCount = vboptr - vboptr_base;
    if(vertexCount == 0){
        return;
    }
    DescribedBuffer* vbo = GenBufferEx_Vk(vboptr_base, vertexCount * sizeof(vertex), BufferUsage_Vertex | BufferUsage_CopyDst);
}