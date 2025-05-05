#ifndef WGVK_STRUCTS_IMPL_H
#define WGVK_STRUCTS_IMPL_H
#include <wgvk.h>
#include <raygpu.h>
#include <unordered_map>
#include <external/VmaUsage.h>

//#define CC_NO_SHORT_NAMES
//#include <external/cc.h>

#include "../src/backend_vulkan/ptr_hash_map.h"

typedef struct ImageViewUsageRecord{
    VkImageLayout initialLayout;
    VkImageLayout lastLayout;
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
}ImageUsageRecord;

typedef struct BufferUsageRecord{
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
}BufferUsageRecord;

DEFINE_PTR_HASH_MAP(static inline, BufferUsageRecordMap, BufferUsageRecord)
DEFINE_PTR_HASH_MAP(static inline, ImageViewUsageRecordMap, ImageUsageRecord)
DEFINE_PTR_HASH_SET(static inline, BindGroupUsageSet)
DEFINE_PTR_HASH_SET(static inline, SamplerUsageSet)
DEFINE_PTR_HASH_SET(static inline, ImageUsageSet)
//DEFINE_PTR_HASH_MAP(static inline, BindGroupUsageMap, uint32_t)
//DEFINE_PTR_HASH_MAP(static inline, SamplerUsageMap, uint32_t)



typedef uint32_t refcount_type;

