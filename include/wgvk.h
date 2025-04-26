// clang-format off

#ifndef WGVK_H_INCLUDED
#define WGVK_H_INCLUDED
#include <macros_and_constants.h>
#if SUPPORT_VULKAN_BACKEND == 1
    #include <external/volk.h>
#endif
#include <webgpu/webgpu.h>
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
typedef WGPUPipelineLayout WGVKPipelineLayout;
typedef WGPUBindGroup WGVKBindGroup;
typedef WGPUBuffer WGVKBuffer;
typedef WGPUInstance WGVKInstance;
typedef WGPUAdapter WGVKAdapter;
typedef WGPUFuture WGVKFuture;
typedef WGPUDevice WGVKDevice;
typedef WGPUQueue WGVKQueue;
typedef WGPURenderPassEncoder WGVKRenderPassEncoder;
typedef WGPUComputePassEncoder WGVKComputePassEncoder;
typedef WGPUCommandBuffer WGVKCommandBuffer;
typedef WGPUCommandEncoder WGVKCommandEncoder;
typedef WGPUTexture WGVKTexture;
typedef WGPUTextureView WGVKTextureView;
typedef WGPUSampler WGVKSampler;
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
typedef WGPUSamplerDescriptor WGVKSamplerDescriptor;
typedef WGPUBufferDescriptor WGVKBufferDescriptor;
typedef WGPUBindGroupDescriptor WGVKBindGroupDescriptor;
typedef WGPUInstanceDescriptor WGVKInstanceDescriptor;
typedef WGPURequestAdapterOptions WGVKRequestAdapterOptions;
typedef WGPURequestAdapterCallbackInfo WGVKRequestAdapterCallbackInfo;
typedef WGPUDeviceDescriptor WGVKDeviceDescriptor;
typedef WGPUVertexAttribute VertexAttribute;
typedef WGPURenderPipelineDescriptor WGVKRenderPipelineDescriptor;
typedef WGPUPipelineLayoutDescriptor WGVKPipelineLayoutDescriptor;
typedef WGPUBlendComponent WGVKBlendComponent;
typedef WGPUBlendState WGVKBlendState;
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
struct WGVKFutureImpl;
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
struct WGVKShaderModuleImpl;
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
typedef struct WGVKFutureImpl* WGVKFuture;
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
typedef struct WGVKSamplerImpl* WGVKSampler;
typedef struct WGVKRenderPipelineImpl* WGVKRenderPipeline;
typedef struct WGVKShaderModuleImpl* WGVKShaderModule;
typedef struct WGVKComputePipelineImpl* WGVKComputePipeline;
typedef struct WGVKTopLevelAccelerationStructureImpl* WGVKTopLevelAccelerationStructure;
typedef struct WGVKBottomLevelAccelerationStructureImpl* WGVKBottomLevelAccelerationStructure;
typedef struct WGVKRaytracingPipelineImpl* WGVKRaytracingPipeline;
typedef struct WGVKRaytracingPassEncoderImpl* WGVKRaytracingPassEncoder;
#endif

typedef enum PresentMode{ 
    PresentMode_Undefined = 0x00000000,
    PresentMode_Fifo = 0x00000001,
    PresentMode_FifoRelaxed = 0x00000002,
    PresentMode_Immediate = 0x00000003,
    PresentMode_Mailbox = 0x00000004,
}PresentMode;

typedef enum TextureAspect {
    TextureAspect_Undefined = 0x00000000,
    TextureAspect_All = 0x00000001,
    TextureAspect_StencilOnly = 0x00000002,
    TextureAspect_DepthOnly = 0x00000003,
    TextureAspect_Plane0Only = 0x00050000,
    TextureAspect_Plane1Only = 0x00050001,
    TextureAspect_Plane2Only = 0x00050002,
    TextureAspect_Force32 = 0x7FFFFFFF
} TextureAspect;
typedef enum PrimitiveType{
    RL_TRIANGLES, RL_TRIANGLE_STRIP, RL_QUADS, RL_LINES, RL_POINTS
}PrimitiveType;
typedef enum VertexStepMode { VertexStepMode_None = 0x0, VertexStepMode_Vertex = 0x1, VertexStepMode_Instance = 0x2, VertexStepMode_Force32 = 0x7FFFFFFF } VertexStepMode;

typedef uint64_t BufferUsage;
typedef uint64_t TextureUsage;

typedef enum TFilterMode { TFilterMode_Undefined = 0x00000000, TFilterMode_Nearest = 0x00000001, TFilterMode_Linear = 0x00000002, TFilterMode_Force32 = 0x7FFFFFFF } TFilterMode;

typedef enum FrontFace { FrontFace_Undefined = 0x00000000, FrontFace_CCW = 0x00000001, FrontFace_CW = 0x00000002, FrontFace_Force32 = 0x7FFFFFFF } FrontFace;

