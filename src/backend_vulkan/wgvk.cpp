#include "vulkan_internals.hpp"

extern "C" WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const BufferDescriptor* desc){
    WGVKBuffer wgvkBuffer = callocnewpp(WGVKBufferImpl);
    wgvkBuffer->refCount = 1;

    VkBufferCreateInfo bufferDesc zeroinit;
    bufferDesc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferDesc.size = desc->size;
    bufferDesc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferDesc.usage = toVulkanBufferUsage(desc->usage);
    
    VkResult bufferCreateResult = vkCreateBuffer(device->device, &bufferDesc, nullptr, &wgvkBuffer->buffer);
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device->device, wgvkBuffer->buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);


    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &wgvkBuffer->memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(device->device, wgvkBuffer->buffer, wgvkBuffer->memory, 0);
    return wgvkBuffer;
}

extern "C" void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, void const * data, size_t size){
    void* mappedMemory = nullptr;

    VkResult result = vkMapMemory(g_vulkanstate.device->device, buffer->memory, bufferOffset, size, 0, &mappedMemory);
    
    if (result == VK_SUCCESS && mappedMemory != nullptr) {
        // Memory is host mappable: copy data and unmap.
        std::memcpy(mappedMemory, data, size);
        vkUnmapMemory(g_vulkanstate.device->device, buffer->memory);
        return;
    }
    assert(false && "Not yet implemented");
    
    abort();
}



extern "C" void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup wvBindGroup, const WGVKBindGroupDescriptor* bgdesc){
    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets = 1;
    
    std::unordered_map<VkDescriptorType, uint32_t> counts;
    for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
        ++counts[toVulkanResourceType(bgdesc->layout->entries[i].type)];
    }
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(counts.size());
    for(const auto& [t, s] : counts){
        sizes.push_back(VkDescriptorPoolSize{.type = t, .descriptorCount = s});
    }

    dpci.poolSizeCount = sizes.size();
    dpci.pPoolSizes = sizes.data();
    dpci.maxSets = 1;
    vkCreateDescriptorPool(device->device, &dpci, nullptr, &wvBindGroup->pool);
    
    //VkCopyDescriptorSet copy{};
    //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    
    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = wvBindGroup->pool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = (VkDescriptorSetLayout*)&bgdesc->layout->layout;
    vkAllocateDescriptorSets(device->device, &dsai, &wvBindGroup->set);



    uint32_t count = bgdesc->entryCount;
    small_vector<VkWriteDescriptorSet> writes(count, VkWriteDescriptorSet{});
    small_vector<VkDescriptorBufferInfo> bufferInfos(count, VkDescriptorBufferInfo{});
    small_vector<VkDescriptorImageInfo> imageInfos(count, VkDescriptorImageInfo{});

    for(uint32_t i = 0;i < count;i++){
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uint32_t binding = bgdesc->entries[i].binding;
        writes[i].dstBinding = binding;
        writes[i].dstSet = wvBindGroup->set;
        writes[i].descriptorType = toVulkanResourceType(bgdesc->layout->entries[i].type);
        writes[i].descriptorCount = 1;

        if(bgdesc->layout->entries[i].type == uniform_buffer || bgdesc->layout->entries[i].type == storage_buffer){
            wvBindGroup->resourceUsage.track((WGVKBuffer)bgdesc->entries[i].buffer);
            bufferInfos[i].buffer = ((WGVKBuffer)bgdesc->entries[i].buffer)->buffer;
            bufferInfos[i].offset = bgdesc->entries[i].offset;
            bufferInfos[i].range =  bgdesc->entries[i].size;
            writes[i].pBufferInfo = bufferInfos.data() + i;
        }

        if(bgdesc->layout->entries[i].type == texture2d || bgdesc->layout->entries[i].type == texture3d){
            wvBindGroup->resourceUsage.track((WGVKTextureView)bgdesc->entries[i].textureView, TextureUsage_TextureBinding);
            imageInfos[i].imageView = ((WGVKTextureView)bgdesc->entries[i].textureView)->view;
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            writes[i].pImageInfo = imageInfos.data() + i;
        }

        if(bgdesc->layout->entries[i].type == texture_sampler){
            VkSampler vksampler = (VkSampler)bgdesc->entries[i].sampler;
            imageInfos[i].sampler = vksampler;
            writes[i].pImageInfo = imageInfos.data() + i;
        }
    }
    vkUpdateDescriptorSets(device->device, writes.size(), writes.data(), 0, nullptr);
}



extern "C" WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc){

    WGVKBindGroup ret = callocnewpp(WGVKBindGroupImpl);
    ret->refCount = 1;
    wgvkWriteBindGroup(device, ret, bgdesc);
    
    
    return ret;
}

