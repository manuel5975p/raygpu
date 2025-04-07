#include "vulkan_internals.hpp"
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
VkBuffer CreateBuffer(WGVKDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkBuffer buffer;
    if (vkCreateBuffer(device->device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to create buffer!");
    
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device->device, buffer, &memReq);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        g_vulkanstate.physicalDevice->physicalDevice, 
        memReq.memoryTypeBits, 
        properties
    );
    
    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to allocate buffer memory!");
    
    vkBindBufferMemory(device->device, buffer, bufferMemory, 0);
    
    return buffer;
}

// Function to create a Vulkan image
WGVKTexture CreateImage(WGVKDevice device, uint32_t width, uint32_t height, uint32_t sampleCount, uint32_t mipLevelCount, VkFormat format, VkImageUsageFlags usage, VkDeviceMemory& imageMemory) {
    WGVKTexture ret = callocnew(WGVKTextureImpl);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = mipLevelCount;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = toVulkanSampleCount(sampleCount);
    
    VkImage image;
    if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to create image!");
    
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device->device, image, &memReq);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        g_vulkanstate.physicalDevice->physicalDevice, 
        memReq.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Failed to allocate image memory!");
        //abort();
    }
    vkBindImageMemory(device->device, image, imageMemory, 0);
    ret->image = image;
    ret->memory = imageMemory;
    ret->device = device;
    ret->width = width;
    ret->height = height;
    ret->format = format;
    ret->sampleCount = sampleCount;
    ret->depthOrArrayLayers = 1;
    ret->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    ret->refCount = 1;
    ret->mipLevels = mipLevelCount;
    return ret;
}

