//#include "vulkan/vulkan_core.h"
#include <inttypes.h>

#if SUPPORT_WAYLAND_SURFACE == 1
    #define VK_KHR_wayland_surface 1 // Define set to 1 for clarity
#endif
#if SUPPORT_WIN32_SURFACE == 1
    #define VK_KHR_win32_surface 1 // Define set to 1 for clarity
#endif
#if SUPPORT_ANDROID_SURFACE == 1
    #define VK_KHR_android_surface 1 // Define set to 1 for clarity
#endif
#if SUPPORT_METAL_SURFACE == 1 // For macOS/iOS using MoltenVK
    #define VK_EXT_metal_surface 1 // Define set to 1 for clarity (Note: EXT, not KHR)
#endif

#if SUPPORT_XLIB_SURFACE == 1
    #include <X11/Xlib.h>
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_xlib.h>
#endif

#if SUPPORT_WAYLAND_SURFACE == 1
    #include <wayland-client.h>
    #include <wayland-client-protocol.h>
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_wayland.h>
#endif

#if SUPPORT_WIN32_SURFACE == 1
    #include <windows.h>
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_win32.h>
#endif

#if SUPPORT_ANDROID_SURFACE == 1
    #include <android/native_window.h>
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_android.h>
#endif

#if SUPPORT_METAL_SURFACE == 1 // For macOS/iOS using MoltenVK
    #define VK_EXT_metal_surface 1 // Define set to 1 for clarity (Note: EXT, not KHR)
    // No specific native C header needed here usually, CAMetalLayer is often handled via void*
    // If Objective-C interop is used elsewhere, #import <Metal/Metal.h> would be needed there.
    #define VK_NO_PROTOTYPES
    #include <vulkan/vulkan.h>
    #include <vulkan/vulkan_metal.h>
#endif
#include <external/volk.h>

#define Font rlFont
#define Matrix rlMatrix
    #include <wgvk_structs_impl.h>
    //#include <enum_translation.h>
    #include <tint_c_api.h>
    //#include "vulkan_internals.hpp"
    //#include <raygpu.h>
#undef Font
#undef Matrix



#ifdef TRACELOG
    #undef TRACELOG
#endif
#define TRACELOG(level, ...) wgpuTraceLog(level, __VA_ARGS__)
void wgpuTraceLog(int logType, const char *text, ...);
#include "../amalgamation/SPIRV-Reflect/spirv_reflect.c"
#include <macros_and_constants.h>
#include <wgvk.h>
#include <external/VmaUsage.h>
#include <stdarg.h>


// WGPU struct implementations

#include <wgvk_structs_impl.h>

static inline uint32_t findMemoryType(WGPUAdapter adapter, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    
    if(adapter->memProperties.memoryTypeCount == 0){
        vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &adapter->memProperties);
    }

    for (uint32_t i = 0; i < adapter->memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (adapter->memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    assert(false && "failed to find suitable memory type!");
    return ~0u;
}



WGPUSurface wgpuInstanceCreateSurface(WGPUInstance instance, const WGPUSurfaceDescriptor* descriptor){
    rassert(descriptor->nextInChain, "SurfaceDescriptor must have a nextInChain");
    WGPUSurface ret = callocnew(WGPUSurfaceImpl);
    ret->refCount = 1;
    switch(descriptor->nextInChain->sType){
        #if SUPPORT_METAL_SURFACE
        case WGPUSType_SurfaceSourceMetalLayer:{

        }break;
        #endif
        #if SUPPORT_WIN32_SURFACE
        case WGPUSType_SurfaceSourceWindowsHWND:{

        }break;
        #endif
        #if SUPPORT_XLIB_SURFACE
        case WGPUSType_SurfaceSourceXlibWindow:{
            WGPUSurfaceSourceXlibWindow* xlibSource = (WGPUSurfaceSourceXlibWindow*)descriptor->nextInChain;
            VkXlibSurfaceCreateInfoKHR sci = {
                .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                .window = (uint64_t)xlibSource->window,
                .dpy = (Display*)xlibSource->display
            };
            //((PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance->instance, "vkCreateXlibSurfaceKHR"))
            vkCreateXlibSurfaceKHR
            (
                instance->instance,
                &sci,
                NULL,
                &ret->surface
            );
            
        }break;
        #endif

        #if SUPPORT_WAYLAND_SURFACE
        case WGPUSType_SurfaceSourceWaylandSurface:{
            WGPUSurfaceSourceWaylandSurface* waylandSource = (WGPUSurfaceSourceWaylandSurface*)descriptor->nextInChain;
            VkWaylandSurfaceCreateInfoKHR sci = {
                .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                .surface = (struct wl_surface*)waylandSource->surface,
                .display = (struct wl_display*)waylandSource->display
            };
            vkCreateWaylandSurfaceKHR(
                instance->instance,
                &sci,
                NULL,
                &ret->surface
            );
        }break;
        #endif

        #if SUPPORT_ANDROID_SURFACE
        case WGPUSType_SurfaceSourceAndroidNativeWindow:{
            
        }break;
        #endif
        #if SUPPORT_XCB_SURFACE
        case WGPUSType_SurfaceSourceXCBWindow:{

        }break;
        #endif
        default:{
            rassert(false, "Unsupported SType for SurfaceDescriptor.nextInChain");
        }
    }
    return ret;
}

static RenderPassLayout GetRenderPassLayout2(const RenderPassCommandBegin* rpdesc){
    RenderPassLayout ret zeroinit;
    if(rpdesc->depthAttachmentPresent){
        ret.depthAttachmentPresent = 1U;
        ret.depthAttachment = CLITERAL(AttachmentDescriptor){
            .format = rpdesc->depthStencilAttachment.view->format, 
            .sampleCount = rpdesc->depthStencilAttachment.view->sampleCount,
            .loadop = rpdesc->depthStencilAttachment.depthLoadOp,
            .storeop = rpdesc->depthStencilAttachment.depthStoreOp
        };
    }

    ret.colorAttachmentCount = rpdesc->colorAttachmentCount;
    rassert(ret.colorAttachmentCount < MAX_COLOR_ATTACHMENTS, "Too many color attachments");
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        ret.colorAttachments[i] = CLITERAL(AttachmentDescriptor){
            .format = rpdesc->colorAttachments[i].view->format, 
            .sampleCount = rpdesc->colorAttachments[i].view->sampleCount,
            .loadop = rpdesc->colorAttachments[i].loadOp,
            .storeop = rpdesc->colorAttachments[i].storeOp
        };
        bool ihasresolve = rpdesc->colorAttachments[i].resolveTarget;
        if(i > 0){
            bool iminus1hasresolve = rpdesc->colorAttachments[i - 1].resolveTarget;
            rassert(ihasresolve == iminus1hasresolve, "Some of the attachments have resolve, others do not, impossible");
        }
        if(rpdesc->colorAttachments[i].resolveTarget != 0){
            ret.colorResolveAttachments[i] = CLITERAL(AttachmentDescriptor){
                .format = rpdesc->colorAttachments[i].resolveTarget->format, 
                .sampleCount = rpdesc->colorAttachments[i].resolveTarget->sampleCount,
                .loadop = rpdesc->colorAttachments[i].loadOp,
                .storeop = rpdesc->colorAttachments[i].storeOp
            };
        }
    }
    return ret;
}

RenderPassLayout GetRenderPassLayout(const WGPURenderPassDescriptor* rpdesc){
    RenderPassLayout ret zeroinit;
    //ret.colorResolveIndex = VK_ATTACHMENT_UNUSED;
    
    if(rpdesc->depthStencilAttachment){
        rassert(rpdesc->depthStencilAttachment->view, "Depth stencil attachment passed but null view");
        ret.depthAttachmentPresent = 1U;
        ret.depthAttachment = CLITERAL(AttachmentDescriptor){
            .format = rpdesc->depthStencilAttachment->view->format, 
            .sampleCount = rpdesc->depthStencilAttachment->view->sampleCount,
            .loadop = rpdesc->depthStencilAttachment->depthLoadOp,
            .storeop = rpdesc->depthStencilAttachment->depthStoreOp
        };
    }
    

    
    ret.colorAttachmentCount = rpdesc->colorAttachmentCount;
    rassert(ret.colorAttachmentCount < MAX_COLOR_ATTACHMENTS, "Too many color attachments");
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        ret.colorAttachments[i] = CLITERAL(AttachmentDescriptor){
            .format = rpdesc->colorAttachments[i].view->format, 
            .sampleCount = rpdesc->colorAttachments[i].view->sampleCount,
            .loadop = rpdesc->colorAttachments[i].loadOp,
            .storeop = rpdesc->colorAttachments[i].storeOp
        };
        bool ihasresolve = rpdesc->colorAttachments[i].resolveTarget;
        if(i > 0){
            bool iminus1hasresolve = rpdesc->colorAttachments[i - 1].resolveTarget;
            rassert(ihasresolve == iminus1hasresolve, "Some of the attachments have resolve, others do not, impossible");
        }
        if(rpdesc->colorAttachments[i].resolveTarget != 0){
            ret.colorResolveAttachments[i] = CLITERAL(AttachmentDescriptor){
                .format = rpdesc->colorAttachments[i].resolveTarget->format, 
                .sampleCount = rpdesc->colorAttachments[i].resolveTarget->sampleCount,
                .loadop = rpdesc->colorAttachments[i].loadOp,
                .storeop = rpdesc->colorAttachments[i].storeOp
            };
        }
    }
    return ret;
}
static inline bool is__depth(PixelFormat fmt){
    return fmt == Depth24 || fmt == Depth32;
}
static inline bool is__depthVk(VkFormat fmt){
    return fmt == VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT;
}

static inline VkSampleCountFlagBits toVulkanSampleCount(uint32_t samples){
    if(samples & (samples - 1)){
        return VK_SAMPLE_COUNT_1_BIT;
    }
    else{
        switch(samples){
            case 2: return VK_SAMPLE_COUNT_2_BIT;
            case 4: return VK_SAMPLE_COUNT_4_BIT;
            case 8: return VK_SAMPLE_COUNT_8_BIT;
            case 16: return VK_SAMPLE_COUNT_16_BIT;
            case 32: return VK_SAMPLE_COUNT_32_BIT;
            case 64: return VK_SAMPLE_COUNT_64_BIT;
            default: return VK_SAMPLE_COUNT_1_BIT;
        }
    }
}

static VkAttachmentDescription atttransformFunction(AttachmentDescriptor att){
    VkAttachmentDescription ret zeroinit;
    ret.samples    = toVulkanSampleCount(att.sampleCount);
    ret.format     = att.format;
    ret.loadOp     = toVulkanLoadOperation(att.loadop);
    ret.storeOp    = toVulkanStoreOperation(att.storeop);
    ret.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ret.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ret.initialLayout  = (att.loadop == WGPULoadOp_Load ? (is__depthVk(att.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) : (VK_IMAGE_LAYOUT_UNDEFINED));
    if(is__depthVk(att.format)){
        ret.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }else{
        ret.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    return ret;
};

LayoutedRenderPass LoadRenderPassFromLayout(WGPUDevice device, RenderPassLayout layout){
    LayoutedRenderPass* lrp = RenderPassCache_get(&device->renderPassCache, layout);
    if(lrp)
        return *lrp;

    TRACELOG(LOG_INFO, "Loading new renderpass");
    
    VkAttachmentDescriptionVector allAttachments;
    VkAttachmentDescriptionVector_init(&allAttachments);
    uint32_t depthAttachmentIndex = VK_ATTACHMENT_UNUSED; // index for depth attachment if any
    uint32_t colorResolveIndex = VK_ATTACHMENT_UNUSED; // index for depth attachment if any
    for(uint32_t i = 0; i < layout.colorAttachmentCount;i++){
        VkAttachmentDescriptionVector_push_back(&allAttachments, atttransformFunction(layout.colorAttachments[i]));
    }
    
    if(layout.depthAttachmentPresent){
        depthAttachmentIndex = allAttachments.size;
        VkAttachmentDescriptionVector_push_back(&allAttachments, atttransformFunction(layout.depthAttachment));
    }
    //TODO check if there
    if(layout.colorAttachmentCount && layout.colorResolveAttachments[0].format){
        colorResolveIndex = allAttachments.size;
        for(uint32_t i = 0;i < layout.colorAttachmentCount;i++){
            VkAttachmentDescriptionVector_push_back(&allAttachments, atttransformFunction(layout.colorResolveAttachments[i]));
        }
    }

    
    uint32_t colorAttachmentCount = layout.colorAttachmentCount;
    

    // Set up color attachment references for the subpass.
    VkAttachmentReference colorRefs[MAX_COLOR_ATTACHMENTS] = {0}; // list of color attachments
    uint32_t colorIndex = 0;
    for (uint32_t i = 0; i < layout.colorAttachmentCount; i++) {
        if (!is__depthVk(layout.colorAttachments[i].format)) {
            colorRefs[colorIndex].attachment = i;
            colorRefs[colorIndex].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++colorIndex;
        }
    }

    // Set up subpass description.
    VkSubpassDescription subpass zeroinit;
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = colorIndex;
    subpass.pColorAttachments       = colorIndex ? colorRefs : NULL;


    // Assign depth attachment if present.
    VkAttachmentReference depthRef = {};
    if (depthAttachmentIndex != VK_ATTACHMENT_UNUSED) {
        depthRef.attachment = depthAttachmentIndex;
        depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        subpass.pDepthStencilAttachment = &depthRef;
    } else {
        subpass.pDepthStencilAttachment = NULL;
    }

    VkAttachmentReference resolveRef = {};
    if (colorResolveIndex != VK_ATTACHMENT_UNUSED) {
        resolveRef.attachment = colorResolveIndex;
        resolveRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpass.pResolveAttachments = &resolveRef;
    } else {
        subpass.pResolveAttachments = NULL;
    }
    

    VkRenderPassCreateInfo rpci = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        NULL,
        0,
        allAttachments.size, allAttachments.data,
        1, &subpass,
        0, NULL
    };
    
    // (Optional: add subpass dependencies if needed.)
    LayoutedRenderPass ret zeroinit;
    VkAttachmentDescriptionVector_move(&ret.allAttachments, &allAttachments);
    //ret.allAttachments = std::move(allAttachments);
    VkResult result = device->functions.vkCreateRenderPass(device->device, &rpci, NULL, &ret.renderPass);
    // (Handle errors appropriately in production code)
    if(result == VK_SUCCESS){
        RenderPassCache_put(&device->renderPassCache, layout, ret);
        //device->renderPassCache.emplace(layout, ret);
        ret.layout = layout;
        return ret;
    }
    TRACELOG(LOG_FATAL, "Error creating renderpass: %s", vkErrorString(result));
    rg_trap();
    return ret;
}

void wgpuTraceLog(int logType, const char *text, ...){
    // Message has level below current threshold, don't emit
    //if(logType < tracelogLevel)return;

    va_list args;
    va_start(args, text);

    //if (traceLog){
    //    traceLog(logType, text, args);
    //    va_end(args);
    //    return;
    //}
    #define MAX_TRACELOG_MSG_LENGTH 16384
    char buffer[MAX_TRACELOG_MSG_LENGTH] = {0};
    int needs_reset = 0;
    switch (logType){
        case LOG_TRACE:   strcpy(buffer, "TRACE: "); break;
        case LOG_DEBUG:   strcpy(buffer, "DEBUG: "); break;
        case LOG_INFO:    strcpy(buffer, TERMCTL_GREEN "INFO: "); needs_reset = 1;break;
        case LOG_WARNING: strcpy(buffer, TERMCTL_YELLOW "WARNING: ");needs_reset = 1; break;
        case LOG_ERROR:   strcpy(buffer, TERMCTL_RED "ERROR: ");needs_reset = 1; break;
        case LOG_FATAL:   strcpy(buffer, TERMCTL_RED "FATAL: "); break;
        default: break;
    }
    size_t offset_now = strlen(buffer);
    
    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (MAX_TRACELOG_MSG_LENGTH - 12))? textSize : (MAX_TRACELOG_MSG_LENGTH - 12));
    if(needs_reset){
        strcat(buffer, TERMCTL_RESET "\n");
    }
    else{
        strcat(buffer, "\n");
    }
    vfprintf(stderr, buffer, args);
    fflush  (stderr);

    va_end(args);
    // If fatal logging, exit program
    if (logType == LOG_FATAL){
        fputs(TERMCTL_RED "Exiting due to fatal error!\n" TERMCTL_RESET, stderr);
        rg_trap();
        exit(EXIT_FAILURE); 
    }

}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT){
        wgpuTraceLog(LOG_ERROR, pCallbackData->pMessage);
        rg_trap();
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        wgpuTraceLog(LOG_WARNING, pCallbackData->pMessage);
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
        wgpuTraceLog(LOG_INFO, pCallbackData->pMessage);
    }

    return VK_FALSE;
}

static inline bool isWritingAccess(VkAccessFlags flags){
    return flags & (
          VK_ACCESS_SHADER_WRITE_BIT
        | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_TRANSFER_WRITE_BIT
        | VK_ACCESS_HOST_WRITE_BIT
        | VK_ACCESS_MEMORY_WRITE_BIT
        | VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT
        | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT
        | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR
        | VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_EXT
    );
}
static inline int endswith_(const char* str, const char* suffix) {
    if (!str || !suffix)
        return 0;
        
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    if (suffix_len > str_len)
        return 0;
        
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}
WGPUInstance wgpuCreateInstance(const WGPUInstanceDescriptor* descriptor) {
#if SUPPORT_VULKAN_BACKEND == 0
    fprintf(stderr, "Vulkan backend is not enabled in this build.\n");
    return NULL;
#else

    WGPUInstance ret = (WGPUInstance)RL_CALLOC(1, sizeof(WGPUInstanceImpl));
    ret->refCount = 1;
    volkInitialize();
    if (!ret) {
        fprintf(stderr, "Failed to allocate memory for WGPUInstanceImpl\n");
        return NULL;
    }
    ret->instance = VK_NULL_HANDLE;
    ret->debugMessenger = VK_NULL_HANDLE;

    // 1. Define Application Info
    VkApplicationInfo appInfo zeroinit;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "WGPU Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WGPU Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3; // Request Vulkan 1.3

    // 2. Define Instance Create Info
    VkInstanceCreateInfo ici zeroinit;
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.flags = 0; // May be updated for portability
    ici.pApplicationInfo = &appInfo;

    // 3. --- Query Available Extensions and Select Required Surface/Debug Extensions ---
    uint32_t availableExtensionCount = 0;
    VkResult enumResult = vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, NULL);
    VkExtensionProperties* availableExtensions = NULL;
    const char** enabledExtensions = NULL; // Array of pointers to enabled names
    const uint32_t maxEnabledExtensions = 16;

    // Define the list of extensions we *want* to enable if available


    if (enumResult != VK_SUCCESS || availableExtensionCount == 0) {
        fprintf(stderr, "Warning: Failed to query instance extensions or none found (Error: %d). Proceeding without optional extensions.\n", (int)enumResult);
        availableExtensionCount = 0;
    } else {
        availableExtensions = (VkExtensionProperties*)RL_CALLOC(availableExtensionCount, sizeof(VkExtensionProperties));
        if (!availableExtensions) {
            fprintf(stderr, "Error: Failed to allocate memory for available instance extensions properties.\n");
            RL_FREE(ret);
            return NULL;
        }
        enumResult = vkEnumerateInstanceExtensionProperties(NULL, &availableExtensionCount, availableExtensions);
        if (enumResult != VK_SUCCESS) {
            fprintf(stderr, "Warning: Failed to retrieve instance extension properties (Error: %d). Proceeding without optional extensions.\n", (int)enumResult);
            RL_FREE(availableExtensions);
            availableExtensions = NULL;
            availableExtensionCount = 0;
        }
    }

    // Allocate buffer for the names of extensions we will actually enable
    enabledExtensions = (const char**)RL_CALLOC(maxEnabledExtensions, sizeof(const char*));
    if (!enabledExtensions) {
        RL_FREE(availableExtensions);
        RL_FREE(ret);
        return NULL;
    }

    int needsPortabilityEnumeration = 0;
    int portabilityEnumerationAvailable = 0;

    // Iterate through available extensions and enable the desired ones
    uint32_t enabledExtensionCount = 0;
    for (uint32_t i = 0; i < availableExtensionCount; ++i) {
        const char* currentExtName = availableExtensions[i].extensionName;
        if(endswith_(currentExtName, "surface") || strstr(currentExtName, "debug") != NULL){
            enabledExtensions[enabledExtensionCount++] = currentExtName;
        }
        int desired = 0;
    }

    // Handle portability enumeration: Enable it *if* needed and available
    if (needsPortabilityEnumeration) {
        if (portabilityEnumerationAvailable && enabledExtensionCount < maxEnabledExtensions) {
            enabledExtensions[enabledExtensionCount++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
            ici.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        } else if (!portabilityEnumerationAvailable) {
            fprintf(stderr, "Error: An enabled surface extension requires '%s', but it is not available! Instance creation may fail.\n", VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            // Proceed anyway, vkCreateInstance will likely fail.
        } else {
            // fprintf(stderr, "Warning: Portability enumeration needed but exceeded max enabled count (%u).\n", maxEnabledExtensions);
        }
    }


    // --- End Extension Handling ---
    ici.enabledExtensionCount = enabledExtensionCount;
    ici.ppEnabledExtensionNames = enabledExtensions;


    // 4. Specify Layers (if requested)
    const char* const* requestedLayers = NULL;
    uint32_t requestedLayerCount = 0;
    VkValidationFeaturesEXT validationFeatures zeroinit;
    WGPUInstanceLayerSelection* ils = NULL;
    int debugUtilsAvailable = 0; // Check if debug utils was actually enabled

    for (uint32_t i = 0; i < enabledExtensionCount; ++i) {
        if (strcmp(enabledExtensions[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            debugUtilsAvailable = 1;
            break;
        }
    }

    if (descriptor && descriptor->nextInChain) {
        if (((WGPUChainedStruct*)descriptor->nextInChain)->sType == WGPUSType_InstanceValidationLayerSelection) {
            ils = (WGPUInstanceLayerSelection*)descriptor->nextInChain;
            requestedLayers = ils->instanceLayers;
            requestedLayerCount = ils->instanceLayerCount;
        }
        // TODO: Handle other potential structs in nextInChain if necessary
    }

    ici.enabledLayerCount = requestedLayerCount;
    ici.ppEnabledLayerNames = requestedLayers;
    VkValidationFeatureEnableEXT enables[] = {
        VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
    };
    // If layers are enabled, configure specific validation features, BUT only if debug utils is available
    if (requestedLayerCount > 0) {
        if(debugUtilsAvailable) {
            
            validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            validationFeatures.enabledValidationFeatureCount = sizeof(enables) / sizeof(enables[0]);
            validationFeatures.pEnabledValidationFeatures = enables;
            validationFeatures.pNext = ici.pNext; // Chain it
            ici.pNext = &validationFeatures;      // Point ici.pNext to this struct
            //fprintf(stdout, "Enabling synchronization validation feature.\n");
        } else {
            //fprintf(stderr, "Warning: Requested validation layers but VK_EXT_debug_utils extension was not found/enabled. Debug messenger and specific validation features cannot be enabled.\n");
            ici.enabledLayerCount = 0; // Disable layers if debug utils isn't there
            ici.ppEnabledLayerNames = NULL;
            requestedLayerCount = 0; // Update count for subsequent checks
            fprintf(stdout, "Disabling requested layers due to missing VK_EXT_debug_utils.\n");
        }
    } else {
        //fprintf(stdout, "Validation layers not requested or not found in descriptor chain.\n");
    }

    // 5. Create the Vulkan Instance
    VkResult result = vkCreateInstance(&ici, NULL, &ret->instance);


    // --- Free temporary extension memory ---
    RL_FREE(availableExtensions);        // Properties buffer
    RL_FREE((void*)enabledExtensions);   // Names buffer (pointers into availableExtensions)
    availableExtensions = NULL;
    enabledExtensions = NULL;
    // --- End Freeing Extension Memory ---


    if (result != VK_SUCCESS) {
        RL_FREE(ret);
        return NULL;
    }
    ret->currentFutureId = 1;
    FutureIDMap_init(&ret->g_futureIDMap);
    // 6. Load instance-level functions using volk
    volkLoadInstance(ret->instance);

    // 7. Create Debug Messenger (if layers requested AND debug utils was available/enabled)
    if (requestedLayerCount > 0 && debugUtilsAvailable && vkCreateDebugUtilsMessengerEXT) {
        VkDebugUtilsMessengerCreateInfoEXT dbgCreateInfo zeroinit;
        dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbgCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbgCreateInfo.pfnUserCallback = debugCallback;

        result = CreateDebugUtilsMessengerEXT(ret->instance, &dbgCreateInfo, NULL, &ret->debugMessenger);
        if (result != VK_SUCCESS) {
            //fprintf(stderr, "Warning: Failed to create debug messenger (Error: %d).\n", (int)result);
            ret->debugMessenger = VK_NULL_HANDLE;
        } else {
            //fprintf(stdout, "Vulkan Debug Messenger created successfully.\n");
        }
    } else if (requestedLayerCount > 0) {
        //fprintf(stdout, "Debug messenger creation skipped because VK_EXT_debug_utils extension was not available/enabled or layers were disabled.\n");
    }


    // 8. Return the created instance handle
    return ret;

#endif // SUPPORT_VULKAN_BACKEND
}
WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance instance, size_t futureCount, WGPUFutureWaitInfo* futureWaitInfos, uint64_t timeoutNS){
    for(uint32_t i = 0;i < futureCount;i++){
        if(!futureWaitInfos[i].completed){
            WGPUFutureImpl* futureObject = FutureIDMap_get(&instance->g_futureIDMap, futureWaitInfos[i].future.id);
            futureObject->functionCalledOnWaitAny(futureObject->userdataForFunction);
            if(futureObject->freeUserData){
                futureObject->freeUserData(futureObject->userdataForFunction);
            }
            futureWaitInfos[i].completed = 1;
        }
    }
    return WGPUWaitStatus_Success;
}
typedef struct userdataforcreateadapter{
    WGPUInstance instance;
    WGPURequestAdapterCallbackInfo info;
    WGPURequestAdapterOptions options;
} userdataforcreateadapter;

static inline VkPhysicalDeviceType tvkpdt(AdapterType atype){
    switch(atype){
        case DISCRETE_GPU: return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        case INTEGRATED_GPU: return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        case SOFTWARE_RENDERER: return VK_PHYSICAL_DEVICE_TYPE_CPU;
        rg_unreachable();
    }
    return (VkPhysicalDeviceType)-1;
}
void wgpuCreateAdapter_impl(void* userdata_v){
    userdataforcreateadapter* userdata = (userdataforcreateadapter*)userdata_v;
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, NULL);
    VkPhysicalDevice* pds = (VkPhysicalDevice*)RL_CALLOC(physicalDeviceCount, sizeof(VkPhysicalDevice));
    VkResult result = vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, pds);
    if(result != VK_SUCCESS){
        const char res[] = "vkEnumeratePhysicalDevices failed";
        userdata->info.callback(WGPURequestAdapterStatus_Unavailable, NULL, CLITERAL(WGPUStringView){.data = res,.length = sizeof(res) - 1},userdata->info.userdata1, userdata->info.userdata2);
        return;
    }
    uint32_t i = 0;
    for(i = 0;i < physicalDeviceCount;i++){
        VkPhysicalDeviceProperties properties zeroinit;
        vkGetPhysicalDeviceProperties(pds[i], &properties);
        if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
            break;
        }
    }
    WGPUAdapter adapter = (WGPUAdapter)RL_CALLOC(1, sizeof(WGPUAdapterImpl));
    adapter->instance = userdata->instance;
    wgpuInstanceAddRef(userdata->instance);
    adapter->refCount = 1;
    adapter->physicalDevice = pds[i];
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &adapter->memProperties);
    uint32_t QueueFamilyPropertyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &QueueFamilyPropertyCount, NULL);
    VkQueueFamilyProperties* props = (VkQueueFamilyProperties*)RL_CALLOC(QueueFamilyPropertyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &QueueFamilyPropertyCount, props);
    adapter->queueIndices = CLITERAL(QueueIndices){
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED
    };

    for(uint32_t i = 0;i < QueueFamilyPropertyCount;i++){
        if(adapter->queueIndices.graphicsIndex == VK_QUEUE_FAMILY_IGNORED && (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (props[i].queueFlags & VK_QUEUE_COMPUTE_BIT)){
            adapter->queueIndices.graphicsIndex = i;
            adapter->queueIndices.computeIndex = i;
            adapter->queueIndices.transferIndex = i;
            adapter->queueIndices.presentIndex = i;
            break;
        }
    }
    RL_FREE((void*)pds);
    RL_FREE((void*)props);
    userdata->info.callback(WGPURequestAdapterStatus_Success, adapter, CLITERAL(WGPUStringView){NULL, 0}, userdata->info.userdata1, userdata->info.userdata2);
}
WGPUFuture wgpuInstanceRequestAdapter(WGPUInstance instance, const WGPURequestAdapterOptions* options, WGPURequestAdapterCallbackInfo callbackInfo){
    userdataforcreateadapter* info = (userdataforcreateadapter*)RL_CALLOC(1, sizeof(userdataforcreateadapter));
    info->instance = instance;
    info->options = *options;
    info->info = callbackInfo;
    WGPUFutureImpl ret = {
        .userdataForFunction = info,
        .functionCalledOnWaitAny = wgpuCreateAdapter_impl,
        .freeUserData = RL_FREE
    };
    uint64_t id = instance->currentFutureId++; //atomic?
    FutureIDMap_put(&instance->g_futureIDMap, id, ret);
    return (WGPUFuture){ id };
}

