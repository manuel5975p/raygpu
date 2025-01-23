/*
 * MIT License
 * 
 * Copyright (c) 2025 @manuel5975p
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */





#ifndef ENUM_HEADER_H
#define ENUM_HEADER_H
#if SUPPORT_WGPU_BACKEND == 1
#include <webgpu/webgpu.h>
#endif
#if SUPPORT_VULKAN_BACKEND == 1
#include <vulkan/vulkan.h>
#endif
// ------------------------- Enum Definitions -------------------------

typedef enum filterMode {
    filter_nearest = 0x1,
    filter_linear = 0x2,
} filterMode;

typedef enum addressMode {
    clampToEdge = 0x1, 
    repeat = 0x2, 
    mirrorRepeat = 0x3,
} addressMode;

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
} BlendOperation;

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
    VertexStepMode_None = 0x0,
    VertexStepMode_Vertex = 0x1,
    VertexStepMode_Instance = 0x2,
    VertexStepMode_Force32 = 0x7FFFFFFF
} VertexStepMode;

typedef enum BufferUsage {
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
} BufferUsage;

// -------------------- Vulkan Translation Functions --------------------

#if SUPPORT_VULKAN_BACKEND == 1

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
        default:
            return zero; // Default fallback
    }
}

static inline VkFilter toVulkanFilterMode(filterMode fm){
    switch (fm){
        case filter_nearest:
            return VK_FILTER_NEAREST;
        case filter_linear:
            return VK_FILTER_LINEAR;
        default:
            return VK_FILTER_NEAREST; // Default fallback
    }
}

static inline VkSamplerAddressMode toVulkanAddressMode(addressMode am){
    switch (am){
        case clampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case mirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        default:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Default fallback
    }
}

static inline VkCompareOp toVulkanCompareFunction(CompareFunction cf){
    switch (cf){
        case CompareFunction_Never:
            return VK_COMPARE_OP_NEVER;
        case CompareFunction_Less:
            return VK_COMPARE_OP_LESS;
        case CompareFunction_Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareFunction_LessEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareFunction_Greater:
            return VK_COMPARE_OP_GREATER;
        case CompareFunction_NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareFunction_GreaterEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareFunction_Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_ALWAYS; // Default fallback
    }
}

static inline VkBlendFactor toVulkanBlendFactor(BlendFactor bf){
    switch (bf){
        case BlendFactor_Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor_One:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor_Src:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor_OneMinusSrc:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor_SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor_OneMinusSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor_Dst:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor_OneMinusDst:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor_DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor_OneMinusDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor_SrcAlphaSaturated:
            return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendFactor_Constant:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor_OneMinusConstant:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor_Src1:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendFactor_OneMinusSrc1:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendFactor_Src1Alpha:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendFactor_OneMinusSrc1Alpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        default:
            return VK_BLEND_FACTOR_ONE; // Default fallback
    }
}

static inline VkBlendOp toVulkanBlendOperation(BlendOperation bo){
    switch (bo){
        case BlendOperation_Add:
            return VK_BLEND_OP_ADD;
        case BlendOperation_Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOperation_ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOperation_Min:
            return VK_BLEND_OP_MIN;
        case BlendOperation_Max:
            return VK_BLEND_OP_MAX;
        default:
            return VK_BLEND_OP_ADD; // Default fallback
    }
}

static inline VkFilter toVulkanTFilterMode(TFilterMode tfm){
    switch (tfm){
        case TFilterMode_Nearest:
            return VK_FILTER_NEAREST;
        case TFilterMode_Linear:
            return VK_FILTER_LINEAR;
        default:
            return VK_FILTER_NEAREST; // Default fallback
    }
}

static inline VkFrontFace toVulkanFrontFace(FrontFace ff){
    switch (ff){
        case FrontFace_CCW:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case FrontFace_CW:
            return VK_FRONT_FACE_CLOCKWISE;
        default:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE; // Default fallback
    }
}

