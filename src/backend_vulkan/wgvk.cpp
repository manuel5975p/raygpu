#if SUPPORT_WAYLAND_SURFACE == 1
    #define VK_KHR_wayland_surface 1 // Define set to 1 for clarity
#endif
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
    #include <raygpu.h>
#undef Font
#ifdef TRACELOG
    #undef TRACELOG
#endif
#define TRACELOG(level, ...) wgvkTraceLog(level, __VA_ARGS__)
void wgvkTraceLog(int logType, const char *text, ...);
#include <macros_and_constants.h>
#include <wgvk.h>
#include "vulkan_internals.hpp"
#include <algorithm>
#include <external/VmaUsage.h>
#include <stdarg.h>


// WGVK struct implementations

#include <wgvk_structs_impl.h>

// End WGVK struct implementations
struct key_value_pair{
    void* key;
    uint64_t value;
};

typedef struct ptr_hash_map{
    uint64_t current_size;
    key_value_pair* table;
    union{
        uint64_t current_capacity;
        uint64_t inline_buffer[6];
    };
}ptr_hash_map;
constexpr size_t sdfsdf = sizeof(ptr_hash_map);

#define PHMAPI static inline
#define PHM_INLINE_CAPACITY 3
#define PHM_INITIAL_HEAP_CAPACITY 8
#define PHM_LOAD_FACTOR_NUM 3
#define PHM_LOAD_FACTOR_DEN 4
#define PHM_HASH_MULTIPLIER 0x9E3779B97F4A7C15ULL

static uint64_t phm_hash_key(void* key) {
     return (uintptr_t)key * PHM_HASH_MULTIPLIER;
}

static void phm_grow(ptr_hash_map* map);

PHMAPI void phm_init(ptr_hash_map* map){
    map->current_size = 0;
    map->table = (struct key_value_pair*)map->inline_buffer;
    memset(map->inline_buffer, 0, sizeof(map->inline_buffer));
}

static void phm_insert_entry(struct key_value_pair* table, uint64_t capacity, void* key, uint64_t value) {
    uint64_t cap_mask = capacity - 1;
    uint64_t h = phm_hash_key(key);
    uint64_t index = h & cap_mask;

    while (1) {
        struct key_value_pair* slot = &table[index];
        if (slot->key == NULL) {
            slot->key = key;
            slot->value = value;
            return;
        }
        index = (index + 1) & cap_mask;
    }
}

static void phm_grow(ptr_hash_map* map) {
    int is_inline = (map->table == (struct key_value_pair*)map->inline_buffer);
    uint64_t old_size = map->current_size;
    uint64_t old_capacity = is_inline ? PHM_INLINE_CAPACITY : map->current_capacity;
    struct key_value_pair* old_table_ptr = map->table;

    uint64_t new_capacity = (old_capacity < PHM_INITIAL_HEAP_CAPACITY) ? PHM_INITIAL_HEAP_CAPACITY : old_capacity * 2;
    struct key_value_pair* new_table = (struct key_value_pair*)calloc(new_capacity, sizeof(struct key_value_pair));
    if (!new_table) return; // Allocation failure

    uint64_t limit = is_inline ? old_size : old_capacity;
    for (uint64_t i = 0; i < limit; ++i) {
        struct key_value_pair entry;
        if (is_inline) {
             entry = ((struct key_value_pair*)map->inline_buffer)[i];
        } else {
             entry = old_table_ptr[i];
        }
        if (entry.key != NULL) {
            phm_insert_entry(new_table, new_capacity, entry.key, entry.value);
        }
    }

    if (!is_inline) {
        free(old_table_ptr);
    }

    map->table = new_table;
    map->current_capacity = new_capacity;
    map->current_size = old_size; // Size remains the same after grow
}


PHMAPI void phm_put(ptr_hash_map* map, void* key, uint64_t value){
    if (key == NULL) return; // NULL key not supported

    if (map->table == (struct key_value_pair*)map->inline_buffer) {
        for (uint64_t i = 0; i < map->current_size; i++) {
            if (map->table[i].key == key) {
                map->table[i].value = value;
                return;
            }
        }
        if (map->current_size < PHM_INLINE_CAPACITY) {
            map->table[map->current_size++] = (struct key_value_pair){key, value};
            return;
        } else {
            phm_grow(map);
            // Fall through to heap insertion
        }
    }

    if (map->current_size * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) {
         phm_grow(map);
    }

    uint64_t cap_mask = map->current_capacity - 1;
    uint64_t h = phm_hash_key(key);
    uint64_t index = h & cap_mask;

    while (1) {
        struct key_value_pair* slot = &map->table[index];
        if (slot->key == NULL) {
            slot->key = key;
            slot->value = value;
            map->current_size++;
            return;
        }
        if (slot->key == key) {
            slot->value = value;
            return;
        }
        index = (index + 1) & cap_mask;
    }
}

PHMAPI uint64_t phm_get(ptr_hash_map* map, void* key) {
     if (key == NULL) return 0; // Or some other sentinel for NULL key query

    if (map->table == (struct key_value_pair*)map->inline_buffer) {
        for (uint64_t i = 0; i < map->current_size; i++) {
            if (map->table[i].key == key) {
                return map->table[i].value;
            }
        }
        return 0; // Not found
    }

    if (map->current_capacity == 0) return 0;

    uint64_t cap_mask = map->current_capacity - 1;
    uint64_t h = phm_hash_key(key);
    uint64_t index = h & cap_mask;

    while (1) {
        struct key_value_pair* slot = &map->table[index];
        if (slot->key == NULL) {
            return 0; // Not found
        }
        if (slot->key == key) {
            return slot->value;
        }
        index = (index + 1) & cap_mask;
    }
}

PHMAPI void phm_free(ptr_hash_map* map) {
    if (map->table != (struct key_value_pair*)map->inline_buffer) {
        free(map->table);
    }
    phm_init(map);
}



WGVKSurface wgvkInstanceCreateSurface(WGVKInstance instance, const WGVKSurfaceDescriptor* descriptor){
    rassert(descriptor->nextInChain, "SurfaceDescriptor must have a nextInChain");
    WGVKSurface ret = callocnewpp(WGVKSurfaceImpl);

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
                nullptr,
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
                nullptr,
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


RenderPassLayout GetRenderPassLayout(const WGVKRenderPassDescriptor* rpdesc){
    RenderPassLayout ret{};
    //ret.colorResolveIndex = VK_ATTACHMENT_UNUSED;
    
    if(rpdesc->depthStencilAttachment){
        rassert(rpdesc->depthStencilAttachment->view, "Depth stencil attachment passed but null view");
        ret.depthAttachmentPresent = 1U;
        ret.depthAttachment = AttachmentDescriptor{
            .format = rpdesc->depthStencilAttachment->view->format, 
            .sampleCount = rpdesc->depthStencilAttachment->view->sampleCount,
            .loadop = rpdesc->depthStencilAttachment->depthLoadOp,
            .storeop = rpdesc->depthStencilAttachment->depthStoreOp
        };
    }

    
    ret.colorAttachmentCount = rpdesc->colorAttachmentCount;
    rassert(ret.colorAttachmentCount < max_color_attachments, "Too many color attachments");
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        ret.colorAttachments[i] = AttachmentDescriptor{
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
            //i++;
            //ret.colorResolveIndex = i;
            ret.colorResolveAttachments[i] = AttachmentDescriptor{
                .format = rpdesc->colorAttachments[i].resolveTarget->format, 
                .sampleCount = rpdesc->colorAttachments[i].resolveTarget->sampleCount,
                .loadop = rpdesc->colorAttachments[i].loadOp,
                .storeop = rpdesc->colorAttachments[i].storeOp
            };
        }
    }

    return ret;
}

