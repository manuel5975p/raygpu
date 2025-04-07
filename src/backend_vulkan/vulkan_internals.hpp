#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#define VMA_MIN_ALIGNMENT 32
#include <external/VmaUsage.h>
#include <external/small_vector.hpp>
#include <unordered_set>
#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <map>
#include <raygpu.h>
#include <internals.hpp>
#include <wgvk.h>
//#define SUPPORT_VULKAN_BACKEND 1
#include <enum_translation.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_core.h>
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


using refcount_type = uint32_t;
template<typename T>
using ref_holder = std::unordered_set<T>;
template<typename T, typename R>
using typed_ref_holder = std::unordered_map<T, R>;

typedef struct ResourceUsage{
    ref_holder<WGVKBuffer> referencedBuffers;
    ref_holder<WGVKTexture> referencedTextures;
    typed_ref_holder<WGVKTextureView, TextureUsage> referencedTextureViews;
    ref_holder<WGVKBindGroup> referencedBindGroups;

    std::unordered_map<WGVKTexture, std::pair<VkImageLayout, VkImageLayout>> entryAndFinalLayouts;

    bool contains(WGVKBuffer buffer)const noexcept;
    bool contains(WGVKTexture texture)const noexcept;
    bool contains(WGVKTextureView view)const noexcept;
    bool contains(WGVKBindGroup bindGroup)const noexcept;

    void track(WGVKBuffer buffer)noexcept;
    void track(WGVKTexture texture)noexcept;
    void track(WGVKTextureView view, TextureUsage usage)noexcept;
    void track(WGVKBindGroup bindGroup)noexcept;
    
    void registerTransition(WGVKTexture tex, VkImageLayout from, VkImageLayout to){
        auto it = entryAndFinalLayouts.find(tex);
        if(it == entryAndFinalLayouts.end()){
            entryAndFinalLayouts.emplace(tex, std::make_pair(from, to));
        }
        else{
            rassert(
                from == it->second.second, 
                "The previous layout transition encoded into this ResourceUsage did not transition to the layout this transition assumes"
            );
            it->second.second = to;
        }
    }
    void releaseAllAndClear()noexcept;
    ~ResourceUsage(){
        //if(referencedBuffers.size()){
        //    abort();
        //}
        //if(referencedTextures.size()){
        //    abort();
        //}
        //if(referencedTextureViews.size()){
        //    abort();
        //}
        //if(referencedBindGroups.size()){
        //    abort();
        //}
    }
}ResourceUsage;

constexpr uint32_t max_color_attachments = 4;
typedef struct AttachmentDescriptor{
    VkFormat format;
    uint32_t sampleCount;
    LoadOp loadop;
    StoreOp storeop;
    bool operator==(const AttachmentDescriptor& other)const noexcept{
        return format == other.format
            && sampleCount == other.sampleCount
            && loadop == other.loadop
            && storeop == other.storeop;
    }
    bool operator!=(const AttachmentDescriptor& other) const noexcept{
        return !(*this == other);
    }
}AttachmentDescriptor;
typedef struct RenderPassLayout{
    uint32_t colorAttachmentCount;
    AttachmentDescriptor colorAttachments[max_color_attachments];
    AttachmentDescriptor colorResolveAttachments[max_color_attachments];
    uint32_t depthAttachmentPresent;
    AttachmentDescriptor depthAttachment;
    //uint32_t colorResolveIndex;
    bool operator==(const RenderPassLayout& other) const noexcept {
        if(colorAttachmentCount != other.colorAttachmentCount)return false;
        for(uint32_t i = 0;i < colorAttachmentCount;i++){
            if(colorAttachments[i] != other.colorAttachments[i]){
                return false;
            }
        }
        if(depthAttachmentPresent != other.depthAttachmentPresent)return false;
        if(depthAttachment != other.depthAttachment)return false;
        //if(colorResolveIndex != other.colorResolveIndex)return false;

        return true;
    }
}RenderPassLayout;
namespace std{
    template<>
    struct hash<RenderPassLayout>{
        constexpr size_t operator()(const RenderPassLayout& layout)const noexcept{

            xorshiftstate ret{0x2545F4918F6CDD1D};
            ret.update(layout.depthAttachmentPresent << 6, layout.colorAttachmentCount);
            for(uint32_t i = 0;i < layout.colorAttachmentCount;i++){
                ret.update(layout.colorAttachments[i].format, layout.colorAttachments[i].sampleCount);
                ret.update(layout.colorAttachments[i].loadop, layout.colorAttachments[i].storeop);
            }
            ret.update(layout.depthAttachmentPresent << 6, layout.depthAttachmentPresent);
            if(layout.depthAttachmentPresent){
                ret.update(layout.depthAttachment.format, layout.depthAttachment.sampleCount);
                ret.update(layout.depthAttachment.loadop, layout.depthAttachment.storeop);
            }
            return ret.x64;
        }
    };
    
}


