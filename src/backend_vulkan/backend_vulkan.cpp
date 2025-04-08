#include <config.h>
#define Font rlFont
#include <raygpu.h>
#undef Font
#include <set>
#include <vulkan/vulkan_core.h>
#include <renderstate.hpp>
#include "vulkan_internals.hpp"
#define RGFW_VULKAN
#include <external/RGFW.h>

VulkanState g_vulkanstate{};

void BufferData(DescribedBuffer* buffer, void* data, size_t size){
    if(buffer->size >= size){
        WGVKBuffer handle = (WGVKBuffer)buffer->buffer;
        void* udata = 0;
        VmaAllocationInfo info zeroinit;

        vmaGetAllocationInfo(handle->device->allocator, handle->allocation, &info);

        VkDeviceMemory baseMemory = info.deviceMemory;
        uint64_t baseOffset = info.offset;

        VkResult mapresult = vkMapMemory(g_vulkanstate.device->device, baseMemory, baseOffset, size, 0, &udata);
        if(mapresult == VK_SUCCESS){
            std::memcpy(data, udata, size);
            vkUnmapMemory(g_vulkanstate.device->device, baseMemory);
        }
        else{
            abort();
        }
        
        //vkBindBufferMemory()
    }
}


void ResetSyncState(){
    //g_vulkanstate.queue->syncState[0].submitsInThisFrame = 0;
    //g_vulkanstate.syncState.semaphoresInThisFrame.clear();
}
__attribute__((noinline)) VkResult vkQueuePresentKHR_Profilable(VkQueue queue, const VkPresentInfoKHR *pPresentInfo){
    return vkQueuePresentKHR(queue, pPresentInfo);
}

__attribute__((noinline)) VkResult vkQueueSubmit_Profilable(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence){
    return vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

void PresentSurface(FullSurface* surface){
    wgvkSurfacePresent((WGVKSurface)surface->surface);
    //static VkSemaphore transitionSemaphore[framesInFlight] = {CreateSemaphore()};
    WGVKSurface wgvksurf = (WGVKSurface)surface->surface;
    uint32_t cacheIndex = wgvksurf->device->submittedFrames % framesInFlight;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    VkSwapchainKHR swapChains[] = {wgvksurf->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &wgvksurf->activeImageIndex;
    VkImageSubresourceRange isr{};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;

    
    VkCommandBuffer transitionBuffer = wgvksurf->device->frameCaches[cacheIndex].finalTransitionBuffer;
    
    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags =  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(transitionBuffer, &beginInfo);
    EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, wgvksurf->images[wgvksurf->activeImageIndex]);
    EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (WGVKTexture)surface->renderTarget.depth.id);
    vkEndCommandBuffer(transitionBuffer);
    VkSubmitInfo cbsinfo zeroinit;
    cbsinfo.commandBufferCount = 1;
    cbsinfo.pCommandBuffers = &transitionBuffer;
    cbsinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    cbsinfo.signalSemaphoreCount = 1;
    cbsinfo.waitSemaphoreCount = 1;
    VkPipelineStageFlags wsmask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    cbsinfo.pWaitDstStageMask = &wsmask;

    cbsinfo.pWaitSemaphores = &g_vulkanstate.queue->syncState[cacheIndex].semaphores[g_vulkanstate.queue->syncState[cacheIndex].submits];
    //TRACELOG(LOG_INFO, "Submit waiting for semaphore index: %u", g_vulkanstate.queue->syncState[cacheIndex].submits);
    
    cbsinfo.pSignalSemaphores = &g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    
    VkFence finalTransitionFence = g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence;
    vkQueueSubmit(wgvksurf->device->queue->graphicsQueue, 1, &cbsinfo, finalTransitionFence);
    
    
    auto it = wgvksurf->device->queue->pendingCommandBuffers[cacheIndex].find(finalTransitionFence);
    if(it == wgvksurf->device->queue->pendingCommandBuffers[cacheIndex].end()){
        wgvksurf->device->queue->pendingCommandBuffers[cacheIndex].emplace(finalTransitionFence, std::unordered_set<WGVKCommandBuffer>{});
    }
    else{
       //it->second.insert(WGVKCommandBuffer{});
    }

    VkResult presentRes = vkQueuePresentKHR(g_vulkanstate.queue->presentQueue, &presentInfo);
    //vkQueueWaitIdle(g_vulkanstate.queue->graphicsQueue);
    ++wgvksurf->device->submittedFrames;
    vmaSetCurrentFrameIndex(g_vulkanstate.device->allocator, wgvksurf->device->submittedFrames % framesInFlight);
    if(presentRes != VK_SUCCESS && presentRes != VK_SUBOPTIMAL_KHR){
        if(presentRes == VK_ERROR_OUT_OF_DATE_KHR){
            TRACELOG(LOG_ERROR, "presentRes is VK_ERROR_OUT_OF_DATE_KHR");
        }
        else
            TRACELOG(LOG_ERROR, "presentRes is %d", presentRes);
    }
    else if(presentRes == VK_SUBOPTIMAL_KHR){
        TRACELOG(LOG_WARNING, "presentRes is VK_SUBOPTIMAL_KHR");
    }
    PostPresentSurface();

}
void DummySubmitOnQueue(){
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    if(g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence == 0){
        g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence = CreateFence();
    }
    VkSubmitInfo emptySubmit zeroinit;
    emptySubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    emptySubmit.waitSemaphoreCount = 1;
    emptySubmit.pWaitSemaphores = g_vulkanstate.queue->syncState[cacheIndex].semaphores.data() + g_vulkanstate.queue->syncState[cacheIndex].submits;
    VkPipelineStageFlags waitmask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    emptySubmit.pWaitDstStageMask = &waitmask;
    vkQueueSubmit(g_vulkanstate.device->queue->graphicsQueue, 1, &emptySubmit, g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence);
    g_vulkanstate.queue->pendingCommandBuffers[cacheIndex].emplace(g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence, std::unordered_set<WGVKCommandBuffer>{});
}

void PostPresentSurface(){
    WGVKDevice surfaceDevice = g_vulkanstate.device;
    WGVKQueue queue = surfaceDevice->queue;

    const uint32_t cacheIndex = surfaceDevice->submittedFrames % framesInFlight;

    if(queue->pendingCommandBuffers[cacheIndex].size() > 0){
        std::vector<VkFence> fences;
        fences.reserve(queue->pendingCommandBuffers[cacheIndex].size());
        for(const auto& [fence, bufferset] : queue->pendingCommandBuffers[cacheIndex]){
            if(fence){
                fences.push_back(fence);
            }
        }
        if(fences.size() > 0){
            VkResult waitResult = vkWaitForFences(surfaceDevice->device, fences.size(), fences.data(), VK_TRUE, ~uint64_t(0));
            if(waitResult != VK_SUCCESS){
                TRACELOG(LOG_FATAL, "Waitresult: %d", waitResult);
            }
            if(fences.size() == 1){
                TRACELOG(LOG_TRACE, "Waiting for fence %p\n", fences[0]);
            }
        }
        else{
            TRACELOG(LOG_INFO, "No fences!");
        }
    }
    else{
        TRACELOG(LOG_INFO, "No fences!");
    }

    for(auto [fence, bufferset] : queue->pendingCommandBuffers[cacheIndex]){
        if(fence){
            vkResetFences(surfaceDevice->device, 1, &fence);
        }
        else{
            //TRACELOG(LOG_INFO, "Amount of buffers to be cleared from null fence. %llu", (unsigned long long)queue->pendingCommandBuffers[cacheIndex].size());
        }
        for(auto buffer : bufferset){
            rassert(buffer->refCount == 1, "CommandBuffer still in use after submit");
            wgvkReleaseCommandBuffer(buffer);
        }
    }
    

    auto& usedBuffers = surfaceDevice->frameCaches[cacheIndex].usedBatchBuffers;
    auto& unusedBuffers = surfaceDevice->frameCaches[cacheIndex].unusedBatchBuffers;
    
    std::copy(usedBuffers.begin(), usedBuffers.end(), std::back_inserter(unusedBuffers));
    usedBuffers.clear();

    queue->pendingCommandBuffers[cacheIndex].clear();
    
    WGVKCommandBuffer buffer = wgvkCommandEncoderFinish(queue->presubmitCache);
    wgvkReleaseCommandEncoder(queue->presubmitCache);
    wgvkReleaseCommandBuffer(buffer);
    vkResetCommandPool(surfaceDevice->device, surfaceDevice->frameCaches[cacheIndex].commandPool, 0);
    WGVKCommandEncoderDescriptor cedesc zeroinit;

    queue->syncState[cacheIndex].submits = 0;
    queue->presubmitCache = wgvkDeviceCreateCommandEncoder(surfaceDevice, &cedesc);
}
extern "C" DescribedBuffer* GenBufferEx(const void* data, size_t size, BufferUsage usage){
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    WGVKBufferDescriptor descriptor{};
    descriptor.size = size;
    descriptor.usage = usage;
    ret->buffer = wgvkDeviceCreateBuffer((WGVKDevice)GetDevice(), &descriptor);
    ret->size = size;
    ret->usage = usage;
    if(data != nullptr){
        wgvkQueueWriteBuffer(g_vulkanstate.queue, (WGVKBuffer)ret->buffer, 0, data, size);
    }
    return ret;
}

extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t width, uint32_t height){
    vkQueueWaitIdle(reinterpret_cast<WGVKSurface>(fsurface->surface)->device->queue->graphicsQueue);
    vkQueueWaitIdle(reinterpret_cast<WGVKSurface>(fsurface->surface)->device->queue->presentQueue);
    VkSemaphoreWaitInfo info;
    fsurface->surfaceConfig.width = width;
    fsurface->surfaceConfig.height = height;

    wgvkSurfaceConfigure((WGVKSurface)fsurface->surface, &fsurface->surfaceConfig);
    UnloadTexture(fsurface->renderTarget.depth);
    fsurface->renderTarget.depth = LoadTexturePro(width, height, Depth32, TextureUsage_RenderAttachment, GetDefaultSettings().sampleCount, 1);
    if(fsurface->renderTarget.depth.sampleCount > 1){
        fsurface->renderTarget.colorMultisample = LoadTexturePro(width, height, BGRA8, TextureUsage_RenderAttachment, GetDefaultSettings().sampleCount, 1);
    }
}

extern "C" void BeginComputepassEx(DescribedComputepass* computePass){
    computePass->cmdEncoder = wgvkDeviceCreateCommandEncoder((WGVKDevice)GetDevice(), nullptr);
    computePass->cpEncoder = wgvkCommandEncoderBeginComputePass((WGVKCommandEncoder)computePass->cmdEncoder);
}

extern "C" void EndComputepassEx(DescribedComputepass* computePass){
    if(computePass->cpEncoder){
        wgvkCommandEncoderEndComputePass((WGVKComputePassEncoder)computePass->cpEncoder);
        computePass->cpEncoder = 0;
    }
    
    //TODO
    g_renderstate.activeComputepass = nullptr;

    WGVKCommandBuffer command = wgvkCommandEncoderFinish((WGVKCommandEncoder)computePass->cmdEncoder);
    wgvkQueueSubmit(g_vulkanstate.queue, 1, &command);
    wgvkReleaseCommandBuffer(command);
    wgvkReleaseCommandEncoder((WGVKCommandEncoder)computePass->cmdEncoder);
}

extern "C" DescribedRenderpass LoadRenderpassEx(RenderSettings settings, bool colorClear, DColor colorClearValue, bool depthClear, float depthClearValue){
    DescribedRenderpass ret{};

    VkRenderPassCreateInfo rpci{};
    ret.settings = settings;

    ret.colorClear = colorClearValue;
    ret.depthClear = depthClearValue;
    ret.colorLoadOp = colorClear ? LoadOp_Clear : LoadOp_Load;
    ret.colorStoreOp = StoreOp_Store;
    ret.depthLoadOp = depthClear ? LoadOp_Clear : LoadOp_Load;
    ret.depthStoreOp = StoreOp_Store;
    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;//toVulkanPixelFormat(g_vulkanstate.surface.surfaceConfig.format);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = toVulkanLoadOperation(ret.colorLoadOp);
    colorAttachment.storeOp = toVulkanStoreOperation(ret.colorStoreOp);
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = colorClear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription& depthAttachment = attachments[1];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = toVulkanLoadOperation(ret.depthLoadOp);
    depthAttachment.storeOp = toVulkanStoreOperation(ret.depthStoreOp);
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachments;
    
    VkAttachmentReference ca{};
    ca.attachment = 0;
    ca.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference da{};
    da.attachment = 1;
    da.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ca;
    subpass.pDepthStencilAttachment = &da;

    rpci.pSubpasses = &subpass;
    rpci.subpassCount = 1;
    vkCreateRenderPass(g_vulkanstate.device->device, &rpci, nullptr, (VkRenderPass*)&ret.VkRenderPass);

    return ret;
}

