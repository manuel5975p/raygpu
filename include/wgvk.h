// clang-format off

#ifndef WGPU_H_INCLUDED
#define WGPU_H_INCLUDED
#include <macros_and_constants.h>
#if SUPPORT_VULKAN_BACKEND == 1
//#include <external/volk.h>
#endif
#ifdef __cplusplus
#include <cstdint>
extern "C"{
#else
#include <stdint.h>
#endif

#define WGPU_NULLABLE
#define WGPU_FUNCTION_ATTRIBUTE
#define VMA_MIN_ALIGNMENT 32

#if SUPPORT_WGPU_BACKEND == 1
typedef struct WGPUBufferImpl WGPUBufferImpl;
typedef struct WGPUTextureImpl WGPUTextureImpl;
typedef struct WGPUTextureViewImpl WGPUTextureViewImpl;


typedef WGPUSurface WGPUSurface;
typedef WGPUBindGroupLayout WGPUBindGroupLayout;
typedef WGPUPipelineLayout WGPUPipelineLayout;
typedef WGPUBindGroup WGPUBindGroup;
typedef WGPUBuffer WGPUBuffer;
typedef WGPUInstance WGPUInstance;
typedef WGPUAdapter WGPUAdapter;
typedef WGPUFuture WGPUFuture;
typedef WGPUDevice WGPUDevice;
typedef WGPUQueue WGPUQueue;
typedef WGPURenderPassEncoder WGPURenderPassEncoder;
typedef WGPUComputePassEncoder WGPUComputePassEncoder;
typedef WGPUCommandBuffer WGPUCommandBuffer;
typedef WGPUCommandEncoder WGPUCommandEncoder;
typedef WGPUTexture WGPUTexture;
typedef WGPUTextureView WGPUTextureView;
typedef WGPUSampler WGPUSampler;
typedef WGPURenderPipeline WGPURenderPipeline;
typedef WGPUComputePipeline WGPUComputePipeline;


typedef WGPUExtent3D WGPUExtent3D;
typedef WGPUSurfaceConfiguration WGPUSurfaceConfiguration;
typedef WGPUSurfaceCapabilities WGPUSurfaceCapabilities;
typedef WGPUStringView WGPUStringView;
typedef WGPUTexelCopyBufferLayout WGPUTexelCopyBufferLayout;
typedef WGPUTexelCopyBufferInfo WGPUTexelCopyBufferInfo;
typedef WGPUOrigin3D WGPUOrigin3D;
typedef WGPUTexelCopyTextureInfo WGPUTexelCopyTextureInfo;
typedef WGPUBindGroupEntry WGPUBindGroupEntry;

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
    /*NULLABLE*/  WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ void* sampler;
    /*NULLABLE*/ WGPUTextureView textureView;
} ResourceDescriptor;

typedef struct DColor{
    double r,g,b,a;
}DColor;
typedef WGPURenderPassColorAttachment WGPURenderPassColorAttachment;
typedef WGPURenderPassDepthStencilAttachment WGPURenderPassDepthStencilAttachment;
typedef WGPURenderPassDescriptor WGPURenderPassDescriptor;
typedef WGPUCommandEncoderDescriptor WGPUCommandEncoderDescriptor;
typedef WGPUExtent3D WGPUExtent3D;
typedef WGPUTextureDescriptor WGPUTextureDescriptor;
typedef WGPUTextureViewDescriptor WGPUTextureViewDescriptor;
typedef WGPUSamplerDescriptor WGPUSamplerDescriptor;
typedef WGPUBufferDescriptor WGPUBufferDescriptor;
typedef WGPUBindGroupDescriptor WGPUBindGroupDescriptor;
typedef WGPUInstanceDescriptor WGPUInstanceDescriptor;
typedef WGPURequestAdapterOptions WGPURequestAdapterOptions;
typedef WGPURequestAdapterCallbackInfo WGPURequestAdapterCallbackInfo;
typedef WGPUDeviceDescriptor WGPUDeviceDescriptor;
typedef WGPUVertexAttribute VertexAttribute;
typedef WGPURenderPipelineDescriptor WGPURenderPipelineDescriptor;
typedef WGPUPipelineLayoutDescriptor WGPUPipelineLayoutDescriptor;
typedef WGPUBlendComponent WGPUBlendComponent;
typedef WGPUBlendState WGPUBlendState;
typedef WGPUFutureWaitInfo WGPUFutureWaitInfo;
typedef WGPUSurfaceDescriptor WGPUSurfaceDescriptor;
typedef WGPUComputePipelineDescriptor WGPUComputePipelineDescriptor;
typedef WGPUWaitStatus WGPUWaitStatus;
typedef void* WGPURaytracingPipeline;
typedef void* WGPURaytracingPassEncoder;
typedef void* WGPUBottomLevelAccelerationStructure;
typedef void* WGPUTopLevelAccelerationStructure;

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

//#ifdef __cplusplus
//#define X(A) extern "C" PFN_vk##A fulk##A;
//#else
//#define X(A) extern PFN_vk##A fulk##A;
//#endif
//RTFunctions
//#undef X

typedef uint64_t WGPUFlags;
typedef uint32_t WGPUBool;
#define WGPU_ENUM_ATTRIBUTE

struct WGPUTextureImpl;
struct WGPUTextureViewImpl;
struct WGPUBufferImpl;
struct WGPUBindGroupImpl;
struct WGPUBindGroupLayoutImpl;
struct WGPUPipelineLayoutImpl;
struct WGPUBufferImpl;
struct WGPUFutureImpl;
struct WGPURenderPassEncoderImpl;
struct WGPUComputePassEncoderImpl;
struct WGPUCommandEncoderImpl;
struct WGPUCommandBufferImpl;
struct WGPUTextureImpl;
struct WGPUTextureViewImpl;
struct WGPUQueueImpl;
struct WGPUInstanceImpl;
struct WGPUAdapterImpl;
struct WGPUDeviceImpl;
struct WGPUSurfaceImpl;
struct WGPUShaderModuleImpl;
struct WGPURenderPipelineImpl;
struct WGPUComputePipelineImpl;
struct WGPUTopLevelAccelerationStructureImpl;
struct WGPUBottomLevelAccelerationStructureImpl;
struct WGPURaytracingPipelineImpl;
struct WGPURaytracingPassEncoderImpl;

typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUFutureImpl* WGPUFuture;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPUComputePassEncoderImpl* WGPUComputePassEncoder;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUFenceImpl* WGPUFence;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPUTopLevelAccelerationStructureImpl* WGPUTopLevelAccelerationStructure;
typedef struct WGPUBottomLevelAccelerationStructureImpl* WGPUBottomLevelAccelerationStructure;
typedef struct WGPURaytracingPipelineImpl* WGPURaytracingPipeline;
typedef struct WGPURaytracingPassEncoderImpl* WGPURaytracingPassEncoder;

typedef enum WGPUShaderStageEnum{
    WGPUShaderStageEnum_Vertex,
    WGPUShaderStageEnum_Fragment,
    WGPUShaderStageEnum_Compute,
    WGPUShaderStageEnum_TessControl,
    WGPUShaderStageEnum_TessEvaluation,
    WGPUShaderStageEnum_Geometry,
    WGPUShaderStageEnum_RayGen,
    WGPUShaderStageEnum_RayGenNV = WGPUShaderStageEnum_RayGen,
    WGPUShaderStageEnum_Intersect,
    WGPUShaderStageEnum_IntersectNV = WGPUShaderStageEnum_Intersect,
    WGPUShaderStageEnum_AnyHit,
    WGPUShaderStageEnum_AnyHitNV = WGPUShaderStageEnum_AnyHit,
    WGPUShaderStageEnum_ClosestHit,
    WGPUShaderStageEnum_ClosestHitNV = WGPUShaderStageEnum_ClosestHit,
    WGPUShaderStageEnum_Miss,
    WGPUShaderStageEnum_MissNV = WGPUShaderStageEnum_Miss,
    WGPUShaderStageEnum_Callable,
    WGPUShaderStageEnum_CallableNV = WGPUShaderStageEnum_Callable,
    WGPUShaderStageEnum_Task,
    WGPUShaderStageEnum_TaskNV = WGPUShaderStageEnum_Task,
    WGPUShaderStageEnum_Mesh,
    WGPUShaderStageEnum_MeshNV = WGPUShaderStageEnum_Mesh,
    WGPUShaderStageEnum_EnumCount,
    WGPUShaderStageEnum_Force32 = 0x7FFFFFFF
}WGPUShaderStageEnum;