static int cmp_uint32_(const void *a, const void *b) {
    uint32_t ua = *(const uint32_t *)a;
    uint32_t ub = *(const uint32_t *)b;
    if (ua < ub) return -1;
    if (ua > ub) return 1;
    return 0;
}

static inline VkSemaphore CreateSemaphoreD(WGPUDevice device){
    VkSemaphore ret = NULL;
    VkSemaphoreCreateInfo sci = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    
    VkResult res = device->functions.vkCreateSemaphore(device->device, &sci, NULL, &ret);
    if(res != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Error creating semaphore");
    }
    return ret;
}

static size_t sort_uniqueuints(uint32_t *arr, size_t count) {
    if (count <= 1) return count;
    qsort(arr, count, sizeof(uint32_t), cmp_uint32_);
    size_t unique_count = 1;
    for (size_t i = 1; i < count; ++i) {
        if (arr[i] != arr[unique_count - 1]) {
            arr[unique_count++] = arr[i];
        }
    }
    return unique_count;
}
#define RG_SWAP(a, b) do { \
    typeof(a) temp = (a); \
    (a) = (b); \
    (b) = temp; \
} while (0)

WGPUDevice wgpuAdapterCreateDevice(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor){
    //std::pair<WGPUDevice, WGPUQueue> ret = {0,0};
    
    for(uint32_t i = 0;i < descriptor->requiredFeatureCount;i++){
        WGPUFeatureName feature = descriptor->requiredFeatures[i];
        switch(feature){
            default:
            (void)0; 
        }
    }
    // Collect unique queue families
    uint32_t queueFamilies[3] = {
        adapter->queueIndices.graphicsIndex,
        adapter->queueIndices.computeIndex,
        adapter->queueIndices.presentIndex
    };
    uint32_t queueFamilyCount = sort_uniqueuints(queueFamilies, 3);
    
    // Create queue create infos
    VkDeviceQueueCreateInfo queueCreateInfos[8] = {0};
    uint32_t queueCreateInfoCount = 0;
    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex = 0;queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
        uint32_t queueFamily = queueFamilies[queueFamilyIndex]; 
        if(queueFamily == VK_QUEUE_FAMILY_IGNORED)continue; // TODO handle this better
        VkDeviceQueueCreateInfo queueCreateInfo zeroinit;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos[queueCreateInfoCount++] = queueCreateInfo;
    }
    
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, NULL, &deviceExtensionCount, NULL);
    VkExtensionProperties* deprops = (VkExtensionProperties*)RL_CALLOC(deviceExtensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, NULL, &deviceExtensionCount, deprops);
    
    const char* deviceExtensionsToLookFor[] = {
        //#ifndef FORCE_HEADLESS
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        //#endif
        #if VULKAN_ENABLE_RAYTRACING == 1
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,      // "VK_KHR_acceleration_structure"
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,        // "VK_KHR_ray_tracing_pipeline"
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,    // "VK_KHR_deferred_host_operations" - required by acceleration structure
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,         // "VK_EXT_descriptor_indexing" - needed for bindless descriptors
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,       // "VK_KHR_buffer_device_address" - needed by AS
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,                   // "VK_KHR_spirv_1_4" - required for ray tracing shaders
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,       // "VK_KHR_shader_float_controls" - required by spirv_1_4
        #endif
    };
    const uint32_t deviceExtensionsToLookForCount = sizeof(deviceExtensionsToLookFor) / sizeof(const char*);
    
    const char* deviceExtensionsFound[deviceExtensionsToLookForCount + 1];
    uint32_t extInsertIndex = 0;
    for(uint32_t i = 0;i < deviceExtensionsToLookForCount;i++){
        for(uint32_t j = 0;j < deviceExtensionCount;j++){
            if(strcmp(deviceExtensionsToLookFor[i], deprops[j].extensionName) == 0){
                deviceExtensionsFound[extInsertIndex++] = deviceExtensionsToLookFor[i];
            }
        }
    }
    // Specify device features

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT props = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT,
        .pNext = NULL,
        .dynamicPrimitiveTopologyUnrestricted = VK_TRUE
    };
    
    VkPhysicalDeviceFeatures2 deviceFeatures zeroinit;
    deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    vkGetPhysicalDeviceFeatures2(adapter->physicalDevice, &deviceFeatures);

    VkDeviceCreateInfo createInfo zeroinit;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    VkPhysicalDeviceVulkan13Features v13features zeroinit;
    v13features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    v13features.dynamicRendering = VK_TRUE;
    #endif

    createInfo.pNext = &v13features;
    //features.pNext = &props;

    createInfo.queueCreateInfoCount = queueCreateInfoCount;
    createInfo.pQueueCreateInfos = queueCreateInfos;

    createInfo.enabledExtensionCount = extInsertIndex;
    createInfo.ppEnabledExtensionNames = deviceExtensionsFound;

    createInfo.pEnabledFeatures = &deviceFeatures.features;
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
    WGPUDevice retDevice = callocnew(WGPUDeviceImpl);
    retDevice->refCount = 1;
    WGPUQueue retQueue = callocnew(WGPUQueueImpl);
    retQueue->refCount = 0;
    VkResult dcresult = vkCreateDevice(adapter->physicalDevice, &createInfo, NULL, &(retDevice->device));
    struct VolkDeviceTable table = {0};

    if (dcresult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create logical device!");
    } else {
        //TRACELOG(LOG_INFO, "Successfully created logical device");
        volkLoadDeviceTable(&retDevice->functions, retDevice->device);
        retDevice->functions.vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(retDevice->device, "vkQueuePresentKHR");
    }
    retDevice->uncapturedErrorCallbackInfo = descriptor->uncapturedErrorCallbackInfo;

    // Retrieve and assign queues
    
    QueueIndices indices = adapter->queueIndices;
    retDevice->functions.vkGetDeviceQueue(retDevice->device, indices.graphicsIndex, 0, &retQueue->graphicsQueue);
    //#ifndef FORCE_HEADLESS
    retDevice->functions.vkGetDeviceQueue(retDevice->device, indices.presentIndex, 0, &retQueue->presentQueue);
    //#endif
    if (indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex) {
        retDevice->functions.vkGetDeviceQueue(retDevice->device, indices.computeIndex, 0, &retQueue->computeQueue);
    } else {
        // If compute Index is same as graphics or present, assign accordingly
        if (indices.computeIndex == indices.graphicsIndex) {
            retQueue->computeQueue = retQueue->graphicsQueue;
        } else if (indices.computeIndex == indices.presentIndex) {
            retQueue->computeQueue = retQueue->presentQueue;
        }
    }
    WGPUCommandEncoderDescriptor cedesc zeroinit;
    cedesc.recyclable = true;
    for(uint32_t i = 0;i < framesInFlight;i++){
        retDevice->frameCaches[i].finalTransitionSemaphore = CreateSemaphoreD(retDevice);
        VkCommandPoolCreateInfo pci zeroinit;
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        retDevice->functions.vkCreateCommandPool(retDevice->device, &pci, NULL, &retDevice->frameCaches[i].commandPool);
        
        VkCommandBufferAllocateInfo cbai zeroinit;
        cbai.commandBufferCount = 1;
        cbai.commandPool = retDevice->frameCaches[i].commandPool;
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        retDevice->functions.vkAllocateCommandBuffers(retDevice->device, &cbai, &retDevice->frameCaches[i].finalTransitionBuffer);
        retDevice->frameCaches[i].finalTransitionFence = wgpuDeviceCreateFence(retDevice);
    }
    retQueue->presubmitCache = wgpuDeviceCreateCommandEncoder(retDevice, &cedesc);

    VmaAllocatorCreateInfo aci zeroinit;
    aci.instance = adapter->instance->instance;
    aci.physicalDevice = adapter->physicalDevice;
    aci.device = retDevice->device;
    #if VULKAN_ENABLE_RAYTRACING == 1
    aci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    #endif

    VkDeviceSize limit = (((uint64_t)1) << 30);
    aci.preferredLargeHeapBlockSize = 1 << 15;
    VkPhysicalDeviceMemoryProperties memoryProperties zeroinit;
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &memoryProperties);

    VkDeviceSize heapsizes[128] = {0};
    
    for(uint32_t i = 0;i < memoryProperties.memoryHeapCount;i++){
        heapsizes[i] = limit;
    }

    aci.pHeapSizeLimit = heapsizes;

    VmaDeviceMemoryCallbacks callbacks = {
        /// Optional, can be null.
        0
        //.pfnAllocate = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
        //    TRACELOG(LOG_WARNING, "Allocating %llu of memory type %u", size, type);
        //},
        //.pfnFree = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
        //    TRACELOG(LOG_WARNING, "Freeing %llu of memory type %u", size, type);
        //}
    };
    aci.pDeviceMemoryCallbacks = &callbacks;
    VmaVulkanFunctions vmaVulkanFunctions
    // = {
    //    .vkAllocateMemory                    = retDevice->functions.vkAllocateMemory,
    //    .vkFreeMemory                        = retDevice->functions.vkFreeMemory,
    //    .vkCreateBuffer                      = retDevice->functions.vkCreateBuffer,
    //    .vkCreateImage                       = retDevice->functions.vkCreateImage,
    //    .vkDestroyBuffer                     = retDevice->functions.vkDestroyBuffer,
    //    .vkDestroyImage                      = retDevice->functions.vkDestroyImage,
    //    .vkGetDeviceBufferMemoryRequirements = retDevice->functions.vkGetDeviceBufferMemoryRequirements,
    //    .vkGetDeviceImageMemoryRequirements  = retDevice->functions.vkGetDeviceImageMemoryRequirements,
    //    .vkBindBufferMemory                  = retDevice->functions.vkBindBufferMemory,
    //    .vkCmdCopyBuffer                     = retDevice->functions.vkCmdCopyBuffer,
    //    .vkGetInstanceProcAddr               = vkGetInstanceProcAddr,
    //    .vkGetDeviceProcAddr                 = vkGetDeviceProcAddr
    //}
     = {0};
    vmaImportVulkanFunctionsFromVolk(&aci, &vmaVulkanFunctions);
    aci.pVulkanFunctions = &vmaVulkanFunctions;
    VkResult allocatorCreateResult = vmaCreateAllocator(&aci, &retDevice->allocator);

    if(allocatorCreateResult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating the allocator: %d", (int)allocatorCreateResult);
    }
    for(uint32_t i = 0;i < framesInFlight;i++){
        VkSemaphoreVector* semvec = &retQueue->syncState[i].semaphores;
        VkSemaphoreVector_reserve(semvec, 100);
        semvec->size = 100;
        for(uint32_t j = 0;j < semvec->size;j++){
            semvec->data[j] = CreateSemaphoreD(retDevice);
        }
        retQueue->syncState[i].acquireImageSemaphore = CreateSemaphoreD(retDevice);

        // TODO what? I don't remember
    }
    VmaPoolCreateInfo vpci zeroinit;
    vpci.minAllocationAlignment = 64;
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &memoryProperties);
    uint32_t hostVisibleCoherentIndex = 0;
    for(;hostVisibleCoherentIndex < memoryProperties.memoryTypeCount;hostVisibleCoherentIndex++){
        if(memoryProperties.memoryTypes[hostVisibleCoherentIndex].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){
            break;
        }
    }
    vpci.memoryTypeIndex = hostVisibleCoherentIndex;
    vpci.blockSize = (1 << 16);
    vmaCreatePool(retDevice->allocator, &vpci, &retDevice->aligned_hostVisiblePool);

    {

        //auto [device, queue] = ret;
        retDevice->queue = retQueue;

        retDevice->adapter = adapter;
        wgpuAdapterAddRef(adapter);
        {
            
            // Get ray tracing pipeline properties
            #if VULKAN_ENABLE_RAYTRACING == 1
            VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties zeroinit;
            rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
            VkPhysicalDeviceProperties2 deviceProperties2 zeroinit;
            deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
            deviceProperties2.pNext = &rayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(adapter->physicalDevice, &deviceProperties2);
            adapter->rayTracingPipelineProperties = rayTracingPipelineProperties;
            #endif
        }
        retDevice->adapter = adapter;
        retQueue->device = retDevice;
    }
    RL_FREE(deprops);
    return retDevice;
}
WGPUQueue wgpuDeviceGetQueue(WGPUDevice device){
    wgpuQueueAddRef(device->queue);
    return device->queue;
}


WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, const WGPUBufferDescriptor* desc){
    //vmaCreateAllocator(const VmaAllocatorCreateInfo * _Nonnull pCreateInfo, VmaAllocator  _Nullable * _Nonnull pAllocator)
    WGPUBuffer wgpuBuffer = callocnew(WGPUBufferImpl);

    uint32_t cacheIndex = device->submittedFrames % framesInFlight;

    wgpuBuffer->device = device;
    wgpuBuffer->cacheIndex = cacheIndex;
    wgpuBuffer->refCount = 1;
    wgpuBuffer->usage = desc->usage;
    
    VkBufferCreateInfo bufferDesc zeroinit;
    bufferDesc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferDesc.size = desc->size;
    bufferDesc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferDesc.usage = toVulkanBufferUsage(desc->usage);
    
    VkMemoryPropertyFlags propertyToFind = 0;
    if(desc->usage & (WGPUBufferUsage_MapRead | WGPUBufferUsage_MapWrite)){
        propertyToFind = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    else{
        //propertyToFind = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        propertyToFind = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    VmaAllocationCreateInfo vallocInfo zeroinit;
    vallocInfo.preferredFlags = propertyToFind;

    VmaAllocation allocation zeroinit;
    VmaAllocationInfo allocationInfo zeroinit;

    VkResult vmabufferCreateResult = vmaCreateBuffer(device->allocator, &bufferDesc, &vallocInfo, &wgpuBuffer->buffer, &allocation, &allocationInfo);

    
    if(vmabufferCreateResult != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Could not allocate buffer: %s", vkErrorString(vmabufferCreateResult));
        //wgpuBuffer->~WGPUBufferImpl();
        RL_FREE(wgpuBuffer);
        return NULL;
    }
    wgpuBuffer->allocation = allocation;
    
    wgpuBuffer->memoryProperties = propertyToFind;

    if(desc->usage & WGPUBufferUsage_ShaderDeviceAddress){
        VkBufferDeviceAddressInfo bdai zeroinit;
        bdai.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        bdai.buffer = wgpuBuffer->buffer;
        wgpuBuffer->address = device->functions.vkGetBufferDeviceAddress(device->device, &bdai);
    }
    return wgpuBuffer;
}

void wgpuBufferMap(WGPUBuffer buffer, WGPUMapMode mapmode, size_t offset, size_t size, void** data){
    if(buffer->latestFence){
        if(buffer->latestFence->state == WGPUFenceState_InUse){
            wgpuFenceWait(buffer->latestFence, ((uint64_t)1) << 40);
        }
        wgpuFenceRelease(buffer->latestFence);
        buffer->latestFence = NULL;    
    }
    vmaMapMemory(buffer->device->allocator, buffer->allocation, data);
}

void wgpuBufferUnmap(WGPUBuffer buffer){
    
    vmaUnmapMemory(buffer->device->allocator, buffer->allocation);
    //VmaAllocationInfo allocationInfo zeroinit;
    //vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &allocationInfo);
    //vkUnmapMemory(buffer->device->device, allocationInfo.deviceMemory);
    //mappedMemories.erase(allocationInfo.deviceMemory);
}

WGPUFuture wgpuBufferMapAsync(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, WGPUBufferMapCallbackInfo callbackInfo){
    return (WGPUFuture){0};
}

size_t wgpuBufferGetSize(WGPUBuffer buffer){
    VmaAllocationInfo info zeroinit;
    vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &info);
    return info.size;
}

void wgpuQueueWriteBuffer(WGPUQueue cSelf, WGPUBuffer buffer, uint64_t bufferOffset, const void* data, size_t size){
    void* mappedMemory = NULL;
    if(buffer->memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
        void* mappedMemory = NULL;
        wgpuBufferMap(buffer, WGPUMapMode_Write, bufferOffset, size, &mappedMemory);
        
        if (mappedMemory != NULL) {
            // Memory is host mappable: copy data and unmap.
            memcpy(mappedMemory, data, size);
            wgpuBufferUnmap(buffer);
            
        }
    }
    else{
        WGPUBufferDescriptor stDesc zeroinit;
        stDesc.size = size;
        stDesc.usage = WGPUBufferUsage_MapWrite;
        WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(cSelf->device, &stDesc);
        wgpuQueueWriteBuffer(cSelf, stagingBuffer, 0, data, size);
        wgpuCommandEncoderCopyBufferToBuffer(cSelf->presubmitCache, stagingBuffer, 0, buffer, bufferOffset, size);
        wgpuBufferRelease(stagingBuffer);
    }
}

void wgpuQueueWriteTexture(WGPUQueue queue, const WGPUTexelCopyTextureInfo* destination, const void* data, size_t dataSize, const WGPUTexelCopyBufferLayout* dataLayout, const WGPUExtent3D* writeSize){

    WGPUBufferDescriptor bdesc zeroinit;
    bdesc.size = dataSize;
    bdesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_MapWrite;
    WGPUBuffer stagingBuffer = wgpuDeviceCreateBuffer(queue->device, &bdesc);
    void* mappedMemory = NULL;
    wgpuBufferMap(stagingBuffer, WGPUMapMode_Write, 0, dataSize, &mappedMemory);
    if(mappedMemory != NULL){
        memcpy(mappedMemory, data, dataSize);
        wgpuBufferUnmap(stagingBuffer);
    }
    WGPUCommandEncoder enkoder = wgpuDeviceCreateCommandEncoder(queue->device, NULL);
    WGPUTexelCopyBufferInfo source;
    source.buffer = stagingBuffer;
    source.layout = *dataLayout;

    wgpuCommandEncoderCopyBufferToTexture(enkoder, &source, destination, writeSize);
    WGPUCommandBuffer puffer = wgpuCommandEncoderFinish(enkoder);

    wgpuQueueSubmit(queue, 1, &puffer);
    wgpuCommandEncoderRelease(enkoder);
    wgpuCommandBufferRelease(puffer);

    wgpuBufferRelease(stagingBuffer);
}


WGPUFence wgpuDeviceCreateFence(WGPUDevice device){
    WGPUFence fence = RL_CALLOC(1, sizeof(WGPUFenceImpl));
    fence->refCount = 1;
    fence->device = device;
    CallbackWithUserdataVector_init(&fence->callbacksOnWaitComplete);
    VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkResult result = device->functions.vkCreateFence(
        device->device, 
        &createInfo,
        NULL,
        &fence->fence
    );
    if(result != VK_SUCCESS){
        RL_FREE(fence);
        return NULL;
    }

    return fence;
}
void wgpuFencesWait(const WGPUFence* fences, uint32_t fenceCount, uint64_t timeoutNS){
    if(fenceCount){
        if(fenceCount <= 128){
            VkFence vkFences[128];
            uint32_t actualFenceCount = 0;
            for(uint32_t i = 0;i < fenceCount;i++){
                if(fences[i]->state == WGPUFenceState_InUse){
                    vkFences[actualFenceCount++] = fences[i]->fence;
                }
            }
            fences[0]->device->functions.vkWaitForFences(fences[0]->device->device, actualFenceCount, vkFences, VK_TRUE, timeoutNS);
        }
        else{
            VkFence* vkFences = (VkFence*)RL_CALLOC(fenceCount, sizeof(VkFence));
            uint32_t actualFenceCount = 0;
            for(uint32_t i = 0;i < fenceCount;i++){
                if(fences[i]->state == WGPUFenceState_InUse){
                    vkFences[actualFenceCount++] = fences[i]->fence;
                }
            }
            fences[0]->device->functions.vkWaitForFences(fences[0]->device->device, actualFenceCount, vkFences, VK_TRUE, timeoutNS);
            RL_FREE((void*)vkFences);
        }
        for(uint32_t i = 0;i < fenceCount;i++){
            if(fences[i]->state == WGPUFenceState_InUse){
                for(uint32_t ci = 0;ci < fences[i]->callbacksOnWaitComplete.size;ci++){
                    fences[i]->callbacksOnWaitComplete.data[ci].callback(fences[i]->callbacksOnWaitComplete.data[ci].userdata);
                }
                fences[i]->state = WGPUFenceState_Finished;
            }
        }
    }
}
RGAPI void wgpuFenceWait(WGPUFence fence, uint64_t timeoutNS){
    VkResult waitResult = fence->device->functions.vkWaitForFences(fence->device->device, 1, &fence->fence, VK_TRUE, timeoutNS);
    if(waitResult == VK_SUCCESS){
        fence->state = WGPUFenceState_Finished;
        for(size_t i = 0;i < fence->callbacksOnWaitComplete.size;i++){
            fence->callbacksOnWaitComplete.data[i].callback(fence->callbacksOnWaitComplete.data[i].userdata);
        }
    }
}
void wgpuFenceAttachCallback(WGPUFence fence, void(*callback)(void*), void* userdata){
    CallbackWithUserdataVector_push_back(&fence->callbacksOnWaitComplete, (CallbackWithUserdata){
        .callback = callback,
        .userdata = userdata
    });
}
void wgpuFenceAddRef(WGPUFence fence){
    ++fence->refCount;
}
void wgpuFenceRelease(WGPUFence fence){
    rassert(fence->refCount > 0, "refCount already zero");
    if(--fence->refCount == 0){
        fence->device->functions.vkDestroyFence(fence->device->device, fence->fence, NULL);
        for(uint32_t i = 0;i < CallbackWithUserdataVector_size(&fence->callbacksOnWaitComplete);i++){
            CallbackWithUserdata* cbu = CallbackWithUserdataVector_get(&fence->callbacksOnWaitComplete, i);
            if(cbu->freeUserData){
                cbu->freeUserData(cbu->userdata);
            }
        }
        CallbackWithUserdataVector_free(&fence->callbacksOnWaitComplete);
        RL_FREE(fence);
    }
}

WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor){
    VkDeviceMemory imageMemory zeroinit;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    WGPUTexture ret = callocnew(WGPUTextureImpl);

    VkImageCreateInfo imageInfo zeroinit;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = (VkExtent3D){ descriptor->size.width, descriptor->size.height, descriptor->size.depthOrArrayLayers };
    imageInfo.mipLevels = descriptor->mipLevelCount;
    imageInfo.arrayLayers = 1;
    imageInfo.format = toVulkanPixelFormat(descriptor->format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = toVulkanTextureUsage(descriptor->usage, descriptor->format);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = toVulkanSampleCount(descriptor->sampleCount);
    
    VkImage image zeroinit;
    if (device->functions.vkCreateImage(device->device, &imageInfo, NULL, &image) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to create image!");
    
    VkMemoryRequirements memReq;
    device->functions.vkGetImageMemoryRequirements(device->device, image, &memReq);
    
    VkMemoryAllocateInfo allocInfo zeroinit;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        device->adapter,
        memReq.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (device->functions.vkAllocateMemory(device->device, &allocInfo, NULL, &imageMemory) != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Failed to allocate image memory!");
    }
    device->functions.vkBindImageMemory(device->device, image, imageMemory, 0);

    ret->image = image;
    ret->memory = imageMemory;
    ret->device = device;
    ret->width =  descriptor->size.width;
    ret->height = descriptor->size.height;
    ret->format = toVulkanPixelFormat(descriptor->format);
    ret->sampleCount = descriptor->sampleCount;
    ret->depthOrArrayLayers = descriptor->size.depthOrArrayLayers;
    ret->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    ret->refCount = 1;
    ret->mipLevels = descriptor->mipLevelCount;
    ret->memory = imageMemory;
    return ret;
}

static inline uint32_t descriptorTypeContiguous(VkDescriptorType type){
    switch(type){
        case VK_DESCRIPTOR_TYPE_SAMPLER: return 0;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return 1;
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return 2;
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return 3;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return 4;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return 5;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return 6;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return 7;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return 8;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return 9;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return 10;
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: return 11;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return 12;
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV: return 13;
        case VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM: return 14;
        case VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM: return 15;
        case VK_DESCRIPTOR_TYPE_MUTABLE_EXT: return 16;
        case VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV: return 17;
        case VK_DESCRIPTOR_TYPE_MAX_ENUM: return 20;
        default: rg_unreachable();
    }
}

static inline VkDescriptorType contiguousDescriptorType(uint32_t cont){
    switch(cont){
        case 0 : return VK_DESCRIPTOR_TYPE_SAMPLER;
        case 1 : return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case 2 : return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case 3 : return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        case 4 : return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
        case 5 : return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
        case 6 : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case 7 : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case 8 : return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case 9 : return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case 10: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        case 11: return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
        case 12: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case 13: return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
        case 14: return VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM;
        case 15: return VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM;
        case 16: return VK_DESCRIPTOR_TYPE_MUTABLE_EXT;
        case 17: return VK_DESCRIPTOR_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_NV;
        case 20: return VK_DESCRIPTOR_TYPE_MAX_ENUM;
        default: rg_unreachable();
    }
}
#define DESCRIPTOR_TYPE_UPPER_LIMIT 32

void wgpuWriteBindGroup(WGPUDevice device, WGPUBindGroup wvBindGroup, const WGPUBindGroupDescriptor* bgdesc){
    
    rassert(bgdesc->layout != NULL, "WGPUBindGroupDescriptor::layout is null");
    
    if(wvBindGroup->pool == NULL){
        wvBindGroup->layout = bgdesc->layout;

        uint32_t counts[DESCRIPTOR_TYPE_UPPER_LIMIT] = {0};

        //std::unordered_map<VkDescriptorType, uint32_t> counts;
        for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
            ++counts[descriptorTypeContiguous(extractVkDescriptorType(bgdesc->layout->entries + i))];
        }

        VkDescriptorPoolSize sizes[DESCRIPTOR_TYPE_UPPER_LIMIT];
        uint32_t VkDescriptorPoolSizeCount = 0;
        //sizes.reserve(counts.size());
        for(uint32_t i = 0;i < DESCRIPTOR_TYPE_UPPER_LIMIT;i++){
            if(counts[i] != 0){
                sizes[VkDescriptorPoolSizeCount++] = (VkDescriptorPoolSize){
                    .type = contiguousDescriptorType(i), 
                    .descriptorCount = counts[i]
                };
            }
        }
        VkDescriptorPoolCreateInfo dpci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .maxSets = 1,
            .poolSizeCount = VkDescriptorPoolSizeCount,
            .pPoolSizes = sizes
        };
        device->functions.vkCreateDescriptorPool(device->device, &dpci, NULL, &wvBindGroup->pool);

        //VkCopyDescriptorSet copy{};
        //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        VkDescriptorSetAllocateInfo dsai = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = wvBindGroup->pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &bgdesc->layout->layout
        };

        device->functions.vkAllocateDescriptorSets(device->device, &dsai, &wvBindGroup->set);
    }
    ResourceUsage newResourceUsage;
    ResourceUsage_init(&newResourceUsage);
    for(uint32_t i = 0;i < bgdesc->entryCount;i++){
        WGPUBindGroupEntry entry = bgdesc->entries[i];
        if(entry.buffer){
            ru_trackBuffer(&newResourceUsage, (WGPUBuffer)entry.buffer, (BufferUsageRecord){
                0,
                0,
                VK_FALSE
            });
        }
        else if(entry.textureView){
            ru_trackTextureView(&newResourceUsage, (WGPUTextureView)entry.textureView);
        }
        else if(entry.sampler){
            ru_trackSampler(&newResourceUsage, entry.sampler);
        }
    }
    releaseAllAndClear(&wvBindGroup->resourceUsage);
    ResourceUsage_move(&wvBindGroup->resourceUsage, &newResourceUsage);

    
    uint32_t count = bgdesc->entryCount;
     
    VkWriteDescriptorSetVector writes zeroinit;
    VkDescriptorBufferInfoVector bufferInfos zeroinit;
    VkDescriptorImageInfoVector imageInfos zeroinit;
    VkWriteDescriptorSetAccelerationStructureKHRVector accelStructInfos zeroinit;


    VkWriteDescriptorSetVector_initWithSize(&writes, count);
    VkDescriptorBufferInfoVector_initWithSize(&bufferInfos, count);
    VkDescriptorImageInfoVector_initWithSize(&imageInfos, count);
    VkWriteDescriptorSetAccelerationStructureKHRVector_initWithSize(&accelStructInfos, count);

    for(uint32_t i = 0;i < count;i++){
        writes.data[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uint32_t binding = bgdesc->entries[i].binding;
        writes.data[i].dstBinding = binding;
        writes.data[i].dstSet = wvBindGroup->set;
        const WGPUBindGroupLayoutEntry* entryi = &bgdesc->layout->entries[i];
        const VkDescriptorType entryType = extractVkDescriptorType(bgdesc->layout->entries + i);
        writes.data[i].descriptorType = entryType;
        writes.data[i].descriptorCount = 1;
        switch(entryType){
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: //[[fallthrough]];
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:{
                WGPUBuffer bufferOfThatEntry = (WGPUBuffer)bgdesc->entries[i].buffer;
                ru_trackBuffer(&wvBindGroup->resourceUsage, bufferOfThatEntry, (BufferUsageRecord){0, 0, VK_FALSE});
                bufferInfos.data[i].buffer = bufferOfThatEntry->buffer;
                bufferInfos.data[i].offset = bgdesc->entries[i].offset;
                bufferInfos.data[i].range  =  bgdesc->entries[i].size;
                writes.data[i].pBufferInfo = bufferInfos.data + i;
            }break;

            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:{
                ru_trackTextureView(&wvBindGroup->resourceUsage, (WGPUTextureView)bgdesc->entries[i].textureView);
                imageInfos.data[i].imageView   = ((WGPUTextureView)bgdesc->entries[i].textureView)->view;
                imageInfos.data[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                writes    .data[i].pImageInfo  = imageInfos.data + i;
            }break;
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:{
                ru_trackTextureView(&wvBindGroup->resourceUsage, (WGPUTextureView)bgdesc->entries[i].textureView);
                imageInfos.data[i].imageView   = ((WGPUTextureView)bgdesc->entries[i].textureView)->view;
                imageInfos.data[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                writes    .data[i].pImageInfo  = imageInfos.data + i;
            }break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:{
                ru_trackSampler(&wvBindGroup->resourceUsage, bgdesc->entries[i].sampler);
                imageInfos.data[i].sampler    = bgdesc->entries[i].sampler->sampler;
                writes.    data[i].pImageInfo = imageInfos.data + i;
            }break;
            default:
            rg_unreachable();
        }
    }

    vkUpdateDescriptorSets(device->device, writes.size, writes.data, 0, NULL);

    VkWriteDescriptorSetVector_free(&writes);
    VkDescriptorBufferInfoVector_free(&bufferInfos);
    VkDescriptorImageInfoVector_free(&imageInfos);
    VkWriteDescriptorSetAccelerationStructureKHRVector_free(&accelStructInfos);
}



WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, const WGPUBindGroupDescriptor* bgdesc){
    rassert(bgdesc->layout != NULL, "WGPUBindGroupDescriptor::layout is null");
    
    WGPUBindGroup ret = callocnew(WGPUBindGroupImpl);
    ret->refCount = 1;
    ResourceUsage_init(&ret->resourceUsage);

    ret->device = device;
    ret->cacheIndex = device->submittedFrames % framesInFlight;

    PerframeCache* fcache = &device->frameCaches[ret->cacheIndex];
    DescriptorSetAndPoolVector* dsap = BindGroupCacheMap_get(&fcache->bindGroupCache, bgdesc->layout);

    if(dsap == NULL || dsap->size == 0){ //Cache miss
        TRACELOG(LOG_INFO, "Allocating new VkDescriptorPool and -Set");
        VkDescriptorPoolCreateInfo dpci zeroinit;
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.maxSets = 1;

        uint32_t counts[DESCRIPTOR_TYPE_UPPER_LIMIT] = {0};

        for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
            ++counts[extractVkDescriptorType(bgdesc->layout->entries + i)];
        }
        VkDescriptorPoolSize sizes[DESCRIPTOR_TYPE_UPPER_LIMIT];
        uint32_t VkDescriptorPoolSizeCount = 0;
        for(uint32_t i = 0;i < DESCRIPTOR_TYPE_UPPER_LIMIT;i++){
            if(counts[i] != 0){
                sizes[VkDescriptorPoolSizeCount++] = (VkDescriptorPoolSize){
                    .type = contiguousDescriptorType(i), 
                    .descriptorCount = counts[i]
                };
            }
        }

        dpci.poolSizeCount = VkDescriptorPoolSizeCount;
        dpci.pPoolSizes = sizes;
        dpci.maxSets = 1;
        device->functions.vkCreateDescriptorPool(device->device, &dpci, NULL, &ret->pool);

        //VkCopyDescriptorSet copy{};
        //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        VkDescriptorSetAllocateInfo dsai zeroinit;
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.descriptorPool = ret->pool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = (VkDescriptorSetLayout*)&bgdesc->layout->layout;
        device->functions.vkAllocateDescriptorSets(device->device, &dsai, &ret->set);
    }
    else{
        ret->pool = dsap->data[dsap->size - 1].pool;
        ret->set  = dsap->data[dsap->size - 1].set;
        --dsap->size;
    }
    ret->entryCount = bgdesc->entryCount;

    ret->entries = RL_CALLOC(bgdesc->entryCount, sizeof(WGPUBindGroupEntry));
    if(bgdesc->entryCount > 0){
        memcpy(ret->entries, bgdesc->entries, bgdesc->entryCount * sizeof(WGPUBindGroupEntry));
    }
    wgpuWriteBindGroup(device, ret, bgdesc);
    ret->layout = bgdesc->layout;
    ++ret->layout->refCount;
    rassert(ret->layout != NULL, "ret->layout is NULL");
    return ret;
}



WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice device, const WGPUBindGroupLayoutDescriptor* bgldesc){
    WGPUBindGroupLayout ret = callocnew(WGPUBindGroupLayoutImpl);
    ret->refCount = 1;
    ret->device = device;
    ret->entryCount = bgldesc->entryCount;
    VkDescriptorSetLayoutCreateInfo slci zeroinit;
    
    slci.bindingCount = bgldesc->entryCount;
    const WGPUBindGroupLayoutEntry* entries = bgldesc->entries;
    const uint32_t entryCount = bgldesc->entryCount;

    VkDescriptorSetLayoutBindingVector bindings;
    VkDescriptorSetLayoutBindingVector_initWithSize(&bindings, slci.bindingCount);

    for(uint32_t i = 0;i < slci.bindingCount;i++){
        bindings.data[i].descriptorCount = 1;
        bindings.data[i].binding = entries[i].binding;
        VkDescriptorType vkdtype = extractVkDescriptorType(entries + i);
        bindings.data[i].descriptorType = vkdtype;

        if(entries[i].visibility == 0){
            TRACELOG(LOG_WARNING, "Empty visibility detected, falling back to Vertex | Fragment | Compute mask");
            bindings.data[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        }

        else{
            bindings.data[i].stageFlags = toVulkanShaderStageBits(entries[i].visibility);
        }
    }

    slci.pBindings = bindings.data;
    slci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    device->functions.vkCreateDescriptorSetLayout(device->device, &slci, NULL, &ret->layout);
    WGPUBindGroupLayoutEntry* entriesCopy = (WGPUBindGroupLayoutEntry*)RL_CALLOC(entryCount, sizeof(WGPUBindGroupLayoutEntry));

    if(entryCount > 0){
        memcpy(entriesCopy, entries, entryCount * sizeof(WGPUBindGroupLayoutEntry));
    }
    ret->entries = entriesCopy;

    VkDescriptorSetLayoutBindingVector_free(&bindings);
    
    return ret;
}
void wgpuPipelineLayoutRelease(WGPUPipelineLayout pllayout){
    if(!--pllayout->refCount){
        for(uint32_t i = 0;i < pllayout->bindGroupLayoutCount;i++){
            wgpuBindGroupLayoutRelease(pllayout->bindGroupLayouts[i]);
        }
        pllayout->device->functions.vkDestroyPipelineLayout(pllayout->device->device, pllayout->layout, NULL);
        RL_FREE((void*)pllayout->bindGroupLayouts);
        RL_FREE(pllayout);
    }
}
WGPUShaderModule wgpuDeviceCreateShaderModule(WGPUDevice device, const WGPUShaderModuleDescriptor* descriptor){
    WGPUShaderModule ret = RL_CALLOC(1, sizeof(WGPUShaderModuleImpl));
    ret->refCount = 1;
    ret->device = device;
    switch(descriptor->nextInChain->sType){
        case WGPUSType_ShaderSourceSPIRV:{
            const WGPUShaderSourceSPIRV* source = (WGPUShaderSourceSPIRV*)descriptor->nextInChain;
            VkShaderModuleCreateInfo sCreateInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                NULL,
                0,
                source->codeSize,
                source->code
            };
            device->functions.vkCreateShaderModule(device->device, &sCreateInfo, NULL, &ret->vulkanModule);
            ret->source = RL_CALLOC(1, sizeof(WGPUShaderSourceSPIRV));
            WGPUShaderSourceSPIRV* copySource = (WGPUShaderSourceSPIRV*)ret->source;
            copySource->chain.sType = WGPUSType_ShaderSourceSPIRV;
            copySource->code = RL_CALLOC(source->codeSize, 1);
            copySource->codeSize = source->codeSize;

            memcpy((void*)copySource->code, source->code, source->codeSize);
            ret->source = (WGPUChainedStruct*)copySource;
            return ret;
        }
        case WGPUSType_ShaderSourceWGSL: {
            const WGPUShaderSourceWGSL* source = (WGPUShaderSourceWGSL*)descriptor->nextInChain;
            size_t length = (source->code.length == WGPU_STRLEN) ? strlen(source->code.data) : source->code.length;
            
            tc_SpirvBlob blob = wgslToSpirv(source);
            VkShaderModuleCreateInfo sCreateInfo = {
                VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                NULL,
                0,
                blob.codeSize,
                blob.code
            };

            device->functions.vkCreateShaderModule(device->device, &sCreateInfo, NULL, &ret->vulkanModule);
            WGPUShaderSourceWGSL* depot = RL_CALLOC(1, sizeof(WGPUShaderSourceWGSL));
            depot->chain.sType = WGPUSType_ShaderSourceWGSL;
            depot->code = CLITERAL(WGPUStringView){
                RL_CALLOC(length, 1),
                length
            };

            memcpy((void*)depot->code.data, source->code.data, length);
            RL_FREE(blob.code);
            ret->source = (WGPUChainedStruct*)depot;
            return ret;
        }
        default: {
            RL_FREE(ret);
            rg_trap();
        }
    }

}


WGPUPipelineLayout wgpuDeviceCreatePipelineLayout(WGPUDevice device, const WGPUPipelineLayoutDescriptor* pldesc){
    WGPUPipelineLayout ret = callocnew(WGPUPipelineLayoutImpl);
    ret->refCount = 1;
    rassert(ret->bindGroupLayoutCount <= 8, "Only supports up to 8 BindGroupLayouts");
    ret->device = device;
    ret->bindGroupLayoutCount = pldesc->bindGroupLayoutCount;
    ret->bindGroupLayouts = (WGPUBindGroupLayout*)RL_CALLOC(pldesc->bindGroupLayoutCount, sizeof(void*));
    memcpy((void*)ret->bindGroupLayouts, (void*)pldesc->bindGroupLayouts, pldesc->bindGroupLayoutCount * sizeof(void*));
    VkDescriptorSetLayout dslayouts[8] zeroinit;
    for(uint32_t i = 0;i < ret->bindGroupLayoutCount;i++){
        wgpuBindGroupLayoutAddRef(ret->bindGroupLayouts[i]);
        dslayouts[i] = ret->bindGroupLayouts[i]->layout;
    }
    VkPipelineLayoutCreateInfo lci zeroinit;
    lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lci.pSetLayouts = dslayouts;
    lci.setLayoutCount = ret->bindGroupLayoutCount;
    VkResult res = vkCreatePipelineLayout(device->device, &lci, NULL, &ret->layout);
    if(res != VK_SUCCESS){
        wgpuPipelineLayoutRelease(ret);
        ret = NULL;
    }
    return ret;
}


WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, const WGPUCommandEncoderDescriptor* desc){
    WGPUCommandEncoder ret = callocnew(WGPUCommandEncoderImpl);
    //TRACELOG(LOG_INFO, "Creating new commandencoder");
    //VkCommandPoolCreateInfo pci{};
    //pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //pci.flags = desc->recyclable ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //ret->recyclable = desc->recyclable;
    ret->cacheIndex = device->submittedFrames % framesInFlight;
    PerframeCache* cache = &device->frameCaches[ret->cacheIndex];
    ret->device = device;
    ret->movedFrom = 0;
    //vkCreateCommandPool(device->device, &pci, NULL, &cache.commandPool);
    if(VkCommandBufferVector_empty(&device->frameCaches[ret->cacheIndex].commandBuffers)){
        VkCommandBufferAllocateInfo bai zeroinit;
        bai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bai.commandPool = cache->commandPool;
        bai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        bai.commandBufferCount = 1;
        device->functions.vkAllocateCommandBuffers(device->device, &bai, &ret->buffer);
        TRACELOG(LOG_INFO, "Allocating new command buffer");
    }
    else{
        //TRACELOG(LOG_INFO, "Reusing");
        ret->buffer = device->frameCaches[ret->cacheIndex].commandBuffers.data[device->frameCaches[ret->cacheIndex].commandBuffers.size - 1];
        VkCommandBufferVector_pop_back(&device->frameCaches[ret->cacheIndex].commandBuffers);
        //vkResetCommandBuffer(ret->buffer, 0);
    }

    const VkCommandBufferBeginInfo bbi = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = NULL,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = NULL
    };
    
    device->functions.vkBeginCommandBuffer(ret->buffer, &bbi);
    
    return ret;
}

WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, const WGPUTextureViewDescriptor *descriptor){
    
    VkImageViewCreateInfo ivci zeroinit;
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = texture->image;

    VkComponentMapping cm = {
        .r = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .g = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .b = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .a = VK_COMPONENT_SWIZZLE_IDENTITY
    };
    ivci.components = cm;
    ivci.viewType = toVulkanTextureViewDimension(descriptor->dimension);
    ivci.format = toVulkanPixelFormat(descriptor->format);
    
    VkImageSubresourceRange sr = {
        .aspectMask = toVulkanAspectMask(descriptor->aspect),
        .baseMipLevel = descriptor->baseMipLevel,
        .levelCount = descriptor->mipLevelCount,
        .baseArrayLayer = descriptor->baseArrayLayer,
        .layerCount = descriptor->arrayLayerCount
    };
    if(!is__depthVk(ivci.format)){
        sr.aspectMask &= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    ivci.subresourceRange = sr;
    WGPUTextureView ret = callocnew(WGPUTextureViewImpl);
    ret->refCount = 1;
    texture->device->functions.vkCreateImageView(texture->device->device, &ivci, NULL, &ret->view);
    ret->format = ivci.format;
    ret->texture = texture;
    ++texture->refCount;
    ret->width = texture->width;
    ret->height = texture->height;
    ret->sampleCount = texture->sampleCount;
    ret->depthOrArrayLayers = texture->depthOrArrayLayers;
    ret->subresourceRange = sr;
    return ret;
}
static inline VkClearValue toVkCV(const WGPUColor c){
    VkClearValue cv zeroinit;
    cv.color.float32[0] = (float)c.r;
    cv.color.float32[1] = (float)c.g;
    cv.color.float32[2] = (float)c.b;
    cv.color.float32[3] = (float)c.a;
    return cv;
};
WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder enc, const WGPURenderPassDescriptor* rpdesc){
    WGPURenderPassEncoder ret = calloc(1, sizeof(WGPURenderPassEncoderImpl));
    VkCommandPool pool = enc->device->frameCaches[enc->cacheIndex].commandPool;


    ret->refCount = 2; //One for WGPURenderPassEncoder the other for the command buffer
    
    WGPURenderPassEncoderSet_add(&enc->referencedRPs, ret);
    //enc->referencedRPs.insert(ret);
    ret->device = enc->device;
    
    ret->cmdEncoder = enc;
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    VkRenderingInfo info zeroinit;
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.colorAttachmentCount = rpdesc->colorAttachmentCount;

    VkRenderingAttachmentInfo colorAttachments[max_color_attachments] zeroinit;
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachments[i].clearValue.color.float32[0] = (float)rpdesc->colorAttachments[i].clearValue.r;
        colorAttachments[i].clearValue.color.float32[1] = (float)rpdesc->colorAttachments[i].clearValue.g;
        colorAttachments[i].clearValue.color.float32[2] = (float)rpdesc->colorAttachments[i].clearValue.b;
        colorAttachments[i].clearValue.color.float32[3] = (float)rpdesc->colorAttachments[i].clearValue.a;
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].imageView = rpdesc->colorAttachments[i].view->view;
        if(rpdesc->colorAttachments[i].resolveTarget){
            colorAttachments[i].resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachments[i].resolveImageView = rpdesc->colorAttachments[i].resolveTarget->view;
            colorAttachments[i].resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            trackTextureView(&ret->resourceUsage, rpdesc->colorAttachments[i].resolveTarget, TextureUsage_RenderAttachment);
        }
        trackTextureView(&ret->resourceUsage, rpdesc->colorAttachments[i].view, TextureUsage_RenderAttachment);
        colorAttachments[i].loadOp = toVulkanLoadOperation(rpdesc->colorAttachments[i].loadOp);
        colorAttachments[i].storeOp = toVulkanStoreOperation(rpdesc->colorAttachments[i].storeOp);
    }
    info.pColorAttachments = colorAttachments;
    VkRenderingAttachmentInfo depthAttachment zeroinit;
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.clearValue.depthStencil.depth = rpdesc->depthStencilAttachment->depthClearValue;

    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.imageView = rpdesc->depthStencilAttachment->view->view;
    trackTextureView(&ret->resourceUsage, rpdesc->depthStencilAttachment->view, TextureUsage_RenderAttachment);
    depthAttachment.loadOp = toVulkanLoadOperation(rpdesc->depthStencilAttachment->depthLoadOp);
    depthAttachment.storeOp = toVulkanStoreOperation(rpdesc->depthStencilAttachment->depthStoreOp);
    info.pDepthAttachment = &depthAttachment;
    info.layerCount = 1;
    info.renderArea = CLITERAL(VkRect2D){
        .offset = CLITERAL(VkOffset2D){0, 0},
        .extent = CLITERAL(VkExtent2D){rpdesc->colorAttachments[0].view->width, rpdesc->colorAttachments[0].view->height}
    };
    vkCmdBeginRendering(ret->cmdBuffer, &info);
    #else
    RenderPassLayout rplayout = GetRenderPassLayout(rpdesc);
    VkRenderPassBeginInfo rpbi zeroinit;
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    LayoutedRenderPass frp = LoadRenderPassFromLayout(enc->device, rplayout);
    ret->renderPass = frp.renderPass;

    VkImageView attachmentViews[2 * max_color_attachments + 2] = {0};// = (VkImageView* )RL_CALLOC(frp.allAttachments.size, sizeof(VkImageView) );
    VkClearValue clearValues   [2 * max_color_attachments + 2] = {0};// = (VkClearValue*)RL_CALLOC(frp.allAttachments.size, sizeof(VkClearValue));
    
    for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
        attachmentViews[i] = rpdesc->colorAttachments[i].view->view;
        clearValues[i] = toVkCV(rpdesc->colorAttachments[i].clearValue);
    }
    uint32_t insertIndex = rplayout.colorAttachmentCount;
    if(rpdesc->depthStencilAttachment){
        rassert(rplayout.depthAttachmentPresent, "renderpasslayout.depthAttachmentPresent != rpdesc->depthAttachment");
        clearValues[insertIndex].depthStencil.depth = rpdesc->depthStencilAttachment->depthClearValue;
        clearValues[insertIndex].depthStencil.stencil = rpdesc->depthStencilAttachment->stencilClearValue;
        attachmentViews[insertIndex++] = rpdesc->depthStencilAttachment->view->view;
    }
    
    if(rpdesc->colorAttachments[0].resolveTarget){
        for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
            rassert(rpdesc->colorAttachments[i].resolveTarget, "All must have resolve or none");
            clearValues[insertIndex] = toVkCV(rpdesc->colorAttachments[i].clearValue);
            attachmentViews[insertIndex++] = rpdesc->colorAttachments[i].resolveTarget->view;
        }
    }

    VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        NULL,
        0,
        ret->renderPass,
        frp.allAttachments.size,
        attachmentViews,
        rpdesc->colorAttachments[0].view->width,
        rpdesc->colorAttachments[0].view->height,
        1
    };
    fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbci.pAttachments = attachmentViews;
    fbci.attachmentCount = frp.allAttachments.size;
    
    const ImageUsageSnap iur_color = {
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .subresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    const ImageUsageSnap iur_resolve = iur_color;
    
    const ImageUsageSnap iur_depth = {
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .subresource = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        rassert(rpdesc->colorAttachments[i].view, "colorAttachments[%d].view is null", (int)i);
        ce_trackTextureView(enc, rpdesc->colorAttachments[i].view, iur_color);
    }

    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        if(rpdesc->colorAttachments[i].resolveTarget){
            ce_trackTextureView(enc, rpdesc->colorAttachments[i].view, iur_resolve);
        }
    }

    if(rpdesc->depthStencilAttachment){
        rassert(rpdesc->depthStencilAttachment->view, "depthStencilAttachment.view is null");
        ce_trackTextureView(enc, rpdesc->depthStencilAttachment->view, iur_depth);
    }

    fbci.width = rpdesc->colorAttachments[0].view->width;
    fbci.height = rpdesc->colorAttachments[0].view->height;
    fbci.layers = 1;
    
    fbci.renderPass = ret->renderPass;
    //VkResult fbresult = vkCreateFramebuffer(enc->device->device, &fbci, NULL, &ret->frameBuffer);
    //if(fbresult != VK_SUCCESS){
    //    TRACELOG(LOG_FATAL, "Error creating framebuffer: %d", (int)fbresult);
    //}
    
    rpbi.renderPass = ret->renderPass;
    rpbi.renderArea = CLITERAL(VkRect2D){
        .offset = CLITERAL(VkOffset2D){0, 0},
        .extent = CLITERAL(VkExtent2D){rpdesc->colorAttachments[0].view->width, rpdesc->colorAttachments[0].view->height}
    };

    rpbi.framebuffer = ret->frameBuffer;
    
    
    rpbi.clearValueCount = frp.allAttachments.size;
    rpbi.pClearValues = clearValues;
    
    ret->cmdEncoder = enc;
    ret->beginInfo = CLITERAL(RenderPassCommandBegin){
        .colorAttachmentCount = rpdesc->colorAttachmentCount,
        .depthAttachmentPresent = rpdesc->depthStencilAttachment != NULL
    };
    rassert(rpdesc->colorAttachmentCount <= max_color_attachments, "Too many colorattachments. supported=%d, provided=%d", ((int)MAX_COLOR_ATTACHMENTS), (int)rpdesc->colorAttachmentCount);
    memcpy(ret->beginInfo.colorAttachments, rpdesc->colorAttachments,rpdesc->colorAttachmentCount * sizeof(WGPURenderPassColorAttachment));
    if(rpdesc->depthStencilAttachment){
        ret->beginInfo.depthStencilAttachment = *rpdesc->depthStencilAttachment;
    }
    RenderPassCommandGenericVector_init(&ret->bufferedCommands);
    //vkCmdBeginRenderPass(ret->secondaryCmdBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    #endif
    return ret;
}

const static VkAccessFlags access_to_vk[8] = {
    [readonly] = VK_ACCESS_SHADER_READ_BIT,
    [writeonly] = VK_ACCESS_SHADER_WRITE_BIT,
    [readwrite] = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
};

void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder renderPassEncoder){
    
    WGPUDevice device = renderPassEncoder->device;
    VkCommandBuffer destination = renderPassEncoder->cmdEncoder->buffer;
    const size_t bufferSize = RenderPassCommandGenericVector_size(&renderPassEncoder->bufferedCommands);

    const RenderPassCommandBegin* beginInfo = &renderPassEncoder->beginInfo;
    RenderPassLayout rplayout = GetRenderPassLayout2(beginInfo);

    


    LayoutedRenderPass frp = LoadRenderPassFromLayout(renderPassEncoder->device, rplayout);
    VkRenderPass vkrenderPass = frp.renderPass;

    VkImageView attachmentViews[2 * max_color_attachments + 2] = {0};// = (VkImageView* )RL_CALLOC(frp.allAttachments.size, sizeof(VkImageView) );
    VkClearValue clearValues   [2 * max_color_attachments + 2] = {0};// = (VkClearValue*)RL_CALLOC(frp.allAttachments.size, sizeof(VkClearValue));
    
    for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
        attachmentViews[i] =        renderPassEncoder->beginInfo.colorAttachments[i].view->view;
        clearValues[i]     = toVkCV(renderPassEncoder->beginInfo.colorAttachments[i].clearValue);
    }
    uint32_t insertIndex = rplayout.colorAttachmentCount;
    
    if(beginInfo->depthAttachmentPresent){
        clearValues[insertIndex].depthStencil.depth   = beginInfo->depthStencilAttachment.depthClearValue;
        clearValues[insertIndex].depthStencil.stencil = beginInfo->depthStencilAttachment.stencilClearValue;
        attachmentViews[insertIndex++]                = beginInfo->depthStencilAttachment.view->view;
    }
    
    if(beginInfo->colorAttachments[0].resolveTarget){
        for(uint32_t i = 0;i < rplayout.colorAttachmentCount;i++){
            rassert(beginInfo->colorAttachments[i].resolveTarget, "All must have resolve or none");
            clearValues[insertIndex] = toVkCV(beginInfo->colorAttachments[i].clearValue);
            attachmentViews[insertIndex++] =  beginInfo->colorAttachments[i].resolveTarget->view;
        }
    }

    const VkFramebufferCreateInfo fbci = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        NULL,
        0,
        vkrenderPass,
        frp.allAttachments.size,
        attachmentViews,
        beginInfo->colorAttachments[0].view->width,
        beginInfo->colorAttachments[0].view->height,
        1
    };

    device->functions.vkCreateFramebuffer(renderPassEncoder->device->device, &fbci, NULL, &renderPassEncoder->frameBuffer);
    
    VkRect2D renderPassRect = {
        .offset = {0, 0},
        .extent = {
            beginInfo->colorAttachments[0].view->width, 
            beginInfo->colorAttachments[0].view->height
        }
    };

    const VkRenderPassBeginInfo rpbi = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = NULL,
        .renderPass = frp.renderPass,
        .framebuffer = renderPassEncoder->frameBuffer,
        .renderArea = renderPassRect,
        .clearValueCount = frp.allAttachments.size,
        .pClearValues = clearValues,
    };

    
    const uint32_t vpWidth = beginInfo->colorAttachments[0].view->width;
    const uint32_t vpHeight = beginInfo->colorAttachments[0].view->height;
    
    const VkViewport viewport = {
        .x        =  ((float)0),
        .y        =  ((float)vpHeight),
        .width    =  ((float)vpWidth),
        .height   = -((float)vpHeight),
        .minDepth =  ((float)0),
        .maxDepth =  ((float)1),
    };

    const VkRect2D scissor = {
        .offset = {
            .x = 0,
            .y = 0,
        },
        .extent = {
            .width = vpWidth,
            .height = vpHeight,
        }
    };
    
    struct stageless_snap{
        VkImageLayout layout;
        VkAccessFlags access;
    } snaps[uniform_type_enumcount] = {
        [texture2d] = {
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .access = VK_ACCESS_SHADER_READ_BIT
        },
        [texture3d] = {
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .access = VK_ACCESS_SHADER_READ_BIT
        },
        [texture2d_array] = {
            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .access = VK_ACCESS_SHADER_READ_BIT
        },
        [storage_texture2d] = {
            .layout = VK_IMAGE_LAYOUT_GENERAL,
            .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
        },
        [storage_texture3d] = {
            .layout = VK_IMAGE_LAYOUT_GENERAL,
            .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
        },
        [storage_texture2d_array] = {
            .layout = VK_IMAGE_LAYOUT_GENERAL,
            .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT
        },
    };

    for(size_t i = 0;i < renderPassEncoder->bufferedCommands.size;i++){
        const RenderPassCommandGeneric* cmd = &renderPassEncoder->bufferedCommands.data[i];
        if(cmd->type == rp_command_type_set_bind_group){
            const RenderPassCommandSetBindGroup* cmdSetBindGroup = &cmd->setBindGroup;
            const WGPUBindGroup       group  = cmdSetBindGroup->group;
            const WGPUBindGroupLayout layout = group->layout;
            for(uint32_t bindingIndex = 0;bindingIndex < layout->entryCount;bindingIndex++){

                rassert(group->entries[bindingIndex].binding == layout->entries[bindingIndex].binding, "Mismatch between layout and group, this will cause bugs.");
                
                const WGPUBindGroupEntry*       groupEntry  = &group ->entries[bindingIndex];
                const WGPUBindGroupLayoutEntry* layoutEntry = &layout->entries[bindingIndex];

                //uniform_type eType = layout->entries[bindingIndex].type;
                if(layout->entries[bindingIndex].buffer.type != WGPUBufferBindingType_BindingNotUsed){
                    rassert(group->entries[bindingIndex].buffer, "Layout indicates buffer but no buffer passed");
                    WGPUShaderStage visibility = layout->entries[bindingIndex].visibility;
                    rassert(visibility, "Empty visibility goddamnit");
                    ce_trackBuffer(
                        renderPassEncoder->cmdEncoder,
                        group->entries[bindingIndex].buffer,
                        (BufferUsageSnap){
                            .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                            //.access = access_to_vk[layout->entries[bindingIndex].access], //TODO
                            .stage = toVulkanPipelineStageBits(visibility)
                        }
                    );
                }

                else if(layout->entries[bindingIndex].texture.sampleType != WGPUTextureSampleType_BindingNotUsed){
                    WGPUShaderStage visibility = layout->entries[bindingIndex].visibility;
                    rassert(visibility, "Empty visibility goddamnit");
                    if(visibility == 0){ //TODO: Get rid of this hack
                        visibility = (WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute);
                    }
                    ce_trackTextureView(
                        renderPassEncoder->cmdEncoder,
                        group->entries[bindingIndex].textureView,
                        (ImageUsageSnap){
                            .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                            .access = VK_ACCESS_SHADER_READ_BIT,
                            .stage = toVulkanPipelineStageBits(visibility)
                        }
                    );
                }
                else if(layout->entries[bindingIndex].storageTexture.access != WGPUStorageTextureAccess_BindingNotUsed){
                    WGPUShaderStage visibility = layout->entries[bindingIndex].visibility;
                    rassert(visibility, "Empty visibility goddamnit");
                    if(visibility == 0){ //TODO: Get rid of this hack
                        visibility = (WGPUShaderStage_Vertex | WGPUShaderStage_Fragment | WGPUShaderStage_Compute);
                    }
                    ce_trackTextureView(
                        renderPassEncoder->cmdEncoder,
                        group->entries[bindingIndex].textureView,
                        (ImageUsageSnap){
                            .layout = VK_IMAGE_LAYOUT_GENERAL,
                            .access = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                            .stage = toVulkanPipelineStageBits(visibility)
                        }
                    );
                }
            }
        }
    }


    device->functions.vkCmdBeginRenderPass(destination, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    for(uint32_t i = 0;i < beginInfo->colorAttachmentCount;i++){
        device->functions.vkCmdSetViewport(destination, i, 1, &viewport);
        device->functions.vkCmdSetScissor (destination, i, 1, &scissor);
    }

    recordVkCommands(destination, renderPassEncoder->device, &renderPassEncoder->bufferedCommands, beginInfo);
    

    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    #error VULKAN_USE_DYNAMIC_RENDERING is not implemented
    //vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions)
    //vkCmdEndRendering(renderPassEncoder->secondaryCmdBuffer);
    #else
    device->functions.vkCmdEndRenderPass(destination);
    #endif
}
/**
 * @brief Ends a CommandEncoder into a CommandBuffer
 * @details This is a one-way transition for WebGPU, therefore we can move resource tracking
 * In Vulkan, this transition is merely a call to vkEndCommandBuffer.
 * 
 * The rest of this function just moves data from the Encoder struct into the buffer. 
 * 
 */
WGPUCommandBuffer wgpuCommandEncoderFinish(WGPUCommandEncoder commandEncoder){
    
    WGPUCommandBuffer ret = callocnew(WGPUCommandBufferImpl);
    ret->refCount = 1;
    rassert(commandEncoder->movedFrom == 0, "Command encoder is already invalidated");
    commandEncoder->movedFrom = 1;
    commandEncoder->device->functions.vkEndCommandBuffer(commandEncoder->buffer);

    WGPURenderPassEncoderSet_move(&ret->referencedRPs, &commandEncoder->referencedRPs);
    WGPUComputePassEncoderSet_move(&ret->referencedCPs, &commandEncoder->referencedCPs);
    WGPURaytracingPassEncoderSet_move(&ret->referencedRTs, &commandEncoder->referencedRTs);
    ResourceUsage_move(&ret->resourceUsage, &commandEncoder->resourceUsage);
    ret->cacheIndex = commandEncoder->cacheIndex;
    ret->buffer = commandEncoder->buffer;
    ret->device = commandEncoder->device;
    commandEncoder->buffer = NULL;
    return ret;
}

