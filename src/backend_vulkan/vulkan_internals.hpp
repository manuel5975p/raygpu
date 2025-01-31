#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#include "small_vector.hpp"
#include <unordered_set>
#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <raygpu.h>
#include <internals.hpp>
#define SUPPORT_VULKAN_BACKEND 1
#include <enum_translation.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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
struct QueueIndices{
    uint32_t graphicsIndex;
    uint32_t computeIndex;
    uint32_t transferIndex;
    uint32_t presentIndex;
};

struct VertexAndFragmentShaderModuleImpl;
struct DescriptorSetHandleImpl;
struct BufferHandleImpl;
struct RenderPassEncoderHandleImpl;
struct CommandBufferHandleImpl;
struct ImageHandleImpl;

typedef VertexAndFragmentShaderModuleImpl* VertexAndFragmentShaderModule;
typedef DescriptorSetHandleImpl* DescriptorSetHandle;
typedef BufferHandleImpl* BufferHandle;
typedef RenderPassEncoderHandleImpl* RenderPassEncoderHandle;
typedef CommandBufferHandleImpl* CommmandBufferHandle;
typedef ImageHandleImpl* ImageHandle;

using refcount_type = uint32_t;
template<typename T>
using ref_holder = std::unordered_set<T>;

typedef struct VertexAndFragmentShaderModuleImpl{
    VkShaderModule vModule;
    VkShaderModule fModule;
}VertexAndFragmentShaderModuleImpl;

typedef struct DescriptorSetHandleImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    refcount_type refCount;
}DescriptorSetHandleImpl;

typedef struct BufferHandleImpl{
    VkBuffer buffer;
    VkDeviceMemory memory;
    refcount_type refCount;
}BufferHandleImpl;

typedef struct RenderPassEncoderHandleImpl{
    VkCommandBuffer cmdBuffer;
    ref_holder<BufferHandle> referencedBuffers;
    ref_holder<DescriptorSetHandle> referencedDescriptorSets;
    VkPipelineLayout lastLayout;
    VkFramebuffer frameBuffer;
    refcount_type refCount;
}RenderPassEncoderHandleImpl;

typedef struct CommandBufferHandleImpl{
    ref_holder<RenderPassEncoderHandle> referencedRPs;
    VkCommandBuffer buffer;
    VkCommandPool pool;
}CommandBufferHandleImpl;

typedef struct ImageHandleImpl{
    VkImage image;
    VkDeviceMemory memory;
}ImageHandleImpl;

struct WGVKSurfaceImpl{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    uint32_t imagecount;
    uint32_t width, height;
    VkFormat swapchainImageFormat;
    VkColorSpaceKHR swapchainColorSpace;
    VkImage* images;
    VkImageView* imageViews;
    VkFramebuffer* framebuffers;
};
typedef WGVKSurfaceImpl* WGVKSurface;

//struct WGVKDevice{
//    VkDevice device;
//};

struct WGVKQueue{
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    VkQueue presentQueue;
};

struct memory_types{
    uint32_t deviceLocal;
    uint32_t hostVisibleCoherent;
};

struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceMemoryProperties memProperties;

    VkDevice device = VK_NULL_HANDLE;
    WGVKQueue queue;

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

static inline uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    const VkPhysicalDeviceMemoryProperties& memProperties = g_vulkanstate.memProperties;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    assert(false && "failed to find suitable memory type!");
    return ~0u;
}
extern "C" Texture LoadTexturePro_Vk(uint32_t width, uint32_t height, PixelFormat format, int usage, uint32_t sampleCount, uint32_t mipmaps, const void* data);

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
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
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
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
inline FullSurface LoadSurface(GLFWwindow* window, SurfaceConfiguration config){
    FullSurface retf{};
    WGVKSurface retp = callocnew(WGVKSurfaceImpl);
    auto& ret = *retp;
    if(glfwCreateWindowSurface(g_vulkanstate.instance, window, nullptr, &ret.surface) != VK_SUCCESS){
        throw std::runtime_error("could not create surface");
    }
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, ret.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;//chooseSwapPresentMode(swapChainSupport.presentModes);
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
        std::cout << "Successfully created swap chain\n";
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
    
    retf.surfaceConfig = config;
    retf.frameBuffer.depth = LoadTexturePro_Vk(extent.width, extent.height, Depth32, TextureUsage_RenderAttachment, 1, 1, nullptr);
    
    return retf;
}

