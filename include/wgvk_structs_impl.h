#ifndef WGVK_STRUCTS_IMPL_H
#define WGVK_STRUCTS_IMPL_H
#include <wgvk.h>
#include <raygpu.h>
#include <external/VmaUsage.h>

//#define CC_NO_SHORT_NAMES
//#include <external/cc.h>

#include "../src/backend_vulkan/ptr_hash_map.h"

typedef struct ImageViewUsageRecord{
    VkImageLayout initialLayout;
    VkImageLayout lastLayout;
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
}ImageViewUsageRecord;

typedef struct BufferUsageRecord{
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
}BufferUsageRecord;

typedef struct ImageLayoutPair{
    VkImageLayout initialLayout;
    VkImageLayout finalLayout;
}ImageLayoutPair;

typedef enum RCPassCommandType{
    rp_command_type_invalid = 0,
    rp_command_type_draw,
    rp_command_type_draw_indexed,
    rp_command_type_set_vertex_buffer,
    rp_command_type_set_index_buffer,
    rp_command_type_set_bind_group,
    rp_command_type_set_render_pipeline,
    cp_command_type_set_compute_pipeline,
    cp_command_type_dispatch_workgroups,
    rp_command_type_set_force32 = 0x7fffffff
}RCPassCommandType;

typedef struct RenderPassCommandDraw{
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
}RenderPassCommandDraw;

typedef struct RenderPassCommandDrawIndexed{
    uint32_t indexCount;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t baseVertex;
    uint32_t firstInstance;
}RenderPassCommandDrawIndexed;

typedef struct RenderPassCommandSetBindGroup{
    uint32_t groupIndex;
    WGVKBindGroup group;
    size_t dynamicOffsetCount;
    const uint32_t* dynamicOffsets;
}RenderPassCommandSetBindGroup;
typedef struct RenderPassCommandSetVertexBuffer {
    uint32_t slot;
    WGVKBuffer buffer;
    uint64_t offset;
} RenderPassCommandSetVertexBuffer;

typedef struct RenderPassCommandSetIndexBuffer {
    WGVKBuffer buffer;
    IndexFormat format;
    uint64_t offset;
    uint64_t size;
} RenderPassCommandSetIndexBuffer;

typedef struct RenderPassCommandSetPipeline {
    WGVKRenderPipeline pipeline;
} RenderPassCommandSetPipeline;

typedef struct ComputePassCommandSetPipeline {
    WGVKComputePipeline pipeline;
} ComputePassCommandSetPipeline;

typedef struct ComputePassCommandDispatchWorkgroups {
    uint32_t x, y, z;
} ComputePassCommandDispatchWorkgroups;

typedef struct RenderPassCommandBegin{
    WGVKStringView label;
    size_t colorAttachmentCount;
    WGVKRenderPassColorAttachment colorAttachments[MAX_COLOR_ATTACHMENTS];
    Bool32 depthAttachmentPresent;
    WGVKRenderPassDepthStencilAttachment depthStencilAttachment;
}RenderPassCommandBegin;

typedef struct RenderPassCommandGeneric {
    RCPassCommandType type;
    union {
        RenderPassCommandDraw draw;
        RenderPassCommandDrawIndexed drawIndexed;
        RenderPassCommandSetVertexBuffer setVertexBuffer;
        RenderPassCommandSetIndexBuffer setIndexBuffer;
        RenderPassCommandSetBindGroup setBindGroup;
        RenderPassCommandSetPipeline setRenderPipeline;
        ComputePassCommandSetPipeline setComputePipeline;
        ComputePassCommandDispatchWorkgroups dispatchWorkgroups;
    };
}RenderPassCommandGeneric;

#define CONTAINERAPI static inline
DEFINE_PTR_HASH_MAP (CONTAINERAPI, BufferUsageRecordMap, BufferUsageRecord)
DEFINE_PTR_HASH_MAP (CONTAINERAPI, ImageViewUsageRecordMap, ImageViewUsageRecord)
DEFINE_PTR_HASH_MAP (CONTAINERAPI, LayoutAssumptions, ImageLayoutPair)
DEFINE_PTR_HASH_SET (CONTAINERAPI, BindGroupUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, BindGroupLayoutUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, SamplerUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, ImageUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGVKRenderPassEncoderSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, RenderPipelineUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, ComputePipelineUsageSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGVKComputePassEncoderSet)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGVKRaytracingPassEncoderSet)

DEFINE_VECTOR (CONTAINERAPI, VkWriteDescriptorSet, VkWriteDescriptorSetVector)
DEFINE_VECTOR (CONTAINERAPI, VkCommandBuffer, VkCommandBufferVector)
DEFINE_VECTOR (CONTAINERAPI, RenderPassCommandGeneric, RenderPassCommandGenericVector)
DEFINE_VECTOR (CONTAINERAPI, VkSemaphore, VkSemaphoreVector)
DEFINE_VECTOR (CONTAINERAPI, WGVKCommandBuffer, WGVKCommandBufferVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorBufferInfo, VkDescriptorBufferInfoVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorImageInfo, VkDescriptorImageInfoVector)
DEFINE_VECTOR (CONTAINERAPI, VkWriteDescriptorSetAccelerationStructureKHR, VkWriteDescriptorSetAccelerationStructureKHRVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorSetLayoutBinding, VkDescriptorSetLayoutBindingVector)

typedef struct DescriptorSetAndPool{
    VkDescriptorPool pool;
    VkDescriptorSet set;
}DescriptorSetAndPool;

DEFINE_VECTOR(static inline, VkAttachmentDescription, VkAttachmentDescriptionVector)
DEFINE_VECTOR(static inline, WGVKBuffer, WGVKBufferVector)
DEFINE_VECTOR(static inline, DescriptorSetAndPool, DescriptorSetAndPoolVector)
DEFINE_PTR_HASH_MAP(static inline, BindGroupCacheMap, DescriptorSetAndPoolVector)


//DEFINE_PTR_HASH_MAP(static inline, BindGroupUsageMap, uint32_t)
//DEFINE_PTR_HASH_MAP(static inline, SamplerUsageMap, uint32_t)

typedef uint32_t refcount_type;

typedef struct ResourceUsage{
    BufferUsageRecordMap referencedBuffers;
    ImageUsageSet referencedTextures;
    ImageViewUsageRecordMap referencedTextureViews;
    BindGroupUsageSet referencedBindGroups;
    BindGroupLayoutUsageSet referencedBindGroupLayouts;
    SamplerUsageSet referencedSamplers;
    RenderPipelineUsageSet referencedRenderPipelines;
    ComputePipelineUsageSet referencedComputePipelines;
    LayoutAssumptions entryAndFinalLayouts;
}ResourceUsage;

static inline void ResourceUsage_free(ResourceUsage* ru){
    BufferUsageRecordMap_free(&ru->referencedBuffers);
    ImageUsageSet_free(&ru->referencedTextures);
    ImageViewUsageRecordMap_free(&ru->referencedTextureViews);
    BindGroupUsageSet_free(&ru->referencedBindGroups);
    BindGroupLayoutUsageSet_free(&ru->referencedBindGroupLayouts);
    SamplerUsageSet_free(&ru->referencedSamplers);
    LayoutAssumptions_free(&ru->entryAndFinalLayouts);
}

static inline void ResourceUsage_move(ResourceUsage* dest, ResourceUsage* source){
    BufferUsageRecordMap_move(&dest->referencedBuffers, &source->referencedBuffers);
    ImageUsageSet_move(&dest->referencedTextures, &source->referencedTextures);
    ImageViewUsageRecordMap_move(&dest->referencedTextureViews, &source->referencedTextureViews);
    BindGroupUsageSet_move(&dest->referencedBindGroups, &source->referencedBindGroups);
    BindGroupLayoutUsageSet_move(&dest->referencedBindGroupLayouts, &source->referencedBindGroupLayouts);
    SamplerUsageSet_move(&dest->referencedSamplers, &source->referencedSamplers);
    LayoutAssumptions_move(&dest->entryAndFinalLayouts, &source->entryAndFinalLayouts);
}