static inline VkIndexType toVulkanIndexFormat(IndexFormat ifmt){
    switch (ifmt){
        case IndexFormat_Uint16:
            return VK_INDEX_TYPE_UINT16;
        case IndexFormat_Uint32:
            return VK_INDEX_TYPE_UINT32;
        default:
            return VK_INDEX_TYPE_UINT16; // Default fallback
    }
}

static inline VkAttachmentLoadOp toVulkanLoadOperation(LoadOperation lop){
    switch (lop){
        case LoadOperation_Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case LoadOperation_Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case LoadOperation_ExpandResolveTexture:
            // Vulkan does not have a direct equivalent; choose appropriate op or handle separately
            return VK_ATTACHMENT_LOAD_OP_LOAD; // Example fallback
        default:
            return VK_ATTACHMENT_LOAD_OP_LOAD; // Default fallback
    }
}

static inline VkFormat toVulkanVertexFormat(VertexFormat vf){
    switch (vf){
        case VertexFormat_Uint8:
            return VK_FORMAT_R8_UINT;
        case VertexFormat_Uint8x2:
            return VK_FORMAT_R8G8_UINT;
        case VertexFormat_Uint8x4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VertexFormat_Sint8:
            return VK_FORMAT_R8_SINT;
        case VertexFormat_Sint8x2:
            return VK_FORMAT_R8G8_SINT;
        case VertexFormat_Sint8x4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case VertexFormat_Unorm8:
            return VK_FORMAT_R8_UNORM;
        case VertexFormat_Unorm8x2:
            return VK_FORMAT_R8G8_UNORM;
        case VertexFormat_Unorm8x4:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VertexFormat_Snorm8:
            return VK_FORMAT_R8_SNORM;
        case VertexFormat_Snorm8x2:
            return VK_FORMAT_R8G8_SNORM;
        case VertexFormat_Snorm8x4:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case VertexFormat_Uint16:
            return VK_FORMAT_R16_UINT;
        case VertexFormat_Uint16x2:
            return VK_FORMAT_R16G16_UINT;
        case VertexFormat_Uint16x4:
            return VK_FORMAT_R16G16B16A16_UINT;
        case VertexFormat_Sint16:
            return VK_FORMAT_R16_SINT;
        case VertexFormat_Sint16x2:
            return VK_FORMAT_R16G16_SINT;
        case VertexFormat_Sint16x4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VertexFormat_Unorm16:
            return VK_FORMAT_R16_UNORM;
        case VertexFormat_Unorm16x2:
            return VK_FORMAT_R16G16_UNORM;
        case VertexFormat_Unorm16x4:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case VertexFormat_Snorm16:
            return VK_FORMAT_R16_SNORM;
        case VertexFormat_Snorm16x2:
            return VK_FORMAT_R16G16_SNORM;
        case VertexFormat_Snorm16x4:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case VertexFormat_Float16:
            return VK_FORMAT_R16_SFLOAT;
        case VertexFormat_Float16x2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VertexFormat_Float16x4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VertexFormat_Float32:
            return VK_FORMAT_R32_SFLOAT;
        case VertexFormat_Float32x2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat_Float32x3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat_Float32x4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexFormat_Uint32:
            return VK_FORMAT_R32_UINT;
        case VertexFormat_Uint32x2:
            return VK_FORMAT_R32G32_UINT;
        case VertexFormat_Uint32x3:
            return VK_FORMAT_R32G32B32_UINT;
        case VertexFormat_Uint32x4:
            return VK_FORMAT_R32G32B32A32_UINT;
        case VertexFormat_Sint32:
            return VK_FORMAT_R32_SINT;
        case VertexFormat_Sint32x2:
            return VK_FORMAT_R32G32_SINT;
        case VertexFormat_Sint32x3:
            return VK_FORMAT_R32G32B32_SINT;
        case VertexFormat_Sint32x4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VertexFormat_Unorm10_10_10_2:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case VertexFormat_Unorm8x4BGRA:
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            return VK_FORMAT_UNDEFINED; // Default fallback
    }
}