typedef WGPUFlags WGPUShaderStage;
const static WGPUShaderStage WGPUShaderStage_None = 0;
const static WGPUShaderStage WGPUShaderStage_Vertex = (((WGPUFlags)1) << WGPUShaderStageEnum_Vertex);
const static WGPUShaderStage WGPUShaderStage_TessControl = (((WGPUFlags)1) << WGPUShaderStageEnum_TessControl);
const static WGPUShaderStage WGPUShaderStage_TessEvaluation = (((WGPUFlags)1) << WGPUShaderStageEnum_TessEvaluation);
const static WGPUShaderStage WGPUShaderStage_Geometry = (((WGPUFlags)1) << WGPUShaderStageEnum_Geometry);
const static WGPUShaderStage WGPUShaderStage_Fragment = (((WGPUFlags)1) << WGPUShaderStageEnum_Fragment);
const static WGPUShaderStage WGPUShaderStage_Compute = (((WGPUFlags)1) << WGPUShaderStageEnum_Compute);
const static WGPUShaderStage WGPUShaderStage_RayGen = (((WGPUFlags)1) << WGPUShaderStageEnum_RayGen);
const static WGPUShaderStage WGPUShaderStage_RayGenNV = (((WGPUFlags)1) << WGPUShaderStageEnum_RayGenNV);
const static WGPUShaderStage WGPUShaderStage_Intersect = (((WGPUFlags)1) << WGPUShaderStageEnum_Intersect);
const static WGPUShaderStage WGPUShaderStage_IntersectNV = (((WGPUFlags)1) << WGPUShaderStageEnum_IntersectNV);
const static WGPUShaderStage WGPUShaderStage_AnyHit = (((WGPUFlags)1) << WGPUShaderStageEnum_AnyHit);
const static WGPUShaderStage WGPUShaderStage_AnyHitNV = (((WGPUFlags)1) << WGPUShaderStageEnum_AnyHitNV);
const static WGPUShaderStage WGPUShaderStage_ClosestHit = (((WGPUFlags)1) << WGPUShaderStageEnum_ClosestHit);
const static WGPUShaderStage WGPUShaderStage_ClosestHitNV = (((WGPUFlags)1) << WGPUShaderStageEnum_ClosestHitNV);
const static WGPUShaderStage WGPUShaderStage_Miss = (((WGPUFlags)1) << WGPUShaderStageEnum_Miss);
const static WGPUShaderStage WGPUShaderStage_MissNV = (((WGPUFlags)1) << WGPUShaderStageEnum_MissNV);
const static WGPUShaderStage WGPUShaderStage_Callable = (((WGPUFlags)1) << WGPUShaderStageEnum_Callable);
const static WGPUShaderStage WGPUShaderStage_CallableNV = (((WGPUFlags)1) << WGPUShaderStageEnum_CallableNV);
const static WGPUShaderStage WGPUShaderStage_Task = (((WGPUFlags)1) << WGPUShaderStageEnum_Task);
const static WGPUShaderStage WGPUShaderStage_TaskNV = (((WGPUFlags)1) << WGPUShaderStageEnum_TaskNV);
const static WGPUShaderStage WGPUShaderStage_Mesh = (((WGPUFlags)1) << WGPUShaderStageEnum_Mesh);
const static WGPUShaderStage WGPUShaderStage_MeshNV = (((WGPUFlags)1) << WGPUShaderStageEnum_MeshNV);
const static WGPUShaderStage WGPUShaderStage_EnumCount = (((WGPUFlags)1) << WGPUShaderStageEnum_EnumCount);

typedef WGPUFlags WGPUTextureUsage;
static const WGPUTextureUsage WGPUTextureUsage_None = 0x0000000000000000;
static const WGPUTextureUsage WGPUTextureUsage_CopySrc = 0x0000000000000001;
static const WGPUTextureUsage WGPUTextureUsage_CopyDst = 0x0000000000000002;
static const WGPUTextureUsage WGPUTextureUsage_TextureBinding = 0x0000000000000004;
static const WGPUTextureUsage WGPUTextureUsage_StorageBinding = 0x0000000000000008;
static const WGPUTextureUsage WGPUTextureUsage_RenderAttachment = 0x0000000000000010;
static const WGPUTextureUsage WGPUTextureUsage_TransientAttachment = 0x0000000000001000;
static const WGPUTextureUsage WGPUTextureUsage_StorageAttachment = 0x0000000000002000;

typedef WGPUFlags WGPUBufferUsage;
static const WGPUBufferUsage WGPUBufferUsage_None = 0x0000000000000000;
static const WGPUBufferUsage WGPUBufferUsage_MapRead = 0x0000000000000001;
static const WGPUBufferUsage WGPUBufferUsage_MapWrite = 0x0000000000000002;
static const WGPUBufferUsage WGPUBufferUsage_CopySrc = 0x0000000000000004;
static const WGPUBufferUsage WGPUBufferUsage_CopyDst = 0x0000000000000008;
static const WGPUBufferUsage WGPUBufferUsage_Index = 0x0000000000000010;
static const WGPUBufferUsage WGPUBufferUsage_Vertex = 0x0000000000000020;
static const WGPUBufferUsage WGPUBufferUsage_Uniform = 0x0000000000000040;
static const WGPUBufferUsage WGPUBufferUsage_Storage = 0x0000000000000080;
static const WGPUBufferUsage WGPUBufferUsage_Indirect = 0x0000000000000100;
static const WGPUBufferUsage WGPUBufferUsage_QueryResolve = 0x0000000000000200;
static const WGPUBufferUsage WGPUBufferUsage_ShaderDeviceAddress = 0x0000000010000000;
static const WGPUBufferUsage WGPUBufferUsage_AccelerationStructureInput = 0x0000000020000000;
static const WGPUBufferUsage WGPUBufferUsage_AccelerationStructureStorage = 0x0000000040000000;
static const WGPUBufferUsage WGPUBufferUsage_ShaderBindingTable = 0x0000000080000000;
typedef enum WGPUWaitStatus {
    WGPUWaitStatus_Success = 0x00000001,
    WGPUWaitStatus_TimedOut = 0x00000002,
    WGPUWaitStatus_Error = 0x00000003,
    WGPUWaitStatus_Force32 = 0x7FFFFFFF
} WGPUWaitStatus;

typedef enum PresentMode{ 
    PresentMode_Undefined = 0x00000000,
    PresentMode_Fifo = 0x00000001,
    PresentMode_FifoRelaxed = 0x00000002,
    PresentMode_Immediate = 0x00000003,
    PresentMode_Mailbox = 0x00000004,
}WGPUPresentMode;

typedef enum WGPUTextureAspect {
    TextureAspect_Undefined = 0x00000000,
    TextureAspect_All = 0x00000001,
    TextureAspect_StencilOnly = 0x00000002,
    TextureAspect_DepthOnly = 0x00000003,
    TextureAspect_Plane0Only = 0x00050000,
    TextureAspect_Plane1Only = 0x00050001,
    TextureAspect_Plane2Only = 0x00050002,
    TextureAspect_Force32 = 0x7FFFFFFF
} WGPUTextureAspect;

typedef enum WGPUPrimitiveTopology {
    WGPUPrimitiveTopology_Undefined = 0x00000000,
    WGPUPrimitiveTopology_PointList = 0x00000001,
    WGPUPrimitiveTopology_LineList = 0x00000002,
    WGPUPrimitiveTopology_LineStrip = 0x00000003,
    WGPUPrimitiveTopology_TriangleList = 0x00000004,
    WGPUPrimitiveTopology_TriangleStrip = 0x00000005,
    WGPUPrimitiveTopology_Force32 = 0x7FFFFFFF
} WGPUPrimitiveTopology WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUSType {
    WGPUSType_ShaderSourceSPIRV = 0x00000001,
    WGPUSType_ShaderSourceWGSL = 0x00000002,
    WGPUSType_SurfaceSourceMetalLayer = 0x00000004,
    WGPUSType_SurfaceSourceWindowsHWND = 0x00000005,
    WGPUSType_SurfaceSourceXlibWindow = 0x00000006,
    WGPUSType_SurfaceSourceWaylandSurface = 0x00000007,
    WGPUSType_SurfaceSourceAndroidNativeWindow = 0x00000008,
    WGPUSType_SurfaceSourceXCBWindow = 0x00000009,
    WGPUSType_SurfaceColorManagement = 0x0000000A,
    WGPUSType_InstanceValidationLayerSelection = 0x10000001
}WGPUSType;

typedef enum WGPU_VK_ImageLayout {
    WGPU_VK_IMAGE_LAYOUT_UNDEFINED = 0,
    WGPU_VK_IMAGE_LAYOUT_GENERAL = 1,
    WGPU_VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
    WGPU_VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
    WGPU_VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    WGPU_VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    WGPU_VK_IMAGE_LAYOUT_PREINITIALIZED = 8,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL = 1000117000,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL = 1000117001,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL = 1000241000,
    WGPU_VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL = 1000241001,
    WGPU_VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL = 1000241002,
    WGPU_VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL = 1000241003,
    WGPU_VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL = 1000314000,
    WGPU_VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL = 1000314001,
    WGPU_VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ = 1000232000,
    WGPU_VK_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR = 1000024000,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR = 1000024001,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR = 1000024002,
    WGPU_VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR = 1000111000,
    WGPU_VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT = 1000218000,
    WGPU_VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR = 1000164003,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR = 1000299000,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR = 1000299001,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR = 1000299002,
    WGPU_VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT = 1000339000,
    WGPU_VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR = 1000553000,
    WGPU_VK_IMAGE_LAYOUT_MAX_ENUM = 0x7FFFFFFF
} WGPU_VK_ImageLayout;

typedef enum WGPUCallbackMode {
    WGPUCallbackMode_WaitAnyOnly = 0x00000001,
    WGPUCallbackMode_AllowProcessEvents = 0x00000002,
    WGPUCallbackMode_AllowSpontaneous = 0x00000003,
    WGPUCallbackMode_Force32 = 0x7FFFFFFF
} WGPUCallbackMode;

typedef struct WGPUStringView{
    const char* data;
    size_t length;
}WGPUStringView;

#define WGPU_ARRAY_LAYER_COUNT_UNDEFINED (UINT32_MAX)
#define WGPU_COPY_STRIDE_UNDEFINED (UINT32_MAX)
#define WGPU_DEPTH_CLEAR_VALUE_UNDEFINED (NAN)
#define WGPU_DEPTH_SLICE_UNDEFINED (UINT32_MAX)
#define WGPU_LIMIT_U32_UNDEFINED (UINT32_MAX)
#define WGPU_LIMIT_U64_UNDEFINED (UINT64_MAX)
#define WGPU_MIP_LEVEL_COUNT_UNDEFINED (UINT32_MAX)
#define WGPU_QUERY_SET_INDEX_UNDEFINED (UINT32_MAX)
#define WGPU_STRLEN (SIZE_MAX)
#define WGPU_WHOLE_MAP_SIZE (SIZE_MAX)
#define WGPU_WHOLE_SIZE (UINT64_MAX)

