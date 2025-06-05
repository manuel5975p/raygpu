#ifndef WGPU_STRUCTS_IMPL_H
#define WGPU_STRUCTS_IMPL_H
#include <wgvk.h>
#include <raygpu.h>
#include <external/VmaUsage.h>

#include "../src/backend_vulkan/ptr_hash_map.h"

typedef struct ImageUsageRecord{
    VkImageLayout initialLayout;
    VkImageLayout lastLayout;
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
    VkImageSubresourceRange lastAccessedSubresource;
}ImageUsageRecord;

typedef struct ImageUsageSnap{
    VkImageLayout layout;
    VkPipelineStageFlags stage;
    VkAccessFlags access;
    VkImageSubresourceRange subresource;
}ImageUsageSnap;

typedef struct BufferUsageRecord{
    VkPipelineStageFlags lastStage;
    VkAccessFlags lastAccess;
    VkBool32 everWrittenTo;
}BufferUsageRecord;

typedef struct BufferUsageSnap{
    VkPipelineStageFlags stage;
    VkAccessFlags access;
}BufferUsageSnap;

typedef enum RCPassCommandType{
    rp_command_type_invalid = 0,
    rp_command_type_draw,
    rp_command_type_draw_indexed,
    rp_command_type_draw_indexed_indirect,
    rp_command_type_draw_indirect,
    rp_command_type_set_blend_constant,
    rp_command_type_set_viewport,
    rp_command_type_set_vertex_buffer,
    rp_command_type_set_index_buffer,
    rp_command_type_set_bind_group,
    rp_command_type_set_render_pipeline,
    cp_command_type_set_compute_pipeline,
    cp_command_type_dispatch_workgroups,
    rp_command_type_enum_count,
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
    WGPUBindGroup group;
    VkPipelineBindPoint bindPoint;
    size_t dynamicOffsetCount;
    const uint32_t* dynamicOffsets;
}RenderPassCommandSetBindGroup;
typedef struct RenderPassCommandSetVertexBuffer {
    uint32_t slot;
    WGPUBuffer buffer;
    uint64_t offset;
} RenderPassCommandSetVertexBuffer;

typedef struct RenderPassCommandSetIndexBuffer {
    WGPUBuffer buffer;
    WGPUIndexFormat format;
    uint64_t offset;
    uint64_t size;
} RenderPassCommandSetIndexBuffer;

typedef struct RenderPassCommandSetPipeline {
    WGPURenderPipeline pipeline;
} RenderPassCommandSetPipeline;

typedef struct ComputePassCommandSetPipeline {
    WGPUComputePipeline pipeline;
} ComputePassCommandSetPipeline;

typedef struct ComputePassCommandDispatchWorkgroups {
    uint32_t x, y, z;
} ComputePassCommandDispatchWorkgroups;

typedef struct RenderPassCommandSetViewport{
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
}RenderPassCommandSetViewport;
typedef struct RenderPassCommandDrawIndexedIndirect{
    WGPUBuffer indirectBuffer;
    uint64_t indirectOffset;
}RenderPassCommandDrawIndexedIndirect;

typedef struct RenderPassCommandDrawIndirect{
    WGPUBuffer indirectBuffer;
    uint64_t indirectOffset;
}RenderPassCommandDrawIndirect;

typedef struct RenderPassCommandSetBlendConstant{
    WGPUColor color;
}RenderPassCommandSetBlendConstant;


typedef struct RenderPassCommandBegin{
    WGPUStringView label;
    size_t colorAttachmentCount;
    WGPURenderPassColorAttachment colorAttachments[MAX_COLOR_ATTACHMENTS];
    Bool32 depthAttachmentPresent;
    WGPURenderPassDepthStencilAttachment depthStencilAttachment;
}RenderPassCommandBegin;

typedef struct RenderPassCommandGeneric {
    RCPassCommandType type;
    union {
        RenderPassCommandDraw draw;
        RenderPassCommandDrawIndexed drawIndexed;
        RenderPassCommandSetViewport setViewport;
        RenderPassCommandDrawIndexedIndirect drawIndexedIndirect;
        RenderPassCommandDrawIndirect drawIndirect;
        RenderPassCommandSetBlendConstant setBlendConstant;
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
//DEFINE_PTR_HASH_MAP (CONTAINERAPI, ImageViewUsageRecordMap, ImageViewUsageRecord)
//DEFINE_PTR_HASH_MAP (CONTAINERAPI, LayoutAssumptions, ImageLayoutPair)
DEFINE_PTR_HASH_MAP (CONTAINERAPI, ImageUsageRecordMap, ImageUsageRecord)
DEFINE_PTR_HASH_SET (CONTAINERAPI, BindGroupUsageSet, WGPUBindGroup)
DEFINE_PTR_HASH_SET (CONTAINERAPI, BindGroupLayoutUsageSet, WGPUBindGroupLayout)
DEFINE_PTR_HASH_SET (CONTAINERAPI, SamplerUsageSet, WGPUSampler)
DEFINE_PTR_HASH_SET (CONTAINERAPI, ImageViewUsageSet, WGPUTextureView)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGPURenderPassEncoderSet, WGPURenderPassEncoder)
DEFINE_PTR_HASH_SET (CONTAINERAPI, RenderPipelineUsageSet, WGPURenderPipeline)
DEFINE_PTR_HASH_SET (CONTAINERAPI, ComputePipelineUsageSet, WGPUComputePipeline)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGPUComputePassEncoderSet, WGPUComputePassEncoder)
DEFINE_PTR_HASH_SET (CONTAINERAPI, WGPURaytracingPassEncoderSet, WGPURaytracingPassEncoder)

DEFINE_VECTOR(static inline, VkDynamicState, VkDynamicStateVector)
DEFINE_VECTOR (CONTAINERAPI, VkWriteDescriptorSet, VkWriteDescriptorSetVector)
DEFINE_VECTOR (CONTAINERAPI, WGPUFence, WGPUFenceVector)
DEFINE_VECTOR (CONTAINERAPI, VkFence, VkFenceVector)
DEFINE_VECTOR (CONTAINERAPI, VkCommandBuffer, VkCommandBufferVector)
DEFINE_VECTOR (CONTAINERAPI, RenderPassCommandGeneric, RenderPassCommandGenericVector)
DEFINE_VECTOR (CONTAINERAPI, VkSemaphore, VkSemaphoreVector)
DEFINE_VECTOR (CONTAINERAPI, WGPUCommandBuffer, WGPUCommandBufferVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorBufferInfo, VkDescriptorBufferInfoVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorImageInfo, VkDescriptorImageInfoVector)
DEFINE_VECTOR (CONTAINERAPI, VkWriteDescriptorSetAccelerationStructureKHR, VkWriteDescriptorSetAccelerationStructureKHRVector)
DEFINE_VECTOR (CONTAINERAPI, VkDescriptorSetLayoutBinding, VkDescriptorSetLayoutBindingVector)

DEFINE_PTR_HASH_MAP(CONTAINERAPI, PendingCommandBufferMap, WGPUCommandBufferVector);

typedef struct DescriptorSetAndPool{
    VkDescriptorPool pool;
    VkDescriptorSet set;
}DescriptorSetAndPool;

DEFINE_VECTOR(static inline, VkAttachmentDescription, VkAttachmentDescriptionVector)
DEFINE_VECTOR(static inline, WGPUBuffer, WGPUBufferVector)
DEFINE_VECTOR(static inline, DescriptorSetAndPool, DescriptorSetAndPoolVector)
DEFINE_PTR_HASH_MAP(static inline, BindGroupCacheMap, DescriptorSetAndPoolVector)


//DEFINE_PTR_HASH_MAP(static inline, BindGroupUsageMap, uint32_t)
//DEFINE_PTR_HASH_MAP(static inline, SamplerUsageMap, uint32_t)

typedef uint32_t refcount_type;

typedef struct ResourceUsage{
    BufferUsageRecordMap referencedBuffers;
    ImageUsageRecordMap referencedTextures;
    ImageViewUsageSet referencedTextureViews;
    BindGroupUsageSet referencedBindGroups;
    BindGroupLayoutUsageSet referencedBindGroupLayouts;
    SamplerUsageSet referencedSamplers;
    RenderPipelineUsageSet referencedRenderPipelines;
    ComputePipelineUsageSet referencedComputePipelines;
    //LayoutAssumptions entryAndFinalLayouts;
}ResourceUsage;

static inline void ResourceUsage_free(ResourceUsage* ru){
    BufferUsageRecordMap_free(&ru->referencedBuffers);
    ImageUsageRecordMap_free(&ru->referencedTextures);
    ImageViewUsageSet_free(&ru->referencedTextureViews);
    BindGroupUsageSet_free(&ru->referencedBindGroups);
    BindGroupLayoutUsageSet_free(&ru->referencedBindGroupLayouts);
    SamplerUsageSet_free(&ru->referencedSamplers);
    //LayoutAssumptions_free(&ru->entryAndFinalLayouts);
}

static inline void ResourceUsage_move(ResourceUsage* dest, ResourceUsage* source){
    BufferUsageRecordMap_move(&dest->referencedBuffers, &source->referencedBuffers);
    ImageUsageRecordMap_move(&dest->referencedTextures, &source->referencedTextures);
    ImageViewUsageSet_move(&dest->referencedTextureViews, &source->referencedTextureViews);
    BindGroupUsageSet_move(&dest->referencedBindGroups, &source->referencedBindGroups);
    BindGroupLayoutUsageSet_move(&dest->referencedBindGroupLayouts, &source->referencedBindGroupLayouts);
    SamplerUsageSet_move(&dest->referencedSamplers, &source->referencedSamplers);
    //LayoutAssumptions_move(&dest->entryAndFinalLayouts, &source->entryAndFinalLayouts);
}