extern "C" WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* desc){
    WGVKCommandEncoder ret = callocnewpp(WGVKCommandEncoderImpl);
    VkCommandPoolCreateInfo pci{};
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = desc->recyclable ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    ret->recyclable = desc->recyclable;
    vkCreateCommandPool(device->device, &pci, nullptr, &ret->pool);
    VkCommandBufferAllocateInfo bai{};
    bai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bai.commandPool = ret->pool;
    bai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bai.commandBufferCount = 1;
    vkAllocateCommandBuffers(device->device, &bai, &ret->buffer);

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(ret->buffer, &bbi);
    return ret;
}
extern "C" void wgvkQueueTransitionLayout(WGVKQueue cSelf, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    EncodeTransitionImageLayout(cSelf->presubmitCache->buffer, from, to, texture);
    cSelf->presubmitCache->resourceUsage.track(texture);
}
extern "C" WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor){
    
    VkImageViewCreateInfo ivci{};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = texture->image;

    VkComponentMapping cm{
        .r = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .g = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .b = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .a = VK_COMPONENT_SWIZZLE_IDENTITY
    };
    ivci.components = cm;
    ivci.viewType = toVulkanTextureViewDimension(descriptor->dimension);
    ivci.format = toVulkanPixelFormat(descriptor->format);
    
    VkImageSubresourceRange sr{
        .aspectMask = toVulkanAspectMask(descriptor->aspect),
        .baseMipLevel = descriptor->baseMipLevel,
        .levelCount = descriptor->mipLevelCount,
        .baseArrayLayer = descriptor->baseArrayLayer,
        .layerCount = descriptor->arrayLayerCount
    };
    if(!is__depth(ivci.format)){
        sr.aspectMask &= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    ivci.subresourceRange = sr;
    WGVKTextureView ret = callocnew(WGVKTextureViewImpl);
    ret->refCount = 1;
    vkCreateImageView(texture->device->device, &ivci, nullptr, &ret->view);
    ret->format = ivci.format;
    ret->texture = texture;
    ++texture->refCount;
    ret->width = texture->width;
    ret->height = texture->height;
    ret->sampleCount = texture->sampleCount;
    ret->depthOrArrayLayers = texture->depthOrArrayLayers;
    return ret;
}
extern "C" WGVKRenderPassEncoder wgvkCommandEncoderBeginRenderPass(WGVKCommandEncoder enc, const WGVKRenderPassDescriptor* rpdesc){
    WGVKRenderPassEncoder ret = callocnewpp(WGVKRenderPassEncoderImpl);
    //One for WGVKRenderPassEncoder the other for the command buffer
    ret->refCount = 2;
    enc->referencedRPs.insert(ret);

    RenderPassLayout rplayout = GetRenderPassLayout(rpdesc);
    VkRenderPassBeginInfo rpbi{};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //TODO DAAAAAAAAAAAAMN device

    ret->renderPass = LoadRenderPassFromLayout(g_vulkanstate.device, rplayout);

    VkImageView attachmentViews[max_color_attachments + 2];
    
    for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
        attachmentViews[i] = rpdesc->colorAttachments[i].view->view;
    }
    attachmentViews[rplayout.colorAttachmentCount] = rpdesc->depthStencilAttachment->view->view;
    if(rpdesc->colorAttachments[0].resolveTarget)
        attachmentViews[rplayout.colorAttachmentCount+1] = rpdesc->colorAttachments[0].resolveTarget->view;

    VkFramebufferCreateInfo fbci zeroinit;
    fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbci.attachmentCount = rplayout.colorAttachmentCount + rplayout.depthAttachmentPresent + uint32_t(rplayout.colorResolveIndex != VK_ATTACHMENT_UNUSED);

    if(rpdesc->colorAttachments[0].view){
        ret->resourceUsage.track(rpdesc->colorAttachments[0].view, TextureUsage_RenderAttachment);
    }
    if(rpdesc->colorAttachments[0].resolveTarget){
        ret->resourceUsage.track(rpdesc->colorAttachments[0].resolveTarget, TextureUsage_RenderAttachment);
    }
    if(rpdesc->depthStencilAttachment->view){
        ret->resourceUsage.track(rpdesc->depthStencilAttachment->view, TextureUsage_RenderAttachment);
    }

    fbci.width = rpdesc->colorAttachments[0].view->width;
    fbci.height = rpdesc->colorAttachments[0].view->height;
    fbci.layers = 1;
    fbci.pAttachments = attachmentViews;
    fbci.renderPass = ret->renderPass;
    VkResult fbresult = vkCreateFramebuffer(g_vulkanstate.device->device, &fbci, nullptr, &ret->frameBuffer);
    if(fbresult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating framebuffer: %d", (int)fbresult);
    }
    rpbi.renderPass = ret->renderPass;
    rpbi.renderArea = VkRect2D{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{rpdesc->colorAttachments[0].view->width, rpdesc->colorAttachments[0].view->height}
    };

    

    rpbi.framebuffer = ret->frameBuffer;
    VkClearValue clearValues[max_color_attachments + 2];
    for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
        clearValues[i].color.float32[0] = rpdesc->colorAttachments[i].clearValue.r;
        clearValues[i].color.float32[1] = rpdesc->colorAttachments[i].clearValue.g;
        clearValues[i].color.float32[2] = rpdesc->colorAttachments[i].clearValue.b;
        clearValues[i].color.float32[3] = rpdesc->colorAttachments[i].clearValue.a;
    }
    if(rplayout.colorResolveIndex != VK_ATTACHMENT_UNUSED){
        clearValues[2].color.float32[0] = rpdesc->colorAttachments[0].clearValue.r;
        clearValues[2].color.float32[1] = rpdesc->colorAttachments[0].clearValue.g;
        clearValues[2].color.float32[2] = rpdesc->colorAttachments[0].clearValue.b;
        clearValues[2].color.float32[3] = rpdesc->colorAttachments[0].clearValue.a;
    }
    clearValues[rplayout.colorAttachmentCount].depthStencil.depth = rpdesc->depthStencilAttachment->depthClearValue;
    rpbi.clearValueCount = rplayout.colorAttachmentCount + rplayout.depthAttachmentPresent + (rplayout.colorResolveIndex != VK_ATTACHMENT_UNUSED);
    rpbi.pClearValues = clearValues;

    vkCmdBeginRenderPass(enc->buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    ret->cmdBuffer = enc->buffer;
    ret->cmdEncoder = enc;
    //allColorAndDepthAttachments[max_color_attachments + 1];
    //uint32_t i = 0;
    return ret;

}
extern "C" void wgvkRenderPassEncoderEnd(WGVKRenderPassEncoder renderPassEncoder){
    vkCmdEndRenderPass(renderPassEncoder->cmdBuffer);
}
/**
 * @brief Ends a CommandEncoder into a CommandBuffer
 * This is a one-way transition for WebGPU, therefore we can move resource tracking
 * In Vulkan, this transition is merely a call to vkEndCommandBuffer
 * 
 */