typedef struct WGPUTexelCopyBufferLayout {
    uint64_t offset;
    uint32_t bytesPerRow;
    uint32_t rowsPerImage;
} WGPUTexelCopyBufferLayout;

typedef enum WGPUCompareFunction {
    WGPUCompareFunction_Undefined = 0x00000000,
    WGPUCompareFunction_Never = 0x00000001,
    WGPUCompareFunction_Less = 0x00000002,
    WGPUCompareFunction_Equal = 0x00000003,
    WGPUCompareFunction_LessEqual = 0x00000004,
    WGPUCompareFunction_Greater = 0x00000005,
    WGPUCompareFunction_NotEqual = 0x00000006,
    WGPUCompareFunction_GreaterEqual = 0x00000007,
    WGPUCompareFunction_Always = 0x00000008
} WGPUCompareFunction;

typedef WGPUFlags WGPUMapMode;
static const WGPUMapMode WGPUMapMode_None = 0x0000000000000000;
static const WGPUMapMode WGPUMapMode_Read = 0x0000000000000001;
static const WGPUMapMode WGPUMapMode_Write = 0x0000000000000002;

typedef enum TextureDimension{
    TextureDimension_Undefined = 0x00000000,
    TextureDimension_1D = 0x00000001,
    TextureDimension_2D = 0x00000002,
    //TextureViewDimension_2DArray = 0x00000003,
    //TextureViewDimension_Cube = 0x00000004,
    //TextureViewDimension_CubeArray = 0x00000005,
    TextureDimension_3D = 0x00000003
}WGPUTextureDimension;



typedef enum WGPUTextureViewDimension{
    WGPUTextureViewDimension_Undefined = 0x00000000,
    WGPUTextureViewDimension_1D = 0x00000001,
    WGPUTextureViewDimension_2D = 0x00000002,
    WGPUTextureViewDimension_2DArray = 0x00000003,
    WGPUTextureViewDimension_Cube = 0x00000004,
    WGPUTextureViewDimension_CubeArray = 0x00000005,
    WGPUTextureViewDimension_3D = 0x00000006,
    WGPUTextureViewDimension_Force32 = 0x7FFFFFFF
}WGPUTextureViewDimension;



typedef enum WGPUCullMode{
    WGPUCullMode_Undefined = 0x00000000,
    WGPUCullMode_None = 0x00000001,
    WGPUCullMode_Front = 0x00000002,
    WGPUCullMode_Back = 0x00000003,
    WGPUCullMode_Force32 = 0x7FFFFFFF
} WGPUCullMode;

typedef enum WGPULoadOp {
    LoadOp_Undefined = 0x00000000,
    LoadOp_Load = 0x00000001,
    LoadOp_Clear = 0x00000002,
    LoadOp_ExpandResolveTexture = 0x00050003,
    LoadOp_Force32 = 0x7FFFFFFF
} WGPULoadOp;

typedef enum WGPUStoreOp {
    StoreOp_Undefined = 0x00000000,
    StoreOp_Store = 0x00000001,
    StoreOp_Discard = 0x00000002,
    StoreOp_Force32 = 0x7FFFFFFF
} WGPUStoreOp;

typedef enum WGPUFrontFace {
    WGPUFrontFace_Undefined = 0x00000000,
    WGPUFrontFace_CCW = 0x00000001,
    WGPUFrontFace_CW = 0x00000002,
    WGPUFrontFace_Force32 = 0x7FFFFFFF
} WGPUFrontFace WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUVertexStepMode { 
    WGPUVertexStepMode_Undefined = 0x0, 
    WGPUVertexStepMode_Vertex = 0x1,
    WGPUVertexStepMode_Instance = 0x2,
    WGPUVertexStepMode_Force32 = 0x7FFFFFFF 
} WGPUVertexStepMode;

typedef enum WGPUIndexFormat {
    WGPUIndexFormat_Undefined = 0x00000000,
    WGPUIndexFormat_Uint16 = 0x00000001,
    WGPUIndexFormat_Uint32 = 0x00000002,
    WGPUIndexFormat_Force32 = 0x7FFFFFFF
} WGPUIndexFormat WGPU_ENUM_ATTRIBUTE;

typedef enum WGPURequestAdapterStatus {
    WGPURequestAdapterStatus_Success = 0x00000001,
    WGPURequestAdapterStatus_CallbackCancelled = 0x00000002,
    WGPURequestAdapterStatus_Unavailable = 0x00000003,
    WGPURequestAdapterStatus_Error = 0x00000004,
    WGPURequestAdapterStatus_Force32 = 0x7FFFFFFF
} WGPURequestAdapterStatus WGPU_ENUM_ATTRIBUTE;


typedef enum WGPUBufferBindingType {
    WGPUBufferBindingType_BindingNotUsed = 0x00000000,
    WGPUBufferBindingType_Undefined = 0x00000001,
    WGPUBufferBindingType_Uniform = 0x00000002,
    WGPUBufferBindingType_Storage = 0x00000003,
    WGPUBufferBindingType_ReadOnlyStorage = 0x00000004,
    WGPUBufferBindingType_Force32 = 0x7FFFFFFF
} WGPUBufferBindingType;

typedef enum WGPUSamplerBindingType {
    WGPUSamplerBindingType_BindingNotUsed = 0x00000000,
    WGPUSamplerBindingType_Undefined = 0x00000001,
    WGPUSamplerBindingType_Filtering = 0x00000002,
    WGPUSamplerBindingType_NonFiltering = 0x00000003,
    WGPUSamplerBindingType_Comparison = 0x00000004,
    WGPUSamplerBindingType_Force32 = 0x7FFFFFFF
} WGPUSamplerBindingType;

typedef enum WGPUStorageTextureAccess {
    WGPUStorageTextureAccess_BindingNotUsed = 0x00000000,
    WGPUStorageTextureAccess_Undefined = 0x00000001,
    WGPUStorageTextureAccess_WriteOnly = 0x00000002,
    WGPUStorageTextureAccess_ReadOnly = 0x00000003,
    WGPUStorageTextureAccess_ReadWrite = 0x00000004,
    WGPUStorageTextureAccess_Force32 = 0x7FFFFFFF
} WGPUStorageTextureAccess;

