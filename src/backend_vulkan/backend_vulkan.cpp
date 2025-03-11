#include <config.h>
#include <raygpu.h>
#include <set>
#include <vulkan/vulkan_core.h>
#include <renderstate.inc>
#include "vulkan_internals.hpp"


VulkanState g_vulkanstate{};

void BufferData(DescribedBuffer* buffer, void* data, size_t size){
    if(buffer->size >= size){
        WGVKBuffer handle = (WGVKBuffer)buffer->buffer;
        void* udata = 0;
        VkResult mapresult = vkMapMemory(g_vulkanstate.device->device, handle->memory, 0, size, 0, &udata);
        if(mapresult == VK_SUCCESS){
            std::memcpy(data, udata, size);
            vkUnmapMemory(g_vulkanstate.device->device, handle->memory);
        }
        else{
            abort();
        }
        
        //vkBindBufferMemory()
    }
}


void ResetSyncState(){
    g_vulkanstate.queue->syncState.submitsInThisFrame = 0;
    //g_vulkanstate.syncState.semaphoresInThisFrame.clear();
}
void PresentSurface(FullSurface* surface){
    //static VkSemaphore transitionSemaphore[framesInFlight] = {CreateSemaphore()};
    WGVKSurface wgvksurf = (WGVKSurface)surface->surface;
    uint32_t cacheIndex = wgvksurf->device->submittedFrames % framesInFlight;

    VkSubmitInfo si zeroinit;
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.signalSemaphoreCount = 1;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    si.pWaitDstStageMask = &stage;
    si.pWaitSemaphores = &g_vulkanstate.queue->syncState.getSemaphoreOfSubmitIndex(0);
    si.pSignalSemaphores = &g_vulkanstate.queue->syncState.getSemaphoreOfSubmitIndex(1);
    si.pCommandBuffers = nullptr;
    si.commandBufferCount = 0;
    vkQueueSubmit(g_vulkanstate.queue->graphicsQueue, 1, &si, VK_NULL_HANDLE);

    if(g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore == 0){
        g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore = CreateSemaphore();
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 2;
    VkSemaphore waiton[2] = {
        g_vulkanstate.queue->syncState.semaphoresInThisFrame[1],
        g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore
    };
    presentInfo.pWaitSemaphores = waiton;
    VkSwapchainKHR swapChains[] = {wgvksurf->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &wgvksurf->activeImageIndex;
    VkImageSubresourceRange isr{};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;

    if(wgvksurf->device->frameCaches[cacheIndex].finalTransitionBuffer == 0){
        VkCommandBufferAllocateInfo cbai zeroinit;
        cbai.commandBufferCount = 1;
        cbai.commandPool = wgvksurf->device->frameCaches[cacheIndex].commandPool;
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(wgvksurf->device->device, &cbai, &wgvksurf->device->frameCaches[cacheIndex].finalTransitionBuffer);
    }

    VkCommandBuffer transitionBuffer = wgvksurf->device->frameCaches[cacheIndex].finalTransitionBuffer;
    
    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(transitionBuffer, &beginInfo);
    
    EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, wgvksurf->images[wgvksurf->activeImageIndex]);
    vkEndCommandBuffer(transitionBuffer);
    VkSubmitInfo cbsinfo zeroinit;
    cbsinfo.commandBufferCount = 1;
    cbsinfo.pCommandBuffers = &transitionBuffer;
    cbsinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    cbsinfo.signalSemaphoreCount = 1;
    cbsinfo.pSignalSemaphores = &g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionSemaphore;

    vkQueueSubmit(wgvksurf->device->queue->graphicsQueue, 1, &cbsinfo, VK_NULL_HANDLE);
    //vkCreateCommandPool(g_vulkanstate.device->device, &pci, nullptr, &oof);
    //TransitionImageLayout(g_vulkanstate.device, oof, g_vulkanstate.queue->graphicsQueue, wgvksurf->images[wgvksurf->activeImageIndex], VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    
    //vkDestroyCommandPool(g_vulkanstate.device->device, oof, nullptr);
    VkResult presentRes = vkQueuePresentKHR(g_vulkanstate.queue->presentQueue, &presentInfo);
    ++wgvksurf->device->submittedFrames;
    if(presentRes != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "presentRes is %d", presentRes);
    }
    PostPresentSurface();

}
void PostPresentSurface(){
    WGVKDevice surfaceDevice = g_vulkanstate.device;
    WGVKQueue queue = surfaceDevice->queue;
    uint64_t submittedFrames = surfaceDevice->submittedFrames;
    uint32_t cacheIndex = submittedFrames % framesInFlight;

    if(queue->pendingCommandBuffers[cacheIndex].size() > 0){
        std::vector<VkFence> fences;
        fences.reserve(queue->pendingCommandBuffers[cacheIndex].size());
        for(auto [fence, bufferset] : queue->pendingCommandBuffers[cacheIndex]){
            fences.push_back(fence);
        }
        vkWaitForFences(surfaceDevice->device, fences.size(), fences.data(), VK_TRUE, 1 << 25);
    }

    for(auto [fence, bufferset] : queue->pendingCommandBuffers[cacheIndex]){
        vkDestroyFence(surfaceDevice->device, fence, nullptr);
        for(auto buffer : bufferset){
            rassert(buffer->refCount == 1, "CommandBuffer still in use after submit");
            wgvkReleaseCommandBuffer(buffer);
        }
    }

    queue->pendingCommandBuffers[cacheIndex].clear();
    wgvkReleaseCommandEncoder(queue->presubmitCache);
    vkResetCommandPool(surfaceDevice->device, surfaceDevice->frameCaches[cacheIndex].commandPool, 0);
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    queue->presubmitCache = wgvkDeviceCreateCommandEncoder(surfaceDevice, &cedesc);
}