typedef struct ResourceUsage{
    BufferUsageRecordMap referencedBuffers;
    ref_holder<WGVKTexture> referencedTextures;
    ImageViewUsageRecordMap referencedTextureViews;
    ref_holder<WGVKBindGroup> referencedBindGroups;
    ref_holder<WGVKBindGroupLayout> referencedBindGroupLayouts;
    ref_holder<WGVKSampler> referencedSamplers;

    std::unordered_map<WGVKTexture, std::pair<VkImageLayout, VkImageLayout>> entryAndFinalLayouts;

    bool contains(WGVKBuffer buffer)const noexcept;
    bool contains(WGVKTexture texture)const noexcept;
    bool contains(WGVKTextureView view)const noexcept;
    bool contains(WGVKBindGroup bindGroup)const noexcept;
    bool contains(WGVKBindGroupLayout bindGroupLayout)const noexcept;
    bool contains(WGVKSampler bindGroup)const noexcept;

    void track(WGVKBuffer buffer)noexcept;
    void track(WGVKTexture texture)noexcept;
    void track(WGVKTextureView view, TextureUsage usage)noexcept;
    void track(WGVKBindGroup bindGroup)noexcept;
    void track(WGVKBindGroupLayout bindGroupLayout)noexcept;
    void track(WGVKSampler sampler)noexcept;
    
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

struct SafelyResettableCommandPool{
    VkCommandPool pool;
    VkFence finishingFence;
    WGVKDevice device;
    struct commandPoolRecord{
        VkCommandBuffer commandBuffer;
        VkSemaphore semaphore;
    };
    std::vector<commandPoolRecord> currentlyInUse;
    std::vector<commandPoolRecord> availableForUse;

    SafelyResettableCommandPool(WGVKDevice p_device); 
    void finish();
    void reset();    
};

struct SyncState{
    std::vector<VkSemaphore> semaphores;
    VkSemaphore acquireImageSemaphore;
    bool acquireImageSemaphoreSignalled;
    uint32_t submits;
    VkSemaphore& getHangingSemaphore(){
        return semaphores[submits];
    }
    //VkFence renderFinishedFence;    
};

typedef struct MappableBufferMemory{
    VkBuffer buffer;
    VmaAllocation allocation;
    VkMemoryPropertyFlags propertyFlags;
    size_t capacity;
}MappableBufferMemory;


constexpr uint32_t max_color_attachments = MAX_COLOR_ATTTACHMENTS;
typedef struct AttachmentDescriptor{
    VkFormat format;
    uint32_t sampleCount;
    LoadOp loadop;
    StoreOp storeop;
#ifdef __cplusplus
    bool operator==(const AttachmentDescriptor& other)const noexcept{
        return format == other.format
        && sampleCount == other.sampleCount
        && loadop == other.loadop
        && storeop == other.storeop;
    }
    bool operator!=(const AttachmentDescriptor& other) const noexcept{
        return !(*this == other);
    }
#endif
}AttachmentDescriptor;


typedef struct RenderPassLayout{
    uint32_t colorAttachmentCount;
    AttachmentDescriptor colorAttachments[max_color_attachments];
    AttachmentDescriptor colorResolveAttachments[max_color_attachments];
    uint32_t depthAttachmentPresent;
    AttachmentDescriptor depthAttachment;
    //uint32_t colorResolveIndex;
#ifdef __cplusplus
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
#endif
}RenderPassLayout;
typedef struct LayoutedRenderPass{
    RenderPassLayout layout;
    std::vector<VkAttachmentDescription> allAttachments;
    VkRenderPass renderPass;
}LayoutedRenderPass;

LayoutedRenderPass LoadRenderPassFromLayout(WGVKDevice device, RenderPassLayout layout);
RenderPassLayout GetRenderPassLayout(const WGVKRenderPassDescriptor* rpdesc);

struct wgvkxorshiftstate{
    uint64_t x64;
    constexpr void update(uint64_t x) noexcept{
        x64 ^= x * 0x2545F4914F6CDD1D;
        x64 ^= x64 << 13;
        x64 ^= x64 >> 7;
        x64 ^= x64 << 17;
    }
    constexpr void update(uint32_t x, uint32_t y)noexcept{
        x64 ^= ((uint64_t(x) << 32) | uint64_t(y)) * 0x2545F4914F6CDD1D;
        x64 ^= x64 << 13;
        x64 ^= x64 >> 7;
        x64 ^= x64 << 17;
    }
};

namespace std{
    template<>
    struct hash<RenderPassLayout>{
        constexpr size_t operator()(const RenderPassLayout& layout)const noexcept{

            wgvkxorshiftstate ret{0x2545F4918F6CDD1D};
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

struct QueueIndices{
    uint32_t graphicsIndex;
    uint32_t computeIndex;
    uint32_t transferIndex;
    uint32_t presentIndex;
};


typedef struct WGVKSamplerImpl{
    VkSampler sampler;
    refcount_type refCount;
    WGVKDevice device;
}WGVKSamplerImpl;

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

typedef struct WGVKPipelineLayoutImpl{
    VkPipelineLayout layout;
    WGVKBindGroupLayout* bindGroupLayouts;
    uint32_t bindGroupLayoutCount;
    refcount_type refCount;
}WGVKPipelineLayoutImpl;

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


typedef struct WGVKInstanceImpl{
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
}WGVKInstanceImpl;

typedef struct WGVKFutureImpl{
    void* userdataForFunction;
    void (*functionCalledOnWaitAny)(void*);
}WGVKFutureImpl;

typedef struct WGVKAdapterImpl{
    VkPhysicalDevice physicalDevice;
    WGVKInstance instance;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
    VkPhysicalDeviceMemoryProperties memProperties;
    QueueIndices queueIndices;
}WGVKAdapterImpl;

constexpr uint32_t framesInFlight = 2;

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
}WGVKTextureImpl;
typedef struct WGVKShaderModuleImpl{
    VkShaderModule vulkanModule;
}WGVKShaderModuleImpl;
typedef struct WGVKRenderPipelineImpl{
    VkPipeline renderPipeline;
    refcount_type refCount;
    WGVKDevice device;
    WGVKPipelineLayout layout;
    std::vector<VkDynamicState> dynamicStates;
}WGVKRenderPipelineImpl;


typedef struct WGVKComputePipelineImpl{
    VkPipeline computePipeline;
    WGVKPipelineLayout layout;
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

typedef struct WGVKSurfaceImpl{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    WGVKDevice device;
    WGVKSurfaceConfiguration lastConfig;
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
}WGVKSurfaceImpl;
typedef struct WGVKQueueImpl{
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    VkQueue presentQueue;

    SyncState syncState[framesInFlight];
    WGVKDevice device;

    WGVKCommandEncoder presubmitCache;
    std::unordered_map<VkFence, std::unordered_set<WGVKCommandBuffer>> pendingCommandBuffers[framesInFlight];
}WGVKQueueImpl;

#endif