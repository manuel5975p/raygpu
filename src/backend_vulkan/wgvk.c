//#include "vulkan/vulkan_core.h"
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
    #include <wgvk_structs_impl.h>
    #include <enum_translation.h>
    //#include "vulkan_internals.hpp"
    //#include <raygpu.h>
#undef Font
#ifdef TRACELOG
    #undef TRACELOG
#endif
#define TRACELOG(level, ...) wgvkTraceLog(level, __VA_ARGS__)
void wgvkTraceLog(int logType, const char *text, ...);
#include <macros_and_constants.h>
#include <wgvk.h>
#include <external/VmaUsage.h>
#include <stdarg.h>


// WGVK struct implementations

#include <wgvk_structs_impl.h>


static inline uint32_t findMemoryType(WGVKAdapter adapter, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    
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



WGVKSurface wgvkInstanceCreateSurface(WGVKInstance instance, const WGVKSurfaceDescriptor* descriptor){
    rassert(descriptor->nextInChain, "SurfaceDescriptor must have a nextInChain");
    WGVKSurface ret = callocnew(WGVKSurfaceImpl);

    switch(descriptor->nextInChain->sType){
        #if SUPPORT_METAL_SURFACE
        case WGVKSType_SurfaceSourceMetalLayer:{

        }break;
        #endif
        #if SUPPORT_WIN32_SURFACE
        case WGVKSType_SurfaceSourceWindowsHWND:{

        }break;
        #endif
        #if SUPPORT_XLIB_SURFACE
        case WGVKSType_SurfaceSourceXlibWindow:{
            WGVKSurfaceSourceXlibWindow* xlibSource = (WGVKSurfaceSourceXlibWindow*)descriptor->nextInChain;
            VkXlibSurfaceCreateInfoKHR sci zeroinit;
            sci.window = (uint64_t)xlibSource->window;
            sci.dpy = (Display*)xlibSource->display;
            vkCreateXlibSurfaceKHR(
                instance->instance,
                &sci,
                NULL,
                &ret->surface
            );
            
        }break;
        #endif

        #if SUPPORT_WAYLAND_SURFACE
        case WGVKSType_SurfaceSourceWaylandSurface:{
            WGVKSurfaceSourceWaylandSurface* waylandSource = (WGVKSurfaceSourceWaylandSurface*)descriptor->nextInChain;
            VkWaylandSurfaceCreateInfoKHR sci zeroinit;
            sci.surface = (wl_surface*)waylandSource->surface;
            sci.display = (wl_display*)waylandSource->display;
            vkCreateWaylandSurfaceKHR(
                instance->instance,
                &sci,
                NULL,
                &ret->surface
            );
        }break;
        #endif

        #if SUPPORT_ANDROID_SURFACE
        case WGVKSType_SurfaceSourceAndroidNativeWindow:{
            
        }break;
        #endif
        #if SUPPORT_XCB_SURFACE
        case WGVKSType_SurfaceSourceXCBWindow:{

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

RenderPassLayout GetRenderPassLayout(const WGVKRenderPassDescriptor* rpdesc){
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
    return fmt ==  Depth24 || fmt == Depth32;
}
static inline bool is__depthVk(VkFormat fmt){
    return fmt ==  VK_FORMAT_D32_SFLOAT || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT || fmt == VK_FORMAT_D24_UNORM_S8_UINT;
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
    ret.initialLayout  = (att.loadop == LoadOp_Load ? (is__depthVk(att.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) : (VK_IMAGE_LAYOUT_UNDEFINED));
    if(is__depthVk(att.format)){
        ret.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }else{
        ret.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    return ret;
};
LayoutedRenderPass LoadRenderPassFromLayout(WGVKDevice device, RenderPassLayout layout){
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
    VkResult result = vkCreateRenderPass(device->device, &rpci, NULL, &ret.renderPass);
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

void wgvkTraceLog(int logType, const char *text, ...){
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
        wgvkTraceLog(LOG_ERROR, pCallbackData->pMessage);
        rg_trap();
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT){
        wgvkTraceLog(LOG_WARNING, pCallbackData->pMessage);
    }
    else if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
        wgvkTraceLog(LOG_INFO, pCallbackData->pMessage);
    }

    return VK_FALSE;
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
WGVKInstance wgvkCreateInstance(const WGVKInstanceDescriptor* descriptor) {
#if SUPPORT_VULKAN_BACKEND == 0
    fprintf(stderr, "Vulkan backend is not enabled in this build.\n");
    return NULL;
#else

    WGVKInstance ret = (WGVKInstance)RL_CALLOC(1, sizeof(WGVKInstanceImpl));
    volkInitialize();
    if (!ret) {
        fprintf(stderr, "Failed to allocate memory for WGVKInstanceImpl\n");
        return NULL;
    }
    ret->instance = VK_NULL_HANDLE;
    ret->debugMessenger = VK_NULL_HANDLE;

    // 1. Define Application Info
    VkApplicationInfo appInfo zeroinit;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "WGVK Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "WGVK Engine";
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
    WGVKInstanceLayerSelection* ils = NULL;
    int debugUtilsAvailable = 0; // Check if debug utils was actually enabled

    for (uint32_t i = 0; i < enabledExtensionCount; ++i) {
        if (strcmp(enabledExtensions[i], VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
            debugUtilsAvailable = 1;
            break;
        }
    }

    if (descriptor && descriptor->nextInChain) {
        if (((WGVKChainedStruct*)descriptor->nextInChain)->sType == WGVKSType_InstanceValidationLayerSelection) {
            ils = (WGVKInstanceLayerSelection*)descriptor->nextInChain;
            requestedLayers = ils->instanceLayers;
            requestedLayerCount = ils->instanceLayerCount;
        }
        // TODO: Handle other potential structs in nextInChain if necessary
    }

    ici.enabledLayerCount = requestedLayerCount;
    ici.ppEnabledLayerNames = requestedLayers;

    // If layers are enabled, configure specific validation features, BUT only if debug utils is available
    if (requestedLayerCount > 0) {
        if(debugUtilsAvailable) {
            VkValidationFeatureEnableEXT enables[] = {
                VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT,
            };
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
    RL_FREE(availableExtensions); // Properties buffer
    RL_FREE(enabledExtensions);   // Names buffer (pointers into availableExtensions)
    availableExtensions = NULL;
    enabledExtensions = NULL;
    // --- End Freeing Extension Memory ---


    if (result != VK_SUCCESS) {
        RL_FREE(ret);
        return NULL;
    }
    
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
            fprintf(stderr, "Warning: Failed to create debug messenger (Error: %d).\n", (int)result);
            ret->debugMessenger = VK_NULL_HANDLE;
        } else {
            fprintf(stdout, "Vulkan Debug Messenger created successfully.\n");
        }
    } else if (requestedLayerCount > 0) {
         fprintf(stdout, "Debug messenger creation skipped because VK_EXT_debug_utils extension was not available/enabled or layers were disabled.\n");
    }


    // 8. Return the created instance handle
    return ret;

#endif // SUPPORT_VULKAN_BACKEND
}
WGVKWaitStatus wgvkInstanceWaitAny(WGVKInstance instance, size_t futureCount, WGVKFutureWaitInfo* futures, uint64_t timeoutNS){
    for(uint32_t i = 0;i < futureCount;i++){
        if(!futures[i].completed){
            futures[i].future->functionCalledOnWaitAny(futures[i].future->userdataForFunction);
            futures[i].completed = 1;
        }
    }
    return WGVKWaitStatus_Success;
}
typedef struct userdataforcreateadapter{
    WGVKInstance instance;
    WGVKRequestAdapterCallbackInfo info;
    WGVKRequestAdapterOptions options;
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
void wgvkCreateAdapter_impl(void* userdata_v){
    userdataforcreateadapter* userdata = (userdataforcreateadapter*)userdata_v;
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, NULL);
    VkPhysicalDevice* pds = (VkPhysicalDevice*)RL_CALLOC(physicalDeviceCount, sizeof(VkPhysicalDevice));
    VkResult result = vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, pds);
    if(result != VK_SUCCESS){
        const char res[] = "vkEnumeratePhysicalDevices failed";
        userdata->info.callback(WGVKRequestAdapterStatus_Unavailable, NULL, CLITERAL(WGVKStringView){.data = res,.length = sizeof(res) - 1},userdata->info.userdata1, userdata->info.userdata2);
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
    WGVKAdapter adapter = (WGVKAdapter)RL_CALLOC(1, sizeof(WGVKAdapterImpl));
    adapter->instance = userdata->instance;
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
    userdata->info.callback(WGVKRequestAdapterStatus_Success, adapter, CLITERAL(WGVKStringView){NULL, 0}, userdata->info.userdata1, userdata->info.userdata2);
}
WGVKFuture wgvkInstanceRequestAdapter(WGVKInstance instance, const WGVKRequestAdapterOptions* options, WGVKRequestAdapterCallbackInfo callbackInfo){
    userdataforcreateadapter* info = (userdataforcreateadapter*)RL_CALLOC(1, sizeof(userdataforcreateadapter));
    info->instance = instance;
    info->options = *options;
    info->info = callbackInfo;
    WGVKFuture ret = (WGVKFuture)RL_CALLOC(1, sizeof(WGVKFutureImpl));
    ret->userdataForFunction = info;
    ret->functionCalledOnWaitAny = wgvkCreateAdapter_impl;
    return ret;
}
int cmp_uint32_(const void *a, const void *b) {
    uint32_t ua = *(const uint32_t *)a;
    uint32_t ub = *(const uint32_t *)b;
    if (ua < ub) return -1;
    if (ua > ub) return 1;
    return 0;
}
static inline VkSemaphore CreateSemaphoreD(WGVKDevice device){
    VkSemaphoreCreateInfo sci zeroinit;
    VkSemaphore ret zeroinit;
    sci.flags = 0;
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult res = device->functions.vkCreateSemaphore(device->device, &sci, NULL, &ret);
    if(res != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Error creating semaphore");
    }
    return ret;
}
size_t sort_uniqueuints(uint32_t *arr, size_t count) {
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

WGVKDevice wgvkAdapterCreateDevice(WGVKAdapter adapter, const WGVKDeviceDescriptor* descriptor){
    //std::pair<WGVKDevice, WGVKQueue> ret = {0,0};
    
    for(uint32_t i = 0;i < descriptor->requiredFeatureCount;i++){
        WGVKFeatureName feature = descriptor->requiredFeatures[i];
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
        #ifndef FORCE_HEADLESS
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        #endif
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
    WGVKDevice retDevice = callocnew(WGVKDeviceImpl);
    WGVKQueue retQueue = callocnew(WGVKQueueImpl);

    VkResult dcresult = vkCreateDevice(adapter->physicalDevice, &createInfo, NULL, &(retDevice->device));
    struct VolkDeviceTable table = {0};

    if (dcresult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create logical device!");
    } else {
        //TRACELOG(LOG_INFO, "Successfully created logical device");
        volkLoadDeviceTable(&retDevice->functions, retDevice->device);
    }
    retDevice->uncapturedErrorCallbackInfo = descriptor->uncapturedErrorCallbackInfo;

    // Retrieve and assign queues
    
    QueueIndices indices = adapter->queueIndices;
    retDevice->functions.vkGetDeviceQueue(retDevice->device, indices.graphicsIndex, 0, &retQueue->graphicsQueue);
    #ifndef FORCE_HEADLESS
    retDevice->functions.vkGetDeviceQueue(retDevice->device, indices.presentIndex, 0, &retQueue->presentQueue);
    #endif
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
    WGVKCommandEncoderDescriptor cedesc zeroinit;
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
        VkFenceCreateInfo sci zeroinit;
        VkFence ret zeroinit;
        sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult res = retDevice->functions.vkCreateFence(retDevice->device, &sci, NULL, &retDevice->frameCaches[i].finalTransitionFence);
    }
    retQueue->presubmitCache = wgvkDeviceCreateCommandEncoder(retDevice, &cedesc);

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
    VmaVulkanFunctions vmaVulkanFunctions = {
        .vkAllocateMemory                    = retDevice->functions.vkAllocateMemory,
        .vkFreeMemory                        = retDevice->functions.vkFreeMemory,
        .vkCreateBuffer                      = retDevice->functions.vkCreateBuffer,
        .vkCreateImage                       = retDevice->functions.vkCreateImage,
        .vkDestroyBuffer                     = retDevice->functions.vkDestroyBuffer,
        .vkDestroyImage                      = retDevice->functions.vkDestroyImage,
        .vkGetDeviceBufferMemoryRequirements = retDevice->functions.vkGetDeviceBufferMemoryRequirements,
        .vkGetDeviceImageMemoryRequirements  = retDevice->functions.vkGetDeviceImageMemoryRequirements,
        .vkBindBufferMemory                  = retDevice->functions.vkBindBufferMemory,
        .vkCmdCopyBuffer                     = retDevice->functions.vkCmdCopyBuffer,
        .vkGetInstanceProcAddr               = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr                 = vkGetDeviceProcAddr
    };

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
        // TODO
    }

    {

        //auto [device, queue] = ret;
        retDevice->queue = retQueue;
        retDevice->adapter = adapter;
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

    return retDevice;
}


WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const WGVKBufferDescriptor* desc){
    //vmaCreateAllocator(const VmaAllocatorCreateInfo * _Nonnull pCreateInfo, VmaAllocator  _Nullable * _Nonnull pAllocator)
    WGVKBuffer wgvkBuffer = callocnew(WGVKBufferImpl);

    uint32_t cacheIndex = device->submittedFrames % framesInFlight;

    wgvkBuffer->device = device;
    wgvkBuffer->cacheIndex = cacheIndex;
    wgvkBuffer->refCount = 1;
    wgvkBuffer->usage = desc->usage;
    
    VkBufferCreateInfo bufferDesc zeroinit;
    bufferDesc.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferDesc.size = desc->size;
    bufferDesc.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferDesc.usage = toVulkanBufferUsage(desc->usage);
    
    VkMemoryPropertyFlags propertyToFind = 0;
    if(desc->usage & (BufferUsage_MapRead | BufferUsage_MapWrite)){
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

    VkResult vmabufferCreateResult = vmaCreateBuffer(device->allocator, &bufferDesc, &vallocInfo, &wgvkBuffer->buffer, &allocation, &allocationInfo);

    
    if(vmabufferCreateResult != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Could not allocate buffer: %s", vkErrorString(vmabufferCreateResult));
        //wgvkBuffer->~WGVKBufferImpl();
        RL_FREE(wgvkBuffer);
        return NULL;
    }
    wgvkBuffer->allocation = allocation;
    
    wgvkBuffer->memoryProperties = propertyToFind;

    if(desc->usage & BufferUsage_ShaderDeviceAddress){
        VkBufferDeviceAddressInfo bdai zeroinit;
        bdai.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        bdai.buffer = wgvkBuffer->buffer;
        wgvkBuffer->address = device->functions.vkGetBufferDeviceAddress(device->device, &bdai);
    }
    return wgvkBuffer;
}

void wgvkBufferMap(WGVKBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data){
    vmaMapMemory(buffer->device->allocator, buffer->allocation, data);
}

void wgvkBufferUnmap(WGVKBuffer buffer){
    vmaUnmapMemory(buffer->device->allocator, buffer->allocation);
    //VmaAllocationInfo allocationInfo zeroinit;
    //vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &allocationInfo);
    //vkUnmapMemory(buffer->device->device, allocationInfo.deviceMemory);
    //mappedMemories.erase(allocationInfo.deviceMemory);
}
size_t wgvkBufferGetSize(WGVKBuffer buffer){
    VmaAllocationInfo info zeroinit;
    vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &info);
    return info.size;
}

void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, const void* data, size_t size){
    void* mappedMemory = NULL;
    if(buffer->memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
        void* mappedMemory = NULL;
        wgvkBufferMap(buffer, MapMode_Write, bufferOffset, size, &mappedMemory);
        
        if (mappedMemory != NULL) {
            // Memory is host mappable: copy data and unmap.
            memcpy(mappedMemory, data, size);
            wgvkBufferUnmap(buffer);
            
        }
    }
    else{
        WGVKBufferDescriptor stDesc zeroinit;
        stDesc.size = size;
        stDesc.usage = BufferUsage_MapWrite;
        WGVKBuffer stagingBuffer = wgvkDeviceCreateBuffer(cSelf->device, &stDesc);
        wgvkQueueWriteBuffer(cSelf, stagingBuffer, 0, data, size);
        wgvkCommandEncoderCopyBufferToBuffer(cSelf->presubmitCache, stagingBuffer, 0, buffer, bufferOffset, size);
        wgvkBufferRelease(stagingBuffer);
    }
}

void wgvkQueueWriteTexture(WGVKQueue cSelf, const WGVKTexelCopyTextureInfo* destination, const void* data, size_t dataSize, const WGVKTexelCopyBufferLayout* dataLayout, const WGVKExtent3D* writeSize){

    WGVKBufferDescriptor bdesc zeroinit;
    bdesc.size = dataSize;
    bdesc.usage = BufferUsage_CopySrc | BufferUsage_MapWrite;
    WGVKBuffer stagingBuffer = wgvkDeviceCreateBuffer(cSelf->device, &bdesc);
    void* mappedMemory = NULL;
    wgvkBufferMap(stagingBuffer, MapMode_Write, 0, dataSize, &mappedMemory);
    if(mappedMemory != NULL){
        memcpy(mappedMemory, data, dataSize);
        wgvkBufferUnmap(stagingBuffer);
    }
    WGVKCommandEncoder enkoder = wgvkDeviceCreateCommandEncoder(cSelf->device, NULL);
    WGVKTexelCopyBufferInfo source;
    source.buffer = stagingBuffer;
    source.layout = *dataLayout;

    wgvkCommandEncoderCopyBufferToTexture(enkoder, &source, destination, writeSize);
    WGVKCommandBuffer puffer = wgvkCommandEncoderFinish(enkoder);

    wgvkQueueSubmit(cSelf, 1, &puffer);
    wgvkReleaseCommandEncoder(enkoder);
    wgvkReleaseCommandBuffer(puffer);

    wgvkBufferRelease(stagingBuffer);
}

WGVKTexture wgvkDeviceCreateTexture(WGVKDevice device, const WGVKTextureDescriptor* descriptor){
    VkDeviceMemory imageMemory zeroinit;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    WGVKTexture ret = callocnew(WGVKTextureImpl);

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
    if (vkCreateImage(device->device, &imageInfo, NULL, &image) != VK_SUCCESS)
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
    
    if (vkAllocateMemory(device->device, &allocInfo, NULL, &imageMemory) != VK_SUCCESS){
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

void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup wvBindGroup, const WGVKBindGroupDescriptor* bgdesc){
    
    rassert(bgdesc->layout != NULL, "WGVKBindGroupDescriptor::layout is null");
    
    if(wvBindGroup->pool == NULL){
        wvBindGroup->layout = bgdesc->layout;

        VkDescriptorType s;
        #define DESCRIPTOR_TYPE_UPPER_LIMIT 32
        uint32_t counts[DESCRIPTOR_TYPE_UPPER_LIMIT] = {0};

        //std::unordered_map<VkDescriptorType, uint32_t> counts;
        for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
            rassert((int)toVulkanResourceType(bgdesc->layout->entries[i].type) < 10, "Unsupported descriptor type");
            ++counts[descriptorTypeContiguous(toVulkanResourceType(bgdesc->layout->entries[i].type))];
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
        ResourceDescriptor entry = bgdesc->entries[i];
        if(entry.buffer){
            ru_trackBuffer(&newResourceUsage, (WGVKBuffer)entry.buffer, (BufferUsageSnap){0});
        }
        else if(entry.textureView){
            ru_trackTextureView(&newResourceUsage, (WGVKTextureView)entry.textureView);
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
        const ResourceTypeDescriptor* entryi = &bgdesc->layout->entries[i];
        writes.data[i].descriptorType = toVulkanResourceType(bgdesc->layout->entries[i].type);
        writes.data[i].descriptorCount = 1;

        if(entryi->type == uniform_buffer || entryi->type == storage_buffer){
            WGVKBuffer bufferOfThatEntry = (WGVKBuffer)bgdesc->entries[i].buffer;
            ru_trackBuffer(&wvBindGroup->resourceUsage, bufferOfThatEntry, (BufferUsageSnap){0});
            bufferInfos.data[i].buffer = bufferOfThatEntry->buffer;
            bufferInfos.data[i].offset = bgdesc->entries[i].offset;
            bufferInfos.data[i].range  =  bgdesc->entries[i].size;
            writes.data[i].pBufferInfo = bufferInfos.data + i;
        }

        if(entryi->type == texture2d || entryi->type == texture3d){
            ru_trackTextureView(&wvBindGroup->resourceUsage, (WGVKTextureView)bgdesc->entries[i].textureView);
            imageInfos.data[i].imageView   = ((WGVKTextureView)bgdesc->entries[i].textureView)->view;
            imageInfos.data[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            writes    .data[i].pImageInfo  = imageInfos.data + i;
        }
        if(entryi->type == storage_texture2d || entryi->type == storage_texture3d){
            ru_trackTextureView(&wvBindGroup->resourceUsage, (WGVKTextureView)bgdesc->entries[i].textureView);
            imageInfos.data[i].imageView   = ((WGVKTextureView)bgdesc->entries[i].textureView)->view;
            imageInfos.data[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            writes    .data[i].pImageInfo  = imageInfos.data + i;
        }

        if(entryi->type == texture_sampler){
            ru_trackSampler(&wvBindGroup->resourceUsage, bgdesc->entries[i].sampler);
            imageInfos.data[i].sampler    = bgdesc->entries[i].sampler->sampler;
            writes.    data[i].pImageInfo = imageInfos.data + i;
        }
        if(entryi->type == acceleration_structure){
            accelStructInfos.data[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            accelStructInfos.data[i].accelerationStructureCount = 1;
            accelStructInfos.data[i].pAccelerationStructures = &bgdesc->entries[i].accelerationStructure->accelerationStructure;
            writes          .data[i].pNext = &accelStructInfos.data[i];
        }
    }

    vkUpdateDescriptorSets(device->device, writes.size, writes.data, 0, NULL);

    VkWriteDescriptorSetVector_free(&writes);
    VkDescriptorBufferInfoVector_free(&bufferInfos);
    VkDescriptorImageInfoVector_free(&imageInfos);
    VkWriteDescriptorSetAccelerationStructureKHRVector_free(&accelStructInfos);
}



WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc){
    rassert(bgdesc->layout != NULL, "WGVKBindGroupDescriptor::layout is null");
    
    WGVKBindGroup ret = callocnew(WGVKBindGroupImpl);
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
            ++counts[toVulkanResourceType(bgdesc->layout->entries[i].type)];
        }
        VkDescriptorPoolSize sizes[DESCRIPTOR_TYPE_UPPER_LIMIT];
        uint32_t VkDescriptorPoolSizeCount = 0;
        for(uint32_t i = 0;i < DESCRIPTOR_TYPE_UPPER_LIMIT;i++){
            if(counts[i] != 0){
                sizes[VkDescriptorPoolSizeCount++] = (VkDescriptorPoolSize){
                    .type = (VkDescriptorType)i, 
                    .descriptorCount = counts[i]
                };
            }
        }

        dpci.poolSizeCount = VkDescriptorPoolSizeCount;
        dpci.pPoolSizes = sizes;
        dpci.maxSets = 1;
        vkCreateDescriptorPool(device->device, &dpci, NULL, &ret->pool);

        //VkCopyDescriptorSet copy{};
        //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        VkDescriptorSetAllocateInfo dsai zeroinit;
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.descriptorPool = ret->pool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = (VkDescriptorSetLayout*)&bgdesc->layout->layout;
        vkAllocateDescriptorSets(device->device, &dsai, &ret->set);
    }
    else{
        ret->pool = dsap->data[dsap->size - 1].pool;
        ret->set  = dsap->data[dsap->size - 1].set;
        --dsap->size;
    }
    ret->entryCount = bgdesc->entryCount;
        
    ret->entries = RL_CALLOC(bgdesc->entryCount, sizeof(ResourceDescriptor));
    if(bgdesc->entryCount > 0){
        memcpy(ret->entries, bgdesc->entries, bgdesc->entryCount * sizeof(ResourceDescriptor));
    }
    wgvkWriteBindGroup(device, ret, bgdesc);
    ret->layout = bgdesc->layout;
    ++ret->layout->refCount;
    rassert(ret->layout != NULL, "ret->layout is NULL");
    return ret;
}
WGVKBindGroupLayout wgvkDeviceCreateBindGroupLayout(WGVKDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount){
    WGVKBindGroupLayout ret = callocnew(WGVKBindGroupLayoutImpl);
    ret->refCount = 1;
    ret->device = device;
    ret->entryCount = entryCount;
    VkDescriptorSetLayoutCreateInfo slci zeroinit;
    slci.bindingCount = entryCount;

    VkDescriptorSetLayoutBindingVector bindings;
    VkDescriptorSetLayoutBindingVector_initWithSize(&bindings, slci.bindingCount);

    for(uint32_t i = 0;i < slci.bindingCount;i++){
        bindings.data[i].descriptorCount = 1;
        bindings.data[i].binding = entries[i].location;
        bindings.data[i].descriptorType = toVulkanResourceType(entries[i].type);
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
    vkCreateDescriptorSetLayout(device->device, &slci, NULL, &ret->layout);
    ResourceTypeDescriptor* entriesCopy = (ResourceTypeDescriptor*)RL_CALLOC(entryCount, sizeof(ResourceTypeDescriptor));
    if(entryCount > 0){
        memcpy(entriesCopy, entries, entryCount * sizeof(ResourceTypeDescriptor));
    }
    ret->entries = entriesCopy;

    VkDescriptorSetLayoutBindingVector_free(&bindings);
    
    return ret;
}
void wgvkReleasePipelineLayout(WGVKPipelineLayout pllayout){
    if(!--pllayout->refCount){
        for(uint32_t i = 0;i < pllayout->bindGroupLayoutCount;i++){
            wgvkBindGroupLayoutRelease(pllayout->bindGroupLayouts[i]);
        }
        RL_FREE((void*)pllayout->bindGroupLayouts);
        RL_FREE(pllayout);
    }
}
WGVKPipelineLayout wgvkDeviceCreatePipelineLayout(WGVKDevice device, const WGVKPipelineLayoutDescriptor* pldesc){
    WGVKPipelineLayout ret = callocnew(WGVKPipelineLayoutImpl);
    ret->refCount = 1;
    rassert(ret->bindGroupLayoutCount <= 8, "Only supports up to 8 BindGroupLayouts");
    ret->device = device;
    ret->bindGroupLayoutCount = pldesc->bindGroupLayoutCount;
    ret->bindGroupLayouts = (WGVKBindGroupLayout*)RL_CALLOC(pldesc->bindGroupLayoutCount, sizeof(void*));
    memcpy((void*)ret->bindGroupLayouts, (void*)pldesc->bindGroupLayouts, pldesc->bindGroupLayoutCount * sizeof(void*));
    VkDescriptorSetLayout dslayouts[8] zeroinit;
    for(uint32_t i = 0;i < ret->bindGroupLayoutCount;i++){
        wgvkBindGroupLayoutAddRef(ret->bindGroupLayouts[i]);
        dslayouts[i] = ret->bindGroupLayouts[i]->layout;
    }
    VkPipelineLayoutCreateInfo lci zeroinit;
    lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lci.pSetLayouts = dslayouts;
    lci.setLayoutCount = ret->bindGroupLayoutCount;
    VkResult res = vkCreatePipelineLayout(device->device, &lci, NULL, &ret->layout);
    if(res != VK_SUCCESS){
        wgvkReleasePipelineLayout(ret);
        ret = NULL;
    }
    return ret;
}


WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* desc){
    WGVKCommandEncoder ret = callocnew(WGVKCommandEncoderImpl);
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
        vkAllocateCommandBuffers(device->device, &bai, &ret->buffer);
        //TRACELOG(LOG_INFO, "Allocating new command buffer");
    }
    else{
        //TRACELOG(LOG_INFO, "Reusing");
        ret->buffer = device->frameCaches[ret->cacheIndex].commandBuffers.data[device->frameCaches[ret->cacheIndex].commandBuffers.size - 1];
        VkCommandBufferVector_pop_back(&device->frameCaches[ret->cacheIndex].commandBuffers);
        //vkResetCommandBuffer(ret->buffer, 0);
    }

    VkCommandBufferBeginInfo bbi zeroinit;
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(ret->buffer, &bbi);
    
    return ret;
}

WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor){
    
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
    WGVKTextureView ret = callocnew(WGVKTextureViewImpl);
    ret->refCount = 1;
    vkCreateImageView(texture->device->device, &ivci, NULL, &ret->view);
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
static inline VkClearValue toVkCV(const DColor c){
    VkClearValue cv zeroinit;
    cv.color.float32[0] = (float)c.r;
    cv.color.float32[1] = (float)c.g;
    cv.color.float32[2] = (float)c.b;
    cv.color.float32[3] = (float)c.a;
    return cv;
};
WGVKRenderPassEncoder wgvkCommandEncoderBeginRenderPass(WGVKCommandEncoder enc, const WGVKRenderPassDescriptor* rpdesc){
    WGVKRenderPassEncoder ret = calloc(1, sizeof(WGVKRenderPassEncoderImpl));
    VkCommandPool pool = enc->device->frameCaches[enc->cacheIndex].commandPool;


    ret->refCount = 2; //One for WGVKRenderPassEncoder the other for the command buffer
    
    WGVKRenderPassEncoderSet_add(&enc->referencedRPs, ret);
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
    VkResult fbresult = vkCreateFramebuffer(enc->device->device, &fbci, NULL, &ret->frameBuffer);
    if(fbresult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating framebuffer: %d", (int)fbresult);
    }
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
    memcpy(ret->beginInfo.colorAttachments, rpdesc->colorAttachments,rpdesc->colorAttachmentCount * sizeof(WGVKRenderPassColorAttachment));
    if(rpdesc->depthStencilAttachment){
        ret->beginInfo.depthStencilAttachment = *rpdesc->depthStencilAttachment;
    }
    RenderPassCommandGenericVector_init(&ret->bufferedCommands);
    //vkCmdBeginRenderPass(ret->secondaryCmdBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    #endif
    return ret;
}

void wgvkRenderPassEncoderEnd(WGVKRenderPassEncoder renderPassEncoder){
    
    WGVKDevice device = renderPassEncoder->device;
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
            const WGVKBindGroup group = cmdSetBindGroup->group;
            const WGVKBindGroupLayout layout = group->layout;
            

            for(uint32_t bindingIndex = 0;bindingIndex < layout->entryCount;bindingIndex++){
                rassert(group->entries[bindingIndex].binding == layout->entries[bindingIndex].location, "Mismatch between layout and group, this will cause bugs.");
                
                uniform_type eType = layout->entries[bindingIndex].type;

                if(snaps[eType].layout != VK_IMAGE_LAYOUT_UNDEFINED){
                    ShaderStageMask visibility = layout->entries[bindingIndex].visibility;
                    if(visibility == 0){ //TODO: Get rid of this hack
                        visibility = (ShaderStageMask_Vertex | ShaderStageMask_Fragment | ShaderStageMask_Compute);
                    }
                    ce_trackTextureView(
                        renderPassEncoder->cmdEncoder,
                        group->entries[bindingIndex].textureView,
                        (ImageUsageSnap){
                            .layout = snaps[eType].layout,
                            .access = snaps[eType].access,
                            .stage = toVulkanPipelineStage(visibility)
                        }
                    );
                }
            }
        }
    }


    vkCmdBeginRenderPass(destination, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    for(uint32_t i = 0;i < beginInfo->colorAttachmentCount;i++){
        device->functions.vkCmdSetViewport(destination, i, 1, &viewport);
        device->functions.vkCmdSetScissor (destination, i, 1, &scissor);
    }

    recordVkCommands(destination, renderPassEncoder->device, &renderPassEncoder->bufferedCommands);
    

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
WGVKCommandBuffer wgvkCommandEncoderFinish(WGVKCommandEncoder commandEncoder){
    WGVKCommandBuffer ret = callocnew(WGVKCommandBufferImpl);
    ret->refCount = 1;
    rassert(commandEncoder->movedFrom == 0, "Command encoder is already invalidated");
    commandEncoder->movedFrom = 1;
    commandEncoder->device->functions.vkEndCommandBuffer(commandEncoder->buffer);

    WGVKRenderPassEncoderSet_move(&ret->referencedRPs, &commandEncoder->referencedRPs);
    WGVKComputePassEncoderSet_move(&ret->referencedCPs, &commandEncoder->referencedCPs);
    WGVKRaytracingPassEncoderSet_move(&ret->referencedRTs, &commandEncoder->referencedRTs);
    ResourceUsage_move(&ret->resourceUsage, &commandEncoder->resourceUsage);
    ret->cacheIndex = commandEncoder->cacheIndex;
    ret->buffer = commandEncoder->buffer;
    ret->device = commandEncoder->device;
    commandEncoder->buffer = NULL;
    return ret;
}

void validatePipelineLayouts(WGVKPipelineLayout inserted, WGVKPipelineLayout base){
    WGVK_VALIDATE_EQ_PTR(inserted->device, inserted->device, base->device, "failed when verifying objects belong to the same device");
   
    WGVK_VALIDATE_EQ_UINT(inserted->device, inserted->bindGroupLayoutCount, base->bindGroupLayoutCount, "failed when validating bindGroupLayoutCounts");
    //VALIDATE_EQ(inserted->device, inserted->bindGroupLayoutCount, base->bindGroupLayoutCount)
    for(uint32_t i = 0;i < base->bindGroupLayoutCount;i++){
        for(uint32_t j = 0;j < base->bindGroupLayouts[i]->entryCount;j++){
            WGVK_VALIDATE_EQ_UINT(
                inserted->device,
                inserted->bindGroupLayouts[i]->entries[j].type,
                base->bindGroupLayouts[i]->entries[j].type,
                "failed when comparing BindGroupLayoutTypes"
            );
        }
    }
}

void recordVkCommand(CommandBufferAndSomeState* destination_, const RenderPassCommandGeneric* command){
    VkCommandBuffer destination = destination_->buffer;
    WGVKDevice device = destination_->device;

    switch(command->type){
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
            device->functions.vkCmdDispatch(
                destination, 
                dispatch->x, 
                dispatch->y, 
                dispatch->z
            );
        }
        break;
        
        case rp_command_type_set_force32: [[fallthrough]];
        case rp_command_type_enum_count: [[fallthrough]];
        case rp_command_type_invalid: rassert(false, "Invalid command type"); rg_unreachable();
    }
}
void recordVkCommands(VkCommandBuffer destination, WGVKDevice device, const RenderPassCommandGenericVector* commands){
    CommandBufferAndSomeState cal = {
        .buffer = destination,
        .device = device,
        .lastLayout = VK_NULL_HANDLE
    };

    for(size_t i = 0;i < commands->size;i++){
        recordVkCommand(&cal, commands->data + i);
    }
}
//void welldamn_sdfd(void* unused, ImageLayoutPair* unused2, void* unused3){
//    rg_trap();
//}
void registerTransitionCallback(void* texture_, ImageUsageRecord* record, void* pscache_){
    WGVKTexture texture = (WGVKTexture)texture_;
    WGVKDevice device = texture->device;
    WGVKCommandEncoder pscache = (WGVKCommandEncoder)pscache_;
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
    WGVKTexture texture = (WGVKTexture)texture_;
    texture->layout = record->lastLayout;
}
void wgvkQueueSubmit(WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers){
    VkSubmitInfo si zeroinit;
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = commandCount + 1;

    VkCommandBufferVector submittable;
    WGVKCommandBufferVector submittableWGVK;

    VkCommandBufferVector_initWithSize(&submittable, commandCount + 1);
    WGVKCommandBufferVector_initWithSize(&submittableWGVK, commandCount + 1);

    WGVKCommandEncoder pscache = queue->presubmitCache;
    
    // LayoutAssumptions_for_each(&pscache->resourceUsage.entryAndFinalLayouts, welldamn_sdfd, NULL);
    
    WGVKCommandBuffer sbuffer = buffers[0];
    ImageUsageRecordMap_for_each(&sbuffer->resourceUsage.referencedTextures, registerTransitionCallback, pscache); 
    
    
    WGVKCommandBuffer cachebuffer = wgvkCommandEncoderFinish(queue->presubmitCache);
    
    submittable.data[0] = cachebuffer->buffer;
    for(size_t i = 0;i < commandCount;i++){
        submittable.data[i + 1] = buffers[i]->buffer;
    }

    submittableWGVK.data[0] = cachebuffer;
    for(size_t i = 0;i < commandCount;i++){
        submittableWGVK.data[i + 1] = buffers[i];
    }

    for(uint32_t i = 0;i < submittableWGVK.size;i++){
        ImageUsageRecordMap_for_each(&submittableWGVK.data[i]->resourceUsage.referencedTextures, updateLayoutCallback, NULL);
    }

    si.pCommandBuffers = submittable.data;
    VkFence fence = VK_NULL_HANDLE;
    
    si.commandBufferCount = submittable.size;
    si.pCommandBuffers = submittable.data;


    const uint64_t frameCount = queue->device->submittedFrames;
    const uint32_t cacheIndex = frameCount % framesInFlight;
    int submitResult = 0;
    
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
        
        VkPipelineStageFlags* waitFlags = (VkPipelineStageFlags*)calloc(waitSemaphores.size, sizeof(VkPipelineStageFlags));
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
        submitResult |= vkQueueSubmit(queue->graphicsQueue, 1, &si, fence);

        VkSemaphoreVector_free(&waitSemaphores);
        free(waitFlags);
    }

    if(submitResult == VK_SUCCESS){
        WGVKCommandBufferVector insert;
        WGVKCommandBufferVector_init(&insert);
        
        WGVKCommandBufferVector_push_back(&insert, cachebuffer);
        ++cachebuffer->refCount;
        
        for(size_t i = 0;i < commandCount;i++){
            WGVKCommandBufferVector_push_back(&insert, buffers[i]);
            //insert.insert(buffers[i]);
            ++buffers[i]->refCount;
        }
        WGVKCommandBufferVector* fence_iterator = PendingCommandBufferMap_get(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence);
        //auto it = queue->pendingCommandBuffers[frameCount % framesInFlight].find(fence);
        if(fence_iterator == NULL){
            WGVKCommandBufferVector insert;
            PendingCommandBufferMap_put(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence, insert);
            fence_iterator = PendingCommandBufferMap_get(&(queue->pendingCommandBuffers[frameCount % framesInFlight]), (void*)fence);
            rassert(fence_iterator != NULL, "Something is wrong with the hash set");
            WGVKCommandBufferVector_init(fence_iterator);
        }

        for(size_t i = 0;i < insert.size;i++){
            WGVKCommandBufferVector_push_back(fence_iterator, insert.data[i]);
        }
        WGVKCommandBufferVector_free(&insert);
    }else{
        TRACELOG(LOG_FATAL, "vkQueueSubmit failed");
    }
    wgvkReleaseCommandEncoder(queue->presubmitCache);
    wgvkReleaseCommandBuffer(cachebuffer);
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    queue->presubmitCache = wgvkDeviceCreateCommandEncoder(queue->device, &cedesc);
    VkCommandBufferVector_free(&submittable);
    WGVKCommandBufferVector_free(&submittableWGVK);
}



void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities){
    VkSurfaceKHR surface = wgvkSurface->surface;
    VkSurfaceCapabilitiesKHR scap zeroinit;
    VkPhysicalDevice vk_physicalDevice = adapter->physicalDevice;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physicalDevice, surface, &scap);
    
    // TRACELOG(LOG_INFO, "scalphaflags: %d", scap.supportedCompositeAlpha);
    
    // Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, NULL);
    if (formatCount != 0) {

        wgvkSurface->formatCache = (PixelFormat*)calloc(formatCount, sizeof(PixelFormat));
        VkSurfaceFormatKHR* surfaceFormats = (VkSurfaceFormatKHR*)calloc(formatCount, sizeof(VkSurfaceFormatKHR));
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, surfaceFormats);
        for(size_t i = 0;i < formatCount;i++){
            wgvkSurface->formatCache[i] = fromVulkanPixelFormat(surfaceFormats[i].format);
        }
        wgvkSurface->formatCount = formatCount;
        free(surfaceFormats);
    }

    // Present Modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, NULL);
    if (presentModeCount != 0) {
        wgvkSurface->presentModeCache = (PresentMode*)RL_CALLOC(presentModeCount, sizeof(PresentMode));
        VkPresentModeKHR presentModes[16] = {0};//(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, presentModes);
        for(size_t i = 0;i < presentModeCount;i++){
            wgvkSurface->presentModeCache[i] = fromVulkanPresentMode(presentModes[i]);
        }
        wgvkSurface->presentModeCount = presentModeCount;
    }
    capabilities->presentModeCount = wgvkSurface->presentModeCount;
    capabilities->formatCount = wgvkSurface->formatCount;
    capabilities->presentModes = wgvkSurface->presentModeCache;
    capabilities->formats = wgvkSurface->formatCache;
    capabilities->usages = fromVulkanTextureUsage(scap.supportedUsageFlags);
}

void wgvkSurfaceConfigure(WGVKSurface surface, const WGVKSurfaceConfiguration* config){
    WGVKDevice device = config->device;
    surface->device = config->device;
    surface->lastConfig = *config;
    device->functions.vkDeviceWaitIdle(device->device);
    if(surface->imageViews){
        for (uint32_t i = 0; i < surface->imagecount; i++) {
            wgvkReleaseTextureView(surface->imageViews[i]);
            //This line is not required, since those are swapchain-owned images
            //These images also have a null memory member
            //wgvkReleaseTexture(surface->images[i]);
        }
    }

    //std::free(surface->framebuffers);
    
    free(surface->imageViews);
    free(surface->images);
    device->functions.vkDestroySwapchainKHR(device->device, surface->swapchain, NULL);
    
    VkSurfaceCapabilitiesKHR vkCapabilities zeroinit;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->adapter->physicalDevice, surface->surface, &vkCapabilities);
    VkSwapchainCreateInfoKHR createInfo zeroinit;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;
    uint32_t correctedWidth, correctedHeight;
    #define ICLAMP_TEMP(V, MINI, MAXI) ((V) < (MINI)) ? (MINI) : (((V) > (MAXI)) ? (MAXI) : (V))
    if(config->width < vkCapabilities.minImageExtent.width || config->width > vkCapabilities.maxImageExtent.width){
        correctedWidth = ICLAMP_TEMP(config->width, vkCapabilities.minImageExtent.width, vkCapabilities.maxImageExtent.width);
        TRACELOG(LOG_WARNING, "Invalid SurfaceConfiguration::width %u, adjusting to %u", config->width, correctedWidth);
    }
    else{
        correctedWidth = config->width;
    }
    if(config->height < vkCapabilities.minImageExtent.height || config->height > vkCapabilities.maxImageExtent.height){
        correctedHeight = ICLAMP_TEMP(config->height, vkCapabilities.minImageExtent.height, vkCapabilities.maxImageExtent.height);
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
    VkExtent2D newExtent = CLITERAL(VkExtent2D){correctedWidth, correctedHeight};
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
        TRACELOG(LOG_INFO, "wgvkSurfaceConfigure(): Successfully created swap chain");
    }

    device->functions.vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, NULL);

    VkImage tmpImages[32] = {0};

    //surface->imageViews = (VkImageView*)std::calloc(surface->imagecount, sizeof(VkImageView));
    TRACELOG(LOG_INFO, "Imagecount: %d", (int)surface->imagecount);
    device->functions.vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, tmpImages);
    surface->images = (WGVKTexture*)calloc(surface->imagecount, sizeof(WGVKTexture));
    for (uint32_t i = 0; i < surface->imagecount; i++) {
        surface->images[i] = callocnew(WGVKTextureImpl);
        surface->images[i]->device = device;
        surface->images[i]->width = correctedWidth;
        surface->images[i]->height = correctedHeight;
        surface->images[i]->depthOrArrayLayers = 1;
        surface->images[i]->refCount = 1;
        surface->images[i]->sampleCount = 1;
        surface->images[i]->image = tmpImages[i];
    }
    surface->imageViews = (WGVKTextureView*)RL_CALLOC(surface->imagecount, sizeof(VkImageView));

    for (uint32_t i = 0; i < surface->imagecount; i++) {
        WGVKTextureViewDescriptor viewDesc zeroinit;
        viewDesc.arrayLayerCount = 1;
        viewDesc.baseArrayLayer = 0;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;
        viewDesc.aspect = TextureAspect_All;
        viewDesc.dimension = TextureViewDimension_2D;
        viewDesc.format = config->format;
        viewDesc.usage = TextureUsage_RenderAttachment | TextureUsage_CopySrc;
        surface->imageViews[i] = wgvkTextureCreateView(surface->images[i], &viewDesc);
    }

}