DescribedBuffer* GenBufferEx(const void *data, size_t size, BufferUsage usage){

    VkBufferUsageFlags vusage = toVulkanBufferUsage(usage);
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    VkBuffer vertexBuffer{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = vusage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_vulkanstate.device->device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "failed to create vertex buffer!");
    }
    VkDeviceMemory vertexBufferMemory;
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device->device, vertexBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (vkAllocateMemory(g_vulkanstate.device->device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device->device, vertexBuffer, vertexBufferMemory, 0);
    if(data != nullptr){
        void* mapdata;
        VkResult vkres = vkMapMemory(g_vulkanstate.device->device, vertexBufferMemory, 0, bufferInfo.size, 0, &mapdata);
        if(vkres != VK_SUCCESS)abort();
        memcpy(mapdata, data, (size_t)bufferInfo.size);
        vkUnmapMemory(g_vulkanstate.device->device, vertexBufferMemory);
    }

    ret->buffer = callocnewpp(WGVKBufferImpl);
    ((WGVKBuffer)ret->buffer)->buffer = vertexBuffer;
    ((WGVKBuffer)ret->buffer)->memory = vertexBufferMemory;
    ((WGVKBuffer)ret->buffer)->refCount = 1;
    //vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, size, void **ppData)
    ret->size = bufferInfo.size;
    ret->usage = usage;

    //void* mdata;
    //vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mdata);
    //memcpy(mdata, data, (size_t)bufferInfo.size);
    //vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    return ret;
}

extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t width, uint32_t height){
    fsurface->surfaceConfig.width = width;
    fsurface->surfaceConfig.height = height;

    wgvkSurfaceConfigure((WGVKSurface)fsurface->surface, &fsurface->surfaceConfig);
    UnloadTexture(fsurface->renderTarget.depth);
    fsurface->renderTarget.depth = LoadTexturePro(width, height, Depth32, TextureUsage_RenderAttachment, 1, 1);
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
    wgvkQueueSubmit((WGVKQueue)g_vulkanstate.queue, 1, &command);
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

    sci.anisotropyEnable = true;
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

    uint32_t imageIndex = ~0;
    //VkCommandPool oof{};
    //VkCommandPoolCreateInfo pci zeroinit;
    //pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //vkCreateCommandPool(g_vulkanstate.device->device, &pci, nullptr, &oof);
    WGVKSurface wgvksurf = ((WGVKSurface)fsurface->surface);
    //TODO: Multiple frames in flight, this amounts to replacing 0 with frameCount % 2 or something similar
    VkResult acquireResult = vkAcquireNextImageKHR(g_vulkanstate.device->device, wgvksurf->swapchain, UINT64_MAX, g_vulkanstate.queue->syncState.getSemaphoreOfSubmitIndex(0), VK_NULL_HANDLE, &imageIndex);
    if(acquireResult != VK_SUCCESS){
        std::cerr << "acquireResult is " << acquireResult << std::endl;
    }
    WGVKDevice surfaceDevice = ((WGVKSurface)fsurface->surface)->device;
    VkCommandBuffer buf = ((WGVKSurface)fsurface->surface)->device->queue->presubmitCache->buffer;//BeginSingleTimeCommands(surfaceDevice, oof);
    EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, wgvksurf->images[imageIndex]);
    EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (WGVKTexture)fsurface->renderTarget.depth.id);
    //EndSingleTimeCommandsAndSubmit(surfaceDevice, oof, g_vulkanstate.queue->graphicsQueue, buf);
    //vkDestroyCommandPool(g_vulkanstate.device->device, oof, nullptr);
    
    fsurface->renderTarget.texture.id = wgvksurf->images[imageIndex];
    fsurface->renderTarget.texture.view = wgvksurf->imageViews[imageIndex];
    fsurface->renderTarget.texture.width = wgvksurf->width;
    fsurface->renderTarget.texture.height = wgvksurf->height;
    
    wgvksurf->activeImageIndex = imageIndex;
}
void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle){
    g_renderstate.throttled_PresentMode = PresentMode_Fifo;
    g_renderstate.unthrottled_PresentMode = PresentMode_Immediate;
    g_renderstate.frameBufferFormat = BGRA8;
}
void* GetInstance(){
    return g_vulkanstate.instance;
}
void* GetDevice(){
    return g_vulkanstate.device;
}
void* GetAdapter(){
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
        __builtin_trap();
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
    const char* const* windowExtensions = nullptr;
    #if SUPPORT_GLFW == 1
    windowExtensions = glfwGetRequiredInstanceExtensions(&requiredGLFWExtensions);
    #elif SUPPORT_SDL2 == 1
    auto dummywindow = SDL_CreateWindow("dummy", 0, 0, 1, 1, SDL_WINDOW_HIDDEN | SDL_WINDOW_VULKAN);
    SDL_bool res = SDL_Vulkan_GetInstanceExtensions(dummywindow, &requiredGLFWExtensions, windowExtensions);
    assert(res && "SDL extensions error");
    windowExtensions = (const char**)std::calloc(requiredGLFWExtensions, sizeof(char*));
    res = SDL_Vulkan_GetInstanceExtensions(dummywindow, &requiredGLFWExtensions, windowExtensions);
    assert(res && "SDL extensions error");
    #elif SUPPORT_SDL3
    windowExtensions = SDL_Vulkan_GetInstanceExtensions(&requiredGLFWExtensions);
    #endif
    #if SUPPORT_GLFW == 1 || SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1
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
            //#ifndef NDEBUG
            TRACELOG(LOG_INFO, "Selecting Validation Layer %s",layer.layerName);
            //validationLayers.push_back(layer.layerName);
            //#else
            //TRACELOG(LOG_INFO, "Validation Layer %s available but not selections since NDEBUG is defined",layer.layerName);
            //#endif
        }
    }

    VkInstanceCreateInfo createInfo{};

    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

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
        TRACELOG(LOG_FATAL, "Failed to create Vulkan instance : %d", (int)instanceCreation);
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