void recordVkCommand(CommandBufferAndSomeState* destination_, const RenderPassCommandGeneric* command, const RenderPassCommandBegin *beginInfo){
    VkCommandBuffer destination = destination_->buffer;
    WGPUDevice device = destination_->device;
    VkDrawIndexedIndirectCommand a;
    switch(command->type){
        case rp_command_type_draw_indexed_indirect:{
            const RenderPassCommandDrawIndexedIndirect* drawIndexedIndirect = &command->drawIndexedIndirect;
            device->functions.vkCmdDrawIndexedIndirect(
                destination,
                drawIndexedIndirect->indirectBuffer->buffer,
                drawIndexedIndirect->indirectOffset,
                1,
                20 // sizeof(VkDrawIndexedIndirectCommand) but irrelefant
            );
        }
        break;

        case rp_command_type_draw_indirect:{
            
            const RenderPassCommandDrawIndirect* drawIndirect = &command->drawIndirect;
            device->functions.vkCmdDrawIndirect(
                destination,
                drawIndirect->indirectBuffer->buffer,
                drawIndirect->indirectOffset,
                1,
                16 // sizeof(VkDrawIndirectCommand) but irrelefant
            );
        }
        break;
        case rp_command_type_set_blend_constant:{
            const RenderPassCommandSetBlendConstant* setBlendConstant = &command->setBlendConstant;
            const float buffer[4] = {
                (float)setBlendConstant->color.r,
                (float)setBlendConstant->color.g,
                (float)setBlendConstant->color.b,
                (float)setBlendConstant->color.a,
            };
            device->functions.vkCmdSetBlendConstants(
                destination,
                buffer
            );
        }
        break;
        case rp_command_type_set_viewport:{
            const RenderPassCommandSetViewport* vp = &command->setViewport;
            const VkViewport viewport[8] = {
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
                {vp->x, vp->y, vp->width, vp->height, vp->minDepth, vp->maxDepth},
            };
            device->functions.vkCmdSetViewport(destination, 0, beginInfo->colorAttachmentCount, viewport);
        }
        break;

        case rp_command_type_draw: {
            const RenderPassCommandDraw* draw = &command->draw;
            device->functions.vkCmdDraw(
                destination, 
                draw->vertexCount,
                draw->instanceCount, 
                draw->firstVertex, 
                draw->firstInstance
            );
        }
        break;
        case rp_command_type_draw_indexed: {
            const RenderPassCommandDrawIndexed* drawIndexed = &command->drawIndexed;
            device->functions.vkCmdDrawIndexed(
                destination,
                drawIndexed->indexCount,
                drawIndexed->instanceCount,
                drawIndexed->firstIndex,
                drawIndexed->baseVertex, 
                drawIndexed->firstInstance
            );
        }
        break;
        case rp_command_type_set_vertex_buffer: {
            const RenderPassCommandSetVertexBuffer* setVertexBuffer = &command->setVertexBuffer;
            device->functions.vkCmdBindVertexBuffers(
                destination,
                setVertexBuffer->slot,
                1,
                &setVertexBuffer->buffer->buffer,
                &setVertexBuffer->offset
            );
        }
        break;
        case rp_command_type_set_index_buffer: {
            const RenderPassCommandSetIndexBuffer* setIndexBuffer = &command->setIndexBuffer;
            device->functions.vkCmdBindIndexBuffer(
                destination,
                setIndexBuffer->buffer->buffer,
                setIndexBuffer->offset,
                toVulkanIndexFormat(setIndexBuffer->format)
            );
        }
        break;
        case rp_command_type_set_bind_group: {
            const RenderPassCommandSetBindGroup* setBindGroup = &command->setBindGroup;
            if(setBindGroup->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
                destination_->graphicsBindGroups[setBindGroup->groupIndex] = setBindGroup->group;
            else
                destination_->computeBindGroups[setBindGroup->groupIndex] = setBindGroup->group;
                
            device->functions.vkCmdBindDescriptorSets(
                destination,
                setBindGroup->bindPoint,
                destination_->lastLayout,
                setBindGroup->groupIndex,
                1,
                &setBindGroup->group->set,
                setBindGroup->dynamicOffsetCount,
                setBindGroup->dynamicOffsets
            );
        }
        break;
        case rp_command_type_set_render_pipeline: {
            const RenderPassCommandSetPipeline* setRenderPipeline = &command->setRenderPipeline;
            device->functions.vkCmdBindPipeline(
                destination,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                setRenderPipeline->pipeline->renderPipeline
            );
            destination_->lastLayout = setRenderPipeline->pipeline->layout->layout;
        }
        break;
        case cp_command_type_set_compute_pipeline: {
            const ComputePassCommandSetPipeline* setComputePipeline = &command->setComputePipeline;
            memset((void*)destination_->computeBindGroups, 0, sizeof(destination_->computeBindGroups));
            device->functions.vkCmdBindPipeline(
                destination,
                VK_PIPELINE_BIND_POINT_COMPUTE,
                setComputePipeline->pipeline->computePipeline
            );

            destination_->lastLayout = setComputePipeline->pipeline->layout->layout;
        }
        break;
        case cp_command_type_dispatch_workgroups: {
            const ComputePassCommandDispatchWorkgroups* dispatch = &command->dispatchWorkgroups;
            //ce_trackBuffer(WGPUCommandEncoder encoder, WGPUBuffer buffer, BufferUsageSnap usage)
            device->functions.vkCmdDispatch(
                destination, 
                dispatch->x, 
                dispatch->y, 
                dispatch->z
            );
        }
        break;
        
        case rp_command_type_set_force32: // fallthrough
        case rp_command_type_enum_count:  // fallthrough
        case rp_command_type_invalid: rassert(false, "Invalid command type"); rg_unreachable();
    }
}

void recordVkCommands(VkCommandBuffer destination, WGPUDevice device, const RenderPassCommandGenericVector* commands, const RenderPassCommandBegin WGPU_NULLABLE *beginInfo){
    CommandBufferAndSomeState cal = {
        .buffer = destination,
        .device = device,
        .lastLayout = VK_NULL_HANDLE
    };

    for(size_t i = 0;i < commands->size;i++){
        recordVkCommand(&cal, commands->data + i, beginInfo);
    }
}

void registerTransitionCallback(void* texture_, ImageUsageRecord* record, void* pscache_){
    WGPUTexture texture = (WGPUTexture)texture_;
    WGPUDevice device = texture->device;
    WGPUCommandEncoder pscache = (WGPUCommandEncoder)pscache_;
    VkImageMemoryBarrier barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        NULL,
        VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        texture->layout,
        record->initialLayout,
        pscache->device->adapter->queueIndices.graphicsIndex,
        pscache->device->adapter->queueIndices.graphicsIndex,
        texture->image,
        (VkImageSubresourceRange){
            is__depthVk(texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            VK_REMAINING_MIP_LEVELS,
            0,
            VK_REMAINING_ARRAY_LAYERS
        }
    };
    device->functions.vkCmdPipelineBarrier(
        pscache->buffer,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );
    //ce_trackTexture(pscache, texture, artificial);
}

void updateLayoutCallback(void* texture_, ImageUsageRecord* record, void* unused){
    WGPUTexture texture = (WGPUTexture)texture_;
    texture->layout = record->lastLayout;
}

void releaseCommandBuffersDependingOnFence(void* userdata){
    PendingCommandBufferListRef* list = (PendingCommandBufferListRef*)userdata;
    WGPUCommandBufferVector* bufferVector = PendingCommandBufferMap_get(list->map, list->fence);
    for(uint32_t i = 0;i < bufferVector->size;i++){
        wgpuCommandBufferRelease(bufferVector->data[i]);
    }
    WGPUCommandBufferVector_free(bufferVector);
}

void wgpuQueueSubmit(WGPUQueue queue, size_t commandCount, const WGPUCommandBuffer* buffers){
    VkSubmitInfo si zeroinit;
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = commandCount + 1;

    VkCommandBufferVector submittable;
    WGPUCommandBufferVector submittableWGPU;

    VkCommandBufferVector_initWithSize(&submittable, commandCount + 1);
    WGPUCommandBufferVector_initWithSize(&submittableWGPU, commandCount + 1);

    WGPUCommandEncoder pscache = queue->presubmitCache;
    
    // LayoutAssumptions_for_each(&pscache->resourceUsage.entryAndFinalLayouts, welldamn_sdfd, NULL);
    
    WGPUCommandBuffer sbuffer = buffers[0];
    ImageUsageRecordMap_for_each(&sbuffer->resourceUsage.referencedTextures, registerTransitionCallback, pscache); 
    
    
    WGPUCommandBuffer cachebuffer = wgpuCommandEncoderFinish(queue->presubmitCache);
    
    submittable.data[0] = cachebuffer->buffer;
    for(size_t i = 0;i < commandCount;i++){
        submittable.data[i + 1] = buffers[i]->buffer;
    }

    submittableWGPU.data[0] = cachebuffer;
    for(size_t i = 0;i < commandCount;i++){
        submittableWGPU.data[i + 1] = buffers[i];
    }

    for(uint32_t i = 0;i < submittableWGPU.size;i++){
        ImageUsageRecordMap_for_each(&submittableWGPU.data[i]->resourceUsage.referencedTextures, updateLayoutCallback, NULL);
    }
    
    WGPUFence fence = wgpuDeviceCreateFence(queue->device);
        
    si.commandBufferCount = submittable.size;
    si.pCommandBuffers = submittable.data;


    const uint64_t frameCount = queue->device->submittedFrames;
    const uint32_t cacheIndex = frameCount % framesInFlight;

    PerframeCache* perFrameCache = &queue->device->frameCaches[cacheIndex];
    
    VkResult submitResult = 0;
    
    for(uint32_t i = 0;i < submittable.size;i++){
        VkSemaphoreVector waitSemaphores;
        VkSemaphoreVector_init(&waitSemaphores);

        if(queue->syncState[cacheIndex].acquireImageSemaphoreSignalled){
            VkSemaphoreVector_push_back(&waitSemaphores, queue->syncState[cacheIndex].acquireImageSemaphore);
            //waitSemaphores.push_back(queue->syncState[cacheIndex].acquireImageSemaphore);   
            queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = false;
        }
        if(queue->syncState[cacheIndex].submits > 0){
            //waitSemaphores.push_back(queue->syncState[cacheIndex].semaphores[queue->syncState[cacheIndex].submits]);
            VkSemaphoreVector_push_back(&waitSemaphores, queue->syncState[cacheIndex].semaphores.data[queue->syncState[cacheIndex].submits]);
        }
        else{
            //std::cout << "";
        }

        si.commandBufferCount = 1;
        uint32_t submits = queue->syncState[cacheIndex].submits;
        
        VkPipelineStageFlags* waitFlags = (VkPipelineStageFlags*)RL_CALLOC(waitSemaphores.size, sizeof(VkPipelineStageFlags));
        for(uint32_t i = 0;i < waitSemaphores.size;i++){
            waitFlags[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        }
        si.waitSemaphoreCount = waitSemaphores.size;
        si.pWaitSemaphores = waitSemaphores.data;
        si.pWaitDstStageMask = waitFlags;

        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = queue->syncState[cacheIndex].semaphores.data + queue->syncState[cacheIndex].submits + 1;
        //std::cout << "And signalling " << queue->syncState[cacheIndex].submits + 1 << std::endl;
        si.pCommandBuffers = submittable.data + i;
        
        ++queue->syncState[cacheIndex].submits;

        WGPUFence submitFence = (i == (submittable.size - 1)) ? fence : NULL;
        submitResult |= queue->device->functions.vkQueueSubmit(queue->graphicsQueue, 1, &si, submitFence ? submitFence->fence : VK_NULL_HANDLE);
        if(submitFence){
            submitFence->state = WGPUFenceState_InUse;
        }
        
        VkSemaphoreVector_free(&waitSemaphores);
        RL_FREE(waitFlags);
    }

    if(submitResult == VK_SUCCESS){

        for(uint32_t i = 0;i < submittableWGPU.size;i++){
            WGPUCommandBuffer submittedBuffer = submittableWGPU.data[i];
            BufferUsageRecordMap map = submittedBuffer->resourceUsage.referencedBuffers; 
            for(size_t refbEntry = 0;refbEntry < map.current_capacity;refbEntry++){
                BufferUsageRecordMap_kv_pair* kv_pair = &map.table[refbEntry];
                WGPUBuffer keybuffer = (WGPUBuffer)kv_pair->key;
                if(kv_pair->key != PHM_EMPTY_SLOT_KEY && kv_pair->value.everWrittenTo != VK_FALSE && (keybuffer->usage & (WGPUBufferUsage_MapWrite | WGPUBufferUsage_MapRead))){
                    if(keybuffer->latestFence)
                        wgpuFenceRelease(keybuffer->latestFence);
                    keybuffer->latestFence = fence;
                    wgpuFenceAddRef(fence);
                    //CallbackWithUserdataVector_push_back(
                    //    &fence->callbacksOnWaitComplete
                    //);
                }
            }
        }
        
        WGPUCommandBufferVector insert;
        WGPUCommandBufferVector_init(&insert);
        
        WGPUCommandBufferVector_push_back(&insert, cachebuffer);
        ++cachebuffer->refCount;
        
        for(size_t i = 0;i < commandCount;i++){
            WGPUCommandBufferVector_push_back(&insert, buffers[i]);
            //insert.insert(buffers[i]);
            ++buffers[i]->refCount;
        }
        WGPUCommandBufferVector* fence_iterator = PendingCommandBufferMap_get(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence);
        //auto it = queue->pendingCommandBuffers[frameCount % framesInFlight].find(fence);
        if(fence_iterator == NULL){
            WGPUCommandBufferVector insert;
            PendingCommandBufferMap_put(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence, insert);
            fence_iterator = PendingCommandBufferMap_get(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence);
            PendingCommandBufferListRef* ud = RL_CALLOC(1, sizeof(PendingCommandBufferListRef));
            ud->fence = fence;
            ud->map = &(queue->pendingCommandBuffers[frameCount % framesInFlight]);
            
            CallbackWithUserdataVector_push_back(&fence->callbacksOnWaitComplete, (CallbackWithUserdata){
                .callback = releaseCommandBuffersDependingOnFence,
                .userdata = ud,
                .freeUserData = free
            });
            rassert(fence_iterator != NULL, "Something is wrong with the hash set");
            WGPUCommandBufferVector_init(fence_iterator);
        }

        for(size_t i = 0;i < insert.size;i++){
            WGPUCommandBufferVector_push_back(fence_iterator, insert.data[i]);
        }
        WGPUCommandBufferVector_free(&insert);
    }else{
        TRACELOG(LOG_FATAL, "vkQueueSubmit failed");
    }
    wgpuCommandEncoderRelease(queue->presubmitCache);
    wgpuCommandBufferRelease(cachebuffer);
    WGPUCommandEncoderDescriptor cedesc zeroinit;
    queue->presubmitCache = wgpuDeviceCreateCommandEncoder(queue->device, &cedesc);
    VkCommandBufferVector_free(&submittable);
    WGPUCommandBufferVector_free(&submittableWGPU);
}



void wgpuSurfaceGetCapabilities(WGPUSurface wgpuSurface, WGPUAdapter adapter, WGPUSurfaceCapabilities* capabilities){
    VkSurfaceKHR surface = wgpuSurface->surface;
    VkSurfaceCapabilitiesKHR scap zeroinit;
    VkPhysicalDevice vk_physicalDevice = adapter->physicalDevice;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physicalDevice, surface, &scap);
    
    // TRACELOG(LOG_INFO, "scalphaflags: %d", scap.supportedCompositeAlpha);
    
    // Formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, NULL);
    
    if (formatCount != 0) {
        wgpuSurface->formatCache = (VkSurfaceFormatKHR*)RL_CALLOC(formatCount, sizeof(VkSurfaceFormatKHR));
        wgpuSurface->wgpuFormatCache = (WGPUTextureFormat*)RL_CALLOC(formatCount, sizeof(WGPUTextureFormat));
        VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)RL_CALLOC(formatCount, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, surfaceFormats);
        for(size_t i = 0;i < formatCount;i++){
            wgpuSurface->formatCache[i] = surfaceFormats[i];
            if(surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                wgpuSurface->wgpuFormatCache[wgpuSurface->wgpuFormatCount++] = fromVulkanPixelFormat(surfaceFormats[i].format);
            }
        }
        wgpuSurface->formatCount = formatCount;
        RL_FREE(surfaceFormats);
    }

    // Present Modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, NULL);
    if (presentModeCount != 0) {
        wgpuSurface->presentModeCache = (WGPUPresentMode*)RL_CALLOC(presentModeCount, sizeof(WGPUPresentMode));
        VkPresentModeKHR presentModes[32] = {0};//(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, presentModes);
        for(size_t i = 0;i < presentModeCount;i++){
            wgpuSurface->presentModeCache[i] = fromVulkanPresentMode(presentModes[i]);
        }
        wgpuSurface->presentModeCount = presentModeCount;
    }
    capabilities->presentModeCount = wgpuSurface->presentModeCount;
    capabilities->formatCount = wgpuSurface->formatCount;

    WGPUPresentMode*   retpm = RL_CALLOC(presentModeCount, sizeof(WGPUPresentMode));
    WGPUTextureFormat* retfm = RL_CALLOC(formatCount,      sizeof(WGPUTextureFormat));

    for(uint32_t i = 0;i < wgpuSurface->presentModeCount;i++){
        retpm[i] = wgpuSurface->presentModeCache[i];
    }
    for(uint32_t i = 0;i < wgpuSurface->formatCount;i++){
        retfm[i] = fromVulkanPixelFormat(wgpuSurface->formatCache[i].format);
    }
    capabilities->usages = fromVulkanWGPUTextureUsage(scap.supportedUsageFlags);
}

void wgpuSurfaceConfigure(WGPUSurface surface, const WGPUSurfaceConfiguration* config){
    WGPUDevice device = config->device;
    surface->device = config->device;
    surface->lastConfig = *config;
    device->functions.vkDeviceWaitIdle(device->device);
    if(surface->imageViews){
        for (uint32_t i = 0; i < surface->imagecount; i++) {
            wgpuTextureViewRelease(surface->imageViews[i]);
            device->functions.vkDestroySemaphore(surface->device->device, surface->presentSemaphores[i], NULL);
            //This line is not required, since those are swapchain-owned images
            //These images also have a null memory member
            //wgpuReleaseTexture(surface->images[i]);
        }
    }

    //std::free(surface->framebuffers);
    if(surface->swapchain){
        RL_FREE((void*)surface->imageViews);
        RL_FREE((void*)surface->images);
        RL_FREE((void*)surface->presentSemaphores);
        device->functions.vkDestroySwapchainKHR(device->device, surface->swapchain, NULL);
    }
    VkSurfaceCapabilitiesKHR vkCapabilities zeroinit;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->adapter->physicalDevice, surface->surface, &vkCapabilities);
    VkSwapchainCreateInfoKHR createInfo zeroinit;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;
    uint32_t correctedWidth, correctedHeight;
    #define SWAPCHAIN_ICLAMP_TEMP(V, MINI, MAXI) ((V) < (MINI)) ? (MINI) : (((V) > (MAXI)) ? (MAXI) : (V))
    if(config->width < vkCapabilities.minImageExtent.width || config->width > vkCapabilities.maxImageExtent.width){
        correctedWidth = SWAPCHAIN_ICLAMP_TEMP(config->width, vkCapabilities.minImageExtent.width, vkCapabilities.maxImageExtent.width);
        TRACELOG(LOG_WARNING, "Invalid SurfaceConfiguration::width %u, adjusting to %u", config->width, correctedWidth);
    }
    else{
        correctedWidth = config->width;
    }
    if(config->height < vkCapabilities.minImageExtent.height || config->height > vkCapabilities.maxImageExtent.height){
        correctedHeight = SWAPCHAIN_ICLAMP_TEMP(config->height, vkCapabilities.minImageExtent.height, vkCapabilities.maxImageExtent.height);
        TRACELOG(LOG_WARNING, "Invalid SurfaceConfiguration::height %u, adjusting to %u", config->height, correctedHeight);
    }
    else{
        correctedHeight = config->height;
    }
    TRACELOG(LOG_INFO, "Capabilities minImageCount: %d", (int)vkCapabilities.minImageCount);
    
    createInfo.minImageCount = vkCapabilities.minImageCount + 1;
    createInfo.imageFormat = toVulkanPixelFormat(config->format);//swapchainImageFormat;
    surface->width  = correctedWidth;
    surface->height = correctedHeight;
    surface->device = config->device;
    VkExtent2D newExtent = {correctedWidth, correctedHeight};
    createInfo.imageExtent = newExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue family indices
    uint32_t queueFamilyIndices[2] = {
        device->adapter->queueIndices.graphicsIndex, 
        device->adapter->queueIndices.transferIndex
    };

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = NULL;
    }

    createInfo.preTransform = vkCapabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = toVulkanPresentMode(config->presentMode); 
    createInfo.clipped = VK_TRUE;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkResult scCreateResult = device->functions.vkCreateSwapchainKHR(device->device, &createInfo, NULL, &(surface->swapchain));
    if (scCreateResult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create swap chain!");
    } else {
        TRACELOG(LOG_INFO, "wgpuSurfaceConfigure(): Successfully created swap chain");
    }

    device->functions.vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, NULL);

    VkImage tmpImages[32] = {0};

    device->functions.vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, tmpImages);
    surface->images            = (WGPUTexture*)RL_CALLOC(surface->imagecount, sizeof(WGPUTexture));
    surface->presentSemaphores = (VkSemaphore*)RL_CALLOC(surface->imagecount, sizeof(VkSemaphore));
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        surface->images[i] = callocnew(WGPUTextureImpl);
        surface->images[i]->device = device;
        surface->images[i]->width = correctedWidth;
        surface->images[i]->height = correctedHeight;
        surface->images[i]->depthOrArrayLayers = 1;
        surface->images[i]->refCount = 1;
        surface->images[i]->sampleCount = 1;
        surface->images[i]->image = tmpImages[i];

        VkSemaphoreCreateInfo vci = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        device->functions.vkCreateSemaphore(device->device, &vci, NULL, surface->presentSemaphores + i);
    }
    surface->imageViews = (WGPUTextureView*)RL_CALLOC(surface->imagecount, sizeof(VkImageView));

    for (uint32_t i = 0; i < surface->imagecount; i++) {
        const WGPUTextureViewDescriptor viewDesc = {
            .arrayLayerCount = 1,
            .baseArrayLayer = 0,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .aspect = WGPUTextureAspect_All,
            .dimension = WGPUTextureViewDimension_2D,
            .format = config->format,
            .usage = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc,
        };
        surface->imageViews[i] = wgpuTextureCreateView(surface->images[i], &viewDesc);
    }
}

void wgpuSurfaceRelease(WGPUSurface surface){
    if(--surface->refCount == 0){
        if(surface->swapchain){
            for(uint32_t i = 0;i < surface->imagecount;i++){
                wgpuTextureViewRelease(surface->imageViews[i]);
            }
            RL_FREE((void*)surface->imageViews);
            RL_FREE((void*)surface->images);
            RL_FREE(surface->formatCache);
            RL_FREE(surface->presentModeCache);
            surface->device->functions.vkDestroySwapchainKHR(surface->device->device, surface->swapchain, NULL);
        }
    }
}

void wgpuComputePassEncoderDispatchWorkgroups(WGPUComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z){
    RenderPassCommandGeneric insert = {
        .type = cp_command_type_dispatch_workgroups,
        .dispatchWorkgroups = {x, y, z}
    };
    for(uint32_t groupIndex = 0;groupIndex < 8;groupIndex++){
        if(cpe->bindGroups[groupIndex]){
            const WGPUBindGroup group = cpe->bindGroups[groupIndex];
            for(uint32_t entryIndex = 0;entryIndex < group->entryCount;entryIndex++){
                const WGPUBindGroupEntry* entry = group->entries + entryIndex;
                if(entry->buffer){
                    ce_trackBuffer(cpe->cmdEncoder, entry->buffer, (BufferUsageSnap){
                        .access = VK_ACCESS_SHADER_WRITE_BIT,
                        .stage  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
                    });
                }
            }
        }
    }

    RenderPassCommandGenericVector_push_back(&cpe->bufferedCommands, insert);
}


void wgpuCommandEncoderRelease(WGPUCommandEncoder commandEncoder) {

    rassert(commandEncoder->movedFrom, "Commandencoder still valid");

    if(commandEncoder->buffer){
        VkCommandBufferVector_push_back(
            &commandEncoder->device->frameCaches[commandEncoder->cacheIndex].commandBuffers,
            commandEncoder->buffer
        );
    }
    
    
    RL_FREE(commandEncoder);
}

static void releaseRPSetCallback(WGPURenderPassEncoder rpEncoder, void* unused){
    wgpuRenderPassEncoderRelease(rpEncoder);
}

static void releaseCPSetCallback(WGPUComputePassEncoder cpEncoder, void* unused){
    wgpuComputePassEncoderRelease(cpEncoder);
}

static void releaseRTSetCallback(WGPURaytracingPassEncoder rtEncoder, void* unused){
    wgpuReleaseRaytracingPassEncoder(rtEncoder);
}
void wgpuCommandBufferRelease(WGPUCommandBuffer commandBuffer) {
    if(--commandBuffer->refCount == 0){
        WGPURenderPassEncoderSet_for_each(&commandBuffer->referencedRPs, releaseRPSetCallback, NULL);
        WGPUComputePassEncoderSet_for_each(&commandBuffer->referencedCPs, releaseCPSetCallback, NULL);
        WGPURaytracingPassEncoderSet_for_each(&commandBuffer->referencedRTs, releaseRTSetCallback, NULL);
        
        releaseAllAndClear(&commandBuffer->resourceUsage);
        // The above performs ResourceUsage_free already!
    

        WGPURenderPassEncoderSet_free(&commandBuffer->referencedRPs);
        WGPUComputePassEncoderSet_free(&commandBuffer->referencedCPs);
        WGPURaytracingPassEncoderSet_free(&commandBuffer->referencedRTs);
        
        PerframeCache* frameCache = &commandBuffer->device->frameCaches[commandBuffer->cacheIndex];
        VkCommandBufferVector_push_back(&frameCache->commandBuffers, commandBuffer->buffer);
        
        RL_FREE(commandBuffer);
    }
}
void wgpuRenderPassEncoderAddRef(WGPURenderPassEncoder rpenc) {
    ++rpenc->refCount;
}

void wgpuRenderPassEncoderRelease(WGPURenderPassEncoder rpenc) {
    if (--rpenc->refCount == 0) {
        releaseAllAndClear(&rpenc->resourceUsage);
        if(rpenc->frameBuffer){
            rpenc->device->functions.vkDestroyFramebuffer(rpenc->device->device, rpenc->frameBuffer, NULL);
        }
        ResourceUsage_free(&rpenc->resourceUsage);
        RenderPassCommandGenericVector_free(&rpenc->bufferedCommands);
        RL_FREE(rpenc);
    }
}

void wgpuShaderModuleRelease(WGPUShaderModule module){
    if(--module->refCount == 0){
        
        module->device->functions.vkDestroyShaderModule(module->device->device, module->vulkanModule, NULL);
        
        if(module->source->sType == WGPUSType_ShaderSourceSPIRV){
            RL_FREE(((WGPUShaderSourceSPIRV*)module->source)->code);
        }
        if(module->source->sType == WGPUSType_ShaderSourceWGSL){
            RL_FREE((void*)((WGPUShaderSourceWGSL*)module->source)->code.data);
        }
        RL_FREE(module->source);
        RL_FREE(module);
    }
}
void wgpuSamplerRelease(WGPUSampler sampler){
    if(!--sampler->refCount){
        sampler->device->functions.vkDestroySampler(sampler->device->device, sampler->sampler, NULL);
        RL_FREE(sampler);
    }
}
void wgpuRenderPipelineRelease(WGPURenderPipeline pipeline){
    if(!--pipeline->refCount){
        wgpuPipelineLayoutRelease(pipeline->layout);
        pipeline->device->functions.vkDestroyPipeline(pipeline->device->device, pipeline->renderPipeline, NULL);
        RL_FREE(pipeline);
    }
}

void wgpuComputePipelineRelease(WGPUComputePipeline pipeline){
    if(!--pipeline->refCount){
        wgpuPipelineLayoutRelease(pipeline->layout);
        pipeline->device->functions.vkDestroyPipeline(pipeline->device->device, pipeline->computePipeline, NULL);
        RL_FREE(pipeline);
    }
}

void wgpuBufferRelease(WGPUBuffer buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        if(buffer->latestFence){
            wgpuFenceRelease(buffer->latestFence);
            buffer->latestFence = NULL;
        }
        vmaDestroyBuffer(buffer->device->allocator, buffer->buffer, buffer->allocation);
        RL_FREE(buffer);
    }
}

void wgpuBindGroupRelease(WGPUBindGroup dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        releaseAllAndClear(&dshandle->resourceUsage);
        wgpuBindGroupLayoutRelease(dshandle->layout);
        BindGroupCacheMap* bgcm = &dshandle->device->frameCaches[dshandle->cacheIndex].bindGroupCache;
        DescriptorSetAndPool insertValue = {
            .pool = dshandle->pool,
            .set = dshandle->set
        };
        DescriptorSetAndPoolVector* maybeAlreadyThere = BindGroupCacheMap_get(bgcm, dshandle->layout);
        if(maybeAlreadyThere == NULL){
            DescriptorSetAndPoolVector empty zeroinit;
            BindGroupCacheMap_put(bgcm, dshandle->layout, empty);
            maybeAlreadyThere = BindGroupCacheMap_get(bgcm, dshandle->layout);
            rassert(maybeAlreadyThere != NULL, "Still null after insert");
            DescriptorSetAndPoolVector_init(maybeAlreadyThere);
        }
        
        if(maybeAlreadyThere){
            DescriptorSetAndPoolVector_push_back(maybeAlreadyThere, insertValue);
        }
        RL_FREE(dshandle->entries);

        // DONT delete them, they are cached
        // vkFreeDescriptorSets(dshandle->device->device, dshandle->pool, 1, &dshandle->set);
        // vkDestroyDescriptorPool(dshandle->device->device, dshandle->pool, NULL);
        
        RL_FREE(dshandle);
    }
}
void wgpuInstanceAddRef                       (WGPUInstance instance){
    ++instance->refCount;
}
void wgpuAdapterAddRef                        (WGPUAdapter adapter){
    ++adapter->refCount;
}
void wgpuDeviceAddRef                         (WGPUDevice device){
    ++device->refCount;
}
void wgpuQueueAddRef                          (WGPUQueue queue){
    ++queue->refCount;
}

void wgpuInstanceRelease                      (WGPUInstance instance){
    if(--instance->refCount == 0){
        vkDestroyDebugUtilsMessengerEXT(instance->instance, instance->debugMessenger, NULL);
        vkDestroyInstance(instance->instance, NULL);
        FutureIDMap_free(&instance->g_futureIDMap);
        RL_FREE(instance);
    }
}

