#include <SDL3/SDL.h>
#include <raygpu.h>
#if SUPPORT_VULKAN_BACKEND == 1
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#endif
#include <webgpu/webgpu.h>

constexpr uint32_t max_format_count = 16;

typedef struct SurfaceAndSwapchainSupport{
    PixelFormat supportedFormats[max_format_count];
    uint32_t supportedFormatCount;
    SurfacePresentMode supportedPresentModes[4];
    uint32_t supportedPresentModeCount;

    uint32_t presentQueueIndex; // Vulkan only
}SurfaceAndSwapchainSupport;

uint32_t GetPresentQueueIndex(void* instanceHandle, void* adapterHandle){
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queueFamilyCount, queueFamilies.data());
    for(uint32_t index = 0;index < queueFamilyCount;index++){
        bool ps = SDL_Vulkan_GetPresentationSupport((VkInstance)instanceHandle, (VkPhysicalDevice)adapterHandle, index);
        if(ps){
            return index;
        }
    }
    return ~0;
}



SurfaceAndSwapchainSupport QuerySurfaceAndSwapchainSupport(void* instanceHandle, void* adapterHandle, void* surfaceHandle){
    SurfaceAndSwapchainSupport ret zeroinit;

    if(surfaceHandle == 0){
        return ret;
    }
    
    #if SUPPORT_VULKAN_BACKEND == 1
    VkInstance instance = (VkInstance)instanceHandle;
    VkPhysicalDevice adapter = (VkPhysicalDevice)adapterHandle;
    VkSurfaceKHR surface = (VkSurfaceKHR)surfaceHandle; 
    
    ret.presentQueueIndex = GetPresentQueueIndex(instanceHandle, adapterHandle);
    VkSurfaceCapabilitiesKHR capabilities zeroinit;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &capabilities);
    uint32_t formatCount;

    vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, nullptr);
    
    if(formatCount != 0) {
        ret.supportedFormatCount = formatCount;
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &formatCount, formats.data());
        for(uint32_t i = 0;i < formatCount;i++){
            ret.supportedFormats[i] = fromVulkanPixelFormat(formats[i].format);
        }
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &presentModeCount, presentModes.data());
    
    #endif
}


void negotiateSurfaceFormatAndPresentMode(void* nsurface){
    //TODO    
}