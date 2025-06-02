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
    VkFence finalTransitionFence;
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
}CallbackWithUserdata;

DEFINE_VECTOR(static inline, CallbackWithUserdata, CallbackWithUserdataVector);

typedef struct WGPUFenceImpl{
    VkFence fence;
    WGPUDevice device;
    refcount_type refCount;
    CallbackWithUserdataVector callbacksOnWaitComplete;
}WGPUFenceImpl;

typedef struct WGPUBindGroupImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    WGPUBindGroupLayout layout;
    refcount_type refCount;
    ResourceUsage resourceUsage;
    WGPUDevice device;
    uint32_t cacheIndex;
    ResourceDescriptor* entries;
    uint32_t entryCount;
}WGPUBindGroupImpl;

typedef struct WGPUBindGroupLayoutImpl{
    VkDescriptorSetLayout layout;
    WGPUDevice device;
    ResourceTypeDescriptor* entries;
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
    VkFence latestFence;
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
    VkDebugUtilsMessengerEXT debugMessenger;
}WGPUInstanceImpl;

typedef struct WGPUFutureImpl{
    void* userdataForFunction;
    void (*functionCalledOnWaitAny)(void*);
}WGPUFutureImpl;

typedef struct WGPUAdapterImpl{
    VkPhysicalDevice physicalDevice;
    WGPUInstance instance;
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
    VkPhysicalDeviceMemoryProperties memProperties;
    QueueIndices queueIndices;
}WGPUAdapterImpl;

#define framesInFlight 2