// Function to pick a suitable physical device (GPU)
VkPhysicalDevice pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        TRACELOG(LOG_FATAL, "Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, devices.data());
    VkPhysicalDevice ret{};
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        TRACELOG(LOG_INFO, "Found device: %s", props.deviceName);
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ret = device;
            goto picked;
        }
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
            ret = device;
            goto picked;
        }
    }

    
    
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            ret = device;
            goto picked;
        }
    }
    

    if (g_vulkanstate.physicalDevice == VK_NULL_HANDLE) {
        TRACELOG(LOG_FATAL, "Failed to find a suitable GPU!");
    }
picked:
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
    return ret;
    //VkPhysicalDeviceExtendedDynamicState3PropertiesEXT ext3{};
    //ext3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    //VkPhysicalDeviceProperties2 props2{};
    //props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    //props2.pNext = &ext3;
    //VkPhysicalDeviceExtendedDynamicState3FeaturesEXT ext{};
    //vkGetPhysicalDeviceProperties2(g_vulkanstate.physicalDevice, &props2);
    //std::cout << "Extended support: " << ext3.dynamicPrimitiveTopologyUnrestricted << "\n";
    //(void)0;
}


// Function to find queue families

QueueIndices findQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    QueueIndices ret{
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX
    };
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Iterate through each queue family and check for the desired capabilities
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        // Check for graphics support
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && ret.graphicsIndex == UINT32_MAX) {
            ret.graphicsIndex = i;
        }
        // Check for presentation support
        #ifdef MAIN_WINDOW_SDL3
        bool presentSupport = true;//SDL_Vulkan_GetPresentationSupport(g_vulkanstate.instance, g_vulkanstate.physicalDevice, i);
        #elif defined(MAIN_WINDOW_SDL2) //uuh 
        VkBool32 presentSupport = VK_TRUE;
        #elif defined(MAIN_WINDOW_GLFW)
        VkBool32 presentSupport = glfwGetPhysicalDevicePresentationSupport(g_vulkanstate.instance, g_vulkanstate.physicalDevice, i) ? VK_TRUE : VK_FALSE;
        #else
        VkBool32 presentSupport = VK_FALSE;
        #endif
        //vkGetPhysicalDeviceSurfaceSupportKHR(g_vulkanstate.physicalDevice, i, g_vulkanstate.surface.surface, &presentSupport);
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
extern "C" DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* descs, uint32_t uniformCount, bool compute){
    DescribedBindGroupLayout retv{};
    DescribedBindGroupLayout* ret = &retv;
    WGVKBindGroupLayout retlayout = wgvkDeviceCreateBindGroupLayout(g_vulkanstate.device, descs, uniformCount);
    ret->layout = retlayout;
    ret->entries = (ResourceTypeDescriptor*)std::calloc(uniformCount, sizeof(ResourceTypeDescriptor));
    std::memcpy(const_cast<ResourceTypeDescriptor*>(ret->entries), descs, uniformCount * sizeof(ResourceTypeDescriptor));
    ret->entryCount = uniformCount;
    std::vector<ResourceTypeDescriptor> a(retlayout->entries, retlayout->entries + ret->entryCount);
    return retv;
}
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
        vkCmdBlitImage(enc->buffer, wgvkTex->image, wgvkTex->layout, wgvkTex->image, wgvkTex->layout, 1, &blitRegion, VK_FILTER_LINEAR);
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
    BufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Uniform;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    //wgvkBuffer->refCount++;
    wgvkQueueWriteBuffer(g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgvkReleaseBuffer(wgvkBuffer);
}
extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->buffer != nullptr && buffer->size >= size){
        wgvkQueueWriteBuffer(g_vulkanstate.queue, (WGVKBuffer)buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgvkReleaseBuffer((WGVKBuffer)buffer->buffer);
        BufferDescriptor nbdesc zeroinit;
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgvkDeviceCreateBuffer((WGVKDevice)GetDevice(), &nbdesc);
        buffer->size = size;
        wgvkQueueWriteBuffer(g_vulkanstate.queue, (WGVKBuffer)buffer->buffer, 0, data, size);
    }
}