static inline VkVertexInputRate toVulkanVertexStepMode(VertexStepMode vsm){
    switch (vsm){
        case VertexStepMode_Vertex:
            return VK_VERTEX_INPUT_RATE_VERTEX;
        case VertexStepMode_Instance:
            return VK_VERTEX_INPUT_RATE_INSTANCE;
        case VertexStepMode_None:
            // Vulkan does not have a direct equivalent for 'None'; defaulting to Vertex
            return VK_VERTEX_INPUT_RATE_VERTEX;
        default:
            return VK_VERTEX_INPUT_RATE_VERTEX; // Default fallback
    }
}

#endif // SUPPORT_VULKAN_BACKEND

// -------------------- WebGPU Translation Functions --------------------

#if SUPPORT_WGPU_BACKEND == 1

// Translation function for filterMode to WGPUFilterMode
static inline WGPUFilterMode toWebGPUFilterMode(filterMode fm){
    switch (fm){
        case filter_nearest:
            return WGPUFilterMode_Nearest;
        case filter_linear:
            return WGPUFilterMode_Linear;
        default:
            return WGPUFilterMode_Nearest; // Default fallback
    }
}

// Translation function for addressMode to WGPUAddressMode
static inline WGPUAddressMode toWebGPUAddressMode(addressMode am){
    switch (am){
        case clampToEdge:
            return WGPUAddressMode_ClampToEdge;
        case repeat:
            return WGPUAddressMode_Repeat;
        case mirrorRepeat:
            return WGPUAddressMode_MirrorRepeat;
        default:
            return WGPUAddressMode_ClampToEdge; // Default fallback
    }
}

// Translation function for CompareFunction to WGPUCompareFunction
static inline WGPUCompareFunction toWebGPUCompareFunction(CompareFunction cf){
    switch (cf){
        case CompareFunction_Never:
            return WGPUCompareFunction_Never;
        case CompareFunction_Less:
            return WGPUCompareFunction_Less;
        case CompareFunction_Equal:
            return WGPUCompareFunction_Equal;
        case CompareFunction_LessEqual:
            return WGPUCompareFunction_LessEqual;
        case CompareFunction_Greater:
            return WGPUCompareFunction_Greater;
        case CompareFunction_NotEqual:
            return WGPUCompareFunction_NotEqual;
        case CompareFunction_GreaterEqual:
            return WGPUCompareFunction_GreaterEqual;
        case CompareFunction_Always:
            return WGPUCompareFunction_Always;
        default:
            return WGPUCompareFunction_Always; // Default fallback
    }
}

// Translation function for BlendFactor to WGPUBlendFactor
static inline WGPUBlendFactor toWebGPUBlendFactor(BlendFactor bf){
    switch (bf){
        case BlendFactor_Zero:
            return WGPUBlendFactor_Zero;
        case BlendFactor_One:
            return WGPUBlendFactor_One;
        case BlendFactor_Src:
            return WGPUBlendFactor_Src;
        case BlendFactor_OneMinusSrc:
            return WGPUBlendFactor_OneMinusSrc;
        case BlendFactor_SrcAlpha:
            return WGPUBlendFactor_SrcAlpha;
        case BlendFactor_OneMinusSrcAlpha:
            return WGPUBlendFactor_OneMinusSrcAlpha;
        case BlendFactor_Dst:
            return WGPUBlendFactor_Dst;
        case BlendFactor_OneMinusDst:
            return WGPUBlendFactor_OneMinusDst;
        case BlendFactor_DstAlpha:
            return WGPUBlendFactor_DstAlpha;
        case BlendFactor_OneMinusDstAlpha:
            return WGPUBlendFactor_OneMinusDstAlpha;
        case BlendFactor_SrcAlphaSaturated:
            return WGPUBlendFactor_SrcAlphaSaturated;
        case BlendFactor_Constant:
            return WGPUBlendFactor_Constant;
        case BlendFactor_OneMinusConstant:
            return WGPUBlendFactor_OneMinusConstant;
        case BlendFactor_Src1:
            return WGPUBlendFactor_Src1;
        case BlendFactor_OneMinusSrc1:
            return WGPUBlendFactor_OneMinusSrc1;
        case BlendFactor_Src1Alpha:
            return WGPUBlendFactor_Src1Alpha;
        case BlendFactor_OneMinusSrc1Alpha:
            return WGPUBlendFactor_OneMinusSrc1Alpha;
        default:
            return WGPUBlendFactor_One; // Default fallback
    }
}