DEFINE_GENERIC_HASH_MAP(CONTAINERAPI, RenderPassCache, RenderPassLayout, LayoutedRenderPass, renderPassLayoutHash, renderPassLayoutCompare, CLITERAL(RenderPassLayout){0});
typedef struct WGPUDeviceImpl{
    VkDevice device;
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





typedef int (*PrintfFunc_t)(const char *format, ...);

// --- Forward Declarations for Impl structs (if not already via wgvk_structs_impl.h) ---
// (Most are defined as typedefs to pointers, e.g. typedef struct WGPUDeviceImpl* WGPUDevice)
// So, we mostly need declarations for the *Impl_DebugPrint functions.

// --- Forward Declarations for _DebugPrint functions for each struct ---

// Forward declarations for WGPU opaque types are implicit via wgpu_structs_impl.h
// We need to declare print functions for the *Impl structs and WGPU opaque types.

// Helper to print indentation
void PFN_Print_Indent(int indent_level, PrintfFunc_t PFN_printf);

// --- Enum to String Helpers (Declarations) ---
// (User should provide actual implementations if desired, otherwise stubs are used)
const char* RCPassCommandType_ToString(RCPassCommandType type);
const char* LoadOp_ToString(WGPULoadOp op);   // Assuming LoadOp is an enum from raygpu.h
const char* StoreOp_ToString(WGPUStoreOp op); // Assuming StoreOp is an enum from raygpu.h
const char* IndexFormat_ToString(IndexFormat format); // Assuming IndexFormat is an enum
const char* BufferUsage_ToString(WGPUBufferUsage usage); // Assuming BufferUsage is an enum

// --- Debug Print Function Declarations for structs defined in wgpu_structs_impl.h ---

void WGPUStringView_DebugPrint(const WGPUStringView* sv, PrintfFunc_t PFN_printf, int indent);
void WGPURenderPassColorAttachment_DebugPrint(const WGPURenderPassColorAttachment* att, PrintfFunc_t PFN_printf, int indent);
void WGPURenderPassDepthStencilAttachment_DebugPrint(const WGPURenderPassDepthStencilAttachment* att, PrintfFunc_t PFN_printf, int indent);
void WGPUSurfaceConfiguration_DebugPrint(const WGPUSurfaceConfiguration* config, PrintfFunc_t PFN_printf, int indent);
void ResourceDescriptor_DebugPrint(const ResourceDescriptor* desc, PrintfFunc_t PFN_printf, int indent);
void ResourceTypeDescriptor_DebugPrint(const ResourceTypeDescriptor* desc, PrintfFunc_t PFN_printf, int indent);
void WGPUUncapturedErrorCallbackInfo_DebugPrint(const WGPUUncapturedErrorCallbackInfo* info, PrintfFunc_t PFN_printf, int indent);
void VolkDeviceTable_DebugPrint(const struct VolkDeviceTable* table, PrintfFunc_t PFN_printf, int indent);

void ImageUsageRecord_DebugPrint(const ImageUsageRecord* record, PrintfFunc_t PFN_printf, int indent);
void ImageUsageSnap_DebugPrint(const ImageUsageSnap* snap, PrintfFunc_t PFN_printf, int indent);
void BufferUsageRecord_DebugPrint(const BufferUsageRecord* record, PrintfFunc_t PFN_printf, int indent);
// BufferUsageSnap is a typedef for BufferUsageRecord

void RenderPassCommandDraw_DebugPrint(const RenderPassCommandDraw* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandDrawIndexed_DebugPrint(const RenderPassCommandDrawIndexed* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandSetBindGroup_DebugPrint(const RenderPassCommandSetBindGroup* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandSetVertexBuffer_DebugPrint(const RenderPassCommandSetVertexBuffer* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandSetIndexBuffer_DebugPrint(const RenderPassCommandSetIndexBuffer* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandSetPipeline_DebugPrint(const RenderPassCommandSetPipeline* cmd, PrintfFunc_t PFN_printf, int indent);
void ComputePassCommandSetPipeline_DebugPrint(const ComputePassCommandSetPipeline* cmd, PrintfFunc_t PFN_printf, int indent);
void ComputePassCommandDispatchWorkgroups_DebugPrint(const ComputePassCommandDispatchWorkgroups* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandBegin_DebugPrint(const RenderPassCommandBegin* cmd, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandGeneric_DebugPrint(const RenderPassCommandGeneric* cmd, PrintfFunc_t PFN_printf, int indent);

// --- Vector Printers ---
void VkWriteDescriptorSetVector_DebugPrint(const VkWriteDescriptorSetVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkCommandBufferVector_DebugPrint(const VkCommandBufferVector* vec, PrintfFunc_t PFN_printf, int indent);
void RenderPassCommandGenericVector_DebugPrint(const RenderPassCommandGenericVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkSemaphoreVector_DebugPrint(const VkSemaphoreVector* vec, PrintfFunc_t PFN_printf, int indent);
void WGPUCommandBufferVector_DebugPrint(const WGPUCommandBufferVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkDescriptorBufferInfoVector_DebugPrint(const VkDescriptorBufferInfoVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkDescriptorImageInfoVector_DebugPrint(const VkDescriptorImageInfoVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkWriteDescriptorSetAccelerationStructureKHRVector_DebugPrint(const VkWriteDescriptorSetAccelerationStructureKHRVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkDescriptorSetLayoutBindingVector_DebugPrint(const VkDescriptorSetLayoutBindingVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkAttachmentDescriptionVector_DebugPrint(const VkAttachmentDescriptionVector* vec, PrintfFunc_t PFN_printf, int indent);
void WGPUBufferVector_DebugPrint(const WGPUBufferVector* vec, PrintfFunc_t PFN_printf, int indent);
void DescriptorSetAndPoolVector_DebugPrint(const DescriptorSetAndPoolVector* vec, PrintfFunc_t PFN_printf, int indent);
void VkDynamicStateVector_DebugPrint(const VkDynamicStateVector* vec, PrintfFunc_t PFN_printf, int indent);


// --- Hash Map/Set Printers (Placeholders) ---
void BufferUsageRecordMap_DebugPrint(const BufferUsageRecordMap* map, PrintfFunc_t PFN_printf, int indent);
void ImageUsageRecordMap_DebugPrint(const ImageUsageRecordMap* map, PrintfFunc_t PFN_printf, int indent);
void BindGroupUsageSet_DebugPrint(const BindGroupUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void BindGroupLayoutUsageSet_DebugPrint(const BindGroupLayoutUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void SamplerUsageSet_DebugPrint(const SamplerUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void ImageViewUsageSet_DebugPrint(const ImageViewUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void WGPURenderPassEncoderSet_DebugPrint(const WGPURenderPassEncoderSet* set, PrintfFunc_t PFN_printf, int indent);
void RenderPipelineUsageSet_DebugPrint(const RenderPipelineUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void ComputePipelineUsageSet_DebugPrint(const ComputePipelineUsageSet* set, PrintfFunc_t PFN_printf, int indent);
void WGPUComputePassEncoderSet_DebugPrint(const WGPUComputePassEncoderSet* set, PrintfFunc_t PFN_printf, int indent);
void WGPURaytracingPassEncoderSet_DebugPrint(const WGPURaytracingPassEncoderSet* set, PrintfFunc_t PFN_printf, int indent);
void PendingCommandBufferMap_DebugPrint(const PendingCommandBufferMap* map, PrintfFunc_t PFN_printf, int indent);
void BindGroupCacheMap_DebugPrint(const BindGroupCacheMap* map, PrintfFunc_t PFN_printf, int indent);
void RenderPassCache_DebugPrint(const RenderPassCache* cache, PrintfFunc_t PFN_printf, int indent);


void DescriptorSetAndPool_DebugPrint(const DescriptorSetAndPool* dsp, PrintfFunc_t PFN_printf, int indent);
void ResourceUsage_DebugPrint(const ResourceUsage* ru, PrintfFunc_t PFN_printf, int indent);
void SyncState_DebugPrint(const SyncState* ss, PrintfFunc_t PFN_printf, int indent);
void MappableBufferMemory_DebugPrint(const MappableBufferMemory* mem, PrintfFunc_t PFN_printf, int indent);
void AttachmentDescriptor_DebugPrint(const AttachmentDescriptor* ad, PrintfFunc_t PFN_printf, int indent);
void RenderPassLayout_DebugPrint(const RenderPassLayout* rpl, PrintfFunc_t PFN_printf, int indent);
void LayoutedRenderPass_DebugPrint(const LayoutedRenderPass* lrpl, PrintfFunc_t PFN_printf, int indent);
void wgpuxorshiftstate_DebugPrint(const wgpuxorshiftstate* state, PrintfFunc_t PFN_printf, int indent);
void PerframeCache_DebugPrint(const PerframeCache* cache, PrintfFunc_t PFN_printf, int indent);
void QueueIndices_DebugPrint(const QueueIndices* qi, PrintfFunc_t PFN_printf, int indent);

void WGPUSamplerImpl_DebugPrint(const WGPUSamplerImpl* sampler, PrintfFunc_t PFN_printf, int indent);
void WGPUBindGroupImpl_DebugPrint(const WGPUBindGroupImpl* bg, PrintfFunc_t PFN_printf, int indent);
void WGPUBindGroupLayoutImpl_DebugPrint(const WGPUBindGroupLayoutImpl* bgl, PrintfFunc_t PFN_printf, int indent);
void WGPUPipelineLayoutImpl_DebugPrint(const WGPUPipelineLayoutImpl* pl, PrintfFunc_t PFN_printf, int indent);
void WGPUBufferImpl_DebugPrint(const WGPUBufferImpl* buffer, PrintfFunc_t PFN_printf, int indent);
void WGPUBottomLevelAccelerationStructureImpl_DebugPrint(const WGPUBottomLevelAccelerationStructureImpl* blas, PrintfFunc_t PFN_printf, int indent);
void WGPUTopLevelAccelerationStructureImpl_DebugPrint(const WGPUTopLevelAccelerationStructureImpl* tlas, PrintfFunc_t PFN_printf, int indent);
void WGPUInstanceImpl_DebugPrint(const WGPUInstanceImpl* instance, PrintfFunc_t PFN_printf, int indent);
void WGPUFutureImpl_DebugPrint(const WGPUFutureImpl* future, PrintfFunc_t PFN_printf, int indent);
void WGPUAdapterImpl_DebugPrint(const WGPUAdapterImpl* adapter, PrintfFunc_t PFN_printf, int indent);
void WGPUDeviceImpl_DebugPrint(const WGPUDeviceImpl* device, PrintfFunc_t PFN_printf, int indent);
void WGPUTextureImpl_DebugPrint(const WGPUTextureImpl* texture, PrintfFunc_t PFN_printf, int indent);
void WGPUShaderModuleImpl_DebugPrint(const WGPUShaderModuleImpl* sm, PrintfFunc_t PFN_printf, int indent);
void WGPURenderPipelineImpl_DebugPrint(const WGPURenderPipelineImpl* rp, PrintfFunc_t PFN_printf, int indent);
void WGPUComputePipelineImpl_DebugPrint(const WGPUComputePipelineImpl* cp, PrintfFunc_t PFN_printf, int indent);
void WGPURaytracingPipelineImpl_DebugPrint(const WGPURaytracingPipelineImpl* rtp, PrintfFunc_t PFN_printf, int indent);
void WGPUTextureViewImpl_DebugPrint(const WGPUTextureViewImpl* tv, PrintfFunc_t PFN_printf, int indent);
void CommandBufferAndSomeState_DebugPrint(const CommandBufferAndSomeState* cbs, PrintfFunc_t PFN_printf, int indent);
void WGPURenderPassEncoderImpl_DebugPrint(const WGPURenderPassEncoderImpl* rpe, PrintfFunc_t PFN_printf, int indent);
void WGPUComputePassEncoderImpl_DebugPrint(const WGPUComputePassEncoderImpl* cpe, PrintfFunc_t PFN_printf, int indent);
void WGPUCommandEncoderImpl_DebugPrint(const WGPUCommandEncoderImpl* ce, PrintfFunc_t PFN_printf, int indent);
void WGPUCommandBufferImpl_DebugPrint(const WGPUCommandBufferImpl* cb, PrintfFunc_t PFN_printf, int indent);
void WGPURaytracingPassEncoderImpl_DebugPrint(const WGPURaytracingPassEncoderImpl* rtpe, PrintfFunc_t PFN_printf, int indent);
void WGPUSurfaceImpl_DebugPrint(const WGPUSurfaceImpl* surface, PrintfFunc_t PFN_printf, int indent);
void WGPUQueueImpl_DebugPrint(const WGPUQueueImpl* queue, PrintfFunc_t PFN_printf, int indent);


// --- Wrapper Debug Print Functions for WGPU Opaque Handles ---
void WGPUSampler_DebugPrint(WGPUSampler sampler, PrintfFunc_t PFN_printf);
void WGPUBindGroup_DebugPrint(WGPUBindGroup bindGroup, PrintfFunc_t PFN_printf);
void WGPUBindGroupLayout_DebugPrint(WGPUBindGroupLayout bindGroupLayout, PrintfFunc_t PFN_printf);
void WGPUPipelineLayout_DebugPrint(WGPUPipelineLayout pipelineLayout, PrintfFunc_t PFN_printf);
void WGPUBuffer_DebugPrint(WGPUBuffer buffer, PrintfFunc_t PFN_printf);
void WGPUBottomLevelAccelerationStructure_DebugPrint(WGPUBottomLevelAccelerationStructure blas, PrintfFunc_t PFN_printf);
void WGPUTopLevelAccelerationStructure_DebugPrint(WGPUTopLevelAccelerationStructure tlas, PrintfFunc_t PFN_printf);
void WGPUInstance_DebugPrint(WGPUInstance instance, PrintfFunc_t PFN_printf);
void WGPUFuture_DebugPrint(WGPUFuture future, PrintfFunc_t PFN_printf);
void WGPUAdapter_DebugPrint(WGPUAdapter adapter, PrintfFunc_t PFN_printf);
void WGPUDevice_DebugPrint(WGPUDevice device, PrintfFunc_t PFN_printf);
void WGPUTexture_DebugPrint(WGPUTexture texture, PrintfFunc_t PFN_printf);
void WGPUShaderModule_DebugPrint(WGPUShaderModule shaderModule, PrintfFunc_t PFN_printf);
void WGPURenderPipeline_DebugPrint(WGPURenderPipeline renderPipeline, PrintfFunc_t PFN_printf);
void WGPUComputePipeline_DebugPrint(WGPUComputePipeline computePipeline, PrintfFunc_t PFN_printf);
void WGPURaytracingPipeline_DebugPrint(WGPURaytracingPipeline raytracingPipeline, PrintfFunc_t PFN_printf);
void WGPUTextureView_DebugPrint(WGPUTextureView textureView, PrintfFunc_t PFN_printf);
void WGPURenderPassEncoder_DebugPrint(WGPURenderPassEncoder renderPassEncoder, PrintfFunc_t PFN_printf);
void WGPUComputePassEncoder_DebugPrint(WGPUComputePassEncoder computePassEncoder, PrintfFunc_t PFN_printf);
void WGPUCommandEncoder_DebugPrint(WGPUCommandEncoder commandEncoder, PrintfFunc_t PFN_printf);
void WGPUCommandBuffer_DebugPrint(WGPUCommandBuffer commandBuffer, PrintfFunc_t PFN_printf);
void WGPURaytracingPassEncoder_DebugPrint(WGPURaytracingPassEncoder raytracingPassEncoder, PrintfFunc_t PFN_printf);
void WGPUSurface_DebugPrint(WGPUSurface surface, PrintfFunc_t PFN_printf);
void WGPUQueue_DebugPrint(WGPUQueue queue, PrintfFunc_t PFN_printf);






#endif