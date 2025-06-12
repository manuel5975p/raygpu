#include "vulkan_internals.hpp"
#include <wgvk_structs_impl.h>
#include <raygpu.h>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan_core.h>


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
    
    TRACELOG(LOG_FATAL, "Failed to find suitable memory type!");
    return 0;
}

// Function to create a Vulkan buffer


// Function to begin a single-use command buffer
VkCommandBuffer transientCommandBuffer{};
VkCommandBuffer BeginSingleTimeCommands(WGPUDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    
    //if(!transientCommandBuffer){
        vkAllocateCommandBuffers(device->device, &allocInfo, &transientCommandBuffer);
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
void EndSingleTimeCommands(WGPUDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer){
    vkEndCommandBuffer(commandBuffer);
}
void EndSingleTimeCommandsAndSubmit(WGPUDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
    EndSingleTimeCommands(device, commandPool, commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to submit command buffer!");
    
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device->device, commandPool, 1, &commandBuffer);
    commandBuffer = nullptr;
}
VkImageAspectFlags GetAspectMask(VkImageLayout layout, VkFormat format /* Add format here */) {
    if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
        layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
        // Ideally, check format here to see if it has stencil
        // if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
        //     return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        // } else {
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        // }
    } else {
        return VK_IMAGE_ASPECT_COLOR_BIT;
    }
    // TODO: Handle separate depth/stencil layouts if used (e.g., VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
}



// Function to transition image layout
//extern "C" void TransitionImageLayout(WGPUDevice device, VkCommandPool commandPool, VkQueue queue, WGPUTexture texture, 
//                           VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
//    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
//    EncodeTransitionImageLayout(commandBuffer, oldLayout, newLayout, texture);
//    
//    EndSingleTimeCommandsAndSubmit(device, commandPool, queue, commandBuffer);
//}

// Function to copy buffer to image
void CopyBufferToImage(WGPUDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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
    
    EndSingleTimeCommandsAndSubmit(device, commandPool, queue, commandBuffer);
}

inline bool is__depth(PixelFormat fmt){
    return fmt ==  Depth24 || fmt == Depth32;
}
inline bool is__depth(VkFormat fmt){
    return fmt ==  VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT;
}

// Generalized LoadTexturePro function