void wgvkComputePassEncoderDispatchWorkgroups(WGVKComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z){
    RenderPassCommandGeneric insert = {
        .type = cp_command_type_dispatch_workgroups,
        .dispatchWorkgroups = {x, y, z}
    };

    RenderPassCommandGenericVector_push_back(&cpe->bufferedCommands, insert);
}


void wgvkReleaseCommandEncoder(WGVKCommandEncoder commandEncoder) {

    rassert(commandEncoder->movedFrom, "Commandencoder still valid");

    if(commandEncoder->buffer){
        VkCommandBufferVector_push_back(
            &commandEncoder->device->frameCaches[commandEncoder->cacheIndex].commandBuffers,
            commandEncoder->buffer
        );
    }
    
    
    RL_FREE(commandEncoder);
}

static void releaseRPSetCallback(WGVKRenderPassEncoder rpEncoder, void* unused){
    wgvkReleaseRenderPassEncoder(rpEncoder);
}

static void releaseCPSetCallback(WGVKComputePassEncoder cpEncoder, void* unused){
    wgvkReleaseComputePassEncoder(cpEncoder);
}

static void releaseRTSetCallback(WGVKRaytracingPassEncoder rtEncoder, void* unused){
    wgvkReleaseRaytracingPassEncoder(rtEncoder);
}
void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer) {
    if(--commandBuffer->refCount == 0){
        WGVKRenderPassEncoderSet_for_each(&commandBuffer->referencedRPs, releaseRPSetCallback, NULL);
        WGVKComputePassEncoderSet_for_each(&commandBuffer->referencedCPs, releaseCPSetCallback, NULL);
        WGVKRaytracingPassEncoderSet_for_each(&commandBuffer->referencedRTs, releaseRTSetCallback, NULL);
        
        releaseAllAndClear(&commandBuffer->resourceUsage);
        // The above performs ResourceUsage_free already!
    

        WGVKRenderPassEncoderSet_free(&commandBuffer->referencedRPs);
        WGVKComputePassEncoderSet_free(&commandBuffer->referencedCPs);
        WGVKRaytracingPassEncoderSet_free(&commandBuffer->referencedRTs);
        
        PerframeCache* frameCache = &commandBuffer->device->frameCaches[commandBuffer->cacheIndex];
        VkCommandBufferVector_push_back(&frameCache->commandBuffers, commandBuffer->buffer);
        
        RL_FREE(commandBuffer);
    }
}