// Translation function for BlendOperation to WGPUBlendOperation
static inline WGPUBlendOperation toWebGPUBlendOperation(BlendOperation bo){
    switch (bo){
        case BlendOperation_Add:
            return WGPUBlendOperation_Add;
        case BlendOperation_Subtract:
            return WGPUBlendOperation_Subtract;
        case BlendOperation_ReverseSubtract:
            return WGPUBlendOperation_ReverseSubtract;
        case BlendOperation_Min:
            return WGPUBlendOperation_Min;
        case BlendOperation_Max:
            return WGPUBlendOperation_Max;
        default:
            return WGPUBlendOperation_Add; // Default fallback
    }
}

// Translation function for TFilterMode to WGPUFilterMode
static inline WGPUFilterMode toWebGPUTFilterMode(TFilterMode tfm){
    switch (tfm){
        case TFilterMode_Nearest:
            return WGPUFilterMode_Nearest;
        case TFilterMode_Linear:
            return WGPUFilterMode_Linear;
        default:
            return WGPUFilterMode_Nearest; // Default fallback
    }
}

// Translation function for FrontFace to WGPUFrontFace
static inline WGPUFrontFace toWebGPUFrontFace(FrontFace ff){
    switch (ff){
        case FrontFace_CCW:
            return WGPUFrontFace_CCW;
        case FrontFace_CW:
            return WGPUFrontFace_CW;
        default:
            return WGPUFrontFace_CCW; // Default fallback
    }
}

// Translation function for IndexFormat to WGPUIndexFormat
static inline WGPUIndexFormat toWebGPUIndexFormat(IndexFormat ifmt){
    switch (ifmt){
        case IndexFormat_Uint16:
            return WGPUIndexFormat_Uint16;
        case IndexFormat_Uint32:
            return WGPUIndexFormat_Uint32;
        default:
            return WGPUIndexFormat_Uint16; // Default fallback
    }
}

// Translation function for LoadOperation to WGPULoadOp
static inline WGPULoadOp toWebGPULoadOperation(LoadOperation lop){
    switch (lop){
        case LoadOperation_Load:
            return WGPULoadOp_Load;
        case LoadOperation_Clear:
            return WGPULoadOp_Clear;
        case LoadOperation_ExpandResolveTexture:
            // WebGPU does not have a direct equivalent; choose appropriate op or handle separately
            return WGPULoadOp_Load; // Example fallback
        default:
            return WGPULoadOp_Load; // Default fallback
    }
}