void wgpuAdapterRelease(WGPUAdapter adapter){
    if(--adapter->refCount == 0){
        wgpuInstanceRelease(adapter->instance);
        RL_FREE(adapter);
    }
}
void resetFenceAndReleaseBuffers(void* fence_, WGPUCommandBufferVector* cBuffers, void* wgpudevice);
void wgpuDeviceRelease(WGPUDevice device){
    if(--device->refCount == 0){
        WGPUCommandBuffer cBuffer = wgpuCommandEncoderFinish(device->queue->presubmitCache);
        wgpuCommandEncoderRelease(device->queue->presubmitCache);
        wgpuCommandBufferRelease(cBuffer);

        {  // Destroy PerframeCaches
            for(uint32_t i = 0;i < framesInFlight;i++){
                PerframeCache* cache = device->frameCaches + i;
                PendingCommandBufferMap* pcm = device->queue->pendingCommandBuffers + i;
                for(size_t c = 0;c < pcm->current_capacity;c++){
                    PendingCommandBufferMap_kv_pair* kvp = pcm->table + c;

                    if(kvp->key != PHM_EMPTY_SLOT_KEY){
                        WGPUFence keyfence = kvp->key;
                        if(keyfence->state == WGPUFenceState_InUse){
                            wgpuFenceWait(keyfence, UINT32_MAX);
                        }
                        wgpuFenceRelease(keyfence);
                        WGPUCommandBufferVector* value = &kvp->value;
                        for(size_t cbi = 0;cbi < value->size;cbi++){
                            wgpuCommandBufferRelease(value->data[cbi]);
                        }
                    }
                }
                PendingCommandBufferMap_free(pcm);
                //PendingCommandBufferMap_for_each(pcm, resetFenceAndReleaseBuffers, device);  
                device->functions.vkFreeCommandBuffers(device->device, cache->commandPool, 1, &cache->finalTransitionBuffer);
                device->functions.vkDestroySemaphore(device->device, cache->finalTransitionSemaphore, NULL);


                device->functions.vkDestroySemaphore(device->device, device->queue->syncState[i].acquireImageSemaphore, NULL);
                for(uint32_t s = 0;s < device->queue->syncState[i].semaphores.size;s++){
                    device->functions.vkDestroySemaphore(device->device, device->queue->syncState[i].semaphores.data[s], NULL);//todo
                }
                VkSemaphoreVector_free(&device->queue->syncState[i].semaphores);
                wgpuFenceRelease(cache->finalTransitionFence);
                
                if(cache->commandBuffers.size){
                    device->functions.vkFreeCommandBuffers(device->device, cache->commandPool, cache->commandBuffers.size, cache->commandBuffers.data);
                    VkCommandBufferVector_free(&cache->commandBuffers);
                }
                for(size_t bgc = 0;bgc < cache->bindGroupCache.current_capacity;bgc++){
                    if(cache->bindGroupCache.table[bgc].key != PHM_EMPTY_SLOT_KEY){
                        DescriptorSetAndPoolVector* dspv = &cache->bindGroupCache.table[bgc].value;
                        for(size_t vi = 0;vi < dspv->size;vi++){
                            //device->functions.vkFreeDescriptorSets(device->device, dspv->data[i].pool, 1, &dspv->data[i].set); 
                            device->functions.vkDestroyDescriptorPool(device->device, dspv->data[i].pool, NULL); 
                        }
                        DescriptorSetAndPoolVector_free(dspv);
                    }
                }
                BindGroupCacheMap_free(&cache->bindGroupCache);
                device->functions.vkDestroyCommandPool(device->device, cache->commandPool, NULL);
            }
            vmaDestroyPool(device->allocator, device->aligned_hostVisiblePool);
            vmaDestroyAllocator(device->allocator);
        }
        
        
        wgpuQueueRelease(device->queue);
        wgpuAdapterRelease(device->adapter);
        device->functions.vkDestroyDevice(device->device, NULL);
        // Still a lot to do
        RL_FREE(device->queue);
        RL_FREE(device);
    }
}
void wgpuQueueRelease                         (WGPUQueue queue){
    if(--queue->refCount == 0){
        //wgpuDeviceRelease(queue->device);
    }
}

WGPUComputePipeline wgpuDeviceCreateComputePipeline(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor){
    WGPUComputePipeline ret = RL_CALLOC(1, sizeof(WGPUComputePipelineImpl));
    ret->device = device;
    ret->refCount = 1;

    char namebuffer[512];
    rassert(descriptor->compute.entryPoint.length < 511, "griiindumehaue");
    memcpy(namebuffer, descriptor->compute.entryPoint.data, descriptor->compute.entryPoint.length);
    namebuffer[descriptor->compute.entryPoint.length] = 0;
    
    
    VkPipelineShaderStageCreateInfo computeStage = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        NULL,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        descriptor->compute.module->vulkanModule,
        namebuffer,
        NULL
    };

    VkComputePipelineCreateInfo cpci = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        NULL,
        0,
        computeStage,
        descriptor->layout->layout,
        0,
        0
    };
    device->functions.vkCreateComputePipelines(device->device, NULL, 1, &cpci, NULL, &ret->computePipeline);
    ret->layout = descriptor->layout;
    wgpuPipelineLayoutAddRef(ret->layout);
    return ret;
}


WGPURenderPipeline wgpuDeviceCreateRenderPipeline(WGPUDevice device, const WGPURenderPipelineDescriptor* descriptor) {
    WGPUDeviceImpl* deviceImpl = (WGPUDeviceImpl*)(device);
    WGPUPipelineLayout pl_layout = descriptor->layout;

    VkPipelineShaderStageCreateInfo shaderStages[16] = {
        (VkPipelineShaderStageCreateInfo){0}
    };
    uint32_t shaderStageInsertPos = 0;

    // Vertex Stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo zeroinit;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = (descriptor->vertex.module)->vulkanModule;
    vertShaderStageInfo.pName = descriptor->vertex.entryPoint.data; // Assuming null-terminated or careful length handling elsewhere
    // TODO: Handle constants if necessary via specialization constants
    // vertShaderStageInfo.pSpecializationInfo = ...;
    shaderStages[shaderStageInsertPos++] = vertShaderStageInfo;

    // Fragment Stage (Optional)
    VkPipelineShaderStageCreateInfo fragShaderStageInfo zeroinit;
    if (descriptor->fragment) {
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = (descriptor->fragment->module)->vulkanModule;
        fragShaderStageInfo.pName = descriptor->fragment->entryPoint.data;
        // TODO: Handle constants
        // fragShaderStageInfo.pSpecializationInfo = ...;
        shaderStages[shaderStageInsertPos++] = fragShaderStageInfo;
    }

    // Vertex Input State
    VkVertexInputBindingDescription   bindingDescriptions  [32] = {(VkVertexInputBindingDescription)  {0}};
    VkVertexInputAttributeDescription attributeDescriptions[32] = {(VkVertexInputAttributeDescription){0}};
    uint32_t attributeDescriptionCount = 0;
    uint32_t currentBinding = 0;
    for (size_t i = 0; i < descriptor->vertex.bufferCount; ++i) {
        const WGPUVertexBufferLayout* layout = &descriptor->vertex.buffers[i];
        bindingDescriptions[i].binding = currentBinding; // Assuming bindings are contiguous from 0, I think webgpu doesn't allow anything else
        bindingDescriptions[i].stride = (uint32_t)layout->arrayStride;
        bindingDescriptions[i].inputRate = toVulkanVertexStepMode(layout->stepMode);

        for (size_t j = 0; j < layout->attributeCount; ++j) {
            const WGPUVertexAttribute* attrib = &layout->attributes[j];
            
            attributeDescriptions[attributeDescriptionCount++] = (VkVertexInputAttributeDescription){
                .location = attrib->shaderLocation,
                .binding = currentBinding,
                .format = toVulkanVertexFormat(attrib->format),
                .offset = (uint32_t)attrib->offset
            };
        }
        currentBinding++;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo zeroinit;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = descriptor->vertex.bufferCount;
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions;
    vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    // Input Assembly State
    VkPipelineInputAssemblyStateCreateInfo inputAssembly zeroinit;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVulkanPrimitiveTopology(descriptor->primitive.topology);
    inputAssembly.primitiveRestartEnable = VK_FALSE; // WGPU doesn't expose primitive restart? Assume false.
    // Strip index formats are for degenerate triangles/lines, only matters if topology is strip and primitiveRestart is enabled.
    // Vulkan handles this via primitiveRestartEnable flag usually. VkIndexType part is handled by vkCmdBindIndexBuffer.

    // Viewport State (Dynamic)
    VkPipelineViewportStateCreateInfo viewportState zeroinit;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Rasterization State
    VkPipelineRasterizationStateCreateInfo rasterizer zeroinit;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE; // Usually false unless specific features needed
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Default
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode =  toVulkanCullMode(descriptor->primitive.cullMode);
    rasterizer.frontFace = toVulkanFrontFace(descriptor->primitive.frontFace);
    rasterizer.depthBiasEnable = descriptor->depthStencil ? (descriptor->depthStencil->depthBias != 0 || descriptor->depthStencil->depthBiasSlopeScale != 0.0f || descriptor->depthStencil->depthBiasClamp != 0.0f) : VK_FALSE;
    rasterizer.depthBiasConstantFactor = descriptor->depthStencil ? (float)descriptor->depthStencil->depthBias : 0.0f;
    rasterizer.depthBiasClamp = descriptor->depthStencil ? descriptor->depthStencil->depthBiasClamp : 0.0f;
    rasterizer.depthBiasSlopeFactor = descriptor->depthStencil ? descriptor->depthStencil->depthBiasSlopeScale : 0.0f;
    // TODO: Handle descriptor->primitive.unclippedDepth (requires VK_EXT_depth_clip_enable or VK 1.3 feature)
    // If unclippedDepth is true, rasterizer.depthClampEnable should probably be VK_FALSE (confusingly).
    // Requires checking device features. Assume false for now.

    // Multisample State
    VkPipelineMultisampleStateCreateInfo multisampling zeroinit;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE; // Basic case
    multisampling.rasterizationSamples = (VkSampleCountFlagBits)descriptor->multisample.count; // Assume direct mapping
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = &descriptor->multisample.mask;
    multisampling.alphaToCoverageEnable = descriptor->multisample.alphaToCoverageEnabled ? VK_TRUE : VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE; // Basic case

    // Depth Stencil State (Optional)
    VkPipelineDepthStencilStateCreateInfo depthStencil zeroinit;
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    if (descriptor->depthStencil) {
        const WGPUDepthStencilState* ds = descriptor->depthStencil;
        depthStencil.depthTestEnable = VK_TRUE; // If struct exists, assume depth test is desired
        depthStencil.depthWriteEnable = ds->depthWriteEnabled ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = toVulkanCompareFunction(ds->depthCompare);
        depthStencil.depthBoundsTestEnable = VK_FALSE; // Not in WGPU descriptor
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;

        bool stencilTestRequired =
            ds->stencilFront.compare != WGPUCompareFunction_Undefined || ds->stencilFront.failOp != WGPUStencilOperation_Undefined ||
            ds->stencilBack.compare != WGPUCompareFunction_Undefined || ds->stencilBack.failOp != WGPUStencilOperation_Undefined ||
            ds->stencilReadMask != 0 || ds->stencilWriteMask != 0;

        depthStencil.stencilTestEnable = stencilTestRequired ? VK_TRUE : VK_FALSE;
        if(stencilTestRequired) {
            depthStencil.front.failOp      = toVulkanStencilOperation(ds->stencilFront.failOp);
            depthStencil.front.passOp      = toVulkanStencilOperation(ds->stencilFront.passOp);
            depthStencil.front.depthFailOp = toVulkanStencilOperation(ds->stencilFront.depthFailOp);
            depthStencil.front.compareOp   = toVulkanCompareFunction(ds->stencilFront.compare);
            depthStencil.front.compareMask = ds->stencilReadMask;
            depthStencil.front.writeMask   = ds->stencilWriteMask;
            depthStencil.front.reference   = 0; // Dynamic state usually

            depthStencil.back.failOp       = toVulkanStencilOperation(ds->stencilBack.failOp);
            depthStencil.back.passOp       = toVulkanStencilOperation(ds->stencilBack.passOp);
            depthStencil.back.depthFailOp  = toVulkanStencilOperation(ds->stencilBack.depthFailOp);
            depthStencil.back.compareOp    = toVulkanCompareFunction(ds->stencilBack.compare);
            depthStencil.back.compareMask  = ds->stencilReadMask;
            depthStencil.back.writeMask    = ds->stencilWriteMask;
            depthStencil.back.reference    = 0; // Dynamic state usually
        }
    } else {
        depthStencil.depthTestEnable = VK_FALSE;
        depthStencil.depthWriteEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
    }

    // Color Blend State (Requires fragment shader)
    VkPipelineColorBlendAttachmentState colorBlendAttachments[MAX_COLOR_ATTACHMENTS];
    VkPipelineColorBlendStateCreateInfo colorBlending zeroinit;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    if (descriptor->fragment) {
        for (size_t i = 0; i < descriptor->fragment->targetCount; ++i) {
            const WGPUColorTargetState* target = &descriptor->fragment->targets[i];
            // Defaults for no blending
            colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // TODO: Map WGPUColorWriteMask if exists
            colorBlendAttachments[i].blendEnable = VK_FALSE;
            colorBlendAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

            if (target->blend) {
                colorBlendAttachments[i].blendEnable = VK_TRUE; // Enable if blend state is provided
                colorBlendAttachments[i].srcColorBlendFactor = toVulkanBlendFactor(target->blend->color.srcFactor);
                colorBlendAttachments[i].dstColorBlendFactor = toVulkanBlendFactor(target->blend->color.dstFactor);
                colorBlendAttachments[i].colorBlendOp = toVulkanBlendOperation(target->blend->color.operation);
                colorBlendAttachments[i].srcAlphaBlendFactor = toVulkanBlendFactor(target->blend->alpha.srcFactor);
                colorBlendAttachments[i].dstAlphaBlendFactor = toVulkanBlendFactor(target->blend->alpha.dstFactor);
                colorBlendAttachments[i].alphaBlendOp = toVulkanBlendOperation(target->blend->alpha.operation);
            }
        }
        colorBlending.attachmentCount = descriptor->fragment->targetCount;
        colorBlending.pAttachments = colorBlendAttachments;
    } else {
        // No fragment shader means no color attachments needed? Check spec.
        // Typically, rasterizerDiscardEnable would be true if no fragment shader, but let's allow it.
        colorBlending.attachmentCount = 0;
        colorBlending.pAttachments = NULL;
    }


    // Dynamic State
    uint32_t dynamicStateCount = 2;
    VkDynamicState dynamicStates[4] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        // Add VK_DYNAMIC_STATE_STENCIL_REFERENCE if stencil test is enabled and reference is not fixed
    };
    if (depthStencil.stencilTestEnable) {
        dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    }
    if (rasterizer.depthBiasEnable) {
        dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
    }


    VkPipelineDynamicStateCreateInfo dynamicState zeroinit;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStateCount;
    dynamicState.pDynamicStates = dynamicStates;

    // Pipeline Rendering Info (for dynamic rendering if used, otherwise NULL)
    // Assuming traditional render passes for now based on WGPU structure.
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo zeroinit; // Zero initialize
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    // Check if dynamic rendering should be used based on descriptor or device capabilities?
    // For simplicity, assuming we have a VkRenderPass (not provided here, usually comes from RenderPassEncoder Begin).
    // If using dynamic rendering, fill format info here. Example:
    /*
    std::vector<VkFormat> colorFormats;
    if (descriptor->fragment) {
        for(size_t i=0; i < descriptor->fragment->targetCount; ++i) {
            colorFormats.push_back(translate_PixelFormat_to_vk(descriptor->fragment->targets[i].format));
        }
    }
    pipelineRenderingCreateInfo.colorAttachmentCount = colorFormats.size();
    pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats.data();
    pipelineRenderingCreateInfo.depthAttachmentFormat = descriptor->depthStencil ? translate_PixelFormat_to_vk(descriptor->depthStencil->format) : VK_FORMAT_UNDEFINED;
    pipelineRenderingCreateInfo.stencilAttachmentFormat = descriptor->depthStencil ? translate_PixelFormat_to_vk(descriptor->depthStencil->format) : VK_FORMAT_UNDEFINED; // Often same as depth
    */
    // We'll assume VkRenderPass is provided externally during vkCreateGraphicsPipelines call for non-dynamic rendering


    // Graphics Pipeline Create Info
    VkGraphicsPipelineCreateInfo pipelineInfo zeroinit;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    // pipelineInfo.pNext = &pipelineRenderingCreateInfo; // Only if using dynamic rendering extension (VK_KHR_dynamic_rendering)
    pipelineInfo.stageCount = shaderStageInsertPos;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = (descriptor->depthStencil) ? &depthStencil : NULL;
    pipelineInfo.pColorBlendState = (descriptor->fragment) ? &colorBlending : NULL; // Only if frag shader exists
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pl_layout->layout;

    // RenderPass needs to be obtained from somewhere, usually associated with the command buffer recording.
    // WGPU API hides this detail? We might need to use dynamic rendering or query/cache render passes.
    // Placeholder: Assume renderPass is NULL and we rely on dynamic rendering or compatibility context.
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Needs a valid VkRenderPass unless VK_KHR_dynamic_rendering is used and enabled.
    pipelineInfo.subpass = 0; // Assuming subpass 0

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional


    WGPURenderPipelineImpl* pipelineImpl = (WGPURenderPipelineImpl*)malloc(sizeof(WGPURenderPipelineImpl));
    if (!pipelineImpl) {
        // Handle allocation failure
        return NULL;
    }
    pipelineImpl->device = device;
    //wgpuDeviceAddRef(device);
    pipelineImpl->layout = pl_layout; // Store for potential use
    wgpuPipelineLayoutAddRef(pl_layout);
    VkResult result = device->functions.vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipelineImpl->renderPipeline);

    if (result != VK_SUCCESS) {
        // Handle pipeline creation failure
        RL_FREE(pipelineImpl);
        // Optionally log error based on result code
        return NULL;
    }

    return pipelineImpl;
}
void wgpuBindGroupLayoutRelease(WGPUBindGroupLayout bglayout){
    --bglayout->refCount;
    if(bglayout->refCount == 0){
        bglayout->device->functions.vkDestroyDescriptorSetLayout(bglayout->device->device, bglayout->layout, NULL);
        RL_FREE((ResourceTypeDescriptor*)(bglayout->entries));
        RL_FREE(bglayout);
    }
}

void wgpuTextureRelease(WGPUTexture texture){
    --texture->refCount;
    if(texture->refCount == 0){
        texture->device->functions.vkDestroyImage(texture->device->device, texture->image, NULL);
        texture->device->functions.vkFreeMemory(texture->device->device, texture->memory, NULL);

        RL_FREE(texture);
    }
}
void wgpuTextureViewRelease(WGPUTextureView view){
    --view->refCount;
    if(view->refCount == 0){
        view->texture->device->functions.vkDestroyImageView(view->texture->device->device, view->view, NULL);
        wgpuTextureRelease(view->texture);
        RL_FREE(view);
    }
}

void wgpuCommandEncoderCopyBufferToBuffer  (WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size){
    ce_trackBuffer(
        commandEncoder,
        source,
        (BufferUsageSnap){
            .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .access = VK_ACCESS_TRANSFER_READ_BIT
        }
    );
    ce_trackBuffer(
        commandEncoder,
        destination,
        (BufferUsageSnap){
            .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .access = VK_ACCESS_TRANSFER_WRITE_BIT
        }
    );

    const VkBufferCopy copy = {
        sourceOffset,
        destinationOffset,
        size
    };

    commandEncoder->device->functions.vkCmdCopyBuffer(commandEncoder->buffer, source->buffer, destination->buffer, 1, &copy);
    if(destination->usage & (WGPUBufferUsage_MapWrite | WGPUBufferUsage_MapRead)){
        const VkMemoryBarrier memoryBarrier = {
            VK_STRUCTURE_TYPE_MEMORY_BARRIER,
            NULL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_HOST_READ_BIT
        };
        commandEncoder->device->functions.vkCmdPipelineBarrier(
            commandEncoder->buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_HOST_BIT,
            0,
            1, &memoryBarrier, 
            0, NULL, 
            0, NULL
        );
    }
}
void wgpuCommandEncoderCopyBufferToTexture (WGPUCommandEncoder commandEncoder, WGPUTexelCopyBufferInfo const * source, WGPUTexelCopyTextureInfo const * destination, WGPUExtent3D const * copySize){
    
    VkImageCopy copy;
    VkBufferImageCopy region zeroinit;

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = is__depthVk(destination->texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = CLITERAL(VkOffset3D){
        (int32_t)destination->origin.x,
        (int32_t)destination->origin.y,
        (int32_t)destination->origin.z,
    };

    region.imageExtent = CLITERAL(VkExtent3D){
        copySize->width,
        copySize->height,
        copySize->depthOrArrayLayers
    };
    
    ce_trackBuffer(commandEncoder, source->buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT});
    ce_trackTexture(commandEncoder, destination->texture, (ImageUsageSnap){
        .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .access = VK_ACCESS_TRANSFER_WRITE_BIT,
        .subresource = {
            .aspectMask = destination->aspect,
            .baseMipLevel = destination->mipLevel,
            .levelCount = 1,
            .baseArrayLayer = destination->origin.z,
            .layerCount = 1
        }
    });

    commandEncoder->device->functions.vkCmdCopyBufferToImage(commandEncoder->buffer, source->buffer->buffer, destination->texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
void wgpuCommandEncoderCopyTextureToBuffer (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyBufferInfo* destination, const WGPUExtent3D* copySize){
    ce_trackTexture(
        commandEncoder,
        source->texture,
        (ImageUsageSnap){
            .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .stage  = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .access = VK_ACCESS_TRANSFER_READ_BIT,
            .subresource = {
                .aspectMask     = toVulkanAspectMask(source->aspect),
                .baseMipLevel   = source->mipLevel,
                .baseArrayLayer = source->origin.z, // ?
                .layerCount     = 1,
                .levelCount     = 1,
            }
    });
    ce_trackBuffer(commandEncoder, destination->buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT});
    
    
    VkBufferImageCopy region = {
        .bufferOffset = destination->layout.offset,
        .bufferRowLength = destination->layout.bytesPerRow,
        .bufferImageHeight = destination->layout.rowsPerImage,
        .imageSubresource = {
            .aspectMask = toVulkanAspectMask(source->aspect),
            .baseArrayLayer = source->origin.z, // ?
            .mipLevel = source->mipLevel,
            .layerCount = 1,
        },
        .imageOffset = {
            .x = (int32_t)source->origin.x,
            .y = (int32_t)source->origin.y,
            .z = (int32_t)source->origin.z
        },
        .imageExtent = {
            .width = copySize->width,
            .height = copySize->height,
            .depth = copySize->depthOrArrayLayers
        }
    };

    commandEncoder->device->functions.vkCmdCopyImageToBuffer(
        commandEncoder->buffer,
        source->texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        destination->buffer->buffer,
        1, &region
    );
    rg_unreachable();
}
void wgpuCommandEncoderCopyTextureToTexture(WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyTextureInfo* destination, const WGPUExtent3D* copySize){
    
    ce_trackTexture(
        commandEncoder,
        source->texture,
        (ImageUsageSnap){
            .layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .stage  = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .access = VK_ACCESS_TRANSFER_READ_BIT,
            .subresource = {
                .aspectMask     = source->aspect,
                .baseMipLevel   = source->mipLevel,
                .baseArrayLayer = source->origin.z, // ?
                .layerCount     = 1,
                .levelCount     = 1,
            }
    });
    ce_trackTexture(
        commandEncoder,
        destination->texture,
        (ImageUsageSnap){
            .layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .stage  = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .access = VK_ACCESS_TRANSFER_WRITE_BIT,
            .subresource = {
                .aspectMask     = destination->aspect,
                .baseMipLevel   = destination->mipLevel,
                .baseArrayLayer = destination->origin.z, // ?
                .layerCount     = 1,
                .levelCount     = 1,
            }
    });

    VkImageBlit region = {
        .srcSubresource = {
            .aspectMask = source->aspect,
            .mipLevel = source->mipLevel,
            .baseArrayLayer = destination->origin.z, // ?
            .layerCount = 1,
        },
        .srcOffsets = {
            {source->origin.x, source->origin.y, source->origin.z},
            {source->origin.x + copySize->width, source->origin.y + copySize->height, source->origin.z + copySize->depthOrArrayLayers}
        },
        .dstSubresource = {
            .aspectMask = destination->aspect,
            .mipLevel = destination->mipLevel,
            .baseArrayLayer = destination->origin.z, // ?
            .layerCount = 1,
        },
        .dstOffsets[0] = {destination->origin.x,                   destination->origin.y,                    destination->origin.z},
        .dstOffsets[1] = {destination->origin.x + copySize->width, destination->origin.y + copySize->height, destination->origin.z + copySize->depthOrArrayLayers}
    };
    commandEncoder->device->functions.vkCmdBlitImage(
        commandEncoder->buffer,
        source->texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        destination->texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region,
        VK_FILTER_NEAREST
    );
}
void RenderPassEncoder_PushCommand(WGPURenderPassEncoder encoder, const RenderPassCommandGeneric* cmd){
    if(cmd->type == rp_command_type_set_render_pipeline){
        encoder->lastLayout = cmd->setRenderPipeline.pipeline->layout;
    }
    RenderPassCommandGenericVector_push_back(&encoder->bufferedCommands, *cmd);
}
void ComputePassEncoder_PushCommand(WGPUComputePassEncoder encoder, const RenderPassCommandGeneric* cmd){
    if(cmd->type == cp_command_type_set_compute_pipeline){
        encoder->lastLayout = cmd->setComputePipeline.pipeline->layout;
    }
    RenderPassCommandGenericVector_push_back(&encoder->bufferedCommands, *cmd);
}



// Implementation of RenderpassEncoderDraw
void wgpuRenderpassEncoderDraw(WGPURenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
    rassert(rpe != NULL, "RenderPassEncoderHandle is null");
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_draw,
        .draw = {
            vertices,
            instances,
            firstvertex,
            firstinstance
        }
    };

    RenderPassEncoder_PushCommand(rpe, &insert);
}