extern "C" Texture LoadTexturePro_Data(uint32_t width, uint32_t height, PixelFormat format_, WGPUTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps, void* data) {
    rassert(mipmaps < MAX_MIP_LEVELS, "Too many mip levels");
    Texture ret{};
    WGPUTextureFormat format = toWGPUPixelFormat(format_);
    VkFormat vkFormat = toVulkanPixelFormat(format);
    VkImageUsageFlags vkUsage = toVulkanTextureUsage(usage, format);
    
    bool hasData = data != nullptr;
    
    WGPUTextureDescriptor tdesc zeroinit;
    tdesc.dimension = TextureDimension_2D;
    tdesc.size = Extent3D{width, height, 1};
    tdesc.format = format;
    tdesc.viewFormatCount = 1;
    tdesc.usage = usage;
    tdesc.sampleCount = sampleCount;
    tdesc.mipLevelCount = mipmaps;
    WGPUTexture image = wgpuDeviceCreateTexture(g_vulkanstate.device, &tdesc);
    if(hasData){
        WGPUTexelCopyTextureInfo destination zeroinit;
        destination.texture = image;
        destination.aspect = WGPUTextureAspect_All;
        destination.mipLevel = 0;
        destination.origin = WGPUOrigin3D{0, 0, 0};

        WGPUTexelCopyBufferLayout bufferLayout zeroinit;
        bufferLayout.bytesPerRow = width * GetPixelSizeInBytes(format_);
        bufferLayout.offset = 0;
        bufferLayout.rowsPerImage = height;
        WGPUExtent3D writeExtent{width, height, 1u};
        wgpuQueueWriteTexture(
            g_vulkanstate.queue,
            &destination,
            data,
            static_cast<size_t>(width) * static_cast<size_t>(height) * GetPixelSizeInBytes(format_),
            &bufferLayout,
            &writeExtent
        );
    }
    ret.width = width;
    ret.height = height;
    ret.mipmaps = mipmaps;
    ret.sampleCount = sampleCount;
    ret.id = image;
    ret.format = format_;
    
    VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if (format_ == Depth24 || format_ == Depth32) {
        aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    WGPUTextureViewDescriptor descriptor zeroinit;
    descriptor.format = format;
    descriptor.arrayLayerCount = 1;
    descriptor.baseArrayLayer = 0;
    descriptor.mipLevelCount = mipmaps;
    descriptor.baseMipLevel = 0;
    descriptor.aspect = (format_ == Depth24 || format_ == Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
    descriptor.dimension = WGPUTextureViewDimension_2D;
    descriptor.usage = usage;
    WGPUTextureView view = wgpuTextureCreateView(image, &descriptor);
    view->width = width;
    view->height = height;
    view->sampleCount = sampleCount;
    view->depthOrArrayLayers = 1;
    view->texture = image;
    ret.view = view;


    if(mipmaps > 1){
        for(uint32_t i = 0;i < mipmaps;i++){
            WGPUTextureViewDescriptor singleMipDescriptor zeroinit;
            singleMipDescriptor.format = format;
            singleMipDescriptor.arrayLayerCount = 1;
            singleMipDescriptor.baseArrayLayer = 0;
            singleMipDescriptor.mipLevelCount = 1;
            singleMipDescriptor.baseMipLevel = i;
            singleMipDescriptor.aspect = (format_ == Depth24 || format_ == Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All;
            singleMipDescriptor.dimension = WGPUTextureViewDimension_2D;
            singleMipDescriptor.usage = usage;
            ret.mipViews[i] = wgpuTextureCreateView(image, &singleMipDescriptor);
        }
    }

    //TRACELOG(LOG_INFO, "Loaded WGPUTexture and view [%u y %u]", (unsigned)width, (unsigned)height);    
    
    return ret;
}
extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, WGPUTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    return LoadTexturePro_Data(width, height, format, usage, sampleCount, mipmaps, nullptr);
}

void UnloadTexture(Texture tex){
    WGPUTextureView view = (WGPUTextureView)tex.view;
    WGPUTexture texture = (WGPUTexture)tex.id;
    if(view != nullptr)
        wgpuTextureViewRelease(view);
    if(texture != nullptr)
        wgpuTextureRelease(texture);   
}
extern "C" Texture2DArray LoadTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, PixelFormat format_){
    Texture2DArray ret zeroinit;
    ret.sampleCount = 1;
    WGPUTextureDescriptor tDesc zeroinit;
    tDesc.format = toWGPUPixelFormat(format_);
    tDesc.size = Extent3D{width, height, layerCount};
    tDesc.dimension = TextureDimension_2D;
    tDesc.sampleCount = 1;
    tDesc.mipLevelCount = 1;
    tDesc.usage = WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
    tDesc.viewFormatCount = 1;
    //tDesc.viewFormats = &tDesc.format;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)g_vulkanstate.device, &tDesc);
    WGPUTextureViewDescriptor vDesc zeroinit;
    vDesc.aspect = WGPUTextureAspect_All;
    vDesc.arrayLayerCount = layerCount;
    vDesc.baseArrayLayer = 0;
    vDesc.mipLevelCount = 1;
    vDesc.baseMipLevel = 0;
    vDesc.dimension = WGPUTextureViewDimension_2DArray;
    vDesc.format = tDesc.format;
    vDesc.usage = tDesc.usage;
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vDesc);
    ret.width = width;
    ret.height = height;
    ret.layerCount = layerCount;
    ret.format = format_;
    return ret;
}
extern "C" Image LoadImageFromTextureEx(WGPUTexture tex, uint32_t mipLevel){
    //static VkCommandPool transientPool = [](){
    //    VkCommandPool ret = nullptr;
    //    VkCommandPoolCreateInfo pci zeroinit;
    //    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //    pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //    vkCreateCommandPool(g_vulkanstate.device->device, &pci, nullptr, &ret);
    //    return ret;
    //}();
    //VkFenceCreateInfo fci = {
    //    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    //    .pNext = NULL, 
    //    .flags = 0
    //};
    //static VkFence fence = VK_NULL_HANDLE;
    //if(fence == VK_NULL_HANDLE){
    //    vkCreateFence(g_vulkanstate.device->device, &fci, nullptr, &fence);
    //}
    Image ret zeroinit;

    const size_t size = GetPixelSizeInBytes(fromWGPUPixelFormat(fromVulkanPixelFormat(tex->format)));
    const size_t bufferSize = size * tex->width * tex->height;
    WGPUBufferDescriptor bdesc = {
        .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapRead,
        .size = bufferSize 
    };
    WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(g_vulkanstate.device, &bdesc);
    WGPUCommandEncoder commandEncoder = wgpuDeviceCreateCommandEncoder(g_vulkanstate.device, NULL);
    VkImageLayout oldLayout = tex->layout;
    //if(oldLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    //    EncodeTransitionImageLayout(commandBuffer, tex->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex);
    WGPUTexelCopyTextureInfo source = {
        .texture = tex,
        .mipLevel = 0,
        .origin = {
            .x = 0,
            .y = 0,
            .z = 0
        },
        .aspect = WGPUTextureAspect_All,
    };
    WGPUTexelCopyBufferInfo destination = {
        .layout = {
            .offset = 0,
            .bytesPerRow = (uint32_t)(tex->width * size),
            .rowsPerImage = tex->height
        },
        .buffer = stagingBuffer
    };

    WGPUExtent3D copySize{
        tex->width,
        tex->height,
        tex->depthOrArrayLayers
    };

    wgpuCommandEncoderCopyTextureToBuffer(commandEncoder, &source, &destination, &copySize);
    WGPUCommandBuffer cbuffer = wgpuCommandEncoderFinish(commandEncoder);
    g_vulkanstate.device->functions.vkQueueWaitIdle (g_vulkanstate.queue->graphicsQueue);
    g_vulkanstate.device->functions.vkDeviceWaitIdle(g_vulkanstate.device->device);
    wgpuQueueSubmit(g_vulkanstate.queue, 1, &cbuffer);
    g_vulkanstate.device->functions.vkQueueWaitIdle (g_vulkanstate.queue->graphicsQueue);
    g_vulkanstate.device->functions.vkDeviceWaitIdle(g_vulkanstate.device->device);
    void* mapPtr = nullptr;
    wgpuBufferMap(stagingBuffer, WGPUMapMode_Read, 0, bdesc.size, &mapPtr);
    g_vulkanstate.device->functions.vkQueueWaitIdle (g_vulkanstate.queue->graphicsQueue);
    g_vulkanstate.device->functions.vkDeviceWaitIdle(g_vulkanstate.device->device);
    const BGRA8Color* colors = (BGRA8Color*)mapPtr;
    if(mapPtr){
        ret.data = std::calloc(bufferSize, 1);
        ret.width = tex->width;
        ret.height = tex->height;
        ret.format = fromWGPUPixelFormat(fromVulkanPixelFormat(tex->format));
        ret.mipmaps = 0;
        ret.rowStrideInBytes = ret.width * size;
        std::memcpy(ret.data, mapPtr, bufferSize);
        wgpuBufferUnmap(stagingBuffer);
    }

    //if(submitResult == VK_SUCCESS){
    //    VkResult fenceWait = vkWaitForFences(g_vulkanstate.device->device, 1, &fence, VK_TRUE, UINT64_MAX);
    //    //if(fenceWait != VK_SUCCESS){
    //    //    TRACELOG(LOG_ERROR, "Waiting for fence not successful");
    //    //}
    //    //else{
    //    //    TRACELOG(LOG_INFO, "Successfully waited for fence");
    //    //}
    //    vkResetFences(g_vulkanstate.device->device, 1, &fence);
    //    vkFreeCommandBuffers(g_vulkanstate.device->device, transientPool, 1, &commandEncoder);
    //    void* mapPtr = nullptr;
    //    VkResult mapResult = vkMapMemory(g_vulkanstate.device->device, bufferMemory, 0, size * tex->width * tex->height, 0, &mapPtr);
    //    if(mapResult == VK_SUCCESS){
    //        ret.data = std::calloc(bufferSize, 1);
    //        ret.width = tex->width;
    //        ret.height = tex->height;
    //        ret.format = fromWGPUPixelFormat(fromVulkanPixelFormat(tex->format));
    //        ret.mipmaps = 0;
    //        ret.rowStrideInBytes = ret.width * size;
    //        std::memcpy(ret.data, mapPtr, bufferSize);
    //    }
    //    else{
    //        TRACELOG(LOG_ERROR, "vkMapMemory failed with errorcode %d", mapResult);
    //    }
    //}
    //vkUnmapMemory(g_vulkanstate.device->device, bufferMemory);
    //vkFreeMemory(g_vulkanstate.device->device, bufferMemory, nullptr);
    wgpuBufferRelease(stagingBuffer);
    return ret;
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
        TRACELOG(LOG_FATAL, "Unsupported image format.");
    }
    rassert(img.mipmaps < MAX_MIP_LEVELS, "Too many mip levels");
    auto ret = LoadTexturePro_Data(
        img.width,
        img.height,
        img.format == GRAYSCALE ? RGBA8 : img.format,
        WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding, // Assuming TextureUsage enum exists and has a Sampled option
        1, // sampleCount
        img.mipmaps > 0 ? img.mipmaps : 1, 
        altdata ? altdata : img.data
    );
    if(altdata)std::free(altdata);
    TRACELOG(LOG_INFO, "Successfully loaded %u x %u texture from image", (unsigned)img.width, (unsigned)img.height);
    return ret;
}
Texture3D LoadTexture3DPro(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format_, WGPUTextureUsage usage, uint32_t sampleCount){
    Texture3D ret zeroinit;
    WGPUTextureFormat format = toWGPUPixelFormat(format_);
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = TextureDimension_3D;
    tDesc.size = Extent3D{width, height, depth};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = sampleCount;
    tDesc.usage = WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
    tDesc.format = format;
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.id = wgpuDeviceCreateTexture(g_vulkanstate.device, &tDesc);
    
    WGPUTextureViewDescriptor vDesc zeroinit;
    vDesc.arrayLayerCount = 1;
    vDesc.baseArrayLayer = 0;
    vDesc.mipLevelCount = 1;
    vDesc.baseMipLevel = 0;
    vDesc.aspect = WGPUTextureAspect_All;
    vDesc.dimension = WGPUTextureViewDimension_3D;
    vDesc.usage = WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst;
    
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vDesc);
    ret.sampleCount = sampleCount;
    return ret;
}