extern "C" WGVKCommandBuffer wgvkCommandEncoderFinish(WGVKCommandEncoder commandEncoder){
    WGVKCommandBuffer ret = callocnewpp(WGVKCommandBufferImpl);
    ret->refCount = 1;
    vkEndCommandBuffer(commandEncoder->buffer);
    ret->referencedRPs = std::move(commandEncoder->referencedRPs);
    ret->resourceUsage = std::move(commandEncoder->resourceUsage);
    ret->pool = commandEncoder->pool;
    ret->buffer = commandEncoder->buffer;
    commandEncoder->buffer = nullptr;
    commandEncoder->pool = nullptr;
    return ret;
}

extern "C" void wgvkQueueSubmit(WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers){
    VkSubmitInfo si zeroinit;
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = commandCount + 1;
    small_vector<VkCommandBuffer> submittable(commandCount + 1);
    for(size_t i = 0;i < commandCount;i++){
        WGVKCommandBuffer buffer = buffers[i];
        for(auto rp : buffer->referencedRPs){
            for(auto dset : rp->resourceUsage.referencedBindGroups){
                for(auto tex : dset->resourceUsage.referencedTextureViews){
                    if(tex.first->texture->layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
                        wgvkCommandEncoderTransitionTextureLayout(queue->presubmitCache, tex.first->texture, tex.first->texture->layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                    }
                }
            }
        }
    }
    WGVKCommandBuffer cachebuffer = wgvkCommandEncoderFinish(queue->presubmitCache);

    submittable[0] = cachebuffer->buffer;
    for(size_t i = 0;i < commandCount;i++){
        submittable[i + 1] = buffers[i]->buffer;
    }
    si.pCommandBuffers = submittable.data();
    VkFence fence = CreateFence();
    VkResult submitResult = vkQueueSubmit(queue->graphicsQueue, 1, &si, fence);
    if(submitResult == VK_SUCCESS){
        std::unordered_set<WGVKCommandBuffer> insert;
        insert.reserve(3);
        insert.insert(cachebuffer);
        ++cachebuffer->refCount;
        
        for(size_t i = 0;i < commandCount;i++){
            insert.insert(buffers[i]);
            ++buffers[i]->refCount;
        }
        uint64_t frameCount = queue->device->submittedFrames;
        queue->pendingCommandBuffers.emplace(fence, std::move(insert));
        TRACELOG(LOG_INFO, "Count: %d", (int)queue->pendingCommandBuffers.size());
        /*if(vkWaitForFences(g_vulkanstate.device->device, 1, &g_vulkanstate.queue->syncState.renderFinishedFence, VK_TRUE, 100000000) != VK_SUCCESS){
            throw std::runtime_error("Could not wait for fence");
        }
        vkResetFences(g_vulkanstate.device->device, 1, &g_vulkanstate.queue->syncState.renderFinishedFence);*/
    }else{
        throw std::runtime_error("vkQueueSubmit failed");
    }
    wgvkReleaseCommandEncoder(queue->presubmitCache);
    wgvkReleaseCommandBuffer(cachebuffer);
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    queue->presubmitCache = wgvkDeviceCreateCommandEncoder(g_vulkanstate.device, &cedesc);
    //queue->presubmitCache = wgvkResetCommandBuffer(cachebuffer);
    //VkPipelineStageFlags bop[1] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    //si.pWaitDstStageMask = bop;
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
        wgvkSurface->presentModeCache = (PresentMode*)std::calloc(presentModeCount, sizeof(PresentMode));
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

void wgvkSurfaceConfigure(WGVKSurface surface, const SurfaceConfiguration* config){
    auto device = WGVKDevice(config->device);

    vkDeviceWaitIdle(device->device);
    if(surface->imageViews){
        for (uint32_t i = 0; i < surface->imagecount; i++) {
            wgvkReleaseTextureView(surface->imageViews[i]);
            //vkDestroyImage(device, surface->images[i], nullptr);
        }
    }

    //std::free(surface->framebuffers);
    
    std::free(surface->imageViews);
    std::free(surface->images);
    vkDestroySwapchainKHR(device->device, surface->swapchain, nullptr);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, surface->surface);
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;
    VkSurfaceCapabilitiesKHR vkCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vulkanstate.physicalDevice, surface->surface, &vkCapabilities);
    
    createInfo.minImageCount = vkCapabilities.minImageCount;
    createInfo.imageFormat = toVulkanPixelFormat(config->format);//swapchainImageFormat;
    surface->width = config->width;
    surface->height = config->height;
    surface->device = (WGVKDevice)config->device;
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

    if (vkCreateSwapchainKHR(device->device, &createInfo, nullptr, &(surface->swapchain)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    } else {
        std::cout << "Successfully created swap chain\n";
    }

    vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, nullptr);

    std::vector<VkImage> tmpImages(surface->imagecount);

    //surface->imageViews = (VkImageView*)std::calloc(surface->imagecount, sizeof(VkImageView));

    vkGetSwapchainImagesKHR(g_vulkanstate.device->device, surface->swapchain, &surface->imagecount, tmpImages.data());
    surface->images = (WGVKTexture*)std::calloc(surface->imagecount, sizeof(WGVKTexture));
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        surface->images[i] = callocnew(WGVKTextureImpl);
        surface->images[i]->device = device;
        surface->images[i]->width = config->width;
        surface->images[i]->height = config->height;
        surface->images[i]->depthOrArrayLayers = 1;
        surface->images[i]->refCount = 1;
        surface->images[i]->sampleCount = 1;
        surface->images[i]->image = tmpImages[i];
    }
    surface->imageViews = (WGVKTextureView*)std::calloc(surface->imagecount, sizeof(VkImageView));

    for (uint32_t i = 0; i < surface->imagecount; i++) {
        WGVKTextureViewDescriptor viewDesc zeroinit;
        viewDesc.arrayLayerCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.aspect = TextureAspect_All;
        viewDesc.dimension = TextureDimension_2D;
        viewDesc.format = config->format;
        viewDesc.usage = TextureUsage_RenderAttachment;
        //VkImageViewCreateInfo viewInfo{};
        //viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //viewInfo.image = surface->images[i]->image;
        //viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        //viewInfo.format = toVulkanPixelFormat(BGRA8);
        //viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        //viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        //viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        //viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        //viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        //viewInfo.subresourceRange.baseMipLevel = 0;
        //viewInfo.subresourceRange.levelCount = 1;
        //viewInfo.subresourceRange.baseArrayLayer = 0;
        //viewInfo.subresourceRange.layerCount = 1;
        //if (vkCreateImageView(device, &viewInfo, nullptr, &surface->imageViews[i]) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to create image views!");
        //}
        surface->imageViews[i] = wgvkTextureCreateView(surface->images[i], &viewDesc);
    }

}
void impl_transition(VkCommandBuffer buffer, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    
    VkImage image = texture->image;
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
        buffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    texture->layout = newLayout;
}
void wgvkRenderPassEncoderTransitionTextureLayout(WGVKRenderPassEncoder encoder, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    impl_transition(encoder->cmdBuffer, texture, oldLayout, newLayout);
}
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    impl_transition(encoder->buffer, texture, oldLayout, newLayout);
}