inline void wgvkSurfaceConfigure(WGVKSurfaceImpl* surface, const SurfaceConfiguration* config){
    auto& device = g_vulkanstate.device;
    vkDeviceWaitIdle(device);
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        vkDestroyImageView(device, surface->imageViews[i], nullptr);
        vkDestroyImage(device, surface->images[i], nullptr);
    }
    std::free(surface->framebuffers);
    std::free(surface->imageViews);
    std::free(surface->images);
    vkDestroySwapchainKHR(device, surface->swapchain, nullptr);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, surface->surface);
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;

    createInfo.minImageCount = surface->imagecount;
    createInfo.imageFormat = surface->swapchainImageFormat;
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
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; 
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
        viewInfo.format = surface->swapchainImageFormat;
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

struct FullVkRenderPass{
    VkRenderPass renderPass;
    VkSemaphore signalSemaphore;
};
inline FullVkRenderPass LoadRenderPass(RenderSettings settings){
    FullVkRenderPass ret{};
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
    vkCreateRenderPass(g_vulkanstate.device, &rpci, nullptr, &ret.renderPass);

    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(g_vulkanstate.device, &si, nullptr, &ret.signalSemaphore);
    return ret;
}

static inline void BeginRenderpassEx_Vk(DescribedRenderpass *renderPass, RenderTexture rtex){

    RenderPassEncoderHandle ret = callocnewpp(RenderPassEncoderHandleImpl);

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    VkClearValue clearvalues[2] = {};
    clearvalues[0].color = VkClearColorValue{};
    clearvalues[0].color.float32[0] = (float)renderPass->colorClear.r;
    clearvalues[0].color.float32[1] = (float)renderPass->colorClear.g;
    clearvalues[0].color.float32[2] = (float)renderPass->colorClear.b;
    clearvalues[0].color.float32[3] = (float)renderPass->colorClear.a;

    clearvalues[1].depthStencil = VkClearDepthStencilValue{};
    clearvalues[1].depthStencil.depth = (float)renderPass->depthClear;

    vkResetCommandBuffer((VkCommandBuffer)renderPass->cmdEncoder, 0);
    rpbi.renderPass = g_vulkanstate.renderPass;
    VkFramebufferCreateInfo fbci{};
    fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    VkImageView fbAttachments[2] = {
        (VkImageView)rtex.texture.view,
        (VkImageView)rtex.depth.view,
    };
    fbci.pAttachments = fbAttachments;
    fbci.attachmentCount = 2;
    fbci.width = rtex.texture.width;
    fbci.height = rtex.texture.height;
    fbci.layers = 1;
    VkFramebuffer rahmePuffer = 0;
    vkCreateFramebuffer(g_vulkanstate.device, &fbci, nullptr, &rahmePuffer);
    rpbi.framebuffer = rahmePuffer;
    //renderPass->renderTarget
}
static inline RenderPassEncoderHandle BeginRenderPass_Vk(VkCommandBuffer cbuffer, FullVkRenderPass rp, VkFramebuffer fb){

    RenderPassEncoderHandle ret = callocnewpp(RenderPassEncoderHandleImpl);
    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    VkClearValue clearvalues[2] = {};
    clearvalues[0].color = VkClearColorValue{};
    clearvalues[0].color.float32[0] = 1.0f;
    clearvalues[1].depthStencil = VkClearDepthStencilValue{};
    clearvalues[1].depthStencil.depth = 1.0f;
    rpbi.renderPass = rp.renderPass;
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clearvalues;
    rpbi.framebuffer = fb;
    rpbi.renderArea.extent.width = g_vulkanstate.surface.surfaceConfig.width;
    rpbi.renderArea.extent.height = g_vulkanstate.surface.surfaceConfig.height;

    vkBeginCommandBuffer(cbuffer, &bbi);
    vkCmdBeginRenderPass(cbuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    ret->cmdBuffer = cbuffer;
    ret->refCount = 1;
    return ret;
}
inline void EndRenderPass_Vk(VkCommandBuffer cbuffer, FullVkRenderPass rp, VkSemaphore imageAvailableSemaphore, VkFence renderFinishedFence){
    vkCmdEndRenderPass(cbuffer);
    vkEndCommandBuffer(cbuffer);
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    VkSubmitInfo sinfo{};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &cbuffer;
    sinfo.waitSemaphoreCount = 1;
    sinfo.pWaitSemaphores = &imageAvailableSemaphore;
    sinfo.pWaitDstStageMask = &stageMask;
    sinfo.signalSemaphoreCount = 1;
    sinfo.pSignalSemaphores = &rp.signalSemaphore;
    //VkFence fence{};
    //VkFenceCreateInfo finfo{};
    //finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    //if(vkCreateFence(g_vulkanstate.device, &finfo, nullptr, &fence) != VK_SUCCESS){
    //    throw std::runtime_error("Could not create fence");
    //}
    if(vkQueueSubmit(g_vulkanstate.queue.graphicsQueue, 1, &sinfo, renderFinishedFence) != VK_SUCCESS){
        throw std::runtime_error("Could not submit commandbuffer");
    }
    //if(vkWaitForFences(g_vulkanstate.device, 1, &fence, VK_TRUE, ~0) != VK_SUCCESS){
    //    throw std::runtime_error("Could not wait for fence");
    //}
    //vkDestroyFence(g_vulkanstate.device, fence, nullptr);
}//
extern "C" Texture LoadTexturePro_Vk(uint32_t width, uint32_t height, PixelFormat format, int usage, uint32_t sampleCount, uint32_t mipmaps, const void* data = nullptr);
extern "C" Texture LoadTextureFromImage_Vk(Image img);
void UnloadTexture_Vk(Texture tex);
extern "C" DescribedShaderModule LoadShaderModuleFromSPIRV_Vk(const uint32_t* vscode, size_t vscodeSizeInBytes, const uint32_t* fscode, size_t fscodeSizeInBytes);
extern "C" DescribedBindGroupLayout LoadBindGroupLayout_Vk(const ResourceTypeDescriptor* descs, uint32_t uniformCount);
extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
extern "C" DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count);
extern "C" void UpdateBindGroup_Vk(DescribedBindGroup* bg);
extern "C" DescribedBuffer* GenBufferEx_Vk(const void *data, size_t size, BufferUsage usage);
extern "C" void UnloadBuffer_Vk(DescribedBuffer* buf);

