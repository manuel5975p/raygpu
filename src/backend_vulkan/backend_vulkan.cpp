#include <config.h>
#include <raygpu.h>
#include <set>
#include <wgpustate.inc>
#include "vulkan_internals.hpp"


VulkanState g_vulkanstate{};

void BufferData(DescribedBuffer* buffer, void* data, size_t size){
    if(buffer->size >= size){
        WGVKBuffer handle = (WGVKBuffer)buffer->buffer;
        void* udata = 0;
        VkResult mapresult = vkMapMemory(g_vulkanstate.device, handle->memory, 0, size, 0, &udata);
        if(mapresult == VK_SUCCESS){
            std::memcpy(data, udata, size);
            vkUnmapMemory(g_vulkanstate.device, handle->memory);
        }
        else{
            abort();
        }
        
        //vkBindBufferMemory()
    }
}

//__attribute__((weak)) void drawCurrentBatch(){
//    size_t vertexCount = vboptr - vboptr_base;
//    if(vertexCount == 0){
//        return;
//    }
//    DescribedBuffer* vbo = GenBufferEx(vboptr_base, vertexCount * sizeof(vertex), BufferUsage_Vertex | BufferUsage_CopyDst);
//    
//    renderBatchVAO->buffers.front().first = vbo;
//    BindPipeline(GetActivePipeline(), WGPUPrimitiveTopology_TriangleList);
//    BindVertexArray_Vk((WGVKRenderPassEncoder)g_renderstate.renderpass.rpEncoder, renderBatchVAO);
//    //wgvkRenderPassEncoderBindVertexBuffer(, uint32_t binding, BufferHandle buffer, VkDeviceSize offset)
//    wgvkRenderpassEncoderDraw((WGVKRenderPassEncoder)g_renderstate.renderpass.rpEncoder, vertexCount, 1, 0, 0);
//    
//    //UnloadBuffer(vbo);
//    vboptr = vboptr_base;
//}
void ResetSyncState(){
    g_vulkanstate.queue.syncState.submitsInThisFrame = 0;
    //g_vulkanstate.syncState.semaphoresInThisFrame.clear();
}
void PresentSurface(FullSurface* surface){

    VkSubmitInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.signalSemaphoreCount = 1;
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    si.pWaitDstStageMask = &stage;
    si.pWaitSemaphores = &g_vulkanstate.queue.syncState.getSemaphoreOfSubmitIndex(0);
    si.pSignalSemaphores = &g_vulkanstate.queue.syncState.getSemaphoreOfSubmitIndex(1);
    si.pCommandBuffers = nullptr;
    si.commandBufferCount = 0;
    vkQueueSubmit(g_vulkanstate.queue.graphicsQueue, 1, &si, VK_NULL_HANDLE);

    WGVKSurface wgvksurf = (WGVKSurface)surface->surface;
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    VkSemaphore waiton[2] = {
        g_vulkanstate.queue.syncState.semaphoresInThisFrame[1],
        //g_vulkanstate.syncState.imageAvailableSemaphores[0]
    };
    presentInfo.pWaitSemaphores = waiton;
    VkSwapchainKHR swapChains[] = {wgvksurf->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &wgvksurf->activeImageIndex;
    VkHostImageLayoutTransitionInfo transition zeroinit;
    VkImageSubresourceRange isr{};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;
    VkCommandPool oof{};
    VkCommandPoolCreateInfo pci zeroinit;
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    vkCreateCommandPool(g_vulkanstate.device, &pci, nullptr, &oof);
    TransitionImageLayout(g_vulkanstate.device, oof, g_vulkanstate.queue.graphicsQueue, wgvksurf->images[wgvksurf->activeImageIndex]->image, VK_FORMAT_B8G8R8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    vkDestroyCommandPool(g_vulkanstate.device, oof, nullptr);
    VkResult presentRes = vkQueuePresentKHR(g_vulkanstate.queue.presentQueue, &presentInfo);
    if(presentRes != VK_SUCCESS){
        std::cerr << "presentRes is " << presentRes << std::endl;
    }
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

    if (vkCreateBuffer(g_vulkanstate.device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
    VkDeviceMemory vertexBufferMemory;
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, vertexBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device, vertexBuffer, vertexBufferMemory, 0);
    if(data != nullptr){
        void* mapdata;
        VkResult vkres = vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mapdata);
        if(vkres != VK_SUCCESS)abort();
        memcpy(mapdata, data, (size_t)bufferInfo.size);
        vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
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
    vkCreateRenderPass(g_vulkanstate.device, &rpci, nullptr, (VkRenderPass*)&ret.VkRenderPass);

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
        }
    };
    DescribedSampler ret{};// = callocnew(DescribedSampler);
    VkSamplerCreateInfo sci{};
    sci.compareEnable = VK_FALSE;
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.addressModeU = vkamode(amode);
    sci.addressModeV = vkamode(amode);
    sci.addressModeW = vkamode(amode);
    
    sci.mipmapMode = ((fmode == filter_linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);

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
    ret.minFilter = mipmapFilter;
    ret.compare = CompareFunction_Undefined;//huh??
    VkResult scr = vkCreateSampler(g_vulkanstate.device, &sci, nullptr, (VkSampler*)&ret.sampler);
    if(scr != VK_SUCCESS){
        throw std::runtime_error("Sampler creation failed: " + std::to_string(scr));
    }
    return ret;
}


extern "C" void GetNewTexture(FullSurface *fsurface){

    uint32_t imageIndex = ~0;
    VkCommandPool oof{};
    VkCommandPoolCreateInfo pci zeroinit;
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    vkCreateCommandPool(g_vulkanstate.device, &pci, nullptr, &oof);
    WGVKSurface wgvksurf = ((WGVKSurface)fsurface->surface);
    //TODO: Multiple frames in flight, this amounts to replacing 0 with frameCount % 2 or something similar
    VkResult acquireResult = vkAcquireNextImageKHR(g_vulkanstate.device, wgvksurf->swapchain, UINT64_MAX, g_vulkanstate.queue.syncState.getSemaphoreOfSubmitIndex(0), VK_NULL_HANDLE, &imageIndex);
    if(acquireResult != VK_SUCCESS){
        std::cerr << "acquireResult is " << acquireResult << std::endl;
    }
    VkDevice vd = ((WGVKSurface)fsurface->surface)->device;
    VkCommandBuffer buf = BeginSingleTimeCommands(vd, oof);
    EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, wgvksurf->images[imageIndex]->image);
    EncodeTransitionImageLayout(buf, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ((WGVKTexture)fsurface->renderTarget.depth.id)->image);
    EndSingleTimeCommands(vd, oof, g_vulkanstate.queue.graphicsQueue, buf);
    vkDestroyCommandPool(g_vulkanstate.device, oof, nullptr);
    
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
    if (!windowExtensions) {
        throw std::runtime_error("Failed to get required extensions for windowing!");
    }

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
    #ifndef NDEBUG
    for (const auto &layer : availableLayers) {
        if (std::string(layer.layerName).find("validat") != std::string::npos) {
            std::cout << "\t[DEBUG]: Selecting layer " << layer.layerName << std::endl;
            validationLayers.push_back(layer.layerName);
        }
    }
    #endif

    VkInstanceCreateInfo createInfo{};

    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Copy GLFW extensions to a vector
    std::vector<const char *> extensions(windowExtensions, windowExtensions + requiredGLFWExtensions);
    
    uint32_t instanceExtensionCount = 0;

    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, availableInstanceExtensions.data());
    for(auto& ext : availableInstanceExtensions){
        std::cout << ext.extensionName << ", ";
    }
    std::cout << std::endl;
    //extensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    

    // (Optional) Enable validation layers here if needed
    VkResult instanceCreation = vkCreateInstance(&createInfo, nullptr, &ret);
    if (instanceCreation != VK_SUCCESS) {
        throw std::runtime_error(std::string("Failed to create Vulkan instance : ") + std::to_string(instanceCreation));
    } else {
        std::cout << "Successfully created Vulkan instance\n";
    }

    
    return ret;
}