// Implementation of RenderpassEncoderDrawIndexed
void wgpuRenderpassEncoderDrawIndexed(WGPURenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t baseVertex, uint32_t firstinstance) {
    rassert(rpe != NULL, "RenderPassEncoderHandle is null");

    RenderPassCommandGeneric insert = {
        .type = rp_command_type_draw_indexed,
        .drawIndexed = {
            indices,
            instances,
            firstindex,
            baseVertex,
            firstinstance
        }
    };

    // Buffer the indexed draw command into the command buffer
    RenderPassEncoder_PushCommand(rpe, &insert);
    
    //vkCmdDrawIndexed(rpe->secondaryCmdBuffer, indices, instances, firstindex, (int32_t)(baseVertex & 0x7fffffff), firstinstance);
}

void wgpuRenderPassEncoderSetPipeline(WGPURenderPassEncoder rpe, WGPURenderPipeline renderPipeline) {
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_render_pipeline,
        .setRenderPipeline = {
            renderPipeline
        }
    };
    RenderPassEncoder_PushCommand(rpe, &insert);
    ru_trackRenderPipeline(&rpe->resourceUsage, renderPipeline);
}


const static int is_storage_texture[uniform_type_enumcount] = {
    [storage_texture2d_array] = 1,
    [storage_texture3d] = 1,
    [storage_texture2d] = 1,
};

const static int is_texture[uniform_type_enumcount] = {
    [storage_texture2d_array] = 1,
    [storage_texture3d] = 1,
    [storage_texture2d] = 1,
    [texture2d_array] = 1,
    [texture2d] = 1,
    [texture3d] = 1,
};

void wgpuRenderPassEncoderSetBindGroup(WGPURenderPassEncoder rpe, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, const uint32_t* dynamicOffsets) {
    rassert(rpe != NULL, "RenderPassEncoderHandle is null");
    rassert(group != NULL, "DescriptorSetHandle is null");

    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_bind_group,
        .setBindGroup = {
            groupIndex,
            group,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            dynamicOffsetCount,
            dynamicOffsets
        }
    };
    
    RenderPassEncoder_PushCommand(rpe, &insert);
    
    
    

    for(uint32_t i = 0;i < group->entryCount;i++){

        const WGPUBindGroupEntry* entry = &group->entries[i];

        if(entry->buffer){
            const VkAccessFlags accessFlags = extractVkAccessFlags(group->layout->entries + i);
            const VkPipelineStageFlags stage = toVulkanPipelineStageBits(group->layout->entries[i].visibility);
            ce_trackBuffer(rpe->cmdEncoder, entry->buffer, (BufferUsageSnap){
                .stage = stage,
                .access = accessFlags
            });
        }

        if(entry->textureView){
            const VkAccessFlags accessFlags = extractVkAccessFlags(group->layout->entries + i);
            const VkPipelineStageFlags stage = toVulkanPipelineStageBits(group->layout->entries[i].visibility) | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            
            //VkImageLayout layout = (extractVkDescriptorType(group->layout->entries + i) == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            ce_trackTextureView(rpe->cmdEncoder, entry->textureView, (ImageUsageSnap){
                .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .access = accessFlags,
                .stage = stage,
                .subresource = entry->textureView->subresourceRange
            });
        }
    }
    ru_trackBindGroup(&rpe->resourceUsage, group);
    return;
    error:
    abort();
    
}

void wgpuComputePassEncoderSetPipeline (WGPUComputePassEncoder cpe, WGPUComputePipeline computePipeline){
    
    RenderPassCommandGeneric insert = {
        .type = cp_command_type_set_compute_pipeline,
        .setComputePipeline = {
            computePipeline
        }
    };

    ComputePassEncoder_PushCommand(cpe, &insert);
}
void wgpuComputePassEncoderSetBindGroup(WGPUComputePassEncoder cpe, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, const uint32_t* dynamicOffsets){
    
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_bind_group,
        .setBindGroup = {
            groupIndex,
            group,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            dynamicOffsetCount,
            dynamicOffsets
        }
    };
    cpe->bindGroups[groupIndex] = group;
    
    ComputePassEncoder_PushCommand(cpe, &insert);
}

WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(WGPUCommandEncoder commandEncoder){
    WGPUComputePassEncoder ret = callocnew(WGPUComputePassEncoderImpl);
    ret->refCount = 2;
    WGPUComputePassEncoderSet_add(&commandEncoder->referencedCPs, ret);

    RenderPassCommandGenericVector_init(&ret->bufferedCommands);

    ret->cmdEncoder = commandEncoder;
    ret->device = commandEncoder->device;
    return ret;
}
void wgpuComputePassEncoderEnd(WGPUComputePassEncoder commandEncoder){
    recordVkCommands(commandEncoder->cmdEncoder->buffer, commandEncoder->device, &commandEncoder->bufferedCommands, NULL);
}
void wgpuComputePassEncoderRelease(WGPUComputePassEncoder cpenc){
    --cpenc->refCount;
    if(cpenc->refCount == 0){
        releaseAllAndClear(&cpenc->resourceUsage);
        RenderPassCommandGenericVector_free(&cpenc->bufferedCommands);
        RL_FREE(cpenc);
    }
}

void wgpuReleaseRaytracingPassEncoder(WGPURaytracingPassEncoder rtenc){
    --rtenc->refCount;
    if(rtenc->refCount == 0){
        releaseAllAndClear(&rtenc->resourceUsage);
        RL_FREE(rtenc);
    }
}

void pcmNonnullFlattenCallback(void* fence_, WGPUCommandBufferVector* key, void* userdata){
    WGPUFenceVector* vector = (WGPUFenceVector*)userdata;
    WGPUFence fence = (WGPUFence)fence_;
    if(fence != VK_NULL_HANDLE){
        WGPUFenceVector_push_back(vector, fence);
    }
}

void resetFenceAndReleaseBuffers(void* fence_, WGPUCommandBufferVector* cBuffers, void* wgpudevice){
    WGPUDevice device = (WGPUDevice)wgpudevice;
    if(fence_){
        WGPUFence fence = fence_;
        device->functions.vkResetFences(device->device, 1, &fence->fence);
        fence->state = WGPUFenceState_Reset;
        wgpuFenceRelease(fence);
    }
    for(size_t i = 0;i < cBuffers->size;i++){
        WGPUCommandBuffer relBuffer = cBuffers->data[i];
        BufferUsageRecordMap* bmap = &relBuffer->resourceUsage.referencedBuffers;
        for(size_t bi = 0; bi < bmap->current_capacity;bi++){
            BufferUsageRecordMap_kv_pair* kvp = bmap->table + bi;
            if(kvp->key != PHM_EMPTY_SLOT_KEY){
                WGPUBuffer buffer = (WGPUBuffer)kvp->key;
                if(buffer->latestFence){
                    wgpuFenceRelease(buffer->latestFence);
                    buffer->latestFence = VK_NULL_HANDLE;
                }
            }
        }
        wgpuCommandBufferRelease(cBuffers->data[i]);
    }
    WGPUCommandBufferVector_free(cBuffers);
}

void wgpuSurfaceGetCurrentTexture(WGPUSurface surface, WGPUSurfaceTexture* surfaceTexture){
    const size_t submittedframes = surface->device->submittedFrames;
    const uint32_t cacheIndex = surface->device->submittedFrames % framesInFlight;
    if(surface->swapchain){
        VkResult acquireResult = surface->device->functions.vkAcquireNextImageKHR(
            surface->device->device,
            surface->swapchain,
            UINT32_MAX,
            surface->device->queue->syncState[cacheIndex].acquireImageSemaphore,
            VK_NULL_HANDLE,
            &surface->activeImageIndex
        );
        surface->device->queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = true;

        switch(acquireResult){
            case VK_SUBOPTIMAL_KHR:
                surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal;
            break;
            case VK_SUCCESS:
                surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal;
            break;
            case VK_TIMEOUT:
                surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_Timeout;
            break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_Outdated;
            break;
            default:
                surfaceTexture->status = WGPUSurfaceGetCurrentTextureStatus_Error;
            break;
        }
        if(surfaceTexture->status == WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal || surfaceTexture->status == WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal){
            surfaceTexture->texture = surface->images[surface->activeImageIndex];
        }
    }
    else{
        TRACELOG(LOG_ERROR, "Surface is not configured");
    }
}

void wgpuSurfacePresent(WGPUSurface surface){
    WGPUDevice device = surface->device;
    uint32_t cacheIndex = surface->device->submittedFrames % framesInFlight;

    
    VkImageSubresourceRange isr zeroinit;
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;

    
    VkCommandBuffer transitionBuffer = surface->device->frameCaches[cacheIndex].finalTransitionBuffer;
    
    VkCommandBufferBeginInfo transitionBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    device->functions.vkBeginCommandBuffer(transitionBuffer, &transitionBufferBeginInfo);

    VkImageMemoryBarrier finalBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        NULL,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        surface->device->adapter->queueIndices.graphicsIndex,
        surface->device->adapter->queueIndices.graphicsIndex,
        surface->images[surface->activeImageIndex]->image,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0, VK_REMAINING_MIP_LEVELS,
            0, VK_REMAINING_ARRAY_LAYERS
        }
    };
    device->functions.vkCmdPipelineBarrier(
        transitionBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, NULL,
        0, NULL,
        1, &finalBarrier  
    );
    surface->images[surface->activeImageIndex]->layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    device->functions.vkEndCommandBuffer(transitionBuffer);
    VkPipelineStageFlags wsmask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    const VkSubmitInfo cbsinfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &transitionBuffer,
        .signalSemaphoreCount = 1,
        .waitSemaphoreCount = 1,
        .pWaitDstStageMask = &wsmask,
        .pWaitSemaphores = &surface->device->queue->syncState[cacheIndex].semaphores.data[surface->device->queue->syncState[cacheIndex].submits],
        .pSignalSemaphores = surface->presentSemaphores + surface->activeImageIndex
    };
    
    WGPUFence finalTransitionFence = surface->device->frameCaches[cacheIndex].finalTransitionFence;
    wgpuFenceAddRef(finalTransitionFence);
    device->functions.vkQueueSubmit(surface->device->queue->graphicsQueue, 1, &cbsinfo, finalTransitionFence->fence);
    finalTransitionFence->state = WGPUFenceState_InUse;
    
    WGPUCommandBufferVector* cmdBuffers = PendingCommandBufferMap_get(&surface->device->queue->pendingCommandBuffers[cacheIndex], (void*)finalTransitionFence);
    
    if(cmdBuffers == NULL){
        WGPUCommandBufferVector insert = {0};
        PendingCommandBufferMap_put(&surface->device->queue->pendingCommandBuffers[cacheIndex], finalTransitionFence, insert);
        cmdBuffers = PendingCommandBufferMap_get(&surface->device->queue->pendingCommandBuffers[cacheIndex], (void*)finalTransitionFence);
        WGPUCommandBufferVector_init(cmdBuffers);
    }

    VkPresentInfoKHR presentInfo  = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = surface->presentSemaphores + surface->activeImageIndex,
        .swapchainCount = 1,
        .pSwapchains = &surface->swapchain,
        .pImageIndices = &surface->activeImageIndex,
    };
    VkResult presentRes = device->functions.vkQueuePresentKHR(surface->device->queue->presentQueue, &presentInfo);

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
    if(presentRes != VK_SUCCESS){
        wgpuSurfaceConfigure(surface, &surface->lastConfig);
    }
    wgpuDeviceTick(surface->device);
}
void wgpuDeviceTick(WGPUDevice device){
    WGPUQueue queue = device->queue;
    WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(queue->presubmitCache);
    wgpuCommandEncoderRelease(queue->presubmitCache);
    wgpuCommandBufferRelease(buffer);
    
    ++device->submittedFrames;
    vmaSetCurrentFrameIndex(device->allocator, device->submittedFrames % framesInFlight);
    
    
    

    uint32_t cacheIndex = device->submittedFrames % framesInFlight;
    PendingCommandBufferMap* pcm = &queue->pendingCommandBuffers[cacheIndex];
    size_t pcmSize = pcm->current_size;

    WGPUFenceVector fences;
    WGPUFenceVector_init(&fences);

    if(pcm->current_size > 0){
        PendingCommandBufferMap_for_each(pcm, pcmNonnullFlattenCallback, (void*)&fences);
        if(fences.size > 0){
            wgpuFencesWait(fences.data, fences.size, UINT32_MAX);
        }
    }
    else{
        TRACELOG(LOG_INFO, "No fences!");
    }

    PendingCommandBufferMap_for_each(pcm, resetFenceAndReleaseBuffers, device);    
    WGPUFenceVector_free(&fences);

    WGPUBufferVector* usedBuffers = &device->frameCaches[cacheIndex].usedBatchBuffers;
    WGPUBufferVector* unusedBuffers = &device->frameCaches[cacheIndex].unusedBatchBuffers;
    if(unusedBuffers->capacity < unusedBuffers->size + usedBuffers->size){
        size_t newcap = (unusedBuffers->size + usedBuffers->size);
        WGPUBufferVector_reserve(unusedBuffers, newcap);
    }
    if(usedBuffers->size > 0){
        memcpy(unusedBuffers->data + unusedBuffers->size, usedBuffers->data, usedBuffers->size * sizeof(WGPUBuffer));
    }
    unusedBuffers->size += usedBuffers->size;
    WGPUBufferVector_clear(usedBuffers);//(WGPUBufferVector *dest, const WGPUBufferVector *source)
    PendingCommandBufferMap_clear(&queue->pendingCommandBuffers[cacheIndex]);
    device->functions.vkResetCommandPool(device->device, device->frameCaches[device->submittedFrames % framesInFlight].commandPool, 0);
    
    
    WGPUCommandEncoderDescriptor cedesc zeroinit;
    device->queue->presubmitCache = wgpuDeviceCreateCommandEncoder(device, &cedesc);
    queue->syncState[cacheIndex].submits = 0;
}
WGPUSampler wgpuDeviceCreateSampler(WGPUDevice device, const WGPUSamplerDescriptor* descriptor){
    WGPUSampler ret = callocnew(WGPUSamplerImpl);
    ret->refCount = 1;
    
    const VkSamplerCreateInfo sci = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .compareEnable = VK_FALSE,
        .maxLod = descriptor->lodMaxClamp,
        .minLod = descriptor->lodMinClamp,
        .addressModeU = toVulkanAddressMode(descriptor->addressModeU),
        .addressModeV = toVulkanAddressMode(descriptor->addressModeV),
        .addressModeW = toVulkanAddressMode(descriptor->addressModeW),
        .mipmapMode = ((descriptor->mipmapFilter == WGPUMipmapFilterMode_Linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST),
        .anisotropyEnable = false,
        .maxAnisotropy = descriptor->maxAnisotropy,
        .magFilter = ((descriptor->magFilter == WGPUFilterMode_Linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST),
        .minFilter = ((descriptor->minFilter == WGPUFilterMode_Linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST),
    };
    VkResult result = device->functions.vkCreateSampler(device->device, &sci, NULL, &(ret->sampler));
    return ret;
}
void wgpuRenderPassEncoderSetVertexBuffer(WGPURenderPassEncoder rpe, uint32_t binding, WGPUBuffer buffer, VkDeviceSize offset) {
    rassert(rpe != NULL, "RenderPassEncoderHandle is null");
    rassert(buffer != NULL, "BufferHandle is null");
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_vertex_buffer,
        .setVertexBuffer = {
            binding,
            buffer,
            offset
        }
    };

    RenderPassEncoder_PushCommand(rpe, &insert);
    
    ce_trackBuffer(rpe->cmdEncoder, buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_VERTEX_INPUT_BIT    , VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT});
}
void wgpuRenderPassEncoderDrawIndexedIndirect(WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE{
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_draw_indexed_indirect,
        .drawIndexedIndirect = {
            indirectBuffer,
            indirectOffset
        }
    };
}
void wgpuRenderPassEncoderDrawIndirect           (WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE{
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_draw_indirect,
        .drawIndirect = {
            indirectBuffer,
            indirectOffset
        }
    };
}
void wgpuRenderPassEncoderSetBlendConstant       (WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color) WGPU_FUNCTION_ATTRIBUTE{
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_blend_constant,
        .setBlendConstant = {
            *color
        }
    };
}
void wgpuRenderPassEncoderSetViewport            (WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth) WGPU_FUNCTION_ATTRIBUTE{
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_viewport,
        .setViewport = {
            x,y,width,height,minDepth,maxDepth
        }
    };
}

void wgpuRenderPassEncoderSetIndexBuffer(WGPURenderPassEncoder rpe, WGPUBuffer buffer, VkDeviceSize offset, WGPUIndexFormat indexType){
    rassert(rpe != NULL, "RenderPassEncoderHandle is null");
    rassert(buffer != NULL, "BufferHandle is null");

    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_index_buffer,
        .setIndexBuffer = {
            .buffer = buffer,
            .format = indexType,
            .offset = offset,
            .size = buffer->capacity
        }
    };

    RenderPassEncoder_PushCommand(rpe, &insert);
    
    ce_trackBuffer(rpe->cmdEncoder, buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT});
}
void wgpuTextureViewAddRef(WGPUTextureView textureView){
    ++textureView->refCount;
}
void wgpuTextureAddRef(WGPUTexture texture){
    ++texture->refCount;
}
void wgpuBufferAddRef(WGPUBuffer buffer){
    ++buffer->refCount;
}
void wgpuBindGroupAddRef(WGPUBindGroup bindGroup){
    ++bindGroup->refCount;
}
void wgpuBindGroupLayoutAddRef(WGPUBindGroupLayout bindGroupLayout){
    ++bindGroupLayout->refCount;
}
void wgpuPipelineLayoutAddRef(WGPUPipelineLayout pipelineLayout){
    ++pipelineLayout->refCount;
}
void wgpuSamplerAddRef(WGPUSampler sampler){
    ++sampler->refCount;
}
void wgpuRenderPipelineAddRef(WGPURenderPipeline rpl){
    ++rpl->refCount;
}
void wgpuComputePipelineAddRef(WGPUComputePipeline cpl){
    ++cpl->refCount;
}
void wgpuShaderModuleAddRef(WGPUShaderModule module){
    ++module->refCount;
}

RGAPI void ru_trackBuffer(ResourceUsage* resourceUsage, WGPUBuffer buffer, BufferUsageRecord brecord){
    if(BufferUsageRecordMap_put(&resourceUsage->referencedBuffers, (void*)buffer, brecord)){
        ++buffer->refCount;
    }
}

RGAPI void ru_trackTexture(ResourceUsage* resourceUsage, WGPUTexture texture, ImageUsageRecord record){
    if(ImageUsageRecordMap_put(&resourceUsage->referencedTextures, texture, record)){
        ++texture->refCount;
    }
}

RGAPI void ru_trackTextureView     (ResourceUsage* resourceUsage, WGPUTextureView view){
    if(ImageViewUsageSet_add(&resourceUsage->referencedTextureViews, view)){
        ++view->refCount;
    }
}
RGAPI void ru_trackRenderPipeline(ResourceUsage* resourceUsage, WGPURenderPipeline renderPipeline){
    if(RenderPipelineUsageSet_add(&resourceUsage->referencedRenderPipelines, renderPipeline)){
        ++renderPipeline->refCount;
    }
}
RGAPI void ru_trackComputePipeline(ResourceUsage* resourceUsage, WGPUComputePipeline computePipeline){
    if(ComputePipelineUsageSet_add(&resourceUsage->referencedComputePipelines, computePipeline)){
        ++computePipeline->refCount;
    }
}
RGAPI void ru_trackBindGroup(ResourceUsage* resourceUsage, WGPUBindGroup bindGroup){
    if(BindGroupUsageSet_add(&resourceUsage->referencedBindGroups, (void*)bindGroup)){
        ++bindGroup->refCount;
    }
}

RGAPI void ru_trackBindGroupLayout (ResourceUsage* resourceUsage, WGPUBindGroupLayout bindGroupLayout){
    if(BindGroupLayoutUsageSet_add(&resourceUsage->referencedBindGroupLayouts, bindGroupLayout)){
        ++bindGroupLayout->refCount;
    }
}

RGAPI void ru_trackSampler         (ResourceUsage* resourceUsage, WGPUSampler sampler){
    if(SamplerUsageSet_add(&resourceUsage->referencedSamplers, sampler)){
        ++sampler->refCount;
    }
}

RGAPI void ce_trackTexture(WGPUCommandEncoder encoder, WGPUTexture texture, ImageUsageSnap usage){
    WGPU_VALIDATE_EQ_PTR(encoder->device, texture->device, encoder->device, "failed when validating devices");
    ImageUsageRecord* alreadyThere = ImageUsageRecordMap_get(&encoder->resourceUsage.referencedTextures, texture);
    if(alreadyThere){
        VkImageMemoryBarrier imageBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            NULL,
            alreadyThere->lastAccess,
            usage.access,
            alreadyThere->lastLayout,
            usage.layout,
            encoder->device->adapter->queueIndices.graphicsIndex,
            encoder->device->adapter->queueIndices.graphicsIndex,
            texture->image,
            usage.subresource
        };
        encoder->device->functions.vkCmdPipelineBarrier(
            encoder->buffer, 
            alreadyThere->lastStage, 
            usage.stage,
            0, 
            0, NULL, 
            0, NULL, 
            1, &imageBarrier
        );
        alreadyThere->lastStage  = usage.stage;
        alreadyThere->lastAccess = usage.access;
        alreadyThere->lastLayout = usage.layout;
    }
    else{
        int newEntry = ImageUsageRecordMap_put(&encoder->resourceUsage.referencedTextures, texture, CLITERAL(ImageUsageRecord){
            usage.layout,
            usage.layout,
            usage.stage,
            usage.access,
            usage.subresource
        });
        rassert(newEntry != 0, "_get failed, but _put did not return 1");
        if(newEntry)
            ++texture->refCount;
    }
}

RGAPI void ce_trackTextureView(WGPUCommandEncoder enc, WGPUTextureView view, ImageUsageSnap usage){
    
    ru_trackTextureView(&enc->resourceUsage, view);
    
    ImageUsageRecord* alreadyThere = ImageUsageRecordMap_get(&enc->resourceUsage.referencedTextures, view->texture);
    rassert(usage.stage, "Stage mask cannot be empty");
    if(alreadyThere != NULL){
        VkImageMemoryBarrier imageBarrier = {
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            NULL,
            alreadyThere->lastAccess,
            usage.access,
            alreadyThere->lastLayout,
            usage.layout, 
            enc->device->adapter->queueIndices.graphicsIndex,
            enc->device->adapter->queueIndices.graphicsIndex,
            view->texture->image,
            view->subresourceRange
        };
        enc->device->functions.vkCmdPipelineBarrier(
            enc->buffer,
            alreadyThere->lastStage, 
            usage.stage,
            0, 
            0, NULL, 
            0, NULL, 
            1, &imageBarrier
        );
        alreadyThere->lastStage  = usage.stage;
        alreadyThere->lastAccess = usage.access;
        alreadyThere->lastLayout = usage.layout;
    }
    else{
        int newEntry = ImageUsageRecordMap_put(&enc->resourceUsage.referencedTextures, view->texture, CLITERAL(ImageUsageRecord){
            usage.layout,
            usage.layout,
            usage.stage,
            usage.access,
            view->subresourceRange
        });
        rassert(newEntry != 0, "_get failed, but _put did not return 1");
        if(newEntry)
            ++view->texture->refCount;
    }
}
RGAPI void ce_trackBuffer(WGPUCommandEncoder encoder, WGPUBuffer buffer, BufferUsageSnap usage){
    BufferUsageRecord* rec = BufferUsageRecordMap_get(&encoder->resourceUsage.referencedBuffers, buffer);
    if(rec != NULL){
        VkBufferMemoryBarrier bufferBarrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            NULL,
            rec->lastAccess,
            usage.access, 
            encoder->device->adapter->queueIndices.graphicsIndex,
            encoder->device->adapter->queueIndices.graphicsIndex,
            buffer->buffer,
            0,
            VK_WHOLE_SIZE
        };

        encoder->device->functions.vkCmdPipelineBarrier(
            encoder->buffer,
            rec->lastStage,
            usage.stage,
            0,
            0, 0,
            1, &bufferBarrier,
            0, 0
        );
        rec->lastAccess = usage.access;
        rec->lastStage = usage.stage;
        rec->everWrittenTo |= isWritingAccess(usage.access);
    }
    else{
        BufferUsageRecord record = {
            usage.stage,
            usage.access,
            isWritingAccess(usage.access)
        };
        ru_trackBuffer(&encoder->resourceUsage, buffer, record);
    }

}