typedef enum WGPUTextureFormat {
    WGPUTextureFormat_Undefined = 0x00000000,
    WGPUTextureFormat_R8Unorm = 0x00000001,
    WGPUTextureFormat_R8Snorm = 0x00000002,
    WGPUTextureFormat_R8Uint = 0x00000003,
    WGPUTextureFormat_R8Sint = 0x00000004,
    WGPUTextureFormat_R16Uint = 0x00000005,
    WGPUTextureFormat_R16Sint = 0x00000006,
    WGPUTextureFormat_R16Float = 0x00000007,
    WGPUTextureFormat_RG8Unorm = 0x00000008,
    WGPUTextureFormat_RG8Snorm = 0x00000009,
    WGPUTextureFormat_RG8Uint = 0x0000000A,
    WGPUTextureFormat_RG8Sint = 0x0000000B,
    WGPUTextureFormat_R32Float = 0x0000000C,
    WGPUTextureFormat_R32Uint = 0x0000000D,
    WGPUTextureFormat_R32Sint = 0x0000000E,
    WGPUTextureFormat_RG16Uint = 0x0000000F,
    WGPUTextureFormat_RG16Sint = 0x00000010,
    WGPUTextureFormat_RG16Float = 0x00000011,
    WGPUTextureFormat_RGBA8Unorm = 0x00000012,
    WGPUTextureFormat_RGBA8UnormSrgb = 0x00000013,
    WGPUTextureFormat_RGBA8Snorm = 0x00000014,
    WGPUTextureFormat_RGBA8Uint = 0x00000015,
    WGPUTextureFormat_RGBA8Sint = 0x00000016,
    WGPUTextureFormat_BGRA8Unorm = 0x00000017,
    WGPUTextureFormat_BGRA8UnormSrgb = 0x00000018,
    WGPUTextureFormat_RGB10A2Uint = 0x00000019,
    WGPUTextureFormat_RGB10A2Unorm = 0x0000001A,
    WGPUTextureFormat_RG11B10Ufloat = 0x0000001B,
    WGPUTextureFormat_RGB9E5Ufloat = 0x0000001C,
    WGPUTextureFormat_RG32Float = 0x0000001D,
    WGPUTextureFormat_RG32Uint = 0x0000001E,
    WGPUTextureFormat_RG32Sint = 0x0000001F,
    WGPUTextureFormat_RGBA16Uint = 0x00000020,
    WGPUTextureFormat_RGBA16Sint = 0x00000021,
    WGPUTextureFormat_RGBA16Float = 0x00000022,
    WGPUTextureFormat_RGBA32Float = 0x00000023,
    WGPUTextureFormat_RGBA32Uint = 0x00000024,
    WGPUTextureFormat_RGBA32Sint = 0x00000025,
    WGPUTextureFormat_Stencil8 = 0x00000026,
    WGPUTextureFormat_Depth16Unorm = 0x00000027,
    WGPUTextureFormat_Depth24Plus = 0x00000028,
    WGPUTextureFormat_Depth24PlusStencil8 = 0x00000029,
    WGPUTextureFormat_Depth32Float = 0x0000002A,
    WGPUTextureFormat_Depth32FloatStencil8 = 0x0000002B,
    WGPUTextureFormat_BC1RGBAUnorm = 0x0000002C,
    WGPUTextureFormat_BC1RGBAUnormSrgb = 0x0000002D,
    WGPUTextureFormat_BC2RGBAUnorm = 0x0000002E,
    WGPUTextureFormat_BC2RGBAUnormSrgb = 0x0000002F,
    WGPUTextureFormat_BC3RGBAUnorm = 0x00000030,
    WGPUTextureFormat_BC3RGBAUnormSrgb = 0x00000031,
    WGPUTextureFormat_BC4RUnorm = 0x00000032,
    WGPUTextureFormat_BC4RSnorm = 0x00000033,
    WGPUTextureFormat_BC5RGUnorm = 0x00000034,
    WGPUTextureFormat_BC5RGSnorm = 0x00000035,
    WGPUTextureFormat_BC6HRGBUfloat = 0x00000036,
    WGPUTextureFormat_BC6HRGBFloat = 0x00000037,
    WGPUTextureFormat_BC7RGBAUnorm = 0x00000038,
    WGPUTextureFormat_BC7RGBAUnormSrgb = 0x00000039,
    WGPUTextureFormat_ETC2RGB8Unorm = 0x0000003A,
    WGPUTextureFormat_ETC2RGB8UnormSrgb = 0x0000003B,
    WGPUTextureFormat_ETC2RGB8A1Unorm = 0x0000003C,
    WGPUTextureFormat_ETC2RGB8A1UnormSrgb = 0x0000003D,
    WGPUTextureFormat_ETC2RGBA8Unorm = 0x0000003E,
    WGPUTextureFormat_ETC2RGBA8UnormSrgb = 0x0000003F,
    WGPUTextureFormat_EACR11Unorm = 0x00000040,
    WGPUTextureFormat_EACR11Snorm = 0x00000041,
    WGPUTextureFormat_EACRG11Unorm = 0x00000042,
    WGPUTextureFormat_EACRG11Snorm = 0x00000043,
    WGPUTextureFormat_ASTC4x4Unorm = 0x00000044,
    WGPUTextureFormat_ASTC4x4UnormSrgb = 0x00000045,
    WGPUTextureFormat_ASTC5x4Unorm = 0x00000046,
    WGPUTextureFormat_ASTC5x4UnormSrgb = 0x00000047,
    WGPUTextureFormat_ASTC5x5Unorm = 0x00000048,
    WGPUTextureFormat_ASTC5x5UnormSrgb = 0x00000049,
    WGPUTextureFormat_ASTC6x5Unorm = 0x0000004A,
    WGPUTextureFormat_ASTC6x5UnormSrgb = 0x0000004B,
    WGPUTextureFormat_ASTC6x6Unorm = 0x0000004C,
    WGPUTextureFormat_ASTC6x6UnormSrgb = 0x0000004D,
    WGPUTextureFormat_ASTC8x5Unorm = 0x0000004E,
    WGPUTextureFormat_ASTC8x5UnormSrgb = 0x0000004F,
    WGPUTextureFormat_ASTC8x6Unorm = 0x00000050,
    WGPUTextureFormat_ASTC8x6UnormSrgb = 0x00000051,
    WGPUTextureFormat_ASTC8x8Unorm = 0x00000052,
    WGPUTextureFormat_ASTC8x8UnormSrgb = 0x00000053,
    WGPUTextureFormat_ASTC10x5Unorm = 0x00000054,
    WGPUTextureFormat_ASTC10x5UnormSrgb = 0x00000055,
    WGPUTextureFormat_ASTC10x6Unorm = 0x00000056,
    WGPUTextureFormat_ASTC10x6UnormSrgb = 0x00000057,
    WGPUTextureFormat_ASTC10x8Unorm = 0x00000058,
    WGPUTextureFormat_ASTC10x8UnormSrgb = 0x00000059,
    WGPUTextureFormat_ASTC10x10Unorm = 0x0000005A,
    WGPUTextureFormat_ASTC10x10UnormSrgb = 0x0000005B,
    WGPUTextureFormat_ASTC12x10Unorm = 0x0000005C,
    WGPUTextureFormat_ASTC12x10UnormSrgb = 0x0000005D,
    WGPUTextureFormat_ASTC12x12Unorm = 0x0000005E,
    WGPUTextureFormat_ASTC12x12UnormSrgb = 0x0000005F,
    WGPUTextureFormat_Force32 = 0x7FFFFFFF
}WGPUTextureFormat;

typedef enum WGPUTextureSampleType {
    WGPUTextureSampleType_BindingNotUsed = 0x00000000,
    WGPUTextureSampleType_Undefined = 0x00000001,
    WGPUTextureSampleType_Float = 0x00000002,
    WGPUTextureSampleType_UnfilterableFloat = 0x00000003,
    WGPUTextureSampleType_Depth = 0x00000004,
    WGPUTextureSampleType_Sint = 0x00000005,
    WGPUTextureSampleType_Uint = 0x00000006,
    WGPUTextureSampleType_Force32 = 0x7FFFFFFF
} WGPUTextureSampleType;


typedef enum WGPUFilterMode {
    WGPUFilterMode_Undefined = 0x00000000,
    WGPUFilterMode_Nearest = 0x00000001,
    WGPUFilterMode_Linear = 0x00000002,
    WGPUFilterMode_Force32 = 0x7FFFFFFF
} WGPUFilterMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUMipmapFilterMode {
    WGPUMipmapFilterMode_Undefined = 0x00000000,
    WGPUMipmapFilterMode_Nearest = 0x00000001,
    WGPUMipmapFilterMode_Linear = 0x00000002,
    WGPUMipmapFilterMode_Force32 = 0x7FFFFFFF
} WGPUMipmapFilterMode WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUAddressMode {
    WGPUAddressMode_Undefined = 0x00000000,
    WGPUAddressMode_ClampToEdge = 0x00000001,
    WGPUAddressMode_Repeat = 0x00000002,
    WGPUAddressMode_MirrorRepeat = 0x00000003,
    WGPUAddressMode_Force32 = 0x7FFFFFFF
} WGPUAddressMode WGPU_ENUM_ATTRIBUTE;

typedef struct WGPUTexelCopyBufferInfo {
    WGPUTexelCopyBufferLayout layout;
    WGPUBuffer buffer;
} WGPUTexelCopyBufferInfo;

typedef struct WGPUOrigin3D {
    uint32_t x;
    uint32_t y;
    uint32_t z;
} WGPUOrigin3D;

typedef struct WGPUExtent3D {
    uint32_t width;
    uint32_t height;
    uint32_t depthOrArrayLayers;
} WGPUExtent3D;

typedef struct WGPUTexelCopyTextureInfo {
    WGPUTexture texture;
    uint32_t mipLevel;
    WGPUOrigin3D origin;
    WGPUTextureAspect aspect;
} WGPUTexelCopyTextureInfo;

typedef struct WGPUChainedStruct {
    struct WGPUChainedStruct* next;
    WGPUSType sType;
} WGPUChainedStruct;

typedef struct WGPUSurfaceSourceXlibWindow {
    WGPUChainedStruct chain;
    void* display;
    uint64_t window;
} WGPUSurfaceSourceXlibWindow;

typedef struct WGPUSurfaceSourceWaylandSurface {
    WGPUChainedStruct chain;
    void* display;
    void* surface;
} WGPUSurfaceSourceWaylandSurface;

typedef struct WGPUSurfaceDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
} WGPUSurfaceDescriptor;


typedef struct WGPURequestAdapterOptions {
    WGPUChainedStruct * nextInChain;
    int featureLevel;
    int powerPreference;
    Bool32 forceFallbackAdapter;
    int backendType;
    WGPUSurface compatibleSurface;
} WGPURequestAdapterOptions;

typedef struct WGPUInstanceCapabilities {
    WGPUChainedStruct* nextInChain;
    Bool32 timedWaitAnyEnable;
    size_t timedWaitAnyMaxCount;
} WGPUInstanceCapabilities;
typedef struct WGPUInstanceLayerSelection{
    WGPUChainedStruct chain;
    const char* const* instanceLayers;
    uint32_t instanceLayerCount;
}WGPUInstanceLayerSelection;

typedef struct WGPUInstanceDescriptor {
    WGPUChainedStruct* nextInChain;
    WGPUInstanceCapabilities capabilities;
} WGPUInstanceDescriptor;

typedef struct WGPUBindGroupEntry{
    WGPUChainedStruct* nextInChain;
    uint32_t binding;
    WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    WGPUSampler sampler;
    WGPUTextureView textureView;
    WGPUTopLevelAccelerationStructure accelerationStructure;
}WGPUBindGroupEntry;
typedef struct WGPUTextureBindingLayout {
    WGPUChainedStruct * nextInChain;
    WGPUTextureSampleType sampleType;
    WGPUTextureViewDimension viewDimension;
    WGPUBool multisampled;
} WGPUTextureBindingLayout;

typedef struct WGPUSamplerBindingLayout {
    WGPUChainedStruct * nextInChain;
    WGPUSamplerBindingType type;
} WGPUSamplerBindingLayout;

typedef struct WGPUStorageTextureBindingLayout {
    WGPUChainedStruct * nextInChain;
    WGPUStorageTextureAccess access;
    WGPUTextureFormat format;
    WGPUTextureViewDimension viewDimension;
} WGPUStorageTextureBindingLayout;

typedef struct WGPUBufferBindingLayout {
    WGPUChainedStruct * nextInChain;
    WGPUBufferBindingType type;
    WGPUBool hasDynamicOffset;
    uint64_t minBindingSize;
} WGPUBufferBindingLayout;

typedef struct WGPUBindGroupLayoutEntry {
    WGPUChainedStruct * nextInChain;
    uint32_t binding;
    WGPUShaderStage visibility;
    WGPUBufferBindingLayout buffer;
    WGPUSamplerBindingLayout sampler;
    WGPUTextureBindingLayout texture;
    WGPUStorageTextureBindingLayout storageTexture;
    WGPUBool accelerationStructure;
} WGPUBindGroupLayoutEntry;