void wgvkReleaseCommandEncoder(WGVKCommandEncoder commandEncoder) {
    for(auto rp : commandEncoder->referencedRPs){
        wgvkReleaseRenderPassEncoder(rp);
    }
    if(commandEncoder->buffer)
        vkFreeCommandBuffers(g_vulkanstate.device->device, commandEncoder->pool, 1, &commandEncoder->buffer);
    std::free(commandEncoder);
}

extern "C" WGVKCommandEncoder wgvkResetCommandBuffer(WGVKCommandBuffer commandBuffer) {
    WGVKCommandEncoder ret = callocnewpp(WGVKCommandEncoderImpl);
    assert(commandBuffer->refCount == 1);

    for(auto rp : commandBuffer->referencedRPs){
        wgvkReleaseRenderPassEncoder(rp);
    }
    commandBuffer->referencedRPs.clear();
    if(commandBuffer->buffer)
        vkResetCommandBuffer(commandBuffer->buffer, 0);
    ret->buffer = commandBuffer->buffer;
    ret->pool = commandBuffer->pool;
    ret->recyclable = true;
    std::free(commandBuffer);
    VkCommandBufferBeginInfo cbbi zeroinit;
    cbbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult beginResult = vkBeginCommandBuffer(ret->buffer, &cbbi);
    if(beginResult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Could not start command recording");
    }
    return ret;
}