// Translation function for VertexFormat to WGPUVertexFormat
static inline WGPUVertexFormat toWebGPUVertexFormat(VertexFormat vf){
    switch (vf){
        case VertexFormat_Uint8:
            return WGPUVertexFormat_Uint8;
        case VertexFormat_Uint8x2:
            return WGPUVertexFormat_Uint8x2;
        case VertexFormat_Uint8x4:
            return WGPUVertexFormat_Uint8x4;
        case VertexFormat_Sint8:
            return WGPUVertexFormat_Sint8;
        case VertexFormat_Sint8x2:
            return WGPUVertexFormat_Sint8x2;
        case VertexFormat_Sint8x4:
            return WGPUVertexFormat_Sint8x4;
        case VertexFormat_Unorm8:
            return WGPUVertexFormat_Unorm8;
        case VertexFormat_Unorm8x2:
            return WGPUVertexFormat_Unorm8x2;
        case VertexFormat_Unorm8x4:
            return WGPUVertexFormat_Unorm8x4;
        case VertexFormat_Snorm8:
            return WGPUVertexFormat_Snorm8;
        case VertexFormat_Snorm8x2:
            return WGPUVertexFormat_Snorm8x2;
        case VertexFormat_Snorm8x4:
            return WGPUVertexFormat_Snorm8x4;
        case VertexFormat_Uint16:
            return WGPUVertexFormat_Uint16;
        case VertexFormat_Uint16x2:
            return WGPUVertexFormat_Uint16x2;
        case VertexFormat_Uint16x4:
            return WGPUVertexFormat_Uint16x4;
        case VertexFormat_Sint16:
            return WGPUVertexFormat_Sint16;
        case VertexFormat_Sint16x2:
            return WGPUVertexFormat_Sint16x2;
        case VertexFormat_Sint16x4:
            return WGPUVertexFormat_Sint16x4;
        case VertexFormat_Unorm16:
            return WGPUVertexFormat_Unorm16;
        case VertexFormat_Unorm16x2:
            return WGPUVertexFormat_Unorm16x2;
        case VertexFormat_Unorm16x4:
            return WGPUVertexFormat_Unorm16x4;
        case VertexFormat_Snorm16:
            return WGPUVertexFormat_Snorm16;
        case VertexFormat_Snorm16x2:
            return WGPUVertexFormat_Snorm16x2;
        case VertexFormat_Snorm16x4:
            return WGPUVertexFormat_Snorm16x4;
        case VertexFormat_Float16:
            return WGPUVertexFormat_Float16;
        case VertexFormat_Float16x2:
            return WGPUVertexFormat_Float16x2;
        case VertexFormat_Float16x4:
            return WGPUVertexFormat_Float16x4;
        case VertexFormat_Float32:
            return WGPUVertexFormat_Float32;
        case VertexFormat_Float32x2:
            return WGPUVertexFormat_Float32x2;
        case VertexFormat_Float32x3:
            return WGPUVertexFormat_Float32x3;
        case VertexFormat_Float32x4:
            return WGPUVertexFormat_Float32x4;
        case VertexFormat_Uint32:
            return WGPUVertexFormat_Uint32;
        case VertexFormat_Uint32x2:
            return WGPUVertexFormat_Uint32x2;
        case VertexFormat_Uint32x3:
            return WGPUVertexFormat_Uint32x3;
        case VertexFormat_Uint32x4:
            return WGPUVertexFormat_Uint32x4;
        case VertexFormat_Sint32:
            return WGPUVertexFormat_Sint32;
        case VertexFormat_Sint32x2:
            return WGPUVertexFormat_Sint32x2;
        case VertexFormat_Sint32x3:
            return WGPUVertexFormat_Sint32x3;
        case VertexFormat_Sint32x4:
            return WGPUVertexFormat_Sint32x4;
        case VertexFormat_Unorm10_10_10_2:
            return WGPUVertexFormat_Unorm10x10x10x2;
        case VertexFormat_Unorm8x4BGRA:
            return WGPUVertexFormat_Unorm8x8x8x8_BGRA;
        default:
            return WGPUVertexFormat_Undefined; // Default fallback
    }
}

// Translation function for VertexStepMode to WGPUVertexStepMode
static inline WGPUVertexStepMode toWebGPUVertexStepMode(VertexStepMode vsm){
    switch (vsm){
        case VertexStepMode_Vertex:
            return WGPUVertexStepMode_Vertex;
        case VertexStepMode_Instance:
            return WGPUVertexStepMode_Instance;
        case VertexStepMode_None:
            // WebGPU does not have a direct equivalent for 'None'; defaulting to Vertex
            return WGPUVertexStepMode_Vertex;
        default:
            return WGPUVertexStepMode_Vertex; // Default fallback
    }
}

#endif // SUPPORT_WGPU_BACKEND

#endif // ENUM_HEADER_H