typedef enum IndexFormat { IndexFormat_Undefined = 0x00000000, IndexFormat_Uint16 = 0x00000001, IndexFormat_Uint32 = 0x00000002, IndexFormat_Force32 = 0x7FFFFFFF } IndexFormat;

typedef enum uniform_type { uniform_type_undefined, uniform_buffer, storage_buffer, texture2d, texture2d_array, storage_texture2d, texture_sampler, texture3d, storage_texture3d, storage_texture2d_array, acceleration_structure, combined_image_sampler} uniform_type;

typedef enum access_type { readonly, readwrite, writeonly } access_type;

typedef enum format_or_sample_type { we_dont_know, sample_f32, sample_u32, format_r32float, format_r32uint, format_rgba8unorm, format_rgba32float } format_or_sample_type;


typedef enum ShaderStage{
    ShaderStage_Vertex,
    ShaderStage_TessControl,
    ShaderStage_TessEvaluation,
    ShaderStage_Geometry,
    ShaderStage_Fragment,
    ShaderStage_Compute,
    ShaderStage_RayGen,
    ShaderStage_RayGenNV = ShaderStage_RayGen,
    ShaderStage_Intersect,
    ShaderStage_IntersectNV = ShaderStage_Intersect,
    ShaderStage_AnyHit,
    ShaderStage_AnyHitNV = ShaderStage_AnyHit,
    ShaderStage_ClosestHit,
    ShaderStage_ClosestHitNV = ShaderStage_ClosestHit,
    ShaderStage_Miss,
    ShaderStage_MissNV = ShaderStage_Miss,
    ShaderStage_Callable,
    ShaderStage_CallableNV = ShaderStage_Callable,
    ShaderStage_Task,
    ShaderStage_TaskNV = ShaderStage_Task,
    ShaderStage_Mesh,
    ShaderStage_MeshNV = ShaderStage_Mesh,
    ShaderStage_EnumCount
}ShaderStage;

typedef enum ShaderStageMask{
    ShaderStageMask_Vertex = (1u << ShaderStage_Vertex),
    ShaderStageMask_TessControl = (1u << ShaderStage_TessControl),
    ShaderStageMask_TessEvaluation = (1u << ShaderStage_TessEvaluation),
    ShaderStageMask_Geometry = (1u << ShaderStage_Geometry),
    ShaderStageMask_Fragment = (1u << ShaderStage_Fragment),
    ShaderStageMask_Compute = (1u << ShaderStage_Compute),
    ShaderStageMask_RayGen = (1u << ShaderStage_RayGen),
    ShaderStageMask_RayGenNV = (1u << ShaderStage_RayGenNV),
    ShaderStageMask_Intersect = (1u << ShaderStage_Intersect),
    ShaderStageMask_IntersectNV = (1u << ShaderStage_IntersectNV),
    ShaderStageMask_AnyHit = (1u << ShaderStage_AnyHit),
    ShaderStageMask_AnyHitNV = (1u << ShaderStage_AnyHitNV),
    ShaderStageMask_ClosestHit = (1u << ShaderStage_ClosestHit),
    ShaderStageMask_ClosestHitNV = (1u << ShaderStage_ClosestHitNV),
    ShaderStageMask_Miss = (1u << ShaderStage_Miss),
    ShaderStageMask_MissNV = (1u << ShaderStage_MissNV),
    ShaderStageMask_Callable = (1u << ShaderStage_Callable),
    ShaderStageMask_CallableNV = (1u << ShaderStage_CallableNV),
    ShaderStageMask_Task = (1u << ShaderStage_Task),
    ShaderStageMask_TaskNV = (1u << ShaderStage_TaskNV),
    ShaderStageMask_Mesh = (1u << ShaderStage_Mesh),
    ShaderStageMask_MeshNV = (1u << ShaderStage_MeshNV),
    ShaderStageMask_EnumCount = (1u << ShaderStage_EnumCount)
}ShaderStageMask;


typedef enum filterMode {
    filter_nearest = 0x1,
    filter_linear = 0x2,
} filterMode;

typedef enum addressMode {
    clampToEdge = 0x1,
    repeat = 0x2,
    mirrorRepeat = 0x3,
} addressMode;