static inline void ResourceUsage_init(ResourceUsage* ru){
    BufferUsageRecordMap_init(&ru->referencedBuffers);
    ImageUsageRecordMap_init(&ru->referencedTextures);
    ImageViewUsageSet_init(&ru->referencedTextureViews);
    BindGroupUsageSet_init(&ru->referencedBindGroups);
    BindGroupLayoutUsageSet_init(&ru->referencedBindGroupLayouts);
    SamplerUsageSet_init(&ru->referencedSamplers);
    //LayoutAssumptions_init(&ru->entryAndFinalLayouts);
}

RGAPI void ru_registerTransition   (ResourceUsage* resourceUsage, WGPUTexture tex, VkImageLayout from, VkImageLayout to);
RGAPI void ru_trackBuffer          (ResourceUsage* resourceUsage, WGPUBuffer buffer, BufferUsageRecord brecord);
RGAPI void ru_trackTexture         (ResourceUsage* resourceUsage, WGPUTexture texture, ImageUsageRecord newRecord);
RGAPI void ru_trackTextureView     (ResourceUsage* resourceUsage, WGPUTextureView view);
RGAPI void ru_trackBindGroup       (ResourceUsage* resourceUsage, WGPUBindGroup bindGroup);
RGAPI void ru_trackBindGroupLayout (ResourceUsage* resourceUsage, WGPUBindGroupLayout bindGroupLayout);
RGAPI void ru_trackSampler         (ResourceUsage* resourceUsage, WGPUSampler sampler);
RGAPI void ru_trackRenderPipeline  (ResourceUsage* resourceUsage, WGPURenderPipeline rpl);
RGAPI void ru_trackComputePipeline (ResourceUsage* resourceUsage, WGPUComputePipeline computePipeline);

RGAPI void ce_trackBuffer(WGPUCommandEncoder encoder, WGPUBuffer buffer, BufferUsageSnap usage);
RGAPI void ce_trackTexture(WGPUCommandEncoder encoder, WGPUTexture texture, ImageUsageSnap usage);
RGAPI void ce_trackTextureView(WGPUCommandEncoder encoder, WGPUTextureView view, ImageUsageSnap usage);

RGAPI Bool32 ru_containsBuffer         (const ResourceUsage* resourceUsage, WGPUBuffer buffer);
RGAPI Bool32 ru_containsTexture        (const ResourceUsage* resourceUsage, WGPUTexture texture);
RGAPI Bool32 ru_containsTextureView    (const ResourceUsage* resourceUsage, WGPUTextureView view);
RGAPI Bool32 ru_containsBindGroup      (const ResourceUsage* resourceUsage, WGPUBindGroup bindGroup);
RGAPI Bool32 ru_containsBindGroupLayout(const ResourceUsage* resourceUsage, WGPUBindGroupLayout bindGroupLayout);
RGAPI Bool32 ru_containsSampler        (const ResourceUsage* resourceUsage, WGPUSampler bindGroup);

RGAPI void releaseAllAndClear(ResourceUsage* resourceUsage);

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
    WGPULoadOp loadop;
    WGPUStoreOp storeop;
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
LayoutedRenderPass LoadRenderPassFromLayout(WGPUDevice device, RenderPassLayout layout);
RenderPassLayout GetRenderPassLayout(const WGPURenderPassDescriptor* rpdesc);
#ifdef __cplusplus
}
#endif
typedef struct wgpuxorshiftstate{
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
}wgpuxorshiftstate;
static inline void xs_update_u32(wgpuxorshiftstate* state, uint32_t x, uint32_t y){
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

            wgpuxorshiftstate ret{0x2545F4918F6CDD1D};
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
    wgpuxorshiftstate ret = {.x64 = 0x2545F4918F6CDD1D};
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

    WGPUBufferVector unusedBatchBuffers;
    WGPUBufferVector usedBatchBuffers;
    
    VkCommandBuffer finalTransitionBuffer;
    VkSemaphore finalTransitionSemaphore;
    WGPUFence finalTransitionFence;
    //std::map<uint64_t, small_vector<MappableBufferMemory>> stagingBufferCache;
    //std::unordered_map<WGPUBindGroupLayout, std::vector<std::pair<VkDescriptorPool, VkDescriptorSet>>> bindGroupCache;
    BindGroupCacheMap bindGroupCache;
    VkFenceVector reusableFences;
}PerframeCache;

typedef struct QueueIndices{
    uint32_t graphicsIndex;
    uint32_t computeIndex;
    uint32_t transferIndex;
    uint32_t presentIndex;
}QueueIndices;

typedef struct WGPUSamplerImpl{
    VkSampler sampler;
    refcount_type refCount;
    WGPUDevice device;
}WGPUSamplerImpl;

typedef struct CallbackWithUserdata{
    void(*callback)(void*);
    void* userdata;
    void(*freeUserData)(void*);
}CallbackWithUserdata;

DEFINE_VECTOR(static inline, CallbackWithUserdata, CallbackWithUserdataVector);

typedef enum WGPUFenceState{
    WGPUFenceState_Reset,
    WGPUFenceState_InUse,
    WGPUFenceState_Finished,
    WGPUFenceState_Force32 = 0x7FFFFFFF,
}WGPUFenceState;
typedef struct WGPUFenceImpl{
    VkFence fence;
    WGPUFenceState state;
    WGPUDevice device;
    refcount_type refCount;
    CallbackWithUserdataVector callbacksOnWaitComplete;
}WGPUFenceImpl;

typedef struct PendingCommandBufferListRef{
    WGPUFence fence;
    PendingCommandBufferMap* map;
}PendingCommandBufferListRef;

typedef struct WGPUBindGroupImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    WGPUBindGroupLayout layout;
    refcount_type refCount;
    ResourceUsage resourceUsage;
    WGPUDevice device;
    uint32_t cacheIndex;
    WGPUBindGroupEntry* entries;
    uint32_t entryCount;
}WGPUBindGroupImpl;

typedef struct WGPUBindGroupLayoutImpl{
    VkDescriptorSetLayout layout;
    WGPUDevice device;
    WGPUBindGroupLayoutEntry* entries;
    uint32_t entryCount;

    refcount_type refCount;
}WGPUBindGroupLayoutImpl;

typedef struct WGPUPipelineLayoutImpl{
    VkPipelineLayout layout;
    WGPUDevice device;
    WGPUBindGroupLayout* bindGroupLayouts;
    uint32_t bindGroupLayoutCount;
    refcount_type refCount;
}WGPUPipelineLayoutImpl;

typedef struct WGPUBufferImpl{
    VkBuffer buffer;
    WGPUDevice device;
    uint32_t cacheIndex;
    WGPUBufferUsage usage;
    size_t capacity;
    VmaAllocation allocation;
    VkMemoryPropertyFlags memoryProperties;
    VkDeviceAddress address; //uint64_t, if applicable (BufferUsage_ShaderDeviceAddress)
    refcount_type refCount;
    WGPUFence latestFence;
}WGPUBufferImpl;

typedef struct WGPUBottomLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
} WGPUBottomLevelAccelerationStructureImpl;
typedef WGPUBottomLevelAccelerationStructureImpl *WGPUBottomLevelAccelerationStructure;

typedef struct WGPUTopLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
    VkBuffer instancesBuffer;
    VkDeviceMemory instancesBufferMemory;
} WGPUTopLevelAccelerationStructureImpl;


typedef struct WGPUInstanceImpl{
    VkInstance instance;
    uint32_t refCount;
    VkDebugUtilsMessengerEXT debugMessenger;
}WGPUInstanceImpl;

typedef struct WGPUFutureImpl{
    void* userdataForFunction;
    void (*functionCalledOnWaitAny)(void*);
}WGPUFutureImpl;

typedef struct WGPUAdapterImpl{
    VkPhysicalDevice physicalDevice;
    uint32_t refCount;
    WGPUInstance instance;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
    VkPhysicalDeviceMemoryProperties memProperties;
    QueueIndices queueIndices;
}WGPUAdapterImpl;

#define framesInFlight 2

DEFINE_GENERIC_HASH_MAP(CONTAINERAPI, RenderPassCache, RenderPassLayout, LayoutedRenderPass, renderPassLayoutHash, renderPassLayoutCompare, CLITERAL(RenderPassLayout){0});
typedef struct WGPUDeviceImpl{
    VkDevice device;
    uint32_t refCount;
    WGPUAdapter adapter;
    WGPUQueue queue;
    size_t submittedFrames;
    VmaAllocator allocator;
    VmaPool aligned_hostVisiblePool;
    PerframeCache frameCaches[framesInFlight];

    RenderPassCache renderPassCache;
    WGPUUncapturedErrorCallbackInfo uncapturedErrorCallbackInfo;
    
    struct VolkDeviceTable functions;
}WGPUDeviceImpl;

typedef struct WGPUTextureImpl{
    VkImage image;
    VkFormat format;
    VkImageLayout layout;
    VkDeviceMemory memory;
    WGPUDevice device;
    refcount_type refCount;
    uint32_t width, height, depthOrArrayLayers;
    uint32_t mipLevels;
    uint32_t sampleCount;
}WGPUTextureImpl;
typedef struct WGPUShaderModuleImpl{
    uint32_t refCount;
    VkShaderModule vulkanModule;
    
    WGPUChainedStruct* source;
}WGPUShaderModuleImpl;

typedef struct WGPURenderPipelineImpl{
    VkPipeline renderPipeline;
    WGPUPipelineLayout layout;
    refcount_type refCount;
    WGPUDevice device;
    VkDynamicStateVector dynamicStates;
}WGPURenderPipelineImpl;

typedef struct WGPUComputePipelineImpl{
    VkPipeline computePipeline;
    WGPUPipelineLayout layout;
    refcount_type refCount;
}WGPUComputePipelineImpl;

