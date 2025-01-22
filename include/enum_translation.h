#ifndef ENUM_HEADER_H
#define ENUM_HEADER_H
#include <webgpu/webgpu.h>
#include <vulkan/vulkan.h>
typedef enum CompareFunction{
    CompareFunction_Undefined = 0x00000000,
    CompareFunction_Never = 0x00000001,
    CompareFunction_Less = 0x00000002,
    CompareFunction_Equal = 0x00000003,
    CompareFunction_LessEqual = 0x00000004,
    CompareFunction_Greater = 0x00000005,
    CompareFunction_NotEqual = 0x00000006,
    CompareFunction_GreaterEqual = 0x00000007,
    CompareFunction_Always = 0x00000008
}CompareFunction;

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
    BlendFactor_Force32 = 0x7FFFFFFF
} BlendFactor;


typedef enum BlendOperation {
    BlendOperation_Undefined = 0x00000000,
    BlendOperation_Add = 0x00000001,
    BlendOperation_Subtract = 0x00000002,
    BlendOperation_ReverseSubtract = 0x00000003,
    BlendOperation_Min = 0x00000004,
    BlendOperation_Max = 0x00000005,
    BlendOperation_Force32 = 0x7FFFFFFF
}BlendOperation;
typedef enum TFilterMode {
    TFilterMode_Undefined = 0x00000000,
    TFilterMode_Nearest = 0x00000001,
    TFilterMode_Linear = 0x00000002,
    TFilterMode_Force32 = 0x7FFFFFFF
} TFilterMode;
typedef enum FrontFace {
    FrontFace_Undefined = 0x00000000,
    FrontFace_CCW = 0x00000001,
    FrontFace_CW = 0x00000002,
    FrontFace_Force32 = 0x7FFFFFFF
} FrontFace;
typedef enum IndexFormat {
    IndexFormat_Undefined = 0x00000000,
    IndexFormat_Uint16 = 0x00000001,
    IndexFormat_Uint32 = 0x00000002,
    IndexFormat_Force32 = 0x7FFFFFFF
} IndexFormat;

typedef enum LoadOperation {
    LoadOperation_Undefined = 0x00000000,
    LoadOperation_Load = 0x00000001,
    LoadOperation_Clear = 0x00000002,
    LoadOperation_ExpandResolveTexture = 0x00050003,
    LoadOperation_Force32 = 0x7FFFFFFF
} LoadOperation;
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

typedef enum VertexStepMode {
    VertexStepMode_None = WGPUVertexStepMode_Undefined,
    VertexStepMode_Vertex = WGPUVertexStepMode_Vertex,
    VertexStepMode_Instance = WGPUVertexStepMode_Instance,
    VertexStepMode_Force32 = 0x7FFFFFFF
} VertexStepMode;

typedef enum BufferUsage{
    BufferUsage_None = 0x0000000000000000,
    BufferUsage_MapRead = 0x0000000000000001,
    BufferUsage_MapWrite = 0x0000000000000002,
    BufferUsage_CopySrc = 0x0000000000000004,
    BufferUsage_CopyDst = 0x0000000000000008,
    BufferUsage_Index = 0x0000000000000010,
    BufferUsage_Vertex = 0x0000000000000020,
    BufferUsage_Uniform = 0x0000000000000040,
    BufferUsage_Storage = 0x0000000000000080,
    BufferUsage_Indirect = 0x0000000000000100,
    BufferUsage_QueryResolve = 0x0000000000000200,
}BufferUsage;

static inline VkBufferUsageFlagBits toVulkanBufferUsage(BufferUsage busg){
    const VkBufferUsageFlagBits zero = (VkBufferUsageFlagBits)0u;
    switch (busg){
        case BufferUsage_CopySrc:
        return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        case BufferUsage_CopyDst:
        return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        case BufferUsage_Vertex:
        return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferUsage_Index:
        return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferUsage_Uniform:
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        case BufferUsage_Storage:
        return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        case BufferUsage_MapRead:
        case BufferUsage_MapWrite:
        case BufferUsage_QueryResolve:
        case BufferUsage_None:
        case BufferUsage_Indirect:
        return zero;
        
        //default:
    }
}
#endif