// Function to create an image view
WGVKTextureView CreateImageView(WGVKDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
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
    if (vkCreateImageView(device->device, &viewInfo, nullptr, &ret->view) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to create image view!");
    ret->format = viewInfo.format;
    ret->refCount = 1;
    return ret;
}

// Function to begin a single-use command buffer
VkCommandBuffer transientCommandBuffer{};
VkCommandBuffer BeginSingleTimeCommands(WGVKDevice device, VkCommandPool commandPool) {
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
void EndSingleTimeCommands(WGVKDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer){
    vkEndCommandBuffer(commandBuffer);
}
void EndSingleTimeCommandsAndSubmit(WGVKDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
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


extern "C" void EncodeTransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    WGVKTexture texture
) {
    VkImage image = texture->image;
    // --- 1. Define the Image Memory Barrier ---
    // This structure describes the memory dependency for a specific image subresource range.
    // It tells Vulkan how access to an image needs to be synchronized between operations.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // Layout the image is currently in
    barrier.newLayout = newLayout; // Layout the image needs to be transitioned to

    // --- Queue Family Ownership ---
    // If the image ownership is being transferred between different queue families (e.g., graphics to compute),
    // you would specify the src and dst queue family indices here.
    // VK_QUEUE_FAMILY_IGNORED means we are not transferring ownership, or the transition happens
    // within the same queue family.
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    // --- Image and Subresource Range ---
    barrier.image = image; // The specific image resource to transition

    // Define which part(s) of the image are affected by this barrier.
    // aspectMask: Specifies which aspects (color, depth, stencil) are affected.
    // TODO: A more robust implementation would check the actual VkFormat of the image.
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
        oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || // Check old layout too
        oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL )
    {
         // Check format here if available to see if stencil exists
         // bool hasStencil = (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
         // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Simple default for now
         // If format has stencil, add: | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;   // First mip level to transition
    barrier.subresourceRange.levelCount = 1;     // Number of mip levels to transition
    barrier.subresourceRange.baseArrayLayer = 0; // First array layer to transition
    barrier.subresourceRange.layerCount = 1;     // Number of array layers to transition

    // --- 2. Define Pipeline Stages and Access Masks ---
    // This is the crucial part for performance and correctness. We need to specify:
    // - sourceStageMask: Which pipeline stage(s) must complete *before* the barrier executes.
    //                    This relates to operations using the `oldLayout`.
    // - destinationStageMask: Which pipeline stage(s) must wait *for* the barrier to complete
    //                         before they can start. This relates to operations using the `newLayout`.
    // - srcAccessMask: What kind of memory access (read/write) was performed in the source stages
    //                  that needs to be made available/visible *before* the transition.
    // - dstAccessMask: What kind of memory access (read/write) will be performed in the destination stages
    //                  that needs to wait *for* the transition to complete.

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // --- Determine Source Stage & Access Mask based on oldLayout ---
    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // No need to wait for any stage, as the image content is undefined or discarded.
            // No prior access needs to be synchronized.
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Represents the very start, no actual stage dependency.
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
             // Image data was prepared by the host.
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_HOST_BIT; // Wait for host writes to be visible.
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image was last used for reading in a transfer operation.
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image was last used for writing in a transfer operation (e.g., vkCmdCopyBufferToImage).
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for transfer writes to complete.
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image was last used as a color attachment (written to by fragment shader output).
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait for color writes to complete.
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image was last used as a depth/stencil attachment (read/written by depth/stencil tests).
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // Wait for depth/stencil ops.
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image was last read by a shader.
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            // Which shader stage? Could be vertex, fragment, compute, etc.
            // To be safe, we might sync against all relevant shader stages,
            // or ideally, track the actual usage. Let's use a common one.
            // VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT is common, but compute might also read.
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or VERTEX, COMPUTE, etc.
             // A more robust barrier could use VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT or add VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
             // Image was just presented. Generally, you transition *to* this layout at the end
             // of a frame and the swapchain handles the transition *from* it implicitly when
             // acquiring the next image. If you manually transition *from* it (e.g. for reuse),
             // usually no specific GPU access needs synchronizing from the presentation itself.
             barrier.srcAccessMask = 0; // Or maybe MEMORY_READ if WSI requires it? Safest is 0.
             sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Presentation is implicitly after all commands.
             break;

        case VK_IMAGE_LAYOUT_GENERAL:
             // General layout could have been used for *anything*. This often implies less optimal access.
             // We need a conservative barrier. Assume it could have been written or read from anywhere.
             // This is often a sign that layout tracking could be improved.
             barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Any potential access
             sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // Wait for any possible command
             break;

        default:
            // If we encounter an unhandled layout, assert or log an error.
            // Using a very broad barrier as a fallback is dangerous and inefficient.
            assert(!"Unsupported oldLayout for transition!");
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Fallback guess
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Fallback guess
            break;
    }

    // --- Determine Destination Stage & Access Mask based on newLayout ---
    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used for reading in a transfer operation.
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used for writing in a transfer operation.
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment (written by fragment shader).
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // Might be read for blending too
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Color output stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image will be used as a depth/stencil attachment (read/written by tests).
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // Depth/stencil stages need to wait.
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read by a shader.
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            // Which shader stage will read it? Fragment is common for textures.
            // Compute is common for imageLoad/Store or sampled images.
            // Vertex shaders can also read textures (vertex texture fetch).
            // Let's assume fragment for now, but be aware it could be others.
            destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; 
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            // Image will be presented to the screen.
            // The presentation engine reads the image implicitly after all commands complete.
            barrier.dstAccessMask = 0; // No specific GPU access needs to be made available *for* presentation itself via this barrier. Some WSI layers might implicitly need MEMORY_READ. 0 is usually fine.
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Ensure all commands finish before presentation can potentially start.
            break;

        case VK_IMAGE_LAYOUT_GENERAL:
             // General layout can be used for *anything* next. This might indicate suboptimal planning.
             // Subsequent operations need to wait. Assume any kind of access might happen.
             barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Make memory available for any access
             destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // Any subsequent command might use it.
             break;

        default:
            // If we encounter an unhandled layout, assert or log an error.
            rassert(false, "Unsupported newLayout for transition!");
            barrier.dstAccessMask = 0; // Fallback guess
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Fallback guess
            break;
    }

    // --- 3. Record the Pipeline Barrier Command ---
    // This command injects the dependency into the command buffer.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,        // Pipeline stage(s) before the barrier
        destinationStage,   // Pipeline stage(s) after the barrier
        0,                  // Dependency flags (usually 0, VK_DEPENDENCY_BY_REGION_BIT is an optimization)
        0, nullptr,         // Global memory barriers (not needed here)
        0, nullptr,         // Buffer memory barriers (not needed here)
        1, &barrier         // Image memory barriers (we have one)
    );
}
// Function to transition image layout
extern "C" void TransitionImageLayout(WGVKDevice device, VkCommandPool commandPool, VkQueue queue, WGVKTexture texture, 
                           VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(device, commandPool);
    EncodeTransitionImageLayout(commandBuffer, oldLayout, newLayout, texture);
    
    EndSingleTimeCommandsAndSubmit(device, commandPool, queue, commandBuffer);
}