LayoutedRenderPass LoadRenderPassFromLayout(WGVKDevice device, RenderPassLayout layout){
    auto it = device->renderPassCache.find(layout);
    if(it != device->renderPassCache.end()){
        return it->second;
    }
    TRACELOG(LOG_INFO, "Loading new renderpass");
    auto transformLambda = [](const AttachmentDescriptor& att){
        VkAttachmentDescription ret zeroinit;
        ret.samples    = toVulkanSampleCount(att.sampleCount);
        ret.format     = att.format;
        ret.loadOp     = toVulkanLoadOperation(att.loadop);
        ret.storeOp    = toVulkanStoreOperation(att.storeop);
        ret.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        ret.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        ret.initialLayout  = (att.loadop == LoadOp_Load ? (is__depth(att.format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) : (VK_IMAGE_LAYOUT_UNDEFINED));
        if(is__depth(att.format)){
            ret.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }else{
            ret.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        return ret;
    };
    
    std::vector<VkAttachmentDescription> allAttachments;
    uint32_t depthAttachmentIndex = VK_ATTACHMENT_UNUSED; // index for depth attachment if any
    uint32_t colorResolveIndex = VK_ATTACHMENT_UNUSED; // index for depth attachment if any

    std::transform(
        layout.colorAttachments, 
        layout.colorAttachments + layout.colorAttachmentCount, 
        std::back_inserter(allAttachments), 
        transformLambda
    );
    if(layout.depthAttachmentPresent){
        depthAttachmentIndex = allAttachments.size();
        allAttachments.push_back(transformLambda(layout.depthAttachment));
    }
    //TODO check if there
    if(layout.colorAttachmentCount && layout.colorResolveAttachments[0].format){
        colorResolveIndex = allAttachments.size();
        std::transform(
            layout.colorResolveAttachments, 
            layout.colorResolveAttachments + layout.colorAttachmentCount, 
            std::back_inserter(allAttachments), 
            transformLambda
        );
    }

    
    uint32_t colorAttachmentCount = layout.colorAttachmentCount;
    

    // Set up color attachment references for the subpass.
    VkAttachmentReference colorRefs[max_color_attachments] = {}; // list of color attachments
    uint32_t colorIndex = 0;
    for (uint32_t i = 0; i < layout.colorAttachmentCount; i++) {
        if (!is__depth(layout.colorAttachments[i].format)) {
            colorRefs[colorIndex].attachment = i;
            colorRefs[colorIndex].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ++colorIndex;
        }
    }

    // Set up subpass description.
    VkSubpassDescription subpass zeroinit;
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

    VkAttachmentReference resolveRef = {};
    if (colorResolveIndex != VK_ATTACHMENT_UNUSED) {
        resolveRef.attachment = colorResolveIndex;
        resolveRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        subpass.pResolveAttachments = &resolveRef;
    } else {
        subpass.pResolveAttachments = nullptr;
    }
    

    // Create render pass create info.
    VkRenderPassCreateInfo rpci = {};
    rpci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = allAttachments.size();
    rpci.pAttachments    = allAttachments.data();
    VkFramebufferCreateInfo fbci;

    rpci.subpassCount    = 1;
    rpci.pSubpasses      = &subpass;
    // (Optional: add subpass dependencies if needed.)
    LayoutedRenderPass ret zeroinit;
    ret.allAttachments = std::move(allAttachments);
    VkResult result = vkCreateRenderPass(device->device, &rpci, nullptr, &ret.renderPass);
    // (Handle errors appropriately in production code)
    if(result == VK_SUCCESS){
        device->renderPassCache.emplace(layout, ret);
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
    constexpr size_t MAX_TRACELOG_MSG_LENGTH = 16384;
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };
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
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
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
    constexpr uint32_t maxEnabledExtensions = 16;

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
    vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, nullptr);
    VkPhysicalDevice* pds = (VkPhysicalDevice*)RL_CALLOC(physicalDeviceCount, sizeof(VkPhysicalDevice));
    VkResult result = vkEnumeratePhysicalDevices(userdata->instance->instance, &physicalDeviceCount, pds);
    if(result != VK_SUCCESS){
        const char res[] = "vkEnumeratePhysicalDevices failed";
        userdata->info.callback(WGVKRequestAdapterStatus_Unavailable, nullptr, WGVKStringView{res, sizeof(res) - 1},userdata->info.userdata1, userdata->info.userdata2);
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
    WGVKAdapter adapter = callocnewpp(WGVKAdapterImpl);
    adapter->instance = userdata->instance;
    adapter->physicalDevice = pds[i];
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &adapter->memProperties);
    uint32_t QueueFamilyPropertyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &QueueFamilyPropertyCount, nullptr);
    VkQueueFamilyProperties* props = (VkQueueFamilyProperties*)RL_CALLOC(QueueFamilyPropertyCount, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(adapter->physicalDevice, &QueueFamilyPropertyCount, props);
    adapter->queueIndices = QueueIndices{
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
    std::pair<WGVKDevice, WGVKQueue> ret{};
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
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamilyIndex = 0;queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
        uint32_t queueFamily = queueFamilies[queueFamilyIndex]; 
        VkDeviceQueueCreateInfo queueCreateInfo zeroinit;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, nullptr, &deviceExtensionCount, nullptr);
    VkExtensionProperties* deprops = (VkExtensionProperties*)RL_CALLOC(deviceExtensionCount, sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(adapter->physicalDevice, nullptr, &deviceExtensionCount, deprops);
    
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
    constexpr uint32_t deviceExtensionsToLookForCount = sizeof(deviceExtensionsToLookFor) / sizeof(const char*);
    
    const char* deviceExtensionsFound[deviceExtensionsToLookForCount];
    uint32_t extInsertIndex = 0;
    for(uint32_t i = 0;i < deviceExtensionsToLookForCount;i++){
        for(uint32_t j = 0;j < deviceExtensionCount;j++){
            if(strcmp(deviceExtensionsToLookFor[i], deprops[j].extensionName) == 0){
                deviceExtensionsFound[extInsertIndex++] = deviceExtensionsToLookFor[i];
            }
        }
    }
    // Specify device features

    {
        VkPhysicalDeviceExtendedDynamicState3PropertiesEXT props = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT,
            .pNext = nullptr,
            .dynamicPrimitiveTopologyUnrestricted = VK_TRUE
        };
    }
    
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

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

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
    ret.first = callocnewpp(WGVKDeviceImpl);
    ret.second = callocnewpp(WGVKQueueImpl);

    VkResult dcresult = vkCreateDevice(adapter->physicalDevice, &createInfo, nullptr, &(ret.first->device));
    if (dcresult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create logical device!");
    } else {
        //TRACELOG(LOG_INFO, "Successfully created logical device");
        volkLoadDevice(ret.first->device);
    }

    // Retrieve and assign queues
    
    auto& indices = adapter->queueIndices;
    vkGetDeviceQueue(ret.first->device, indices.graphicsIndex, 0, &ret.second->graphicsQueue);
    #ifndef FORCE_HEADLESS
    vkGetDeviceQueue(ret.first->device, indices.presentIndex, 0, &ret.second->presentQueue);
    #endif
    if (indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex) {
        vkGetDeviceQueue(ret.first->device, indices.computeIndex, 0, &ret.second->computeQueue);
    } else {
        // If compute Index is same as graphics or present, assign accordingly
        if (indices.computeIndex == indices.graphicsIndex) {
            ret.second->computeQueue = ret.second->graphicsQueue;
        } else if (indices.computeIndex == indices.presentIndex) {
            ret.second->computeQueue = ret.second->presentQueue;
        }
    }
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    cedesc.recyclable = true;
    for(uint32_t i = 0;i < framesInFlight;i++){
        WGVKDevice device = ret.first;
        ret.first->frameCaches[i].finalTransitionSemaphore = CreateSemaphoreD(device->device);
        VkCommandPoolCreateInfo pci zeroinit;
        pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        vkCreateCommandPool(ret.first->device, &pci, nullptr, &ret.first->frameCaches[i].commandPool);
        
        VkCommandBufferAllocateInfo cbai zeroinit;
        cbai.commandBufferCount = 1;
        cbai.commandPool = device->frameCaches[i].commandPool;
        cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        vkAllocateCommandBuffers(device->device, &cbai, &device->frameCaches[i].finalTransitionBuffer);
        VkFenceCreateInfo sci zeroinit;
        VkFence ret zeroinit;
        sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkResult res = vkCreateFence(device->device, &sci, nullptr, &device->frameCaches[i].finalTransitionFence);
    }
    ret.second->presubmitCache = wgvkDeviceCreateCommandEncoder(ret.first, &cedesc);

    VmaAllocatorCreateInfo aci zeroinit;
    aci.instance = adapter->instance->instance;
    aci.physicalDevice = adapter->physicalDevice;
    aci.device = ret.first->device;
    #if VULKAN_ENABLE_RAYTRACING == 1
    aci.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    #endif
    VkDeviceSize limit = (uint64_t(1) << 30);
    aci.preferredLargeHeapBlockSize = 1 << 15;
    VkPhysicalDeviceMemoryProperties memoryProperties zeroinit;
    vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &memoryProperties);
    std::vector<VkDeviceSize> heapsizes(memoryProperties.memoryHeapCount, limit);
    aci.pHeapSizeLimit = heapsizes.data();

    VmaDeviceMemoryCallbacks callbacks{
        /// Optional, can be null.
        .pfnAllocate = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
            TRACELOG(LOG_WARNING, "Allocating %llu of memory type %u", size, type);
        },
        .pfnFree = [](VmaAllocator allocator, uint32_t type, VkDeviceMemory, VkDeviceSize size, void * _Nullable){
            TRACELOG(LOG_WARNING, "Freeing %llu of memory type %u", size, type);
        }
    };
    aci.pDeviceMemoryCallbacks = &callbacks;
    VmaVulkanFunctions vmaVulkanFunctions zeroinit;
    vmaVulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaVulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    aci.pVulkanFunctions = &vmaVulkanFunctions;
    VkResult allocatorCreateResult = vmaCreateAllocator(&aci, &ret.first->allocator);

    if(allocatorCreateResult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating the allocator: %d", (int)allocatorCreateResult);
    }
    for(uint32_t i = 0;i < framesInFlight;i++){
        ret.second->syncState[i].semaphores.resize(100);
        for(uint32_t j = 0;j < ret.second->syncState[i].semaphores.size();j++){
            ret.second->syncState[i].semaphores[j] = CreateSemaphoreD(ret.first->device);
        }
        ret.second->syncState[i].acquireImageSemaphore = CreateSemaphoreD(ret.first->device);

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
        vmaCreatePool(ret.first->allocator, &vpci, &ret.first->aligned_hostVisiblePool);
        // TODO
    }

    {
        auto [device, queue] = ret;
        device->queue = queue;
        device->adapter = adapter;
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
        device->adapter = adapter;
        queue->device = device;
    }

    return ret.first;
}


extern "C" WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const WGVKBufferDescriptor* desc){
    //vmaCreateAllocator(const VmaAllocatorCreateInfo * _Nonnull pCreateInfo, VmaAllocator  _Nullable * _Nonnull pAllocator)
    WGVKBuffer wgvkBuffer = callocnewpp(WGVKBufferImpl);

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
        propertyToFind = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //propertyToFind = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }
    VmaAllocationCreateInfo vallocInfo zeroinit;
    vallocInfo.preferredFlags = propertyToFind;

    VmaAllocation allocation zeroinit;
    VmaAllocationInfo allocationInfo zeroinit;

    VkResult vmabufferCreateResult = vmaCreateBuffer(device->allocator, &bufferDesc, &vallocInfo, &wgvkBuffer->buffer, &allocation, &allocationInfo);

    
    if(vmabufferCreateResult != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Could not allocate buffer: %s", vkErrorString(vmabufferCreateResult));
        wgvkBuffer->~WGVKBufferImpl();
        std::free(wgvkBuffer);
        return nullptr;
    }
    wgvkBuffer->allocation = allocation;
    
    wgvkBuffer->memoryProperties = propertyToFind;

    if(desc->usage & BufferUsage_ShaderDeviceAddress){
        VkBufferDeviceAddressInfo bdai zeroinit;
        bdai.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR;
        bdai.buffer = wgvkBuffer->buffer;
        wgvkBuffer->address = vkGetBufferDeviceAddress(device->device, &bdai);
    }
    return wgvkBuffer;
}
std::unordered_set<VkDeviceMemory> mappedMemories;

extern "C" void wgvkBufferMap(WGVKBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data){
    
    vmaMapMemory(buffer->device->allocator, buffer->allocation, data);
    //VmaAllocationInfo allocationInfo zeroinit;
    //vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &allocationInfo);
    //uint64_t baseOffset = allocationInfo.offset;
    //
    //VkDeviceMemory bufferMemory = allocationInfo.deviceMemory;
    //rassert(mappedMemories.find(bufferMemory) == mappedMemories.end(), "The VkDeviceMemory of this buffer is already mapped");
    //VkResult result = vkMapMemory(buffer->device->device, bufferMemory, baseOffset + offset, size, 0, data);
    //if(result != VK_SUCCESS){
    //    *data = nullptr;
    //}
    //else{
    //    mappedMemories.insert(bufferMemory);
    //}
}

extern "C" void wgvkBufferUnmap(WGVKBuffer buffer){
    vmaUnmapMemory(buffer->device->allocator, buffer->allocation);
    //VmaAllocationInfo allocationInfo zeroinit;
    //vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &allocationInfo);
    //vkUnmapMemory(buffer->device->device, allocationInfo.deviceMemory);
    //mappedMemories.erase(allocationInfo.deviceMemory);
}
extern "C" size_t wgvkBufferGetSize(WGVKBuffer buffer){
    VmaAllocationInfo info zeroinit;
    vmaGetAllocationInfo(buffer->device->allocator, buffer->allocation, &info);
    return info.size;
}