void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc) {
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
void wgvkSamplerRelease(WGVKSampler sampler){
    if(!--sampler->refCount){
        sampler->device->functions.vkDestroySampler(sampler->device->device, sampler->sampler, NULL);
        RL_FREE(sampler);
    }
}
void wgvkBufferRelease(WGVKBuffer buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vmaDestroyBuffer(buffer->device->allocator, buffer->buffer, buffer->allocation);
        RL_FREE(buffer);
    }
}

void wgvkBindGroupRelease(WGVKBindGroup dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        releaseAllAndClear(&dshandle->resourceUsage);
        wgvkBindGroupLayoutRelease(dshandle->layout);
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

        //DONT delete them, they are cached
        //vkFreeDescriptorSets(dshandle->device->device, dshandle->pool, 1, &dshandle->set);
        //vkDestroyDescriptorPool(dshandle->device->device, dshandle->pool, NULL);
        
        free(dshandle);
    }
}
WGVKRenderPipeline wgvkDeviceCreateRenderPipeline(WGVKDevice device, WGVKRenderPipelineDescriptor const * descriptor) {
    WGVKDeviceImpl* deviceImpl = (WGVKDeviceImpl*)(device);
    WGVKPipelineLayout pl_layout = descriptor->layout;

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
        const WGVKVertexBufferLayout* layout = &descriptor->vertex.buffers[i];
        bindingDescriptions[i].binding = currentBinding; // Assuming bindings are contiguous from 0, I think webgpu doesn't allow anything else
        bindingDescriptions[i].stride = (uint32_t)layout->arrayStride;
        bindingDescriptions[i].inputRate = toVulkanVertexStepMode(layout->stepMode);

        for (size_t j = 0; j < layout->attributeCount; ++j) {
            const VertexAttribute* attrib = &layout->attributes[j];
            VkVertexInputAttributeDescription vkAttrib zeroinit;
            vkAttrib.binding = currentBinding;
            vkAttrib.location = attrib->shaderLocation;
            vkAttrib.format = toVulkanVertexFormat(attrib->format);
            vkAttrib.offset = (uint32_t)attrib->offset;
            attributeDescriptions[attributeDescriptionCount++] = vkAttrib;
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
    inputAssembly.topology = toVulkanPrimitive(descriptor->primitive.topology);
    inputAssembly.primitiveRestartEnable = VK_FALSE; // WGVK doesn't expose primitive restart? Assume false.
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
        const WGVKDepthStencilState* ds = descriptor->depthStencil;
        depthStencil.depthTestEnable = VK_TRUE; // If struct exists, assume depth test is desired
        depthStencil.depthWriteEnable = ds->depthWriteEnabled ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = toVulkanCompareFunction(ds->depthCompare);
        depthStencil.depthBoundsTestEnable = VK_FALSE; // Not in WGVK descriptor
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;

        bool stencilTestRequired =
            ds->stencilFront.compare != CompareFunction_Undefined || ds->stencilFront.failOp != WGVKStencilOperation_Undefined ||
            ds->stencilBack.compare != CompareFunction_Undefined || ds->stencilBack.failOp != WGVKStencilOperation_Undefined ||
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
            const WGVKColorTargetState* target = &descriptor->fragment->targets[i];
            // Defaults for no blending
            colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // TODO: Map WGVKColorWriteMask if exists
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
    // Assuming traditional render passes for now based on WGVK structure.
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
    // WGVK API hides this detail? We might need to use dynamic rendering or query/cache render passes.
    // Placeholder: Assume renderPass is NULL and we rely on dynamic rendering or compatibility context.
    pipelineInfo.renderPass = VK_NULL_HANDLE; // Needs a valid VkRenderPass unless VK_KHR_dynamic_rendering is used and enabled.
    pipelineInfo.subpass = 0; // Assuming subpass 0

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional


    WGVKRenderPipelineImpl* pipelineImpl = (WGVKRenderPipelineImpl*)malloc(sizeof(WGVKRenderPipelineImpl));
    if (!pipelineImpl) {
        // Handle allocation failure
        return NULL;
    }
    pipelineImpl->device = device;
    //wgvkDeviceAddRef(device);
    pipelineImpl->layout = pl_layout; // Store for potential use
    wgvkPipelineLayoutAddRef(pl_layout);
    VkResult result = vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &pipelineImpl->renderPipeline);

    if (result != VK_SUCCESS) {
        // Handle pipeline creation failure
        RL_FREE(pipelineImpl);
        // Optionally log error based on result code
        return NULL;
    }

    return pipelineImpl;
}
void wgvkBindGroupLayoutRelease(WGVKBindGroupLayout bglayout){
    --bglayout->refCount;
    if(bglayout->refCount == 0){
        bglayout->device->functions.vkDestroyDescriptorSetLayout(bglayout->device->device, bglayout->layout, NULL);
        RL_FREE((ResourceTypeDescriptor*)(bglayout->entries));
        RL_FREE(bglayout);
    }
}

