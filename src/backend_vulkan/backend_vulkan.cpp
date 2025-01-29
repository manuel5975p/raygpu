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
    UnloadBuffer(vbo);
}

DescribedBuffer* GenBufferEx_Vk(const void *data, size_t size, BufferUsage usage){
    VkBufferUsageFlags vusage = toVulkanBufferUsage(usage);
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    VkBuffer vertexBuffer{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = vusage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_vulkanstate.device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
    VkDeviceMemory vertexBufferMemory;
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, vertexBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = g_vulkanstate.memoryTypes.hostVisibleCoherent;
    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device, vertexBuffer, vertexBufferMemory, 0);
    if(data != nullptr){
        void* mapdata;
        vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mapdata);
        memcpy(mapdata, data, (size_t)bufferInfo.size);
        vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    }
    ret->buffer = callocnewpp(BufferHandleImpl);
    ((BufferHandle)ret->buffer)->buffer = vertexBuffer;
    ((BufferHandle)ret->buffer)->memory = vertexBufferMemory;
    ((BufferHandle)ret->buffer)->refCount = 1;
    //vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, size, void **ppData)
    ret->size = bufferInfo.size;
    ret->usage = usage;

    //void* mdata;
    //vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mdata);
    //memcpy(mdata, data, (size_t)bufferInfo.size);
    //vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    return ret;
}