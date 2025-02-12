#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#include "small_vector.hpp"
#include <unordered_set>
#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <raygpu.h>
#include <internals.hpp>
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
struct QueueIndices{
    uint32_t graphicsIndex;
    uint32_t computeIndex;
    uint32_t transferIndex;
    uint32_t presentIndex;
};

struct VertexAndFragmentShaderModuleImpl;
struct WGVKBindGroupImpl;
struct WGVKBufferImpl;
struct WGVKRenderPassEncoderImpl;
struct WGVKCommandEncoderImpl;
struct WGVKCommandBufferImpl;
struct WGVKTextureImpl;

typedef VertexAndFragmentShaderModuleImpl* VertexAndFragmentShaderModule;
typedef WGVKBindGroupImpl* WGVKBindGroup;
typedef WGVKBufferImpl* WGVKBuffer;
typedef WGVKRenderPassEncoderImpl* WGVKRenderPassEncoder;
typedef WGVKCommandBufferImpl* WGVKCommandBuffer;
typedef WGVKCommandEncoderImpl* WGVKCommandEncoder;
typedef WGVKTextureImpl* WGVKTexture;

using refcount_type = uint32_t;
template<typename T>
using ref_holder = std::unordered_set<T>;

typedef struct VertexAndFragmentShaderModuleImpl{
    VkShaderModule vModule;
    VkShaderModule fModule;
}VertexAndFragmentShaderModuleImpl;

typedef struct WGVKBindGroupImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    refcount_type refCount;
}DescriptorSetHandleImpl;

typedef struct WGVKBufferImpl{
    VkBuffer buffer;
    VkDeviceMemory memory;
    refcount_type refCount;
}WGVKBufferImpl;

typedef struct WGVKRenderPassEncoderImpl{
    VkCommandBuffer cmdBuffer;
    ref_holder<WGVKBuffer> referencedBuffers;
    ref_holder<WGVKBindGroup> referencedDescriptorSets;
    VkPipelineLayout lastLayout;
    VkFramebuffer frameBuffer;
    refcount_type refCount;
}RenderPassEncoderHandleImpl;

typedef struct WGVKCommandBufferImpl{
    ref_holder<WGVKRenderPassEncoder> referencedRPs;
    VkCommandBuffer buffer;
    VkCommandPool pool;
}WGVKCommandBufferImpl;
typedef struct WGVKCommandEncoderImpl{

}WGVKCommandEncoderImpl;

typedef struct WGVKTextureImpl{
    VkImage image;
    VkDeviceMemory memory;
}ImageHandleImpl;

typedef struct AttachmentDescriptor{
    PixelFormat format;
    uint32_t sampleCount;
    LoadOp loadop;
    StoreOp storeop;
}AttachmentDescriptor;

constexpr uint32_t max_attachments = 8;
typedef struct RenderPassLayout{
    uint32_t attachmentCount;
    AttachmentDescriptor attachments[max_attachments];
}RenderPassLayout;

struct xorshiftstate{
    uint64_t x64;
    void update(uint32_t x, uint32_t y)noexcept{
        x64 ^= ((uint64_t(x) << 32) | uint64_t(y)) * 0x2545F4914F6CDD1D;
        x64 ^= x64 << 13;
        x64 ^= x64 >> 7;
        x64 ^= x64 << 17;
    }
};
namespace std{
    template<>
    struct hash<RenderPassLayout>{
        inline constexpr size_t operator()(const RenderPassLayout& layout)const noexcept{

            xorshiftstate ret{0x2545F4918F6CDD1D};
            for(uint32_t i = 0;i < layout.attachmentCount;i++){
                ret.update(layout.attachments[i].format, layout.attachments[i].sampleCount);
                ret.update(layout.attachments[i].loadop, layout.attachments[i].storeop);
            }
            return ret.x64;
        }
    };
}
typedef struct WGVKRenderPassColorAttachment{
    void* nextInChain;
    NativeImageViewHandle view;
    NativeImageViewHandle resolveTarget;
    uint32_t depthSlice;
    LoadOp loadOp;
    StoreOp storeOp;
    DColor clearValue;
}WGVKRenderPassColorAttachment;

typedef struct WGVKRenderPassDepthStencilAttachment{
    void* nextInChain;
    NativeImageViewHandle view;
    LoadOp depthLoadOp;
    StoreOp depthStoreOp;
    float depthClearValue;
    uint32_t depthReadOnly;
    LoadOp stencilLoadOp;
    StoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    uint32_t stencilReadOnly;
}WGVKRenderPassDepthStencilAttachment;

