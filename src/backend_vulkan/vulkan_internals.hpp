#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#define VMA_MIN_ALIGNMENT 32
#define VK_NO_PROTOTYPES
#include <external/volk.h>
#include <external/VmaUsage.h>
#include <unordered_set>
#include <vector>
#include <utility>
#include <map>
#include <internals.hpp>
#include <wgvk.h>
//#define SUPPORT_VULKAN_BACKEND 1
#include <enum_translation.h>
#if SUPPORT_GLFW == 1
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
#if SUPPORT_SDL3 == 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>
#endif
#if SUPPORT_SDL2 == 1
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#endif



inline std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>> genericVertexLayoutSetToVulkan(const VertexBufferLayoutSet& vls){
    std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>> ret{};

    std::vector<VkVertexInputAttributeDescription>& attribs = ret.first;
    uint32_t totalNumberOfAttributes = 0;
    
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        totalNumberOfAttributes += vls.layouts[i].attributeCount;
    }
    attribs.reserve(totalNumberOfAttributes);
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        for(uint32_t j = 0;j < vls.layouts[i].attributeCount;j++){
            VkVertexInputAttributeDescription adesc{};
            adesc.location = vls.layouts[i].attributes[j].shaderLocation;
            adesc.offset = vls.layouts[i].attributes[j].offset;
            adesc.binding = i;
            adesc.format = toVulkanVertexFormat(vls.layouts[i].attributes[j].format);
            attribs.push_back(adesc);
        }
    }


    std::vector<VkVertexInputBindingDescription>& bufferLayouts = ret.second;
    bufferLayouts.resize(vls.number_of_buffers);
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        bufferLayouts[i].binding = i;
        bufferLayouts[i].stride = vls.layouts[i].arrayStride;
        bufferLayouts[i].inputRate = toVulkanVertexStepMode(vls.layouts[i].stepMode);
    }
    return ret;
}



static inline VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0);


struct memory_types{
    uint32_t deviceLocal;
    uint32_t hostVisibleCoherent;
};

struct VulkanState {
    WGPUInstance instance = VK_NULL_HANDLE;
    WGPUAdapter physicalDevice = nullptr;
    VkPhysicalDeviceMemoryProperties memProperties;

    WGPUDevice device = nullptr;
    WGPUQueue queue;

    // Separate queues for clarity
    //VkQueue graphicsQueue = VK_NULL_HANDLE;
    //VkQueue presentQueue = VK_NULL_HANDLE;
    //VkQueue computeQueue = VK_NULL_HANDLE; // Example for an additional queue type

    // Separate queue family indices
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    uint32_t computeFamily = UINT32_MAX; // Example for an additional queue type

    memory_types memoryTypes;

    FullSurface surface;

    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;

    DescribedPipeline* defaultPipeline{};
    
    //VkExtent2D swapchainExtent = {0, 0};
    //std::vector<VkImage> swapchainImages;
    //std::vector<VkImageView> swapchainImageViews;
    //std::vector<VkFramebuffer> swapchainImageFramebuffers;

};  extern VulkanState g_vulkanstate; 


struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


inline SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    // Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    // Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}


// Function to choose the best surface format
inline VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // If the preferred format is not available, return the first one
    return availableFormats[0];
}

// Function to choose the best present mode
inline VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentMode;
        }
    }
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    // Fallback to FIFO which is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

// Function to choose the swap extent (resolution)
inline VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        #if SUPPORT_GLFW == 1
        glfwGetFramebufferSize(window, &width, &height);
        #elif SUPPORT_SDL2 == 1
        SDL_Vulkan_GetDrawableSize((SDL_Window*)window, &width, &height);
        #endif
        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
