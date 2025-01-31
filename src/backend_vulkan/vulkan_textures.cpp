#include "vulkan_internals.hpp"
#include <raygpu.h>
#include <stdexcept>
#include <cstring>
#include <iostream>

// Utility function to translate PixelFormat to VkFormat


// Function to find a suitable memory type
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
    
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    throw std::runtime_error("Failed to find suitable memory type!");
}

// Function to create a Vulkan buffer
VkBuffer CreateBuffer(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create buffer!");
    
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, buffer, &memReq);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        g_vulkanstate.physicalDevice, 
        memReq.memoryTypeBits, 
        properties
    );
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate buffer memory!");
    
    vkBindBufferMemory(device, buffer, bufferMemory, 0);
    
    return buffer;
}

// Function to create a Vulkan image
ImageHandle CreateImage(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkDeviceMemory& imageMemory) {
    ImageHandle ret = callocnew(ImageHandleImpl);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    
    VkImage image;
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image!");
    
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, image, &memReq);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        g_vulkanstate.physicalDevice, 
        memReq.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate image memory!");
        abort();
    }
    vkBindImageMemory(device, image, imageMemory, 0);
    ret->image = image;
    ret->memory = imageMemory;
    return ret;
}

// Function to create an image view
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    
    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view!");
    
    return imageView;
}

// Function to begin a single-use command buffer
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    return commandBuffer;
}

// Function to end and submit a single-use command buffer
void EndSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("Failed to submit command buffer!");
    
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

// Function to transition image layout
void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, 
                           VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    barrier.image = image;
    barrier.subresourceRange.aspectMask = 
        (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("Unsupported layout transition!");
    }
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    
    EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

// Function to copy buffer to image
void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { width, height, 1 };
    
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    
    EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}

// Main function to create Vulkan image from RGBA8 data or as empty
ImageHandle CreateVkImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, 
                                                const uint8_t* data, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, bool hasData) {
    VkDeviceMemory imageMemory;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    
    
    ImageHandle image = CreateImage(device, width, height, format, usage, imageMemory);
    
    if (hasData && data != nullptr) {
        // Create staging buffer
        VkDeviceSize imageSize = width * height * 4; // Adjust based on format if necessary
        VkDeviceMemory stagingMemory;
        VkBuffer stagingBuffer = CreateBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingMemory);
        
        // Map and copy data to staging buffer
        void* mappedData;
        vkMapMemory(device, stagingMemory, 0, imageSize, 0, &mappedData);
        std::memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingMemory);
        
        // Transition image layout to TRANSFER_DST_OPTIMAL
        TransitionImageLayout(device, commandPool, queue, image->image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        // Copy buffer to image
        CopyBufferToImage(device, commandPool, queue, stagingBuffer, image->image, width, height);
        
        // Transition image layout to SHADER_READ_ONLY_OPTIMAL
        TransitionImageLayout(device, commandPool, queue, image->image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        // Cleanup staging resources
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
    }
    
    return image;
}

// Generalized LoadTexturePro function
extern "C" Texture LoadTexturePro_Vk(uint32_t width, uint32_t height, PixelFormat format, int usage, uint32_t sampleCount, uint32_t mipmaps, const void* data) {
    Texture ret{};
    
    VkFormat vkFormat = toVulkanPixelFormat(format);
    
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = g_vulkanstate.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    
    VkCommandPool commandPool;
    if (vkCreateCommandPool(g_vulkanstate.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");
    
    bool hasData = (data != nullptr);
    
    ImageHandle image = CreateVkImage(
        g_vulkanstate.device,
        g_vulkanstate.physicalDevice, 
        commandPool, 
        g_vulkanstate.queue.computeQueue, 
        reinterpret_cast<const uint8_t*>(data), 
        width, 
        height, 
        vkFormat,
        usage,
        hasData
    );
    
    ret.width = width;
    ret.height = height;
    ret.mipmaps = mipmaps;
    ret.sampleCount = sampleCount;
    ret.id = image;
    
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == Depth24 || format == Depth32) {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    
    ret.view = CreateImageView(g_vulkanstate.device, image->image, vkFormat, aspectFlags);
    
    // Handle mipmaps if necessary (not implemented here)
    // For simplicity, only base mip level is created. Extend as needed.
    
    std::cout << "Successfully created texture and image view\n";
    
    vkDestroyCommandPool(g_vulkanstate.device, commandPool, nullptr);
    
    return ret;
}

void UnloadTexture_Vk(Texture tex){
    vkDestroyImageView(g_vulkanstate.device, (VkImageView)tex.view, nullptr);

    ImageHandle handle = (ImageHandle)tex.id;
    vkDestroyImage(g_vulkanstate.device, handle->image, nullptr);
    vkFreeMemory(g_vulkanstate.device, handle->memory, nullptr);
}

// Updated LoadTextureFromImage_Vk function using LoadTexturePro
Texture LoadTextureFromImage_Vk(Image img) {
    if (img.format != RGBA8 && img.format != BGRA8 && img.format != RGBA16F && img.format != RGBA32F && img.format != Depth24) {
        throw std::runtime_error("Unsupported image format.");
    }
    
    return LoadTexturePro_Vk(
        img.width,
        img.height,
        img.format,
        TextureUsage_TextureBinding, // Assuming TextureUsage enum exists and has a Sampled option
        1, // sampleCount
        img.mipmaps > 0 ? img.mipmaps : 1, 
        img.data
    );
}