typedef struct WGVKRenderPassDescriptor{
    void* nextInChain;
    WGVKStringView label;
    size_t colorAttachmentCount;
    WGVKRenderPassColorAttachment const* colorAttachments;
    WGVKRenderPassDepthStencilAttachment const * depthStencilAttachment;
    
    //idgaf members
    void* occlusionQuerySet;
    void const *timestampWrites;
}WGVKRenderPassDescriptor;
inline bool is__depth(PixelFormat fmt){
    return fmt ==  Depth24 || fmt == Depth32;
}

static inline VkRenderPass LoadRenderPassFromLayout(VkDevice device, RenderPassLayout layout) {
    VkAttachmentDescription vkAttachments[max_attachments] = {}; // array for Vulkan attachments

    [[maybe_unused]] uint32_t colorAttachmentCount = 0;
    uint32_t depthAttachmentIndex = VK_ATTACHMENT_UNUSED; // index for depth attachment if any

    // Convert custom attachments to Vulkan attachments
    for (uint32_t i = 0; i < layout.attachmentCount; i++) {
        const AttachmentDescriptor &att = layout.attachments[i];
        vkAttachments[i].format     = toVulkanPixelFormat(att.format);
        vkAttachments[i].loadOp     = toVulkanLoadOperation(att.loadop);
        vkAttachments[i].storeOp    = toVulkanStoreOperation(att.storeop);
        vkAttachments[i].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vkAttachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vkAttachments[i].initialLayout  = (att.loadop == LoadOp_Load ? (is__depth(att.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) : (VK_IMAGE_LAYOUT_UNDEFINED));
        if (is__depth(att.format)) {
            vkAttachments[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachmentIndex = i;
        } else {
            vkAttachments[i].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++colorAttachmentCount;
        }
    }

    // Set up color attachment references for the subpass.
    VkAttachmentReference colorRefs[max_attachments] = {}; // list of color attachments
    uint32_t colorIndex = 0;
    for (uint32_t i = 0; i < layout.attachmentCount; i++) {
        if (!is__depth(layout.attachments[i].format)) {
            colorRefs[colorIndex].attachment = i;
            colorRefs[colorIndex].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++colorIndex;
        }
    }

    // Set up subpass description.
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = colorIndex;
    subpass.pColorAttachments       = colorIndex ? colorRefs : nullptr;
    // Assign depth attachment if present.
    VkAttachmentReference depthRef = {};
    if (depthAttachmentIndex != VK_ATTACHMENT_UNUSED) {
        depthRef.attachment = depthAttachmentIndex;
        depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &depthRef;
    } else {
        subpass.pDepthStencilAttachment = nullptr;
    }

    // Create render pass create info.
    VkRenderPassCreateInfo rpci = {};
    rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = layout.attachmentCount;
    rpci.pAttachments    = vkAttachments;
    rpci.subpassCount    = 1;
    rpci.pSubpasses      = &subpass;
    // (Optional: add subpass dependencies if needed.)

    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkResult result = vkCreateRenderPass(device, &rpci, nullptr, &renderPass);
    // (Handle errors appropriately in production code)
    return renderPass;
}


struct WGVKSurfaceImpl{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    uint32_t imagecount;

    uint32_t activeImageIndex;
    uint32_t width, height;
    VkFormat swapchainImageFormat;
    VkColorSpaceKHR swapchainColorSpace;
    VkImage* images;
    VkImageView* imageViews;
    VkFramebuffer* framebuffers;

    uint32_t formatCount;
    PixelFormat* formatCache;
    uint32_t presentModeCount;
    SurfacePresentMode* presentModeCache;
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
struct SyncState{
    //VkSemaphore imageAvailableSemaphores[1];
    //VkSemaphore presentSemaphores[1];
    
    uint32_t submitsInThisFrame;
    std::vector<VkSemaphore> semaphoresInThisFrame;
    VkFence renderFinishedFence;
    VkSemaphore getSemaphoreOfSubmitIndex(uint32_t index);
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
    SyncState syncState;
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

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
extern "C" void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, VkPhysicalDevice adapter, SurfaceCapabilities* capabilities);

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
inline FullSurface LoadSurface(GLFWwindow* window, SurfaceConfiguration config){
    FullSurface retf{};
    WGVKSurface retp = callocnew(WGVKSurfaceImpl);
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
}

extern "C" void wgvkSurfaceConfigure(WGVKSurfaceImpl* surface, const SurfaceConfiguration* config);


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
extern "C" void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
static inline VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0){
    VkSemaphoreCreateInfo sci zeroinit;
    VkSemaphore ret zeroinit;
    sci.flags = flags;
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult res = vkCreateSemaphore(g_vulkanstate.device, &sci, nullptr, &ret);
    if(res != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Error creating semaphore");
    }
    return ret;
}
static inline VkFence CreateFence(VkFenceCreateFlags flags = 0){
    VkFenceCreateInfo sci zeroinit;
    VkFence ret zeroinit;
    sci.flags = flags;
    sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult res = vkCreateFence(g_vulkanstate.device, &sci, nullptr, &ret);
    if(res != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Error creating fence");
    }
    return ret;
}
extern "C" void BeginRenderpassEx(DescribedRenderpass *renderPass);


static inline WGVKRenderPassEncoder BeginRenderPass_Vk(VkCommandBuffer cbuffer, DescribedRenderpass* rp, VkFramebuffer fb, uint32_t width, uint32_t height){

    WGVKRenderPassEncoder ret = callocnewpp(RenderPassEncoderHandleImpl);
    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    VkClearValue clearvalues[2] = {};
    clearvalues[0].color = VkClearColorValue{};
    clearvalues[0].color.float32[0] = rp->colorClear.r;
    clearvalues[0].color.float32[1] = rp->colorClear.g;
    clearvalues[0].color.float32[2] = rp->colorClear.b;
    clearvalues[0].color.float32[3] = rp->colorClear.a;
    clearvalues[1].depthStencil = VkClearDepthStencilValue{};
    clearvalues[1].depthStencil.depth = rp->depthClear;

    rpbi.renderPass = (VkRenderPass)rp->VkRenderPass;
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.clearValueCount = (rp->colorLoadOp == LoadOp_Clear) ? 2 : 0;
    rpbi.pClearValues = clearvalues;
    rpbi.framebuffer = fb;
    rpbi.renderArea.extent.width = width;//g_vulkanstate.surface.surfaceConfig.width;
    rpbi.renderArea.extent.height = height;//g_vulkanstate.surface.surfaceConfig.height;
    rpbi.renderPass = (VkRenderPass)rp->VkRenderPass;

    vkBeginCommandBuffer(cbuffer, &bbi);
    vkCmdBeginRenderPass(cbuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    ret->cmdBuffer = cbuffer;
    ret->refCount = 1;
    rp->rpEncoder = ret;
    return ret;
}
extern "C" void EndRenderPass(VkCommandBuffer cbuffer, DescribedRenderpass* rp);


extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps);
extern "C" Texture LoadTextureFromImage(Image img);
extern "C" void UnloadTexture(Texture tex);
extern "C" DescribedShaderModule LoadShaderModuleFromSPIRV_Vk(const uint32_t* vscode, size_t vscodeSizeInBytes, const uint32_t* fscode, size_t fscodeSizeInBytes);

extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
extern "C" DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count);
extern "C" void UpdateBindGroup(DescribedBindGroup* bg);
extern "C" DescribedBuffer* GenBufferEx(const void *data, size_t size, BufferUsage usage);
extern "C" void UnloadBuffer(DescribedBuffer* buf);

//wgvk I guess

extern "C" WGVKBuffer wgvkDeviceCreateBuffer(VkDevice device, const BufferDescriptor* desc);
extern "C" void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, void const * data, size_t size);


extern "C" void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer);
extern "C" void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc);
extern "C" void wgvkReleaseBuffer(WGVKBuffer commandBuffer);
extern "C" void wgvkReleaseDescriptorSet(WGVKBindGroup commandBuffer);

extern "C" void wgvkRenderpassEncoderDraw(WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
extern "C" void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance);
extern "C" void wgvkRenderPassEncoderBindDescriptorSet(WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset);
extern "C" void wgvkRenderPassEncoderBindPipeline(WGVKRenderPassEncoder rpe, DescribedPipeline* pipeline);
extern "C" void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, VkPipeline pipeline, VkPipelineLayout layout);
extern "C" void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
extern "C" void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset);

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
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}
static inline void BindVertexArray_Vk(WGVKRenderPassEncoder rpenc, VertexArray* vao){
    for(uint32_t i = 0;i < vao->buffers.size();i++){
        wgvkRenderPassEncoderBindVertexBuffer(rpenc, i, (WGVKBuffer)vao->buffers[i].first->buffer, 0);
    }
}






#endif