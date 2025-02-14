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
WGVKTexture CreateImage(VkDevice device, uint32_t width, uint32_t height, uint32_t sampleCount, VkFormat format, VkImageUsageFlags usage, VkDeviceMemory& imageMemory) {
    WGVKTexture ret = callocnew(ImageHandleImpl);

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
    imageInfo.samples = toVulkanSampleCount(sampleCount);
    
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
        //abort();
    }
    vkBindImageMemory(device, image, imageMemory, 0);
    ret->image = image;
    ret->memory = imageMemory;
    ret->device = device;
    ret->width = width;
    ret->height = height;
    ret->sampleCount = sampleCount;
    ret->depthOrArrayLayers = 1;
    return ret;
}

// Function to create an image view
WGVKTextureView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    WGVKTextureView ret = callocnew(WGVKTextureViewImpl);
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
    if (vkCreateImageView(device, &viewInfo, nullptr, &ret->view) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image view!");
    ret->format = viewInfo.format;

    return ret;
}

// Function to begin a single-use command buffer
VkCommandBuffer transientCommandBuffer{};
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    //if(!transientCommandBuffer){
        vkAllocateCommandBuffers(device, &allocInfo, &transientCommandBuffer);
    //}
    //else{
        //Never the case currently
        //vkResetCommandBuffer(transientCommandBuffer, 0);
    //}

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(transientCommandBuffer, &beginInfo);
    
    return transientCommandBuffer;
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
    commandBuffer = nullptr;
}
extern "C" void EncodeTransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image){
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
    VkRenderPassCreateInfo rpci{};
    
    
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
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        
        sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }
    
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}
// Function to transition image layout
extern "C" void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, 
                           VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    
    EncodeTransitionImageLayout(commandBuffer, oldLayout, newLayout, image);
    
    EndSingleTimeCommands(device, commandPool, queue, commandBuffer);
}
extern "C" WGVKTexture wgvkDeviceCreateTexture(VkDevice device, const WGVKTextureDescriptor* descriptor){
    VkDeviceMemory imageMemory;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    
    
    WGVKTexture image = CreateImage(device, descriptor->size.width, descriptor->size.height, descriptor->sampleCount, toVulkanPixelFormat(descriptor->format), descriptor->usage, imageMemory);
    
    /*if (hasData && data != nullptr) {
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
    }*/
    
    return image;
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
WGVKTexture CreateVkImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, 
                                                const uint8_t* data, uint32_t width, uint32_t height, uint32_t sampleCount, VkFormat format, VkImageUsageFlags usage, bool hasData) {
    VkDeviceMemory imageMemory;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    
    
    WGVKTexture image = CreateImage(device, width, height, sampleCount, format, usage, imageMemory);
    
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

extern "C" Texture LoadTexturePro_Data(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps, void* data) {
    Texture ret{};
    
    VkFormat vkFormat = toVulkanPixelFormat(format);
    VkImageUsageFlags vkUsage = toVulkanTextureUsage(usage, format);

    VkCommandPoolCreateInfo poolInfo zeroinit;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = g_vulkanstate.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    
    VkCommandPool commandPool;
    if (vkCreateCommandPool(g_vulkanstate.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");
    
    bool hasData = data != nullptr;
    
    WGVKTexture image = CreateVkImage(
        g_vulkanstate.device,
        g_vulkanstate.physicalDevice, 
        commandPool, 
        g_vulkanstate.queue.computeQueue, 
        (uint8_t*)data, 
        width, 
        height,
        sampleCount,
        vkFormat,
        vkUsage,
        hasData
    );
    
    ret.width = width;
    ret.height = height;
    ret.mipmaps = mipmaps;
    ret.sampleCount = sampleCount;
    ret.id = image;
    ret.format = format;
    
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format == Depth24 || format == Depth32) {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    WGVKTextureView view = CreateImageView(g_vulkanstate.device, image->image, vkFormat, aspectFlags);
    view->width = width;
    view->height = height;
    view->sampleCount = sampleCount;
    view->depthOrArrayLayers = 1;
    ret.view = view;
    // Handle mipmaps if necessary (not implemented here)
    // For simplicity, only base mip level is created. Extend as needed.
    
    TRACELOG(LOG_INFO, "Loaded WGVKTexture and view [%u y %u]", (unsigned)width, (unsigned)height);
    
    vkDestroyCommandPool(g_vulkanstate.device, commandPool, nullptr);
    
    return ret;
}
extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    return LoadTexturePro_Data(width, height, format, usage, sampleCount, mipmaps, nullptr);
}

void UnloadTexture(Texture tex){
    vkDestroyImageView(g_vulkanstate.device, (VkImageView)tex.view, nullptr);

    WGVKTexture handle = (WGVKTexture)tex.id;
    vkDestroyImage(g_vulkanstate.device, handle->image, nullptr);
    vkFreeMemory(g_vulkanstate.device, handle->memory, nullptr);
}

// Updated LoadTextureFromImage_Vk function using LoadTexturePro
Texture LoadTextureFromImage(Image img) {
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
    else if (img.format != RGBA8 && img.format != BGRA8 && img.format != RGBA16F && img.format != RGBA32F && img.format != Depth24) {
        throw std::runtime_error("Unsupported image format.");
    }
    TextureUsage x;
    auto ret = LoadTexturePro_Data(
        img.width,
        img.height,
        img.format == GRAYSCALE ? RGBA8 : img.format,
        TextureUsage_CopyDst | TextureUsage_TextureBinding, // Assuming TextureUsage enum exists and has a Sampled option
        1, // sampleCount
        img.mipmaps > 0 ? img.mipmaps : 1, 
        altdata ? altdata : img.data
    );
    if(altdata)std::free(altdata);
    return ret;
}