typedef enum PixelFormat {
    RGBA8      = 0x12, //WGPUTextureFormat_RGBA8Unorm,
    RGBA8_Srgb = 0x13, //WGPUTextureFormat_RGBA8UnormSrgb,
    BGRA8      = 0x17, //WGPUTextureFormat_BGRA8Unorm,
    BGRA8_Srgb = 0x18, //WGPUTextureFormat_BGRA8UnormSrgb,
    RGBA16F    = 0x22, //WGPUTextureFormat_RGBA16Float,
    RGBA32F    = 0x23, //WGPUTextureFormat_RGBA32Float,
    Depth24    = 0x28, //WGPUTextureFormat_Depth24Plus,
    Depth32    = 0x2A, //WGPUTextureFormat_Depth32Float,

    GRAYSCALE = 0x100000, // No WGPU_ equivalent
    RGB8 = 0x100001,      // No WGPU_ equivalent
    PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum CompareFunction {
    CompareFunction_Undefined = 0x00000000,
    CompareFunction_Never = 0x00000001,
    CompareFunction_Less = 0x00000002,
    CompareFunction_Equal = 0x00000003,
    CompareFunction_LessEqual = 0x00000004,
    CompareFunction_Greater = 0x00000005,
    CompareFunction_NotEqual = 0x00000006,
    CompareFunction_GreaterEqual = 0x00000007,
    CompareFunction_Always = 0x00000008
} CompareFunction;
typedef enum MapMode{
    MapMode_Read,
    MapMode_Write,
    MapMode_ReadWrite,
}MapMode;

typedef enum TextureDimension{
    TextureDimension_Undefined = 0x00000000,
    TextureDimension_1D = 0x00000001,
    TextureDimension_2D = 0x00000002,
    //TextureViewDimension_2DArray = 0x00000003,
    //TextureViewDimension_Cube = 0x00000004,
    //TextureViewDimension_CubeArray = 0x00000005,
    TextureDimension_3D = 0x00000003
}TextureDimension;
typedef enum TextureViewDimension{
    TextureViewDimension_Undefined = 0x00000000,
    TextureViewDimension_1D = 0x00000001,
    TextureViewDimension_2D = 0x00000002,
    TextureViewDimension_2DArray = 0x00000003,
    //TextureViewDimension_Cube = 0x00000004,
    //TextureViewDimension_CubeArray = 0x00000005,
    TextureViewDimension_3D = 0x00000006,
    TextureViewDimension_Force32 = 0x7FFFFFFF
}TextureViewDimension;

typedef enum WGVKCompositeAlphaMode {
    WGVKCompositeAlphaMode_Auto = 0x00000000,
    WGVKCompositeAlphaMode_Opaque = 0x00000001,
    WGVKCompositeAlphaMode_Premultiplied = 0x00000002,
    WGVKCompositeAlphaMode_Unpremultiplied = 0x00000003,
    WGVKCompositeAlphaMode_Inherit = 0x00000004,
    WGVKCompositeAlphaMode_Force32 = 0x7FFFFFFF
} WGVKCompositeAlphaMode;

typedef enum WGVKCullMode{
    WGVKCullMode_Undefined = 0x00000000,
    WGVKCullMode_None = 0x00000001,
    WGVKCullMode_Front = 0x00000002,
    WGVKCullMode_Back = 0x00000003,
    WGVKCullMode_Force32 = 0x7FFFFFFF
}WGVKCullMode;

typedef enum LoadOp {
    LoadOp_Undefined = 0x00000000,
    LoadOp_Load = 0x00000001,
    LoadOp_Clear = 0x00000002,
    LoadOp_ExpandResolveTexture = 0x00050003,
    LoadOp_Force32 = 0x7FFFFFFF
} LoadOp;

typedef enum StoreOp {
    StoreOp_Undefined = 0x00000000,
    StoreOp_Store = 0x00000001,
    StoreOp_Discard = 0x00000002,
    StoreOp_Force32 = 0x7FFFFFFF
} StoreOp;


typedef enum VertexFormat {
    VertexFormat_Uint8 = 0x00000001,
    VertexFormat_Uint8x2 = 0x00000002,
    VertexFormat_Uint8x4 = 0x00000003,
    VertexFormat_Sint8 = 0x00000004,
    VertexFormat_Sint8x2 = 0x00000005,
    VertexFormat_Sint8x4 = 0x00000006,
    VertexFormat_Unorm8 = 0x00000007,
    VertexFormat_Unorm8x2 = 0x00000008,
    VertexFormat_Unorm8x4 = 0x00000009,
    VertexFormat_Snorm8 = 0x0000000A,
    VertexFormat_Snorm8x2 = 0x0000000B,
    VertexFormat_Snorm8x4 = 0x0000000C,
    VertexFormat_Uint16 = 0x0000000D,
    VertexFormat_Uint16x2 = 0x0000000E,
    VertexFormat_Uint16x4 = 0x0000000F,
    VertexFormat_Sint16 = 0x00000010,
    VertexFormat_Sint16x2 = 0x00000011,
    VertexFormat_Sint16x4 = 0x00000012,
    VertexFormat_Unorm16 = 0x00000013,
    VertexFormat_Unorm16x2 = 0x00000014,
    VertexFormat_Unorm16x4 = 0x00000015,
    VertexFormat_Snorm16 = 0x00000016,
    VertexFormat_Snorm16x2 = 0x00000017,
    VertexFormat_Snorm16x4 = 0x00000018,
    VertexFormat_Float16 = 0x00000019,
    VertexFormat_Float16x2 = 0x0000001A,
    VertexFormat_Float16x4 = 0x0000001B,
    VertexFormat_Float32 = 0x0000001C,
    VertexFormat_Float32x2 = 0x0000001D,
    VertexFormat_Float32x3 = 0x0000001E,
    VertexFormat_Float32x4 = 0x0000001F,
    VertexFormat_Uint32 = 0x00000020,
    VertexFormat_Uint32x2 = 0x00000021,
    VertexFormat_Uint32x3 = 0x00000022,
    VertexFormat_Uint32x4 = 0x00000023,
    VertexFormat_Sint32 = 0x00000024,
    VertexFormat_Sint32x2 = 0x00000025,
    VertexFormat_Sint32x3 = 0x00000026,
    VertexFormat_Sint32x4 = 0x00000027,
    VertexFormat_Unorm10_10_10_2 = 0x00000028,
    VertexFormat_Unorm8x4BGRA = 0x00000029,
    VertexFormat_Force32 = 0x7FFFFFFF
} VertexFormat;
typedef enum BlendFactor {
    BlendFactor_Undefined = 0x00000000,
    BlendFactor_Zero = 0x00000001,
    BlendFactor_One = 0x00000002,
    BlendFactor_Src = 0x00000003,
    BlendFactor_OneMinusSrc = 0x00000004,
    BlendFactor_SrcAlpha = 0x00000005,
    BlendFactor_OneMinusSrcAlpha = 0x00000006,
    BlendFactor_Dst = 0x00000007,
    BlendFactor_OneMinusDst = 0x00000008,
    BlendFactor_DstAlpha = 0x00000009,
    BlendFactor_OneMinusDstAlpha = 0x0000000A,
    BlendFactor_SrcAlphaSaturated = 0x0000000B,
    BlendFactor_Constant = 0x0000000C,
    BlendFactor_OneMinusConstant = 0x0000000D,
    BlendFactor_Src1 = 0x0000000E,
    BlendFactor_OneMinusSrc1 = 0x0000000F,
    BlendFactor_Src1Alpha = 0x00000010,
    BlendFactor_OneMinusSrc1Alpha = 0x00000011,
} BlendFactor;

typedef enum BlendOperation {
    BlendOperation_Undefined = 0x00000000,
    BlendOperation_Add = 0x00000001,
    BlendOperation_Subtract = 0x00000002,
    BlendOperation_ReverseSubtract = 0x00000003,
    BlendOperation_Min = 0x00000004,
    BlendOperation_Max = 0x00000005,
    //BlendOperation_Force32 = 0x7FFFFFFF
} BlendOperation;

typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
    ShaderStageMask visibility;
}ResourceTypeDescriptor;