static inline void bufferReleaseCallback(void* buffer, BufferUsageRecord* bu_record, void* unused){
    wgpuBufferRelease(buffer);
}
static inline void textureReleaseCallback(void* texture, ImageUsageRecord* iur, void* unused){
    (void)unused;
    wgpuTextureRelease((WGPUTexture)texture);
}
static inline void textureViewReleaseCallback(WGPUTextureView textureView, void* unused){
    (void)unused;
    wgpuTextureViewRelease(textureView);
}
static inline void bindGroupReleaseCallback(WGPUBindGroup bindGroup, void* unused){
    wgpuBindGroupRelease(bindGroup);
}
static inline void bindGroupLayoutReleaseCallback(WGPUBindGroupLayout bgl, void* unused){
    wgpuBindGroupLayoutRelease(bgl);
}
static inline void samplerReleaseCallback(WGPUSampler sampler, void* unused){
    wgpuSamplerRelease(sampler);
}
static inline void computePipelineReleaseCallback(WGPUComputePipeline computePipeline, void* unused){
    wgpuComputePipelineRelease(computePipeline);
}
static inline void renderPipelineReleaseCallback(WGPURenderPipeline renderPipeline, void* unused){
    wgpuRenderPipelineRelease(renderPipeline);
}



static WGPUTextureViewDimension spvDimToWGPUViewDimension(SpvDim dim, bool arrayed) {
    switch (dim) {
        case SpvDim1D: return WGPUTextureViewDimension_1D; // WebGPU doesn't have 1D array textures
        case SpvDim2D: return arrayed ? WGPUTextureViewDimension_2DArray : WGPUTextureViewDimension_2D;
        case SpvDim3D: return WGPUTextureViewDimension_3D; // 3D textures cannot be arrayed
        case SpvDimCube: return arrayed ? WGPUTextureViewDimension_CubeArray : WGPUTextureViewDimension_Cube;
        default: return WGPUTextureViewDimension_Undefined;
    }
}

static WGPUTextureSampleType spvImageTypeToWGPUSampleType(const SpvImageFormat type_desc, bool is_depth_image) {
    if (is_depth_image) {
        return WGPUTextureSampleType_Depth;
    }
    //if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
    //    // SPIRV-Reflect doesn't directly distinguish filterable vs unfilterable for float.
    //    // This often depends on the format and usage. Assume filterable for simplicity here.
    //    // You might need more sophisticated logic based on SpvImageFormat.
    //    return WGPUTextureSampleType_Float;
    //}
    //if (type_desc->type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
    //    if (type_desc->traits.numeric.scalar.signedness) {
    //        return WGPUTextureSampleType_Sint;
    //    } else {
    //        return WGPUTextureSampleType_Uint;
    //    }
    //}
    return WGPUTextureSampleType_Float;
}

static WGPUTextureFormat spvImageFormatToWGPUFormat(SpvImageFormat spv_format) {
    switch (spv_format) {
        case SpvImageFormatRgba32f:         return WGPUTextureFormat_RGBA32Float;
        case SpvImageFormatRgba16f:         return WGPUTextureFormat_RGBA16Float;
        case SpvImageFormatR32f:            return WGPUTextureFormat_R32Float;
        case SpvImageFormatRgba8:           return WGPUTextureFormat_RGBA8Unorm;
        case SpvImageFormatRgba8Snorm:      return WGPUTextureFormat_RGBA8Snorm;
        case SpvImageFormatRg32f:           return WGPUTextureFormat_RG32Float;
        case SpvImageFormatRg16f:           return WGPUTextureFormat_RG16Float;
        case SpvImageFormatR11fG11fB10f:    return WGPUTextureFormat_RG11B10Ufloat;
        case SpvImageFormatR16f:            return WGPUTextureFormat_R16Float;
        case SpvImageFormatRgba16:          return WGPUTextureFormat_Undefined; // WGPUTextureFormat_RGBA16Unorm missing
        case SpvImageFormatRgb10A2:         return WGPUTextureFormat_RGB10A2Unorm;
        case SpvImageFormatRg16:            return WGPUTextureFormat_Undefined; // WGPUTextureFormat_RG16Unorm missing
        case SpvImageFormatRg8:             return WGPUTextureFormat_RG8Unorm;
        case SpvImageFormatR16:             return WGPUTextureFormat_Undefined; // WGPUTextureFormat_R16Unorm missing
        case SpvImageFormatR8:              return WGPUTextureFormat_R8Unorm;
        case SpvImageFormatRgba16Snorm:     return WGPUTextureFormat_Undefined; // WGPUTextureFormat_RGBA16Snorm missing
        case SpvImageFormatRg16Snorm:       return WGPUTextureFormat_Undefined; // WGPUTextureFormat_RG16Snorm missing
        case SpvImageFormatRg8Snorm:        return WGPUTextureFormat_RG8Snorm;
        case SpvImageFormatR16Snorm:        return WGPUTextureFormat_Undefined; // WGPUTextureFormat_R16Snorm missing
        case SpvImageFormatR8Snorm:         return WGPUTextureFormat_R8Snorm;
        case SpvImageFormatRgba32i:         return WGPUTextureFormat_RGBA32Sint;
        case SpvImageFormatRgba16i:         return WGPUTextureFormat_RGBA16Sint;
        case SpvImageFormatRgba8i:          return WGPUTextureFormat_RGBA8Sint;
        case SpvImageFormatR32i:            return WGPUTextureFormat_R32Sint;
        case SpvImageFormatRg32i:           return WGPUTextureFormat_RG32Sint;
        case SpvImageFormatRg16i:           return WGPUTextureFormat_RG16Sint;
        case SpvImageFormatRg8i:            return WGPUTextureFormat_RG8Sint;
        case SpvImageFormatR16i:            return WGPUTextureFormat_R16Sint;
        case SpvImageFormatR8i:             return WGPUTextureFormat_R8Sint;
        case SpvImageFormatRgba32ui:        return WGPUTextureFormat_RGBA32Uint;
        case SpvImageFormatRgba16ui:        return WGPUTextureFormat_RGBA16Uint;
        case SpvImageFormatRgba8ui:         return WGPUTextureFormat_RGBA8Uint;
        case SpvImageFormatR32ui:           return WGPUTextureFormat_R32Uint;
        case SpvImageFormatRgb10a2ui:       return WGPUTextureFormat_RGB10A2Uint;
        case SpvImageFormatRg32ui:          return WGPUTextureFormat_RG32Uint;
        case SpvImageFormatRg16ui:          return WGPUTextureFormat_RG16Uint;
        case SpvImageFormatRg8ui:           return WGPUTextureFormat_RG8Uint;
        case SpvImageFormatR16ui:           return WGPUTextureFormat_R16Uint;
        case SpvImageFormatR8ui:            return WGPUTextureFormat_R8Uint;
        case SpvImageFormatR64ui:           return WGPUTextureFormat_Undefined; // No 64-bit formats in WGPUTextureFormat
        case SpvImageFormatR64i:            return WGPUTextureFormat_Undefined; // No 64-bit formats in WGPUTextureFormat
        case SpvImageFormatUnknown:
        // SpvImageFormatMax is not typically used as an input format value
        default:
            return WGPUTextureFormat_Undefined;
    }
}

static WGPUStorageTextureAccess getStorageTextureAccess(const SpvReflectDescriptorBinding* entry) {
    bool non_readable = (entry->decoration_flags & SPV_REFLECT_DECORATION_NON_READABLE);
    bool non_writable = (entry->decoration_flags & SPV_REFLECT_DECORATION_NON_WRITABLE);

    if (non_readable && !non_writable) { // WriteOnly
        return WGPUStorageTextureAccess_WriteOnly;
    } else if (!non_readable && non_writable) { // ReadOnly
        return WGPUStorageTextureAccess_ReadOnly;
    } else if (!non_readable && !non_writable) { // ReadWrite
        // SPIR-V default for storage image is read_write.
        // WebGPU requires explicit access. If using for typical compute output, WriteOnly is common.
        // If truly read-write is intended and supported by format, this could be ReadWrite.
        // For safety and common WebGPU patterns, often WriteOnly is expected for storage textures unless specified.
        // However, based on SPIR-V, ReadWrite is more accurate if no flags.
        // Let's assume a choice that aligns with user's previous note:
        // "return WGPUStorageTextureAccess_WriteOnly; // Common default for WebGPU pipeline creation."
        return WGPUStorageTextureAccess_WriteOnly;
    } else { // non_readable && non_writable
        rassert(false, "Storage image cannot be both non-readable and non-writable");
        return WGPUStorageTextureAccess_Undefined;
    }
}

WGPUGlobalReflectionInfo* getGlobalRI(SpvReflectShaderModule mod, uint32_t* count){
    rassert(count != NULL, "count may not be NULL here");
    *count = 0;

    uint32_t descriptorSetCount = 0;
    SpvReflectResult result = spvReflectEnumerateDescriptorSets(&mod, &descriptorSetCount, NULL);
    rassert(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate descriptor sets (count)");
    if (descriptorSetCount == 0) {
        return NULL;
    }

    SpvReflectDescriptorSet** descriptorSets = (SpvReflectDescriptorSet**)RL_CALLOC(descriptorSetCount, sizeof(SpvReflectDescriptorSet*));
    rassert(descriptorSets != NULL, "Failed to allocate memory for descriptor set pointers");
    result = spvReflectEnumerateDescriptorSets(&mod, &descriptorSetCount, descriptorSets);
    rassert(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to enumerate descriptor sets (pointers)");
    
    uint32_t totalGlobalCount = 0;
    for(uint32_t i = 0; i < descriptorSetCount; i++){
        totalGlobalCount += descriptorSets[i]->binding_count;
    }

    if (totalGlobalCount == 0) {
        RL_FREE((void*)descriptorSets); // Free the pointers array if no bindings.
        return NULL;
    }

    WGPUGlobalReflectionInfo* allGlobals = (WGPUGlobalReflectionInfo*)RL_CALLOC(totalGlobalCount, sizeof(WGPUGlobalReflectionInfo));
    rassert(allGlobals != NULL, "Failed to allocate memory for global reflection info array");
    *count = totalGlobalCount;

    uint32_t globalInsertIndex = 0;
    for(uint32_t bindGroupIndex = 0; bindGroupIndex < descriptorSetCount; bindGroupIndex++){
        const SpvReflectDescriptorSet* set = descriptorSets[bindGroupIndex];
        for(uint32_t entryIndex = 0; entryIndex < set->binding_count; entryIndex++){
            const SpvReflectDescriptorBinding* entry = set->bindings[entryIndex];
            WGPUGlobalReflectionInfo insert = {0}; // Initialize to zero

            insert.bindGroup = set->set; // Use the actual set number from reflection
            insert.binding = entry->binding;
            
            const char* entry_name_ptr = entry->type_description->type_name ? entry->type_description->type_name : (entry->name ? entry->name : "");
            insert.name.data = entry_name_ptr;
            insert.name.length = strlen(entry_name_ptr);
            // insert.stageFlags = entry->shader_stage_flags; // If WGPUGlobalReflectionInfo has stage flags

            switch(entry->descriptor_type){    
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    // insert.resourceType = WGPUBindingResourceType_Buffer; // If using a type enum for a union
                    insert.buffer.type = WGPUBufferBindingType_Uniform;
                    insert.buffer.minBindingSize = entry->block.size; // Size of the UBO block
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    // insert.resourceType = WGPUBindingResourceType_Buffer;
                    insert.buffer.type = WGPUBufferBindingType_Storage; // Or ReadOnlyStorage if distinguishable and needed
                    insert.buffer.minBindingSize = entry->block.size; // Size of the SSBO block
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                    // insert.resourceType = WGPUBindingResourceType_Sampler;
                    insert.sampler.type = WGPUSamplerBindingType_Filtering; // Default, see notes
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    // insert.resourceType = WGPUBindingResourceType_Texture;
                    insert.texture.sampleType = spvImageTypeToWGPUSampleType(entry->image.image_format, (entry->image.depth == 1));
                    insert.texture.viewDimension = spvDimToWGPUViewDimension(entry->image.dim, entry->image.arrayed);
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    // insert.resourceType = WGPUBindingResourceType_StorageTexture;
                    insert.storageTexture.access = getStorageTextureAccess(entry);
                    insert.storageTexture.format = spvImageFormatToWGPUFormat(entry->image.image_format);
                    insert.storageTexture.viewDimension = spvDimToWGPUViewDimension(entry->image.dim, entry->image.arrayed);
                    break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    // WebGPU requires separate texture and sampler.
                    // This reflection info might need to create two entries or have a special type.
                    // For now, matches your original rassert.
                    rassert(false, "SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER not handled/allowed.");
                    break;
                // Handle other types like SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT if necessary
                default:
                    rassert(false, "Unhandled or illegal descriptor type in SPIR-V reflection.");
            }
            allGlobals[globalInsertIndex++] = insert;
        }
    }
    
    RL_FREE((void*)descriptorSets); // Free the array of pointers (not the content they point to)
    return allGlobals;
}


static inline WGPUShaderStage toWGPUShaderStage(SpvReflectShaderStageFlagBits stageBits){
    WGPUShaderStage ret = 0;

    if(stageBits & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT){
        ret |= WGPUShaderStage_Vertex;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT){
        ret |= WGPUShaderStage_TessControl;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT){
        ret |= WGPUShaderStage_TessEvaluation;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT){
        ret |= WGPUShaderStage_Geometry;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT){
        ret |= WGPUShaderStage_Fragment;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT){
        ret |= WGPUShaderStage_Compute;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV){
        ret |= WGPUShaderStage_Task;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_TASK_BIT_EXT){
        ret |= WGPUShaderStage_Task;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV){
        ret |= WGPUShaderStage_Mesh;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_MESH_BIT_EXT){
        ret |= WGPUShaderStage_Mesh;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR){
        ret |= WGPUShaderStage_RayGen;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR){
        ret |= WGPUShaderStage_AnyHit;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR){
        ret |= WGPUShaderStage_ClosestHit;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR){
        ret |= WGPUShaderStage_Miss;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR){
        ret |= WGPUShaderStage_Intersect;
    }
    if(stageBits & SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR){
        ret |= WGPUShaderStage_Callable;
    }
    return ret;
}

struct wgpuShaderModuleGetReflectionInfo_sync_userdata{
    WGPUShaderModule module;
    WGPUReflectionInfoCallbackInfo callbackInfo;
};
WGPUReflectionAttribute spvReflectToWGPUReflectAttrib(SpvReflectInterfaceVariable* spvAttrib){
    
    WGPUReflectionAttribute result = {0};
    result.location = spvAttrib->location;
    switch(spvAttrib->format){
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
            result.compositionType = WGPUReflectionCompositionType_Scalar;
            result.componentType = WGPUReflectionComponentType_Float32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            result.compositionType = WGPUReflectionCompositionType_Vec2;
            result.componentType = WGPUReflectionComponentType_Float32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            result.compositionType = WGPUReflectionCompositionType_Vec3;
            result.componentType = WGPUReflectionComponentType_Float32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            result.compositionType = WGPUReflectionCompositionType_Vec4;
            result.componentType = WGPUReflectionComponentType_Float32;
            break;
        case SPV_REFLECT_FORMAT_R32_UINT:
            result.compositionType = WGPUReflectionCompositionType_Scalar;
            result.componentType = WGPUReflectionComponentType_Uint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_UINT:
            result.compositionType = WGPUReflectionCompositionType_Vec2;
            result.componentType = WGPUReflectionComponentType_Uint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:
            result.compositionType = WGPUReflectionCompositionType_Vec3;
            result.componentType = WGPUReflectionComponentType_Uint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
            result.compositionType = WGPUReflectionCompositionType_Vec4;
            result.componentType = WGPUReflectionComponentType_Uint32;
            break;
        case SPV_REFLECT_FORMAT_R32_SINT:
            result.compositionType = WGPUReflectionCompositionType_Scalar;
            result.componentType = WGPUReflectionComponentType_Sint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SINT:
            result.compositionType = WGPUReflectionCompositionType_Vec2;
            result.componentType = WGPUReflectionComponentType_Sint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:
            result.compositionType = WGPUReflectionCompositionType_Vec3;
            result.componentType = WGPUReflectionComponentType_Sint32;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
            result.compositionType = WGPUReflectionCompositionType_Vec4;
            result.componentType = WGPUReflectionComponentType_Sint32;
            break;
        default: break; //rassert(false, "Unhandled Spirv-reflect attribute type");
    }
    return result;
}
static void wgpuShaderModuleGetReflectionInfo_sync(void* userdata_){
    struct wgpuShaderModuleGetReflectionInfo_sync_userdata* userdata = (struct wgpuShaderModuleGetReflectionInfo_sync_userdata*)userdata_;
    WGPUShaderModule module = userdata->module;
    
    rassert(module,         "shaderModule is NULL");
    rassert(module->source, "shaderModule->source is NULL");

    switch(module->source->sType){
        case WGPUSType_ShaderSourceSPIRV:{
            WGPUShaderSourceSPIRV* spirvSource = (WGPUShaderSourceSPIRV*)module->source;

            SpvReflectShaderModule mod zeroinit;
            SpvReflectResult result = spvReflectCreateShaderModule(spirvSource->codeSize, spirvSource->code, &mod);
            
            if(result == SPV_REFLECT_RESULT_SUCCESS){
                SpvReflectDescriptorSet* descriptorSets = NULL;
                
                WGPUReflectionInfo reflectionInfo zeroinit;
                reflectionInfo.globals = getGlobalRI(mod,  &reflectionInfo.globalCount);
                
                SpvReflectInterfaceVariable** input_vars = NULL;
                SpvReflectInterfaceVariable** output_vars = NULL;

                uint32_t input_var_count = 0;
                uint32_t output_var_count = 0;
                spvReflectEnumerateInputVariables(&mod, &input_var_count, NULL);
                spvReflectEnumerateOutputVariables(&mod, &output_var_count, NULL);
                input_vars = (SpvReflectInterfaceVariable**)RL_CALLOC(input_var_count, sizeof(uintptr_t));
                output_vars = (SpvReflectInterfaceVariable**)RL_CALLOC(output_var_count, sizeof(uintptr_t));

                spvReflectEnumerateInputVariables(&mod, &input_var_count, input_vars);
                spvReflectEnumerateOutputVariables(&mod, &output_var_count, output_vars);
                
                WGPUAttributeReflectionInfo input_attribute_info = {0};
                WGPUAttributeReflectionInfo output_attribute_info = {0};

                input_attribute_info.attributeCount = input_var_count;
                output_attribute_info.attributeCount = output_var_count;
                input_attribute_info.attributes = RL_CALLOC(input_attribute_info.attributeCount, sizeof(WGPUReflectionAttribute));
                output_attribute_info.attributes = RL_CALLOC(output_attribute_info.attributeCount, sizeof(WGPUReflectionAttribute));
                
                for(uint32_t i = 0;i < input_var_count;i++){
                    SpvReflectInterfaceVariable* input_var_i = input_vars[i];
                    input_attribute_info.attributes[i] = spvReflectToWGPUReflectAttrib(input_vars[i]);
                }
                for(uint32_t i = 0;i < output_var_count;i++){
                    SpvReflectInterfaceVariable* output_var_i = output_vars[i];
                    output_attribute_info.attributes[i] = spvReflectToWGPUReflectAttrib(output_vars[i]);
                }

                reflectionInfo.inputAttributes = &input_attribute_info;

                userdata->callbackInfo.callback(
                    WGPUReflectionInfoRequestStatus_Success,
                    &reflectionInfo,
                    userdata->callbackInfo.userdata1,
                    userdata->callbackInfo.userdata2
                );

                RL_FREE(descriptorSets);
                RL_FREE(input_attribute_info.attributes);
                RL_FREE(output_attribute_info.attributes);
                RL_FREE((void*)input_vars);
                RL_FREE((void*)output_vars);
                RL_FREE((void*)reflectionInfo.globals);
                spvReflectDestroyShaderModule(&mod);
            }
        }
        break;
        case WGPUSType_ShaderSourceWGSL:{
            WGPUShaderSourceWGSL* wgslSource = (WGPUShaderSourceWGSL*)module->source;
            WGPUReflectionInfo reflectionInfo = reflectionInfo_wgsl_sync(wgslSource->code);
            userdata->callbackInfo.callback(
                WGPUReflectionInfoRequestStatus_Success,
                &reflectionInfo,
                userdata->callbackInfo.userdata1,
                userdata->callbackInfo.userdata2
            );
            reflectionInfo_wgsl_free(&reflectionInfo);
        }break;
        default:
        rassert(false, "Invalid sType for source");
        rg_unreachable();
    }
    
}



WGPUFuture wgpuShaderModuleGetReflectionInfo(WGPUShaderModule shaderModule, WGPUReflectionInfoCallbackInfo callbackInfo){
    struct wgpuShaderModuleGetReflectionInfo_sync_userdata* udff = RL_CALLOC(1, sizeof(struct wgpuShaderModuleGetReflectionInfo_sync_userdata));
    WGPUInstance instance = shaderModule->device->adapter->instance;
    udff->callbackInfo = callbackInfo;
    udff->module = shaderModule;
    WGPUFutureImpl ret = {
        .functionCalledOnWaitAny = wgpuShaderModuleGetReflectionInfo_sync,
        .userdataForFunction = udff,
        .freeUserData = RL_FREE
    };
    WGPUFuture rete = { shaderModule->device->adapter->instance->currentFutureId++ };
    FutureIDMap_put(&instance->g_futureIDMap, rete.id, ret);
    return rete;
}








void releaseAllAndClear(ResourceUsage* resourceUsage){
    BufferUsageRecordMap_for_each(&resourceUsage->referencedBuffers, bufferReleaseCallback, NULL); 
    ImageUsageRecordMap_for_each(&resourceUsage->referencedTextures, textureReleaseCallback, NULL); 
    ImageViewUsageSet_for_each(&resourceUsage->referencedTextureViews, textureViewReleaseCallback, NULL); 
    BindGroupUsageSet_for_each(&resourceUsage->referencedBindGroups, bindGroupReleaseCallback, NULL);
    BindGroupLayoutUsageSet_for_each(&resourceUsage->referencedBindGroupLayouts, bindGroupLayoutReleaseCallback, NULL);
    SamplerUsageSet_for_each(&resourceUsage->referencedSamplers, samplerReleaseCallback, NULL);
    ComputePipelineUsageSet_for_each(&resourceUsage->referencedComputePipelines, computePipelineReleaseCallback, NULL);
    RenderPipelineUsageSet_for_each(&resourceUsage->referencedRenderPipelines, renderPipelineReleaseCallback, NULL);

    BufferUsageRecordMap_free(&resourceUsage->referencedBuffers);
    ImageUsageRecordMap_free(&resourceUsage->referencedTextures);
    ImageViewUsageSet_free(&resourceUsage->referencedTextureViews);
    BindGroupUsageSet_free(&resourceUsage->referencedBindGroups);
    BindGroupLayoutUsageSet_free(&resourceUsage->referencedBindGroupLayouts);
    SamplerUsageSet_free(&resourceUsage->referencedSamplers);
    ComputePipelineUsageSet_free(&resourceUsage->referencedComputePipelines);
    RenderPipelineUsageSet_free(&resourceUsage->referencedRenderPipelines);
}


const char* vkErrorString(int code){
    
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