// Function to pick a suitable physical device (GPU)
VkPhysicalDevice pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, devices.data());
    VkPhysicalDevice ret{};
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
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
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
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

    if (g_vulkanstate.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
picked:
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
        #endif
        #ifdef MAIN_WINDOW_SDL2 //uuh 
        VkBool32 presentSupport = VK_TRUE;
        #elif defined(MAIN_WINDOW_GLFW)
        VkBool32 presentSupport = glfwGetPhysicalDevicePresentationSupport(g_vulkanstate.instance, g_vulkanstate.physicalDevice, i) ? VK_TRUE : VK_FALSE;
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
        throw std::runtime_error("Failed to find graphics queue, probably something went wong");
    }

    return ret;
}
extern "C" DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* descs, uint32_t uniformCount, bool compute){
    DescribedBindGroupLayout retv{};
    DescribedBindGroupLayout* ret = &retv;
    VkDescriptorSetLayout layout{};
    VkDescriptorSetLayoutCreateInfo lci{};
    lci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lci.bindingCount = uniformCount;
    
    small_vector<VkDescriptorSetLayoutBinding> entries(uniformCount);
    lci.pBindings = entries.data();
    for(uint32_t i = 0;i < uniformCount;i++){
        switch(descs[i].type){
            case storage_texture2d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }break;
            case storage_texture3d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }break;
            case storage_buffer:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }break;
            case uniform_buffer:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }break;
            case texture2d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }break;
            case texture3d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }break;
            case texture_sampler:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            }break;
        }
        entries[i].descriptorCount = 1;
        entries[i].binding = descs[i].location;
        entries[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    ret->entries = (ResourceTypeDescriptor*)std::calloc(uniformCount, sizeof(ResourceTypeDescriptor));
    ret->entryCount = uniformCount;
    std::memcpy(ret->entries, descs, uniformCount * sizeof(ResourceTypeDescriptor));
    VkResult createResult = vkCreateDescriptorSetLayout(g_vulkanstate.device, &lci, nullptr, (VkDescriptorSetLayout*)&ret->layout);
    return retv;
}