typedef struct WGPUSamplerDescriptor {
    WGPUChainedStruct * nextInChain;
    WGPUStringView label;
    WGPUAddressMode addressModeU;
    WGPUAddressMode addressModeV;
    WGPUAddressMode addressModeW;
    WGPUFilterMode magFilter;
    WGPUFilterMode minFilter;
    WGPUMipmapFilterMode mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    WGPUCompareFunction compare;
    uint16_t maxAnisotropy;
} WGPUSamplerDescriptor;

typedef struct WGPUFutureWaitInfo {
    WGPUFuture future;
    Bool32 completed;
} WGPUFutureWaitInfo;

typedef enum WGPUSurfaceGetCurrentTextureStatus {
    WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal = 0x00000001,
    WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal = 0x00000002,
    WGPUSurfaceGetCurrentTextureStatus_Timeout = 0x00000003,
    WGPUSurfaceGetCurrentTextureStatus_Outdated = 0x00000004,
    WGPUSurfaceGetCurrentTextureStatus_Lost = 0x00000005,
    WGPUSurfaceGetCurrentTextureStatus_Error = 0x00000006,
    WGPUSurfaceGetCurrentTextureStatus_Force32 = 0x7FFFFFFF
} WGPUSurfaceGetCurrentTextureStatus WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUFeatureLevel {
    WGPUFeatureLevel_Undefined = 0x00000000,
    WGPUFeatureLevel_Compatibility = 0x00000001,
    WGPUFeatureLevel_Core = 0x00000002,
    WGPUFeatureLevel_Force32 = 0x7FFFFFFF
} WGPUFeatureLevel;
typedef enum WGPUFeatureName {
    WGPUFeatureName_DepthClipControl = 0x00000001,
    WGPUFeatureName_Depth32FloatStencil8 = 0x00000002,
    WGPUFeatureName_TimestampQuery = 0x00000003,
    WGPUFeatureName_TextureCompressionBC = 0x00000004,
    WGPUFeatureName_TextureCompressionBCSliced3D = 0x00000005,
    WGPUFeatureName_TextureCompressionETC2 = 0x00000006,
    WGPUFeatureName_TextureCompressionASTC = 0x00000007,
    WGPUFeatureName_TextureCompressionASTCSliced3D = 0x00000008,
    WGPUFeatureName_IndirectFirstInstance = 0x00000009,
    WGPUFeatureName_ShaderF16 = 0x0000000A,
    WGPUFeatureName_RG11B10UfloatRenderable = 0x0000000B,
    WGPUFeatureName_BGRA8UnormStorage = 0x0000000C,
    WGPUFeatureName_Float32Filterable = 0x0000000D,
    WGPUFeatureName_Float32Blendable = 0x0000000E,
    WGPUFeatureName_ClipDistances = 0x0000000F,
    WGPUFeatureName_DualSourceBlending = 0x00000010,
    WGPUFeatureName_Subgroups = 0x00000011,
    WGPUFeatureName_CoreFeaturesAndLimits = 0x00000012,
    WGPUFeatureName_Force32 = 0x7FFFFFFF
} WGPUFeatureName;

typedef enum WGPUMapAsyncStatus {
    WGPUMapAsyncStatus_Success = 0x00000001,
    WGPUMapAsyncStatus_CallbackCancelled = 0x00000002,
    WGPUMapAsyncStatus_Error = 0x00000003,
    WGPUMapAsyncStatus_Aborted = 0x00000004,
    WGPUMapAsyncStatus_Force32 = 0x7FFFFFFF
} WGPUMapAsyncStatus WGPU_ENUM_ATTRIBUTE;

typedef struct WGPULimits {
    WGPUChainedStruct* nextInChain;
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
}WGPULimits;

typedef struct WGPUQueueDescriptor {
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
}WGPUQueueDescriptor;

typedef enum WGPUErrorType {
    WGPUErrorType_NoError = 0x00000001,
    WGPUErrorType_Validation = 0x00000002,
    WGPUErrorType_OutOfMemory = 0x00000003,
    WGPUErrorType_Internal = 0x00000004,
    WGPUErrorType_Unknown = 0x00000005,
    WGPUErrorType_Force32 = 0x7FFFFFFF
} WGPUErrorType WGPU_ENUM_ATTRIBUTE;


typedef enum WGPUDeviceLostReason {
    WGPUDeviceLostReason_Unknown = 0x00000001,
    WGPUDeviceLostReason_Destroyed = 0x00000002,
    WGPUDeviceLostReason_CallbackCancelled = 0x00000003,
    WGPUDeviceLostReason_FailedCreation = 0x00000004,
    WGPUDeviceLostReason_Force32 = 0x7FFFFFFF
} WGPUDeviceLostReason WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUVertexFormat {
    WGPUVertexFormat_Uint8 = 0x00000001,
    WGPUVertexFormat_Uint8x2 = 0x00000002,
    WGPUVertexFormat_Uint8x4 = 0x00000003,
    WGPUVertexFormat_Sint8 = 0x00000004,
    WGPUVertexFormat_Sint8x2 = 0x00000005,
    WGPUVertexFormat_Sint8x4 = 0x00000006,
    WGPUVertexFormat_Unorm8 = 0x00000007,
    WGPUVertexFormat_Unorm8x2 = 0x00000008,
    WGPUVertexFormat_Unorm8x4 = 0x00000009,
    WGPUVertexFormat_Snorm8 = 0x0000000A,
    WGPUVertexFormat_Snorm8x2 = 0x0000000B,
    WGPUVertexFormat_Snorm8x4 = 0x0000000C,
    WGPUVertexFormat_Uint16 = 0x0000000D,
    WGPUVertexFormat_Uint16x2 = 0x0000000E,
    WGPUVertexFormat_Uint16x4 = 0x0000000F,
    WGPUVertexFormat_Sint16 = 0x00000010,
    WGPUVertexFormat_Sint16x2 = 0x00000011,
    WGPUVertexFormat_Sint16x4 = 0x00000012,
    WGPUVertexFormat_Unorm16 = 0x00000013,
    WGPUVertexFormat_Unorm16x2 = 0x00000014,
    WGPUVertexFormat_Unorm16x4 = 0x00000015,
    WGPUVertexFormat_Snorm16 = 0x00000016,
    WGPUVertexFormat_Snorm16x2 = 0x00000017,
    WGPUVertexFormat_Snorm16x4 = 0x00000018,
    WGPUVertexFormat_Float16 = 0x00000019,
    WGPUVertexFormat_Float16x2 = 0x0000001A,
    WGPUVertexFormat_Float16x4 = 0x0000001B,
    WGPUVertexFormat_Float32 = 0x0000001C,
    WGPUVertexFormat_Float32x2 = 0x0000001D,
    WGPUVertexFormat_Float32x3 = 0x0000001E,
    WGPUVertexFormat_Float32x4 = 0x0000001F,
    WGPUVertexFormat_Uint32 = 0x00000020,
    WGPUVertexFormat_Uint32x2 = 0x00000021,
    WGPUVertexFormat_Uint32x3 = 0x00000022,
    WGPUVertexFormat_Uint32x4 = 0x00000023,
    WGPUVertexFormat_Sint32 = 0x00000024,
    WGPUVertexFormat_Sint32x2 = 0x00000025,
    WGPUVertexFormat_Sint32x3 = 0x00000026,
    WGPUVertexFormat_Sint32x4 = 0x00000027,
    WGPUVertexFormat_Unorm10_10_10_2 = 0x00000028,
    WGPUVertexFormat_Unorm8x4BGRA = 0x00000029,
    WGPUVertexFormat_Force32 = 0x7FFFFFFF
} WGPUVertexFormat WGPU_ENUM_ATTRIBUTE;
typedef void (*WGPUDeviceLostCallback)(const WGPUDevice*, WGPUDeviceLostReason, struct WGPUStringView, void*, void*);
typedef void (*WGPUUncapturedErrorCallback)(const WGPUDevice*, WGPUErrorType, struct WGPUStringView, void*, void*);

typedef struct WGPUDeviceLostCallbackInfo {
    WGPUChainedStruct * nextInChain;
    int mode;
    WGPUDeviceLostCallback callback;
    void* userdata1;
    void* userdata2;
} WGPUDeviceLostCallbackInfo;
typedef struct WGPUUncapturedErrorCallbackInfo {
    WGPUChainedStruct * nextInChain;
    WGPUUncapturedErrorCallback callback;
    void* userdata1;
    void* userdata2;
} WGPUUncapturedErrorCallbackInfo;

typedef struct WGPUDeviceDescriptor {
    WGPUChainedStruct * nextInChain;
    WGPUStringView label;
    size_t requiredFeatureCount;
    WGPUFeatureName const * requiredFeatures;
    WGPULimits const * requiredLimits;
    WGPUQueueDescriptor defaultQueue;
    WGPUDeviceLostCallbackInfo deviceLostCallbackInfo;
    WGPUUncapturedErrorCallbackInfo uncapturedErrorCallbackInfo;
} WGPUDeviceDescriptor;

typedef struct WGPUColor {
    double r;
    double g;
    double b;
    double a;
} WGPUColor;

typedef struct WGPURenderPassColorAttachment{
    WGPUChainedStruct* nextInChain;
    WGPUTextureView view;
    WGPUTextureView resolveTarget;
    uint32_t depthSlice;
    WGPULoadOp loadOp;
    WGPUStoreOp storeOp;
    WGPUColor clearValue;
}WGPURenderPassColorAttachment;

typedef struct WGPURenderPassDepthStencilAttachment{
    WGPUChainedStruct* nextInChain;
    WGPUTextureView view;
    WGPULoadOp depthLoadOp;
    WGPUStoreOp depthStoreOp;
    float depthClearValue;
    uint32_t depthReadOnly;
    WGPULoadOp stencilLoadOp;
    WGPUStoreOp stencilStoreOp;
    uint32_t stencilClearValue;
    uint32_t stencilReadOnly;
}WGPURenderPassDepthStencilAttachment;

