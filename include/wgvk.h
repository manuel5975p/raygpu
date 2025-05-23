// clang-format off

#ifndef WGPU_H_INCLUDED
#define WGPU_H_INCLUDED
#include <enum_translation.h>
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
#ifdef __cplusplus
#define X(A) extern "C" PFN_vk##A fulk##A;
#else
#define X(A) extern PFN_vk##A fulk##A;
#endif
RTFunctions
#undef X

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
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPUTopLevelAccelerationStructureImpl* WGPUTopLevelAccelerationStructure;
typedef struct WGPUBottomLevelAccelerationStructureImpl* WGPUBottomLevelAccelerationStructure;
typedef struct WGPURaytracingPipelineImpl* WGPURaytracingPipeline;
typedef struct WGPURaytracingPassEncoderImpl* WGPURaytracingPassEncoder;

typedef enum WGPUWaitStatus {
    WGPUWaitStatus_Success = 0x00000001,
    WGPUWaitStatus_TimedOut = 0x00000002,
    WGPUWaitStatus_Error = 0x00000003,
    WGPUWaitStatus_Force32 = 0x7FFFFFFF
} WGPUWaitStatus;



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
    VkImageAspectFlagBits aspect;
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
    VkSampler sampler;
    WGPUTextureView textureView;
}WGPUBindGroupEntry;

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
    /*NULLABLE*/ WGPUSampler sampler;
    /*NULLABLE*/ WGPUTextureView textureView;
    /*NULLABLE*/ WGPUTopLevelAccelerationStructure accelerationStructure;
} ResourceDescriptor;

typedef struct WGPUSamplerDescriptor {
    WGPUChainedStruct * nextInChain;
    WGPUStringView label;
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
} WGPUSamplerDescriptor;
typedef struct WGPUFutureWaitInfo {
    WGPUFuture future;
    Bool32 completed;
} WGPUFutureWaitInfo;

typedef struct DColor{
    double r,g,b,a;
}DColor;
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
} WGPUErrorType;

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

typedef struct WGPURenderPassColorAttachment{
    WGPUChainedStruct* nextInChain;
    WGPUTextureView view;
    WGPUTextureView resolveTarget;
    uint32_t depthSlice;
    LoadOp loadOp;
    StoreOp storeOp;
    DColor clearValue;
}WGPURenderPassColorAttachment;

typedef struct WGPURenderPassDepthStencilAttachment{
    WGPUChainedStruct* nextInChain;
    WGPUTextureView view;
    LoadOp depthLoadOp;
    StoreOp depthStoreOp;
    float depthClearValue;
    uint32_t depthReadOnly;
    LoadOp stencilLoadOp;
    StoreOp stencilStoreOp;
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
    TextureUsage usage;
    TextureDimension dimension;
    Extent3D size;
    PixelFormat format;
    uint32_t mipLevelCount;
    uint32_t sampleCount;
    size_t viewFormatCount;
}WGPUTextureDescriptor;

typedef struct WGPUTextureViewDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    PixelFormat format;
    TextureViewDimension dimension;
    uint32_t baseMipLevel;
    uint32_t mipLevelCount;
    uint32_t baseArrayLayer;
    uint32_t arrayLayerCount;
    TextureAspect aspect;
    TextureUsage usage;
}WGPUTextureViewDescriptor;

typedef struct WGPUBufferDescriptor{
    BufferUsage usage;
    uint64_t size;
}WGPUBufferDescriptor;

typedef struct WGPUBindGroupDescriptor{
    WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    WGPUBindGroupLayout layout;
    size_t entryCount;
    const ResourceDescriptor* entries;
}WGPUBindGroupDescriptor;

typedef struct WGPUPipelineLayoutDescriptor {
    const WGPUChainedStruct* nextInChain;
    WGPUStringView label;
    size_t bindGroupLayoutCount;
    const WGPUBindGroupLayout * bindGroupLayouts;
    uint32_t immediateDataRangeByteSize;
}WGPUPipelineLayoutDescriptor;

