#include <wgvk.h>
#include <config.h>
#define Font rlFont
#include <raygpu.h>
#undef Font
#include <set>
#include <external/volk.h>
#include <renderstate.hpp>
#include "vulkan_internals.hpp"
#include <wgvk_structs_impl.h>
#if SUPPORT_RGFW == 1
    #define RGFW_VULKAN
    #include <external/RGFW.h>
#endif
VulkanState g_vulkanstate{};

void PresentSurface(FullSurface* surface){
    wgpuSurfacePresent((WGPUSurface)surface->surface);
    WGPUSurface wgpusurf = (WGPUSurface)surface->surface;
}
void DummySubmitOnQueue(){
    return;
    const uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    const uint32_t submits = g_vulkanstate.queue->syncState[cacheIndex].submits;
    if(g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence == 0){
        abort();
        //VkFenceCreateInfo vci = {
        //    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        //    .pNext = NULL,
        //    .flags = 0,
        //};
        //vkCreateFence(g_vulkanstate.device->device, &vci, nullptr, &g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence);
    }
    VkSubmitInfo emptySubmit zeroinit;
    emptySubmit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    emptySubmit.waitSemaphoreCount = 1;
    rassert(g_vulkanstate.queue->syncState[cacheIndex].semaphores.size > submits, "Too many submits (more than semaphores). This is an internal bug");
    emptySubmit.pWaitSemaphores = g_vulkanstate.queue->syncState[cacheIndex].semaphores.data + submits;
    VkPipelineStageFlags waitmask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    emptySubmit.pWaitDstStageMask = &waitmask;
    g_vulkanstate.device->functions.vkQueueSubmit(g_vulkanstate.device->queue->graphicsQueue, 1, &emptySubmit, g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence->fence);
    g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence->state = WGPUFenceState_InUse;
    WGPUCommandBufferVector insert{};

    PendingCommandBufferMap* pcmap = &g_vulkanstate.queue->pendingCommandBuffers[cacheIndex];
    WGPUFence ftf = g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence;
    PendingCommandBufferMap_put(pcmap, ftf, insert);
    WGPUCommandBufferVector* inserted = PendingCommandBufferMap_get(pcmap, ftf);
    WGPUCommandBufferVector_init(inserted);
    //g_vulkanstate.queue->pendingCommandBuffers[cacheIndex].emplace(g_vulkanstate.queue->device->frameCaches[cacheIndex].finalTransitionFence, std::unordered_set<WGPUCommandBuffer>{});
}

extern "C" DescribedBuffer* GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage){
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    WGPUBufferDescriptor descriptor{};
    descriptor.size = size;
    descriptor.usage = usage;
    ret->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor);
    ret->size = size;
    ret->usage = usage;
    if(data != nullptr){
        wgpuQueueWriteBuffer(g_vulkanstate.queue, (WGPUBuffer)ret->buffer, 0, data, size);
    }
    return ret;
}

extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t width, uint32_t height){
    vkQueueWaitIdle(reinterpret_cast<WGPUSurface>(fsurface->surface)->device->queue->graphicsQueue);
    vkQueueWaitIdle(reinterpret_cast<WGPUSurface>(fsurface->surface)->device->queue->presentQueue);
    VkSemaphoreWaitInfo info;
    fsurface->surfaceConfig.width = width;
    fsurface->surfaceConfig.height = height;

    wgpuSurfaceConfigure((WGPUSurface)fsurface->surface, &fsurface->surfaceConfig);
    width = fsurface->surface->images[0]->width;
    height = fsurface->surface->images[0]->height;
    UnloadTexture(fsurface->renderTarget.depth);
    fsurface->renderTarget.depth = LoadTexturePro(width, height, Depth32, WGPUTextureUsage_RenderAttachment, GetDefaultSettings().sampleCount, 1);
    if(fsurface->renderTarget.depth.sampleCount > 1){
        fsurface->renderTarget.colorMultisample = LoadTexturePro(width, height, BGRA8, WGPUTextureUsage_RenderAttachment, GetDefaultSettings().sampleCount, 1);
    }
}

extern "C" void BeginComputepassEx(DescribedComputepass* computePass){
    computePass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), nullptr);
    computePass->cpEncoder = wgpuCommandEncoderBeginComputePass((WGPUCommandEncoder)computePass->cmdEncoder);
}

extern "C" void EndComputepassEx(DescribedComputepass* computePass){
    if(computePass->cpEncoder){
        wgpuComputePassEncoderEnd((WGPUComputePassEncoder)computePass->cpEncoder);
        wgpuComputePassEncoderRelease((WGPUComputePassEncoder)computePass->cpEncoder);
        computePass->cpEncoder = 0;
    }
    
    //TODO
    g_renderstate.activeComputepass = nullptr;
    WGPUCommandBufferDescriptor cBufferD = {
        .label = {
            .data = "ComputeBuffer",
            .length = WGPU_STRLEN
        }
    };
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)computePass->cmdEncoder, &cBufferD);
    wgpuQueueSubmit(g_vulkanstate.queue, 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)computePass->cmdEncoder);
}