typedef struct WGPURenderPassDescriptor {
    WGPUChainedStruct * nextInChain;
    WGPUStringView label;
    size_t colorAttachmentCount;
    const WGPURenderPassColorAttachment* colorAttachments;
    /*WGPU_NULLABLE*/ const WGPURenderPassDepthStencilAttachment* depthStencilAttachment;
    /*WGPU_NULLABLE*/ void* occlusionQuerySet;
    /*WGPU_NULLABLE*/ void* timestampWrites;
} WGPURenderPassDescriptor;

typedef struct WGPUCommandEncoderDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    bool recyclable;
}WGPUCommandEncoderDescriptor;

typedef struct Extent3D{
    uint32_t width, height, depthOrArrayLayers;
}Extent3D;

typedef struct WGPUTextureDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUTextureUsage usage;
    WGPUTextureDimension dimension;
    Extent3D size;
    WGPUTextureFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    size_t viewFormatCount;
}WGPUTextureDescriptor;

typedef struct WGPUTextureViewDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUTextureFormat format;
    WGPUTextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    WGPUTextureAspect aspect;
    WGPUTextureUsage usage;
}WGPUTextureViewDescriptor;

typedef struct WGPUBufferDescriptor{
    WGPUBufferUsage usage;
    uint64_t size;
    WGPUBool mappedAtCreation;
}WGPUBufferDescriptor;

typedef void (*WGPUBufferMapCallback)(WGPUMapAsyncStatus status, WGPUStringView message, WGPU_NULLABLE void* userdata1, WGPU_NULLABLE void* userdata2);

typedef struct WGPUBufferMapCallbackInfo {
    WGPUChainedStruct * nextInChain;
    WGPUCallbackMode mode;
    WGPUBufferMapCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
} WGPUBufferMapCallbackInfo;

typedef struct WGPUBindGroupDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUBindGroupLayout layout;
    size_t entryCount;
    const WGPUBindGroupEntry* entries;
}WGPUBindGroupDescriptor;

typedef struct WGPUBindGroupLayoutDescriptor {
    WGPUChainedStruct * nextInChain;
    WGPUStringView label;
    size_t entryCount;
    WGPUBindGroupLayoutEntry const * entries;
} WGPUBindGroupLayoutDescriptor;

typedef struct WGPUPipelineLayoutDescriptor {
    const WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    size_t bindGroupLayoutCount;
    const WGPUBindGroupLayout * bindGroupLayouts;
    uint32_t immediateDataRangeByteSize;
}WGPUPipelineLayoutDescriptor;

typedef struct WGPUSurfaceTexture {
    WGPUChainedStruct * nextInChain;
    WGPUTexture texture;
    WGPUSurfaceGetCurrentTextureStatus status;
} WGPUSurfaceTexture;

typedef struct WGPUSurfaceCapabilities{
    WGPUTextureUsage usages;
    size_t formatCount;
    WGPUTextureFormat const* formats;
    size_t presentModeCount;
    WGPUPresentMode const * presentModes;
}WGPUSurfaceCapabilities;

typedef struct WGPUConstantEntry {
    WGPUChainedStruct* nextInChain;
    WGPUStringView key;
    double value;
} WGPUConstantEntry;

typedef struct WGPUVertexAttribute {
    WGPUChainedStruct* nextInChain;
    WGPUVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}WGPUVertexAttribute;

typedef struct WGPUVertexBufferLayout {
    WGPUChainedStruct* nextInChain;
    WGPUVertexStepMode stepMode;
    uint64_t arrayStride;
    size_t attributeCount;
    const WGPUVertexAttribute* attributes;
} WGPUVertexBufferLayout;

typedef struct WGPUVertexState {
    WGPUChainedStruct* nextInChain;
    WGPUShaderModule module;
    WGPUStringView entryPoint;
    size_t constantCount;
    const WGPUConstantEntry* constants;
    size_t bufferCount;
    const WGPUVertexBufferLayout* buffers;
} WGPUVertexState;
typedef enum WGPUBlendOperation {
    WGPUBlendOperation_Undefined = 0x00000000,
    WGPUBlendOperation_Add = 0x00000001,
    WGPUBlendOperation_Subtract = 0x00000002,
    WGPUBlendOperation_ReverseSubtract = 0x00000003,
    WGPUBlendOperation_Min = 0x00000004,
    WGPUBlendOperation_Max = 0x00000005,
    WGPUBlendOperation_Force32 = 0x7FFFFFFF
} WGPUBlendOperation WGPU_ENUM_ATTRIBUTE;

typedef enum WGPUBlendFactor {
    WGPUBlendFactor_Undefined = 0x00000000,
    WGPUBlendFactor_Zero = 0x00000001,
    WGPUBlendFactor_One = 0x00000002,
    WGPUBlendFactor_Src = 0x00000003,
    WGPUBlendFactor_OneMinusSrc = 0x00000004,
    WGPUBlendFactor_SrcAlpha = 0x00000005,
    WGPUBlendFactor_OneMinusSrcAlpha = 0x00000006,
    WGPUBlendFactor_Dst = 0x00000007,
    WGPUBlendFactor_OneMinusDst = 0x00000008,
    WGPUBlendFactor_DstAlpha = 0x00000009,
    WGPUBlendFactor_OneMinusDstAlpha = 0x0000000A,
    WGPUBlendFactor_SrcAlphaSaturated = 0x0000000B,
    WGPUBlendFactor_Constant = 0x0000000C,
    WGPUBlendFactor_OneMinusConstant = 0x0000000D,
    WGPUBlendFactor_Src1 = 0x0000000E,
    WGPUBlendFactor_OneMinusSrc1 = 0x0000000F,
    WGPUBlendFactor_Src1Alpha = 0x00000010,
    WGPUBlendFactor_OneMinusSrc1Alpha = 0x00000011,
    WGPUBlendFactor_Force32 = 0x7FFFFFFF
} WGPUBlendFactor WGPU_ENUM_ATTRIBUTE;

typedef struct WGPUBlendComponent {
    WGPUBlendOperation operation;
    WGPUBlendFactor srcFactor;
    WGPUBlendFactor dstFactor;
    #ifdef __cplusplus
    constexpr bool operator==(const WGPUBlendComponent& other)const noexcept{
        return operation == other.operation && srcFactor == other.srcFactor && dstFactor == other.dstFactor;
    }
    #endif
} WGPUBlendComponent;

typedef struct WGPUBlendState {
    WGPUBlendComponent color;
    WGPUBlendComponent alpha;
    #ifdef __cplusplus
    constexpr bool operator==(const WGPUBlendState& other)const noexcept{
        return color == other.color && alpha == other.alpha;
    }
    #endif
} WGPUBlendState;




typedef struct WGPUShaderSourceSPIRV {
    WGPUChainedStruct chain;
    uint32_t codeSize;
    uint32_t* code;
} WGPUShaderSourceSPIRV;

typedef struct WGPUShaderSourceWGSL {
    WGPUChainedStruct chain;
    WGPUStringView code;
} WGPUShaderSourceWGSL;

typedef struct WGPUShaderModuleDescriptor {
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
} WGPUShaderModuleDescriptor;

typedef struct WGPUColorTargetState {
    WGPUChainedStruct* nextInChain;
    WGPUTextureFormat format;
    const WGPUBlendState* blend;
    //WGPUColorWriteMask writeMask;
} WGPUColorTargetState;

typedef struct WGPUFragmentState {
    WGPUChainedStruct* nextInChain;
    WGPUShaderModule module;
    WGPUStringView entryPoint;
    size_t constantCount;
    const WGPUConstantEntry* constants;
    size_t targetCount;
    const WGPUColorTargetState* targets;
} WGPUFragmentState;

typedef struct WGPUPrimitiveState {
    WGPUChainedStruct* nextInChain;
    WGPUPrimitiveTopology topology;
    WGPUIndexFormat stripIndexFormat;
    WGPUFrontFace frontFace;
    WGPUCullMode cullMode;
    Bool32 unclippedDepth;
} WGPUPrimitiveState;

typedef enum WGPUStencilOperation {
    WGPUStencilOperation_Undefined = 0x00000000,
    WGPUStencilOperation_Keep = 0x00000001,
    WGPUStencilOperation_Zero = 0x00000002,
    WGPUStencilOperation_Replace = 0x00000003,
    WGPUStencilOperation_Invert = 0x00000004,
    WGPUStencilOperation_IncrementClamp = 0x00000005,
    WGPUStencilOperation_DecrementClamp = 0x00000006,
    WGPUStencilOperation_IncrementWrap = 0x00000007,
    WGPUStencilOperation_DecrementWrap = 0x00000008,
    WGPUStencilOperation_Force32 = 0x7FFFFFFF
} WGPUStencilOperation WGPU_ENUM_ATTRIBUTE;

typedef struct WGPUStencilFaceState {
    WGPUCompareFunction compare;
    WGPUStencilOperation failOp;
    WGPUStencilOperation depthFailOp;
    WGPUStencilOperation passOp;
} WGPUStencilFaceState;