DescribedSampler LoadSamplerEx(addressMode amode, filterMode fmode, filterMode mipmapFilter, float maxAnisotropy){
    auto vkamode = [](addressMode a){
        switch(a){
            case addressMode::clampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case addressMode::mirrorRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case addressMode::repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            default:
                rg_unreachable();
        }
    };
    DescribedSampler ret{};// = callocnew(DescribedSampler);
    VkSamplerCreateInfo sci{};
    sci.compareEnable = VK_FALSE;
    //sci.compareOp = VK_COMPARE_OP_LESS;
    sci.maxLod = 10;
    sci.minLod = 0;
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.addressModeU = vkamode(amode);
    sci.addressModeV = vkamode(amode);
    sci.addressModeW = vkamode(amode);
    
    sci.mipmapMode = ((mipmapFilter == filter_linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);

    sci.anisotropyEnable = false;
    sci.maxAnisotropy = maxAnisotropy;
    sci.magFilter = ((fmode == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    sci.minFilter = ((fmode == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);

    ret.magFilter = fmode;
    ret.minFilter = fmode;
    ret.addressModeU = amode;
    ret.addressModeV = amode;
    ret.addressModeW = amode;
    ret.maxAnisotropy = maxAnisotropy;
    ret.compare = CompareFunction_Less;//huh??
    VkResult scr = vkCreateSampler(g_vulkanstate.device->device, &sci, nullptr, (VkSampler*)&ret.sampler);
    if(scr != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Sampler creation failed: %s", (int)scr);
    }
    return ret;
}


extern "C" void GetNewTexture(FullSurface *fsurface){

    const size_t submittedframes = fsurface->surfaceConfig.device->submittedFrames;
    const uint32_t cacheIndex = fsurface->surfaceConfig.device->submittedFrames % framesInFlight;

    uint32_t imageIndex = ~0;
    if(fsurface->headless){
        VkSubmitInfo submitInfo zeroinit;
        //g_vulkanstate.queue->syncState[cacheIndex].submits = 0;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &g_vulkanstate.queue->syncState[cacheIndex].semaphores[0];
        vkQueueSubmit(g_vulkanstate.queue->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }
    else{
        WGVKSurface wgvksurf = ((WGVKSurface)fsurface->surface);
        
        //g_vulkanstate.queue->syncState[cacheIndex].submits = 0;

        VkResult acquireResult = vkAcquireNextImageKHR(
            g_vulkanstate.device->device, 
            wgvksurf->swapchain, 
            UINT64_MAX, 
            g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphore, 
            VK_NULL_HANDLE, 
            &imageIndex
        );
        g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = true;
        if(acquireResult != VK_SUCCESS){
            std::cerr << "acquireResult is " << acquireResult << std::endl;
        }
        WGVKDevice surfaceDevice = ((WGVKSurface)fsurface->surface)->device;
        VkCommandBuffer buf = ((WGVKSurface)fsurface->surface)->device->queue->presubmitCache->buffer;
        EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, wgvksurf->images[imageIndex]);
        EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (WGVKTexture)fsurface->renderTarget.depth.id);

        fsurface->renderTarget.texture.id = wgvksurf->images[imageIndex];
        fsurface->renderTarget.texture.view = wgvksurf->imageViews[imageIndex];
        fsurface->renderTarget.texture.width = wgvksurf->width;
        fsurface->renderTarget.texture.height = wgvksurf->height;

        wgvksurf->activeImageIndex = imageIndex;
    }
}
void negotiateSurfaceFormatAndPresentMode(WGVKAdapter adapter, const void* SurfaceHandle){
    WGVKSurface surface = (WGVKSurface)SurfaceHandle;
    uint32_t presentModeCount = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter->physicalDevice, surface->surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter->physicalDevice, surface->surface, &presentModeCount, presentModes.data());
    
    if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_FIFO_RELAXED_KHR) != presentModes.end()){
        g_renderstate.throttled_PresentMode = PresentMode_FifoRelaxed;
    }
    else{
        g_renderstate.throttled_PresentMode = PresentMode_Fifo;
    }
    if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end()){
        g_renderstate.unthrottled_PresentMode = PresentMode_Mailbox;
    }
    else if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end()){
        g_renderstate.unthrottled_PresentMode = PresentMode_Immediate;
    }
    else{
        g_renderstate.unthrottled_PresentMode = PresentMode_Fifo;
    }

    g_renderstate.frameBufferFormat = BGRA8;
}
void* GetInstance(){
    return g_vulkanstate.instance;
}
WGVKDevice GetDevice(){
    return g_vulkanstate.device;
}
WGVKQueue GetQueue  (cwoid){
    return g_vulkanstate.queue;
}
WGVKAdapter GetAdapter(){
    return g_vulkanstate.physicalDevice;
}
// Function to create Vulkan instance
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
        TRACELOG(LOG_ERROR, pCallbackData->pMessage);
        rg_trap();
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        TRACELOG(LOG_WARNING, pCallbackData->pMessage);
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
        TRACELOG(LOG_INFO, pCallbackData->pMessage);
    }
    

    return VK_FALSE;
}
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
VkInstance createInstance() {
    VkInstance ret{};

    uint32_t requiredGLFWExtensions = 0;
    const char*const* windowExtensions = nullptr;
    #if SUPPORT_GLFW == 1
    windowExtensions = glfwGetRequiredInstanceExtensions(&requiredGLFWExtensions);
    #elif SUPPORT_SDL2 == 1
    auto dummywindow = SDL_CreateWindow("dummy", 0, 0, 1, 1, SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN);
    SDL_bool res = SDL_Vulkan_GetInstanceExtensions(dummywindow, &requiredGLFWExtensions, windowExtensions);
    assert(res && "SDL extensions error");
    windowExtensions = (const char**)std::calloc(requiredGLFWExtensions, sizeof(char*));
    res = SDL_Vulkan_GetInstanceExtensions(dummywindow, &requiredGLFWExtensions, windowExtensions);
    assert(res && "SDL extensions error");
    #elif SUPPORT_SDL3 == 1
    windowExtensions = SDL_Vulkan_GetInstanceExtensions(&requiredGLFWExtensions);
    for(uint32_t i = 0;i < requiredGLFWExtensions;i++){
        std::string ext(windowExtensions[i]);
        std::cout << ext << "\n";
    }
    #elif SUPPORT_RGFW == 1
    requiredGLFWExtensions = 2;
    windowExtensions = (const char**)std::calloc(requiredGLFWExtensions, sizeof(char*));
    ((const char**)windowExtensions)[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    ((const char**)windowExtensions)[1] = RGFW_VK_SURFACE;
    #endif
    #if SUPPORT_GLFW == 1 || SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1 || SUPPORT_RGFW == 1
    if (!windowExtensions) {
        TRACELOG(LOG_FATAL, "Failed to get required extensions for windowing!");
    }
    #endif

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<const char *> validationLayers;
    for (const auto &layer : availableLayers) {
        if (std::string(layer.layerName).find("valid") != std::string::npos) {
            #ifndef NDEBUG
            TRACELOG(LOG_INFO, "Selecting Validation Layer %s",layer.layerName);
            validationLayers.push_back(layer.layerName);
            #else
            TRACELOG(LOG_INFO, "Validation Layer %s available but not selections since NDEBUG is defined",layer.layerName);
            //validationLayers.push_back(layer.layerName);
            #endif
        }
    }
    VkValidationFeatureEnableEXT enables[] = { 
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT 
    };
    
    VkValidationFeaturesEXT validationFeatures = {};
    validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validationFeatures.enabledValidationFeatureCount = 1;
    validationFeatures.pEnabledValidationFeatures = enables;

    VkInstanceCreateInfo createInfo{};

    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    if(validationLayers.size() > 0){
        createInfo.pNext = &validationFeatures;
    }

    // Copy GLFW extensions to a vector
    std::vector<const char *> extensions(windowExtensions, windowExtensions + requiredGLFWExtensions);

    uint32_t instanceExtensionCount = 0;
    
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    std::vector<VkExtensionProperties> availableInstanceExtensions_vec(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, availableInstanceExtensions_vec.data());
    std::unordered_set<std::string> availableInstanceExtensions;
    for(auto ext : availableInstanceExtensions_vec){
        availableInstanceExtensions.emplace(ext.extensionName);
        //std::cout << ext.extensionName << ", ";
    }
    #ifndef NDEBUG
    if(availableInstanceExtensions.find(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != availableInstanceExtensions.end()){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    #endif
    //std::cout << std::endl;
    //extensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    

    // (Optional) Enable validation layers here if needed
    VkResult instanceCreation = vkCreateInstance(&createInfo, nullptr, &ret);
    if (instanceCreation != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create Vulkan instance : %s", vkErrorString(instanceCreation));
    } else {
        TRACELOG(LOG_INFO, "Successfully created Vulkan instance");
    }
    #ifndef NDEBUG
    
    VkDebugUtilsMessengerEXT debugMessenger zeroinit;
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr; // Optional
        VkResult dbcResult = CreateDebugUtilsMessengerEXT(ret, &createInfo, nullptr, &debugMessenger);
        if (dbcResult != VK_SUCCESS) {
            TRACELOG(LOG_ERROR, "Failed to set up debug callback, error code: %d! Valudation errors will be printed but ignored", (int)dbcResult);
        }
    }
#endif

    
    return ret;
}

VkPhysicalDeviceType preferredPhysicalDeviceTypes[3] = {
    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
    VK_PHYSICAL_DEVICE_TYPE_CPU
};
int getPhysicalDeviceTypeRank(VkPhysicalDeviceType x){
    for(uint32_t i = 0;i < sizeof(preferredPhysicalDeviceTypes) / sizeof(preferredPhysicalDeviceTypes[0]);i++){
        if(preferredPhysicalDeviceTypes[i] == x)return i;
    }
    return sizeof(preferredPhysicalDeviceTypes) / sizeof(preferredPhysicalDeviceTypes[0]);
}
static inline VkPhysicalDeviceType tvkpdt(AdapterType atype){
    switch(atype){
        case DISCRETE_GPU: return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        case INTEGRATED_GPU: return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        case SOFTWARE_RENDERER: return VK_PHYSICAL_DEVICE_TYPE_CPU;
        rg_unreachable();
    }
    return (VkPhysicalDeviceType)-1;
}
extern "C" void RequestAdapterType(AdapterType type){
    VkPhysicalDeviceType vktype = tvkpdt(type);
    auto it = std::find(std::begin(preferredPhysicalDeviceTypes), std::end(preferredPhysicalDeviceTypes), vktype);
    VkPhysicalDeviceType ittype = *it;
    std::move_backward(std::begin(preferredPhysicalDeviceTypes), it, std::begin(preferredPhysicalDeviceTypes) + 1);
    *std::begin(preferredPhysicalDeviceTypes) = ittype;
}

QueueIndices findQueueFamilies(WGVKAdapter adapter) {
    uint32_t queueFamilyCount = 0;
    QueueIndices ret{
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX
    };
    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Iterate through each queue family and check for the desired capabilities
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        // Check for graphics support
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && ret.graphicsIndex == UINT32_MAX) {
            ret.graphicsIndex = i;
        }
        // Check for presentation support
        #ifdef MAIN_WINDOW_SDL3
        //TRACELOG(LOG_WARNING, "Using the SDL3 route");
        std::cout << SDL_GetError() << "\n";
        SDL_CreateWindow("dummy", 0, 0, SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN);
        bool presentSupport = SDL_Vulkan_GetPresentationSupport(g_vulkanstate.instance, adapter->physicalDevice, i);
        #elif defined(MAIN_WINDOW_SDL2) //uuh 
        VkBool32 presentSupport = VK_TRUE;
        #elif defined(MAIN_WINDOW_GLFW)
        VkBool32 presentSupport = glfwGetPhysicalDevicePresentationSupport(g_vulkanstate.instance, adapter->physicalDevice, i) ? VK_TRUE : VK_FALSE;
        #elif SUPPORT_RGFW == 1
        VkBool32 presentSupport = RGFW_getVKPresentationSupport_noinline(g_vulkanstate.instance, adapter->physicalDevice, i);
        #else
        VkBool32 presentSupport = VK_FALSE;
        #endif
        //vkGetPhysicalDeviceSurfaceSupportKHR(adapter->physicalDevice, i, g_vulkanstate.surface.surface, &presentSupport);
        if (!!presentSupport && ret.presentIndex == UINT32_MAX) {
            ret.presentIndex = i;
        }
        if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT){
            ret.transferIndex = i;
        }

        // Example: Check for compute support
        if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && ret.computeIndex == UINT32_MAX) {
            ret.computeIndex = i;
        }

        // If all families are found, no need to continue
        if (ret.graphicsIndex != UINT32_MAX && ret.presentIndex != UINT32_MAX && ret.computeIndex != UINT32_MAX && ret.transferIndex != UINT32_MAX) {
            break;
        }
    }

    // Validate that at least graphics and present families are found
    if (ret.graphicsIndex == UINT32_MAX) {
        TRACELOG(LOG_FATAL, "Failed to find graphics queue, probably something went wong");
    }

    return ret;
}
extern "C" DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* descs, uint32_t uniformCount, bool){
    DescribedBindGroupLayout retv{};
    DescribedBindGroupLayout* ret = &retv;
    WGVKBindGroupLayout retlayout = wgvkDeviceCreateBindGroupLayout(g_vulkanstate.device, descs, uniformCount);
    ret->layout = retlayout;
    ret->entries = (ResourceTypeDescriptor*)std::calloc(uniformCount, sizeof(ResourceTypeDescriptor));
    if(uniformCount > 0)
        std::memcpy(const_cast<ResourceTypeDescriptor*>(ret->entries), descs, uniformCount * sizeof(ResourceTypeDescriptor));
    ret->entryCount = uniformCount;
    std::vector<ResourceTypeDescriptor> a(retlayout->entries, retlayout->entries + ret->entryCount);
    return retv;
}