#if SUPPORT_VULKAN_BACKEND == 1
typedef enum WGVKRequestAdapterStatus {
    WGVKRequestAdapterStatus_Success = 0x00000001,
    WGVKRequestAdapterStatus_InstanceDropped = 0x00000002,
    WGVKRequestAdapterStatus_Unavailable = 0x00000003,
    WGVKRequestAdapterStatus_Error = 0x00000004,
    WGVKRequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGVKRequestAdapterStatus;





typedef enum WGVKStencilOperation {
    WGVKStencilOperation_Undefined = 0x00000000,
    WGVKStencilOperation_Keep = 0x00000001,
    WGVKStencilOperation_Zero = 0x00000002,
    WGVKStencilOperation_Replace = 0x00000003,
    WGVKStencilOperation_Invert = 0x00000004,
    WGVKStencilOperation_IncrementClamp = 0x00000005,
    WGVKStencilOperation_DecrementClamp = 0x00000006,
    WGVKStencilOperation_IncrementWrap = 0x00000007,
    WGVKStencilOperation_DecrementWrap = 0x00000008,
    WGVKStencilOperation_Force32 = 0x7FFFFFFF
} WGVKStencilOperation;

typedef enum WGVKSType {
    WGVKSType_ShaderSourceSPIRV = 0x00000001,
    WGVKSType_ShaderSourceWGSL = 0x00000002,
    WGVKSType_InstanceValidationLayerSelection = 0x10000001
}WGVKSType;

typedef int WGVKDeviceLostReason;
typedef int WGVKErrorType;

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

typedef struct WGVKChainedStruct {
    struct WGVKChainedStruct* next;
    WGVKSType sType;
} WGVKChainedStruct;

typedef struct WGVKRequestAdapterOptions {
    WGVKChainedStruct * nextInChain;
    int featureLevel;
    int powerPreference;
    Bool32 forceFallbackAdapter;
    int backendType;
    WGVKSurface compatibleSurface;
} WGVKRequestAdapterOptions;

typedef struct WGVKInstanceCapabilities {
    WGVKChainedStruct* nextInChain;
    Bool32 timedWaitAnyEnable;
    size_t timedWaitAnyMaxCount;
} WGVKInstanceCapabilities;

typedef struct WGVKInstanceLayerSelection{
    WGVKChainedStruct chain;
    const char* const* instanceLayers;
    uint32_t instanceLayerCount;
}WGVKInstanceLayerSelection;

typedef struct WGVKInstanceDescriptor {
    WGVKChainedStruct* nextInChain;
    WGVKInstanceCapabilities capabilities;
} WGVKInstanceDescriptor;

typedef struct WGVKBindGroupEntry{
    WGVKChainedStruct* nextInChain;
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
    /*NULLABLE*/ WGVKSampler sampler;
    /*NULLABLE*/ WGVKTextureView textureView;
    /*NULLABLE*/ WGVKTopLevelAccelerationStructure accelerationStructure;
} ResourceDescriptor;

typedef struct WGVKSamplerDescriptor {
    WGVKChainedStruct * nextInChain;
    WGVKStringView label;
    addressMode addressModeU;
    addressMode addressModeV;
    addressMode addressModeW;
    filterMode magFilter;
    filterMode minFilter;
    filterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    CompareFunction compare;
    uint16_t maxAnisotropy;
} WGVKSamplerDescriptor;

typedef struct DColor{
    double r,g,b,a;
}DColor;
typedef enum WGVKFeatureLevel {
    WGVKFeatureLevel_Undefined = 0x00000000,
    WGVKFeatureLevel_Compatibility = 0x00000001,
    WGVKFeatureLevel_Core = 0x00000002,
    WGVKFeatureLevel_Force32 = 0x7FFFFFFF
} WGVKFeatureLevel;
typedef enum WGVKFeatureName {
    WGVKFeatureName_DepthClipControl = 0x00000001,
    WGVKFeatureName_Depth32FloatStencil8 = 0x00000002,
    WGVKFeatureName_TimestampQuery = 0x00000003,
    WGVKFeatureName_TextureCompressionBC = 0x00000004,
    WGVKFeatureName_TextureCompressionBCSliced3D = 0x00000005,
    WGVKFeatureName_TextureCompressionETC2 = 0x00000006,
    WGVKFeatureName_TextureCompressionASTC = 0x00000007,
    WGVKFeatureName_TextureCompressionASTCSliced3D = 0x00000008,
    WGVKFeatureName_IndirectFirstInstance = 0x00000009,
    WGVKFeatureName_ShaderF16 = 0x0000000A,
    WGVKFeatureName_RG11B10UfloatRenderable = 0x0000000B,
    WGVKFeatureName_BGRA8UnormStorage = 0x0000000C,
    WGVKFeatureName_Float32Filterable = 0x0000000D,
    WGVKFeatureName_Float32Blendable = 0x0000000E,
    WGVKFeatureName_ClipDistances = 0x0000000F,
    WGVKFeatureName_DualSourceBlending = 0x00000010,
    WGVKFeatureName_Subgroups = 0x00000011,
    WGVKFeatureName_CoreFeaturesAndLimits = 0x00000012,
    WGVKFeatureName_Force32 = 0x7FFFFFFF
} WGVKFeatureName;
typedef struct WGVKLimits {
    WGVKChainedStruct* nextInChain;
    uint32_t maxTextureDimension1D;
    uint32_t maxTextureDimension2D;
    uint32_t maxTextureDimension3D;
    uint32_t maxTextureArrayLayers;
    uint32_t maxBindGroups;
    uint32_t maxBindGroupsPlusVertexBuffers;
    uint32_t maxBindingsPerBindGroup;
    uint32_t maxDynamicUniformBuffersPerPipelineLayout;
    uint32_t maxDynamicStorageBuffersPerPipelineLayout;
    uint32_t maxSampledTexturesPerShaderStage;
    uint32_t maxSamplersPerShaderStage;
    uint32_t maxStorageBuffersPerShaderStage;
    uint32_t maxStorageTexturesPerShaderStage;
    uint32_t maxUniformBuffersPerShaderStage;
    uint64_t maxUniformBufferBindingSize;
    uint64_t maxStorageBufferBindingSize;
    uint32_t minUniformBufferOffsetAlignment;
    uint32_t minStorageBufferOffsetAlignment;
    uint32_t maxVertexBuffers;
    uint64_t maxBufferSize;
    uint32_t maxVertexAttributes;
    uint32_t maxVertexBufferArrayStride;
    uint32_t maxInterStageShaderVariables;
    uint32_t maxColorAttachments;
    uint32_t maxColorAttachmentBytesPerSample;
    uint32_t maxComputeWorkgroupStorageSize;
    uint32_t maxComputeInvocationsPerWorkgroup;
    uint32_t maxComputeWorkgroupSizeX;
    uint32_t maxComputeWorkgroupSizeY;
    uint32_t maxComputeWorkgroupSizeZ;
    uint32_t maxComputeWorkgroupsPerDimension;
    uint32_t maxStorageBuffersInVertexStage;
    uint32_t maxStorageTexturesInVertexStage;
    uint32_t maxStorageBuffersInFragmentStage;
    uint32_t maxStorageTexturesInFragmentStage;
}WGVKLimits;

typedef struct WGVKQueueDescriptor {
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
}WGVKQueueDescriptor;

typedef void (*WGVKDeviceLostCallback)(const WGVKDevice*, WGVKDeviceLostReason, struct WGVKStringView, void*, void*);
typedef void (*WGVKUncapturedErrorCallback)(const WGVKDevice*, WGVKErrorType, struct WGVKStringView, void*, void*);

typedef struct WGVKDeviceLostCallbackInfo {
    WGVKChainedStruct * nextInChain;
    int mode;
    WGVKDeviceLostCallback callback;
    void* userdata1;
    void* userdata2;
} WGVKDeviceLostCallbackInfo;
typedef struct WGVKUncapturedErrorCallbackInfo {
    WGVKChainedStruct * nextInChain;
    WGVKUncapturedErrorCallback callback;
    void* userdata1;
    void* userdata2;
} WGVKUncapturedErrorCallbackInfo;
typedef struct WGVKDeviceDescriptor {
    WGVKChainedStruct * nextInChain;
    WGVKStringView label;
    size_t requiredFeatureCount;
    WGVKFeatureName const * requiredFeatures;
    WGVKLimits const * requiredLimits;
    WGVKQueueDescriptor defaultQueue;
    WGVKDeviceLostCallbackInfo deviceLostCallbackInfo;
    WGVKUncapturedErrorCallbackInfo uncapturedErrorCallbackInfo;
} WGVKDeviceDescriptor;

typedef struct WGVKRenderPassColorAttachment{
    WGVKChainedStruct* nextInChain;
    WGVKTextureView view;
    WGVKTextureView resolveTarget;
    uint32_t depthSlice;
    LoadOp loadOp;
    StoreOp storeOp;
    DColor clearValue;
}WGVKRenderPassColorAttachment;

typedef struct WGVKRenderPassDepthStencilAttachment{
    WGVKChainedStruct* nextInChain;
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
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
    size_t colorAttachmentCount;
    WGVKRenderPassColorAttachment const* colorAttachments;
    WGVKRenderPassDepthStencilAttachment const * depthStencilAttachment;
    
    //Ignore those members
    void* occlusionQuerySet;
    void const *timestampWrites;
}WGVKRenderPassDescriptor;

typedef struct WGVKCommandEncoderDescriptor{
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
    bool recyclable;
}WGVKCommandEncoderDescriptor;

typedef struct Extent3D{
    uint32_t width, height, depthOrArrayLayers;
}Extent3D;

typedef struct WGVKTextureDescriptor{
    WGVKChainedStruct* nextInChain;
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
    WGVKChainedStruct* nextInChain;
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
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
    WGVKBindGroupLayout layout;
    size_t entryCount;
    const ResourceDescriptor* entries;
}WGVKBindGroupDescriptor;

typedef struct WGVKPipelineLayoutDescriptor {
    const WGVKChainedStruct* nextInChain;
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

typedef struct WGVKConstantEntry {
    WGVKChainedStruct* nextInChain;
    WGVKStringView key;
    double value;
} WGVKConstantEntry;
typedef struct VertexAttribute {
    WGVKChainedStruct* nextInChain;
    VertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}VertexAttribute;

typedef struct WGVKVertexBufferLayout {
    WGVKChainedStruct* nextInChain;
    VertexStepMode stepMode;
    uint64_t arrayStride;
    size_t attributeCount;
    const VertexAttribute* attributes;
} WGVKVertexBufferLayout;

typedef struct WGVKVertexState {
    WGVKChainedStruct* nextInChain;
    WGVKShaderModule module;
    WGVKStringView entryPoint;
    size_t constantCount;
    const WGVKConstantEntry* constants;
    size_t bufferCount;
    const WGVKVertexBufferLayout* buffers;
} WGVKVertexState;
typedef struct WGVKBlendComponent {
    BlendOperation operation;
    BlendFactor srcFactor;
    BlendFactor dstFactor;
    #ifdef __cplusplus
    constexpr bool operator==(const WGVKBlendComponent& other)const noexcept{
        return operation == other.operation && srcFactor == other.srcFactor && dstFactor == other.dstFactor;
    }
    #endif
} WGVKBlendComponent;

typedef struct WGVKBlendState {
    WGVKBlendComponent color;
    WGVKBlendComponent alpha;
    #ifdef __cplusplus
    constexpr bool operator==(const WGVKBlendState& other)const noexcept{
        return color == other.color && alpha == other.alpha;
    }
    #endif
} WGVKBlendState;



typedef struct WGVKShaderSourceSPIRV {
    WGVKChainedStruct chain;
    uint32_t codeSize;
    const uint32_t* code;
} WGVKShaderSourceSPIRV;

typedef struct WGVKShaderModuleDescriptor {
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
} WGVKShaderModuleDescriptor;

typedef struct WGVKColorTargetState {
    WGVKChainedStruct* nextInChain;
    PixelFormat format;
    const WGVKBlendState* blend;
    //WGVKColorWriteMask writeMask;
} WGVKColorTargetState;

typedef struct WGVKFragmentState {
    WGVKChainedStruct* nextInChain;
    WGVKShaderModule module;
    WGVKStringView entryPoint;
    size_t constantCount;
    const WGVKConstantEntry* constants;
    size_t targetCount;
    const WGVKColorTargetState* targets;
} WGVKFragmentState;

typedef struct WGVKPrimitiveState {
    WGVKChainedStruct* nextInChain;
    PrimitiveType topology;
    IndexFormat stripIndexFormat;
    FrontFace frontFace;
    WGVKCullMode cullMode;
    Bool32 unclippedDepth;
} WGVKPrimitiveState;

typedef struct WGVKStencilFaceState {
    CompareFunction compare;
    WGVKStencilOperation failOp;
    WGVKStencilOperation depthFailOp;
    WGVKStencilOperation passOp;
} WGVKStencilFaceState;

typedef struct WGVkDepthStencilState {
    WGVKChainedStruct* nextInChain;
    PixelFormat format;
    Bool32 depthWriteEnabled;
    CompareFunction depthCompare;
    
    WGVKStencilFaceState stencilFront;
    WGVKStencilFaceState stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
} WGVKDepthStencilState;

typedef struct WGVKMultisampleState {
    WGVKChainedStruct* nextInChain;
    uint32_t count;
    uint32_t mask;
    Bool32 alphaToCoverageEnabled;
} WGVKMultisampleState;

typedef struct WGVKRenderPipelineDescriptor {
    WGVKChainedStruct* nextInChain;
    WGVKStringView label;
    WGVKPipelineLayout layout;
    WGVKVertexState vertex;
    WGVKPrimitiveState primitive;
    const WGVKDepthStencilState* depthStencil;
    WGVKMultisampleState multisample;
    const WGVKFragmentState* fragment;
} WGVKRenderPipelineDescriptor;

typedef struct WGVKSurfaceConfiguration {
    WGVKChainedStruct* nextInChain;
    WGVKDevice device;                // Device that surface belongs to (WPGUDevice or WGVKDevice)
    uint32_t width;                   // Width of the rendering surface
    uint32_t height;                  // Height of the rendering surface
    PixelFormat format;               // Pixel format of the surface
    WGVKCompositeAlphaMode alphaMode; // Composite alpha mode
    PresentMode presentMode;          // Present mode for image presentation
} WGVKSurfaceConfiguration;

typedef void (*WGVKRequestAdapterCallback)(WGVKRequestAdapterStatus status, WGVKAdapter adapter, struct WGVKStringView message, void* userdata1, void* userdata2);
typedef struct WGVKRequestAdapterCallbackInfo {
    WGVKChainedStruct * nextInChain;
    int mode;
    WGVKRequestAdapterCallback callback;
    void* userdata1;
    void* userdata2;
} WGVKRequestAdapterCallbackInfo;

typedef struct WGVKBottomLevelAccelerationStructureDescriptor {
    WGVKBuffer vertexBuffer;          // Buffer containing vertex data
    uint32_t vertexCount;             // Number of vertices
    WGVKBuffer indexBuffer;           // Optional index buffer
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

WGVKInstance wgvkCreateInstance(const WGVKInstanceDescriptor *descriptor);
WGVKFuture wgvkInstanceRequestAdapter(WGVKInstance instance, const WGVKRequestAdapterOptions* options, WGVKRequestAdapterCallbackInfo callbackInfo);
WGVKDevice wgvkAdapterCreateDevice(WGVKAdapter adapter, const WGVKDeviceDescriptor *descriptor);

void wgvkSurfaceGetCapabilities(WGVKSurface wgvkSurface, WGVKAdapter adapter, WGVKSurfaceCapabilities* capabilities);
void wgvkSurfaceConfigure(WGVKSurface surface, const WGVKSurfaceConfiguration* config);
WGVKTexture wgvkDeviceCreateTexture(WGVKDevice device, const WGVKTextureDescriptor* descriptor);
WGVKTextureView wgvkTextureCreateView(WGVKTexture texture, const WGVKTextureViewDescriptor *descriptor);
WGVKSampler wgvkDeviceCreateSampler(WGVKDevice device, const WGVKSamplerDescriptor* descriptor);
WGVKBuffer wgvkDeviceCreateBuffer(WGVKDevice device, const WGVKBufferDescriptor* desc);
void wgvkQueueWriteBuffer(WGVKQueue cSelf, WGVKBuffer buffer, uint64_t bufferOffset, const void* data, size_t size);
void wgvkBufferMap(WGVKBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data);
void wgvkBufferUnmap(WGVKBuffer buffer);
size_t wgvkBufferGetSize(WGVKBuffer buffer);
void wgvkQueueWriteTexture(WGVKQueue queue, WGVKTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGVKTexelCopyBufferLayout const * dataLayout, WGVKExtent3D const * writeSize);
WGVKBindGroupLayout wgvkDeviceCreateBindGroupLayout(WGVKDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount);
WGVKPipelineLayout wgvkDeviceCreatePipelineLayout(WGVKDevice device, const WGVKPipelineLayoutDescriptor* pldesc);
WGVKRenderPipeline wgpuDeviceCreateRenderPipeline(WGVKDevice device, WGVKRenderPipelineDescriptor const * descriptor);

WGVKBindGroup wgvkDeviceCreateBindGroup(WGVKDevice device, const WGVKBindGroupDescriptor* bgdesc);
void wgvkWriteBindGroup(WGVKDevice device, WGVKBindGroup, const WGVKBindGroupDescriptor* bgdesc);


WGVKCommandEncoder wgvkDeviceCreateCommandEncoder(WGVKDevice device, const WGVKCommandEncoderDescriptor* cdesc);
WGVKCommandBuffer wgvkCommandEncoderFinish    (WGVKCommandEncoder commandEncoder);
void wgvkQueueSubmit                          (WGVKQueue queue, size_t commandCount, const WGVKCommandBuffer* buffers);
void wgvkCommandEncoderCopyBufferToBuffer     (WGVKCommandEncoder commandEncoder, WGVKBuffer source, uint64_t sourceOffset, WGVKBuffer destination, uint64_t destinationOffset, uint64_t size);
void wgvkCommandEncoderCopyBufferToTexture    (WGVKCommandEncoder commandEncoder, const WGVKTexelCopyBufferInfo*  source, const WGVKTexelCopyTextureInfo* destination, const WGVKExtent3D* copySize);
void wgvkCommandEncoderCopyTextureToBuffer    (WGVKCommandEncoder commandEncoder, const WGVKTexelCopyTextureInfo* source, const WGVKTexelCopyBufferInfo* destination, const WGVKExtent3D* copySize);
void wgvkCommandEncoderCopyTextureToTexture   (WGVKCommandEncoder commandEncoder, const WGVKTexelCopyTextureInfo* source, const WGVKTexelCopyTextureInfo* destination, const WGVKExtent3D* copySize);
void wgvkRenderpassEncoderDraw                (WGVKRenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
void wgvkRenderpassEncoderDrawIndexed         (WGVKRenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t basevertex, uint32_t firstinstance);
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
void wgvkSamplerAddRef                        (WGVKSampler texture);
void wgvkBufferAddRef                         (WGVKBuffer buffer);
void wgvkBindGroupAddRef                      (WGVKBindGroup bindGroup);
void wgvkBindGroupLayoutAddRef                (WGVKBindGroupLayout bindGroupLayout);
void wgvkPipelineLayoutAddRef                 (WGVKPipelineLayout pipelineLayout);
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
void wgvkSamplerRelease                       (WGVKSampler sampler);


WGVKCommandEncoder wgvkResetCommandBuffer(WGVKCommandBuffer commandEncoder);

void wgvkCommandEncoderTraceRays(WGVKRenderPassEncoder encoder);
#ifdef __cplusplus
} //extern "C"
    #if SUPPORT_WGPU_BACKEND == 1
        constexpr bool operator==(const WGVKBlendComponent& a, const WGVKBlendComponent& b) noexcept{
            return a.operation == b.operation && a.srcFactor == b.srcFactor && a.dstFactor == b.dstFactor;
        }
        constexpr bool operator==(const WGVKBlendState& a, const WGVKBlendState& b) noexcept{
            return a.color == b.color && a.alpha == b.alpha;
        }
    #endif
#endif

#endif // WGVK_H_INCLUDED