typedef struct WGPURaytracingPipelineImpl{
    VkPipeline raytracingPipeline;
    VkPipelineLayout layout;
    WGPUBuffer raygenBindingTable;
    WGPUBuffer missBindingTable;
    WGPUBuffer hitBindingTable;
}WGPURaytracingPipelineImpl;

typedef struct WGPUTextureViewImpl{
    VkImageView view;
    VkFormat format;
    refcount_type refCount;
    WGPUTexture texture;
    VkImageSubresourceRange subresourceRange;
    uint32_t width, height, depthOrArrayLayers;
    uint32_t sampleCount;
}WGPUTextureViewImpl;

typedef struct CommandBufferAndSomeState{
    VkCommandBuffer buffer;
    VkPipelineLayout lastLayout;
    WGPUDevice device;
    WGPUBuffer vertexBuffers[8];
    WGPUBuffer indexBuffer;
    WGPUBindGroup graphicsBindGroups[8];
    WGPUBindGroup computeBindGroups[8];
}CommandBufferAndSomeState;
void recordVkCommand(CommandBufferAndSomeState* destination, const RenderPassCommandGeneric* command, const RenderPassCommandBegin *beginInfo);
void recordVkCommands(VkCommandBuffer destination, WGPUDevice device, const RenderPassCommandGenericVector* commands, const RenderPassCommandBegin *beginInfo);

typedef struct WGPURenderPassEncoderImpl{
    VkRenderPass renderPass; //ONLY if !dynamicRendering

    RenderPassCommandBegin beginInfo;
    RenderPassCommandGenericVector bufferedCommands;
    
    WGPUDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    WGPUPipelineLayout lastLayout;
    VkFramebuffer frameBuffer;
    WGPUCommandEncoder cmdEncoder;
}WGPURenderPassEncoderImpl;

typedef struct WGPUComputePassEncoderImpl{
    RenderPassCommandGenericVector bufferedCommands;
    WGPUDevice device;
    ResourceUsage resourceUsage;
    refcount_type refCount;

    WGPUPipelineLayout lastLayout;
    WGPUCommandEncoder cmdEncoder;
    WGPUBindGroup bindGroups[8];
}WGPUComputePassEncoderImpl;

void RenderPassEncoder_PushCommand(WGPURenderPassEncoder, const RenderPassCommandGeneric* cmd);
void ComputePassEncoder_PushCommand(WGPUComputePassEncoder, const RenderPassCommandGeneric* cmd);

typedef struct WGPUCommandEncoderImpl{
    VkCommandBuffer buffer;
    
    WGPURenderPassEncoderSet referencedRPs;
    WGPUComputePassEncoderSet referencedCPs;
    WGPURaytracingPassEncoderSet referencedRTs;

    ResourceUsage resourceUsage;
    WGPUDevice device;
    uint32_t cacheIndex;
    uint32_t movedFrom;
    
    
}WGPUCommandEncoderImpl;
typedef struct WGPUCommandBufferImpl{
    VkCommandBuffer buffer;
    refcount_type refCount;
    WGPURenderPassEncoderSet referencedRPs;
    WGPUComputePassEncoderSet referencedCPs;
    WGPURaytracingPassEncoderSet referencedRTs;
    
    ResourceUsage resourceUsage;
    
    WGPUDevice device;
    uint32_t cacheIndex;
}WGPUCommandBufferImpl;


typedef struct WGPURaytracingPassEncoderImpl{
    VkCommandBuffer cmdBuffer;
    WGPUDevice device;
    WGPURaytracingPipeline lastPipeline;

    ResourceUsage resourceUsage;
    refcount_type refCount;
    VkPipelineLayout lastLayout;
    WGPUCommandEncoder cmdEncoder;
}WGPURaytracingPassEncoderImpl;

typedef struct WGPUSurfaceImpl{
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    WGPUDevice device;
    WGPUSurfaceConfiguration lastConfig;
    uint32_t imagecount;

    uint32_t activeImageIndex;
    uint32_t width, height;
    VkFormat swapchainImageFormat;
    VkColorSpaceKHR swapchainColorSpace;
    WGPUTexture* images;
    WGPUTextureView* imageViews;
    VkSemaphore* presentSemaphores;
    //VkFramebuffer* framebuffers;

    uint32_t formatCount;
    VkSurfaceFormatKHR* formatCache;
    uint32_t presentModeCount;
    WGPUPresentMode* presentModeCache;
}WGPUSurfaceImpl;

typedef struct WGPUQueueImpl{
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
    VkQueue presentQueue;
    uint32_t refCount;

    SyncState syncState[framesInFlight];
    WGPUDevice device;

    WGPUCommandEncoder presubmitCache;

    PendingCommandBufferMap pendingCommandBuffers[framesInFlight];
}WGPUQueueImpl;


#ifndef WGPU_VALIDATION_ENABLED
#define WGPU_VALIDATION_ENABLED 1
#endif




#if WGPU_VALIDATION_ENABLED
#include <stdio.h> // For snprintf
#include <string.h> // For strlen, strcmp

// WGPU_VALIDATE_IMPL remains the same
#define WGPU_VALIDATE_IMPL(device_ptr, condition, error_type, message_str, log_level_on_fail) \
    do { \
        if (!(condition)) { \
            const char* resolved_message = (message_str); \
            size_t message_len = strlen(resolved_message); \
            if ((device_ptr) && (device_ptr)->uncapturedErrorCallbackInfo.callback) { \
                (device_ptr)->uncapturedErrorCallbackInfo.callback( \
                    (const WGPUDevice*)(device_ptr), \
                    (error_type), \
                    (WGPUStringView){resolved_message, message_len}, \
                    (device_ptr)->uncapturedErrorCallbackInfo.userdata1, \
                    (device_ptr)->uncapturedErrorCallbackInfo.userdata2 \
                ); \
            } \
            TRACELOG(log_level_on_fail, "Validation Failed: %s", resolved_message); \
            rassert(condition, resolved_message); \
        } \
    } while (0)

#define WGPU_VALIDATE(device_ptr, condition, message_str) WGPU_VALIDATE_IMPL(device_ptr, condition, WGPUErrorType_Validation, message_str, LOG_ERROR)
#define WGPU_VALIDATE_PTR(device_ptr, ptr, ptr_name_str) WGPU_VALIDATE_IMPL(device_ptr, (ptr) != NULL, WGPUErrorType_Validation, ptr_name_str " must not be NULL.", LOG_ERROR)
#define WGPU_VALIDATE_HANDLE(device_ptr, handle, handle_name_str) WGPU_VALIDATE_IMPL(device_ptr, (handle) != VK_NULL_HANDLE, WGPUErrorType_Validation, handle_name_str " is VK_NULL_HANDLE.", LOG_ERROR)

// Specific validation for descriptor structs
#define WGPU_VALIDATE_DESC_PTR(parent_device_ptr, desc_ptr, desc_name_str) \
    do { \
        if (!(desc_ptr)) { \
            const char* msg = desc_name_str " descriptor must not be NULL."; \
            WGPU_VALIDATE_IMPL(parent_device_ptr, false, WGPUErrorType_Validation, msg, LOG_ERROR); \
        } \
    } while (0)


// --- NEW MACROS FOR EQUALITY/INEQUALITY ---

/**
 * @brief Validates that two expressions `a` and `b` are equal.
 * Generates a detailed message on failure, including the stringified expressions and their runtime values.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first expression.
 * @param b The second expression.
 * @param fmt_a The printf-style format specifier for the type of 'a'.
 * @param fmt_b The printf-style format specifier for the type of 'b'.
 * @param context_msg A string providing context for the check (e.g., "sCreateInfo->sType").
 */