void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer) {
    --commandBuffer->refCount;
    if(commandBuffer->refCount == 0){
        for(auto rp : commandBuffer->referencedRPs){
            wgvkReleaseRenderPassEncoder(rp);
        }
        vkFreeCommandBuffers(g_vulkanstate.device->device, commandBuffer->pool, 1, &commandBuffer->buffer);
        vkDestroyCommandPool(g_vulkanstate.device->device, commandBuffer->pool, nullptr);

        std::free(commandBuffer);
    }
}

void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc) {
    --rpenc->refCount;
    if (rpenc->refCount == 0) {
        rpenc->resourceUsage.releaseAllAndClear();
        std::free(rpenc);
    }
}
void wgvkReleaseBuffer(WGVKBuffer buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vkDestroyBuffer(g_vulkanstate.device->device, buffer->buffer, nullptr);
        vkFreeMemory(g_vulkanstate.device->device, buffer->memory, nullptr);
        std::free(buffer);
    }
}
void wgvkReleaseDescriptorSet(WGVKBindGroup dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        dshandle->resourceUsage.releaseAllAndClear();
        //vkFreeDescriptorSets(g_vulkanstate.device->device, dshandle->pool, 1, &dshandle->set);
        vkDestroyDescriptorPool(g_vulkanstate.device->device, dshandle->pool, nullptr);
        std::free(dshandle);
    }
}

extern "C" void wgvkReleaseTexture(WGVKTexture texture){
    --texture->refCount;
    if(texture->refCount == 0){
        vkDestroyImage(texture->device->device, texture->image, nullptr);
        vkFreeMemory(texture->device->device, texture->memory, nullptr);
        std::free(texture);
    }
}
extern "C" void wgvkReleaseTextureView(WGVKTextureView view){
    --view->refCount;
    if(view->refCount == 0){
        vkDestroyImageView(view->texture->device->device, view->view, nullptr);
        wgvkReleaseTexture(view->texture);
        std::free(view);
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
void wgvkRenderPassEncoderBindDescriptorSet(WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset) {
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


    rpe->resourceUsage.track(dset);
}

void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");
    
    // Bind the vertex buffer to the command buffer at the specified binding point
    vkCmdBindVertexBuffers(rpe->cmdBuffer, binding, 1, &(buffer->buffer), &offset);

    rpe->resourceUsage.track(buffer);
}
void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, VkIndexType indexType) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(buffer != nullptr && "BufferHandle is null");

    // Bind the index buffer to the command buffer
    vkCmdBindIndexBuffer(rpe->cmdBuffer, buffer->buffer, offset, indexType);

    rpe->resourceUsage.track(buffer);
}