//wgvk I guess
extern "C" void wgvkReleaseCommandBuffer(CommmandBufferHandle commandBuffer);
extern "C" void wgvkReleaseRenderPassEncoder(RenderPassEncoderHandle rpenc);
extern "C" void wgvkReleaseBuffer(BufferHandle commandBuffer);
extern "C" void wgvkReleaseDescriptorSet(DescriptorSetHandle commandBuffer);

extern "C" void wgvkRenderpassEncoderDraw(RenderPassEncoderHandle rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
extern "C" void wgvkRenderpassEncoderDrawIndexed(RenderPassEncoderHandle rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance);
extern "C" void wgvkRenderPassEncoderBindDescriptorSet(RenderPassEncoderHandle rpe, uint32_t group, DescriptorSetHandle dset);
extern "C" void wgvkRenderPassEncoderBindPipeline(RenderPassEncoderHandle rpe, DescribedPipeline* pipeline);
extern "C" void wgvkRenderPassEncoderBindIndexBuffer(RenderPassEncoderHandle rpe, BufferHandle buffer, VkDeviceSize offset, VkIndexType indexType);
extern "C" void wgvkRenderPassEncoderBindVertexBuffer(RenderPassEncoderHandle rpe, uint32_t binding, BufferHandle buffer, VkDeviceSize offset);

extern "C" void UpdateBindGroupEntry_Vk(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);





static inline void SetBindgroupSampler_Vk(DescribedBindGroup* bg, uint32_t index, DescribedSampler smp){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.sampler = smp.sampler;
    
    UpdateBindGroupEntry_Vk(bg, index, entry);
}
static inline void SetBindgroupTexture_Vk(DescribedBindGroup* bg, uint32_t index, Texture tex){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry_Vk(bg, index, entry);
}
static inline void BindVertexArray_Vk(RenderPassEncoderHandle rpenc, VertexArray* vao){
    for(uint32_t i = 0;i < vao->buffers.size();i++){
        wgvkRenderPassEncoderBindVertexBuffer(rpenc, i, (BufferHandle)vao->buffers[i].first->buffer, 0);
    }
}






#endif