typedef struct WGVKBindGroupImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    WGVKBindGroupLayout layout;
    refcount_type refCount;
    ResourceUsage resourceUsage;
    WGVKDevice device;
    uint32_t cacheIndex;
}WGVKBindGroupImpl;

typedef struct WGVKBindGroupLayoutImpl{
    VkDescriptorSetLayout layout;
    WGVKDevice device;
    const ResourceTypeDescriptor* entries;
    uint32_t entryCount;

    refcount_type refCount;
}WGVKBindGroupLayoutImpl;

typedef struct WGVKBufferImpl{
    VkBuffer buffer;
    WGVKDevice device;
    uint32_t cacheIndex;
    BufferUsage usage;
    size_t capacity;
    VmaAllocation allocation;
    VkMemoryPropertyFlags memoryProperties;
    VkDeviceAddress address; //uint64_t, if applicable (BufferUsage_ShaderDeviceAddress)
    refcount_type refCount;
}WGVKBufferImpl;
typedef struct WGVKBottomLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
} WGVKBottomLevelAccelerationStructureImpl;
typedef WGVKBottomLevelAccelerationStructureImpl *WGVKBottomLevelAccelerationStructure;

typedef struct WGVKTopLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
    VkBuffer instancesBuffer;
    VkDeviceMemory instancesBufferMemory;
} WGVKTopLevelAccelerationStructureImpl;

constexpr uint32_t framesInFlight = 2;
namespace std{
    template<>
    struct hash<WGVKBindGroupLayout>{
        constexpr size_t operator()(const WGVKBindGroupLayout bglayout)const noexcept{
            xorshiftstate ret{0x2545F4918F6CDD1D};
            for(uint32_t i = 0;i < bglayout->entryCount;i++){
                ret.update((uint32_t)bglayout->entries[i].fstype, (uint32_t)bglayout->entries[i].type);
                ret.update((uint32_t)bglayout->entries[i].access, (uint32_t)bglayout->entries[i].location);
            }
            return ret.x64;
        }
    };
}
typedef struct MappableBufferMemory{
    VkBuffer buffer;
    VmaAllocation allocation;
    VkMemoryPropertyFlags propertyFlags;
    size_t capacity;
}MappableBufferMemory;

struct PerframeCache{
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<WGVKBuffer> unusedBatchBuffers;
    std::vector<WGVKBuffer> usedBatchBuffers;
    
    VkCommandBuffer finalTransitionBuffer;
    VkSemaphore finalTransitionSemaphore;
    VkFence finalTransitionFence;
    std::map<uint64_t, small_vector<MappableBufferMemory>> stagingBufferCache;
    std::unordered_map<WGVKBindGroupLayout, std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>>> bindGroupCache;
};

typedef struct WGVKAdapterImpl{
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
    VkPhysicalDeviceMemoryProperties memProperties;
    QueueIndices queueIndices;
}WGVKAdapterImpl;

typedef struct LayoutedRenderPass{
    RenderPassLayout layout;
    std::vector<VkAttachmentDescription> allAttachments;
    VkRenderPass renderPass;
}LayoutedRenderPass;