static inline void ResourceUsage_init(ResourceUsage* ru){
    BufferUsageRecordMap_init(&ru->referencedBuffers);
    ImageUsageSet_init(&ru->referencedTextures);
    ImageViewUsageRecordMap_init(&ru->referencedTextureViews);
    BindGroupUsageSet_init(&ru->referencedBindGroups);
    BindGroupLayoutUsageSet_init(&ru->referencedBindGroupLayouts);
    SamplerUsageSet_init(&ru->referencedSamplers);
    LayoutAssumptions_init(&ru->entryAndFinalLayouts);
}

RGAPI void ru_registerTransition   (ResourceUsage* resourceUsage, WGVKTexture tex, VkImageLayout from, VkImageLayout to);
RGAPI void ru_trackBuffer          (ResourceUsage* resourceUsage, WGVKBuffer buffer, VkPipelineStageFlags stage, VkAccessFlags access);
RGAPI void ru_trackTexture         (ResourceUsage* resourceUsage, WGVKTexture texture);
RGAPI void ru_trackTextureView     (ResourceUsage* resourceUsage, WGVKTextureView view, ImageViewUsageRecord newRecird);
RGAPI void ru_trackBindGroup       (ResourceUsage* resourceUsage, WGVKBindGroup bindGroup);
RGAPI void ru_trackBindGroupLayout (ResourceUsage* resourceUsage, WGVKBindGroupLayout bindGroupLayout);
RGAPI void ru_trackSampler         (ResourceUsage* resourceUsage, WGVKSampler sampler);

RGAPI Bool32 ru_containsBuffer         (const ResourceUsage* resourceUsage, WGVKBuffer buffer);
RGAPI Bool32 ru_containsTexture        (const ResourceUsage* resourceUsage, WGVKTexture texture);
RGAPI Bool32 ru_containsTextureView    (const ResourceUsage* resourceUsage, WGVKTextureView view);
RGAPI Bool32 ru_containsBindGroup      (const ResourceUsage* resourceUsage, WGVKBindGroup bindGroup);
RGAPI Bool32 ru_containsBindGroupLayout(const ResourceUsage* resourceUsage, WGVKBindGroupLayout bindGroupLayout);
RGAPI Bool32 ru_containsSampler        (const ResourceUsage* resourceUsage, WGVKSampler bindGroup);

RGAPI void releaseAllAndClear(ResourceUsage* resourceUsage);

//struct SafelyResettableCommandPool{
//    VkCommandPool pool;
//    VkFence finishingFence;
//    WGVKDevice device;
//    struct commandPoolRecord{
//        VkCommandBuffer commandBuffer;
//        VkSemaphore semaphore;
//    };
//    std::vector<commandPoolRecord> currentlyInUse;
//    std::vector<commandPoolRecord> availableForUse;
//
//    SafelyResettableCommandPool(WGVKDevice p_device); 
//    void finish();
//    void reset();    
//};

typedef struct SyncState{
    VkSemaphoreVector semaphores;
    VkSemaphore acquireImageSemaphore;
    bool acquireImageSemaphoreSignalled;
    uint32_t submits;
    //VkFence renderFinishedFence;    
}SyncState;

typedef struct MappableBufferMemory{
    VkBuffer buffer;
    VmaAllocation allocation;
    VkMemoryPropertyFlags propertyFlags;
    size_t capacity;
}MappableBufferMemory;

#ifdef __cplusplus
constexpr uint32_t max_color_attachments = MAX_COLOR_ATTACHMENTS;
#else
#define max_color_attachments MAX_COLOR_ATTACHMENTS
#endif
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
static bool attachmentDescriptorCompare(AttachmentDescriptor a, AttachmentDescriptor b){
    return a.format      == b.format
        && a.sampleCount == b.sampleCount
        && a.loadop      == b.loadop
        && a.storeop     == b.storeop;
}


