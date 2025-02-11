#include "vulkan_internals.hpp"

extern "C" WGVKBuffer wgvkDeviceCreateBuffer(VkDevice device, const BufferDescriptor* desc){
    WGVKBuffer wgvkBuffer = callocnewpp(WGVKBufferImpl);
    wgvkBuffer->refCount = 1;

    VkBufferCreateInfo bufferDesc zeroinit;
    bufferDesc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferDesc.size = desc->size;
    bufferDesc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferDesc.usage = toVulkanBufferUsage(desc->usage);
    
    VkResult bufferCreateResult = vkCreateBuffer((VkDevice)GetDevice(), &bufferDesc, nullptr, &wgvkBuffer->buffer);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, wgvkBuffer->buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &wgvkBuffer->memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device, wgvkBuffer->buffer, wgvkBuffer->memory, 0);
    return wgvkBuffer;
}

extern "C" void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, void const * data, size_t size){
    void* mappedMemory = nullptr;

    VkResult result = vkMapMemory(g_vulkanstate.device, buffer->memory, bufferOffset, size, 0, &mappedMemory);
    
    if (result == VK_SUCCESS && mappedMemory != nullptr) {
        // Memory is host mappable: copy data and unmap.
        std::memcpy(mappedMemory, data, size);
        vkUnmapMemory(g_vulkanstate.device, buffer->memory);
        return;
    }
    assert(false && "Not yet implemented");
    
    abort();
}
extern "C" void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, VkPhysicalDevice adapter, SurfaceCapabilities* capabilities){
    VkSurfaceKHR surface = wgvkSurface->surface;
    VkSurfaceCapabilitiesKHR scap{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &scap);

    // Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        wgvkSurface->formatCache = (PixelFormat*)std::calloc(formatCount, sizeof(PixelFormat));
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, surfaceFormats.data());
        for(size_t i = 0;i < formatCount;i++){
            wgvkSurface->formatCache[i] = fromVulkanPixelFormat(surfaceFormats[i].format);
        }
        wgvkSurface->formatCount = formatCount;
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        wgvkSurface->presentModeCache = (SurfacePresentMode*)std::calloc(presentModeCount, sizeof(SurfacePresentMode));
        std::vector<VkPresentModeKHR> presentModes(formatCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, presentModes.data());
        for(size_t i = 0;i < presentModeCount;i++){
            wgvkSurface->presentModeCache[i] = fromVulkanPresentMode(presentModes[i]);
        }
        wgvkSurface->presentModeCount = presentModeCount;
    }
    capabilities->presentModeCount = wgvkSurface->presentModeCount;
    capabilities->formatCount = wgvkSurface->formatCount;
    capabilities->presentModes = wgvkSurface->presentModeCache;
    capabilities->formats = wgvkSurface->formatCache;
    capabilities->usages = fromVulkanTextureUsage(scap.supportedUsageFlags);
}

void wgvkSurfaceConfigure(WGVKSurfaceImpl* surface, const SurfaceConfiguration* config){
    auto& device = g_vulkanstate.device;
    vkDeviceWaitIdle(device);
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        vkDestroyImageView(device, surface->imageViews[i], nullptr);
        //vkDestroyImage(device, surface->images[i], nullptr);
    }
    std::free(surface->framebuffers);
    std::free(surface->imageViews);
    std::free(surface->images);
    vkDestroySwapchainKHR(device, surface->swapchain, nullptr);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, surface->surface);
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;

    createInfo.minImageCount = 1;
    createInfo.imageFormat = toVulkanPixelFormat(config->format);//swapchainImageFormat;
    surface->width = config->width;
    surface->height = config->height;
    VkExtent2D newExtent{config->width, config->height};
    createInfo.imageExtent = newExtent;
    createInfo.imageArrayLayers = 1; // For stereoscopic 3D applications
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue family indices
    uint32_t queueFamilyIndices[] = {g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily};

    if (g_vulkanstate.graphicsFamily != g_vulkanstate.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = toVulkanPresentMode(config->presentMode); 
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(g_vulkanstate.device, &createInfo, nullptr, &(surface->swapchain)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    } else {
        std::cout << "Successfully created swap chain\n";
    }

    vkGetSwapchainImagesKHR(g_vulkanstate.device, surface->swapchain, &surface->imagecount, nullptr);
    surface->images = (VkImage*)std::calloc(surface->imagecount, sizeof(VkImage));
    surface->imageViews = (VkImageView*)std::calloc(surface->imagecount, sizeof(VkImageView));

    vkGetSwapchainImagesKHR(g_vulkanstate.device, surface->swapchain, &surface->imagecount, surface->images);

    surface->imageViews = (VkImageView*)std::calloc(surface->imagecount, sizeof(VkImageView));
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = surface->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = toVulkanPixelFormat(BGRA8);
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &surface->imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

}
void wgvkReleaseCommandBuffer(CommmandBufferHandle commandBuffer) { vkFreeCommandBuffers(g_vulkanstate.device, commandBuffer->pool, 1, &commandBuffer->buffer); }
void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc) {
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
void wgvkReleaseBuffer(WGVKBuffer buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vkDestroyBuffer(g_vulkanstate.device, buffer->buffer, nullptr);
        vkFreeMemory(g_vulkanstate.device, buffer->memory, nullptr);
        std::free(buffer);
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
void wgvkRenderpassEncoderDraw(WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Record the draw command into the command buffer
    vkCmdDraw(rpe->cmdBuffer, vertices, instances, firstvertex, firstinstance);
}

// Implementation of RenderpassEncoderDrawIndexed
void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Assuming vertexOffset is 0. Modify if you have a different offset.
    int32_t vertexOffset = 0;

    // Record the indexed draw command into the command buffer
    vkCmdDrawIndexed(rpe->cmdBuffer, indices, instances, firstindex, vertexOffset, firstinstance);
}
void wgvkRenderPassEncoderBindPipeline(WGVKRenderPassEncoder rpe, DescribedPipeline *pipeline) { 
    rpe->lastLayout = (VkPipelineLayout)pipeline->layout.layout; 
    vkCmdBindPipeline(rpe->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pipeline->quartet.pipeline_TriangleList);
}

void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, VkPipeline pipeline, VkPipelineLayout layout) { 
    rpe->lastLayout = layout; 
    vkCmdBindPipeline(rpe->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)pipeline);
}

// Implementation of RenderPassDescriptorBindDescriptorSet
void wgvkRenderPassEncoderBindDescriptorSet(WGVKRenderPassEncoder rpe, uint32_t group, DescriptorSetHandle dset) {
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
void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the vertex buffer to the command buffer at the specified binding point
    vkCmdBindVertexBuffers(rpe->cmdBuffer, binding, 1, &(buffer->buffer), &offset);

    if(!rpe->referencedBuffers.contains(buffer)){
        ++buffer->refCount;
        rpe->referencedBuffers.insert(buffer);
    }
}
void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the index buffer to the command buffer
    vkCmdBindIndexBuffer(rpe->cmdBuffer, buffer->buffer, offset, indexType);

    if(!rpe->referencedBuffers.contains(buffer)){
        ++buffer->refCount;
        rpe->referencedBuffers.insert(buffer);
    }
}