void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    BufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Uniform;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    wgvkBuffer->refCount++;
    wgvkQueueWriteBuffer(&g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
}
extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->buffer != nullptr && buffer->size >= size){
        wgvkQueueWriteBuffer(&g_vulkanstate.queue, (WGVKBuffer)buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgvkReleaseBuffer((WGVKBuffer)buffer->buffer);
        BufferDescriptor nbdesc zeroinit;
        nbdesc.size = size;
        nbdesc.usage = buffer->usage;

        buffer->buffer = wgvkDeviceCreateBuffer((VkDevice)GetDevice(), &nbdesc);
        buffer->size = size;
        wgpuQueueWriteBuffer(GetQueue(), (WGPUBuffer)buffer->buffer, 0, data, size);
    }
}
void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    BufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = BufferUsage_CopyDst | BufferUsage_Storage;
    WGVKBuffer wgvkBuffer = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bufferDesc);
    wgvkQueueWriteBuffer(&g_vulkanstate.queue, wgvkBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = wgvkBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
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
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

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
    

    if (vkCreateRenderPass(g_vulkanstate.device, &renderPassInfo, nullptr, &g_vulkanstate.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
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
std::pair<VkDevice, WGVKQueueImpl> createLogicalDevice(VkPhysicalDevice physicalDevice, QueueIndices indices) {
    // Find queue families
    QueueIndices qind = findQueueFamilies();
    std::pair<VkDevice, WGVKQueueImpl> ret{};
    // Collect unique queue families
    std::set<uint32_t> uniqueQueueFamilies; // = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

    uniqueQueueFamilies.insert(indices.computeIndex);
    uniqueQueueFamilies.insert(indices.graphicsIndex);
    uniqueQueueFamilies.insert(indices.presentIndex);

    // Example: Include computeFamily if it's different
    std::vector<uint32_t> queueFamilies(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

    // Create queue create infos
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify device extensions
    std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
    auto dcresult = vkCreateDevice(g_vulkanstate.physicalDevice, &createInfo, nullptr, &ret.first);
    if (dcresult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    } else {
        std::cout << "Successfully created logical device\n";
    }

    // Retrieve and assign queues
    vkGetDeviceQueue(ret.first, indices.graphicsIndex, 0, &ret.second.graphicsQueue);
    vkGetDeviceQueue(ret.first, indices.presentIndex, 0, &ret.second.presentQueue);

    if (indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex) {
        vkGetDeviceQueue(ret.first, indices.computeIndex, 0, &ret.second.computeQueue);
    } else {
        // If compute Index is same as graphics or present, assign accordingly
        if (indices.computeIndex == indices.graphicsIndex) {
            ret.second.computeQueue = ret.second.graphicsQueue;
        } else if (indices.computeIndex == indices.presentIndex) {
            ret.second.computeQueue = ret.second.presentQueue;
        }
    }
    //__builtin_dump_struct(&g_vulkanstate, printf);
    // std::cin.get();

    std::cout << "Successfully retrieved queues\n";

    return ret;
}
memory_types discoverMemoryTypes(VkPhysicalDevice physicalDevice) {
    memory_types ret{~0u, ~0u};
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
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
    #if SUPPORT_SDL3 == 1
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
    g_vulkanstate.queue.syncState.semaphoresInThisFrame.resize(10);
    for(uint32_t i = 0;i < 10;i++){
        g_vulkanstate.queue.syncState.semaphoresInThisFrame[i] = CreateSemaphore(0);
    }
    //g_vulkanstate.syncState.imageAvailableSemaphores[0] = CreateSemaphore(0);
    //g_vulkanstate.syncState.presentSemaphores[0] = CreateSemaphore(0);
    g_vulkanstate.queue.syncState.renderFinishedFence = CreateFence(0);

    
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
extern "C" void BeginRenderpassEx(DescribedRenderpass *renderPass){

    WGVKRenderPassEncoder ret = callocnewpp(WGVKRenderPassEncoderImpl);

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};

    
    renderPass->cmdEncoder = wgvkDeviceCreateCommandEncoder(g_vulkanstate.device);
    
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
    }
    else{
        rca.view = (WGVKTextureView)rtex.texture.view;
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
extern "C" void BindPipeline(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode){

    switch(drawMode){
        case WGPUPrimitiveTopology_TriangleList:
        //std::cout << "Binding: " <<  pipeline->pipeline << "\n";
        
        wgvkRenderPassEncoderSetPipeline((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (VkPipeline)pipeline->quartet.pipeline_TriangleList, (VkPipelineLayout)pipeline->layout.layout);
        break;
        //case WGPUPrimitiveTopology_TriangleStrip:
        //wgvkRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_TriangleStrip);
        //break;
        //case WGPUPrimitiveTopology_LineList:
        //wgvkRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_LineList);
        //break;
        //case WGPUPrimitiveTopology_PointList:
        //wgvkRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_PointList);
        //break;
        default:
            assert(false && "Unsupported Drawmode");
            abort();
    }
    //pipeline->lastUsedAs = drawMode;
    wgvkRenderPassEncoderBindDescriptorSet((WGVKRenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGVKBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup));
    //wgvkRenderPassEncoderSetBindGroup ((WGPURenderPassEncoder)g_renderstate.activeRenderpass->rpEncoder, 0, (WGPUBindGroup)GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);

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
    VkSemaphore waitsemaphore = g_vulkanstate.queue.syncState.getSemaphoreOfSubmitIndex(g_vulkanstate.queue.syncState.submitsInThisFrame);
    VkSemaphore signalesemaphore = g_vulkanstate.queue.syncState.getSemaphoreOfSubmitIndex(g_vulkanstate.queue.syncState.submitsInThisFrame + 1);
    sinfo.pWaitSemaphores = &waitsemaphore;
    sinfo.pWaitDstStageMask = &stageMask;
    sinfo.signalSemaphoreCount = 1;
    sinfo.pSignalSemaphores = &signalesemaphore;
    ++g_vulkanstate.queue.syncState.submitsInThisFrame;
    //VkFence fence{};
    //VkFenceCreateInfo finfo{};
    //finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //if(vkCreateFence(g_vulkanstate.device, &finfo, nullptr, &fence) != VK_SUCCESS){
    //    throw std::runtime_error("Could not create fence");
    //}
    wgvkQueueSubmit(&g_vulkanstate.queue, 1, &cbuffer);
    //if(vkQueueSubmit(g_vulkanstate.queue.graphicsQueue, 1, &sinfo, g_vulkanstate.queue.syncState.renderFinishedFence) != VK_SUCCESS){
    //    throw std::runtime_error("Could not submit commandbuffer");
    //}
    //if(vkWaitForFences(g_vulkanstate.device, 1, &g_vulkanstate.queue.syncState.renderFinishedFence, VK_TRUE, ~0) != VK_SUCCESS){
    //    throw std::runtime_error("Could not wait for fence");
    //}
    if(rp->rpEncoder)wgvkReleaseRenderPassEncoder((WGVKRenderPassEncoder)rp->rpEncoder);
    wgvkReleaseCommandEncoder((WGVKCommandEncoder)rp->cmdEncoder);
    wgvkReleaseCommandBuffer(cbuffer);
    //vkResetFences(g_vulkanstate.device, 1, &g_vulkanstate.queue.syncState.renderFinishedFence);
    //g_vulkanstate.queue.syncState.submitsInThisFrame = 0;
    //vkDestroyFence(g_vulkanstate.device, fence, nullptr);

}
extern "C" void EndRenderpassPro(DescribedRenderpass* rp, bool renderTexture){
    if(renderTexture){
        wgvkRenderPassEncoderEnd((WGVKRenderPassEncoder)rp->rpEncoder);
        wgvkReleaseRenderPassEncoder((WGVKRenderPassEncoder)rp->rpEncoder);
        wgvkCommandEncoderTransitionTextureLayout((WGVKCommandEncoder)rp->cmdEncoder, (WGVKTexture)GetActiveColorTarget(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        rp->rpEncoder = nullptr;
    }
    EndRenderpassEx(rp);
}