/*inline FullSurface LoadSurface(GLFWwindow* window, SurfaceConfiguration config){
    FullSurface retf{};
    WGPUSurface retp = callocnew(WGPUSurfaceImpl);
    auto& ret = *retp;
    #if SUPPORT_GLFW == 1
    if(glfwCreateWindowSurface(g_vulkanstate.instance, window, nullptr, &ret.surface) != VK_SUCCESS){
        throw std::runtime_error("could not create surface");
    }
    #elif SUPPORT_SDL2 == 1
    if (!SDL_Vulkan_CreateSurface((SDL_Window*)window, g_vulkanstate.instance, &ret.surface)) {
        // Retrieve SDL error message and throw an exception
        throw std::runtime_error("could not create surface: " + std::string(SDL_GetError()));
    }
    #elif SUPPORT_SDL3 == 1
    if (!SDL_Vulkan_CreateSurface((SDL_Window*)window, g_vulkanstate.instance, nullptr, &ret.surface)) {
        // Retrieve SDL error message and throw an exception
        throw std::runtime_error("could not create surface: " + std::string(SDL_GetError()));
    }
    //#endif
    #endif
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, ret.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;//toVulkanPresentMode(config.presentMode);//chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
    ret.width = extent.width;
    ret.height = extent.height;
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ret.surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    ret.swapchainImageFormat = surfaceFormat.format;
    ret.swapchainColorSpace = surfaceFormat.colorSpace;

    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
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
    createInfo.presentMode = presentMode; 
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(g_vulkanstate.device, &createInfo, nullptr, &ret.swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    } else {
        std::cout << "Successfully created swap chain with presentmode " << createInfo.presentMode << std::endl;
    }

    // Retrieve swapchain images
    vkGetSwapchainImagesKHR(g_vulkanstate.device, ret.swapchain, &imageCount, nullptr);
    ret.imagecount = imageCount;
    ret.images = (VkImage*)std::calloc(imageCount, sizeof(VkImage));
    ret.imageViews = (VkImageView*)std::calloc(imageCount, sizeof(VkImageView));

    vkGetSwapchainImagesKHR(g_vulkanstate.device, ret.swapchain, &imageCount, ret.images);

    
    for (size_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = ret.images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = ret.swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(g_vulkanstate.device, &createInfo, nullptr, ret.imageViews + i) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }

    ret.swapchainImageFormat = surfaceFormat.format;
    retf.surface = retp;

    config.format = fromVulkanPixelFormat(ret.swapchainImageFormat);
    
    config.width = extent.width;
    config.height = extent.height;
    retf.surfaceConfig = config;
    retf.renderTarget.texture.width = extent.width;
    retf.renderTarget.texture.height = extent.height;
    retf.renderTarget.depth = LoadTexturePro(extent.width, extent.height, Depth32, TextureUsage_RenderAttachment, 1, 1);
    
    return retf;
}*/



struct FullVkRenderPass{
    VkRenderPass renderPass;
    VkSemaphore signalSemaphore;
};
/*inline DescribedRenderpass LoadRenderPass(RenderSettings settings){
    DescribedRenderpass ret{};
    VkRenderPassCreateInfo rpci{};

    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    colorAttachment.format = toVulkanPixelFormat(g_vulkanstate.surface.surfaceConfig.format);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentDescription& depthAttachment = attachments[1];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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

    //VkSemaphoreCreateInfo si{};
    //si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    //vkCreateSemaphore(g_vulkanstate.device, &si, nullptr, &ret.signalSemaphore);
    return ret;
}*/
extern "C" void TransitionImageLayout(WGPUDevice device, VkCommandPool commandPool, VkQueue queue, WGPUTexture texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
extern "C" void EncodeTransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, WGPUTexture texture);
extern "C" VkCommandBuffer BeginSingleTimeCommands(WGPUDevice device, VkCommandPool commandPool);
extern "C" void EndSingleTimeCommandsAndSubmit(WGPUDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice, uint32_t i);
//static inline VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags){
//    return CreateSemaphoreD(g_vulkanstate.device->device, flags);
//}
//
//static inline VkFence CreateFence(VkFenceCreateFlags flags = 0){
//    VkFenceCreateInfo sci zeroinit;
//    VkFence ret zeroinit;
//    sci.flags = flags;
//    sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//    VkResult res = vkCreateFence(g_vulkanstate.device->device, &sci, nullptr, &ret);
//    if(res != VK_SUCCESS){
//        TRACELOG(LOG_ERROR, "Error creating fence");
//    }
//    return ret;
//}
extern "C" void BeginRenderpassEx(DescribedRenderpass *renderPass);
extern "C" void EndRenderPass(VkCommandBuffer cbuffer, DescribedRenderpass* rp);
extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, WGPUTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps);
extern "C" Texture LoadTextureFromImage(Image img);
extern "C" void UnloadTexture(Texture tex);


extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
//extern "C" DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count);
extern "C" void UpdateBindGroup(DescribedBindGroup* bg);
extern "C" DescribedBuffer* GenBufferEx(const void *data, size_t size, WGPUBufferUsage usage);
extern "C" void UnloadBuffer(DescribedBuffer* buf);

//wgpu I guess

extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);
extern "C" void GetNewTexture(FullSurface *fsurface);
extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t width, uint32_t height);



static inline void SetBindgroupSampler_Vk(DescribedBindGroup* bg, uint32_t index, DescribedSampler smp){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.sampler = smp.sampler;
    
    UpdateBindGroupEntry(bg, index, entry);
}
static inline void SetBindgroupTexture_Vk(DescribedBindGroup* bg, uint32_t index, Texture tex){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.textureView = (WGPUTextureView)tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}
static inline void BindVertexArray_Vk(WGPURenderPassEncoder rpenc, VertexArray* vao){
    for(uint32_t i = 0;i < vao->buffers.size();i++){
        wgpuRenderPassEncoderSetVertexBuffer(rpenc, i, (WGPUBuffer)vao->buffers[i].first->buffer, 0);
    }
}






#endif