typedef struct WGVkDepthStencilState {
    WGPUChainedStruct* nextInChain;
    WGPUTextureFormat format;
    Bool32 depthWriteEnabled;
    WGPUCompareFunction depthCompare;
    
    WGPUStencilFaceState stencilFront;
    WGPUStencilFaceState stencilBack;
    uint32_t stencilReadMask;
    uint32_t stencilWriteMask;
    int32_t depthBias;
    float depthBiasSlopeScale;
    float depthBiasClamp;
} WGPUDepthStencilState;
typedef struct WGPUBufferBindingInfo {
    WGPUChainedStruct * nextInChain;
    WGPUBufferBindingType type;
    uint64_t minBindingSize;
}WGPUBufferBindingInfo;
typedef struct WGPUSamplerBindingInfo {
    // same as WGPUSamplerBindingLayout
    WGPUChainedStruct * nextInChain;
    WGPUSamplerBindingType type;
}WGPUSamplerBindingInfo;
typedef struct WGPUTextureBindingInfo {
    WGPUChainedStruct * nextInChain;
    WGPUTextureSampleType sampleType;
    WGPUTextureViewDimension viewDimension;
    // no ‘multisampled’
}WGPUTextureBindingInfo;
typedef struct WGPUStorageTextureBindingInfo {
    // same as WGPUStorageTextureBindingLayout
    WGPUChainedStruct* nextInChain;
    WGPUStorageTextureAccess access;
    WGPUTextureFormat format;
    WGPUTextureViewDimension viewDimension;
}WGPUStorageTextureBindingInfo;
typedef struct WGPUGlobalReflectionInfo {
    WGPUStringView name;
    uint32_t bindGroup;
    uint32_t binding;
    WGPUShaderStage visibility;
    WGPUBufferBindingInfo buffer;
    WGPUSamplerBindingInfo sampler;
    WGPUTextureBindingInfo texture;
    WGPUStorageTextureBindingInfo storageTexture;
}WGPUGlobalReflectionInfo;


typedef enum WGPUReflectionComponentType{
    WGPUReflectionComponentType_Invalid,
    WGPUReflectionComponentType_Sint32,
    WGPUReflectionComponentType_Uint32,
    WGPUReflectionComponentType_Float32,
    WGPUReflectionComponentType_Float16
}WGPUReflectionComponentType;

typedef enum WGPUReflectionCompositionType{
    WGPUReflectionCompositionType_Invalid,
    WGPUReflectionCompositionType_Scalar,
    WGPUReflectionCompositionType_Vec2,
    WGPUReflectionCompositionType_Vec3,
    WGPUReflectionCompositionType_Vec4
}WGPUReflectionCompositionType;


typedef struct WGPUReflectionAttribute{
    uint32_t location;
    WGPUReflectionComponentType componentType;
    WGPUReflectionCompositionType compositionType;
}WGPUReflectionAttribute;

typedef struct WGPUAttributeReflectionInfo{
    uint32_t attributeCount;
    WGPUReflectionAttribute* attributes;
}WGPUAttributeReflectionInfo;

typedef enum WGPUReflectionInfoRequestStatus {
    WGPUReflectionInfoRequestStatus_Unused            = 0x00000000,
    WGPUReflectionInfoRequestStatus_Success           = 0x00000001,
    WGPUReflectionInfoRequestStatus_CallbackCancelled = 0x00000002,
    WGPUReflectionInfoRequestStatus_Force32           = 0x7FFFFFFF
}WGPUReflectionInfoRequestStatus;



typedef struct WGPUReflectionInfo {
    WGPUChainedStruct* nextInChain;
    uint32_t globalCount;
    const WGPUGlobalReflectionInfo* globals;
    const WGPUAttributeReflectionInfo* inputAttributes;
    const WGPUAttributeReflectionInfo* outputAttributes;
}WGPUReflectionInfo;
typedef void (*WGPUReflectionInfoCallback)(WGPUReflectionInfoRequestStatus status, WGPUReflectionInfo const* reflectionInfo, void* userdata1, void* userdata2);

typedef struct WGPUReflectionInfoCallbackInfo {
    WGPUChainedStruct* nextInChain;
    WGPUCallbackMode mode;
    WGPUReflectionInfoCallback callback;
    WGPU_NULLABLE void* userdata1;
    WGPU_NULLABLE void* userdata2;
}WGPUReflectionInfoCallbackInfo;

typedef struct WGPUMultisampleState {
    WGPUChainedStruct* nextInChain;
    uint32_t count;
    uint32_t mask;
    Bool32 alphaToCoverageEnabled;
} WGPUMultisampleState;

typedef struct WGPUComputeState {
    WGPUChainedStruct * nextInChain;
    WGPUShaderModule module;
    WGPUStringView entryPoint;
    size_t constantCount;
    WGPUConstantEntry const * constants;
} WGPUComputeState;

typedef struct WGPURenderPipelineDescriptor {
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUPipelineLayout layout;
    WGPUVertexState vertex;
    WGPUPrimitiveState primitive;
    const WGPUDepthStencilState* depthStencil;
    WGPUMultisampleState multisample;
    const WGPUFragmentState* fragment;
} WGPURenderPipelineDescriptor;

typedef struct WGPUComputePipelineDescriptor {
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUPipelineLayout layout;
    WGPUComputeState compute;
} WGPUComputePipelineDescriptor;

typedef enum WGPUCompositeAlphaMode {
    WGPUCompositeAlphaMode_Auto = 0x00000000,
    WGPUCompositeAlphaMode_Opaque = 0x00000001,
    WGPUCompositeAlphaMode_Premultiplied = 0x00000002,
    WGPUCompositeAlphaMode_Unpremultiplied = 0x00000003,
    WGPUCompositeAlphaMode_Inherit = 0x00000004,
    WGPUCompositeAlphaMode_Force32 = 0x7FFFFFFF
} WGPUCompositeAlphaMode WGPU_ENUM_ATTRIBUTE;
typedef struct WGPUSurfaceConfiguration {
    WGPUChainedStruct* nextInChain;
    WGPUDevice device;                // Device that surface belongs to (WPGUDevice or WGPUDevice)
    uint32_t width;                   // Width of the rendering surface
    uint32_t height;                  // Height of the rendering surface
    WGPUTextureFormat format;               // Pixel format of the surface
    WGPUCompositeAlphaMode alphaMode; // Composite alpha mode
    WGPUPresentMode presentMode;          // Present mode for image presentation
} WGPUSurfaceConfiguration;

typedef void (*WGPURequestAdapterCallback)(WGPURequestAdapterStatus status, WGPUAdapter adapter, struct WGPUStringView message, void* userdata1, void* userdata2);
typedef struct WGPURequestAdapterCallbackInfo {
    WGPUChainedStruct * nextInChain;
    int mode;
    WGPURequestAdapterCallback callback;
    void* userdata1;
    void* userdata2;
} WGPURequestAdapterCallbackInfo;

typedef struct WGPUBottomLevelAccelerationStructureDescriptor {
    WGPUBuffer vertexBuffer;          // Buffer containing vertex data
    uint32_t vertexCount;             // Number of vertices
    WGPUBuffer indexBuffer;           // Optional index buffer
    uint32_t indexCount;              // Number of indices
    size_t vertexStride;        // Size of each vertex
}WGPUBottomLevelAccelerationStructureDescriptor;

typedef struct WGPUTopLevelAccelerationStructureDescriptor {
    WGPUBottomLevelAccelerationStructure* bottomLevelAS;       // Array of bottom level acceleration structures
    uint32_t blasCount;                                        // Number of BLAS instances
    void* transformMatrices;                                   // Optional transformation matrices
    uint32_t* instanceCustomIndexes;                           // Optional custom instance indexes
    uint32_t* instanceShaderBindingTableRecordOffsets;         // Optional SBT record offsets
    void* instanceFlags;               // Optional instance flags
}WGPUTopLevelAccelerationStructureDescriptor;
#ifdef __cplusplus
extern "C"{
#endif
void wgpuQueueTransitionLayout                (WGPUQueue cSelf, WGPUTexture texture, WGPU_VK_ImageLayout from, WGPU_VK_ImageLayout to);
void wgpuCommandEncoderTransitionTextureLayout(WGPUCommandEncoder encoder, WGPUTexture texture, WGPU_VK_ImageLayout from, WGPU_VK_ImageLayout to);
WGPUTopLevelAccelerationStructure wgpuDeviceCreateTopLevelAccelerationStructure(WGPUDevice device, const WGPUTopLevelAccelerationStructureDescriptor *descriptor);
WGPUBottomLevelAccelerationStructure wgpuDeviceCreateBottomLevelAccelerationStructure(WGPUDevice device, const WGPUBottomLevelAccelerationStructureDescriptor *descriptor);
#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
extern "C"{
#endif
WGPUInstance wgpuCreateInstance(const WGPUInstanceDescriptor *descriptor);
WGPUWaitStatus wgpuInstanceWaitAny(WGPUInstance instance, size_t futureCount, WGPUFutureWaitInfo* futures, uint64_t timeoutNS);
WGPUFuture wgpuInstanceRequestAdapter(WGPUInstance instance, const WGPURequestAdapterOptions* options, WGPURequestAdapterCallbackInfo callbackInfo);
WGPUSurface wgpuInstanceCreateSurface(WGPUInstance instance, const WGPUSurfaceDescriptor* descriptor);
WGPUDevice wgpuAdapterCreateDevice(WGPUAdapter adapter, const WGPUDeviceDescriptor *descriptor);
WGPUQueue wgpuDeviceGetQueue(WGPUDevice device);
void wgpuSurfaceGetCapabilities(WGPUSurface wgpuSurface, WGPUAdapter adapter, WGPUSurfaceCapabilities* capabilities);
void wgpuSurfaceConfigure(WGPUSurface surface, const WGPUSurfaceConfiguration* config);
WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor);
WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, const WGPUTextureViewDescriptor *descriptor);
WGPUSampler wgpuDeviceCreateSampler(WGPUDevice device, const WGPUSamplerDescriptor* descriptor);
WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, const WGPUBufferDescriptor* desc);
void wgpuQueueWriteBuffer(WGPUQueue cSelf, WGPUBuffer buffer, uint64_t bufferOffset, const void* data, size_t size);
void wgpuBufferMap(WGPUBuffer buffer, WGPUMapMode mapmode, size_t offset, size_t size, void** data);
void wgpuBufferUnmap(WGPUBuffer buffer);

