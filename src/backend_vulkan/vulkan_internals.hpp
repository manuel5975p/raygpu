#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#include <vulkan/vulkan.h>
#include <vector>
#include <raygpu.h>
struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    // Separate queues for clarity
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE; // Example for an additional queue type

    // Separate queue family indices
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    uint32_t computeFamily = UINT32_MAX; // Example for an additional queue type

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;

    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;

    VkExtent2D swapchainExtent = {0, 0};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainImageFramebuffers;

    VkBuffer vbuffer; 
};
extern "C" Texture LoadTexturePro_Vk(uint32_t width, uint32_t height, PixelFormat format, int usage, uint32_t sampleCount, uint32_t mipmaps, const void* data = nullptr);
extern "C" Texture LoadTextureFromImage_Vk(Image img);
extern VulkanState g_vulkanstate; 
#endif