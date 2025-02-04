#include <SDL3/SDL.h>
#include <raygpu.h>
#if SUPPORT_VULKAN_BACKEND == 1
#include <vulkan/vulkan.h>
#include <SDL3/SDL_vulkan.h>
#endif
typedef struct SurfaceAndSwapchainSupport{
    PixelFormat supportedFormats[16];
    uint32_t supportedFormatCount;
    SurfacePresentMode supportedPresentModes[4];
    uint32_t supportedPresentModeCount;

    uint32_t presentQueueIndex; // Vulkan only
}SurfaceAndSwapchainSupport;
SurfaceAndSwapchainSupport QuerySurfaceAndSwapchainSupport(void* adapterHandle){
    SurfaceAndSwapchainSupport ret zeroinit;
    #if SUPPORT_VULKAN_BACKEND == 1
    
    #endif
}


void negotiateSurfaceFormatAndPresentMode(void* nsurface){
    //TODO    
}