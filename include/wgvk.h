// clang-format off

#ifndef WGVK_H_INCLUDED
#define WGVK_H_INCLUDED
#include <enum_translation.h>
#include <macros_and_constants.h>
#if SUPPORT_VULKAN_BACKEND == 1
#include <vulkan/vulkan.h>
#endif
#ifdef __cplusplus
#include <cstdint>
extern "C"{
#else
#include <stdint.h>
#endif


#define VMA_MIN_ALIGNMENT 32

#if SUPPORT_WGPU_BACKEND == 1
typedef struct WGPUBufferImpl WGVKBufferImpl;
typedef struct WGPUTextureImpl WGVKTextureImpl;
typedef struct WGPUTextureViewImpl WGVKTextureViewImpl;




typedef WGPUSurface WGVKSurface;
typedef WGPUBindGroupLayout WGVKBindGroupLayout;
typedef WGPUBindGroup WGVKBindGroup;
typedef WGPUBuffer WGVKBuffer;
typedef WGPUAdapter WGVKAdapter;
typedef WGPUDevice WGVKDevice;
typedef WGPUQueue WGVKQueue;
typedef WGPURenderPassEncoder WGVKRenderPassEncoder;
typedef WGPUComputePassEncoder WGVKComputePassEncoder;
typedef WGPUCommandBuffer WGVKCommandBuffer;
typedef WGPUCommandEncoder WGVKCommandEncoder;
typedef WGPUTexture WGVKTexture;
typedef WGPUTextureView WGVKTextureView;
typedef WGPURenderPipeline WGVKRenderPipeline;
typedef WGPUComputePipeline WGVKComputePipeline;


typedef WGPUExtent3D WGVKExtent3D;
typedef WGPUSurfaceConfiguration WGVKSurfaceConfiguration;
typedef WGPUSurfaceCapabilities WGVKSurfaceCapabilities;
typedef WGPUStringView WGVKStringView;
typedef WGPUTexelCopyBufferLayout WGVKTexelCopyBufferLayout;
typedef WGPUTexelCopyBufferInfo WGVKTexelCopyBufferInfo;
typedef WGPUOrigin3D WGVKOrigin3D;
typedef WGPUTexelCopyTextureInfo WGVKTexelCopyTextureInfo;
typedef WGPUBindGroupEntry WGVKBindGroupEntry;

typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
    ShaderStageMask visibility;
}ResourceTypeDescriptor;

typedef struct ResourceDescriptor {
    void const * nextInChain; //hmm
    uint32_t binding;
    /*NULLABLE*/  WGVKBuffer buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ void* sampler;
    /*NULLABLE*/ WGVKTextureView textureView;
} ResourceDescriptor;

typedef struct DColor{
    double r,g,b,a;
}DColor;
typedef WGPURenderPassColorAttachment WGVKRenderPassColorAttachment;
typedef WGPURenderPassDepthStencilAttachment WGVKRenderPassDepthStencilAttachment;
typedef WGPURenderPassDescriptor WGVKRenderPassDescriptor;
typedef WGPUCommandEncoderDescriptor WGVKCommandEncoderDescriptor;
typedef WGPUExtent3D WGVKExtent3D;
typedef WGPUTextureDescriptor WGVKTextureDescriptor;
typedef WGPUTextureViewDescriptor WGVKTextureViewDescriptor;
typedef WGPUBufferDescriptor WGVKBufferDescriptor;
typedef WGPUBindGroupDescriptor WGVKBindGroupDescriptor;
typedef void* WGVKRaytracingPipeline;
typedef void* WGVKRaytracingPassEncoder;
typedef void* WGVKBottomLevelAccelerationStructure;
typedef void* WGVKTopLevelAccelerationStructure;

#elif SUPPORT_VULKAN_BACKEND == 1