extern "C" DescribedRenderpass LoadRenderpassEx(RenderSettings settings, bool colorClear, WGPUColor colorClearValue, bool depthClear, float depthClearValue){
    DescribedRenderpass ret{};

    VkRenderPassCreateInfo rpci{};
    ret.settings = settings;

    ret.colorClear = colorClearValue;
    ret.depthClear = depthClearValue;
    ret.colorLoadOp = colorClear ? WGPULoadOp_Clear : WGPULoadOp_Load;
    ret.colorStoreOp = WGPUStoreOp_Store;
    ret.depthLoadOp = depthClear ? WGPULoadOp_Clear : WGPULoadOp_Load;
    ret.depthStoreOp = WGPUStoreOp_Store;
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
    DescribedSampler ret zeroinit;// = callocnew(DescribedSampler);
    WGPUSamplerDescriptor sdesc zeroinit;
    
    sdesc.compare = WGPUCompareFunction_Less;
    sdesc.addressModeU = toWGPUAddressMode(amode);
    sdesc.addressModeV = toWGPUAddressMode(amode);
    sdesc.addressModeW = toWGPUAddressMode(amode);
    sdesc.lodMaxClamp = 10;
    sdesc.lodMinClamp = 0;
    sdesc.mipmapFilter = toWGPUMipmapFilterMode(fmode);
    sdesc.maxAnisotropy = static_cast<uint16_t>(maxAnisotropy);

    sdesc.minFilter = toWGPUFilterMode(fmode);
    sdesc.magFilter = toWGPUFilterMode(fmode);
    
    ret.magFilter = fmode;
    ret.minFilter = fmode;
    ret.addressModeU = amode;
    ret.addressModeV = amode;
    ret.addressModeW = amode;
    ret.maxAnisotropy = maxAnisotropy;
    ret.compare = WGPUCompareFunction_Less;//huh??
    ret.sampler = wgpuDeviceCreateSampler(g_vulkanstate.device, &sdesc);
    return ret;
}