typedef struct RenderPassLayout{
    uint32_t colorAttachmentCount;
    AttachmentDescriptor colorAttachments [4];
    AttachmentDescriptor colorResolveAttachments [4];
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

static bool renderPassLayoutCompare(RenderPassLayout first, RenderPassLayout other){
    if(first.colorAttachmentCount != other.colorAttachmentCount)return false;
    for(uint32_t i = 0;i < first.colorAttachmentCount;i++){
        if(!attachmentDescriptorCompare(first.colorAttachments[i], other.colorAttachments[i])){
            return false;
        }
    }
    return (first.depthAttachmentPresent == other.depthAttachmentPresent) && attachmentDescriptorCompare(first.depthAttachment, other.depthAttachment);
}

typedef struct LayoutedRenderPass{
    RenderPassLayout layout;
    VkAttachmentDescriptionVector allAttachments;
    VkRenderPass renderPass;
}LayoutedRenderPass;
#ifdef __cplusplus
extern "C"{
#endif
LayoutedRenderPass LoadRenderPassFromLayout(WGVKDevice device, RenderPassLayout layout);
RenderPassLayout GetRenderPassLayout(const WGVKRenderPassDescriptor* rpdesc);
#ifdef __cplusplus
}
#endif
typedef struct wgvkxorshiftstate{
    uint64_t x64;
    #ifdef __cplusplus
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
    #endif
}wgvkxorshiftstate;
static inline void xs_update_u32(wgvkxorshiftstate* state, uint32_t x, uint32_t y){
    state->x64 ^= ((((uint64_t)x) << 32) | ((uint64_t)y)) * 0x2545F4914F6CDD1D;
    state->x64 ^= state->x64 << 13;
    state->x64 ^= state->x64 >> 7;
    state->x64 ^= state->x64 << 17;

}
#ifdef __cplusplus
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
#endif

static size_t renderPassLayoutHash(RenderPassLayout layout){
    wgvkxorshiftstate ret = {.x64 = 0x2545F4918F6CDD1D};
    xs_update_u32(&ret, layout.depthAttachmentPresent << 6, layout.colorAttachmentCount);
    for(uint32_t i = 0;i < layout.colorAttachmentCount;i++){
        xs_update_u32(&ret, layout.colorAttachments[i].format, layout.colorAttachments[i].sampleCount);
        xs_update_u32(&ret, layout.colorAttachments[i].loadop, layout.colorAttachments[i].storeop);
    }
    xs_update_u32(&ret, layout.depthAttachmentPresent << 6, layout.depthAttachmentPresent);
    if(layout.depthAttachmentPresent){
        xs_update_u32(&ret, layout.depthAttachment.format, layout.depthAttachment.sampleCount);
        xs_update_u32(&ret, layout.depthAttachment.loadop, layout.depthAttachment.storeop);
    }
    return ret.x64;
}


typedef struct PerframeCache{
    VkCommandPool commandPool;
    VkCommandBufferVector commandBuffers;

    WGVKBufferVector unusedBatchBuffers;
    WGVKBufferVector usedBatchBuffers;
    
    VkCommandBuffer finalTransitionBuffer;
    VkSemaphore finalTransitionSemaphore;
    VkFence finalTransitionFence;
    //std::map<uint64_t, small_vector<MappableBufferMemory>> stagingBufferCache;
    //std::unordered_map<WGVKBindGroupLayout, std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>>> bindGroupCache;
    BindGroupCacheMap bindGroupCache;
}PerframeCache;

typedef struct QueueIndices{
    uint32_t graphicsIndex;
    uint32_t computeIndex;
    uint32_t transferIndex;
    uint32_t presentIndex;
}QueueIndices;

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

#define framesInFlight 2

DEFINE_GENERIC_HASH_MAP(CONTAINERAPI, RenderPassCache, RenderPassLayout, LayoutedRenderPass, renderPassLayoutHash, renderPassLayoutCompare, CLITERAL(RenderPassLayout){0});
typedef struct WGVKDeviceImpl{
    VkDevice device;
    WGVKAdapter adapter;
    WGVKQueue queue;
    size_t submittedFrames;
    VmaAllocator allocator;
    VmaPool aligned_hostVisiblePool;
    PerframeCache frameCaches[framesInFlight];

    RenderPassCache renderPassCache;
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
DEFINE_VECTOR(static inline, VkDynamicState, VkDynamicStateVector)
typedef struct WGVKRenderPipelineImpl{
    VkPipeline renderPipeline;
    WGVKPipelineLayout layout;
    refcount_type refCount;
    WGVKDevice device;
    VkDynamicStateVector dynamicStates;
}WGVKRenderPipelineImpl;

typedef struct WGVKComputePipelineImpl{
    VkPipeline computePipeline;
    WGVKPipelineLayout layout;
    refcount_type refCount;
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
    VkImageSubresourceRange subresourceRange;
    uint32_t width, height, depthOrArrayLayers;
    uint32_t sampleCount;
}WGVKTextureViewImpl;

typedef struct CommandBufferAndLayout{
    VkCommandBuffer buffer;
    VkPipelineLayout lastLayout;
}CommandBufferAndLayout;
void recordVkCommand(CommandBufferAndLayout* destination, const RenderPassCommandGeneric* command);
void recordVkCommands(VkCommandBuffer destination, const RenderPassCommandGenericVector* commands);

typedef struct WGVKRenderPassEncoderImpl{
    VkRenderPass renderPass; //ONLY if !dynamicRendering

    RenderPassCommandBegin beginInfo;
    RenderPassCommandGenericVector bufferedCommands;
    
    WGVKDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    VkPipelineLayout lastLayout;
    VkFramebuffer frameBuffer;
    WGVKCommandEncoder cmdEncoder;
}WGVKRenderPassEncoderImpl;

typedef struct WGVKComputePassEncoderImpl{
    RenderPassCommandGenericVector bufferedCommands;
    WGVKDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    VkPipelineLayout lastLayout;
    WGVKCommandEncoder cmdEncoder;
}WGVKComputePassEncoderImpl;



typedef struct WGVKCommandEncoderImpl{
    VkCommandBuffer buffer;
    
    WGVKRenderPassEncoderSet referencedRPs;
    WGVKComputePassEncoderSet referencedCPs;
    WGVKRaytracingPassEncoderSet referencedRTs;

    ResourceUsage resourceUsage;
    WGVKDevice device;
    uint32_t cacheIndex;
    uint32_t movedFrom;
    
    
}WGVKCommandEncoderImpl;
typedef enum WGVKiotresult{
    iot_thats_new, iot_already_registered,
}WGVKiotresult;

static inline WGVKiotresult initializeOrTransition(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout layout){
    ResourceUsage* ru = &encoder->resourceUsage;
    ImageLayoutPair* if_layout = LayoutAssumptions_get(&ru->entryAndFinalLayouts, (void*)texture);
    if(if_layout == NULL){
        ImageLayoutPair empl = {
            .initialLayout = layout,
            .finalLayout = layout
        };
        LayoutAssumptions_put(&ru->entryAndFinalLayouts, (void*)texture, empl);
        return iot_thats_new;
    }
    else{
        if(if_layout->finalLayout != layout){
            wgvkCommandEncoderTransitionTextureLayout(encoder, texture, if_layout->finalLayout, layout);
            if_layout->finalLayout = layout;
        }
        return iot_already_registered;
    }
}
typedef struct WGVKCommandBufferImpl{
    VkCommandBuffer buffer;
    refcount_type refCount;
    WGVKRenderPassEncoderSet referencedRPs;
    WGVKComputePassEncoderSet referencedCPs;
    WGVKRaytracingPassEncoderSet referencedRTs;
    
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
DEFINE_PTR_HASH_MAP(CONTAINERAPI, PendingCommandBufferMap, WGVKCommandBufferVector);
typedef struct WGVKQueueImpl{
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    VkQueue presentQueue;

    SyncState syncState[framesInFlight];
    WGVKDevice device;

    WGVKCommandEncoder presubmitCache;

    PendingCommandBufferMap pendingCommandBuffers[framesInFlight];
}WGVKQueueImpl;

#endif