#define RTFunctions \
X(CreateRayTracingPipelinesKHR) \
X(CmdBuildAccelerationStructuresKHR) \
X(GetAccelerationStructureBuildSizesKHR) \
X(CreateAccelerationStructureKHR) \
X(DestroyAccelerationStructureKHR) \
X(GetAccelerationStructureDeviceAddressKHR) \
X(GetRayTracingShaderGroupHandlesKHR) \
X(CmdTraceRaysKHR)
#ifdef __cplusplus
#define X(A) extern "C" PFN_vk##A fulk##A;
#else
#define X(A) extern PFN_vk##A fulk##A;
#endif
RTFunctions
#undef X

struct WGVKTextureImpl;
struct WGVKTextureViewImpl;
struct WGVKBufferImpl;
struct WGVKBindGroupImpl;
struct WGVKBindGroupLayoutImpl;
struct WGVKPipelineLayoutImpl;
struct WGVKBufferImpl;
struct WGVKRenderPassEncoderImpl;
struct WGVKComputePassEncoderImpl;
struct WGVKCommandEncoderImpl;
struct WGVKCommandBufferImpl;
struct WGVKTextureImpl;
struct WGVKTextureViewImpl;
struct WGVKQueueImpl;
struct WGVKInstanceImpl;
struct WGVKAdapterImpl;
struct WGVKDeviceImpl;
struct WGVKSurfaceImpl;
struct WGVKRenderPipelineImpl;
struct WGVKComputePipelineImpl;
struct WGVKTopLevelAccelerationStructureImpl;
struct WGVKBottomLevelAccelerationStructureImpl;
struct WGVKRaytracingPipelineImpl;
struct WGVKRaytracingPassEncoderImpl;

typedef struct WGVKSurfaceImpl* WGVKSurface;
typedef struct WGVKBindGroupLayoutImpl* WGVKBindGroupLayout;
typedef struct WGVKPipelineLayoutImpl* WGVKPipelineLayout;
typedef struct WGVKBindGroupImpl* WGVKBindGroup;
typedef struct WGVKBufferImpl* WGVKBuffer;
typedef struct WGVKQueueImpl* WGVKQueue;
typedef struct WGVKInstanceImpl* WGVKInstance;
typedef struct WGVKAdapterImpl* WGVKAdapter;
typedef struct WGVKDeviceImpl* WGVKDevice;
typedef struct WGVKRenderPassEncoderImpl* WGVKRenderPassEncoder;
typedef struct WGVKComputePassEncoderImpl* WGVKComputePassEncoder;
typedef struct WGVKCommandBufferImpl* WGVKCommandBuffer;
typedef struct WGVKCommandEncoderImpl* WGVKCommandEncoder;
typedef struct WGVKTextureImpl* WGVKTexture;
typedef struct WGVKTextureViewImpl* WGVKTextureView;
typedef struct WGVKRenderPipelineImpl* WGVKRenderPipeline;
typedef struct WGVKComputePipelineImpl* WGVKComputePipeline;
typedef struct WGVKTopLevelAccelerationStructureImpl* WGVKTopLevelAccelerationStructure;
typedef struct WGVKBottomLevelAccelerationStructureImpl* WGVKBottomLevelAccelerationStructure;
typedef struct WGVKRaytracingPipelineImpl* WGVKRaytracingPipeline;
typedef struct WGVKRaytracingPassEncoderImpl* WGVKRaytracingPassEncoder;

typedef struct WGVKStringView{
    const char* data;
    size_t length;
}WGVKStringView;

typedef struct WGVKTexelCopyBufferLayout {
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
} WGVKTexelCopyBufferLayout;

typedef struct WGVKTexelCopyBufferInfo {
    WGVKTexelCopyBufferLayout layout;
    WGVKBuffer buffer;
} WGVKTexelCopyBufferInfo;

typedef struct WGVKOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} WGVKOrigin3D;

typedef struct WGVKExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
} WGVKExtent3D;

typedef struct WGVKTexelCopyTextureInfo {
    WGVKTexture texture;
    uint32_t mipLevel;
    WGVKOrigin3D origin;
    VkImageAspectFlagBits aspect;
} WGVKTexelCopyTextureInfo;

typedef struct WGVKBindGroupEntry{
    void* nextInChain;
    uint32_t binding;
    WGVKBuffer buffer;
    uint64_t offset;
    uint64_t size;
    VkSampler sampler;
    WGVKTextureView textureView;
}WGVKBindGroupEntry;

typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
    ShaderStageMask visibility;
}ResourceTypeDescriptor;

typedef struct ResourceDescriptor {
    void const * nextInChain; //hmm
    uint32_t binding;
    /*NULLABLE*/  WGVKBuffer buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ void* sampler;
    /*NULLABLE*/ WGVKTextureView textureView;
    /*NULLABLE*/ WGVKTopLevelAccelerationStructure accelerationStructure;
} ResourceDescriptor;

typedef struct DColor{
    double r,g,b,a;
}DColor;
typedef struct WGVKDeviceDescriptor{
    const void* nextInChain;
}WGVKDeviceDescriptor;
typedef struct WGVKRenderPassColorAttachment{
    void* nextInChain;
    WGVKTextureView view;
    WGVKTextureView resolveTarget;
    uint32_t depthSlice;
    LoadOp loadOp;
    StoreOp storeOp;
    DColor clearValue;
}WGVKRenderPassColorAttachment;

typedef struct WGVKRenderPassDepthStencilAttachment{
    void* nextInChain;
    WGVKTextureView view;
    LoadOp depthLoadOp;
    StoreOp depthStoreOp;
    float depthClearValue;
    uint32_t depthReadOnly;
    LoadOp stencilLoadOp;
    StoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    uint32_t stencilReadOnly;
}WGVKRenderPassDepthStencilAttachment;

typedef struct WGVKRenderPassDescriptor{
    void* nextInChain;
    WGVKStringView label;
    size_t colorAttachmentCount;
    WGVKRenderPassColorAttachment const* colorAttachments;
    WGVKRenderPassDepthStencilAttachment const * depthStencilAttachment;
    
    //Ignore those members
    void* occlusionQuerySet;
    void const *timestampWrites;
}WGVKRenderPassDescriptor;

typedef struct WGVKCommandEncoderDescriptor{
    void* nextInChain;
    WGVKStringView label;
    bool recyclable;
}WGVKCommandEncoderDescriptor;

typedef struct Extent3D{
    uint32_t width, height, depthOrArrayLayers;
}Extent3D;

typedef struct WGVKTextureDescriptor{
    void* nextInChain;
    WGVKStringView label;
    TextureUsage usage;
    TextureDimension dimension;
    Extent3D size;
    PixelFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    size_t viewFormatCount;
}WGVKTextureDescriptor;

typedef struct WGVKTextureViewDescriptor{
    void* nextInChain;
    WGVKStringView label;
    PixelFormat format;
    TextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    TextureAspect aspect;
    TextureUsage usage;
}WGVKTextureViewDescriptor;

typedef struct WGVKBufferDescriptor{
    BufferUsage usage;
    uint64_t size;
}WGVKBufferDescriptor;

typedef struct WGVKBindGroupDescriptor{
    void* nextInChain;
    WGVKStringView label;
    WGVKBindGroupLayout layout;
    size_t entryCount;
    const ResourceDescriptor* entries;
}WGVKBindGroupDescriptor;

typedef struct WGVKPipelineLayoutDescriptor {
    const void* nextInChain;
    WGVKStringView label;
    size_t bindGroupLayoutCount;
    const WGVKBindGroupLayout * bindGroupLayouts;
    uint32_t immediateDataRangeByteSize;
}WGVKPipelineLayoutDescriptor;

typedef struct WGVKSurfaceCapabilities{
    TextureUsage usages;
    size_t formatCount;
    PixelFormat const* formats;
    size_t presentModeCount;
    PresentMode const * presentModes;
}WGVKSurfaceCapabilities;

typedef struct WGVKSurfaceConfiguration {
    WGVKDevice device;                     // Device that surface belongs to (WPGUDevice or WGVKDevice)
    uint32_t width;                   // Width of the rendering surface
    uint32_t height;                  // Height of the rendering surface
    PixelFormat format;               // Pixel format of the surface
    PresentMode presentMode;          // Present mode for image presentation
} WGVKSurfaceConfiguration;