// Function to copy buffer to image
void CopyBufferToImage(WGVKDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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

// Main function to create Vulkan image from RGBA8 data or as empty
WGVKTexture CreateVkImage(WGVKDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, 
                                                const uint8_t* data, uint32_t width, uint32_t height, uint32_t sampleCount,uint32_t mipLevelCount, VkFormat format, VkImageUsageFlags usage, bool hasData) {
    VkDeviceMemory imageMemory;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    
    
    WGVKTexture image = CreateImage(device, width, height, sampleCount, mipLevelCount, format, usage, imageMemory);
    
    if (hasData && data != nullptr) {
        // Create staging buffer
        VkDeviceSize imageSize = width * height * 4; // Adjust based on format if necessary
        VkDeviceMemory stagingMemory;
        VkBuffer stagingBuffer = CreateBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingMemory);
        
        // Map and copy data to staging buffer
        void* mappedData;
        vkMapMemory(device->device, stagingMemory, 0, imageSize, 0, &mappedData);
        std::memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(device->device, stagingMemory);
        
        // Transition image layout to TRANSFER_DST_OPTIMAL
        TransitionImageLayout(device, commandPool, queue, image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        
        // Copy buffer to image
        CopyBufferToImage(device, commandPool, queue, stagingBuffer, image->image, width, height);
        
        // Transition image layout to SHADER_READ_ONLY_OPTIMAL
        TransitionImageLayout(device, commandPool, queue, image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        // Cleanup staging resources
        vkDestroyBuffer(device->device, stagingBuffer, nullptr);
        vkFreeMemory(device->device, stagingMemory, nullptr);
    }
    
    return image;
}

// Generalized LoadTexturePro function

extern "C" Texture LoadTexturePro_Data(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps, void* data) {
    rassert(mipmaps < MAX_MIP_LEVELS, "Too many mip levels");
    Texture ret{};
    
    VkFormat vkFormat = toVulkanPixelFormat(format);
    VkImageUsageFlags vkUsage = toVulkanTextureUsage(usage, format);
    
    bool hasData = data != nullptr;
    
    WGVKTextureDescriptor tdesc zeroinit;
    tdesc.dimension = TextureDimension_2D;
    tdesc.size = Extent3D{width, height, 1};
    tdesc.format = format;
    tdesc.viewFormatCount = 1;
    tdesc.usage = usage;
    tdesc.sampleCount = sampleCount;
    tdesc.mipLevelCount = mipmaps;
    WGVKTexture image = wgvkDeviceCreateTexture(g_vulkanstate.device, &tdesc);
    if(hasData){
        WGVKTexelCopyTextureInfo destination zeroinit;
        destination.texture = image;
        destination.aspect = is__depth(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        destination.mipLevel = 0;
        destination.origin = WGVKOrigin3D{0, 0, 0};

        WGVKTexelCopyBufferLayout bufferLayout zeroinit;
        bufferLayout.bytesPerRow = width * GetPixelSizeInBytes(format);
        bufferLayout.offset = 0;
        bufferLayout.rowsPerImage = height;
        WGVKExtent3D writeExtent{width, height, 1u};
        wgvkQueueWriteTexture(g_vulkanstate.queue, &destination, data, static_cast<size_t>(width) * static_cast<size_t>(height) * GetPixelSizeInBytes(format), &bufferLayout, &writeExtent);
    }
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
    WGVKTextureViewDescriptor descriptor zeroinit;
    descriptor.format = format;
    descriptor.arrayLayerCount = 1;
    descriptor.baseArrayLayer = 0;
    descriptor.mipLevelCount = mipmaps;
    descriptor.baseMipLevel = 0;
    descriptor.aspect = (format == Depth24 || format == Depth32) ? TextureAspect_DepthOnly : TextureAspect_All;
    descriptor.dimension = TextureViewDimension_2D;
    descriptor.usage = usage;
    WGVKTextureView view = wgvkTextureCreateView(image, &descriptor);
    view->width = width;
    view->height = height;
    view->sampleCount = sampleCount;
    view->depthOrArrayLayers = 1;
    view->texture = image;
    ret.view = view;


    if(mipmaps > 1){
        for(uint32_t i = 0;i < mipmaps;i++){
            WGVKTextureViewDescriptor singleMipDescriptor zeroinit;
            singleMipDescriptor.format = format;
            singleMipDescriptor.arrayLayerCount = 1;
            singleMipDescriptor.baseArrayLayer = 0;
            singleMipDescriptor.mipLevelCount = 1;
            singleMipDescriptor.baseMipLevel = i;
            singleMipDescriptor.aspect = (format == Depth24 || format == Depth32) ? TextureAspect_DepthOnly : TextureAspect_All;
            singleMipDescriptor.dimension = TextureViewDimension_2D;
            singleMipDescriptor.usage = usage;
            ret.mipViews[i] = wgvkTextureCreateView(image, &singleMipDescriptor);
        }
    }

    //TRACELOG(LOG_INFO, "Loaded WGVKTexture and view [%u y %u]", (unsigned)width, (unsigned)height);    
    
    return ret;
}
extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    return LoadTexturePro_Data(width, height, format, usage, sampleCount, mipmaps, nullptr);
}

void UnloadTexture(Texture tex){
    WGVKTextureView view = (WGVKTextureView)tex.view;
    WGVKTexture texture = (WGVKTexture)tex.id;
    if(view != nullptr)
        wgvkReleaseTextureView(view);
    if(texture != nullptr)
        wgvkReleaseTexture(texture);   
}
extern "C" Texture2DArray LoadTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, PixelFormat format){
    Texture2DArray ret zeroinit;
    ret.sampleCount = 1;
    WGVKTextureDescriptor tDesc zeroinit;
    tDesc.format = (format);
    tDesc.size = Extent3D{width, height, layerCount};
    tDesc.dimension = TextureDimension_2D;
    tDesc.sampleCount = 1;
    tDesc.mipLevelCount = 1;
    tDesc.usage = TextureUsage_StorageBinding | TextureUsage_CopySrc | TextureUsage_CopyDst;
    tDesc.viewFormatCount = 1;
    //tDesc.viewFormats = &tDesc.format;
    ret.id = wgvkDeviceCreateTexture((WGVKDevice)g_vulkanstate.device, &tDesc);
    WGVKTextureViewDescriptor vDesc zeroinit;
    vDesc.aspect = TextureAspect_All;
    vDesc.arrayLayerCount = layerCount;
    vDesc.baseArrayLayer = 0;
    vDesc.mipLevelCount = 1;
    vDesc.baseMipLevel = 0;
    vDesc.dimension = TextureViewDimension_2DArray;
    vDesc.format = tDesc.format;
    vDesc.usage = tDesc.usage;
    ret.view = wgvkTextureCreateView((WGVKTexture)ret.id, &vDesc);
    ret.width = width;
    ret.height = height;
    ret.layerCount = layerCount;
    ret.format = format;
    return ret;
}
extern "C" Image LoadImageFromTextureEx(WGVKTexture tex, uint32_t mipLevel){
    static VkCommandPool transientPool = [](){
        VkCommandPool ret = nullptr;
        VkCommandPoolCreateInfo pci zeroinit;
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        vkCreateCommandPool(g_vulkanstate.device->device, &pci, nullptr, &ret);
        return ret;
    }();
    static VkFence fence = CreateFence();
    Image ret zeroinit;
    VkBufferImageCopy region{};

    size_t size = GetPixelSizeInBytes(fromVulkanPixelFormat(tex->format));
    region.bufferOffset = 0;
    region.bufferRowLength = 0;//size * tex->width; // Tightly packed
    region.bufferImageHeight = 0;//tex->height;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = mipLevel;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    //TRACELOG(LOG_INFO, "Copying image with extent %u x %u", tex->width, tex->height);
    region.imageExtent = VkExtent3D{tex->width, tex->height, 1u};
    VkDeviceMemory bufferMemory{};
    size_t bufferSize = size * tex->width * tex->height;
    VkBuffer stagingBuffer = CreateBuffer(g_vulkanstate.device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, bufferMemory);
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(g_vulkanstate.device, transientPool);
    VkImageLayout oldLayout = tex->layout;
    if(oldLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        EncodeTransitionImageLayout(commandBuffer, tex->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex);
    
    vkCmdCopyImageToBuffer(
        commandBuffer,
        tex->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        stagingBuffer,
        1,
        &region
    );
    if(oldLayout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        EncodeTransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, oldLayout, tex);

    EndSingleTimeCommands(g_vulkanstate.device, transientPool, commandBuffer);
    VkSubmitInfo sinfo zeroinit;
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &commandBuffer;
    VkResult submitResult = vkQueueSubmit(g_vulkanstate.queue->graphicsQueue, 1, &sinfo, fence);
    if(submitResult == VK_SUCCESS){
        VkResult fenceWait = vkWaitForFences(g_vulkanstate.device->device, 1, &fence, VK_TRUE, UINT64_MAX);
        //if(fenceWait != VK_SUCCESS){
        //    TRACELOG(LOG_ERROR, "Waiting for fence not successful");
        //}
        //else{
        //    TRACELOG(LOG_INFO, "Successfully waited for fence");
        //}
        vkResetFences(g_vulkanstate.device->device, 1, &fence);

        vkFreeCommandBuffers(g_vulkanstate.device->device, transientPool, 1, &commandBuffer);
        void* mapPtr = nullptr;
        VkResult mapResult = vkMapMemory(g_vulkanstate.device->device, bufferMemory, 0, size * tex->width * tex->height, 0, &mapPtr);
        if(mapResult == VK_SUCCESS){
            ret.data = std::calloc(bufferSize, 1);
            ret.width = tex->width;
            ret.height = tex->height;
            ret.format = fromVulkanPixelFormat(tex->format);
            ret.mipmaps = 0;
            ret.rowStrideInBytes = ret.width * size;
            std::memcpy(ret.data, mapPtr, bufferSize);
        }
        else{
            TRACELOG(LOG_ERROR, "vkMapMemory failed with errorcode %d", mapResult);
        }
    }
    vkUnmapMemory(g_vulkanstate.device->device, bufferMemory);
    vkFreeMemory(g_vulkanstate.device->device, bufferMemory, nullptr);
    vkDestroyBuffer(g_vulkanstate.device->device, stagingBuffer, nullptr);
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
    TextureUsage x;
    rassert(img.mipmaps < MAX_MIP_LEVELS, "Too many mip levels");
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
    TRACELOG(LOG_INFO, "Successfully loaded %u x %u texture from image", (unsigned)img.width, (unsigned)img.height);
    return ret;
}
Texture3D LoadTexture3DPro(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, TextureUsage usage, uint32_t sampleCount){
    Texture3D ret zeroinit;
    WGVKTextureDescriptor tDesc{};
    tDesc.dimension = TextureDimension_3D;
    tDesc.size = Extent3D{width, height, depth};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = sampleCount;
    tDesc.usage = TextureUsage_StorageBinding | TextureUsage_TextureBinding | TextureUsage_CopySrc | TextureUsage_CopyDst;
    tDesc.format = format;
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.id = wgvkDeviceCreateTexture(g_vulkanstate.device, &tDesc);
    
    WGVKTextureViewDescriptor vDesc zeroinit;
    vDesc.arrayLayerCount = 1;
    vDesc.baseArrayLayer = 0;
    vDesc.mipLevelCount = 1;
    vDesc.baseMipLevel = 0;
    vDesc.aspect = TextureAspect_All;
    vDesc.dimension = TextureViewDimension_3D;
    vDesc.usage = TextureUsage_StorageBinding | TextureUsage_TextureBinding | TextureUsage_CopySrc | TextureUsage_CopyDst;
    
    ret.view = wgvkTextureCreateView((WGVKTexture)ret.id, &vDesc);
    ret.sampleCount = sampleCount;
    return ret;
}