void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    BufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Storage;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    wgvkQueueWriteBuffer(g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgvkReleaseBuffer(wgvkBuffer);
}


extern "C" void UnloadBuffer(DescribedBuffer* buf){
    WGVKBuffer handle = (WGVKBuffer)buf->buffer;
    wgvkReleaseBuffer(handle);
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
    wgvkRenderPassEncoderBindIndexBuffer((WGVKRenderPassEncoder)drp->rpEncoder, (WGVKBuffer)buffer->buffer, 0, toVulkanIndexFormat(format));
    //wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)drp->rpEncoder, (WGPUBuffer)buffer->buffer, format, offset, buffer->size);
}
extern "C" void RenderPassSetVertexBuffer(DescribedRenderpass* drp, uint32_t slot, DescribedBuffer* buffer, uint64_t offset){
    wgvkRenderPassEncoderBindVertexBuffer((WGVKRenderPassEncoder)drp->rpEncoder, slot, (WGVKBuffer)buffer->buffer, offset);
    //wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, buffer->size);
}
extern "C" void RenderPassSetBindGroup(DescribedRenderpass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgvkRenderPassEncoderBindDescriptorSet((WGVKRenderPassEncoder)drp->rpEncoder, group, (WGVKBindGroup)bindgroup->bindGroup);
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

// Function to create logical device and retrieve queues
std::pair<WGVKDevice, WGVKQueue> createLogicalDevice(VkPhysicalDevice physicalDevice, QueueIndices indices) {
    // Find queue families
    QueueIndices qind = findQueueFamilies();
    std::pair<WGVKDevice, WGVKQueue> ret{};
    // Collect unique queue families
    std::vector<uint32_t> queueFamilies;
    {
        std::set<uint32_t> uniqueQueueFamilies; // = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

        uniqueQueueFamilies.insert(indices.computeIndex);
        uniqueQueueFamilies.insert(indices.graphicsIndex);
        uniqueQueueFamilies.insert(indices.presentIndex);
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
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> deprops(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, deprops.data());
    
    for(auto e : deprops){
        //std::cout << e.extensionName << ", ";
    }
    // Specify device extensions
    std::vector<const char *> deviceExtensions = {
        #ifndef FORCE_HEADLESS
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
        #endif
        // Add other required extensions here
    };

    // Specify device features
    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    props.dynamicPrimitiveTopologyUnrestricted = VK_TRUE;
    
    
    VkPhysicalDeviceFeatures deviceFeatures{};


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    //createInfo.pNext = &features;
    //features.pNext = &props;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // (Optional) Enable validation layers for device-specific debugging
    ret.first = callocnewpp(WGVKDeviceImpl);
    ret.second = callocnewpp(WGVKQueueImpl);
    auto dcresult = vkCreateDevice(g_vulkanstate.physicalDevice, &createInfo, nullptr, &(ret.first->device));
    if (dcresult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create logical device!");
    } else {
        TRACELOG(LOG_INFO, "Successfully created logical device");
    }

    // Retrieve and assign queues
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
        VkCommandPoolCreateInfo pci zeroinit;
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCreateCommandPool(ret.first->device, &pci, nullptr, &ret.first->frameCaches[i].commandPool);
    }
    ret.second->presubmitCache = wgvkDeviceCreateCommandEncoder(ret.first, &cedesc);
    //__builtin_dump_struct(&g_vulkanstate, printf);
    // std::cin.get();

    

    TRACELOG(LOG_INFO, "Successfully retrieved queues");
    {
        auto [device, queue] = ret;
        device->queue = queue;
        queue->device = device;
    }
    return ret;
}
extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    WGVKBindGroup bindGroup = (WGVKBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup);
    wgvkComputePassEncoderSetPipeline ((WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder,    (VkPipeline)pipeline->pipeline, (VkPipelineLayout)pipeline->layout);
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
void InitBackend(){
    #if SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1
    SDL_Init(SDL_INIT_VIDEO);
    #endif
    #if SUPPORT_GLFW
    glfwInit();
    #endif
    g_vulkanstate.instance = createInstance();
    g_vulkanstate.physicalDevice = pickPhysicalDevice();
    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice, &g_vulkanstate.memProperties);
    g_vulkanstate.memoryTypes = discoverMemoryTypes(g_vulkanstate.physicalDevice);
    QueueIndices queues = findQueueFamilies();
    g_vulkanstate.graphicsFamily = queues.graphicsIndex;
    g_vulkanstate.computeFamily = queues.computeIndex;
    g_vulkanstate.presentFamily = queues.presentIndex;
    auto device_and_queues = createLogicalDevice(g_vulkanstate.physicalDevice, queues);
    g_vulkanstate.device = device_and_queues.first;
    g_vulkanstate.queue = device_and_queues.second;
    g_vulkanstate.queue->syncState.semaphoresInThisFrame.resize(10);
    for(uint32_t i = 0;i < 10;i++){
        g_vulkanstate.queue->syncState.semaphoresInThisFrame[i] = CreateSemaphore(0);
    }
    //g_vulkanstate.syncState.imageAvailableSemaphores[0] = CreateSemaphore(0);
    //g_vulkanstate.syncState.presentSemaphores[0] = CreateSemaphore(0);
    g_vulkanstate.queue->syncState.renderFinishedFence = CreateFence(0);

    
    createRenderPass();
}
const VkSemaphore& SyncState::getSemaphoreOfSubmitIndex(uint32_t index){
    uint32_t capacity = semaphoresInThisFrame.capacity();
    if(semaphoresInThisFrame.size() <= index){
        semaphoresInThisFrame.resize(index + 1);
    }
    if(semaphoresInThisFrame.size() > capacity){
        for(uint32_t i = capacity;i < semaphoresInThisFrame.size();i++){
            semaphoresInThisFrame[i] = CreateSemaphore(0);
        }
    }
    return semaphoresInThisFrame[index];
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

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    WGVKCommandEncoderDescriptor cedesc zeroinit;

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

    auto rtex = g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition];
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
        if(rca.view->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.view->texture, rca.view->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
        if(rca.resolveTarget->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.resolveTarget->texture, rca.resolveTarget->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }
    else{
        rca.view = (WGVKTextureView)rtex.texture.view;
        if(rca.view->texture->layout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL){
            wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)renderPass->cmdEncoder, rca.view->texture, rca.view->texture->layout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    }

    rpdesc.depthStencilAttachment = &dsa;
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
extern "C" void BindPipeline(DescribedPipeline* pipeline, PrimitiveType drawMode){

    switch(drawMode){
        case RL_TRIANGLES:
            wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (VkPipeline)pipeline->quartet.pipeline_TriangleList, (VkPipelineLayout)pipeline->layout.layout);
        break;
        case RL_TRIANGLE_STRIP:
            wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (VkPipeline)pipeline->quartet.pipeline_TriangleStrip, (VkPipelineLayout)pipeline->layout.layout);
        break;
        case RL_LINES:
            wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (VkPipeline)pipeline->quartet.pipeline_LineList, (VkPipelineLayout)pipeline->layout.layout);
        break;
        case RL_POINTS:
            wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (VkPipeline)pipeline->quartet.pipeline_PointList, (VkPipelineLayout)pipeline->layout.layout);
        break;
        default:
            assert(false && "Unsupported Drawmode");
            abort();
    }
    //pipeline->lastUsedAs = drawMode;
    wgvkRenderPassEncoderBindDescriptorSet((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGVKBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup));
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
    if(sourceTexture->layout != VK_IMAGE_LAYOUT_GENERAL && sourceTexture->layout != VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL){
        wgvkCommandEncoderTransitionTextureLayout(encodingDest, sourceTexture, sourceTexture->layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    }

    wgvkCommandEncoderTransitionTextureLayout(encodingDest, destTexture, destTexture->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    
    vkCmdBlitImage(
        encodingDest->buffer, 
        sourceTexture->image,
        sourceTexture->layout,
        destTexture->image, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST
    );
    
    wgvkCommandEncoderTransitionTextureLayout(encodingDest, destTexture, destTexture->layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if(!tttIntermediate){
        BufferDescriptor desc zeroinit;
        desc.size = source.width * source.height * GetPixelSizeInBytes(source.format);
        desc.usage = BufferUsage_CopyDst | BufferUsage_CopySrc;
        tttIntermediate = wgvkDeviceCreateBuffer(g_vulkanstate.device, &desc);
    }
    //TRACELOG(LOG_FATAL, "Unimplemented function: %s", __FUNCTION__);
}
void ComputepassEndOnlyComputing(cwoid){
    WGVKComputePassEncoder computePassEncoder = (WGVKComputePassEncoder)g_renderstate.computepass.cpEncoder;
    wgvkCommandEncoderEndComputePass(computePassEncoder);
}

extern "C" void EndRenderpassEx(DescribedRenderpass* rp){
    if(rp->rpEncoder){
        wgvkRenderPassEncoderEnd((WGVKRenderPassEncoder)rp->rpEncoder);
    }
    WGVKCommandBuffer cbuffer = wgvkCommandEncoderFinish((WGVKCommandEncoder)rp->cmdEncoder);

    g_renderstate.activeRenderpass = nullptr;
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    VkSubmitInfo sinfo{};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &cbuffer->buffer;
    sinfo.waitSemaphoreCount = 1;
    VkSemaphore waitsemaphore    = g_vulkanstate.queue->syncState.getSemaphoreOfSubmitIndex(g_vulkanstate.queue->syncState.submitsInThisFrame);
    VkSemaphore signalesemaphore = g_vulkanstate.queue->syncState.getSemaphoreOfSubmitIndex(g_vulkanstate.queue->syncState.submitsInThisFrame + 1);
    sinfo.pWaitSemaphores = &waitsemaphore;
    sinfo.pWaitDstStageMask = &stageMask;
    sinfo.signalSemaphoreCount = 1;
    sinfo.pSignalSemaphores = &signalesemaphore;
    ++g_vulkanstate.queue->syncState.submitsInThisFrame;
    //VkFence fence{};
    //VkFenceCreateInfo finfo{};
    //finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //if(vkCreateFence(g_vulkanstate.device, &finfo, nullptr, &fence) != VK_SUCCESS){
    //    TRACELOG(LOG_FATAL, "Could not create fence");
    //}
    wgvkQueueSubmit(g_vulkanstate.queue, 1, &cbuffer);
    //if(vkQueueSubmit(g_vulkanstate.queue.graphicsQueue, 1, &sinfo, g_vulkanstate.queue.syncState.renderFinishedFence) != VK_SUCCESS){
    //    TRACELOG(LOG_FATAL, "Could not submit commandbuffer");
    //}
    //if(vkWaitForFences(g_vulkanstate.device, 1, &g_vulkanstate.queue.syncState.renderFinishedFence, VK_TRUE, ~0) != VK_SUCCESS){
    //    TRACELOG(LOG_FATAL, "Could not wait for fence");
    //}
    WGVKRenderPassEncoder rpe = (WGVKRenderPassEncoder)rp->rpEncoder;
    if(rpe){
        wgvkReleaseRenderPassEncoder(rpe);
    }
    WGVKCommandEncoder cmdEncoder = (WGVKCommandEncoder)rp->cmdEncoder;
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