// Function to pick a suitable physical device (GPU)
WGVKAdapter pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        TRACELOG(LOG_FATAL, "Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, devices.data());
    
    std::vector<std::pair<VkPhysicalDevice, VkPhysicalDeviceProperties>> dwp;
    dwp.reserve(deviceCount);

    VkPhysicalDevice ret{};
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        dwp.emplace_back(device, props);
        TRACELOG(LOG_INFO, "Found device: %s", props.deviceName);
    }
    
    std::sort(dwp.begin(), dwp.end(), [](const std::pair<VkPhysicalDevice, VkPhysicalDeviceProperties>& a, const std::pair<VkPhysicalDevice, VkPhysicalDeviceProperties>& b){
        if(getPhysicalDeviceTypeRank(a.second.deviceType) < getPhysicalDeviceTypeRank(b.second.deviceType))return true;
        else if(getPhysicalDeviceTypeRank(a.second.deviceType) > getPhysicalDeviceTypeRank(b.second.deviceType))return false;
        return (a.second.driverVersion > b.second.driverVersion);
    });
    ret = dwp.front().first;
    VkPhysicalDeviceProperties pProperties;
    vkGetPhysicalDeviceProperties(ret, &pProperties);
    auto deviceTypeDescription = [](VkPhysicalDeviceType type){
        switch(type){
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "CPU (Software Renderer)";
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "Integrated GPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "Dedicated GPU";
            default: 
                return "?Unknown Adapter Type?";
        }
    };
    const char* article = pProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "an" : "a";
    TRACELOG(LOG_INFO, "Picked Adapter: %s, which is %s %s", pProperties.deviceName, article, deviceTypeDescription(pProperties.deviceType));
    int major = VK_API_VERSION_MAJOR(pProperties.apiVersion);
    int minor = VK_API_VERSION_MINOR(pProperties.apiVersion);
    int patch = VK_API_VERSION_PATCH(pProperties.apiVersion);
    TRACELOG(LOG_INFO, "Running on Vulkan %d.%d.%d", major, minor, patch);
    WGVKAdapter reta = callocnewpp(WGVKAdapterImpl);
    reta->physicalDevice = ret;
    vkGetPhysicalDeviceMemoryProperties(ret, &reta->memProperties);
    reta->queueIndices = findQueueFamilies(reta);
    return reta;
}

// Function to find queue families

constexpr char mipmapComputerSource2[] = R"(
@group(0) @binding(0) var previousMipLevel: texture_2d<f32>;
@group(0) @binding(1) var nextMipLevel: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(8, 8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let offset = vec2<u32>(0, 1);
    
    let color = (
        textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)
    ) * 0.25;
    textureStore(nextMipLevel, id.xy, color);
}
)";
extern "C" void ComputePassSetBindGroup(DescribedComputepass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgvkComputePassEncoderSetBindGroup((WGVKComputePassEncoder)drp->cpEncoder, group, (WGVKBindGroup)UpdateAndGetNativeBindGroup(bindgroup));
}
void GenTextureMipmaps(Texture2D* tex){
    WGVKCommandEncoderDescriptor cdesc zeroinit;
    WGVKCommandEncoder enc = wgvkDeviceCreateCommandEncoder(g_vulkanstate.device, &cdesc);
    
    rassert(tex->mipmaps >= 1, "Mipmaps must always be at least 1, 0 probably means that's an invalid texture");
    rassert(tex->width < (uint32_t(1) << 31), "Texture too humongous");
    rassert(tex->height < (uint32_t(1) << 31), "Texture too humongous");
    
    WGVKTexture wgvkTex = reinterpret_cast<WGVKTexture>(tex->id);

    VkOffset3D initial = VkOffset3D{(int32_t)tex->width, (int32_t)tex->height, 1};
    auto mipExtent = [initial](const uint32_t mipLevel){
        VkOffset3D i = initial;
        i.x >>= mipLevel;
        i.y >>= mipLevel;
        i.x = std::max(i.x, 1);
        i.y = std::max(i.y, 1);
        return i;
    };
    enc->initializeOrTransition(wgvkTex, VK_IMAGE_LAYOUT_GENERAL);
    //wgvkCommandEncoderTransitionTextureLayout(enc, wgvkTex, wgvkTex->layout, VK_IMAGE_LAYOUT_GENERAL);
    for(uint32_t i = 0;i < tex->mipmaps - 1;i++){
        VkImageBlit blitRegion zeroinit;
        blitRegion.srcOffsets[1] = mipExtent(0    );
        blitRegion.dstOffsets[1] = mipExtent(i + 1);
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcSubresource.mipLevel = 0;
        blitRegion.dstSubresource.mipLevel = i + 1;
        vkCmdBlitImage(enc->buffer, wgvkTex->image, VK_IMAGE_LAYOUT_GENERAL, wgvkTex->image, VK_IMAGE_LAYOUT_GENERAL, 1, &blitRegion, VK_FILTER_LINEAR);
    }
    enc->resourceUsage.track(wgvkTex);
    WGVKCommandBuffer buffer = wgvkCommandEncoderFinish(enc);
    wgvkQueueSubmit(g_vulkanstate.queue, 1, &buffer);
    wgvkReleaseCommandBuffer(buffer);
    wgvkReleaseCommandEncoder(enc);

    /* Compute based implementation
        static DescribedComputePipeline* cpl = LoadComputePipeline(mipmapComputerSource2);
        VkImageBlit blit;
        BeginComputepass();
        for(uint32_t i = 0;i < tex->mipmaps - 1;i++){
            SetBindgroupTextureView(&cpl->bindGroup, 0, (WGVKTextureView)tex->mipViews[i    ]);
            SetBindgroupTextureView(&cpl->bindGroup, 1, (WGVKTextureView)tex->mipViews[i + 1]);
            if(i == 0){
                BindComputePipeline(cpl);
            }
            ComputePassSetBindGroup(&g_renderstate.computepass, 0, &cpl->bindGroup);
            uint32_t divisor = (1 << i) * 8;
            DispatchCompute((tex->width + divisor - 1) & -(divisor) / 8, (tex->height + divisor - 1) & -(divisor) / 8, 1);
        }
        EndComputepass();
    */
}

void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGVKBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Uniform;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    //wgvkBuffer->refCount++;
    wgvkQueueWriteBuffer(g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgvkBufferRelease(wgvkBuffer);
}

void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGVKBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Storage;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    wgvkQueueWriteBuffer(g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgvkBufferRelease(wgvkBuffer);
}

extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->buffer != nullptr && buffer->size >= size){
        wgvkQueueWriteBuffer(g_vulkanstate.queue, (WGVKBuffer)buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgvkBufferRelease((WGVKBuffer)buffer->buffer);
        WGVKBufferDescriptor nbdesc zeroinit;
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgvkDeviceCreateBuffer((WGVKDevice)GetDevice(), &nbdesc);
        buffer->size = size;
        wgvkQueueWriteBuffer(g_vulkanstate.queue, (WGVKBuffer)buffer->buffer, 0, data, size);
    }
}


extern "C" void UnloadBuffer(DescribedBuffer* buf){
    WGVKBuffer handle = (WGVKBuffer)buf->buffer;
    wgvkBufferRelease(handle);
    std::free(buf);
}


void createRenderPass() {
    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    //colorAttachment.format = toVulkanPixelFormat(g_vulkanstate.surface.surfaceConfig.format);
    colorAttachment.format = toVulkanPixelFormat(BGRA8);
    colorAttachment.samples = VK_SAMPLE_COUNT_4_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    #ifdef FORCE_HEADLESS
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    #else
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    #endif
    VkAttachmentDescription& depthAttachment = attachments[1];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_4_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    

    if (vkCreateRenderPass(g_vulkanstate.device->device, &renderPassInfo, nullptr, &g_vulkanstate.renderPass) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "failed to create render pass!");
    }
}
extern "C" void RenderPassSetIndexBuffer(DescribedRenderpass* drp, DescribedBuffer* buffer, IndexFormat format, uint64_t offset){
    wgvkRenderPassEncoderBindIndexBuffer((WGVKRenderPassEncoder)drp->rpEncoder, (WGVKBuffer)buffer->buffer, 0, format);
    //wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)drp->rpEncoder, (WGPUBuffer)buffer->buffer, format, offset, buffer->size);
}
extern "C" void RenderPassSetVertexBuffer(DescribedRenderpass* drp, uint32_t slot, DescribedBuffer* buffer, uint64_t offset){
    wgvkRenderPassEncoderBindVertexBuffer((WGVKRenderPassEncoder)drp->rpEncoder, slot, (WGVKBuffer)buffer->buffer, offset);
    //wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, buffer->size);
}
extern "C" void RenderPassSetBindGroup(DescribedRenderpass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgvkRenderPassEncoderSetBindGroup((WGVKRenderPassEncoder)drp->rpEncoder, group, (WGVKBindGroup)bindgroup->bindGroup);
    //wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)drp->rpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, nullptr);
}
extern "C" void RenderPassDraw        (DescribedRenderpass* drp, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance){
    wgvkRenderpassEncoderDraw((WGVKRenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
    //wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
extern "C" void RenderPassDrawIndexed (DescribedRenderpass* drp, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance){
    wgvkRenderpassEncoderDrawIndexed((WGVKRenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, baseVertex, firstInstance);
    //wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}
extern "C" void PrepareFrameGlobals(){
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    if(vbo_buf != 0){
        wgvkBufferUnmap(vbo_buf);
    }
    if(cache.unusedBatchBuffers.empty()){
        WGVKBufferDescriptor bdesc{
            .usage = BufferUsage_CopyDst | BufferUsage_MapWrite | BufferUsage_Vertex,
            .size = (RENDERBATCH_SIZE * sizeof(vertex))
        };

        vbo_buf = wgvkDeviceCreateBuffer(g_vulkanstate.device,  &bdesc);
        
        cache.usedBatchBuffers.push_back(vbo_buf);
        wgvkBufferAddRef(vbo_buf);

        wgvkBufferMap(vbo_buf, MapMode_Write, 0, bdesc.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        
    }
    else{
        vbo_buf = cache.unusedBatchBuffers.back();
        cache.unusedBatchBuffers.pop_back();
        cache.usedBatchBuffers.push_back(vbo_buf);
        VmaAllocationInfo allocationInfo zeroinit;
        vmaGetAllocationInfo(g_vulkanstate.device->allocator, vbo_buf->allocation, &allocationInfo);
        wgvkBufferMap(vbo_buf, MapMode_Write, 0, allocationInfo.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        wgvkBufferAddRef(vbo_buf);
    }
}
extern "C" DescribedBuffer* UpdateVulkanRenderbatch(){
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    DescribedBuffer* db = callocnewpp(DescribedBuffer); 
    db->usage = vbo_buf->usage;
    db->size = wgvkBufferGetSize(vbo_buf);
    db->buffer = vbo_buf;
    if(vbo_buf != 0){
        wgvkBufferUnmap(vbo_buf);
    }
    if(cache.unusedBatchBuffers.empty()){
        WGVKBufferDescriptor bdesc{
            .usage = BufferUsage_CopyDst | BufferUsage_MapWrite | BufferUsage_Vertex,
            .size = (RENDERBATCH_SIZE * sizeof(vertex))
        };

        vbo_buf = wgvkDeviceCreateBuffer(g_vulkanstate.device,  &bdesc);
        wgvkBufferMap(vbo_buf, MapMode_Write, 0, bdesc.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        cache.usedBatchBuffers.push_back(vbo_buf);
        wgvkBufferAddRef(vbo_buf);
    }
    else{
        vbo_buf = cache.unusedBatchBuffers.back();
        cache.unusedBatchBuffers.pop_back();
        cache.usedBatchBuffers.push_back(vbo_buf);
        VmaAllocationInfo allocationInfo zeroinit;
        vmaGetAllocationInfo(g_vulkanstate.device->allocator,vbo_buf->allocation, &allocationInfo);
        wgvkBufferMap(vbo_buf, MapMode_Write, 0, allocationInfo.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        wgvkBufferAddRef(vbo_buf);
    }

    return db;
}
void PushUsedBuffer(void* nativeBuffer){
    return;
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    WGVKBuffer buffer = (WGVKBuffer)nativeBuffer;
    
}


extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    WGVKBindGroup bindGroup = (WGVKBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup);
    wgvkComputePassEncoderSetPipeline ((WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder, (WGVKComputePipeline)pipeline->pipeline);
    wgvkComputePassEncoderSetBindGroup((WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder, 0, bindGroup);
}

void printVkPhysicalDeviceMemoryProperties(const VkPhysicalDeviceMemoryProperties* properties) {
    if (!properties) {
        printf("Invalid VkPhysicalDeviceMemoryProperties pointer\n");
        return;
    }

    printf("VkPhysicalDeviceMemoryProperties:\n");
    
    printf("  Memory Type Count: %u\n", properties->memoryTypeCount);
    for (uint32_t i = 0; i < properties->memoryTypeCount; i++) {
        VkMemoryType memoryType = properties->memoryTypes[i];
        printf("    Memory Type %u:\n", i);
        printf("      Heap Index: %u\n", memoryType.heapIndex);
        printf("      Property Flags: 0x%08X\n", memoryType.propertyFlags);

        // Decode memory property flags
        printf("      Properties: ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) printf("DEVICE_LOCAL ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) printf("HOST_VISIBLE ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) printf("HOST_COHERENT ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) printf("HOST_CACHED ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) printf("LAZILY_ALLOCATED ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) printf("PROTECTED ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) printf("DEVICE_COHERENT_AMD ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) printf("DEVICE_UNCACHED_AMD ");
        if (memoryType.propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) printf("RDMA_CAPABLE_NV ");
        printf("\n");
    }

    printf("  Memory Heap Count: %u\n", properties->memoryHeapCount);
    for (uint32_t i = 0; i < properties->memoryHeapCount; i++) {
        VkMemoryHeap memoryHeap = properties->memoryHeaps[i];
        printf("    Memory Heap %u:\n", i);
        printf("      Size: %llu bytes (%.2f MB)\n", (unsigned long long)memoryHeap.size, memoryHeap.size / (1024.0 * 1024.0));
        printf("      Flags: 0x%08X\n", memoryHeap.flags);

        // Decode memory heap flags
        printf("      Properties: ");
        if (memoryHeap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) printf("DEVICE_LOCAL ");
        if (memoryHeap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) printf("MULTI_INSTANCE ");
        printf("\n");
    }
}
memory_types discoverMemoryTypes(VkPhysicalDevice physicalDevice) {
    memory_types ret{~0u, ~0u};
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    //printVkPhysicalDeviceMemoryProperties(&memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            ret.hostVisibleCoherent = i;
        }
        if((memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
            ret.deviceLocal = i;
        }
    }
    return ret;
}
WGVKDevice wgvkAdapterCreateDevice(WGVKAdapter adapter, const WGVKDeviceDescriptor* descriptor){
    std::pair<WGVKDevice, WGVKQueue> ret{};
    // Collect unique queue families
    std::vector<uint32_t> queueFamilies;
    {
        std::set<uint32_t> uniqueQueueFamilies; // = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

        uniqueQueueFamilies.insert(adapter->queueIndices.computeIndex);
        uniqueQueueFamilies.insert(adapter->queueIndices.graphicsIndex);
        uniqueQueueFamilies.insert(adapter->queueIndices.presentIndex);
        auto it = uniqueQueueFamilies.find(VK_QUEUE_FAMILY_IGNORED);
        if(it != uniqueQueueFamilies.end()){
            uniqueQueueFamilies.erase(it);
        }
        // Example: Include computeFamily if it's different
        queueFamilies = std::vector<uint32_t>(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
    }
    // Create queue create infos
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : queueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deprops(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, nullptr, &deviceExtensionCount, deprops.data());
    
    for(auto e : deprops){
        //std::cout << e.extensionName << ", ";
    }
    // Specify device extensions
    
    std::vector<const char *> deviceExtensions = {
        #ifndef FORCE_HEADLESS
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        #endif
        #if VULKAN_ENABLE_RAYTRACING == 1
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,      // "VK_KHR_acceleration_structure"
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,        // "VK_KHR_ray_tracing_pipeline"
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,    // "VK_KHR_deferred_host_operations" - required by acceleration structure
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,         // "VK_EXT_descriptor_indexing" - needed for bindless descriptors
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,       // "VK_KHR_buffer_device_address" - needed by AS
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,                   // "VK_KHR_spirv_1_4" - required for ray tracing shaders
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,        // "VK_KHR_shader_float_controls" - required by spirv_1_4
        #endif
        // Add other required extensions here
    };

    // Specify device features


    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    props.dynamicPrimitiveTopologyUnrestricted = VK_TRUE;
    
    
    VkPhysicalDeviceFeatures deviceFeatures{};


    VkDeviceCreateInfo createInfo zeroinit;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    VkPhysicalDeviceVulkan13Features v13features{};
    v13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    v13features.dynamicRendering = VK_TRUE;
    #endif

    createInfo.pNext = &v13features;
    //features.pNext = &props;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    #if VULKAN_ENABLE_RAYTRACING == 1
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR deviceFeaturesAddressKhr zeroinit;
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationrStructureFeatures zeroinit;
    accelerationrStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationrStructureFeatures.accelerationStructure = VK_TRUE;
    deviceFeaturesAddressKhr.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR;
    deviceFeaturesAddressKhr.bufferDeviceAddress = VK_TRUE;
    v13features.pNext = &deviceFeaturesAddressKhr;
    deviceFeaturesAddressKhr.pNext = &accelerationrStructureFeatures;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR pipelineFeatures zeroinit;
    pipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    pipelineFeatures.rayTracingPipeline = VK_TRUE;
    accelerationrStructureFeatures.pNext = &pipelineFeatures;
    #endif

    // (Optional) Enable validation layers for device-specific debugging
    ret.first = callocnewpp(WGVKDeviceImpl);
    ret.second = callocnewpp(WGVKQueueImpl);
    auto dcresult = vkCreateDevice(adapter->physicalDevice, &createInfo, nullptr, &(ret.first->device));
    if (dcresult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create logical device!");
    } else {
        TRACELOG(LOG_INFO, "Successfully created logical device");
    }

    // Retrieve and assign queues
    
    auto& indices = adapter->queueIndices;
    vkGetDeviceQueue(ret.first->device, indices.graphicsIndex, 0, &ret.second->graphicsQueue);
    #ifndef FORCE_HEADLESS
    vkGetDeviceQueue(ret.first->device, indices.presentIndex, 0, &ret.second->presentQueue);
    #endif
    if (indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex) {
        vkGetDeviceQueue(ret.first->device, indices.computeIndex, 0, &ret.second->computeQueue);
    } else {
        // If compute Index is same as graphics or present, assign accordingly
        if (indices.computeIndex == indices.graphicsIndex) {
            ret.second->computeQueue = ret.second->graphicsQueue;
        } else if (indices.computeIndex == indices.presentIndex) {
            ret.second->computeQueue = ret.second->presentQueue;
        }
    }
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    cedesc.recyclable = true;
    for(uint32_t i = 0;i < framesInFlight;i++){
        WGVKDevice device = ret.first;
        ret.first->frameCaches[i].finalTransitionSemaphore = CreateSemaphoreD(device->device);
        VkCommandPoolCreateInfo pci zeroinit;
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCreateCommandPool(ret.first->device, &pci, nullptr, &ret.first->frameCaches[i].commandPool);
        
        VkCommandBufferAllocateInfo cbai zeroinit;
        cbai.commandBufferCount = 1;
        cbai.commandPool = device->frameCaches[i].commandPool;
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(device->device, &cbai, &device->frameCaches[i].finalTransitionBuffer);
        VkFenceCreateInfo sci zeroinit;
        VkFence ret zeroinit;
        sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult res = vkCreateFence(device->device, &sci, nullptr, &device->frameCaches[i].finalTransitionFence);
    }
    ret.second->presubmitCache = wgvkDeviceCreateCommandEncoder(ret.first, &cedesc);

    VmaAllocatorCreateInfo aci zeroinit;
    aci.instance = g_vulkanstate.instance;
    aci.physicalDevice = adapter->physicalDevice;
    aci.device = ret.first->device;
    #if VULKAN_ENABLE_RAYTRACING == 1
    aci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    #endif
    VkDeviceSize limit = (uint64_t(1) << 30);
    aci.preferredLargeHeapBlockSize = 1 << 15;
    VkPhysicalDeviceMemoryProperties memoryProperties zeroinit;
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &memoryProperties);
    std::vector<VkDeviceSize> heapsizes(memoryProperties.memoryHeapCount, limit);
    aci.pHeapSizeLimit = heapsizes.data();

    VmaDeviceMemoryCallbacks callbacks{
        /// Optional, can be null.
        .pfnAllocate = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
            TRACELOG(LOG_WARNING, "Allocating %llu of memory type %u", size, type);
        },
        .pfnFree = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
            TRACELOG(LOG_WARNING, "Freeing %llu of memory type %u", size, type);
        }
    };
    aci.pDeviceMemoryCallbacks = &callbacks;
    VkResult allocatorCreateResult = vmaCreateAllocator(&aci, &ret.first->allocator);

    if(allocatorCreateResult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating the allocator: %d", (int)allocatorCreateResult);
    }
    for(uint32_t i = 0;i < framesInFlight;i++){
        ret.second->syncState[i].semaphores.resize(100);
        for(uint32_t j = 0;j < ret.second->syncState[i].semaphores.size();j++){
            ret.second->syncState[i].semaphores[j] = CreateSemaphoreD(ret.first->device);
        }
        ret.second->syncState[i].acquireImageSemaphore = CreateSemaphoreD(ret.first->device);

        VmaPoolCreateInfo vpci zeroinit;
        vpci.minAllocationAlignment = 64;
        vpci.memoryTypeIndex = g_vulkanstate.memoryTypes.hostVisibleCoherent;
        vpci.blockSize = (1 << 16);
        vmaCreatePool(ret.first->allocator, &vpci, &ret.first->aligned_hostVisiblePool);
        // TODO
    }

    {
        auto [device, queue] = ret;
        device->queue = queue;
        device->adapter = adapter;
        {
            
            // Get ray tracing pipeline properties
            #if VULKAN_ENABLE_RAYTRACING == 1
            VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties zeroinit;
		    rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		    VkPhysicalDeviceProperties2 deviceProperties2 zeroinit;
		    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		    deviceProperties2.pNext = &rayTracingPipelineProperties;
		    vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);
            adapter->rayTracingPipelineProperties = rayTracingPipelineProperties;
            #endif
        }
        device->adapter = adapter;
        queue->device = device;
    }
    return ret.first;
}
extern "C" void raytracing_LoadDeviceFunctions(VkDevice device);
void InitBackend(){
    #if SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1
    SDL_Init(SDL_INIT_VIDEO);
    #endif
    #if SUPPORT_GLFW
    glfwInit();
    #endif
    
    g_vulkanstate.instance = createInstance();
    g_vulkanstate.physicalDevice = pickPhysicalDevice();

    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice->physicalDevice, &g_vulkanstate.memProperties);

    WGVKDeviceDescriptor ddescriptor zeroinit;
    WGVKDevice device = wgvkAdapterCreateDevice(g_vulkanstate.physicalDevice, &ddescriptor);
    g_vulkanstate.device = device;
    g_vulkanstate.queue = device->queue;
    //auto device_and_queues = createLogicalDevice(g_vulkanstate.physicalDevice, queues);
    //raytracing_LoadDeviceFunctions(device_and_queues.first->device);
    //g_vulkanstate.device = device_and_queues.first;
    //g_vulkanstate.queue = device_and_queues.second;
    //for(uint32_t fif = 0;fif < framesInFlight;fif++){
    //    g_vulkanstate.queue->syncState[fif].renderFinishedFence = CreateFence(0);
    //}

    
    createRenderPass();
}

extern "C" FullSurface CreateSurface(void* nsurface, uint32_t width, uint32_t height){
    FullSurface ret{};
    ret.surface = (WGVKSurface)nsurface;
    WGVKAdapter adapter = g_vulkanstate.physicalDevice;
    negotiateSurfaceFormatAndPresentMode(adapter, nsurface);
    WGVKSurfaceCapabilities capa{};

    wgvkSurfaceGetCapabilities((WGVKSurface)ret.surface, adapter, &capa);
    WGVKSurfaceConfiguration config{};

    PresentMode thm = g_renderstate.throttled_PresentMode;
    PresentMode um  = g_renderstate.unthrottled_PresentMode;
    if (g_renderstate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT) {
        config.presentMode = (((g_renderstate.unthrottled_PresentMode == PresentMode_Mailbox) ? um : thm));
    } else if (g_renderstate.windowFlags & FLAG_VSYNC_HINT) {
        config.presentMode = thm;
    } else {
        config.presentMode = um;
    }
    //TRACELOG(LOG_INFO, "Initialized surface with %s", presentModeSpellingTable.at(config.presentMode).c_str());
    //config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = g_renderstate.frameBufferFormat;
    //config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    //config.viewFormats = &config.format;
    //config.viewFormatCount = 1;
    config.device = g_vulkanstate.device;

    ret.surfaceConfig.presentMode = (PresentMode)config.presentMode;
    ret.surfaceConfig.device = config.device;
    ret.surfaceConfig.width = config.width;
    ret.surfaceConfig.height = config.width;
    ret.surfaceConfig.format = (PixelFormat)config.format;
    
    ret.renderTarget = LoadRenderTexture(width, height);
    wgvkSurfaceConfigure((WGVKSurface)ret.surface, &config);
    return ret;
}

RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    RenderTexture ret{
        .texture = LoadTextureEx(width, height, (PixelFormat)g_renderstate.frameBufferFormat, true),
        .colorMultisample = Texture{}, 
        .depth = LoadTexturePro(width, height, Depth32, TextureUsage_RenderAttachment | TextureUsage_CopySrc, (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1)
    };
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT){
        ret.colorMultisample = LoadTexturePro(width, height, (PixelFormat)g_renderstate.frameBufferFormat, TextureUsage_RenderAttachment | TextureUsage_CopySrc, 4, 1);
    }
    WGVKTextureView colorTargetView = (WGVKTextureView)ret.texture.view;
    wgvkQueueTransitionLayout(g_vulkanstate.queue, colorTargetView->texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    if(ret.colorMultisample.id)
        wgvkQueueTransitionLayout(g_vulkanstate.queue, ((WGVKTexture)ret.colorMultisample.id), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    wgvkQueueTransitionLayout(g_vulkanstate.queue, ((WGVKTexture)ret.depth.id), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    return ret;
}
extern "C" void BeginRenderpassEx(DescribedRenderpass *renderPass){

    //WGVKRenderPassEncoder ret = callocnewpp(WGVKRenderPassEncoderImpl);

    renderPass->settings = g_renderstate.currentSettings;

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    WGVKCommandEncoder encoder = (WGVKCommandEncoder)renderPass->cmdEncoder;
    renderPass->cmdEncoder = wgvkDeviceCreateCommandEncoder(g_vulkanstate.device, &cedesc);
    
    //if(renderPass->cmdEncoder == nullptr){
    //    VkCommandPool oof{};
    //    VkCommandPoolCreateInfo pci zeroinit;
    //    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    //    VkCommandBufferAllocateInfo cbai zeroinit;
    //    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    //    cbai.commandBufferCount = 1;
    //    vkCreateCommandPool(g_vulkanstate.device, &pci, nullptr, &oof);
    //    cbai.commandPool = oof;
    //    vkAllocateCommandBuffers(g_vulkanstate.device, &cbai, (VkCommandBuffer*)&renderPass->cmdEncoder);
    //}
    //else{
    //    vkResetCommandBuffer((VkCommandBuffer)renderPass->cmdEncoder, 0);
    //}
    rpbi.renderPass = g_vulkanstate.renderPass;

    auto rtex = g_renderstate.renderTargetStack.peek();
    //VkFramebufferCreateInfo fbci{};
    //fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //VkImageView fbAttachments[2] = {
    //    ((WGVKTextureView)rtex.texture.view)->view,
    //    ((WGVKTextureView)rtex.depth.view)->view,
    //};

    WGVKRenderPassDescriptor rpdesc zeroinit;
    WGVKRenderPassDepthStencilAttachment dsa zeroinit;
    dsa.depthClearValue = renderPass->depthClear;
    dsa.stencilClearValue = 0;
    dsa.stencilLoadOp = LoadOp_Undefined;
    dsa.stencilStoreOp = StoreOp_Discard;
    dsa.depthLoadOp = renderPass->depthLoadOp;
    dsa.depthStoreOp = renderPass->depthStoreOp;
    dsa.view = (WGVKTextureView)rtex.depth.view;
    WGVKRenderPassColorAttachment rca zeroinit;
    rca.depthSlice = 0;
    rca.clearValue = renderPass->colorClear;
    rca.loadOp = renderPass->colorLoadOp;
    rca.storeOp = renderPass->colorStoreOp;
    if(rtex.colorMultisample.view){
        rca.view = (WGVKTextureView)rtex.colorMultisample.view;
        rca.resolveTarget = (WGVKTextureView)rtex.texture.view;
        reinterpret_cast<WGVKCommandEncoder>(renderPass->cmdEncoder)->initializeOrTransition(rca.view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        reinterpret_cast<WGVKCommandEncoder>(renderPass->cmdEncoder)->initializeOrTransition(rca.resolveTarget->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        if(rca.view->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            //wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.view->texture, rca.view->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
        if(rca.resolveTarget->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            //wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.resolveTarget->texture, rca.resolveTarget->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }
    else{
        rca.view = (WGVKTextureView)rtex.texture.view;
        reinterpret_cast<WGVKCommandEncoder>(renderPass->cmdEncoder)->initializeOrTransition(rca.view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        if(rca.view->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            //wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.view->texture, rca.view->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }
    if(renderPass->settings.depthTest){
        rpdesc.depthStencilAttachment = &dsa;
    }
    rpdesc.colorAttachments = &rca;
    rpdesc.colorAttachmentCount = 1;
    //fbci.pAttachments = fbAttachments;
    //fbci.attachmentCount = 2;
    //fbci.width = rtex.texture.width;
    //fbci.height = rtex.texture.height;
    //fbci.layers = 1;
    //fbci.renderPass = (VkRenderPass)renderPass->VkRenderPass;
    //VkFramebuffer rahmePuffer = 0;
    //vkCreateFramebuffer(g_vulkanstate.device, &fbci, nullptr, &rahmePuffer);
    //rpbi.framebuffer = rahmePuffer;
    //renderPass->rpEncoder = BeginRenderPass_Vk((VkCommandBuffer)renderPass->cmdEncoder, renderPass, rahmePuffer, rtex.texture.width, rtex.texture.height);
    renderPass->rpEncoder = wgvkCommandEncoderBeginRenderPass((WGVKCommandEncoder)renderPass->cmdEncoder, &rpdesc);
    VkViewport viewport zeroinit;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    viewport.x = 0;
    viewport.y = rtex.texture.height;
    viewport.width  = rtex.texture.width;
    viewport.height = -((float)rtex.texture.height);

    VkRect2D scissor zeroinit;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width =  rtex.texture.width;
    scissor.extent.height = rtex.texture.height;
    vkCmdSetViewport(((WGVKRenderPassEncoder)renderPass->rpEncoder)->cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor (((WGVKRenderPassEncoder)renderPass->rpEncoder)->cmdBuffer, 0, 1, &scissor);
    //wgvkRenderPassEncoderBindPipeline((WGVKRenderPassEncoder)renderPass->rpEncoder, g_renderstate.defaultPipeline);
    g_renderstate.activeRenderpass = renderPass;
    //UpdateBindGroup_Vk(&g_renderstate.defaultPipeline->bindGroup);
    //wgvkRenderPassEncoderBindDescriptorSet((WGVKRenderPassEncoder)renderPass->rpEncoder, 0, (DescriptorSetHandle)g_renderstate.defaultPipeline->bindGroup.bindGroup);
    //rlVertex2f(0, 0);
    //rlVertex2f(1, 0);
    //rlVertex2f(0, 1);
    //drawCurrentBatch();
    //BindPipeline(g_renderstate.defaultPipeline, WGPUPrimitiveTopology_TriangleList);
}
extern "C" void BindPipelineWithSettings(DescribedPipeline* pipeline, PrimitiveType drawMode, RenderSettings settings){
    pipeline->state.primitiveType = drawMode;
    pipeline->state.settings = settings;
    pipeline->activePipeline = pipeline->pipelineCache.getOrCreate(pipeline->state, pipeline->shaderModule, pipeline->bglayout, pipeline->layout);
    wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (WGVKRenderPipeline)pipeline->activePipeline);
    wgvkRenderPassEncoderSetBindGroup((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGVKBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup));
}
extern "C" void BindPipeline(DescribedPipeline* pipeline, PrimitiveType drawMode){
    BindPipelineWithSettings(pipeline, drawMode, g_renderstate.currentSettings);
    
    //pipeline->lastUsedAs = drawMode;
    //wgvkRenderPassEncoderSetBindGroup ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGPUBindGroup)GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);

}
WGVKBuffer tttIntermediate zeroinit;
extern "C" void CopyTextureToTexture(Texture source, Texture dest){
    
    VkImageBlit region zeroinit;
    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.srcSubresource.baseArrayLayer = 0;
    region.srcSubresource.mipLevel = 0;
    region.srcSubresource.layerCount = 1;

    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.dstSubresource.baseArrayLayer = 0;
    region.dstSubresource.mipLevel = 0;
    region.dstSubresource.layerCount = 1;

    region.srcOffsets[1] = VkOffset3D{(int32_t)source.width, (int32_t)source.height, 1};
    region.dstOffsets[1] = VkOffset3D{(int32_t)source.width, (int32_t)source.height, 1};
    WGVKTexture sourceTexture = reinterpret_cast<WGVKTexture>(source.id);
    WGVKTexture destTexture = reinterpret_cast<WGVKTexture>(dest.id);
    WGVKCommandEncoder encodingDest = reinterpret_cast<WGVKCommandEncoder>(g_renderstate.computepass.cmdEncoder);
    encodingDest->initializeOrTransition(sourceTexture, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    encodingDest->initializeOrTransition(destTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    
    vkCmdBlitImage(
        encodingDest->buffer, 
        sourceTexture->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        destTexture->image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST
    );
    
    //wgvkCommandEncoderTransitionTextureLayout(encodingDest, destTexture, destTexture->layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //if(!tttIntermediate){
    //    BufferDescriptor desc zeroinit;
    //    desc.size = source.width * source.height * GetPixelSizeInBytes(source.format);
    //    desc.usage = BufferUsage_CopyDst | BufferUsage_CopySrc;
    //    tttIntermediate = wgvkDeviceCreateBuffer(g_vulkanstate.device, &desc);
    //}
    //TRACELOG(LOG_FATAL, "Unimplemented function: %s", __FUNCTION__);
}
void ComputepassEndOnlyComputing(cwoid){
    WGVKComputePassEncoder computePassEncoder = (WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder;
    wgvkCommandEncoderEndComputePass(computePassEncoder);
}

extern "C" void EndRenderpassEx(DescribedRenderpass* rp){
    drawCurrentBatch();
    if(rp->rpEncoder){
        wgvkRenderPassEncoderEnd((WGVKRenderPassEncoder)rp->rpEncoder);
    }
    VkImageMemoryBarrier rpAttachmentBarriers[2] zeroinit;
    auto rtex = g_renderstate.renderTargetStack.peek();
    rpAttachmentBarriers[0].image = reinterpret_cast<WGVKTexture>(rtex.texture.id)->image;
    rpAttachmentBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    rpAttachmentBarriers[0].oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rpAttachmentBarriers[0].newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    rpAttachmentBarriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    rpAttachmentBarriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    rpAttachmentBarriers[0].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    rpAttachmentBarriers[0].subresourceRange.baseMipLevel = 0;
    rpAttachmentBarriers[0].subresourceRange.levelCount = 1;
    rpAttachmentBarriers[0].subresourceRange.baseArrayLayer = 0;
    rpAttachmentBarriers[0].subresourceRange.layerCount = 1;
    rpAttachmentBarriers[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    rpAttachmentBarriers[0].dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    rpAttachmentBarriers[0].subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(((WGVKCommandEncoder)rp->cmdEncoder)->buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, 0, 0, 1, rpAttachmentBarriers);
    


    WGVKCommandBuffer cbuffer = wgvkCommandEncoderFinish((WGVKCommandEncoder)rp->cmdEncoder);
    
    g_renderstate.activeRenderpass = nullptr;
    VkSubmitInfo sinfo{};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &cbuffer->buffer;


    wgvkQueueSubmit(g_vulkanstate.queue, 1, &cbuffer);


    WGVKRenderPassEncoder rpe = (WGVKRenderPassEncoder)rp->rpEncoder;
    if(rpe){
        wgvkReleaseRenderPassEncoder(rpe);
    }
    WGVKCommandEncoder cmdEncoder = (WGVKCommandEncoder)rp->cmdEncoder;
    rp->cmdEncoder = nullptr;
    wgvkReleaseCommandEncoder(cmdEncoder);
    wgvkReleaseCommandBuffer(cbuffer);
    //vkResetFences(g_vulkanstate.device, 1, &g_vulkanstate.queue.syncState.renderFinishedFence);
    //g_vulkanstate.queue.syncState.submitsInThisFrame = 0;
    //vkDestroyFence(g_vulkanstate.device, fence, nullptr);

}

void UpdateTexture(Texture tex, void* data){
    WGVKTexelCopyTextureInfo destination{};
    destination.texture = (WGVKTexture)tex.id;
    destination.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    destination.mipLevel = 0;
    destination.origin = WGVKOrigin3D{0,0,0};

    WGVKTexelCopyBufferLayout source{};
    source.offset = 0;
    source.bytesPerRow = GetPixelSizeInBytes(tex.format) * tex.width;
    source.rowsPerImage = tex.height;
    WGVKExtent3D writeSize{};
    writeSize.depthOrArrayLayers = 1;
    writeSize.width = tex.width;
    writeSize.height = tex.height;
    wgvkQueueWriteTexture(g_vulkanstate.queue, &destination, data, tex.width * tex.height * GetPixelSizeInBytes(tex.format), &source, &writeSize);
}



extern "C" void EndRenderpassPro(DescribedRenderpass* rp, bool renderTexture){
    //if(renderTexture){
    //    wgvkRenderPassEncoderEnd((WGVKRenderPassEncoder)rp->rpEncoder);
    //    wgvkReleaseRenderPassEncoder((WGVKRenderPassEncoder)rp->rpEncoder);
    //    WGVKTexture ctarget = (WGVKTexture)GetActiveColorTarget();
    //    wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)rp->cmdEncoder, ctarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //    rp->rpEncoder = nullptr;
    //}
    EndRenderpassEx(rp);
}
void DispatchCompute(uint32_t x, uint32_t y, uint32_t z){
    wgvkComputePassEncoderDispatchWorkgroups((WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder, x, y, z);
}
const char* vkErrorString(VkResult code){
    switch(code){
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_NOT_PERMITTED: return "VK_ERROR_NOT_PERMITTED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return "VK_INCOMPATIBLE_SHADER_BINARY_EXT";
        case VK_PIPELINE_BINARY_MISSING_KHR: return "VK_PIPELINE_BINARY_MISSING_KHR";
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR: return "VK_ERROR_NOT_ENOUGH_SPACE_KHR";
        default: return "<Unknown VkResult enum>";
    }
}