typedef struct WGVKDeviceImpl{
    VkDevice device;
    WGVKAdapter adapter;
    WGVKQueue queue;
    size_t submittedFrames = 0;
    VmaAllocator allocator;
    VmaPool aligned_hostVisiblePool;
    PerframeCache frameCaches[framesInFlight];

    std::unordered_map<RenderPassLayout, LayoutedRenderPass> renderPassCache;
}WGVKDeviceImpl;

typedef struct WGVKTextureImpl{
    VkImage image;
    VkFormat format;
    VkImageLayout layout;
    VkDeviceMemory memory;
    WGVKDevice device;
    refcount_type refCount;
    uint32_t width, height, depthOrArrayLayers;
    uint32_t mipLevels;
    uint32_t sampleCount;
}ImageHandleImpl;

typedef struct WGVKRenderPipelineImpl{
    VkPipeline renderPipeline;
    VkPipelineLayout layout;
    std::vector<VkDynamicState> dynamicStates;
}WGVKRenderPipelineImpl;


typedef struct WGVKComputePipelineImpl{
    VkPipeline graphicsPipeline;
    VkPipelineLayout layout;
}WGVKComputePipelineImpl;

typedef struct WGVKRaytracingPipelineImpl{
    VkPipeline raytracingPipeline;
    VkPipelineLayout layout;
    WGVKBuffer raygenBindingTable;
    WGVKBuffer missBindingTable;
    WGVKBuffer hitBindingTable;
}WGVKRaytracingPipelineImpl;

typedef struct WGVKTextureViewImpl{
    VkImageView view;
    VkFormat format;
    refcount_type refCount;
    WGVKTexture texture;
    uint32_t width, height, depthOrArrayLayers;
    uint32_t sampleCount;
}WGVKTextureViewImpl;









extern "C" const char* vkErrorString(VkResult code);
inline bool ResourceUsage::contains(WGVKBuffer buffer)const noexcept{
    return referencedBuffers.find(buffer) != referencedBuffers.end();
}
inline bool ResourceUsage::contains(WGVKTexture texture)const noexcept{
    return referencedTextures.find(texture) != referencedTextures.end();
}
inline bool ResourceUsage::contains(WGVKTextureView view)const noexcept{
    return referencedTextureViews.find(view) != referencedTextureViews.end();
}
inline bool ResourceUsage::contains(WGVKBindGroup bindGroup)const noexcept{
    return referencedBindGroups.find(bindGroup) != referencedBindGroups.end();
}

inline void ResourceUsage::track(WGVKBuffer buffer)noexcept{
    if(!contains(buffer)){
        ++buffer->refCount;
        referencedBuffers.insert(buffer);
    }
}
inline void ResourceUsage::track(WGVKTexture texture)noexcept{
    if(!contains(texture)){
        ++texture->refCount;
        referencedTextures.insert(texture);
    }
}
inline void ResourceUsage::track(WGVKTextureView view, TextureUsage usage)noexcept{
    if(!contains(view)){
        ++view->refCount;
        referencedTextureViews.emplace(view, usage);
    }
}
inline void ResourceUsage::track(WGVKBindGroup bindGroup)noexcept{
    rassert(bindGroup->layout != nullptr, "Layout is nullptr");
    if(!contains(bindGroup)){
        ++bindGroup->refCount;
        referencedBindGroups.insert(bindGroup);
    }
}
inline void ResourceUsage::releaseAllAndClear()noexcept{
    for(auto buffer : referencedBuffers){
        wgvkBufferRelease(buffer);
    }
    for(auto bindGroup : referencedBindGroups){
        wgvkBindGroupRelease(bindGroup);
    }
    for(auto texture : referencedTextures){
        wgvkReleaseTexture(texture);
    }
    for(const auto [view, _] : referencedTextureViews){
        wgvkReleaseTextureView(view);
    }
    referencedBuffers.clear();
    referencedTextures.clear();
    referencedTextureViews.clear();
    referencedBindGroups.clear();
}
typedef struct WGVKRenderPassEncoderImpl{
    VkCommandBuffer cmdBuffer;
    VkRenderPass renderPass;
    
    WGVKDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    VkPipelineLayout lastLayout;
    VkFramebuffer frameBuffer;
    WGVKCommandEncoder cmdEncoder;
}RenderPassEncoderHandleImpl;