#define WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, fmt_a, fmt_b, context_msg) \
    do { \
        /* Evaluate a and b once to avoid side effects if they are complex expressions */ \
        /* Note: This requires C99 or for the types of a_val and b_val to be known, */ \
        /* or rely on type inference if using C++ or newer C standards with typeof. */ \
        /* For maximum C89/C90 compatibility, one might pass a and b directly, accepting multiple evaluations. */ \
        /* Given the context of Vulkan, C99+ is a safe assumption. */ \
        /* We will use __auto_type if available (GCC/Clang extension), otherwise user must be careful. */ \
        /* Or, simply evaluate (a) and (b) directly in the if and snprintf. */ \
        if (!((a) == (b))) { \
            char __wgpu_msg_buffer[512]; \
            snprintf(__wgpu_msg_buffer, sizeof(__wgpu_msg_buffer), \
                     "%s %s: Equality check '%s == %s' failed. LHS (" #a " = " fmt_a ") != RHS (" #b " = " fmt_b ").", \
                     __func__, (context_msg), #a, #b, (a), (b)); \
            WGPU_VALIDATE_IMPL(device_ptr, false, WGPUErrorType_Validation, __wgpu_msg_buffer, LOG_ERROR); \
        } \
    } while (0)


#define WGPU_VALIDATE_NEQ_FORMAT(device_ptr, a, b, fmt_a, fmt_b, context_msg) \
    do { \
        /* Evaluate a and b once to avoid side effects if they are complex expressions */ \
        /* Note: This requires C99 or for the types of a_val and b_val to be known, */ \
        /* or rely on type inference if using C++ or newer C standards with typeof. */ \
        /* For maximum C89/C90 compatibility, one might pass a and b directly, accepting multiple evaluations. */ \
        /* Given the context of Vulkan, C99+ is a safe assumption. */ \
        /* We will use __auto_type if available (GCC/Clang extension), otherwise user must be careful. */ \
        /* Or, simply evaluate (a) and (b) directly in the if and snprintf. */ \
        if (!((a) != (b))) { \
            char __wgpu_msg_buffer[512]; \
            snprintf(__wgpu_msg_buffer, sizeof(__wgpu_msg_buffer), \
                     "%s %s: Non equality check '%s != %s' failed. LHS (" #a " = " fmt_a ") != RHS (" #b " = " fmt_b ").", \
                     __func__, (context_msg), #a, #b, (a), (b)); \
            WGPU_VALIDATE_IMPL(device_ptr, false, WGPUErrorType_Validation, __wgpu_msg_buffer, LOG_ERROR); \
        } \
    } while (0)
/**
 * @brief Validates that two integer expressions `a` and `b` are equal.
 * Provides a default format specifier for integers.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first integer expression.
 * @param b The second integer expression.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_EQ_INT(device_ptr, a, b, context_msg) \
    WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, "%d", "%d", context_msg)

/**
 * @brief Validates that two unsigned integer expressions `a` and `b` are equal.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first unsigned integer expression.
 * @param b The second unsigned integer expression.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_EQ_UINT(device_ptr, a, b, context_msg) \
    WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, "%u", "%u", context_msg)

/**
 * @brief Validates that two pointer expressions `a` and `b` are equal.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first pointer expression.
 * @param b The second pointer expression.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_EQ_PTR(device_ptr, a, b, context_msg) \
    WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, "%p", "%p", context_msg)


#define WGPU_VALIDATE_NEQ_PTR(device_ptr, a, b, context_msg) \
    WGPU_VALIDATE_NEQ_FORMAT(device_ptr, a, b, "%p", "%p", context_msg)
/**
 * @brief Validates that two VkBool32 expressions `a` and `b` are equal.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first VkBool32 expression.
 * @param b The second VkBool32 expression.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_EQ_BOOL32(device_ptr, a, b, context_msg) \
    WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, "%u (BOOL32)", "%u (BOOL32)", context_msg)


/**
 * @brief Validates that two expressions `a` and `b` are NOT equal.
 * Generates a detailed message on failure.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first expression.
 * @param b The second expression.
 * @param fmt_a The printf-style format specifier for the type of 'a'.
 * @param fmt_b The printf-style format specifier for the type of 'b'.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_NE_FORMAT(device_ptr, a, b, fmt_a, fmt_b, context_msg) \
    do { \
        if (!((a) != (b))) { \
            char __wgpu_msg_buffer[512]; \
            snprintf(__wgpu_msg_buffer, sizeof(__wgpu_msg_buffer), \
                     "%s: Inequality check '%s != %s' failed. LHS (" #a " = " fmt_a ") == RHS (" #b " = " fmt_b ").", \
                     (context_msg), #a, #b, (a), (b)); \
            WGPU_VALIDATE_IMPL(device_ptr, false, WGPUErrorType_Validation, __wgpu_msg_buffer, LOG_ERROR); \
        } \
    } while (0)

/**
 * @brief Validates that two C-style strings `a` and `b` are equal using strcmp.
 * Assumes `a` and `b` are non-NULL (use WGPU_VALIDATE_PTR first if needed).
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first C-string.
 * @param b The second C-string.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_STREQ(device_ptr, a, b, context_msg) \
    do { \
        /* Ensure strings are not NULL before strcmp, or ensure this is handled by caller */ \
        /* WGPU_VALIDATE_PTR(device_ptr, (a), #a " for STREQ"); */ \
        /* WGPU_VALIDATE_PTR(device_ptr, (b), #b " for STREQ"); */ \
        if (strcmp((a), (b)) != 0) { \
            char __wgpu_msg_buffer[1024]; /* Potentially longer for strings */ \
            snprintf(__wgpu_msg_buffer, sizeof(__wgpu_msg_buffer), \
                     "%s: String equality check 'strcmp(%s, %s) == 0' failed. LHS (\"%s\") != RHS (\"%s\").", \
                     (context_msg), #a, #b, (a), (b)); \
            WGPU_VALIDATE_IMPL(device_ptr, false, WGPUErrorType_Validation, __wgpu_msg_buffer, LOG_ERROR); \
        } \
    } while (0)


#define WGPU_VALIDATION_ERROR_MESSAGE(message ...) \
    do {  \
        char vmessageBuffer[8192] = {0}; \
        snprintf(vmessageBuffer, 8192, message); \
        TRACELOG(LOG_ERROR, "Validation error: %s"); \
    } while (0)


/**
 * @brief Validates that two C-style strings `a` and `b` are NOT equal using strcmp.
 * Assumes `a` and `b` are non-NULL.
 * @param device_ptr Pointer to the WGPUDevice (can be NULL).
 * @param a The first C-string.
 * @param b The second C-string.
 * @param context_msg A string providing context for the check.
 */
#define WGPU_VALIDATE_STRNEQ(device_ptr, a, b, context_msg) \
    do { \
        if (strcmp((a), (b)) == 0) { \
            char __wgpu_msg_buffer[1024]; \
            snprintf(__wgpu_msg_buffer, sizeof(__wgpu_msg_buffer), \
                     "%s: String inequality check 'strcmp(%s, %s) != 0' failed. LHS (\"%s\") == RHS (\"%s\").", \
                     (context_msg), #a, #b, (a), (b)); \
            WGPU_VALIDATE_IMPL(device_ptr, false, WGPUErrorType_Validation, __wgpu_msg_buffer, LOG_ERROR); \
        } \
    } while (0)

    


#else // WGPU_VALIDATION_ENABLED not defined
// WGPU_VALIDATE_IMPL needs a dummy definition for the other macros to compile to ((void)0)
#define WGPU_VALIDATE_IMPL(device_ptr, condition, error_type, message_str, log_level_on_fail) ((void)0)

#define WGPU_VALIDATE(device_ptr, condition, message_str) ((void)0)
#define WGPU_VALIDATE_PTR(device_ptr, ptr, ptr_name_str) ((void)0)
#define WGPU_VALIDATE_HANDLE(device_ptr, handle, handle_name_str) ((void)0)
#define WGPU_VALIDATE_DESC_PTR(parent_device_ptr, desc_ptr, desc_name_str) ((void)0)

#define WGPU_VALIDATE_EQ_FORMAT(device_ptr, a, b, fmt_a, fmt_b, context_msg) ((void)0)
#define WGPU_VALIDATE_EQ_INT(device_ptr, a, b, context_msg) ((void)0)
#define WGPU_VALIDATE_EQ_UINT(device_ptr, a, b, context_msg) ((void)0)
#define WGPU_VALIDATE_EQ_BOOL32(device_ptr, a, b, context_msg) ((void)0)
#define WGPU_VALIDATE_NE_FORMAT(device_ptr, a, b, fmt_a, fmt_b, context_msg) ((void)0)
#define WGPU_VALIDATE_STREQ(device_ptr, a, b, context_msg) ((void)0)
#define WGPU_VALIDATE_STRNEQ(device_ptr, a, b, context_msg) ((void)0)

#endif // WGPU_VALIDATION_ENABLED


static inline VkImageUsageFlags toVulkanTextureUsage(WGPUTextureUsage usage, WGPUTextureFormat format) {
    VkImageUsageFlags vkUsage = 0;

    if (usage & WGPUTextureUsage_CopySrc) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage & WGPUTextureUsage_CopyDst) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    if (usage & WGPUTextureUsage_TextureBinding) {
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (usage & WGPUTextureUsage_StorageBinding) {
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (usage & WGPUTextureUsage_RenderAttachment) {
        if (format == WGPUTextureFormat_Depth24Plus || format == WGPUTextureFormat_Depth32Float) { // Assuming Depth24 and Depth32 are defined enum/macro values
            vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
    }
    if (usage & WGPUTextureUsage_TransientAttachment) {
        vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }
    if (usage & WGPUTextureUsage_StorageAttachment) { // Note: Original code mapped this to VK_IMAGE_USAGE_STORAGE_BIT
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    // Any WGPUTextureUsage flags not explicitly handled here will be ignored.

    return vkUsage;
}

static inline VkImageAspectFlags toVulkanAspectMask(WGPUTextureAspect aspect){
    
    switch(aspect){
        case TextureAspect_All:
        return VK_IMAGE_ASPECT_COLOR_BIT;// | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case TextureAspect_DepthOnly:
        return VK_IMAGE_ASPECT_DEPTH_BIT;
        case TextureAspect_StencilOnly:
        return VK_IMAGE_ASPECT_STENCIL_BIT;

        default: {
            assert(false && "This aspect is not implemented");
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
}
// Inverse conversion: Vulkan usage flags -> WGPUTextureUsage flags
static inline WGPUTextureUsage fromVulkanWGPUTextureUsage(VkImageUsageFlags vkUsage) {
    WGPUTextureUsage usage = 0;
    
    // Map transfer bits
    if (vkUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        usage |= WGPUTextureUsage_CopySrc;
    if (vkUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        usage |= WGPUTextureUsage_CopyDst;
    
    // Map sampling bit
    if (vkUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
        usage |= WGPUTextureUsage_TextureBinding;
    
    // Map storage bit (ambiguous: could originate from either storage flag)
    if (vkUsage & VK_IMAGE_USAGE_STORAGE_BIT)
        usage |= WGPUTextureUsage_StorageBinding | WGPUTextureUsage_StorageAttachment;
    
    // Map render attachment bits (depth/stencil or color, both yield RenderAttachment)
    if (vkUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        usage |= WGPUTextureUsage_RenderAttachment;
    if (vkUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        usage |= WGPUTextureUsage_RenderAttachment;
    
    // Map transient attachment
    if (vkUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
        usage |= WGPUTextureUsage_TransientAttachment;
    
    return usage;
}




static inline VkBufferUsageFlags toVulkanBufferUsage(WGPUBufferUsage busg) {
    VkBufferUsageFlags usage = 0;

    // Input: WGPUBufferUsageFlags busg

    if (busg & WGPUBufferUsage_CopySrc) {
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (busg & WGPUBufferUsage_CopyDst) {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (busg & WGPUBufferUsage_Vertex) {
        usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (busg & WGPUBufferUsage_Index) {
        usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (busg & WGPUBufferUsage_Uniform) {
        usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (busg & WGPUBufferUsage_Storage) {
        usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    if (busg & WGPUBufferUsage_Indirect) {
        usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }
    if (busg & WGPUBufferUsage_MapRead) {
        usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    if (busg & WGPUBufferUsage_MapWrite) {
        usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (busg & WGPUBufferUsage_ShaderDeviceAddress) {
        usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (busg & WGPUBufferUsage_AccelerationStructureInput) {
        usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if (busg & WGPUBufferUsage_AccelerationStructureStorage) {
        usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }
    if (busg & WGPUBufferUsage_ShaderBindingTable) {
        usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }


    return usage;
}
static inline VkImageViewType toVulkanTextureViewDimension(WGPUTextureViewDimension dim){
    VkImageViewCreateInfo info;
    switch(dim){
        default:
        case 0:{
            rg_unreachable();
        }
        case WGPUTextureViewDimension_1D:{
            return VK_IMAGE_VIEW_TYPE_1D;
        }
        case WGPUTextureViewDimension_2D:{
            return VK_IMAGE_VIEW_TYPE_2D;
        }
        case WGPUTextureViewDimension_3D:{
            return VK_IMAGE_VIEW_TYPE_3D;
        }
        case WGPUTextureViewDimension_2DArray:{
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

    }
}
static inline VkImageType toVulkanTextureDimension(WGPUTextureDimension dim){
    VkImageViewCreateInfo info;
    switch(dim){
        default:
        case 0:{
            rg_unreachable();
        }
        case TextureDimension_1D:{
            return VK_IMAGE_TYPE_1D;
        }
        case TextureDimension_2D:{
            return VK_IMAGE_TYPE_2D;
        }
        case TextureDimension_3D:{
            return VK_IMAGE_TYPE_3D;
        }
    }
}

// Converts WGPUTextureFormat to VkFormat
static inline VkFormat toVulkanPixelFormat(WGPUTextureFormat format) {
    switch (format) {
        case WGPUTextureFormat_Undefined:            return VK_FORMAT_UNDEFINED;
        case WGPUTextureFormat_R8Unorm:              return VK_FORMAT_R8_UNORM;
        case WGPUTextureFormat_R8Snorm:              return VK_FORMAT_R8_SNORM;
        case WGPUTextureFormat_R8Uint:               return VK_FORMAT_R8_UINT;
        case WGPUTextureFormat_R8Sint:               return VK_FORMAT_R8_SINT;
        case WGPUTextureFormat_R16Uint:              return VK_FORMAT_R16_UINT;
        case WGPUTextureFormat_R16Sint:              return VK_FORMAT_R16_SINT;
        case WGPUTextureFormat_R16Float:             return VK_FORMAT_R16_SFLOAT;
        case WGPUTextureFormat_RG8Unorm:             return VK_FORMAT_R8G8_UNORM;
        case WGPUTextureFormat_RG8Snorm:             return VK_FORMAT_R8G8_SNORM;
        case WGPUTextureFormat_RG8Uint:              return VK_FORMAT_R8G8_UINT;
        case WGPUTextureFormat_RG8Sint:              return VK_FORMAT_R8G8_SINT;
        case WGPUTextureFormat_R32Float:             return VK_FORMAT_R32_SFLOAT;
        case WGPUTextureFormat_R32Uint:              return VK_FORMAT_R32_UINT;
        case WGPUTextureFormat_R32Sint:              return VK_FORMAT_R32_SINT;
        case WGPUTextureFormat_RG16Uint:             return VK_FORMAT_R16G16_UINT;
        case WGPUTextureFormat_RG16Sint:             return VK_FORMAT_R16G16_SINT;
        case WGPUTextureFormat_RG16Float:            return VK_FORMAT_R16G16_SFLOAT;
        case WGPUTextureFormat_RGBA8Unorm:           return VK_FORMAT_R8G8B8A8_UNORM;
        case WGPUTextureFormat_RGBA8UnormSrgb:       return VK_FORMAT_R8G8B8A8_SRGB;
        case WGPUTextureFormat_RGBA8Snorm:           return VK_FORMAT_R8G8B8A8_SNORM;
        case WGPUTextureFormat_RGBA8Uint:            return VK_FORMAT_R8G8B8A8_UINT;
        case WGPUTextureFormat_RGBA8Sint:            return VK_FORMAT_R8G8B8A8_SINT;
        case WGPUTextureFormat_BGRA8Unorm:           return VK_FORMAT_B8G8R8A8_UNORM;
        case WGPUTextureFormat_BGRA8UnormSrgb:       return VK_FORMAT_B8G8R8A8_SRGB;
        case WGPUTextureFormat_RGB10A2Uint:          return VK_FORMAT_A2R10G10B10_UINT_PACK32; // Or A2B10G10R10_UINT_PACK32, check WGPU spec
        case WGPUTextureFormat_RGB10A2Unorm:         return VK_FORMAT_A2R10G10B10_UNORM_PACK32; // Or A2B10G10R10_UNORM_PACK32
        case WGPUTextureFormat_RG11B10Ufloat:        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case WGPUTextureFormat_RGB9E5Ufloat:         return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case WGPUTextureFormat_RG32Float:            return VK_FORMAT_R32G32_SFLOAT;
        case WGPUTextureFormat_RG32Uint:             return VK_FORMAT_R32G32_UINT;
        case WGPUTextureFormat_RG32Sint:             return VK_FORMAT_R32G32_SINT;
        case WGPUTextureFormat_RGBA16Uint:           return VK_FORMAT_R16G16B16A16_UINT;
        case WGPUTextureFormat_RGBA16Sint:           return VK_FORMAT_R16G16B16A16_SINT;
        case WGPUTextureFormat_RGBA16Float:          return VK_FORMAT_R16G16B16A16_SFLOAT;
        case WGPUTextureFormat_RGBA32Float:          return VK_FORMAT_R32G32B32A32_SFLOAT;
        case WGPUTextureFormat_RGBA32Uint:           return VK_FORMAT_R32G32B32A32_UINT;
        case WGPUTextureFormat_RGBA32Sint:           return VK_FORMAT_R32G32B32A32_SINT;
        case WGPUTextureFormat_Stencil8:             return VK_FORMAT_S8_UINT;
        case WGPUTextureFormat_Depth16Unorm:         return VK_FORMAT_D16_UNORM;
        case WGPUTextureFormat_Depth24Plus:          return VK_FORMAT_X8_D24_UNORM_PACK32; // Or D24_UNORM_S8_UINT if stencil is guaranteed
        case WGPUTextureFormat_Depth24PlusStencil8:  return VK_FORMAT_D24_UNORM_S8_UINT;
        case WGPUTextureFormat_Depth32Float:         return VK_FORMAT_D32_SFLOAT;
        case WGPUTextureFormat_Depth32FloatStencil8: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case WGPUTextureFormat_BC1RGBAUnorm:         return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case WGPUTextureFormat_BC1RGBAUnormSrgb:     return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case WGPUTextureFormat_BC2RGBAUnorm:         return VK_FORMAT_BC2_UNORM_BLOCK;
        case WGPUTextureFormat_BC2RGBAUnormSrgb:     return VK_FORMAT_BC2_SRGB_BLOCK;
        case WGPUTextureFormat_BC3RGBAUnorm:         return VK_FORMAT_BC3_UNORM_BLOCK;
        case WGPUTextureFormat_BC3RGBAUnormSrgb:     return VK_FORMAT_BC3_SRGB_BLOCK;
        case WGPUTextureFormat_BC4RUnorm:            return VK_FORMAT_BC4_UNORM_BLOCK;
        case WGPUTextureFormat_BC4RSnorm:            return VK_FORMAT_BC4_SNORM_BLOCK;
        case WGPUTextureFormat_BC5RGUnorm:           return VK_FORMAT_BC5_UNORM_BLOCK;
        case WGPUTextureFormat_BC5RGSnorm:           return VK_FORMAT_BC5_SNORM_BLOCK;
        case WGPUTextureFormat_BC6HRGBUfloat:        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case WGPUTextureFormat_BC6HRGBFloat:         return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case WGPUTextureFormat_BC7RGBAUnorm:         return VK_FORMAT_BC7_UNORM_BLOCK;
        case WGPUTextureFormat_BC7RGBAUnormSrgb:     return VK_FORMAT_BC7_SRGB_BLOCK;
        case WGPUTextureFormat_ETC2RGB8Unorm:        return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case WGPUTextureFormat_ETC2RGB8UnormSrgb:    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case WGPUTextureFormat_ETC2RGB8A1Unorm:      return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:  return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case WGPUTextureFormat_ETC2RGBA8Unorm:       return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case WGPUTextureFormat_ETC2RGBA8UnormSrgb:   return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case WGPUTextureFormat_EACR11Unorm:          return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case WGPUTextureFormat_EACR11Snorm:          return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case WGPUTextureFormat_EACRG11Unorm:         return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case WGPUTextureFormat_EACRG11Snorm:         return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
        case WGPUTextureFormat_ASTC4x4Unorm:         return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC4x4UnormSrgb:     return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC5x4Unorm:         return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC5x4UnormSrgb:     return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC5x5Unorm:         return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC5x5UnormSrgb:     return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC6x5Unorm:         return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC6x5UnormSrgb:     return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC6x6Unorm:         return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC6x6UnormSrgb:     return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC8x5Unorm:         return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC8x5UnormSrgb:     return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC8x6Unorm:         return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC8x6UnormSrgb:     return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC8x8Unorm:         return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC8x8UnormSrgb:     return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC10x5Unorm:        return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC10x5UnormSrgb:    return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC10x6Unorm:        return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC10x6UnormSrgb:    return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC10x8Unorm:        return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC10x8UnormSrgb:    return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC10x10Unorm:       return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC10x10UnormSrgb:   return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC12x10Unorm:       return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC12x10UnormSrgb:   return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case WGPUTextureFormat_ASTC12x12Unorm:       return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case WGPUTextureFormat_ASTC12x12UnormSrgb:   return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
        // WGPUTextureFormat_Force32 is a utility, not a real format.
        default:                                     return VK_FORMAT_UNDEFINED;
    }
}

static inline PixelFormat fromWGPUPixelFormat(WGPUTextureFormat format) {
    switch (format) {
        case WGPUTextureFormat_RGBA8Unorm:      return RGBA8;
        case WGPUTextureFormat_RGBA8UnormSrgb:  return RGBA8_Srgb;
        case WGPUTextureFormat_BGRA8Unorm:      return BGRA8;
        case WGPUTextureFormat_BGRA8UnormSrgb:  return BGRA8_Srgb;
        case WGPUTextureFormat_RGBA16Float:     return RGBA16F;
        case WGPUTextureFormat_RGBA32Float:     return RGBA32F;
        case WGPUTextureFormat_Depth24Plus:     return Depth24;
        case WGPUTextureFormat_Depth32Float:    return Depth32;
        default:
            rg_unreachable();
    }
    return (PixelFormat)(-1); // Unreachable but silences compiler warnings
}
static inline VkSamplerAddressMode toVulkanAddressMode(WGPUAddressMode mode){
    switch(mode){
        case WGPUAddressMode_ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case WGPUAddressMode_Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case WGPUAddressMode_MirrorRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case WGPUAddressMode_Undefined:
        case WGPUAddressMode_Force32:
        rg_unreachable();
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; 
    }
}

static inline WGPUTextureFormat fromVulkanPixelFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED: return WGPUTextureFormat_Undefined;
        case VK_FORMAT_R8_UNORM: return WGPUTextureFormat_R8Unorm;
        case VK_FORMAT_R8_SNORM: return WGPUTextureFormat_R8Snorm;
        case VK_FORMAT_R8_UINT: return WGPUTextureFormat_R8Uint;
        case VK_FORMAT_R8_SINT: return WGPUTextureFormat_R8Sint;
        case VK_FORMAT_R16_UINT: return WGPUTextureFormat_R16Uint;
        case VK_FORMAT_R16_SINT: return WGPUTextureFormat_R16Sint;
        case VK_FORMAT_R16_SFLOAT: return WGPUTextureFormat_R16Float;
        case VK_FORMAT_R8G8_UNORM: return WGPUTextureFormat_RG8Unorm;
        case VK_FORMAT_R8G8_SNORM: return WGPUTextureFormat_RG8Snorm;
        case VK_FORMAT_R8G8_UINT: return WGPUTextureFormat_RG8Uint;
        case VK_FORMAT_R8G8_SINT: return WGPUTextureFormat_RG8Sint;
        case VK_FORMAT_R32_SFLOAT: return WGPUTextureFormat_R32Float;
        case VK_FORMAT_R32_UINT: return WGPUTextureFormat_R32Uint;
        case VK_FORMAT_R32_SINT: return WGPUTextureFormat_R32Sint;
        case VK_FORMAT_R16G16_UINT: return WGPUTextureFormat_RG16Uint;
        case VK_FORMAT_R16G16_SINT: return WGPUTextureFormat_RG16Sint;
        case VK_FORMAT_R16G16_SFLOAT: return WGPUTextureFormat_RG16Float;
        case VK_FORMAT_R8G8B8A8_UNORM: return WGPUTextureFormat_RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SRGB: return WGPUTextureFormat_RGBA8UnormSrgb;
        case VK_FORMAT_R8G8B8A8_SNORM: return WGPUTextureFormat_RGBA8Snorm;
        case VK_FORMAT_R8G8B8A8_UINT: return WGPUTextureFormat_RGBA8Uint;
        case VK_FORMAT_R8G8B8A8_SINT: return WGPUTextureFormat_RGBA8Sint;
        case VK_FORMAT_B8G8R8A8_UNORM: return WGPUTextureFormat_BGRA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB: return WGPUTextureFormat_BGRA8UnormSrgb;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32: return WGPUTextureFormat_RGB10A2Uint;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return WGPUTextureFormat_RGB10A2Unorm;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return WGPUTextureFormat_RG11B10Ufloat;
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return WGPUTextureFormat_RGB9E5Ufloat;
        case VK_FORMAT_R32G32_SFLOAT: return WGPUTextureFormat_RG32Float;
        case VK_FORMAT_R32G32_UINT: return WGPUTextureFormat_RG32Uint;
        case VK_FORMAT_R32G32_SINT: return WGPUTextureFormat_RG32Sint;
        case VK_FORMAT_R16G16B16A16_UINT: return WGPUTextureFormat_RGBA16Uint;
        case VK_FORMAT_R16G16B16A16_SINT: return WGPUTextureFormat_RGBA16Sint;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return WGPUTextureFormat_RGBA16Float;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return WGPUTextureFormat_RGBA32Float;
        case VK_FORMAT_R32G32B32A32_UINT: return WGPUTextureFormat_RGBA32Uint;
        case VK_FORMAT_R32G32B32A32_SINT: return WGPUTextureFormat_RGBA32Sint;
        case VK_FORMAT_S8_UINT: return WGPUTextureFormat_Stencil8;
        case VK_FORMAT_D16_UNORM: return WGPUTextureFormat_Depth16Unorm;
        case VK_FORMAT_X8_D24_UNORM_PACK32: return WGPUTextureFormat_Depth24Plus;
        case VK_FORMAT_D24_UNORM_S8_UINT: return WGPUTextureFormat_Depth24PlusStencil8;
        case VK_FORMAT_D32_SFLOAT: return WGPUTextureFormat_Depth32Float;
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return WGPUTextureFormat_Depth32FloatStencil8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return WGPUTextureFormat_BC1RGBAUnorm;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return WGPUTextureFormat_BC1RGBAUnormSrgb;
        case VK_FORMAT_BC2_UNORM_BLOCK: return WGPUTextureFormat_BC2RGBAUnorm;
        case VK_FORMAT_BC2_SRGB_BLOCK: return WGPUTextureFormat_BC2RGBAUnormSrgb;
        case VK_FORMAT_BC3_UNORM_BLOCK: return WGPUTextureFormat_BC3RGBAUnorm;
        case VK_FORMAT_BC3_SRGB_BLOCK: return WGPUTextureFormat_BC3RGBAUnormSrgb;
        case VK_FORMAT_BC4_UNORM_BLOCK: return WGPUTextureFormat_BC4RUnorm;
        case VK_FORMAT_BC4_SNORM_BLOCK: return WGPUTextureFormat_BC4RSnorm;
        case VK_FORMAT_BC5_UNORM_BLOCK: return WGPUTextureFormat_BC5RGUnorm;
        case VK_FORMAT_BC5_SNORM_BLOCK: return WGPUTextureFormat_BC5RGSnorm;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK: return WGPUTextureFormat_BC6HRGBUfloat;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK: return WGPUTextureFormat_BC6HRGBFloat;
        case VK_FORMAT_BC7_UNORM_BLOCK: return WGPUTextureFormat_BC7RGBAUnorm;
        case VK_FORMAT_BC7_SRGB_BLOCK: return WGPUTextureFormat_BC7RGBAUnormSrgb;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return WGPUTextureFormat_ETC2RGB8Unorm;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return WGPUTextureFormat_ETC2RGB8UnormSrgb;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return WGPUTextureFormat_ETC2RGB8A1Unorm;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return WGPUTextureFormat_ETC2RGB8A1UnormSrgb;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return WGPUTextureFormat_ETC2RGBA8Unorm;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return WGPUTextureFormat_ETC2RGBA8UnormSrgb;
        case VK_FORMAT_EAC_R11_UNORM_BLOCK: return WGPUTextureFormat_EACR11Unorm;
        case VK_FORMAT_EAC_R11_SNORM_BLOCK: return WGPUTextureFormat_EACR11Snorm;
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return WGPUTextureFormat_EACRG11Unorm;
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return WGPUTextureFormat_EACRG11Snorm;
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return WGPUTextureFormat_ASTC4x4Unorm;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return WGPUTextureFormat_ASTC4x4UnormSrgb;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return WGPUTextureFormat_ASTC5x4Unorm;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return WGPUTextureFormat_ASTC5x4UnormSrgb;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return WGPUTextureFormat_ASTC5x5Unorm;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return WGPUTextureFormat_ASTC5x5UnormSrgb;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return WGPUTextureFormat_ASTC6x5Unorm;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return WGPUTextureFormat_ASTC6x5UnormSrgb;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return WGPUTextureFormat_ASTC6x6Unorm;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return WGPUTextureFormat_ASTC6x6UnormSrgb;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return WGPUTextureFormat_ASTC8x5Unorm;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return WGPUTextureFormat_ASTC8x5UnormSrgb;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return WGPUTextureFormat_ASTC8x6Unorm;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return WGPUTextureFormat_ASTC8x6UnormSrgb;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return WGPUTextureFormat_ASTC8x8Unorm;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return WGPUTextureFormat_ASTC8x8UnormSrgb;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return WGPUTextureFormat_ASTC10x5Unorm;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return WGPUTextureFormat_ASTC10x5UnormSrgb;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return WGPUTextureFormat_ASTC10x6Unorm;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return WGPUTextureFormat_ASTC10x6UnormSrgb;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return WGPUTextureFormat_ASTC10x8Unorm;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return WGPUTextureFormat_ASTC10x8UnormSrgb;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return WGPUTextureFormat_ASTC10x10Unorm;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return WGPUTextureFormat_ASTC10x10UnormSrgb;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return WGPUTextureFormat_ASTC12x10Unorm;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return WGPUTextureFormat_ASTC12x10UnormSrgb;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return WGPUTextureFormat_ASTC12x12Unorm;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return WGPUTextureFormat_ASTC12x12UnormSrgb;
        default:                                     return WGPUTextureFormat_Undefined;
    }
}


static inline VkAttachmentStoreOp toVulkanStoreOperation(WGPUStoreOp lop) {
    switch (lop) {
    case StoreOp_Store:
        return VK_ATTACHMENT_STORE_OP_STORE;
    case StoreOp_Discard:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    case StoreOp_Undefined:
    
        return VK_ATTACHMENT_STORE_OP_DONT_CARE; // Example fallback
    default:
        return VK_ATTACHMENT_STORE_OP_DONT_CARE; // Default fallback
    }
}
static inline VkAttachmentLoadOp toVulkanLoadOperation(WGPULoadOp lop) {
    switch (lop) {
    case LoadOp_Load:
        return VK_ATTACHMENT_LOAD_OP_LOAD;
    case LoadOp_Clear:
        return VK_ATTACHMENT_LOAD_OP_CLEAR;
    case LoadOp_ExpandResolveTexture:
        // Vulkan does not have a direct equivalent; choose appropriate op or handle separately
        return VK_ATTACHMENT_LOAD_OP_LOAD; // Example fallback
    default:
        return VK_ATTACHMENT_LOAD_OP_LOAD; // Default fallback
    }
}
static inline WGPUPresentMode fromVulkanPresentMode(VkPresentModeKHR mode){
    switch(mode){
        case VK_PRESENT_MODE_FIFO_KHR:
            return PresentMode_Fifo;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
            return PresentMode_FifoRelaxed;
        case VK_PRESENT_MODE_IMMEDIATE_KHR:
            return PresentMode_Immediate;
        case VK_PRESENT_MODE_MAILBOX_KHR:
            return PresentMode_Mailbox;
        default:
            return (WGPUPresentMode)~0;
    }
}

static inline VkPresentModeKHR toVulkanPresentMode(WGPUPresentMode mode){
    switch(mode){
        case PresentMode_Fifo:
            return VK_PRESENT_MODE_FIFO_KHR;
        case PresentMode_FifoRelaxed:
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        case PresentMode_Immediate:
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        case PresentMode_Mailbox:
            return VK_PRESENT_MODE_MAILBOX_KHR;
        default:
            rg_trap();
    }
    return (VkPresentModeKHR)~0;
}

static inline VkCompareOp toVulkanCompareFunction(WGPUCompareFunction cf) {
    switch (cf) {
    case WGPUCompareFunction_Never:
        return VK_COMPARE_OP_NEVER;
    case WGPUCompareFunction_Less:
        return VK_COMPARE_OP_LESS;
    case WGPUCompareFunction_Equal:
        return VK_COMPARE_OP_EQUAL;
    case WGPUCompareFunction_LessEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case WGPUCompareFunction_Greater:
        return VK_COMPARE_OP_GREATER;
    case WGPUCompareFunction_NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case WGPUCompareFunction_GreaterEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case WGPUCompareFunction_Always:
        return VK_COMPARE_OP_ALWAYS;
    default:
        rg_unreachable();
        return VK_COMPARE_OP_ALWAYS; // Default fallback
    }
}

static inline VkStencilOp toVulkanStencilOperation(WGPUStencilOperation op){
    switch(op){
        case WGPUStencilOperation_Keep: return VK_STENCIL_OP_KEEP;
        case WGPUStencilOperation_Zero: return VK_STENCIL_OP_ZERO;
        case WGPUStencilOperation_Replace: return VK_STENCIL_OP_REPLACE;
        case WGPUStencilOperation_Invert: return VK_STENCIL_OP_INVERT;
        case WGPUStencilOperation_IncrementClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case WGPUStencilOperation_DecrementClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case WGPUStencilOperation_IncrementWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case WGPUStencilOperation_DecrementWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default: rg_unreachable();
    }
}
static inline VkCullModeFlags toVulkanCullMode(WGPUCullMode cm){
    switch(cm){
        case WGPUCullMode_Back: return VK_CULL_MODE_BACK_BIT;
        case WGPUCullMode_Front: return VK_CULL_MODE_FRONT_BIT;
        case WGPUCullMode_None: return 0;
        default: rg_unreachable();
    }
}

static inline VkPrimitiveTopology toVulkanPrimitive(PrimitiveType type){
    switch(type){
        case RL_TRIANGLE_STRIP: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case RL_TRIANGLES: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case RL_LINES: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case RL_POINTS: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case RL_QUADS:
            //rassert(false, "Quads are not a primitive type");
        default:
            rg_unreachable();
    }
}
static inline VkPrimitiveTopology toVulkanPrimitiveTopology(WGPUPrimitiveTopology type){
    switch(type){
        case WGPUPrimitiveTopology_PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case WGPUPrimitiveTopology_LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case WGPUPrimitiveTopology_LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case WGPUPrimitiveTopology_TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case WGPUPrimitiveTopology_TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case WGPUPrimitiveTopology_Undefined:
        case WGPUPrimitiveTopology_Force32:
        rg_unreachable();
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
}

static inline VkFrontFace toVulkanFrontFace(WGPUFrontFace ff) {
    switch (ff) {
    case WGPUFrontFace_CCW:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case WGPUFrontFace_CW:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE; // Default fallback
    }
}

static inline VkFormat toVulkanVertexFormat(WGPUVertexFormat vf) {
    switch (vf) {
    case WGPUVertexFormat_Uint8:
        return VK_FORMAT_R8_UINT;
    case WGPUVertexFormat_Uint8x2:
        return VK_FORMAT_R8G8_UINT;
    case WGPUVertexFormat_Uint8x4:
        return VK_FORMAT_R8G8B8A8_UINT;
    case WGPUVertexFormat_Sint8:
        return VK_FORMAT_R8_SINT;
    case WGPUVertexFormat_Sint8x2:
        return VK_FORMAT_R8G8_SINT;
    case WGPUVertexFormat_Sint8x4:
        return VK_FORMAT_R8G8B8A8_SINT;
    case WGPUVertexFormat_Unorm8:
        return VK_FORMAT_R8_UNORM;
    case WGPUVertexFormat_Unorm8x2:
        return VK_FORMAT_R8G8_UNORM;
    case WGPUVertexFormat_Unorm8x4:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case WGPUVertexFormat_Snorm8:
        return VK_FORMAT_R8_SNORM;
    case WGPUVertexFormat_Snorm8x2:
        return VK_FORMAT_R8G8_SNORM;
    case WGPUVertexFormat_Snorm8x4:
        return VK_FORMAT_R8G8B8A8_SNORM;
    case WGPUVertexFormat_Uint16:
        return VK_FORMAT_R16_UINT;
    case WGPUVertexFormat_Uint16x2:
        return VK_FORMAT_R16G16_UINT;
    case WGPUVertexFormat_Uint16x4:
        return VK_FORMAT_R16G16B16A16_UINT;
    case WGPUVertexFormat_Sint16:
        return VK_FORMAT_R16_SINT;
    case WGPUVertexFormat_Sint16x2:
        return VK_FORMAT_R16G16_SINT;
    case WGPUVertexFormat_Sint16x4:
        return VK_FORMAT_R16G16B16A16_SINT;
    case WGPUVertexFormat_Unorm16:
        return VK_FORMAT_R16_UNORM;
    case WGPUVertexFormat_Unorm16x2:
        return VK_FORMAT_R16G16_UNORM;
    case WGPUVertexFormat_Unorm16x4:
        return VK_FORMAT_R16G16B16A16_UNORM;
    case WGPUVertexFormat_Snorm16:
        return VK_FORMAT_R16_SNORM;
    case WGPUVertexFormat_Snorm16x2:
        return VK_FORMAT_R16G16_SNORM;
    case WGPUVertexFormat_Snorm16x4:
        return VK_FORMAT_R16G16B16A16_SNORM;
    case WGPUVertexFormat_Float16:
        return VK_FORMAT_R16_SFLOAT;
    case WGPUVertexFormat_Float16x2:
        return VK_FORMAT_R16G16_SFLOAT;
    case WGPUVertexFormat_Float16x4:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case WGPUVertexFormat_Float32:
        return VK_FORMAT_R32_SFLOAT;
    case WGPUVertexFormat_Float32x2:
        return VK_FORMAT_R32G32_SFLOAT;
    case WGPUVertexFormat_Float32x3:
        return VK_FORMAT_R32G32B32_SFLOAT;
    case WGPUVertexFormat_Float32x4:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case WGPUVertexFormat_Uint32:
        return VK_FORMAT_R32_UINT;
    case WGPUVertexFormat_Uint32x2:
        return VK_FORMAT_R32G32_UINT;
    case WGPUVertexFormat_Uint32x3:
        return VK_FORMAT_R32G32B32_UINT;
    case WGPUVertexFormat_Uint32x4:
        return VK_FORMAT_R32G32B32A32_UINT;
    case WGPUVertexFormat_Sint32:
        return VK_FORMAT_R32_SINT;
    case WGPUVertexFormat_Sint32x2:
        return VK_FORMAT_R32G32_SINT;
    case WGPUVertexFormat_Sint32x3:
        return VK_FORMAT_R32G32B32_SINT;
    case WGPUVertexFormat_Sint32x4:
        return VK_FORMAT_R32G32B32A32_SINT;
    case WGPUVertexFormat_Unorm10_10_10_2:
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case WGPUVertexFormat_Unorm8x4BGRA:
        return VK_FORMAT_B8G8R8A8_UNORM;
    default:
        return VK_FORMAT_UNDEFINED; // Default fallback
    }
}
static inline VkDescriptorType toVulkanResourceType(uniform_type type) {
    switch (type) {
        case storage_texture2d:       [[fallthrough]];
        case storage_texture2d_array: [[fallthrough]];
        case storage_texture3d: 
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        
        case storage_buffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case uniform_buffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        
        case texture2d:       [[fallthrough]];
        case texture2d_array: [[fallthrough]];
        case texture3d:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;


        case texture_sampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case acceleration_structure:
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        case combined_image_sampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case uniform_type_undefined: [[fallthrough]];
        case uniform_type_force32:   [[fallthrough]];
        case uniform_type_enumcount: [[fallthrough]];
        default:
            rg_unreachable();
    }
    rg_unreachable();
}

static inline VkVertexInputRate toVulkanVertexStepMode(WGPUVertexStepMode vsm) {
    switch (vsm) {
    case WGPUVertexStepMode_Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case WGPUVertexStepMode_Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    
    case WGPUVertexStepMode_Undefined: //fallthrough
    default:
        return VK_VERTEX_INPUT_RATE_MAX_ENUM; // Default fallback
    }
}
static inline VkIndexType toVulkanIndexFormat(WGPUIndexFormat ifmt) {
    switch (ifmt) {
    case WGPUIndexFormat_Uint16:
        return VK_INDEX_TYPE_UINT16;
    case WGPUIndexFormat_Uint32:
        return VK_INDEX_TYPE_UINT32;
    default:
        return VK_INDEX_TYPE_UINT16; // Default fallback
    }
}

static inline VkShaderStageFlags toVulkanShaderStage(WGPUShaderStageEnum stage) {
    switch (stage){
        case WGPUShaderStageEnum_Vertex:         return VK_SHADER_STAGE_VERTEX_BIT;
        case WGPUShaderStageEnum_TessControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case WGPUShaderStageEnum_TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case WGPUShaderStageEnum_Geometry:       return VK_SHADER_STAGE_GEOMETRY_BIT;
        case WGPUShaderStageEnum_Fragment:       return VK_SHADER_STAGE_FRAGMENT_BIT;
        case WGPUShaderStageEnum_Compute:        return VK_SHADER_STAGE_COMPUTE_BIT;
        case WGPUShaderStageEnum_RayGen:         return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case WGPUShaderStageEnum_Miss:           return VK_SHADER_STAGE_MISS_BIT_KHR;
        case WGPUShaderStageEnum_ClosestHit:     return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case WGPUShaderStageEnum_AnyHit:         return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case WGPUShaderStageEnum_Intersect:      return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case WGPUShaderStageEnum_Callable:       return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case WGPUShaderStageEnum_Task:           return VK_SHADER_STAGE_TASK_BIT_EXT;
        case WGPUShaderStageEnum_Mesh:           return VK_SHADER_STAGE_MESH_BIT_EXT;
        default: rg_unreachable();
    }
}


static inline VkShaderStageFlags toVulkanShaderStageBits(WGPUShaderStage stage) {
    VkShaderStageFlags ret = 0;
    if(stage & WGPUShaderStage_Vertex){ret |= VK_SHADER_STAGE_VERTEX_BIT;}
    if(stage & WGPUShaderStage_TessControl){ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;}
    if(stage & WGPUShaderStage_TessEvaluation){ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;}
    if(stage & WGPUShaderStage_Geometry){ret |= VK_SHADER_STAGE_GEOMETRY_BIT;}
    if(stage & WGPUShaderStage_Fragment){ret |= VK_SHADER_STAGE_FRAGMENT_BIT;}
    if(stage & WGPUShaderStage_Compute){ret |= VK_SHADER_STAGE_COMPUTE_BIT;}
    if(stage & WGPUShaderStage_RayGen){ret |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;}
    if(stage & WGPUShaderStage_Miss){ret |= VK_SHADER_STAGE_MISS_BIT_KHR;}
    if(stage & WGPUShaderStage_ClosestHit){ret |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;}
    if(stage & WGPUShaderStage_AnyHit){ret |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;}
    if(stage & WGPUShaderStage_Intersect){ret |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;}
    if(stage & WGPUShaderStage_Callable){ret |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;}
    if(stage & WGPUShaderStage_Task){ret |= VK_SHADER_STAGE_TASK_BIT_EXT;}
    if(stage & WGPUShaderStage_Mesh){ret |= VK_SHADER_STAGE_MESH_BIT_EXT;}
    return ret;
}

static inline VkPipelineStageFlags toVulkanPipelineStageBits(WGPUShaderStage stage) {
    VkPipelineStageFlags ret = 0;
    if(stage & WGPUShaderStage_Vertex){
        ret |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_TessControl){
        ret |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_TessEvaluation){
        ret |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_Geometry){
        ret |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_Fragment){
        ret |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_Compute){
        ret |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if(stage & WGPUShaderStage_RayGen){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_Miss){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_ClosestHit){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_AnyHit){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_Intersect){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_Callable){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & WGPUShaderStage_Task){
        ret |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
    }
    if(stage & WGPUShaderStage_Mesh){
        ret |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
    }
    return ret;
}

static inline VkBlendFactor toVulkanBlendFactor(WGPUBlendFactor bf) {
    switch (bf) {
    case WGPUBlendFactor_Zero:
        return VK_BLEND_FACTOR_ZERO;
    case WGPUBlendFactor_One:
        return VK_BLEND_FACTOR_ONE;
    case WGPUBlendFactor_Src:
        return VK_BLEND_FACTOR_SRC_COLOR;
    case WGPUBlendFactor_OneMinusSrc:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case WGPUBlendFactor_SrcAlpha:
        return VK_BLEND_FACTOR_SRC_ALPHA;
    case WGPUBlendFactor_OneMinusSrcAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case WGPUBlendFactor_Dst:
        return VK_BLEND_FACTOR_DST_COLOR;
    case WGPUBlendFactor_OneMinusDst:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case WGPUBlendFactor_DstAlpha:
        return VK_BLEND_FACTOR_DST_ALPHA;
    case WGPUBlendFactor_OneMinusDstAlpha:
        return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case WGPUBlendFactor_SrcAlphaSaturated:
        return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case WGPUBlendFactor_Constant:
        return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case WGPUBlendFactor_OneMinusConstant:
        return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case WGPUBlendFactor_Src1:
        return VK_BLEND_FACTOR_SRC1_COLOR;
    case WGPUBlendFactor_OneMinusSrc1:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case WGPUBlendFactor_Src1Alpha:
        return VK_BLEND_FACTOR_SRC1_ALPHA;
    case WGPUBlendFactor_OneMinusSrc1Alpha:
        return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:
        return VK_BLEND_FACTOR_ONE; // Default fallback
    }
}

static inline VkBlendOp toVulkanBlendOperation(WGPUBlendOperation bo) {
    switch (bo) {
    case WGPUBlendOperation_Add:
        return VK_BLEND_OP_ADD;
    case WGPUBlendOperation_Subtract:
        return VK_BLEND_OP_SUBTRACT;
    case WGPUBlendOperation_ReverseSubtract:
        return VK_BLEND_OP_REVERSE_SUBTRACT;
    case WGPUBlendOperation_Min:
        return VK_BLEND_OP_MIN;
    case WGPUBlendOperation_Max:
        return VK_BLEND_OP_MAX;
    default:
        return VK_BLEND_OP_ADD; // Default fallback
    }
}

static inline WGPUTextureFormat toWGPUPixelFormat(PixelFormat format) {
    switch (format) {
        case RGBA8:
            return WGPUTextureFormat_RGBA8Unorm;
        case RGBA8_Srgb:
            return WGPUTextureFormat_RGBA8UnormSrgb;
        case BGRA8:
            return WGPUTextureFormat_BGRA8Unorm;
        case BGRA8_Srgb:
            return WGPUTextureFormat_BGRA8UnormSrgb;
        case RGBA16F:
            return WGPUTextureFormat_RGBA16Float;
        case RGBA32F:
            return WGPUTextureFormat_RGBA32Float;
        case Depth24:
            return WGPUTextureFormat_Depth24Plus;
        case Depth32:
            return WGPUTextureFormat_Depth32Float;
        case GRAYSCALE:
            assert(0 && "GRAYSCALE format not supported in Vulkan.");
        case RGB8:
            assert(0 && "RGB8 format not supported in Vulkan.");
        default:
            rg_unreachable();
    }
    return WGPUTextureFormat_Undefined;
}

static inline VkDescriptorType extractVkDescriptorType(const WGPUBindGroupLayoutEntry* entry){
    if(entry->buffer.type == WGPUBufferBindingType_Storage){
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    if(entry->buffer.type == WGPUBufferBindingType_ReadOnlyStorage){
        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    if(entry->buffer.type == WGPUBufferBindingType_Uniform){
        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
    if(entry->storageTexture.access != WGPUStorageTextureAccess_BindingNotUsed){
        return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    if(entry->texture.sampleType != WGPUTextureSampleType_BindingNotUsed){
        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    }
    if(entry->sampler.type != WGPUSamplerBindingType_BindingNotUsed){
        return VK_DESCRIPTOR_TYPE_SAMPLER;
    }
    if(entry->accelerationStructure){
        return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    }
    rg_trap();
}

static inline VkAccessFlags extractVkAccessFlags(const WGPUBindGroupLayoutEntry* entry){
    if(entry->buffer.type == WGPUBufferBindingType_Storage){
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if(entry->buffer.type == WGPUBufferBindingType_ReadOnlyStorage){
        return VK_ACCESS_SHADER_READ_BIT;
    }
    if(entry->buffer.type == WGPUBufferBindingType_Uniform){
        return VK_ACCESS_SHADER_READ_BIT;
    }
    if(entry->storageTexture.access != WGPUStorageTextureAccess_BindingNotUsed){
        return VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    }
    if(entry->texture.sampleType != WGPUTextureSampleType_BindingNotUsed){
        return VK_ACCESS_SHADER_READ_BIT;
    }
    if(entry->sampler.type != WGPUSamplerBindingType_BindingNotUsed){
        return 0;
    }
    rg_trap();
}







#endif