typedef struct WGPUSurfaceCapabilities{
    TextureUsage usages;
    size_t formatCount;
    PixelFormat const* formats;
    size_t presentModeCount;
    PresentMode const * presentModes;
}WGPUSurfaceCapabilities;

typedef struct WGPUConstantEntry {
    WGPUChainedStruct* nextInChain;
    WGPUStringView key;
    double value;
} WGPUConstantEntry;

typedef struct VertexAttribute {
    WGPUChainedStruct* nextInChain;
    VertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}VertexAttribute;

typedef struct WGPUVertexBufferLayout {
    WGPUChainedStruct* nextInChain;
    VertexStepMode stepMode;
    uint64_t arrayStride;
    size_t attributeCount;
    const VertexAttribute* attributes;
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
typedef struct WGPUBlendComponent {
    BlendOperation operation;
    BlendFactor srcFactor;
    BlendFactor dstFactor;
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
    PixelFormat format;
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
    PrimitiveType topology;
    IndexFormat stripIndexFormat;
    FrontFace frontFace;
    WGPUCullMode cullMode;
    Bool32 unclippedDepth;
} WGPUPrimitiveState;

typedef struct WGPUStencilFaceState {
    CompareFunction compare;
    WGPUStencilOperation failOp;
    WGPUStencilOperation depthFailOp;
    WGPUStencilOperation passOp;
} WGPUStencilFaceState;

typedef struct WGVkDepthStencilState {
    WGPUChainedStruct* nextInChain;
    PixelFormat format;
    Bool32 depthWriteEnabled;
    CompareFunction depthCompare;
    
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
    
    ShaderStageMask visibility;
    WGPUBufferBindingInfo buffer;
    WGPUSamplerBindingInfo sampler;
    WGPUTextureBindingInfo texture;
    WGPUStorageTextureBindingInfo storageTexture;
}WGPUGlobalReflectionInfo;


typedef enum WGPUReflectionVectorEntryType{
    WGPUReflectionVectorEntryType_Invalid,
    WGPUReflectionVectorEntryType_Sint32,
    WGPUReflectionVectorEntryType_Uint32,
    WGPUReflectionVectorEntryType_Float32
}WGPUReflectionVectorEntryType;

typedef struct WGPUReflectionAttribute{
    uint32_t location;
    WGPUReflectionVectorEntryType entryType;
    uint32_t components;
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
    WGPUComputeState compute;
} WGPUComputePipelineDescriptor;

typedef struct WGPUSurfaceConfiguration {
    WGPUChainedStruct* nextInChain;
    WGPUDevice device;                // Device that surface belongs to (WPGUDevice or WGPUDevice)
    uint32_t width;                   // Width of the rendering surface
    uint32_t height;                  // Height of the rendering surface
    PixelFormat format;               // Pixel format of the surface
    WGPUCompositeAlphaMode alphaMode; // Composite alpha mode
    PresentMode presentMode;          // Present mode for image presentation
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
    VkDeviceSize vertexStride;        // Size of each vertex
}WGPUBottomLevelAccelerationStructureDescriptor;

typedef struct WGPUTopLevelAccelerationStructureDescriptor {
    WGPUBottomLevelAccelerationStructure *bottomLevelAS;     // Array of bottom level acceleration structures
    uint32_t blasCount;                                      // Number of BLAS instances
    VkTransformMatrixKHR *transformMatrices;                 // Optional transformation matrices
    uint32_t *instanceCustomIndexes;                         // Optional custom instance indexes
    uint32_t *instanceShaderBindingTableRecordOffsets;       // Optional SBT record offsets
    VkGeometryInstanceFlagsKHR *instanceFlags;               // Optional instance flags
}WGPUTopLevelAccelerationStructureDescriptor;
#ifdef __cplusplus
extern "C"{
#endif
void wgpuQueueTransitionLayout                (WGPUQueue cSelf, WGPUTexture texture, VkImageLayout from, VkImageLayout to);
void wgpuCommandEncoderTransitionTextureLayout(WGPUCommandEncoder encoder, WGPUTexture texture, VkImageLayout from, VkImageLayout to);
void wgpuRenderPassEncoderBindIndexBuffer     (WGPURenderPassEncoder rpe, WGPUBuffer buffer, VkDeviceSize offset, IndexFormat indexType);
void wgpuRenderPassEncoderBindVertexBuffer    (WGPURenderPassEncoder rpe, uint32_t binding, WGPUBuffer buffer, VkDeviceSize offset);
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

void wgpuSurfaceGetCapabilities(WGPUSurface wgpuSurface, WGPUAdapter adapter, WGPUSurfaceCapabilities* capabilities);
void wgpuSurfaceConfigure(WGPUSurface surface, const WGPUSurfaceConfiguration* config);
WGPUTexture wgpuDeviceCreateTexture(WGPUDevice device, const WGPUTextureDescriptor* descriptor);
WGPUTextureView wgpuTextureCreateView(WGPUTexture texture, const WGPUTextureViewDescriptor *descriptor);
WGPUSampler wgpuDeviceCreateSampler(WGPUDevice device, const WGPUSamplerDescriptor* descriptor);
WGPUBuffer wgpuDeviceCreateBuffer(WGPUDevice device, const WGPUBufferDescriptor* desc);
void wgpuQueueWriteBuffer(WGPUQueue cSelf, WGPUBuffer buffer, uint64_t bufferOffset, const void* data, size_t size);
void wgpuBufferMap(WGPUBuffer buffer, MapMode mapmode, size_t offset, size_t size, void** data);
void wgpuBufferUnmap(WGPUBuffer buffer);
size_t wgpuBufferGetSize(WGPUBuffer buffer);
void wgpuQueueWriteTexture(WGPUQueue queue, WGPUTexelCopyTextureInfo const * destination, void const * data, size_t dataSize, WGPUTexelCopyBufferLayout const * dataLayout, WGPUExtent3D const * writeSize);

WGPUBindGroupLayout wgpuDeviceCreateBindGroupLayout(WGPUDevice device, const ResourceTypeDescriptor* entries, uint32_t entryCount);
WGPUShaderModule    wgpuDeviceCreateShaderModule   (WGPUDevice device, WGPUShaderModuleDescriptor const * descriptor);
WGPUPipelineLayout  wgpuDeviceCreatePipelineLayout (WGPUDevice device, const WGPUPipelineLayoutDescriptor* pldesc);
WGPURenderPipeline  wgpuDeviceCreateRenderPipeline (WGPUDevice device, WGPURenderPipelineDescriptor const * descriptor);
WGPURenderPipeline  wgpuDeviceCreateComputePipeline(WGPUDevice device, WGPUComputePipelineDescriptor const * descriptor);

WGPUFuture wgpuShaderModuleGetReflectionInfo(WGPUShaderModule shaderModule, WGPUReflectionInfoCallbackInfo callbackInfo);

WGPUBindGroup wgpuDeviceCreateBindGroup(WGPUDevice device, const WGPUBindGroupDescriptor* bgdesc);
void wgpuWriteBindGroup(WGPUDevice device, WGPUBindGroup, const WGPUBindGroupDescriptor* bgdesc);


WGPUCommandEncoder wgpuDeviceCreateCommandEncoder(WGPUDevice device, const WGPUCommandEncoderDescriptor* cdesc);
WGPUCommandBuffer wgpuCommandEncoderFinish    (WGPUCommandEncoder commandEncoder);
void wgpuQueueSubmit                          (WGPUQueue queue, size_t commandCount, const WGPUCommandBuffer* buffers);
void wgpuCommandEncoderCopyBufferToBuffer     (WGPUCommandEncoder commandEncoder, WGPUBuffer source, uint64_t sourceOffset, WGPUBuffer destination, uint64_t destinationOffset, uint64_t size);
void wgpuCommandEncoderCopyBufferToTexture    (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyBufferInfo*  source, const WGPUTexelCopyTextureInfo* destination, const WGPUExtent3D* copySize);
void wgpuCommandEncoderCopyTextureToBuffer    (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyBufferInfo* destination, const WGPUExtent3D* copySize);
void wgpuCommandEncoderCopyTextureToTexture   (WGPUCommandEncoder commandEncoder, const WGPUTexelCopyTextureInfo* source, const WGPUTexelCopyTextureInfo* destination, const WGPUExtent3D* copySize);
void wgpuRenderpassEncoderDraw                (WGPURenderPassEncoder rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
void wgpuRenderpassEncoderDrawIndexed         (WGPURenderPassEncoder rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, int32_t basevertex, uint32_t firstinstance);
void wgpuRenderPassEncoderSetBindGroup        (WGPURenderPassEncoder rpe, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const * dynamicOffsets);
void wgpuRenderPassEncoderSetPipeline         (WGPURenderPassEncoder rpe, WGPURenderPipeline renderPipeline);
void wgpuComputePassEncoderSetPipeline        (WGPUComputePassEncoder cpe, WGPUComputePipeline computePipeline);
void wgpuComputePassEncoderSetBindGroup       (WGPUComputePassEncoder cpe, uint32_t groupIndex, WGPUBindGroup group, size_t dynamicOffsetCount, uint32_t const* dynamicOffsets);
void wgpuRaytracingPassEncoderSetPipeline     (WGPURaytracingPassEncoder cpe, WGPURaytracingPipeline raytracingPipeline);
void wgpuRaytracingPassEncoderSetBindGroup    (WGPURaytracingPassEncoder cpe, uint32_t groupIndex, WGPUBindGroup bindGroup);
void wgpuRaytracingPassEncoderTraceRays       (WGPURaytracingPassEncoder cpe, uint32_t width, uint32_t height, uint32_t depth);

void wgpuComputePassEncoderDispatchWorkgroups (WGPUComputePassEncoder cpe, uint32_t x, uint32_t y, uint32_t z);
void wgpuReleaseComputePassEncoder            (WGPUComputePassEncoder cpenc);
void wgpuSurfacePresent                       (WGPUSurface surface);

WGPURaytracingPassEncoder wgpuCommandEncoderBeginRaytracingPass(WGPUCommandEncoder enc);
void wgpuCommandEncoderEndRaytracingPass(WGPURaytracingPassEncoder commandEncoder);
WGPUComputePassEncoder wgpuCommandEncoderBeginComputePass(WGPUCommandEncoder enc);
void wgpuCommandEncoderEndComputePass(WGPUComputePassEncoder commandEncoder);
WGPURenderPassEncoder wgpuCommandEncoderBeginRenderPass(WGPUCommandEncoder enc, const WGPURenderPassDescriptor* rpdesc);
void wgpuRenderPassEncoderEnd(WGPURenderPassEncoder renderPassEncoder);

void wgpuReleaseRaytracingPassEncoder         (WGPURaytracingPassEncoder rtenc);
void wgpuTextureAddRef                        (WGPUTexture texture);
void wgpuTextureViewAddRef                    (WGPUTextureView textureView);
void wgpuSamplerAddRef                        (WGPUSampler texture);
void wgpuBufferAddRef                         (WGPUBuffer buffer);
void wgpuBindGroupAddRef                      (WGPUBindGroup bindGroup);
void wgpuShaderModuleAddRef                   (WGPUShaderModule module);
void wgpuBindGroupLayoutAddRef                (WGPUBindGroupLayout bindGroupLayout);
void wgpuPipelineLayoutAddRef                 (WGPUPipelineLayout pipelineLayout);
void wgpuReleaseCommandEncoder                (WGPUCommandEncoder commandBuffer);
void wgpuReleaseCommandBuffer                 (WGPUCommandBuffer commandBuffer);
void wgpuReleaseRenderPassEncoder             (WGPURenderPassEncoder rpenc);
void wgpuReleaseComputePassEncoder            (WGPUComputePassEncoder rpenc);
void wgpuBufferRelease                        (WGPUBuffer commandBuffer);
void wgpuBindGroupRelease                     (WGPUBindGroup commandBuffer);
void wgpuBindGroupLayoutRelease               (WGPUBindGroupLayout commandBuffer);
void wgpuReleaseBindGroupLayout               (WGPUBindGroupLayout bglayout);
void wgpuReleaseTexture                       (WGPUTexture texture);
void wgpuReleaseTextureView                   (WGPUTextureView view);
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