typedef struct WGVKComputePassEncoderImpl{
    VkCommandBuffer cmdBuffer;
    
    WGVKDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    VkPipelineLayout lastLayout;
    WGVKCommandEncoder cmdEncoder;
}WGVKComputePassEncoderImpl;



typedef struct WGVKCommandEncoderImpl{
    VkCommandBuffer buffer;
    
    ref_holder<WGVKRenderPassEncoder    > referencedRPs;
    ref_holder<WGVKComputePassEncoder   > referencedCPs;
    ref_holder<WGVKRaytracingPassEncoder> referencedRTs;

    ResourceUsage resourceUsage;
    WGVKDevice device;
    uint32_t cacheIndex;
    uint32_t movedFrom;
    enum struct iotresult{
        thats_new, already_registered,
    };
    iotresult initializeOrTransition(WGVKTexture texture, VkImageLayout layout){
        auto it = resourceUsage.entryAndFinalLayouts.find(texture);
        if(it == resourceUsage.entryAndFinalLayouts.end()){
            resourceUsage.entryAndFinalLayouts.emplace(texture, std::make_pair(layout, layout));
            return iotresult::thats_new;
        }
        else{
            if(it->second.second != layout){
                wgvkCommandEncoderTransitionTextureLayout(this, texture, it->second.second, layout);
                it->second.second = layout;
            }
        }
        return iotresult::already_registered;
    }
}WGVKCommandEncoderImpl;

typedef struct WGVKCommandBufferImpl{
    VkCommandBuffer buffer;
    refcount_type refCount;
    ref_holder<WGVKRenderPassEncoder> referencedRPs;
    ref_holder<WGVKComputePassEncoder> referencedCPs;
    ref_holder<WGVKRaytracingPassEncoder> referencedRTs;
    ResourceUsage resourceUsage;
    
    WGVKDevice device;
    uint32_t cacheIndex;
}WGVKCommandBufferImpl;


typedef struct WGVKRaytracingPassEncoderImpl{
    VkCommandBuffer cmdBuffer;
    WGVKDevice device;
    WGVKRaytracingPipeline lastPipeline;

    ResourceUsage resourceUsage;
    refcount_type refCount;
    VkPipelineLayout lastLayout;
    WGVKCommandEncoder cmdEncoder;
}WGVKRaytracingPassEncoderImpl;