typedef struct WGVKBottomLevelAccelerationStructureDescriptor {
    WGVKBuffer vertexBuffer;            // Buffer containing vertex data
    uint32_t vertexCount;             // Number of vertices
    WGVKBuffer indexBuffer;             // Optional index buffer
    uint32_t indexCount;              // Number of indices
    VkDeviceSize vertexStride;        // Size of each vertex
}WGVKBottomLevelAccelerationStructureDescriptor;

typedef struct WGVKTopLevelAccelerationStructureDescriptor {
    WGVKBottomLevelAccelerationStructure *bottomLevelAS;     // Array of bottom level acceleration structures
    uint32_t blasCount;                                      // Number of BLAS instances
    VkTransformMatrixKHR *transformMatrices;                 // Optional transformation matrices
    uint32_t *instanceCustomIndexes;                         // Optional custom instance indexes
    uint32_t *instanceShaderBindingTableRecordOffsets;       // Optional SBT record offsets
    VkGeometryInstanceFlagsKHR *instanceFlags;               // Optional instance flags
}WGVKTopLevelAccelerationStructureDescriptor;
void wgvkQueueTransitionLayout                (WGVKQueue cSelf, WGVKTexture texture, VkImageLayout from, VkImageLayout to);
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout from, VkImageLayout to);
void wgvkCommandEncoderTransitionTextureLayout(WGVKCommandEncoder encoder, WGVKTexture texture, VkImageLayout from, VkImageLayout to);
void wgvkRenderPassEncoderBindIndexBuffer     (WGVKRenderPassEncoder rpe, WGVKBuffer buffer, VkDeviceSize offset, IndexFormat indexType);
void wgvkRenderPassEncoderBindVertexBuffer    (WGVKRenderPassEncoder rpe, uint32_t binding, WGVKBuffer buffer, VkDeviceSize offset);
WGVKTopLevelAccelerationStructure wgvkDeviceCreateTopLevelAccelerationStructure(WGVKDevice device, const WGVKTopLevelAccelerationStructureDescriptor *descriptor);
WGVKBottomLevelAccelerationStructure wgvkDeviceCreateBottomLevelAccelerationStructure(WGVKDevice device, const WGVKBottomLevelAccelerationStructureDescriptor *descriptor);
#endif
void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities);
void wgvkSurfaceConfigure(WGVKSurface surface, const WGVKSurfaceConfiguration* config);
WGVKTexture wgvkDeviceCreateTexture(WGVKDevice device, const WGVKTextureDescriptor* descriptor);
WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor);
WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const WGVKBufferDescriptor* desc);
void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, const void* data, size_t size);
void wgvkBufferMap(WGVKBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data);
void wgvkBufferUnmap(WGVKBuffer buffer);
size_t wgvkBufferGetSize(WGVKBuffer buffer);
void wgvkQueueWriteTexture(WGVKQueue queue, WGVKTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGVKTexelCopyBufferLayout const * dataLayout, WGVKExtent3D const * writeSize);
WGVKBindGroupLayout wgvkDeviceCreateBindGroupLayout(WGVKDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount);
WGVKPipelineLayout wgvkDeviceCreatePipelineLayout(WGVKDevice device, const WGVKPipelineLayoutDescriptor* pldesc);
WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc);
void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup, const WGVKBindGroupDescriptor* bgdesc);


WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* cdesc);
WGVKCommandBuffer wgvkCommandEncoderFinish    (WGVKCommandEncoder commandEncoder);
void wgvkQueueSubmit                          (WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers);
void wgvkCommandEncoderCopyBufferToBuffer     (WGVKCommandEncoder commandEncoder, WGVKBuffer source, uint64_t sourceOffset, WGVKBuffer destination, uint64_t destinationOffset, uint64_t size);
void wgvkCommandEncoderCopyBufferToTexture    (WGVKCommandEncoder commandEncoder, WGVKTexelCopyBufferInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize);
void wgvkCommandEncoderCopyTextureToBuffer    (WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyBufferInfo const * destination, WGVKExtent3D const * copySize);
void wgvkCommandEncoderCopyTextureToTexture   (WGVKCommandEncoder commandEncoder, WGVKTexelCopyTextureInfo const * source, WGVKTexelCopyTextureInfo const * destination, WGVKExtent3D const * copySize);
void wgvkRenderpassEncoderDraw                (WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
void wgvkRenderpassEncoderDrawIndexed         (WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance);
void wgvkRenderPassEncoderSetBindGroup        (WGVKRenderPassEncoder rpe, uint32_t group, WGVKBindGroup dset);
void wgvkRenderPassEncoderSetPipeline         (WGVKRenderPassEncoder rpe, WGVKRenderPipeline renderPipeline);
void wgvkComputePassEncoderSetPipeline        (WGVKComputePassEncoder cpe, WGVKComputePipeline computePipeline);
void wgvkComputePassEncoderSetBindGroup       (WGVKComputePassEncoder cpe, uint32_t groupIndex, WGVKBindGroup bindGroup);
void wgvkRaytracingPassEncoderSetPipeline     (WGVKRaytracingPassEncoder cpe, WGVKRaytracingPipeline raytracingPipeline);
void wgvkRaytracingPassEncoderSetBindGroup    (WGVKRaytracingPassEncoder cpe, uint32_t groupIndex, WGVKBindGroup bindGroup);
void wgvkRaytracingPassEncoderTraceRays       (WGVKRaytracingPassEncoder cpe, uint32_t width, uint32_t height, uint32_t depth);

void wgvkComputePassEncoderDispatchWorkgroups (WGVKComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z);
void wgvkReleaseComputePassEncoder            (WGVKComputePassEncoder cpenc);
void wgvkSurfacePresent                       (WGVKSurface surface);

WGVKRaytracingPassEncoder wgvkCommandEncoderBeginRaytracingPass(WGVKCommandEncoder enc);
void wgvkCommandEncoderEndRaytracingPass(WGVKRaytracingPassEncoder commandEncoder);
WGVKComputePassEncoder wgvkCommandEncoderBeginComputePass(WGVKCommandEncoder enc);
void wgvkCommandEncoderEndComputePass(WGVKComputePassEncoder commandEncoder);
WGVKRenderPassEncoder wgvkCommandEncoderBeginRenderPass(WGVKCommandEncoder enc, const WGVKRenderPassDescriptor* rpdesc);
void wgvkRenderPassEncoderEnd(WGVKRenderPassEncoder renderPassEncoder);

void wgvkReleaseRaytracingPassEncoder         (WGVKRaytracingPassEncoder rtenc);
void wgvkTextureAddRef                        (WGVKTexture texture);
void wgvkTextureViewAddRef                    (WGVKTextureView textureView);
void wgvkBufferAddRef                         (WGVKBuffer buffer);
void wgvkBindGroupAddRef                      (WGVKBindGroup bindGroup);
void wgvkBindGroupLayoutAddRef                (WGVKBindGroupLayout bindGroupLayout);
void wgvkReleaseCommandEncoder                (WGVKCommandEncoder commandBuffer);
void wgvkReleaseCommandBuffer                 (WGVKCommandBuffer commandBuffer);
void wgvkReleaseRenderPassEncoder             (WGVKRenderPassEncoder rpenc);
void wgvkReleaseComputePassEncoder            (WGVKComputePassEncoder rpenc);
void wgvkBufferRelease                        (WGVKBuffer commandBuffer);
void wgvkBindGroupRelease                     (WGVKBindGroup commandBuffer);
void wgvkBindGroupLayoutRelease               (WGVKBindGroupLayout commandBuffer);
void wgvkReleaseBindGroupLayout               (WGVKBindGroupLayout bglayout);
void wgvkReleaseTexture                       (WGVKTexture texture);
void wgvkReleaseTextureView                   (WGVKTextureView view);


WGVKCommandEncoder wgvkResetCommandBuffer(WGVKCommandBuffer commandEncoder);

void wgvkCommandEncoderTraceRays(WGVKRenderPassEncoder encoder);
#ifdef __cplusplus
} //extern "C"
#endif

#endif // WGVK_H_INCLUDED