WGPUFuture wgpuBufferMapAsync(WGPUBuffer buffer, WGPUMapMode mode, size_t offset, size_t size, WGPUBufferMapCallbackInfo callbackInfo);
size_t wgpuBufferGetSize(WGPUBuffer buffer);
void wgpuQueueWriteTexture(WGPUQueue queue, WGPUTexelCopyTextureInfo const * destination, const void* data, size_t dataSize, WGPUTexelCopyBufferLayout const * dataLayout, WGPUExtent3D const * writeSize);

WGPUFence wgpuDeviceCreateFence  (WGPUDevice device);
void wgpuFenceWait               (WGPUFence fence, uint64_t timeoutNS);
void wgpuFencesWait              (const WGPUFence* fences, uint32_t fenceCount, uint64_t timeoutNS);
void wgpuFenceAttachCallback     (WGPUFence fence, void(*callback)(void*), void* userdata);
void wgpuFenceAddRef             (WGPUFence fence);
void wgpuFenceRelease            (WGPUFence fence);

WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout  (WGPUDevice device, const WGPUBindGroupLayoutDescriptor* bindGroupLayoutDescriptor);
WGPUShaderModule    wgpuDeviceCreateShaderModule     (WGPUDevice device, const WGPUShaderModuleDescriptor* descriptor);
WGPUPipelineLayout  wgpuDeviceCreatePipelineLayout   (WGPUDevice device, const WGPUPipelineLayoutDescriptor* pldesc);
WGPURenderPipeline  wgpuDeviceCreateRenderPipeline   (WGPUDevice device, const WGPURenderPipelineDescriptor* descriptor);
WGPUComputePipeline wgpuDeviceCreateComputePipeline  (WGPUDevice device, const WGPUComputePipelineDescriptor* descriptor);
WGPUFuture          wgpuShaderModuleGetReflectionInfo(WGPUShaderModule shaderModule, WGPUReflectionInfoCallbackInfo callbackInfo);

WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, const WGPUBindGroupDescriptor* bgdesc);
void wgpuWriteBindGroup(WGPUDevice device, WGPUBindGroup, const WGPUBindGroupDescriptor* bgdesc);


WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, const WGPUCommandEncoderDescriptor* cdesc);
WGPUCommandBuffer wgpuCommandEncoderFinish       (WGPUCommandEncoder commandEncoder);
void wgpuQueueSubmit                             (WGPUQueue queue, size_t commandCount, const WGPUCommandBuffer* buffers);
void wgpuCommandEncoderCopyBufferToBuffer        (WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size);
void wgpuCommandEncoderCopyBufferToTexture       (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyBufferInfo*  source, const WGPUTexelCopyTextureInfo* destination, const WGPUExtent3D* copySize);
void wgpuCommandEncoderCopyTextureToBuffer       (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyBufferInfo* destination, const WGPUExtent3D* copySize);
void wgpuCommandEncoderCopyTextureToTexture      (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyTextureInfo* destination, const WGPUExtent3D* copySize);
void wgpuRenderpassEncoderDraw                   (WGPURenderPassEncoder rpenc, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
void wgpuRenderpassEncoderDrawIndexed            (WGPURenderPassEncoder rpenc, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t basevertex, uint32_t firstinstance);
void wgpuRenderPassEncoderSetBindGroup           (WGPURenderPassEncoder rpenc, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
void wgpuRenderPassEncoderSetPipeline            (WGPURenderPassEncoder rpenc, WGPURenderPipeline renderPipeline);
void wgpuRenderPassEncoderEnd                    (WGPURenderPassEncoder rrpenc);
void wgpuRenderPassEncoderRelease                (WGPURenderPassEncoder rpenc);
void wgpuRenderPassEncoderAddRef                 (WGPURenderPassEncoder rpenc);
void wgpuRenderPassEncoderSetIndexBuffer         (WGPURenderPassEncoder rpe, WGPUBuffer buffer, size_t offset, WGPUIndexFormat indexType);
void wgpuRenderPassEncoderSetVertexBuffer        (WGPURenderPassEncoder rpe, uint32_t binding, WGPUBuffer buffer, size_t offset);
void wgpuRenderPassEncoderDrawIndexedIndirect    (WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
void wgpuRenderPassEncoderDrawIndirect           (WGPURenderPassEncoder renderPassEncoder, WGPUBuffer indirectBuffer, uint64_t indirectOffset) WGPU_FUNCTION_ATTRIBUTE;
void wgpuRenderPassEncoderSetBlendConstant       (WGPURenderPassEncoder renderPassEncoder, WGPUColor const * color) WGPU_FUNCTION_ATTRIBUTE;
void wgpuRenderPassEncoderSetViewport            (WGPURenderPassEncoder renderPassEncoder, float x, float y, float width, float height, float minDepth, float maxDepth) WGPU_FUNCTION_ATTRIBUTE;
void wgpuRenderPassEncoderSetScissorRect         (WGPURenderPassEncoder renderPassEncoder, uint32_t x, uint32_t y, uint32_t width, uint32_t height);



void wgpuComputePassEncoderSetPipeline        (WGPUComputePassEncoder cpe, WGPUComputePipeline computePipeline);
void wgpuComputePassEncoderSetBindGroup       (WGPUComputePassEncoder cpe, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const* dynamicOffsets);
void wgpuRaytracingPassEncoderSetPipeline     (WGPURaytracingPassEncoder cpe, WGPURaytracingPipeline raytracingPipeline);
void wgpuRaytracingPassEncoderSetBindGroup    (WGPURaytracingPassEncoder cpe, uint32_t groupIndex, WGPUBindGroup bindGroup);
void wgpuRaytracingPassEncoderTraceRays       (WGPURaytracingPassEncoder cpe, uint32_t width, uint32_t height, uint32_t depth);

void wgpuComputePassEncoderDispatchWorkgroups (WGPUComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z);
void wgpuComputePassEncoderRelease            (WGPUComputePassEncoder cpenc);

void wgpuSurfaceGetCurrentTexture             (WGPUSurface surface, WGPUSurfaceTexture * surfaceTexture);
void wgpuSurfacePresent                       (WGPUSurface surface);

WGPURaytracingPassEncoder wgpuCommandEncoderBeginRaytracingPass(WGPUCommandEncoder enc);
void wgpuCommandEncoderEndRaytracingPass(WGPURaytracingPassEncoder commandEncoder);
WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(WGPUCommandEncoder enc);
void wgpuComputePassEncoderEnd(WGPUComputePassEncoder commandEncoder);
WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder enc, const WGPURenderPassDescriptor* rpdesc);

void wgpuInstanceAddRef                       (WGPUInstance instance);
void wgpuAdapterAddRef                        (WGPUAdapter adapter);
void wgpuDeviceAddRef                         (WGPUDevice device);
void wgpuQueueAddRef                          (WGPUQueue device);
void wgpuReleaseRaytracingPassEncoder         (WGPURaytracingPassEncoder rtenc);
void wgpuTextureAddRef                        (WGPUTexture texture);
void wgpuTextureViewAddRef                    (WGPUTextureView textureView);
void wgpuSamplerAddRef                        (WGPUSampler texture);
void wgpuBufferAddRef                         (WGPUBuffer buffer);
void wgpuBindGroupAddRef                      (WGPUBindGroup bindGroup);
void wgpuShaderModuleAddRef                   (WGPUShaderModule module);
void wgpuBindGroupLayoutAddRef                (WGPUBindGroupLayout bindGroupLayout);
void wgpuPipelineLayoutAddRef                 (WGPUPipelineLayout pipelineLayout);
void wgpuCommandEncoderRelease                (WGPUCommandEncoder commandBuffer);
void wgpuCommandBufferRelease                 (WGPUCommandBuffer commandBuffer);

void wgpuInstanceRelease                      (WGPUInstance instance);
void wgpuAdapterRelease                       (WGPUAdapter adapter);
void wgpuDeviceRelease                        (WGPUDevice device);
void wgpuQueueRelease                         (WGPUQueue device);
void wgpuComputePassEncoderRelease            (WGPUComputePassEncoder rpenc);
void wgpuComputePipelineRelease               (WGPUComputePipeline pipeline);
void wgpuRenderPipelineRelease                (WGPURenderPipeline pipeline);
void wgpuBufferRelease                        (WGPUBuffer buffer);
void wgpuBindGroupRelease                     (WGPUBindGroup commandBuffer);
void wgpuBindGroupLayoutRelease               (WGPUBindGroupLayout commandBuffer);
void wgpuBindGroupLayoutRelease               (WGPUBindGroupLayout bglayout);
void wgpuPipelineLayoutRelease                (WGPUPipelineLayout layout);
void wgpuTextureRelease                       (WGPUTexture texture);
void wgpuTextureViewRelease                   (WGPUTextureView view);
void wgpuSamplerRelease                       (WGPUSampler sampler);
void wgpuShaderModuleRelease                  (WGPUShaderModule module);

WGPUCommandEncoder wgpuResetCommandBuffer(WGPUCommandBuffer commandEncoder);

void wgpuCommandEncoderTraceRays(WGPURenderPassEncoder encoder);
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
} //extern "C"
    #if SUPPORT_WGPU_BACKEND == 1
        constexpr bool operator==(const WGPUBlendComponent& a, const WGPUBlendComponent& b) noexcept{
            return a.operation == b.operation && a.srcFactor == b.srcFactor && a.dstFactor == b.dstFactor;
        }
        constexpr bool operator==(const WGPUBlendState& a, const WGPUBlendState& b) noexcept{
            return a.color == b.color && a.alpha == b.alpha;
        }
    #endif
#endif

#endif // WGPU_H_INCLUDED