void wgvkReleaseTexture(WGVKTexture texture){
    --texture->refCount;
    if(texture->refCount == 0){
        texture->device->functions.vkDestroyImage(texture->device->device, texture->image, NULL);
        texture->device->functions.vkFreeMemory(texture->device->device, texture->memory, NULL);

        RL_FREE(texture);
    }
}
void wgvkReleaseTextureView(WGVKTextureView view){
    --view->refCount;
    if(view->refCount == 0){
        view->texture->device->functions.vkDestroyImageView(view->texture->device->device, view->view, NULL);
        wgvkReleaseTexture(view->texture);
        RL_FREE(view);
    }
}

void wgvkCommandEncoderCopyBufferToBuffer  (WGVKCommandEncoder commandEncoder, WGVKBuffer source, uint64_t sourceOffset, WGVKBuffer destination, uint64_t destinationOffset, uint64_t size){
    ce_trackBuffer(
        commandEncoder,
        source,
        (BufferUsageSnap){
            .lastStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .lastAccess = VK_ACCESS_TRANSFER_READ_BIT
        }
    );
    ce_trackBuffer(
        commandEncoder,
        destination,
        (BufferUsageSnap){
            .lastStage = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .lastAccess = VK_ACCESS_TRANSFER_WRITE_BIT
        }
    );

    VkBufferCopy copy zeroinit;
    copy.srcOffset = sourceOffset;
    copy.dstOffset = destinationOffset;
    copy.size = size;
    vkCmdCopyBuffer(commandEncoder->buffer, source->buffer, destination->buffer, 1, &copy);
    
    VkMemoryBarrier memoryBarrier zeroinit;
    VkBufferMemoryBarrier bufferbarr zeroinit;
    bufferbarr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    bufferbarr.buffer = destination->buffer;
    bufferbarr.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    bufferbarr.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
    bufferbarr.size = VK_WHOLE_SIZE;

    bufferbarr.dstQueueFamilyIndex = commandEncoder->device->adapter->queueIndices.graphicsIndex;
    bufferbarr.srcQueueFamilyIndex = commandEncoder->device->adapter->queueIndices.graphicsIndex;
    vkCmdPipelineBarrier(commandEncoder->buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, 0, 1, &bufferbarr, 0, 0);
}
void wgvkCommandEncoderCopyBufferToTexture (WGVKCommandEncoder commandEncoder, WGVKTexelCopyBufferInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize){
    
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

    vkCmdCopyBufferToImage(commandEncoder->buffer, source->buffer->buffer, destination->texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
void wgvkCommandEncoderCopyTextureToBuffer (WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyBufferInfo const * destination, WGVKExtent3D const * copySize){
    TRACELOG(LOG_FATAL, "Not implemented");
    rg_unreachable();
}
void wgvkCommandEncoderCopyTextureToTexture(WGVKCommandEncoder commandEncoder, const WGVKTexelCopyTextureInfo* source, const WGVKTexelCopyTextureInfo* destination, const WGVKExtent3D* copySize){
    
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
void RenderPassEncoder_PushCommand(WGVKRenderPassEncoder encoder, const RenderPassCommandGeneric* cmd){
    if(cmd->type == rp_command_type_set_render_pipeline){
        encoder->lastLayout = cmd->setRenderPipeline.pipeline->layout;
    }
    RenderPassCommandGenericVector_push_back(&encoder->bufferedCommands, *cmd);
}
void ComputePassEncoder_PushCommand(WGVKComputePassEncoder encoder, const RenderPassCommandGeneric* cmd){
    if(cmd->type == cp_command_type_set_compute_pipeline){
        encoder->lastLayout = cmd->setComputePipeline.pipeline->layout;
    }
    RenderPassCommandGenericVector_push_back(&encoder->bufferedCommands, *cmd);
}



// Implementation of RenderpassEncoderDraw
void wgvkRenderpassEncoderDraw(WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
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
void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t baseVertex, uint32_t firstinstance) {
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

void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, WGVKRenderPipeline renderPipeline) {
    RenderPassCommandGeneric insert = {
        .type = rp_command_type_set_render_pipeline,
        .setRenderPipeline = {
            renderPipeline
        }
    };
    RenderPassEncoder_PushCommand(rpe, &insert);
    ru_trackRenderPipeline(&rpe->resourceUsage, renderPipeline);
}

const static VkAccessFlags access_to_vk[8] = {
    [readonly] = VK_ACCESS_SHADER_READ_BIT,
    [writeonly] = VK_ACCESS_SHADER_WRITE_BIT,
    [readwrite] = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
};
const static int is_storage_texture[uniform_type_enumcount] = {
    [storage_texture2d_array] = 1,
    [storage_texture3d] = 1,
    [storage_texture2d] = 1,
};
void wgvkRenderPassEncoderSetBindGroup(WGVKRenderPassEncoder rpe, uint32_t groupIndex, WGVKBindGroup group, size_t dynamicOffsetCount, const uint32_t* dynamicOffsets) {
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
    
    //VkBufferMemoryBarrier bufferbarriers[64] = {0};
    //uint32_t bbInsertPos = 0;
    //VkImageMemoryBarrier imageBarriers  [64] = {0};
    //uint32_t ibInsertPos = 0;
    
    

    for(uint32_t i = 0;i < group->entryCount;i++){
        if(group->entries[i].buffer){
            const VkAccessFlags accessFlags = access_to_vk[group->layout->entries[i].access];
            const VkPipelineStageFlags stage = toVulkanPipelineStage(group->layout->entries[i].visibility);
            ce_trackBuffer(rpe->cmdEncoder, group->entries[i].buffer, (BufferUsageSnap){
                .lastStage = stage,
                .lastAccess = accessFlags
            });
        }
        if(group->entries[i].textureView){
            
            const VkAccessFlags accessFlags = access_to_vk[group->layout->entries[i].access];
            const VkPipelineStageFlags stage = toVulkanPipelineStage(group->layout->entries[i].visibility) | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            ImageUsageSnap usageSnap zeroinit;
            VkImageLayout layout = is_storage_texture[group->layout->entries[i].type] ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            ce_trackTextureView(rpe->cmdEncoder, group->entries[i].textureView, (ImageUsageSnap){
                .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .access = accessFlags,
                .stage = stage,
                .subresource = group->entries[i].textureView->subresourceRange
            });
        }
    }
    ru_trackBindGroup(&rpe->resourceUsage, group);
    
}

void wgvkComputePassEncoderSetPipeline (WGVKComputePassEncoder cpe, WGVKComputePipeline computePipeline){
    
    RenderPassCommandGeneric insert = {
        .type = cp_command_type_set_compute_pipeline,
        .setComputePipeline = {
            computePipeline
        }
    };

    ComputePassEncoder_PushCommand(cpe, &insert);
}
void wgvkComputePassEncoderSetBindGroup(WGVKComputePassEncoder cpe, uint32_t groupIndex, WGVKBindGroup group, size_t dynamicOffsetCount, const uint32_t* dynamicOffsets){
    
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
    ComputePassEncoder_PushCommand(cpe, &insert);
}

WGVKComputePassEncoder wgvkCommandEncoderBeginComputePass(WGVKCommandEncoder commandEncoder){
    WGVKComputePassEncoder ret = callocnew(WGVKComputePassEncoderImpl);
    ret->refCount = 2;
    WGVKComputePassEncoderSet_add(&commandEncoder->referencedCPs, ret);

    RenderPassCommandGenericVector_init(&ret->bufferedCommands);

    ret->cmdEncoder = commandEncoder;
    ret->device = commandEncoder->device;
    return ret;
}
void wgvkCommandEncoderEndComputePass(WGVKComputePassEncoder commandEncoder){
    recordVkCommands(commandEncoder->cmdEncoder->buffer, commandEncoder->device, &commandEncoder->bufferedCommands);
}
void wgvkReleaseComputePassEncoder(WGVKComputePassEncoder cpenc){
    --cpenc->refCount;
    if(cpenc->refCount == 0){
        releaseAllAndClear(&cpenc->resourceUsage);
        RenderPassCommandGenericVector_free(&cpenc->bufferedCommands);
        RL_FREE(cpenc);
    }
}

void wgvkReleaseRaytracingPassEncoder(WGVKRaytracingPassEncoder rtenc){
    --rtenc->refCount;
    if(rtenc->refCount == 0){
        releaseAllAndClear(&rtenc->resourceUsage);
        RL_FREE(rtenc);
    }
}
void wgvkSurfacePresent(WGVKSurface surface){
    WGVKDevice device = surface->device;
    uint32_t cacheIndex = surface->device->submittedFrames % framesInFlight;

    VkPresentInfoKHR presentInfo zeroinit;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &surface->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    VkSwapchainKHR swapChains[] = {surface->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &surface->activeImageIndex;
    VkImageSubresourceRange isr zeroinit;
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;

    
    VkCommandBuffer transitionBuffer = surface->device->frameCaches[cacheIndex].finalTransitionBuffer;
    
    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags =  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    device->functions.vkBeginCommandBuffer(transitionBuffer, &beginInfo);

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
    //EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, surface->images[surface->activeImageIndex]);
    //EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (WGVKTexture)surface->renderTarget.depth.id);
    device->functions.vkEndCommandBuffer(transitionBuffer);
    VkSubmitInfo cbsinfo zeroinit;
    cbsinfo.commandBufferCount = 1;
    cbsinfo.pCommandBuffers = &transitionBuffer;
    cbsinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    cbsinfo.signalSemaphoreCount = 1;
    cbsinfo.waitSemaphoreCount = 1;
    VkPipelineStageFlags wsmask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    cbsinfo.pWaitDstStageMask = &wsmask;

    cbsinfo.pWaitSemaphores = &surface->device->queue->syncState[cacheIndex].semaphores.data[surface->device->queue->syncState[cacheIndex].submits];
    //TRACELOG(LOG_INFO, "Submit waiting for semaphore index: %u", g_vulkanstate.queue->syncState[cacheIndex].submits);
    
    cbsinfo.pSignalSemaphores = &surface->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    
    VkFence finalTransitionFence = surface->device->frameCaches[cacheIndex].finalTransitionFence;
    device->functions.vkQueueSubmit(surface->device->queue->graphicsQueue, 1, &cbsinfo, finalTransitionFence);
    
    
    WGVKCommandBufferVector* cmdBuffers = PendingCommandBufferMap_get(&surface->device->queue->pendingCommandBuffers[cacheIndex], (void*)finalTransitionFence);
    
    if(cmdBuffers == NULL){
        WGVKCommandBufferVector insert;
        PendingCommandBufferMap_put(&surface->device->queue->pendingCommandBuffers[cacheIndex], finalTransitionFence, insert);
        cmdBuffers = PendingCommandBufferMap_get(&surface->device->queue->pendingCommandBuffers[cacheIndex], (void*)finalTransitionFence);
        WGVKCommandBufferVector_init(cmdBuffers);
    }

    VkResult presentRes = device->functions.vkQueuePresentKHR(surface->device->queue->presentQueue, &presentInfo);

    ++surface->device->submittedFrames;
    vmaSetCurrentFrameIndex(surface->device->allocator, surface->device->submittedFrames % framesInFlight);
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
        wgvkSurfaceConfigure(surface, &surface->lastConfig);
    }
}

static inline VkSamplerAddressMode vkamode(addressMode a){
    switch(a){
        case clampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case mirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        default:
            rg_unreachable();
    }
};
WGVKSampler wgvkDeviceCreateSampler(WGVKDevice device, const WGVKSamplerDescriptor* descriptor){

    WGVKSampler ret = callocnew(WGVKSamplerImpl);
    ret->refCount = 1;
    VkSamplerCreateInfo sci zeroinit;
    sci.compareEnable = VK_FALSE;
    //sci.compareOp = VK_COMPARE_OP_LESS;
    sci.maxLod = descriptor->lodMaxClamp;
    sci.minLod = descriptor->lodMinClamp;
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.addressModeU = vkamode(descriptor->addressModeU);
    sci.addressModeV = vkamode(descriptor->addressModeV);
    sci.addressModeW = vkamode(descriptor->addressModeW);
    
    sci.mipmapMode = ((descriptor->mipmapFilter == filter_linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);

    sci.anisotropyEnable = false;
    sci.maxAnisotropy = descriptor->maxAnisotropy;
    sci.magFilter = ((descriptor->magFilter == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    sci.minFilter = ((descriptor->minFilter == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    VkResult result = device->functions.vkCreateSampler(device->device, &sci, NULL, &(ret->sampler));
    return ret;
}
void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset) {
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
    
    ru_trackBuffer(&rpe->resourceUsage, buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT});
}
void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, IndexFormat indexType){
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
    
    ru_trackBuffer(&rpe->resourceUsage, buffer, (BufferUsageSnap){VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT});
}
void wgvkTextureViewAddRef(WGVKTextureView textureView){
    ++textureView->refCount;
}
void wgvkTextureAddRef(WGVKTexture texture){
    ++texture->refCount;
}
void wgvkBufferAddRef(WGVKBuffer buffer){
    ++buffer->refCount;
}
void wgvkBindGroupAddRef(WGVKBindGroup bindGroup){
    ++bindGroup->refCount;
}
void wgvkBindGroupLayoutAddRef(WGVKBindGroupLayout bindGroupLayout){
    ++bindGroupLayout->refCount;
}
void wgvkPipelineLayoutAddRef(WGVKPipelineLayout pipelineLayout){
    ++pipelineLayout->refCount;
}
void wgvkSamplerAddRef(WGVKSampler sampler){
    ++sampler->refCount;
}
void wgvkRenderPipelineAddRef(WGVKRenderPipeline rpl){
    ++rpl->refCount;
}
void wgvkComputePipelineAddRef(WGVKComputePipeline cpl){
    ++cpl->refCount;
}

RGAPI void ru_trackBuffer(ResourceUsage* resourceUsage, WGVKBuffer buffer, BufferUsageRecord brecord){
    if(BufferUsageRecordMap_put(&resourceUsage->referencedBuffers, (void*)buffer, brecord)){
        ++buffer->refCount;
    }
}

RGAPI void ru_trackTexture(ResourceUsage* resourceUsage, WGVKTexture texture, ImageUsageRecord record){
    if(ImageUsageRecordMap_put(&resourceUsage->referencedTextures, texture, record)){
        ++texture->refCount;
    }
}

RGAPI void ru_trackTextureView     (ResourceUsage* resourceUsage, WGVKTextureView view){
    if(ImageViewUsageSet_add(&resourceUsage->referencedTextureViews, view)){
        ++view->refCount;
    }
}
RGAPI void ru_trackRenderPipeline(ResourceUsage* resourceUsage, WGVKRenderPipeline renderPipeline){
    if(RenderPipelineUsageSet_add(&resourceUsage->referencedRenderPipelines, renderPipeline)){
        ++renderPipeline->refCount;
    }
}
RGAPI void ru_trackComputePipeline(ResourceUsage* resourceUsage, WGVKComputePipeline computePipeline){
    if(ComputePipelineUsageSet_add(&resourceUsage->referencedComputePipelines, computePipeline)){
        ++computePipeline->refCount;
    }
}
RGAPI void ru_trackBindGroup(ResourceUsage* resourceUsage, WGVKBindGroup bindGroup){
    if(BindGroupUsageSet_add(&resourceUsage->referencedBindGroups, (void*)bindGroup)){
        ++bindGroup->refCount;
    }
}

RGAPI void ru_trackBindGroupLayout (ResourceUsage* resourceUsage, WGVKBindGroupLayout bindGroupLayout){
    if(BindGroupLayoutUsageSet_add(&resourceUsage->referencedBindGroupLayouts, bindGroupLayout)){
        ++bindGroupLayout->refCount;
    }
}

RGAPI void ru_trackSampler         (ResourceUsage* resourceUsage, WGVKSampler sampler){
    if(SamplerUsageSet_add(&resourceUsage->referencedSamplers, sampler)){
        ++sampler->refCount;
    }
}

RGAPI void ce_trackTexture(WGVKCommandEncoder encoder, WGVKTexture texture, ImageUsageSnap usage){
    WGVK_VALIDATE_EQ_PTR(encoder->device, texture->device, encoder->device, "failed when validating devices");
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

RGAPI void ce_trackTextureView(WGVKCommandEncoder enc, WGVKTextureView view, ImageUsageSnap usage){
    
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
RGAPI void ce_trackBuffer(WGVKCommandEncoder encoder, WGVKBuffer buffer, BufferUsageSnap usage){
    BufferUsageRecord* rec = BufferUsageRecordMap_get(&encoder->resourceUsage.referencedBuffers, buffer);
    if(rec != NULL){
        VkBufferMemoryBarrier bufferBarrier = {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            NULL,
            rec->lastAccess,
            usage.lastAccess, 
            encoder->device->adapter->queueIndices.graphicsIndex,
            encoder->device->adapter->queueIndices.graphicsIndex,
            buffer->buffer,
            0,
            VK_WHOLE_SIZE
        };

        encoder->device->functions.vkCmdPipelineBarrier(
            encoder->buffer,
            rec->lastStage,
            usage.lastStage,
            0,
            0, 0,
            1, &bufferBarrier,
            0, 0
        );
    }
    else{
        ru_trackBuffer(&encoder->resourceUsage, buffer, usage);
    }

}


static inline void bufferReleaseCallback(void* buffer, BufferUsageRecord* bu_record, void* unused){
    wgvkBufferRelease(buffer);
}
static inline void textureReleaseCallback(void* texture, ImageUsageRecord* iur, void* unused){
    (void)unused;
    wgvkReleaseTexture((WGVKTexture)texture);
}
static inline void textureViewReleaseCallback(WGVKTextureView textureView, void* unused){
    (void)unused;
    wgvkReleaseTextureView(textureView);
}
static inline void bindGroupReleaseCallback(WGVKBindGroup bindGroup, void* unused){
    wgvkBindGroupRelease(bindGroup);
}
static inline void bindGroupLayoutReleaseCallback(WGVKBindGroupLayout bgl, void* unused){
    wgvkBindGroupLayoutRelease(bgl);
}
static inline void samplerReleaseCallback(WGVKSampler sampler, void* unused){
    wgvkSamplerRelease(sampler);
}


void releaseAllAndClear(ResourceUsage* resourceUsage){
    BufferUsageRecordMap_for_each(&resourceUsage->referencedBuffers, bufferReleaseCallback, NULL); 
    ImageUsageRecordMap_for_each(&resourceUsage->referencedTextures, textureReleaseCallback, NULL); 
    ImageViewUsageSet_for_each(&resourceUsage->referencedTextureViews, textureViewReleaseCallback, NULL); 
    BindGroupUsageSet_for_each(&resourceUsage->referencedBindGroups, bindGroupReleaseCallback, NULL);
    BindGroupLayoutUsageSet_for_each(&resourceUsage->referencedBindGroupLayouts, bindGroupLayoutReleaseCallback, NULL);
    SamplerUsageSet_for_each(&resourceUsage->referencedSamplers, samplerReleaseCallback, NULL);
    
    BufferUsageRecordMap_free(&resourceUsage->referencedBuffers);
    ImageUsageRecordMap_free(&resourceUsage->referencedTextures);
    ImageViewUsageSet_free(&resourceUsage->referencedTextureViews);
    BindGroupUsageSet_free(&resourceUsage->referencedBindGroups);
    BindGroupLayoutUsageSet_free(&resourceUsage->referencedBindGroupLayouts);
    SamplerUsageSet_free(&resourceUsage->referencedSamplers);
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