extern "C" void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, const void* data, size_t size){
    void* mappedMemory = nullptr;
    if(buffer->memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
        void* mappedMemory = nullptr;
        wgvkBufferMap(buffer, MapMode_Write, bufferOffset, size, &mappedMemory);
        
        if (mappedMemory != nullptr) {
            // Memory is host mappable: copy data and unmap.
            std::memcpy(mappedMemory, data, size);
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

extern "C" void wgvkQueueWriteTexture(WGVKQueue cSelf, const WGVKTexelCopyTextureInfo* destination, const void* data, size_t dataSize, const WGVKTexelCopyBufferLayout* dataLayout, const WGVKExtent3D* writeSize){

    WGVKBufferDescriptor bdesc zeroinit;
    bdesc.size = dataSize;
    bdesc.usage = BufferUsage_CopySrc | BufferUsage_MapWrite;
    WGVKBuffer stagingBuffer = wgvkDeviceCreateBuffer(cSelf->device, &bdesc);
    void* mappedMemory = nullptr;
    wgvkBufferMap(stagingBuffer, MapMode_Write, 0, dataSize, &mappedMemory);
    if(mappedMemory != nullptr){
        std::memcpy(mappedMemory, data, dataSize);
        wgvkBufferUnmap(stagingBuffer);
    }
    WGVKCommandEncoder enkoder = wgvkDeviceCreateCommandEncoder(cSelf->device, nullptr);
    WGVKTexelCopyBufferInfo source;
    source.buffer = stagingBuffer;
    source.layout = *dataLayout;
    enkoder->initializeOrTransition(destination->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    //if(destination->texture->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
    //    wgvkCommandEncoderTransitionTextureLayout(enkoder, destination->texture, destination->texture->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //}
    wgvkCommandEncoderCopyBufferToTexture(enkoder, &source, destination, writeSize);
    auto puffer = wgvkCommandEncoderFinish(enkoder);

    wgvkQueueSubmit(cSelf, 1, &puffer);
    wgvkReleaseCommandEncoder(enkoder);
    wgvkReleaseCommandBuffer(puffer);

    wgvkBufferRelease(stagingBuffer);
}

extern "C" WGVKTexture wgvkDeviceCreateTexture(WGVKDevice device, const WGVKTextureDescriptor* descriptor){
    VkDeviceMemory imageMemory zeroinit;
    // Adjust usage flags based on format (e.g., depth formats might need different usages)
    WGVKTexture ret = callocnewpp(WGVKTextureImpl);

    VkImageCreateInfo imageInfo zeroinit;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = { descriptor->size.width, descriptor->size.height, descriptor->size.depthOrArrayLayers };
    imageInfo.mipLevels = descriptor->mipLevelCount;
    imageInfo.arrayLayers = 1;
    imageInfo.format = toVulkanPixelFormat(descriptor->format);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = toVulkanTextureUsage(descriptor->usage, descriptor->format);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = toVulkanSampleCount(descriptor->sampleCount);
    
    VkImage image zeroinit;
    if (vkCreateImage(device->device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        TRACELOG(LOG_FATAL, "Failed to create image!");
    
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device->device, image, &memReq);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        device->adapter->physicalDevice,
        memReq.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    
    if (vkAllocateMemory(device->device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Failed to allocate image memory!");
    }
    vkBindImageMemory(device->device, image, imageMemory, 0);

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

extern "C" void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup wvBindGroup, const WGVKBindGroupDescriptor* bgdesc){
    
    rassert(bgdesc->layout != nullptr, "WGVKBindGroupDescriptor::layout is null");
    
    if(wvBindGroup->pool == nullptr){
        wvBindGroup->layout = bgdesc->layout;
        VkDescriptorPoolCreateInfo dpci{};
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.maxSets = 1;

        std::unordered_map<VkDescriptorType, uint32_t> counts;
        for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
            ++counts[toVulkanResourceType(bgdesc->layout->entries[i].type)];
        }
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(counts.size());
        for(const auto& [t, s] : counts){
            sizes.push_back(VkDescriptorPoolSize{.type = t, .descriptorCount = s});
        }

        dpci.poolSizeCount = sizes.size();
        dpci.pPoolSizes = sizes.data();
        dpci.maxSets = 1;
        vkCreateDescriptorPool(device->device, &dpci, nullptr, &wvBindGroup->pool);

        //VkCopyDescriptorSet copy{};
        //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        VkDescriptorSetAllocateInfo dsai{};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.descriptorPool = wvBindGroup->pool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = (VkDescriptorSetLayout*)&bgdesc->layout->layout;
        vkAllocateDescriptorSets(device->device, &dsai, &wvBindGroup->set);
    }
    ResourceUsage newResourceUsage;
    for(uint32_t i = 0;i < bgdesc->entryCount;i++){
        auto& entry = bgdesc->entries[i];
        if(entry.buffer){
            newResourceUsage.track((WGVKBuffer)entry.buffer);
            //wgvkBufferAddRef((WGVKBuffer)entry.buffer);
        }
        else if(entry.textureView){
            uniform_type utype = bgdesc->layout->entries[i].type;
            if(utype == storage_texture2d || utype == storage_texture3d || utype == storage_texture2d_array){
                newResourceUsage.track((WGVKTextureView)entry.textureView, TextureUsage_StorageBinding);
            }
            else if(utype == texture2d || utype == texture3d || utype == texture2d_array){
                newResourceUsage.track((WGVKTextureView)entry.textureView, TextureUsage_TextureBinding);
            }
            //wgvkTextureViewAddRef((WGVKTextureView)entry.textureView);
        }
        else if(entry.sampler){
            newResourceUsage.track(entry.sampler);
        }
    }
    wvBindGroup->resourceUsage.releaseAllAndClear();
    wvBindGroup->resourceUsage = std::move(newResourceUsage);

    uint32_t count = bgdesc->entryCount;
    small_vector<VkWriteDescriptorSet> writes(count, VkWriteDescriptorSet{});
    small_vector<VkDescriptorBufferInfo> bufferInfos(count, VkDescriptorBufferInfo{});
    small_vector<VkDescriptorImageInfo> imageInfos(count, VkDescriptorImageInfo{});
    small_vector<VkWriteDescriptorSetAccelerationStructureKHR> accelStructInfos(count, VkWriteDescriptorSetAccelerationStructureKHR{});

    for(uint32_t i = 0;i < count;i++){
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uint32_t binding = bgdesc->entries[i].binding;
        writes[i].dstBinding = binding;
        writes[i].dstSet = wvBindGroup->set;
        const ResourceTypeDescriptor& entryi = bgdesc->layout->entries[i];
        writes[i].descriptorType = toVulkanResourceType(bgdesc->layout->entries[i].type);
        writes[i].descriptorCount = 1;

        if(entryi.type == uniform_buffer || entryi.type == storage_buffer){
            WGVKBuffer bufferOfThatEntry = (WGVKBuffer)bgdesc->entries[i].buffer;
            wvBindGroup->resourceUsage.track(bufferOfThatEntry);
            bufferInfos[i].buffer = bufferOfThatEntry->buffer;
            bufferInfos[i].offset = bgdesc->entries[i].offset;
            bufferInfos[i].range =  bgdesc->entries[i].size;
            writes[i].pBufferInfo = bufferInfos.data() + i;
        }

        if(entryi.type == texture2d || entryi.type == texture3d){
            wvBindGroup->resourceUsage.track((WGVKTextureView)bgdesc->entries[i].textureView, TextureUsage_TextureBinding);
            imageInfos[i].imageView = ((WGVKTextureView)bgdesc->entries[i].textureView)->view;
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            writes[i].pImageInfo = imageInfos.data() + i;
        }
        if(entryi.type == storage_texture2d || entryi.type == storage_texture3d){
            wvBindGroup->resourceUsage.track((WGVKTextureView)bgdesc->entries[i].textureView, TextureUsage_StorageBinding);
            imageInfos[i].imageView = ((WGVKTextureView)bgdesc->entries[i].textureView)->view;
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            writes[i].pImageInfo = imageInfos.data() + i;
        }

        if(entryi.type == texture_sampler){
            VkSampler vksampler = bgdesc->entries[i].sampler->sampler;
            imageInfos[i].sampler = vksampler;
            writes[i].pImageInfo = imageInfos.data() + i;
        }
        if(entryi.type == acceleration_structure){

            //wvBindGroup->resourceUsage.track((WGVKAccelerationStructure)bgdesc->entries[i].accelerationStructure);
            accelStructInfos[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
            accelStructInfos[i].accelerationStructureCount = 1;
            accelStructInfos[i].pAccelerationStructures = &bgdesc->entries[i].accelerationStructure->accelerationStructure;
            writes[i].pNext = &accelStructInfos[i];
        }
    }

    vkUpdateDescriptorSets(device->device, writes.size(), writes.data(), 0, nullptr);
}



extern "C" WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc){
    rassert(bgdesc->layout != nullptr, "WGVKBindGroupDescriptor::layout is null");
    
    WGVKBindGroup ret = callocnewpp(WGVKBindGroupImpl);
    ret->refCount = 1;

    ret->device = device;
    ret->cacheIndex = device->submittedFrames % framesInFlight;

    PerframeCache& fcache = device->frameCaches[ret->cacheIndex];
    auto it = fcache.bindGroupCache.find(bgdesc->layout);
    if(it == fcache.bindGroupCache.end() || it->second.size() == 0){ //Cache miss
        TRACELOG(LOG_INFO, "Allocating new VkDescriptorPool and -Set");
        VkDescriptorPoolCreateInfo dpci{};
        dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        dpci.maxSets = 1;

        std::unordered_map<VkDescriptorType, uint32_t> counts;
        for(uint32_t i = 0;i < bgdesc->layout->entryCount;i++){
            ++counts[toVulkanResourceType(bgdesc->layout->entries[i].type)];
        }
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(counts.size());
        for(const auto& [t, s] : counts){
            sizes.push_back(VkDescriptorPoolSize{.type = t, .descriptorCount = s});
        }

        dpci.poolSizeCount = sizes.size();
        dpci.pPoolSizes = sizes.data();
        dpci.maxSets = 1;
        vkCreateDescriptorPool(device->device, &dpci, nullptr, &ret->pool);

        //VkCopyDescriptorSet copy{};
        //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;

        VkDescriptorSetAllocateInfo dsai{};
        dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        dsai.descriptorPool = ret->pool;
        dsai.descriptorSetCount = 1;
        dsai.pSetLayouts = (VkDescriptorSetLayout*)&bgdesc->layout->layout;
        vkAllocateDescriptorSets(device->device, &dsai, &ret->set);
    }
    else{
        //TRACELOG(LOG_INFO, "Reusing Descriptorset");
        ret->pool = it->second.back().first;
        ret->set = it->second.back().second;
        it->second.pop_back();
    }
    wgvkWriteBindGroup(device, ret, bgdesc);
    ret->layout = bgdesc->layout;
    ++ret->layout->refCount;
    rassert(ret->layout != nullptr, "ret->layout is nullptr");
    return ret;
}
extern "C" WGVKBindGroupLayout wgvkDeviceCreateBindGroupLayout(WGVKDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount){
    WGVKBindGroupLayout ret = callocnewpp(WGVKBindGroupLayoutImpl);
    ret->refCount = 1;
    ret->device = device;
    ret->entryCount = entryCount;
    VkDescriptorSetLayoutCreateInfo slci zeroinit;
    slci.bindingCount = entryCount;
    small_vector<VkDescriptorSetLayoutBinding> bindings(slci.bindingCount);
    for(uint32_t i = 0;i < slci.bindingCount;i++){
        bindings[i].descriptorCount = 1;
        bindings[i].binding = entries[i].location;
        bindings[i].descriptorType = toVulkanResourceType(entries[i].type);
        if(entries[i].visibility == 0){    
            TRACELOG(LOG_WARNING, "Empty visibility detected, falling back to Vertex | Fragment | Compute mask");
            bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        }

        else{
            bindings[i].stageFlags = toVulkanShaderStageBits(entries[i].visibility);
        }
    }

    slci.pBindings = bindings.data();
    slci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkCreateDescriptorSetLayout(device->device, &slci, nullptr, &ret->layout);
    ResourceTypeDescriptor* entriesCopy = (ResourceTypeDescriptor*)std::calloc(entryCount, sizeof(ResourceTypeDescriptor));
    if(entryCount > 0)
        std::memcpy(entriesCopy, entries, entryCount * sizeof(ResourceTypeDescriptor));
    ret->entries = entriesCopy;
    
    return ret;
}
extern "C" void wgvkReleasePipelineLayout(WGVKPipelineLayout pllayout){
    for(uint32_t i = 0;i < pllayout->bindGroupLayoutCount;i++){
        wgvkBindGroupLayoutRelease(pllayout->bindGroupLayouts[i]);
    }
    std::free((void*)pllayout->bindGroupLayouts);
}
extern "C" WGVKPipelineLayout wgvkDeviceCreatePipelineLayout(WGVKDevice device, const WGVKPipelineLayoutDescriptor* pldesc){
    WGVKPipelineLayout ret = callocnewpp(WGVKPipelineLayoutImpl);
    rassert(ret->bindGroupLayoutCount <= 8, "Only supports up to 8 BindGroupLayouts");
    ret->bindGroupLayoutCount = pldesc->bindGroupLayoutCount;
    ret->bindGroupLayouts = (WGVKBindGroupLayout*)RL_CALLOC(pldesc->bindGroupLayoutCount, sizeof(void*));
    std::memcpy((void*)ret->bindGroupLayouts, (void*)pldesc->bindGroupLayouts, pldesc->bindGroupLayoutCount * sizeof(void*));
    VkDescriptorSetLayout dslayouts[8] zeroinit;
    for(uint32_t i = 0;i < ret->bindGroupLayoutCount;i++){
        wgvkBindGroupLayoutAddRef(ret->bindGroupLayouts[i]);
        dslayouts[i] = ret->bindGroupLayouts[i]->layout;
    }
    VkPipelineLayoutCreateInfo lci zeroinit;
    lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lci.pSetLayouts = dslayouts;
    lci.setLayoutCount = ret->bindGroupLayoutCount;
    VkResult res = vkCreatePipelineLayout(device->device, &lci, nullptr, &ret->layout);
    if(res != VK_SUCCESS){
        wgvkReleasePipelineLayout(ret);
        ret = nullptr;
    }
    return ret;
}


extern "C" WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* desc){
    WGVKCommandEncoder ret = callocnewpp(WGVKCommandEncoderImpl);
    //TRACELOG(LOG_INFO, "Creating new commandencoder");
    //§VkCommandPoolCreateInfo pci{};
    //§pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    //§pci.flags = desc->recyclable ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    //§ret->recyclable = desc->recyclable;
    ret->cacheIndex = device->submittedFrames % framesInFlight;
    PerframeCache& cache = device->frameCaches[ret->cacheIndex];
    ret->device = device;
    ret->movedFrom = 0;
    //vkCreateCommandPool(device->device, &pci, nullptr, &cache.commandPool);
    if(device->frameCaches[ret->cacheIndex].commandBuffers.empty()){
        VkCommandBufferAllocateInfo bai{};
        bai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bai.commandPool = cache.commandPool;
        bai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        bai.commandBufferCount = 1;
        vkAllocateCommandBuffers(device->device, &bai, &ret->buffer);
        //TRACELOG(LOG_INFO, "Allocating new command buffer");
    }
    else{
        //TRACELOG(LOG_INFO, "Reusing");
        ret->buffer = device->frameCaches[ret->cacheIndex].commandBuffers.back();
        device->frameCaches[ret->cacheIndex].commandBuffers.pop_back();
        //vkResetCommandBuffer(ret->buffer, 0);
    }

    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(ret->buffer, &bbi);
    
    return ret;
}
extern "C" void wgvkQueueTransitionLayout(WGVKQueue cSelf, WGVKTexture texture, VkImageLayout from, VkImageLayout to){
    EncodeTransitionImageLayout(cSelf->presubmitCache->buffer, from, to, texture);
    cSelf->presubmitCache->resourceUsage.track(texture);
}
extern "C" WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor){
    
    VkImageViewCreateInfo ivci{};
    ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ivci.image = texture->image;

    VkComponentMapping cm{
        .r = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .g = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .b = VK_COMPONENT_SWIZZLE_IDENTITY, 
        .a = VK_COMPONENT_SWIZZLE_IDENTITY
    };
    ivci.components = cm;
    ivci.viewType = toVulkanTextureViewDimension(descriptor->dimension);
    ivci.format = toVulkanPixelFormat(descriptor->format);
    
    VkImageSubresourceRange sr{
        .aspectMask = toVulkanAspectMask(descriptor->aspect),
        .baseMipLevel = descriptor->baseMipLevel,
        .levelCount = descriptor->mipLevelCount,
        .baseArrayLayer = descriptor->baseArrayLayer,
        .layerCount = descriptor->arrayLayerCount
    };
    if(!is__depth(ivci.format)){
        sr.aspectMask &= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    ivci.subresourceRange = sr;
    WGVKTextureView ret = callocnewpp(WGVKTextureViewImpl);
    ret->refCount = 1;
    vkCreateImageView(texture->device->device, &ivci, nullptr, &ret->view);
    ret->format = ivci.format;
    ret->texture = texture;
    ++texture->refCount;
    ret->width = texture->width;
    ret->height = texture->height;
    ret->sampleCount = texture->sampleCount;
    ret->depthOrArrayLayers = texture->depthOrArrayLayers;
    return ret;
}

extern "C" WGVKRenderPassEncoder wgvkCommandEncoderBeginRenderPass(WGVKCommandEncoder enc, const WGVKRenderPassDescriptor* rpdesc){
    WGVKRenderPassEncoder ret = callocnewpp(WGVKRenderPassEncoderImpl);

    ret->refCount = 2; //One for WGVKRenderPassEncoder the other for the command buffer
    
    enc->referencedRPs.insert(ret);
    ret->device = enc->device;
    ret->cmdBuffer = enc->buffer;
    ret->cmdEncoder = enc;
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    VkRenderingInfo info zeroinit;
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.colorAttachmentCount = rpdesc->colorAttachmentCount;

    VkRenderingAttachmentInfo colorAttachments[max_color_attachments] zeroinit;
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++){
        colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachments[i].clearValue.color.float32[0] = rpdesc->colorAttachments[i].clearValue.r;
        colorAttachments[i].clearValue.color.float32[1] = rpdesc->colorAttachments[i].clearValue.g;
        colorAttachments[i].clearValue.color.float32[2] = rpdesc->colorAttachments[i].clearValue.b;
        colorAttachments[i].clearValue.color.float32[3] = rpdesc->colorAttachments[i].clearValue.a;
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].imageView = rpdesc->colorAttachments[i].view->view;
        if(rpdesc->colorAttachments[i].resolveTarget){
            colorAttachments[i].resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachments[i].resolveImageView = rpdesc->colorAttachments[i].resolveTarget->view;
            colorAttachments[i].resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
            ret->resourceUsage.track(rpdesc->colorAttachments[i].resolveTarget, TextureUsage_RenderAttachment);
        }
        ret->resourceUsage.track(rpdesc->colorAttachments[i].view, TextureUsage_RenderAttachment);
        colorAttachments[i].loadOp = toVulkanLoadOperation(rpdesc->colorAttachments[i].loadOp);
        colorAttachments[i].storeOp = toVulkanStoreOperation(rpdesc->colorAttachments[i].storeOp);
    }
    info.pColorAttachments = colorAttachments;
    VkRenderingAttachmentInfo depthAttachment zeroinit;
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.clearValue.depthStencil.depth = rpdesc->depthStencilAttachment->depthClearValue;

    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.imageView = rpdesc->depthStencilAttachment->view->view;
    ret->resourceUsage.track(rpdesc->depthStencilAttachment->view, TextureUsage_RenderAttachment);
    depthAttachment.loadOp = toVulkanLoadOperation(rpdesc->depthStencilAttachment->depthLoadOp);
    depthAttachment.storeOp = toVulkanStoreOperation(rpdesc->depthStencilAttachment->depthStoreOp);
    info.pDepthAttachment = &depthAttachment;
    info.layerCount = 1;
    info.renderArea = VkRect2D{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{rpdesc->colorAttachments[0].view->width, rpdesc->colorAttachments[0].view->height}
    };
    uint64_t v = (uint64_t)reinterpret_cast<PFN_vkCmdBeginRendering>(vkCmdBeginRendering);
    vkCmdBeginRendering(ret->cmdBuffer, &info);
    #else
    RenderPassLayout rplayout = GetRenderPassLayout(rpdesc);
    VkRenderPassBeginInfo rpbi{};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    LayoutedRenderPass frp = LoadRenderPassFromLayout(enc->device, rplayout);
    ret->renderPass = frp.renderPass;
    auto toVkCV = [](const DColor& c){
        VkClearValue cv zeroinit;
        cv.color.float32[0] = static_cast<float>(c.r);
        cv.color.float32[1] = static_cast<float>(c.g);
        cv.color.float32[2] = static_cast<float>(c.b);
        cv.color.float32[3] = static_cast<float>(c.a);
        return cv;
    };
    std::vector<VkImageView> attachmentViews(frp.allAttachments.size());
    std::vector<VkClearValue> clearValues(frp.allAttachments.size(), VkClearValue{});
    
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

    VkFramebufferCreateInfo fbci zeroinit;
    fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbci.pAttachments = attachmentViews.data();
    fbci.attachmentCount = attachmentViews.size();
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++)
    if(rpdesc->colorAttachments[i].view){
        ret->resourceUsage.track(rpdesc->colorAttachments[i].view, TextureUsage_RenderAttachment);
        ret->cmdEncoder->initializeOrTransition(rpdesc->colorAttachments->view->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    for(uint32_t i = 0;i < rpdesc->colorAttachmentCount;i++)
        if(rpdesc->colorAttachments[i].resolveTarget){
            ret->resourceUsage.track(rpdesc->colorAttachments[i].resolveTarget, TextureUsage_RenderAttachment);
            ret->cmdEncoder->initializeOrTransition(rpdesc->colorAttachments->resolveTarget->texture, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        }
    if(rpdesc->depthStencilAttachment){
        ret->resourceUsage.track(rpdesc->depthStencilAttachment->view, TextureUsage_RenderAttachment);
        ret->cmdEncoder->initializeOrTransition(rpdesc->depthStencilAttachment->view->texture, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    fbci.width = rpdesc->colorAttachments[0].view->width;
    fbci.height = rpdesc->colorAttachments[0].view->height;
    fbci.layers = 1;
    
    fbci.renderPass = ret->renderPass;
    VkResult fbresult = vkCreateFramebuffer(enc->device->device, &fbci, nullptr, &ret->frameBuffer);
    if(fbresult != VK_SUCCESS){
        TRACELOG(LOG_FATAL, "Error creating framebuffer: %d", (int)fbresult);
    }
    rpbi.renderPass = ret->renderPass;
    rpbi.renderArea = VkRect2D{
        .offset = VkOffset2D{0, 0},
        .extent = VkExtent2D{rpdesc->colorAttachments[0].view->width, rpdesc->colorAttachments[0].view->height}
    };

    rpbi.framebuffer = ret->frameBuffer;
    
    
    rpbi.clearValueCount = clearValues.size();
    rpbi.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(enc->buffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    ret->cmdBuffer = enc->buffer;
    ret->cmdEncoder = enc;
    #endif
    return ret;
}

extern "C" void wgvkRenderPassEncoderEnd(WGVKRenderPassEncoder renderPassEncoder){
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    //vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions)
    vkCmdEndRendering(renderPassEncoder->cmdBuffer);
    #else
    vkCmdEndRenderPass(renderPassEncoder->cmdBuffer);
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
extern "C" WGVKCommandBuffer wgvkCommandEncoderFinish(WGVKCommandEncoder commandEncoder){
    WGVKCommandBuffer ret = callocnewpp(WGVKCommandBufferImpl);
    ret->refCount = 1;
    rassert(commandEncoder->movedFrom == 0, "Command encoder is already invalidated");
    commandEncoder->movedFrom = 1;
    vkEndCommandBuffer(commandEncoder->buffer);
    ret->referencedRPs = std::move(commandEncoder->referencedRPs);
    ret->referencedCPs = std::move(commandEncoder->referencedCPs);
    ret->referencedRTs = std::move(commandEncoder->referencedRTs);
    ret->resourceUsage = std::move(commandEncoder->resourceUsage);
    
    //new (&commandEncoder->referencedRPs)(ref_holder<WGVKRenderPassEncoder>){};
    //new (&commandEncoder->referencedCPs)(ref_holder<WGVKComputePassEncoder>){};
    //new (&commandEncoder->resourceUsage)(ResourceUsage){};
    ret->cacheIndex = commandEncoder->cacheIndex;
    ret->buffer = commandEncoder->buffer;
    ret->device = commandEncoder->device;
    commandEncoder->buffer = nullptr;
    return ret;
}

extern "C" void wgvkQueueSubmit(WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers){
    VkSubmitInfo si zeroinit;
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = commandCount + 1;
    small_vector<VkCommandBuffer> submittable(commandCount + 1);
    small_vector<WGVKCommandBuffer> submittableWGVK(commandCount + 1);

    auto& pscache = queue->presubmitCache;
    {
        bool needs_another_one = false;
        for(auto [lex, layouts] : pscache->resourceUsage.entryAndFinalLayouts){
            rg_trap();
            needs_another_one = true;
        }
        //if(needs_another_one){
        //    WGVKCommandEncoder cend = wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor *desc)
        //}
    }

    {
        auto& sbuffer = buffers[0];
        for(auto& [tex, layouts] : sbuffer->resourceUsage.entryAndFinalLayouts){
            auto it = pscache->resourceUsage.entryAndFinalLayouts.find(tex);
            if(it != pscache->resourceUsage.entryAndFinalLayouts.end()){
                if(it->second.second != layouts.first){
                    wgvkCommandEncoderTransitionTextureLayout(pscache, tex, it->second.second, layouts.first);
                }
            }
            else if(tex->layout != layouts.first){            
                wgvkCommandEncoderTransitionTextureLayout(pscache, tex, tex->layout, layouts.first);
            }
        }
    }
    
    
    WGVKCommandBuffer cachebuffer = wgvkCommandEncoderFinish(queue->presubmitCache);
    submittable[0] = cachebuffer->buffer;
    for(size_t i = 0;i < commandCount;i++){
        submittable[i + 1] = buffers[i]->buffer;
    }

    submittableWGVK[0] = cachebuffer;
    for(size_t i = 0;i < commandCount;i++){
        submittableWGVK[i + 1] = buffers[i];
    }
    for(uint32_t i = 0;i < submittableWGVK.size();i++){
        for(auto [tex, layoutpair] : submittableWGVK[i]->resourceUsage.entryAndFinalLayouts){
            tex->layout = layoutpair.second;
        }
    }

    si.pCommandBuffers = submittable.data();
    VkFence fence = VK_NULL_HANDLE;
    
    si.commandBufferCount = submittable.size();
    si.pCommandBuffers = submittable.data();
    
    VkPipelineStageFlags wsmaskp = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    const uint64_t frameCount = queue->device->submittedFrames;
    const uint32_t cacheIndex = frameCount % framesInFlight;
    int submitResult = 0;
    
    for(uint32_t i = 0;i < submittable.size();i++){
        small_vector<VkSemaphore> waitSemaphores;

        if(queue->syncState[cacheIndex].acquireImageSemaphoreSignalled){
            waitSemaphores.push_back(queue->syncState[cacheIndex].acquireImageSemaphore);   
            queue->syncState[cacheIndex].acquireImageSemaphoreSignalled = false;
        }
        if(queue->syncState[cacheIndex].submits > 0){
            waitSemaphores.push_back(queue->syncState[cacheIndex].semaphores[queue->syncState[cacheIndex].submits]);
            //std::cout << "Waiting for " << queue->syncState[cacheIndex].submits << " ";
        }
        else{
            //std::cout << "";
        }

        si.commandBufferCount = 1;
        uint32_t submits = queue->syncState[cacheIndex].submits;
        
        std::vector<VkPipelineStageFlags> waitFlags(waitSemaphores.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
        si.waitSemaphoreCount = waitSemaphores.size();
        si.pWaitSemaphores = waitSemaphores.data();
        si.pWaitDstStageMask = waitFlags.data();

        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores = queue->syncState[cacheIndex].semaphores.data() + queue->syncState[cacheIndex].submits + 1;
        //std::cout << "And signalling " << queue->syncState[cacheIndex].submits + 1 << std::endl;
        si.pCommandBuffers = submittable.data() + i;
        ++queue->syncState[cacheIndex].submits;
        submitResult |= vkQueueSubmit(queue->graphicsQueue, 1, &si, fence);
    }

    if(submitResult == VK_SUCCESS){
        std::unordered_set<WGVKCommandBuffer> insert;
        insert.reserve(3);
        insert.insert(cachebuffer);
        ++cachebuffer->refCount;
        
        for(size_t i = 0;i < commandCount;i++){
            insert.insert(buffers[i]);
            ++buffers[i]->refCount;
        }
        auto it = queue->pendingCommandBuffers[frameCount % framesInFlight].find(fence);
        if(it == queue->pendingCommandBuffers[frameCount % framesInFlight].end()){
            queue->pendingCommandBuffers[frameCount % framesInFlight].emplace(fence, std::move(insert));
        }
        else{
            for(auto element_from_insert : insert)
                queue->pendingCommandBuffers[frameCount % framesInFlight][fence].insert(element_from_insert);
        }
        
        //TRACELOG(LOG_INFO, "Count: %d", (int)queue->pendingCommandBuffers.size());
        /*if(vkWaitForFences(g_vulkanstate.device->device, 1, &g_vulkanstate.queue->syncState.renderFinishedFence, VK_TRUE, 100000000) != VK_SUCCESS){
            TRACELOG(LOG_FATAL, "Could not wait for fence");
        }
        vkResetFences(g_vulkanstate.device->device, 1, &g_vulkanstate.queue->syncState.renderFinishedFence);*/
    }else{
        TRACELOG(LOG_FATAL, "vkQueueSubmit failed");
    }
    wgvkReleaseCommandEncoder(queue->presubmitCache);
    wgvkReleaseCommandBuffer(cachebuffer);
    WGVKCommandEncoderDescriptor cedesc zeroinit;
    queue->presubmitCache = wgvkDeviceCreateCommandEncoder(queue->device, &cedesc);
    //queue->semaphoreThatTheNextBufferWillNeedToWaitFor = VK_NULL_HANDLE;
    //queue->presubmitCache = wgvkResetCommandBuffer(cachebuffer);
    //VkPipelineStageFlags bop[1] = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
    //si.pWaitDstStageMask = bop;
}



extern "C" void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities){
    VkSurfaceKHR surface = wgvkSurface->surface;
    VkSurfaceCapabilitiesKHR scap{};
    VkPhysicalDevice vk_physicalDevice = adapter->physicalDevice;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physicalDevice, surface, &scap);
    
    TRACELOG(LOG_INFO, "scalphaflags: %d", scap.supportedCompositeAlpha);
    // Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        wgvkSurface->formatCache = (PixelFormat*)std::calloc(formatCount, sizeof(PixelFormat));
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physicalDevice, surface, &formatCount, surfaceFormats.data());
        for(size_t i = 0;i < formatCount;i++){
            wgvkSurface->formatCache[i] = fromVulkanPixelFormat(surfaceFormats[i].format);
        }
        wgvkSurface->formatCount = formatCount;
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        wgvkSurface->presentModeCache = (PresentMode*)RL_CALLOC(presentModeCount, sizeof(PresentMode));
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physicalDevice, surface, &presentModeCount, presentModes.data());
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
    auto device = WGVKDevice(config->device);
    surface->device = config->device;
    surface->lastConfig = *config;
    vkDeviceWaitIdle(device->device);
    if(surface->imageViews){
        for (uint32_t i = 0; i < surface->imagecount; i++) {
            wgvkReleaseTextureView(surface->imageViews[i]);
            //This line is not required, since those are swapchain-owned images
            //These images also have a null memory member
            //wgvkReleaseTexture(surface->images[i]);
        }
    }

    //std::free(surface->framebuffers);
    
    std::free(surface->imageViews);
    std::free(surface->images);
    vkDestroySwapchainKHR(device->device, surface->swapchain, nullptr);
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device->adapter->physicalDevice, surface->surface);
    VkSwapchainCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface->surface;
    VkSurfaceCapabilitiesKHR vkCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(surface->device->adapter->physicalDevice, surface->surface, &vkCapabilities);
    uint32_t correctedWidth, correctedHeight;
    
    if(config->width < vkCapabilities.minImageExtent.width || config->width > vkCapabilities.maxImageExtent.width){
        correctedWidth = std::clamp(config->width, vkCapabilities.minImageExtent.width, vkCapabilities.maxImageExtent.width);
        TRACELOG(LOG_WARNING, "Invalid SurfaceConfiguration::width %u, adjusting to %u", config->width, correctedWidth);
    }
    else{
        correctedWidth = config->width;
    }
    if(config->height < vkCapabilities.minImageExtent.height || config->height > vkCapabilities.maxImageExtent.height){
        correctedHeight = std::clamp(config->height, vkCapabilities.minImageExtent.height, vkCapabilities.maxImageExtent.height);
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
    VkExtent2D newExtent{correctedWidth, correctedHeight};
    createInfo.imageExtent = newExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue family indices
    uint32_t queueFamilyIndices[2] = {device->adapter->queueIndices.graphicsIndex, device->adapter->queueIndices.transferIndex};

    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
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
    createInfo.presentMode = toVulkanPresentMode(config->presentMode); 
    createInfo.clipped = VK_TRUE;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkResult scCreateResult = vkCreateSwapchainKHR(device->device, &createInfo, nullptr, &(surface->swapchain));
    if (scCreateResult != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to create swap chain!");
    } else {
        TRACELOG(LOG_INFO, "wgvkSurfaceConfigure(): Successfully created swap chain");
    }

    vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, nullptr);

    std::vector<VkImage> tmpImages(surface->imagecount);

    //surface->imageViews = (VkImageView*)std::calloc(surface->imagecount, sizeof(VkImageView));
    TRACELOG(LOG_INFO, "Imagecount: %d", (int)surface->imagecount);
    vkGetSwapchainImagesKHR(device->device, surface->swapchain, &surface->imagecount, tmpImages.data());
    surface->images = (WGVKTexture*)std::calloc(surface->imagecount, sizeof(WGVKTexture));
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
    surface->imageViews = (WGVKTextureView*)std::calloc(surface->imagecount, sizeof(VkImageView));

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
void impl_transition(VkCommandBuffer buffer, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    EncodeTransitionImageLayout(buffer, oldLayout, newLayout, texture);
}

void wgvkRenderPassEncoderTransitionTextureLayout(WGVKRenderPassEncoder encoder, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    impl_transition(encoder->cmdBuffer, texture, oldLayout, newLayout);
    encoder->resourceUsage.registerTransition(texture, oldLayout, newLayout);
}
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout oldLayout, VkImageLayout newLayout){
    impl_transition(encoder->buffer, texture, oldLayout, newLayout);
    encoder->resourceUsage.registerTransition(texture, oldLayout, newLayout);
}
extern "C" void wgvkComputePassEncoderDispatchWorkgroups(WGVKComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z){
    vkCmdDispatch(cpe->cmdBuffer, x, y, z);
}


void wgvkReleaseCommandEncoder(WGVKCommandEncoder commandEncoder) {
    rassert(commandEncoder->movedFrom, "Commandencoder still valid");

    //for(auto rp : commandEncoder->referencedRPs){
    //    wgvkReleaseRenderPassEncoder(rp);
    //}
    //for(auto rp : commandEncoder->referencedCPs){
    //    wgvkReleaseComputePassEncoder(rp);
    //}
    //commandEncoder->resourceUsage.releaseAllAndClear();

    if(commandEncoder->buffer){
        auto& buffers = commandEncoder->device->frameCaches[commandEncoder->cacheIndex].commandBuffers;
        commandEncoder->device->frameCaches[commandEncoder->cacheIndex].commandBuffers.push_back(commandEncoder->buffer);
        //vkFreeCommandBuffers(commandEncoder->device->device, commandEncoder->device->frameCaches[commandEncoder->cacheIndex].commandPool, 1, &commandEncoder->buffer);
    }
    
    commandEncoder->~WGVKCommandEncoderImpl();
    
    std::free(commandEncoder);
}

void wgvkReleaseCommandBuffer(WGVKCommandBuffer commandBuffer) {
    --commandBuffer->refCount;
    if(commandBuffer->refCount == 0){
        //TRACELOG(LOG_INFO, "Destroying commandbuffer");
        for(auto rp : commandBuffer->referencedRPs){
            wgvkReleaseRenderPassEncoder(rp);
        }
        for(auto rp : commandBuffer->referencedCPs){
            wgvkReleaseComputePassEncoder(rp);
        }
        for(auto ct : commandBuffer->referencedRTs){
            wgvkReleaseRaytracingPassEncoder(ct);
        }
        commandBuffer->resourceUsage.releaseAllAndClear();
        PerframeCache& frameCache = commandBuffer->device->frameCaches[commandBuffer->cacheIndex];
        frameCache.commandBuffers.push_back(commandBuffer->buffer);
        
        commandBuffer->~WGVKCommandBufferImpl();
        std::free(commandBuffer);
    }
}

void wgvkReleaseRenderPassEncoder(WGVKRenderPassEncoder rpenc) {
    --rpenc->refCount;
    if (rpenc->refCount == 0) {
        rpenc->resourceUsage.releaseAllAndClear();
        if(rpenc->frameBuffer)
            vkDestroyFramebuffer(rpenc->device->device, rpenc->frameBuffer, nullptr);
        //vkDestroyRenderPass(rpenc->device->device, rpenc->renderPass, nullptr);
        rpenc->~WGVKRenderPassEncoderImpl();
        std::free(rpenc);
    }
}
void wgvkSamplerRelease(WGVKSampler sampler){
    if(!--sampler->refCount){
        vkDestroySampler(sampler->device->device, sampler->sampler, nullptr);
        RL_FREE(sampler);
    }
}
void wgvkBufferRelease(WGVKBuffer buffer) {
    --buffer->refCount;
    if (buffer->refCount == 0) {
        vmaDestroyBuffer(buffer->device->allocator, buffer->buffer, buffer->allocation);
        /*if(buffer->usage == BufferUsage_MapWrite){
            buffer->device->frameCaches[buffer->cacheIndex].stagingBufferCache[buffer->capacity].push_back(MappableBufferMemory{
                .buffer = buffer->buffer,
                .allocation = buffer->allocation,
                .propertyFlags = buffer->memoryProperties,
                .capacity = buffer->capacity
            });
        }
        else{
            vmaDestroyBuffer(buffer->device->allocator, buffer->buffer, buffer->allocation);
            //vkDestroyBuffer(buffer->device->device, buffer->buffer, nullptr);
            //vmaFreeMemory(buffer->device->allocator, buffer->allocation);
            //vkFreeMemory(dshandle->device->device, buffer->memory, nullptr);
        }*/
        buffer->~WGVKBufferImpl();
        std::free(buffer);
    }
}

void wgvkBindGroupRelease(WGVKBindGroup dshandle) {
    --dshandle->refCount;
    if (dshandle->refCount == 0) {
        dshandle->resourceUsage.releaseAllAndClear();
        wgvkBindGroupLayoutRelease(dshandle->layout);
        dshandle->device->frameCaches[dshandle->cacheIndex].bindGroupCache[dshandle->layout].emplace_back(dshandle->pool, dshandle->set);
        
        //DONT delete them, they are cached
        //vkFreeDescriptorSets(dshandle->device->device, dshandle->pool, 1, &dshandle->set);
        //vkDestroyDescriptorPool(dshandle->device->device, dshandle->pool, nullptr);
        
        dshandle->~WGVKBindGroupImpl();
        std::free(dshandle);
    }
}
RGAPICXX WGVKRenderPipeline wgvkDeviceCreateRenderPipeline(WGVKDevice device, WGVKRenderPipelineDescriptor const * descriptor) {
    WGVKDeviceImpl* deviceImpl = reinterpret_cast<WGVKDeviceImpl*>(device);
    WGVKPipelineLayoutImpl* layoutImpl = reinterpret_cast<WGVKPipelineLayoutImpl*>(descriptor->layout);

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    // Vertex Stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo zeroinit;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = reinterpret_cast<WGVKShaderModule>(descriptor->vertex.module)->vulkanModule;
    vertShaderStageInfo.pName = descriptor->vertex.entryPoint.data; // Assuming null-terminated or careful length handling elsewhere
    // TODO: Handle constants if necessary via specialization constants
    // vertShaderStageInfo.pSpecializationInfo = ...;
    shaderStages.push_back(vertShaderStageInfo);

    // Fragment Stage (Optional)
    VkPipelineShaderStageCreateInfo fragShaderStageInfo zeroinit;
    if (descriptor->fragment) {
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = reinterpret_cast<WGVKShaderModule>(descriptor->fragment->module)->vulkanModule;
        fragShaderStageInfo.pName = descriptor->fragment->entryPoint.data;
        // TODO: Handle constants
        // fragShaderStageInfo.pSpecializationInfo = ...;
        shaderStages.push_back(fragShaderStageInfo);
    }

    // Vertex Input State
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(descriptor->vertex.bufferCount);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32_t currentBinding = 0;
    for (size_t i = 0; i < descriptor->vertex.bufferCount; ++i) {
        const WGVKVertexBufferLayout* layout = &descriptor->vertex.buffers[i];
        bindingDescriptions[i].binding = currentBinding; // Assuming bindings are contiguous from 0
        bindingDescriptions[i].stride = (uint32_t)layout->arrayStride;
        bindingDescriptions[i].inputRate = toVulkanVertexStepMode(layout->stepMode);

        for (size_t j = 0; j < layout->attributeCount; ++j) {
            const VertexAttribute* attrib = &layout->attributes[j];
            VkVertexInputAttributeDescription vkAttrib{};
            vkAttrib.binding = currentBinding;
            vkAttrib.location = attrib->shaderLocation;
            vkAttrib.format = toVulkanVertexFormat(attrib->format);
            vkAttrib.offset = (uint32_t)attrib->offset;
            attributeDescriptions.push_back(vkAttrib);
        }
        currentBinding++;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo zeroinit;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    VkPipelineColorBlendStateCreateInfo colorBlending zeroinit;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    if (descriptor->fragment) {
        colorBlendAttachments.resize(descriptor->fragment->targetCount);
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
        colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
        colorBlending.pAttachments = colorBlendAttachments.data();
    } else {
        // No fragment shader means no color attachments needed? Check spec.
        // Typically, rasterizerDiscardEnable would be true if no fragment shader, but let's allow it.
        colorBlending.attachmentCount = 0;
        colorBlending.pAttachments = nullptr;
    }


    // Dynamic State
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
        // Add VK_DYNAMIC_STATE_STENCIL_REFERENCE if stencil test is enabled and reference is not fixed
    };
    if (depthStencil.stencilTestEnable) {
       dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
    }
     if (rasterizer.depthBiasEnable) {
         dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
     }


    VkPipelineDynamicStateCreateInfo dynamicState zeroinit;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Pipeline Rendering Info (for dynamic rendering if used, otherwise NULL)
    // Assuming traditional render passes for now based on WGVK structure.
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{}; // Zero initialize
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
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = (descriptor->depthStencil) ? &depthStencil : nullptr;
    pipelineInfo.pColorBlendState = (descriptor->fragment) ? &colorBlending : nullptr; // Only if frag shader exists
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = layoutImpl->layout;

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
        return nullptr;
    }
    pipelineImpl->device = device;
    //wgvkDeviceAddRef(device);
    pipelineImpl->layout = layoutImpl; // Store for potential use
    wgvkPipelineLayoutAddRef(layoutImpl);
    VkResult result = vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelineImpl->renderPipeline);

    if (result != VK_SUCCESS) {
        // Handle pipeline creation failure
        RL_FREE(pipelineImpl);
        // Optionally log error based on result code
        return nullptr;
    }

    return reinterpret_cast<WGVKRenderPipeline>(pipelineImpl);
}
extern "C" void wgvkBindGroupLayoutRelease(WGVKBindGroupLayout bglayout){
    --bglayout->refCount;
    if(bglayout->refCount == 0){
        vkDestroyDescriptorSetLayout(bglayout->device->device, bglayout->layout, nullptr);
        bglayout->~WGVKBindGroupLayoutImpl();
        std::free(const_cast<ResourceTypeDescriptor*>(bglayout->entries));
        std::free(bglayout);
    }
}

extern "C" void wgvkReleaseTexture(WGVKTexture texture){
    --texture->refCount;
    if(texture->refCount == 0){
        vkDestroyImage(texture->device->device, texture->image, nullptr);
        vkFreeMemory(texture->device->device, texture->memory, nullptr);

        texture->~WGVKTextureImpl();
        std::free(texture);
    }
}
extern "C" void wgvkReleaseTextureView(WGVKTextureView view){
    --view->refCount;
    if(view->refCount == 0){
        vkDestroyImageView(view->texture->device->device, view->view, nullptr);
        wgvkReleaseTexture(view->texture);
        view->~WGVKTextureViewImpl();
        std::free(view);
    }
}

extern "C" void wgvkCommandEncoderCopyBufferToBuffer  (WGVKCommandEncoder commandEncoder, WGVKBuffer source, uint64_t sourceOffset, WGVKBuffer destination, uint64_t destinationOffset, uint64_t size){
    commandEncoder->resourceUsage.track(source);
    commandEncoder->resourceUsage.track(destination);

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
extern "C" void wgvkCommandEncoderCopyBufferToTexture (WGVKCommandEncoder commandEncoder, WGVKTexelCopyBufferInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize){
    
    VkImageCopy copy;
    VkBufferImageCopy region{};

    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = is__depth(destination->texture->format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = VkOffset3D{
        (int32_t)destination->origin.x,
        (int32_t)destination->origin.y,
        (int32_t)destination->origin.z,
    };

    region.imageExtent = {
        copySize->width,
        copySize->height,
        copySize->depthOrArrayLayers
    };
    commandEncoder->resourceUsage.track(source->buffer);
    WGVKCommandEncoderImpl::iotresult result = commandEncoder->initializeOrTransition(destination->texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //commandEncoder->resourceUsage.track(destination->texture);
    
    //if(destination->texture->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
    //    wgvkCommandEncoderTransitionTextureLayout(commandEncoder, destination->texture, destination->texture->layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    //}
    vkCmdCopyBufferToImage(commandEncoder->buffer, source->buffer->buffer, destination->texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
extern "C" void wgvkCommandEncoderCopyTextureToBuffer (WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyBufferInfo const * destination, WGVKExtent3D const * copySize){
    TRACELOG(LOG_FATAL, "Not implemented");
    rg_unreachable();
}
extern "C" void wgvkCommandEncoderCopyTextureToTexture(WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize){
    TRACELOG(LOG_FATAL, "Not implemented");
    rg_unreachable();
}



// Implementation of RenderpassEncoderDraw
void wgvkRenderpassEncoderDraw(WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Record the draw command into the command buffer
    vkCmdDraw(rpe->cmdBuffer, vertices, instances, firstvertex, firstinstance);
}

// Implementation of RenderpassEncoderDrawIndexed
extern "C" void wgvkRenderpassEncoderDrawIndexed(WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t baseVertex, uint32_t firstinstance) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");

    // Record the indexed draw command into the command buffer
    vkCmdDrawIndexed(rpe->cmdBuffer, indices, instances, firstindex, (int32_t)(baseVertex & 0x7fffffff), firstinstance);
}

extern "C" void wgvkRenderPassEncoderSetPipeline(WGVKRenderPassEncoder rpe, WGVKRenderPipeline renderPipeline) { 
    vkCmdBindPipeline(rpe->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderPipeline->renderPipeline);
    rpe->lastLayout = renderPipeline->layout->layout; 
}

// Implementation of RenderPassDescriptorBindDescriptorSet
void wgvkRenderPassEncoderSetBindGroup(WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset) {
    assert(rpe != nullptr && "RenderPassEncoderHandle is null");
    assert(dset != nullptr && "DescriptorSetHandle is null");
    assert(rpe->lastLayout != VK_NULL_HANDLE && "Pipeline layout is not set");
    
    // Bind the descriptor set to the command buffer
    vkCmdBindDescriptorSets(rpe->cmdBuffer,                  // Command buffer
                            VK_PIPELINE_BIND_POINT_GRAPHICS, // Pipeline bind point
                            rpe->lastLayout,                 // Pipeline layout
                            group,                           // First set
                            1,                               // Descriptor set count
                            &(dset->set),                    // Pointer to descriptor set
                            0,                               // Dynamic offset count
                            nullptr                          // Pointer to dynamic offsets
    );


    rpe->resourceUsage.track(dset);
    for(auto viewAndUsage : dset->resourceUsage.referencedTextureViews){
        if(viewAndUsage.second == TextureUsage_TextureBinding)
            rpe->cmdEncoder->initializeOrTransition(viewAndUsage.first->texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        else if(viewAndUsage.second == TextureUsage_StorageBinding){
            rpe->cmdEncoder->initializeOrTransition(viewAndUsage.first->texture, VK_IMAGE_LAYOUT_GENERAL);
        }
    }
}
extern "C" void wgvkComputePassEncoderSetPipeline (WGVKComputePassEncoder cpe, WGVKComputePipeline computePipeline){
    vkCmdBindPipeline(cpe->cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline->computePipeline);
    cpe->lastLayout = computePipeline->layout->layout;
}
extern "C" void wgvkComputePassEncoderSetBindGroup(WGVKComputePassEncoder cpe, uint32_t groupIndex, WGVKBindGroup bindGroup){
    rassert(cpe->lastLayout != nullptr, "Must bind at least one pipeline with wgvkComputePassEncoderSetPipeline before wgvkComputePassEncoderSetBindGroup");
    
    vkCmdBindDescriptorSets(cpe->cmdBuffer,                // Command buffer
                            VK_PIPELINE_BIND_POINT_COMPUTE,// Pipeline bind point
                            cpe->lastLayout,               // Pipeline layout
                            groupIndex,                    // First set
                            1,                             // Descriptor set count
                            &(bindGroup->set),             // Pointer to descriptor set
                            0,                             // Dynamic offset count
                            nullptr                        // Pointer to dynamic offsets
    );
    cpe->resourceUsage.track(bindGroup);
    for(auto viewAndUsage : bindGroup->resourceUsage.referencedTextureViews){
        if(viewAndUsage.second == TextureUsage_TextureBinding)
            cpe->cmdEncoder->initializeOrTransition(viewAndUsage.first->texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        else if(viewAndUsage.second == TextureUsage_StorageBinding){
            cpe->cmdEncoder->initializeOrTransition(viewAndUsage.first->texture, VK_IMAGE_LAYOUT_GENERAL);
        }
    }
}
extern "C" WGVKComputePassEncoder wgvkCommandEncoderBeginComputePass(WGVKCommandEncoder commandEncoder){
    WGVKComputePassEncoder ret = callocnewpp(WGVKComputePassEncoderImpl);
    ret->refCount = 2;
    commandEncoder->referencedCPs.insert(ret);
    ret->cmdEncoder = commandEncoder;
    ret->cmdBuffer = commandEncoder->buffer;
    ret->device = commandEncoder->device;
    return ret;
}
extern "C" void wgvkCommandEncoderEndComputePass(WGVKComputePassEncoder commandEncoder){
    
}
extern "C" void wgvkReleaseComputePassEncoder(WGVKComputePassEncoder cpenc){
    --cpenc->refCount;
    if(cpenc->refCount == 0){
        cpenc->resourceUsage.releaseAllAndClear();
        cpenc->~WGVKComputePassEncoderImpl();
        std::free(cpenc);
    }
}

extern "C" void wgvkReleaseRaytracingPassEncoder(WGVKRaytracingPassEncoder rtenc){
    --rtenc->refCount;
    if(rtenc->refCount == 0){
        rtenc->resourceUsage.releaseAllAndClear();
        rtenc->~WGVKRaytracingPassEncoderImpl();
        std::free(rtenc);
    }
}
extern "C" void wgvkSurfacePresent(WGVKSurface surface){
    uint32_t cacheIndex = surface->device->submittedFrames % framesInFlight;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &surface->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    VkSwapchainKHR swapChains[] = {surface->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &surface->activeImageIndex;
    VkImageSubresourceRange isr{};
    isr.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    isr.layerCount = 1;
    isr.levelCount = 1;

    
    VkCommandBuffer transitionBuffer = surface->device->frameCaches[cacheIndex].finalTransitionBuffer;
    
    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType =  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags =  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(transitionBuffer, &beginInfo);
    EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, surface->images[surface->activeImageIndex]);
    //EncodeTransitionImageLayout(transitionBuffer, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (WGVKTexture)surface->renderTarget.depth.id);
    vkEndCommandBuffer(transitionBuffer);
    VkSubmitInfo cbsinfo zeroinit;
    cbsinfo.commandBufferCount = 1;
    cbsinfo.pCommandBuffers = &transitionBuffer;
    cbsinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    cbsinfo.signalSemaphoreCount = 1;
    cbsinfo.waitSemaphoreCount = 1;
    VkPipelineStageFlags wsmask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    cbsinfo.pWaitDstStageMask = &wsmask;

    cbsinfo.pWaitSemaphores = &surface->device->queue->syncState[cacheIndex].semaphores[surface->device->queue->syncState[cacheIndex].submits];
    //TRACELOG(LOG_INFO, "Submit waiting for semaphore index: %u", g_vulkanstate.queue->syncState[cacheIndex].submits);
    
    cbsinfo.pSignalSemaphores = &surface->device->frameCaches[cacheIndex].finalTransitionSemaphore;
    
    VkFence finalTransitionFence = surface->device->frameCaches[cacheIndex].finalTransitionFence;
    vkQueueSubmit(surface->device->queue->graphicsQueue, 1, &cbsinfo, finalTransitionFence);
    
    
    auto it = surface->device->queue->pendingCommandBuffers[cacheIndex].find(finalTransitionFence);
    if(it == surface->device->queue->pendingCommandBuffers[cacheIndex].end()){
        surface->device->queue->pendingCommandBuffers[cacheIndex].emplace(finalTransitionFence, std::unordered_set<WGVKCommandBuffer>{});
    }
    else{
       //it->second.insert(WGVKCommandBuffer{});
    }

    VkResult presentRes = vkQueuePresentKHR(surface->device->queue->presentQueue, &presentInfo);

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

extern "C" void EncodeTransitionImageLayout(
    VkCommandBuffer commandBuffer,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    WGVKTexture texture
) {
    VkImage image = texture->image;
    // --- 1. Define the Image Memory Barrier ---
    // This structure describes the memory dependency for a specific image subresource range.
    // It tells Vulkan how access to an image needs to be synchronized between operations.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout; // Layout the image is currently in
    barrier.newLayout = newLayout; // Layout the image needs to be transitioned to

    // --- Queue Family Ownership ---
    // If the image ownership is being transferred between different queue families (e.g., graphics to compute),
    // you would specify the src and dst queue family indices here.
    // VK_QUEUE_FAMILY_IGNORED means we are not transferring ownership, or the transition happens
    // within the same queue family.
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    // --- Image and Subresource Range ---
    barrier.image = image; // The specific image resource to transition

    // Define which part(s) of the image are affected by this barrier.
    // aspectMask: Specifies which aspects (color, depth, stencil) are affected.
    // TODO: A more robust implementation would check the actual VkFormat of the image.
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL ||
        oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || // Check old layout too
        oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL )
    {
         // Check format here if available to see if stencil exists
         // bool hasStencil = (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT);
         // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; // Simple default for now
         // If format has stencil, add: | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;   // First mip level to transition
    barrier.subresourceRange.levelCount = 1;     // Number of mip levels to transition
    barrier.subresourceRange.baseArrayLayer = 0; // First array layer to transition
    barrier.subresourceRange.layerCount = 1;     // Number of array layers to transition

    // --- 2. Define Pipeline Stages and Access Masks ---
    // This is the crucial part for performance and correctness. We need to specify:
    // - sourceStageMask: Which pipeline stage(s) must complete *before* the barrier executes.
    //                    This relates to operations using the `oldLayout`.
    // - destinationStageMask: Which pipeline stage(s) must wait *for* the barrier to complete
    //                         before they can start. This relates to operations using the `newLayout`.
    // - srcAccessMask: What kind of memory access (read/write) was performed in the source stages
    //                  that needs to be made available/visible *before* the transition.
    // - dstAccessMask: What kind of memory access (read/write) will be performed in the destination stages
    //                  that needs to wait *for* the transition to complete.

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // --- Determine Source Stage & Access Mask based on oldLayout ---
    switch (oldLayout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            // No need to wait for any stage, as the image content is undefined or discarded.
            // No prior access needs to be synchronized.
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Represents the very start, no actual stage dependency.
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
             // Image data was prepared by the host.
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_HOST_BIT; // Wait for host writes to be visible.
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image was last used for reading in a transfer operation.
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image was last used for writing in a transfer operation (e.g., vkCmdCopyBufferToImage).
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Wait for transfer writes to complete.
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image was last used as a color attachment (written to by fragment shader output).
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Wait for color writes to complete.
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image was last used as a depth/stencil attachment (read/written by depth/stencil tests).
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // Wait for depth/stencil ops.
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image was last read by a shader.
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            // Which shader stage? Could be vertex, fragment, compute, etc.
            // To be safe, we might sync against all relevant shader stages,
            // or ideally, track the actual usage. Let's use a common one.
            // VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT is common, but compute might also read.
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Or VERTEX, COMPUTE, etc.
             // A more robust barrier could use VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT or add VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
             // Image was just presented. Generally, you transition *to* this layout at the end
             // of a frame and the swapchain handles the transition *from* it implicitly when
             // acquiring the next image. If you manually transition *from* it (e.g. for reuse),
             // usually no specific GPU access needs synchronizing from the presentation itself.
             barrier.srcAccessMask = 0; // Or maybe MEMORY_READ if WSI requires it? Safest is 0.
             sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Presentation is implicitly after all commands.
             break;

        case VK_IMAGE_LAYOUT_GENERAL:
             // General layout could have been used for *anything*. This often implies less optimal access.
             // We need a conservative barrier. Assume it could have been written or read from anywhere.
             // This is often a sign that layout tracking could be improved.
             barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Any potential access
             sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // Wait for any possible command
             break;

        default:
            // If we encounter an unhandled layout, assert or log an error.
            // Using a very broad barrier as a fallback is dangerous and inefficient.
            rassert(false, "Unsupported oldLayout for transition!");
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Fallback guess
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Fallback guess
            break;
    }

    // --- Determine Destination Stage & Access Mask based on newLayout ---
    switch (newLayout) {
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            // Image will be used for reading in a transfer operation.
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            // Image will be used for writing in a transfer operation.
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // Transfer stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            // Image will be used as a color attachment (written by fragment shader).
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // Might be read for blending too
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Color output stage needs to wait.
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            // Image will be used as a depth/stencil attachment (read/written by tests).
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; // Depth/stencil stages need to wait.
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            // Image will be read by a shader.
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            // Which shader stage will read it? Fragment is common for textures.
            // Compute is common for imageLoad/Store or sampled images.
            // Vertex shaders can also read textures (vertex texture fetch).
            // Let's assume fragment for now, but be aware it could be others.
            destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT; 
            break;

        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            // Image will be presented to the screen.
            // The presentation engine reads the image implicitly after all commands complete.
            barrier.dstAccessMask = 0; // No specific GPU access needs to be made available *for* presentation itself via this barrier. Some WSI layers might implicitly need MEMORY_READ. 0 is usually fine.
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Ensure all commands finish before presentation can potentially start.
            break;

        case VK_IMAGE_LAYOUT_GENERAL:
             // General layout can be used for *anything* next. This might indicate suboptimal planning.
             // Subsequent operations need to wait. Assume any kind of access might happen.
             barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT; // Make memory available for any access
             destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // Any subsequent command might use it.
             break;

        default:
            // If we encounter an unhandled layout, assert or log an error.
            rassert(false, "Unsupported newLayout for transition!");
            barrier.dstAccessMask = 0; // Fallback guess
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // Fallback guess
            break;
    }

    // --- 3. Record the Pipeline Barrier Command ---
    // This command injects the dependency into the command buffer.
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage,        // Pipeline stage(s) before the barrier
        destinationStage,   // Pipeline stage(s) after the barrier
        0,                  // Dependency flags (usually 0, VK_DEPENDENCY_BY_REGION_BIT is an optimization)
        0, nullptr,         // Global memory barriers (not needed here)
        0, nullptr,         // Buffer memory barriers (not needed here)
        1, &barrier         // Image memory barriers (we have one)
    );
}
WGVKSampler wgvkDeviceCreateSampler(WGVKDevice device, const WGVKSamplerDescriptor* descriptor){
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

    WGVKSampler ret = callocnewpp(WGVKSamplerImpl);
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
    VkResult result = vkCreateSampler(device->device, &sci, nullptr, &(ret->sampler));
    return ret;
}
void wgvkRenderPassEncoderBindVertexBuffer(WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset) {
    rassert(rpe != nullptr, "RenderPassEncoderHandle is null");
    rassert(buffer != nullptr, "BufferHandle is null");
    
    // Bind the vertex buffer to the command buffer at the specified binding point
    vkCmdBindVertexBuffers(rpe->cmdBuffer, binding, 1, &(buffer->buffer), &offset);

    rpe->resourceUsage.track(buffer);
}
extern "C" void wgvkRenderPassEncoderBindIndexBuffer(WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, IndexFormat indexType){
    rassert(rpe != nullptr, "RenderPassEncoderHandle is null");
    rassert(buffer != nullptr, "BufferHandle is null");

    // Bind the index buffer to the command buffer
    vkCmdBindIndexBuffer(rpe->cmdBuffer, buffer->buffer, offset, toVulkanIndexFormat(indexType));

    rpe->resourceUsage.track(buffer);
}
extern "C" void wgvkTextureViewAddRef(WGVKTextureView textureView){
    ++textureView->refCount;
}
extern "C" void wgvkTextureAddRef(WGVKTexture texture){
    ++texture->refCount;
}
extern "C" void wgvkBufferAddRef(WGVKBuffer buffer){
    ++buffer->refCount;
}
extern "C" void wgvkBindGroupAddRef(WGVKBindGroup bindGroup){
    ++bindGroup->refCount;
}
extern "C" void wgvkBindGroupLayoutAddRef(WGVKBindGroupLayout bindGroupLayout){
    ++bindGroupLayout->refCount;
}
extern "C" void wgvkPipelineLayoutAddRef(WGVKPipelineLayout pipelineLayout){
    ++pipelineLayout->refCount;
}
extern "C" void wgvkSamplerAddRef(WGVKSampler sampler){
    ++sampler->refCount;
}
SafelyResettableCommandPool::SafelyResettableCommandPool(WGVKDevice p_device) : 
    pool(VK_NULL_HANDLE),
    finishingFence(VK_NULL_HANDLE),
    device(p_device)
{
    VkCommandPoolCreateInfo pci zeroinit;
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = device->adapter->queueIndices.graphicsIndex;
    VkResult pcr = vkCreateCommandPool(device->device, &pci, nullptr, &pool);
    if(pcr != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Could not create command pool: %s", vkErrorString(pcr));
    }
    VkFenceCreateInfo vci zeroinit;
    vci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(device->device, &vci, nullptr, &finishingFence);
}

void SafelyResettableCommandPool::finish(){
    VkSubmitInfo sinfo;
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkQueueSubmit(device->queue->graphicsQueue, 1, &sinfo, finishingFence);
}

void SafelyResettableCommandPool::reset(){
    vkWaitForFences(device->device, 1, &finishingFence, VK_TRUE, uint64_t(1) << 32); // Wait about 4.2 seconds
    vkResetCommandPool(device->device, pool, 0);
    std::copy(currentlyInUse.begin(), currentlyInUse.end(), std::back_inserter(availableForUse));
    currentlyInUse.clear();
}


bool ResourceUsage::contains(WGVKBuffer buffer)const noexcept{
    return referencedBuffers.find(buffer) != referencedBuffers.end();
}
bool ResourceUsage::contains(WGVKTexture texture)const noexcept{
    return referencedTextures.find(texture) != referencedTextures.end();
}
bool ResourceUsage::contains(WGVKTextureView view)const noexcept{
    return referencedTextureViews.find(view) != referencedTextureViews.end();
}
bool ResourceUsage::contains(WGVKBindGroup bindGroup)const noexcept{
    return referencedBindGroups.find(bindGroup) != referencedBindGroups.end();
}
bool ResourceUsage::contains(WGVKBindGroupLayout bindGroupLayout)const noexcept{
    return referencedBindGroupLayouts.find(bindGroupLayout) != referencedBindGroupLayouts.end();
}
bool ResourceUsage::contains(WGVKSampler sampler)const noexcept{
    return referencedSamplers.find(sampler) != referencedSamplers.end();
}

void ResourceUsage::track(WGVKBuffer buffer)noexcept{
    if(!contains(buffer)){
        ++buffer->refCount;
        referencedBuffers.insert(buffer);
    }
}
void ResourceUsage::track(WGVKTexture texture)noexcept{
    if(!contains(texture)){
        ++texture->refCount;
        referencedTextures.insert(texture);
    }
}
void ResourceUsage::track(WGVKTextureView view, TextureUsage usage)noexcept{
    if(!contains(view)){
        ++view->refCount;
        referencedTextureViews.emplace(view, usage);
    }
}
void ResourceUsage::track(WGVKBindGroup bindGroup)noexcept{
    rassert(bindGroup->layout != nullptr, "Layout is nullptr");
    if(!contains(bindGroup)){
        ++bindGroup->refCount;
        referencedBindGroups.insert(bindGroup);
    }
}
void ResourceUsage::track(WGVKBindGroupLayout bindGroupLayout)noexcept{
    rassert(bindGroupLayout->layout != nullptr, "Layout is nullptr");
    if(!contains(bindGroupLayout)){
        ++bindGroupLayout->refCount;
        referencedBindGroupLayouts.insert(bindGroupLayout);
    }
}
void ResourceUsage::track(WGVKSampler sampler)noexcept{
    if(!contains(sampler)){
        ++sampler->refCount;
        referencedSamplers.insert(sampler);
    }
}
void ResourceUsage::releaseAllAndClear()noexcept{
    for(auto buffer : referencedBuffers){
        wgvkBufferRelease(buffer);
    }
    for(auto bindGroup : referencedBindGroups){
        wgvkBindGroupRelease(bindGroup);
    }
    for(auto bindGroupLayout : referencedBindGroupLayouts){
        wgvkBindGroupLayoutRelease(bindGroupLayout);
    }
    for(auto texture : referencedTextures){
        wgvkReleaseTexture(texture);
    }
    for(const auto [view, _] : referencedTextureViews){
        wgvkReleaseTextureView(view);
    }
    for(const auto smp : referencedSamplers){
        wgvkSamplerRelease(smp);
    }
    referencedBuffers.clear();
    referencedTextures.clear();
    referencedTextureViews.clear();
    referencedBindGroups.clear();
    referencedSamplers.clear();
}


extern "C" const char* vkErrorString(int code){
    
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