static inline RenderPassLayout GetRenderPassLayout(const WGVKRenderPassDescriptor* rpdesc){
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


inline bool is__depth(PixelFormat fmt){
    return fmt ==  Depth24 || fmt == Depth32;
}
inline bool is__depth(VkFormat fmt){
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


static inline LayoutedRenderPass LoadRenderPassFromLayout(WGVKDevice device, RenderPassLayout layout) {
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


struct WGVKSurfaceImpl{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    WGVKDevice device;
    uint32_t imagecount;

    uint32_t activeImageIndex;
    uint32_t width, height;
    VkFormat swapchainImageFormat;
    VkColorSpaceKHR swapchainColorSpace;
    WGVKTexture* images;
    WGVKTextureView* imageViews;
    //VkFramebuffer* framebuffers;

    uint32_t formatCount;
    PixelFormat* formatCache;
    uint32_t presentModeCount;
    PresentMode* presentModeCache;
};


//struct WGVKDevice{
//    VkDevice device;
//};
static inline VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0);
struct SyncState{
    std::vector<VkSemaphore> semaphores;
    VkSemaphore acquireImageSemaphore;
    bool acquireImageSemaphoreSignalled;
    uint32_t submits;
    VkFence renderFinishedFence;    
};

struct WGVKQueueImpl{
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    VkQueue presentQueue;

    SyncState syncState[framesInFlight];
    WGVKDevice device;

    WGVKCommandEncoder presubmitCache;
    std::unordered_map<VkFence, std::unordered_set<WGVKCommandBuffer>> pendingCommandBuffers[framesInFlight];
};

struct memory_types{
    uint32_t deviceLocal;
    uint32_t hostVisibleCoherent;
};

struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    WGVKAdapter physicalDevice = nullptr;
    VkPhysicalDeviceMemoryProperties memProperties;

    WGVKDevice device = nullptr;
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
static inline uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties zeroinit;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    assert(false && "failed to find suitable memory type!");
    return ~0u;
}
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
extern "C" void TransitionImageLayout(WGVKDevice device, VkCommandPool commandPool, VkQueue queue, WGVKTexture texture, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
extern "C" void EncodeTransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, WGVKTexture texture);
extern "C" VkCommandBuffer BeginSingleTimeCommands(WGVKDevice device, VkCommandPool commandPool);
extern "C" void EndSingleTimeCommandsAndSubmit(WGVKDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);
static inline VkSemaphore CreateSemaphoreD(VkDevice device, VkSemaphoreCreateFlags flags = 0){
    VkSemaphoreCreateInfo sci zeroinit;
    VkSemaphore ret zeroinit;
    sci.flags = flags;
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult res = vkCreateSemaphore(device, &sci, nullptr, &ret);
    if(res != VK_SUCCESS){
        TRACELOG(LOG_ERROR, "Error creating semaphore");
    }
    return ret;
}
VkBool32 RGFW_getVKPresentationSupport_noinline(VkInstance instance, VkPhysicalDevice, uint32_t i);
static inline VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags){
    return CreateSemaphoreD(g_vulkanstate.device->device, flags);
}

static inline VkFence CreateFence(VkFenceCreateFlags flags = 0){
    VkFenceCreateInfo sci zeroinit;
    VkFence ret zeroinit;
    sci.flags = flags;
    sci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkResult res = vkCreateFence(g_vulkanstate.device->device, &sci, nullptr, &ret);
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
    VkClearValue clearvalues[3] = {};
    clearvalues[0].color = VkClearColorValue{};
    clearvalues[0].color.float32[0] = rp->colorClear.r;
    clearvalues[0].color.float32[1] = rp->colorClear.g;
    clearvalues[0].color.float32[2] = rp->colorClear.b;
    clearvalues[0].color.float32[3] = rp->colorClear.a;
    clearvalues[1].depthStencil = VkClearDepthStencilValue{};
    clearvalues[1].depthStencil.depth = rp->depthClear;
    clearvalues[2].color = VkClearColorValue{};
    clearvalues[2].color.float32[0] = rp->colorClear.r;
    clearvalues[2].color.float32[1] = rp->colorClear.g;
    clearvalues[2].color.float32[2] = rp->colorClear.b;
    clearvalues[2].color.float32[3] = rp->colorClear.a;

    rpbi.renderPass = (VkRenderPass)rp->VkRenderPass;
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.clearValueCount = (rp->colorLoadOp == LoadOp_Clear) ? (3) : 0;
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


extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
//extern "C" DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count);
extern "C" void UpdateBindGroup(DescribedBindGroup* bg);
extern "C" DescribedBuffer* GenBufferEx(const void *data, size_t size, BufferUsage usage);
extern "C" void UnloadBuffer(DescribedBuffer* buf);

//wgvk I guess

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
    entry.textureView = (WGVKTextureView)tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}
static inline void BindVertexArray_Vk(WGVKRenderPassEncoder rpenc, VertexArray* vao){
    for(uint32_t i = 0;i < vao->buffers.size();i++){
        wgvkRenderPassEncoderBindVertexBuffer(rpenc, i, (WGVKBuffer)vao->buffers[i].first->buffer, 0);
    }
}






#endif