extern "C" void GetNewTexture(FullSurface* fsurface){

    const size_t submittedframes = fsurface->surfaceConfig.device->submittedFrames;
    const uint32_t cacheIndex = fsurface->surfaceConfig.device->submittedFrames % framesInFlight;

    uint32_t imageIndex = ~0;
    if(fsurface->headless){
        //g_vulkanstate.queue->syncState[cacheIndex].submits = 0;
        VkSubmitInfo submitInfo zeroinit;
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphore;
        g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = true;
        vkQueueSubmit(g_vulkanstate.queue->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    }
    else{
        WGPUSurface wgpusurf = (WGPUSurface)fsurface->surface;
        WGPUDevice device = g_vulkanstate.device;
        //g_vulkanstate.queue->syncState[cacheIndex].submits = 0;
        
        VkResult acquireResult = g_vulkanstate.device->functions.vkAcquireNextImageKHR(
            g_vulkanstate.device->device,
            wgpusurf->swapchain,
            UINT64_MAX,
            g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphore,
            VK_NULL_HANDLE,
            &imageIndex
        );
        g_vulkanstate.queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = true;
        if(acquireResult != VK_SUCCESS){
            TRACELOG(LOG_WARNING, "vkAcquireNextImageKHR returned %s", vkErrorString(acquireResult));
        }
        WGPUDevice surfaceDevice = ((WGPUSurface)fsurface->surface)->device;
        VkCommandBuffer buf = ((WGPUSurface)fsurface->surface)->device->queue->presubmitCache->buffer;
        
        fsurface->renderTarget.texture.id = wgpusurf->images[imageIndex];
        
        WGPUTextureViewDescriptor tvDesc = {
            .nextInChain = {},
            .label = {},
            .format = WGPUTextureFormat_BGRA8Unorm,
            .dimension = WGPUTextureViewDimension_2D,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .aspect = WGPUTextureAspect_All,
            .usage = {},
        };
        fsurface->renderTarget.texture.view = wgpuTextureCreateView(wgpusurf->images[imageIndex], &tvDesc);
        fsurface->renderTarget.texture.width = wgpusurf->width;
        fsurface->renderTarget.texture.height = wgpusurf->height;

        wgpusurf->activeImageIndex = imageIndex;
    }
}
void negotiateSurfaceFormatAndPresentMode(WGPUAdapter adapter, const void* SurfaceHandle){
    WGPUSurface surface = (WGPUSurface)SurfaceHandle;
    uint32_t presentModeCount = 0;

    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter->physicalDevice, surface->surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter->physicalDevice, surface->surface, &presentModeCount, presentModes.data());
    
    if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_FIFO_RELAXED_KHR) != presentModes.end()){
        g_renderstate.throttled_PresentMode = WGPUPresentMode_FifoRelaxed;
    }
    else{
        g_renderstate.throttled_PresentMode = WGPUPresentMode_Fifo;
    }
    if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end()){
        g_renderstate.unthrottled_PresentMode = WGPUPresentMode_Mailbox;
    }
    else if(std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.end()){
        g_renderstate.unthrottled_PresentMode = WGPUPresentMode_Immediate;
    }
    else{
        g_renderstate.unthrottled_PresentMode = WGPUPresentMode_Fifo;
    }
    uint32_t surfaceFormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter->physicalDevice, surface->surface, &surfaceFormatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter->physicalDevice, surface->surface, &surfaceFormatCount, surfaceFormats.data());
    for(uint32_t i = 0;i < surfaceFormats.size();i++){
        VkSurfaceFormatKHR fmt = surfaceFormats[i];
        TRACELOG(LOG_INFO, "Supported surface formats: %d, %d, ", fmt.format, fmt.colorSpace);
    }
    TRACELOG(LOG_INFO, "\n");
    
    g_renderstate.frameBufferFormat = BGRA8;
}
void* GetInstance(){
    return g_vulkanstate.instance;
}
WGPUDevice GetDevice(){
    return g_vulkanstate.device;
}
WGPUQueue GetQueue  (cwoid){
    return g_vulkanstate.queue;
}
WGPUAdapter GetAdapter(){
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
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
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
            //TRACELOG(LOG_INFO, "Selecting Validation Layer %s",layer.layerName);
            //validationLayers.push_back(layer.layerName);
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
        if(preferredPhysicalDeviceTypes[i] == x)return (int)i;
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

QueueIndices findQueueFamilies(WGPUAdapter adapter) {
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
        bool presentSupport = SDL_Vulkan_GetPresentationSupport(g_vulkanstate.instance->instance, adapter->physicalDevice, i);
        #elif defined(MAIN_WINDOW_SDL2) //uuh 
        VkBool32 presentSupport = VK_TRUE;
        #elif defined(MAIN_WINDOW_GLFW)
        VkBool32 presentSupport = glfwGetPhysicalDevicePresentationSupport(g_vulkanstate.instance->instance, adapter->physicalDevice, i) ? VK_TRUE : VK_FALSE;
        #elif SUPPORT_RGFW == 1
        VkBool32 presentSupport = RGFW_getVKPresentationSupport_noinline(g_vulkanstate.instance->instance, adapter->physicalDevice, i);
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






static inline WGPUStorageTextureAccess toStorageTextureAccess(access_type acc){
    switch(acc){
        case access_type::readonly:return WGPUStorageTextureAccess_ReadOnly;
        case access_type::readwrite:return WGPUStorageTextureAccess_ReadWrite;
        case access_type::writeonly:return WGPUStorageTextureAccess_WriteOnly;
        default: rg_unreachable();
    }
    return WGPUStorageTextureAccess_Force32;
}
static inline WGPUBufferBindingType toStorageBufferAccess(access_type acc){
    switch(acc){
        case access_type::readonly: return WGPUBufferBindingType_ReadOnlyStorage;
        case access_type::readwrite: [[fallthrough]];
        case access_type::writeonly:return WGPUBufferBindingType_Storage;
        default: rg_unreachable();
    }
    return WGPUBufferBindingType_Force32;
}
static inline WGPUTextureFormat toStorageTextureFormat(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::format_r32float: return WGPUTextureFormat_R32Float;
        case format_or_sample_type::format_r32uint: return WGPUTextureFormat_R32Uint;
        case format_or_sample_type::format_rgba8unorm: return WGPUTextureFormat_RGBA8Unorm;
        case format_or_sample_type::format_rgba32float: return WGPUTextureFormat_RGBA32Float;
        default: rg_unreachable();
    }
    return WGPUTextureFormat_Force32;
}
static inline WGPUTextureSampleType toTextureSampleType(format_or_sample_type fmt){
    switch(fmt){
        case format_or_sample_type::sample_f32: return WGPUTextureSampleType_Float;
        case format_or_sample_type::sample_u32: return WGPUTextureSampleType_Uint;
        default: return WGPUTextureSampleType_Float;//rg_unreachable();
    }
    return WGPUTextureSampleType_Force32;
}
RGAPI DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, bool compute){
    DescribedBindGroupLayout ret{};
    WGPUShaderStage visible;
    WGPUShaderStage vfragmentOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Fragment;
    WGPUShaderStage vvertexOnly = compute ? WGPUShaderStage_Compute : WGPUShaderStage_Vertex;
    if(compute){
        visible = WGPUShaderStage_Compute;
    }
    else{
        visible = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    }

    
    WGPUBindGroupLayoutEntry* blayouts = (WGPUBindGroupLayoutEntry*)RL_CALLOC(uniformCount, sizeof(WGPUBindGroupLayoutEntry));
    WGPUBindGroupLayoutDescriptor bglayoutdesc{};

    for(size_t i = 0;i < uniformCount;i++){
        blayouts[i].binding = uniforms[i].location;
        auto& ui = uniforms[i];
        switch(uniforms[i].type){
            default:
                rg_unreachable();
            
            case uniform_buffer:
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                blayouts[i].buffer.minBindingSize = uniforms[i].minBindingSize;
            break;
            case storage_buffer:{
                blayouts[i].visibility = visible;
                blayouts[i].buffer.type = toStorageBufferAccess(uniforms[i].access);
                blayouts[i].buffer.minBindingSize = 0;
            }
            break;
            case texture2d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(uniforms[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;    
            break;
            case texture_sampler:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            case texture3d:
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].texture.sampleType = toTextureSampleType(uniforms[i].fstype);
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_3D;
            break;
            case storage_texture2d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2D;
            break;
            case storage_texture2d_array:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_2DArray;
            break;
            case storage_texture3d:
                blayouts[i].storageTexture.access = toStorageTextureAccess(uniforms[i].access);
                blayouts[i].visibility = vfragmentOnly;
                blayouts[i].storageTexture.format = toStorageTextureFormat(uniforms[i].fstype);
                blayouts[i].storageTexture.viewDimension = WGPUTextureViewDimension_3D;
            break;
            case acceleration_structure:
                blayouts[i].accelerationStructure = 1;
            break;
        }
    }
    bglayoutdesc.entryCount = uniformCount;
    bglayoutdesc.entries = blayouts;

    ret.entries = (WGPUBindGroupLayoutEntry*)std::calloc(uniformCount, sizeof(WGPUBindGroupLayoutEntry));
    if(uniformCount > 0){
        std::memcpy(ret.entries, uniforms, uniformCount * sizeof(WGPUBindGroupLayoutEntry));
    }
    ret.layout = wgpuDeviceCreateBindGroupLayout((WGPUDevice)GetDevice(), &bglayoutdesc);

    std::free(blayouts);
    return ret;
}

// Function to pick a suitable physical device (GPU)
WGPUAdapter pickPhysicalDevice(WGPUInstance instance) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance->instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        TRACELOG(LOG_FATAL, "Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance->instance, &deviceCount, devices.data());
    
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
    WGPUAdapter reta = callocnewpp(WGPUAdapterImpl);
    reta->physicalDevice = ret;
    reta->instance = instance;
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
    wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)drp->cpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, NULL);
}
void GenTextureMipmaps(Texture2D* tex){
    WGPUCommandEncoderDescriptor cdesc zeroinit;
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(g_vulkanstate.device, &cdesc);
    
    rassert(tex->mipmaps >= 1, "Mipmaps must always be at least 1, 0 probably means that's an invalid texture");
    rassert(tex->width < (uint32_t(1) << 31), "Texture too humongous");
    rassert(tex->height < (uint32_t(1) << 31), "Texture too humongous");
    
    WGPUTexture wgpuTex = reinterpret_cast<WGPUTexture>(tex->id);

    VkOffset3D initial = VkOffset3D{(int32_t)tex->width, (int32_t)tex->height, 1};
    auto mipExtent = [initial](const uint32_t mipLevel){
        VkOffset3D i = initial;
        i.x >>= mipLevel;
        i.y >>= mipLevel;
        i.x = std::max(i.x, 1);
        i.y = std::max(i.y, 1);
        return i;
    };
    ImageUsageSnap usage_general = {
        .layout = VK_IMAGE_LAYOUT_GENERAL,
        .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .access = VK_ACCESS_TRANSFER_WRITE_BIT,
        .subresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = tex->mipmaps,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };
    ce_trackTexture(enc, wgpuTex, usage_general);
    //wgpuCommandEncoderTransitionTextureLayout(enc, wgpuTex, wgpuTex->layout, VK_IMAGE_LAYOUT_GENERAL);
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
        vkCmdBlitImage(enc->buffer, wgpuTex->image, VK_IMAGE_LAYOUT_GENERAL, wgpuTex->image, VK_IMAGE_LAYOUT_GENERAL, 1, &blitRegion, VK_FILTER_LINEAR);
    }
    
    //ru_trackTexture(&enc->resourceUsage, wgpuTex);
    WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(enc, NULL);
    wgpuQueueSubmit(g_vulkanstate.queue, 1, &buffer);
    wgpuCommandBufferRelease(buffer);
    wgpuCommandEncoderRelease(enc);

    /* Compute based implementation
        static DescribedComputePipeline* cpl = LoadComputePipeline(mipmapComputerSource2);
        VkImageBlit blit;
        BeginComputepass();
        for(uint32_t i = 0;i < tex->mipmaps - 1;i++){
            SetBindgroupTextureView(&cpl->bindGroup, 0, (WGPUTextureView)tex->mipViews[i    ]);
            SetBindgroupTextureView(&cpl->bindGroup, 1, (WGPUTextureView)tex->mipViews[i + 1]);
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
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    WGPUBuffer wgpuBuffer = wgpuDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    //wgpuBuffer->refCount++;
    wgpuQueueWriteBuffer(g_vulkanstate.queue, wgpuBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgpuBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgpuBufferRelease(wgpuBuffer);
}

void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    WGPUBuffer wgpuBuffer = wgpuDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_vulkanstate.queue, wgpuBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgpuBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    wgpuBufferRelease(wgpuBuffer);
}

extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->buffer != nullptr && buffer->size >= size){
        wgpuQueueWriteBuffer(g_vulkanstate.queue, (WGPUBuffer)buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgpuBufferRelease((WGPUBuffer)buffer->buffer);
        WGPUBufferDescriptor nbdesc zeroinit;
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &nbdesc);
        buffer->size = size;
        wgpuQueueWriteBuffer(g_vulkanstate.queue, (WGPUBuffer)buffer->buffer, 0, data, size);
    }
}


extern "C" void UnloadBuffer(DescribedBuffer* buf){
    WGPUBuffer handle = (WGPUBuffer)buf->buffer;
    wgpuBufferRelease(handle);
    std::free(buf);
}


void createRenderPass() {
    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    //colorAttachment.format = toVulkanPixelFormat(g_vulkanstate.surface.surfaceConfig.format);
    colorAttachment.format = toVulkanPixelFormat(WGPUTextureFormat_BGRA8Unorm);
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
extern "C" void RenderPassSetIndexBuffer(DescribedRenderpass* drp, DescribedBuffer* buffer, WGPUIndexFormat format, uint64_t offset){
    wgpuRenderPassEncoderSetIndexBuffer(drp->rpEncoder, (WGPUBuffer)buffer->buffer, format, 0, WGPU_WHOLE_SIZE);
    //wgpuRenderPassEncoderSetIndexBuffer((WGPURenderPassEncoder)drp->rpEncoder, (WGPUBuffer)buffer->buffer, format, offset, buffer->size);
}
extern "C" void RenderPassSetVertexBuffer(DescribedRenderpass* drp, uint32_t slot, DescribedBuffer* buffer, uint64_t offset){
    wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, WGPU_WHOLE_SIZE);
    //wgpuRenderPassEncoderSetVertexBuffer((WGPURenderPassEncoder)drp->rpEncoder, slot, (WGPUBuffer)buffer->buffer, offset, buffer->size);
}
extern "C" void RenderPassSetBindGroup(DescribedRenderpass* drp, uint32_t group, DescribedBindGroup* bindgroup){
    wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)drp->rpEncoder, group, (WGPUBindGroup)bindgroup->bindGroup, 0, NULL);
    //wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)drp->rpEncoder, group, (WGPUBindGroup)UpdateAndGetNativeBindGroup(bindgroup), 0, nullptr);
}
extern "C" void RenderPassDraw        (DescribedRenderpass* drp, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance){
    wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
    //wgpuRenderPassEncoderDraw((WGPURenderPassEncoder)drp->rpEncoder, vertexCount, instanceCount, firstVertex, firstInstance);
}
extern "C" void RenderPassDrawIndexed (DescribedRenderpass* drp, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance){
    wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    //wgpuRenderPassEncoderDrawIndexed((WGPURenderPassEncoder)drp->rpEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
}
extern "C" void PrepareFrameGlobals(){
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    if(vbo_buf != 0){
        wgpuBufferUnmap(vbo_buf);
    }
    if(cache.unusedBatchBuffers.size == 0){
        WGPUBufferDescriptor bdesc{
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapWrite | WGPUBufferUsage_Vertex,
            .size = (((size_t)RENDERBATCH_SIZE) * sizeof(vertex))
        };

        vbo_buf = wgpuDeviceCreateBuffer(g_vulkanstate.device,  &bdesc);
        
        WGPUBufferVector_push_back(&cache.usedBatchBuffers, vbo_buf);
        wgpuBufferAddRef(vbo_buf);

        wgpuBufferMap(vbo_buf, WGPUMapMode_Write, 0, bdesc.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        
    }
    else{
        vbo_buf = cache.unusedBatchBuffers.data[cache.unusedBatchBuffers.size - 1];
        WGPUBufferVector_pop_back(&cache.unusedBatchBuffers);
        WGPUBufferVector_push_back(&cache.usedBatchBuffers, vbo_buf);
        wgpuBufferMap(vbo_buf, WGPUMapMode_Write, 0, wgpuBufferGetSize(vbo_buf), (void**)&vboptr_base);
        vboptr = vboptr_base;
        wgpuBufferAddRef(vbo_buf);
    }
}
extern "C" DescribedBuffer* UpdateVulkanRenderbatch(){
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    DescribedBuffer* db = callocnewpp(DescribedBuffer); 
    db->usage = vbo_buf->usage;
    db->size = wgpuBufferGetSize(vbo_buf);
    db->buffer = vbo_buf;
    if(vbo_buf != 0){
        wgpuBufferUnmap(vbo_buf);
    }
    if(WGPUBufferVector_empty(&cache.unusedBatchBuffers)){
        WGPUBufferDescriptor bdesc = {
            .usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_MapWrite | WGPUBufferUsage_Vertex,
            .size = (RENDERBATCH_SIZE * sizeof(vertex))
        };

        vbo_buf = wgpuDeviceCreateBuffer(g_vulkanstate.device,  &bdesc);
        wgpuBufferMap(vbo_buf, WGPUMapMode_Write, 0, bdesc.size, (void**)&vboptr_base);
        vboptr = vboptr_base;
        WGPUBufferVector_push_back(&cache.usedBatchBuffers, vbo_buf);
        wgpuBufferAddRef(vbo_buf);
    }
    else{
        vbo_buf = cache.unusedBatchBuffers.data[cache.unusedBatchBuffers.size - 1];

        WGPUBufferVector_pop_back(&cache.unusedBatchBuffers);
        WGPUBufferVector_push_back(&cache.usedBatchBuffers, vbo_buf);
        
        wgpuBufferMap(vbo_buf, WGPUMapMode_Write, 0, WGPU_WHOLE_SIZE, (void**)&vboptr_base);
        vboptr = vboptr_base;
        wgpuBufferAddRef(vbo_buf);
    }

    return db;
}
void PushUsedBuffer(void* nativeBuffer){
    return;
    uint32_t cacheIndex = g_vulkanstate.device->submittedFrames % framesInFlight;
    auto& cache = g_vulkanstate.device->frameCaches[cacheIndex];
    WGPUBuffer buffer = (WGPUBuffer)nativeBuffer;
    
}


extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    WGPUBindGroup bindGroup = UpdateAndGetNativeBindGroup(&pipeline->bindGroup);
    wgpuComputePassEncoderSetPipeline ((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, pipeline->pipeline);
    wgpuComputePassEncoderSetBindGroup((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, 0, bindGroup, 0, NULL);
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

extern "C" void raytracing_LoadDeviceFunctions(VkDevice device);
void InitBackend(){
    #if SUPPORT_SDL2 == 1 || SUPPORT_SDL3 == 1
    SDL_Init(SDL_INIT_VIDEO);
    #endif
    #if SUPPORT_GLFW == 1
    glfwInit();
    #endif
    volkInitialize();
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
            TRACELOG(LOG_INFO, "Validation Layer %s available but not selected since NDEBUG is defined",layer.layerName);
            #endif
        }
    }

    WGPUInstanceLayerSelection instanceLayers zeroinit;
    instanceLayers.chain.sType = WGPUSType_InstanceValidationLayerSelection;
    
    instanceLayers.instanceLayers = validationLayers.data();
    instanceLayers.instanceLayerCount = validationLayers.size();
    
    WGPUInstanceDescriptor idesc zeroinit;
    idesc.nextInChain = &instanceLayers.chain;
    g_vulkanstate.instance = wgpuCreateInstance(&idesc);
    volkLoadInstance(g_vulkanstate.instance->instance);
    g_vulkanstate.physicalDevice = pickPhysicalDevice(g_vulkanstate.instance);

    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice->physicalDevice, &g_vulkanstate.memProperties);

    WGPUDeviceDescriptor ddescriptor zeroinit;
    WGPUDevice device = NULL;
    WGPURequestDeviceCallbackInfo rdci = {
        .mode = WGPUCallbackMode_WaitAnyOnly,
        .callback = [](WGPURequestDeviceStatus, WGPUDevice device, WGPUStringView, void* ud1, void *){
            *reinterpret_cast<WGPUDevice*>(ud1) = device;
        },
        .userdata1 = reinterpret_cast<void*>(&device),
        .userdata2 = NULL,
    };
    WGPUFuture future = wgpuAdapterRequestDevice(g_vulkanstate.physicalDevice, &ddescriptor, rdci);
    WGPUFutureWaitInfo info{
        future, 0
    };
    wgpuInstanceWaitAny(g_vulkanstate.instance, 1, &info, 1000000000);
    g_vulkanstate.device = device;

    g_vulkanstate.queue = device->queue;
    //auto device_and_queues = createLogicalDevice(g_vulkanstate.physicalDevice, queues);
#if VULKAN_ENABLE_RAYTRACING == 1
    raytracing_LoadDeviceFunctions(device->device);
#endif
    //g_vulkanstate.device = device_and_queues.first;
    //g_vulkanstate.queue = device_and_queues.second;
    //for(uint32_t fif = 0;fif < framesInFlight;fif++){
    //    g_vulkanstate.queue->syncState[fif].renderFinishedFence = CreateFence(0);
    //}

    
    createRenderPass();
}

extern "C" FullSurface CreateSurface(void* nsurface, uint32_t width, uint32_t height){
    FullSurface ret{};
    ret.surface = (WGPUSurface)nsurface;
    WGPUAdapter adapter = g_vulkanstate.physicalDevice;
    negotiateSurfaceFormatAndPresentMode(adapter, nsurface);
    WGPUSurfaceCapabilities capa{};

    wgpuSurfaceGetCapabilities((WGPUSurface)ret.surface, adapter, &capa);
    WGPUSurfaceConfiguration config{};

    WGPUPresentMode thm = g_renderstate.throttled_PresentMode;
    WGPUPresentMode um  = g_renderstate.unthrottled_PresentMode;
    if (g_renderstate.windowFlags & FLAG_VSYNC_LOWLATENCY_HINT) {
        config.presentMode = (((g_renderstate.unthrottled_PresentMode == WGPUPresentMode_Mailbox) ? um : thm));
    } else if (g_renderstate.windowFlags & FLAG_VSYNC_HINT) {
        config.presentMode = thm;
    } else {
        config.presentMode = um;
    }
    //TRACELOG(LOG_INFO, "Initialized surface with %s", presentModeSpellingTable.at(config.presentMode).c_str());
    //config.alphaMode = WGPUCompositeAlphaMode_Opaque;
    config.format = toWGPUPixelFormat(g_renderstate.frameBufferFormat);
    //config.usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
    config.width = width;
    config.height = height;
    //config.viewFormats = &config.format;
    //config.viewFormatCount = 1;
    config.device = g_vulkanstate.device;

    ret.surfaceConfig.presentMode = config.presentMode;
    ret.surfaceConfig.device = config.device;
    ret.surfaceConfig.width = config.width;
    ret.surfaceConfig.height = config.width;
    ret.surfaceConfig.format = config.format;
    
    wgpuSurfaceConfigure((WGPUSurface)ret.surface, &config);
    WGPUSurface wvS = (WGPUSurface)ret.surface;
    ret.renderTarget = LoadRenderTexture(wvS->width, wvS->height);
    return ret;
}

RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    RenderTexture ret{
        .texture = LoadTextureEx(width, height, (PixelFormat)g_renderstate.frameBufferFormat, true),
        .colorMultisample = Texture{}, 
        .depth = LoadTexturePro(width, height, Depth32, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1)
    };
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT){
        ret.colorMultisample = LoadTexturePro(width, height, (PixelFormat)g_renderstate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4, 1);
    }
    WGPUTextureView colorTargetView = (WGPUTextureView)ret.texture.view;
    //wgpuQueueTransitionLayout(g_vulkanstate.queue, colorTargetView->texture, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //if(ret.colorMultisample.id)
    //    wgpuQueueTransitionLayout(g_vulkanstate.queue, ((WGPUTexture)ret.colorMultisample.id), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //wgpuQueueTransitionLayout(g_vulkanstate.queue, ((WGPUTexture)ret.depth.id), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    ret.colorAttachmentCount = 1;
    return ret;
}
extern "C" void BeginRenderpassEx(DescribedRenderpass *renderPass){

    //WGPURenderPassEncoder ret = callocnewpp(WGPURenderPassEncoderImpl);

    renderPass->settings = g_renderstate.currentSettings;

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    WGPUCommandEncoderDescriptor cedesc zeroinit;
    WGPUCommandEncoder encoder = (WGPUCommandEncoder)renderPass->cmdEncoder;
    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder(g_vulkanstate.device, &cedesc);
    
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
    //    ((WGPUTextureView)rtex.texture.view)->view,
    //    ((WGPUTextureView)rtex.depth.view)->view,
    //};

    WGPURenderPassDescriptor rpdesc zeroinit;
    WGPURenderPassDepthStencilAttachment dsa zeroinit;
    dsa.depthClearValue = renderPass->depthClear;
    dsa.stencilClearValue = 0;
    dsa.stencilLoadOp = WGPULoadOp_Undefined;
    dsa.stencilStoreOp = WGPUStoreOp_Discard;
    dsa.depthLoadOp = renderPass->depthLoadOp;
    dsa.depthStoreOp = renderPass->depthStoreOp;
    dsa.view = (WGPUTextureView)rtex.depth.view;
    WGPURenderPassColorAttachment rca[max_color_attachments] zeroinit;
    rpdesc.colorAttachmentCount = rtex.colorAttachmentCount;
    rca[0].depthSlice = 0;
    rca[0].clearValue = renderPass->colorClear;
    rca[0].loadOp = renderPass->colorLoadOp;
    rca[0].storeOp = renderPass->colorStoreOp;
    if(rtex.colorMultisample.view){
        rca[0].view = (WGPUTextureView)rtex.colorMultisample.view;
        rca[0].resolveTarget = (WGPUTextureView)rtex.texture.view;
        //initializeOrTransition(((WGPUCommandEncoder)renderPass->cmdEncoder), rca[0].view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        //initializeOrTransition(((WGPUCommandEncoder)renderPass->cmdEncoder), rca[0].resolveTarget->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    else{
        rca[0].view = (WGPUTextureView)rtex.texture.view;
        //initializeOrTransition(((WGPUCommandEncoder)renderPass->cmdEncoder), rca[0].view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    if(renderPass->settings.depthTest){
        rpdesc.depthStencilAttachment = &dsa;
    }
    //if(rpdesc.colorAttachmentCount > 1){
    //    for(uint32_t i = 0;i < rpdesc.colorAttachmentCount - 1;i++){
    //        WGPURenderPassColorAttachment& ar = rca[i + 1];
    //        ar.depthSlice = 0;
    //        ar.clearValue = renderPass->colorClear;
    //        ar.loadOp = renderPass->colorLoadOp;
    //        ar.storeOp = renderPass->colorStoreOp;
    //        ar.view = rtex.moreColorAttachments[i].view;
    //        initializeOrTransition(((WGPUCommandEncoder)renderPass->cmdEncoder), ar.view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //    }
    //}
    rpdesc.colorAttachments = rca;
    
    //fbci.pAttachments = fbAttac   hments;
    //fbci.attachmentCount = 2;
    //fbci.width = rtex.texture.width;
    //fbci.height = rtex.texture.height;
    //fbci.layers = 1;
    //fbci.renderPass = (VkRenderPass)renderPass->VkRenderPass;
    //VkFramebuffer rahmePuffer = 0;
    //vkCreateFramebuffer(g_vulkanstate.device, &fbci, nullptr, &rahmePuffer);
    //rpbi.framebuffer = rahmePuffer;
    //renderPass->rpEncoder = BeginRenderPass_Vk((VkCommandBuffer)renderPass->cmdEncoder, renderPass, rahmePuffer, rtex.texture.width, rtex.texture.height);
    renderPass->rpEncoder = wgpuCommandEncoderBeginRenderPass((WGPUCommandEncoder)renderPass->cmdEncoder, &rpdesc);
    
    VkViewport viewport{
        .x        =  static_cast<float>(0),
        .y        =  static_cast<float>(rtex.texture.height),
        .width    =  static_cast<float>(rtex.texture.width),
        .height   = -static_cast<float>(rtex.texture.height),
        .minDepth =  static_cast<float>(0),
        .maxDepth =  static_cast<float>(1),
    };

    VkRect2D scissor{
        .offset = VkOffset2D{
            .x = 0,
            .y = 0,
        },
        .extent = VkExtent2D{
            .width = rtex.texture.width,
            .height = rtex.texture.height,
        }
    };
    //TODO delet all this
    //for(uint32_t i = 0;i < rpdesc.colorAttachmentCount;i++){
    //    vkCmdSetViewport(((WGPURenderPassEncoder)renderPass->rpEncoder)->secondaryCmdBuffer, i, 1, &viewport);
    //    vkCmdSetScissor (((WGPURenderPassEncoder)renderPass->rpEncoder)->secondaryCmdBuffer, i, 1, &scissor);
    //}
    //wgpuRenderPassEncoderBindPipeline((WGPURenderPassEncoder)renderPass->rpEncoder, g_renderstate.defaultPipeline);
    g_renderstate.activeRenderpass = renderPass;
    //UpdateBindGroup_Vk(&g_renderstate.defaultPipeline->bindGroup);
    //wgpuRenderPassEncoderBindDescriptorSet((WGPURenderPassEncoder)renderPass->rpEncoder, 0, (DescriptorSetHandle)g_renderstate.defaultPipeline->bindGroup.bindGroup);
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
    wgpuRenderPassEncoderSetPipeline((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, pipeline->activePipeline);
    wgpuRenderPassEncoderSetBindGroup((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, UpdateAndGetNativeBindGroup(&pipeline->bindGroup), 0, NULL);
}

extern "C" void BindPipeline(DescribedPipeline* pipeline, PrimitiveType drawMode){
    BindPipelineWithSettings(pipeline, drawMode, g_renderstate.currentSettings);
    
    //pipeline->lastUsedAs = drawMode;
    //wgpuRenderPassEncoderSetBindGroup ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGPUBindGroup)GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);

}
WGPUBuffer tttIntermediate zeroinit;
extern "C" void CopyTextureToTexture(Texture source, Texture dest){
    

    WGPUTexture sourceTexture = reinterpret_cast<WGPUTexture>(source.id);
    WGPUTexture destTexture = reinterpret_cast<WGPUTexture>(dest.id);
    WGPUCommandEncoder encodingDest = reinterpret_cast<WGPUCommandEncoder>(g_renderstate.computepass.cmdEncoder);
    
    //wgpuCommandEncoderCopyTextureToTexture;
    WGPUTexelCopyTextureInfo sourceinfo{
        source.id,
        0,
        WGPUOrigin3D{},
        WGPUTextureAspect_All
    };
    WGPUTexelCopyTextureInfo destinfo{
        dest.id,
        0,
        WGPUOrigin3D{},
        WGPUTextureAspect_All
    };
    WGPUExtent3D copySizeInfo{
        .width = source.width,
        .height = source.height,
        .depthOrArrayLayers = 1
    };
    wgpuCommandEncoderCopyTextureToTexture(
        encodingDest,
        &sourceinfo,
        &destinfo, 
        &copySizeInfo
    );
}
void ComputepassEndOnlyComputing(cwoid){
    WGPUComputePassEncoder computePassEncoder = (WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder;
    wgpuComputePassEncoderEnd(computePassEncoder);
}

extern "C" void EndRenderpassEx(DescribedRenderpass* rp){
    drawCurrentBatch();
    if(rp->rpEncoder){
        wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)rp->rpEncoder);
        wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)rp->rpEncoder);
        rp->rpEncoder = nullptr;
    }
    VkImageMemoryBarrier rpAttachmentBarriers[2] zeroinit;
    auto rtex = g_renderstate.renderTargetStack.peek();
    rpAttachmentBarriers[0].image = reinterpret_cast<WGPUTexture>(rtex.texture.id)->image;
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
    vkCmdPipelineBarrier(((WGPUCommandEncoder)rp->cmdEncoder)->buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, 0, 0, 1, rpAttachmentBarriers);
    


    WGPUCommandBuffer cbuffer = wgpuCommandEncoderFinish(rp->cmdEncoder, NULL);
    
    g_renderstate.activeRenderpass = nullptr;
    VkSubmitInfo sinfo{};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &cbuffer->buffer;


    wgpuQueueSubmit(g_vulkanstate.queue, 1, &cbuffer);


    WGPURenderPassEncoder rpe = (WGPURenderPassEncoder)rp->rpEncoder;
    if(rpe){
        wgpuRenderPassEncoderRelease(rpe);
    }
    WGPUCommandEncoder cmdEncoder = (WGPUCommandEncoder)rp->cmdEncoder;
    rp->cmdEncoder = nullptr;
    wgpuCommandEncoderRelease(cmdEncoder);
    wgpuCommandBufferRelease(cbuffer);
    //vkResetFences(g_vulkanstate.device, 1, &g_vulkanstate.queue.syncState.renderFinishedFence);
    //g_vulkanstate.queue.syncState.submitsInThisFrame = 0;
    //vkDestroyFence(g_vulkanstate.device, fence, nullptr);

}

void UpdateTexture(Texture tex, void* data){
    WGPUTexelCopyTextureInfo destination{};
    destination.texture = (WGPUTexture)tex.id;
    destination.aspect = WGPUTextureAspect_All;
    destination.mipLevel = 0;
    destination.origin = WGPUOrigin3D{0,0,0};

    WGPUTexelCopyBufferLayout source{};
    source.offset = 0;
    source.bytesPerRow = GetPixelSizeInBytes(tex.format) * tex.width;
    source.rowsPerImage = tex.height;
    WGPUExtent3D writeSize{};
    writeSize.depthOrArrayLayers = 1;
    writeSize.width = tex.width;
    writeSize.height = tex.height;
    wgpuQueueWriteTexture(g_vulkanstate.queue, &destination, data, tex.width * tex.height * GetPixelSizeInBytes(tex.format), &source, &writeSize);
}



extern "C" void EndRenderpassPro(DescribedRenderpass* rp, bool renderTexture){
    //if(renderTexture){
    //    wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)rp->rpEncoder);
    //    wgpuReleaseRenderPassEncoder((WGPURenderPassEncoder)rp->rpEncoder);
    //    WGPUTexture ctarget = (WGPUTexture)GetActiveColorTarget();
    //    wgpuCommandEncoderTransitionTextureLayout((WGPUCommandEncoder)rp->cmdEncoder, ctarget, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    //    rp->rpEncoder = nullptr;
    //}
    EndRenderpassEx(rp);
}
void DispatchCompute(uint32_t x, uint32_t y, uint32_t z){
    wgpuComputePassEncoderDispatchWorkgroups((WGPUComputePassEncoder)g_renderstate.computepass.cpEncoder, x, y, z);
}
