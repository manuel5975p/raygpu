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

//#define SUPPORT_WGPU_BACKEND 1
//#define SUPPORT_VULKAN_BACKEND 1
#ifdef __cplusplus
#include <cassert>
#include <cstdint>

using std::uint32_t;
using std::uint64_t;
#else
#include <assert.h>
#include <stdint.h>
#endif

#include <macros_and_constants.h>
#if SUPPORT_WGPU_BACKEND == 1
#include <webgpu/webgpu.h>
#endif
#if SUPPORT_VULKAN_BACKEND == 1
#define VK_NO_PROTOTYPES
#include <wgvk.h>
#include <vulkan/vulkan_core.h>
#endif
// ------------------------- Enum Definitions -------------------------

#if SUPPORT_VULKAN_BACKEND == 1




#else 

#define VertexFormat_Uint8 WGPUVertexFormat_Uint8
#define VertexFormat_Uint8x2 WGPUVertexFormat_Uint8x2
#define VertexFormat_Uint8x4 WGPUVertexFormat_Uint8x4
#define VertexFormat_Sint8 WGPUVertexFormat_Sint8
#define VertexFormat_Sint8x2 WGPUVertexFormat_Sint8x2
#define VertexFormat_Sint8x4 WGPUVertexFormat_Sint8x4
#define VertexFormat_Unorm8 WGPUVertexFormat_Unorm8
#define VertexFormat_Unorm8x2 WGPUVertexFormat_Unorm8x2
#define VertexFormat_Unorm8x4 WGPUVertexFormat_Unorm8x4
#define VertexFormat_Snorm8 WGPUVertexFormat_Snorm8
#define VertexFormat_Snorm8x2 WGPUVertexFormat_Snorm8x2
#define VertexFormat_Snorm8x4 WGPUVertexFormat_Snorm8x4
#define VertexFormat_Uint16 WGPUVertexFormat_Uint16
#define VertexFormat_Uint16x2 WGPUVertexFormat_Uint16x2
#define VertexFormat_Uint16x4 WGPUVertexFormat_Uint16x4
#define VertexFormat_Sint16 WGPUVertexFormat_Sint16
#define VertexFormat_Sint16x2 WGPUVertexFormat_Sint16x2
#define VertexFormat_Sint16x4 WGPUVertexFormat_Sint16x4
#define VertexFormat_Unorm16 WGPUVertexFormat_Unorm16
#define VertexFormat_Unorm16x2 WGPUVertexFormat_Unorm16x2
#define VertexFormat_Unorm16x4 WGPUVertexFormat_Unorm16x4
#define VertexFormat_Snorm16 WGPUVertexFormat_Snorm16
#define VertexFormat_Snorm16x2 WGPUVertexFormat_Snorm16x2
#define VertexFormat_Snorm16x4 WGPUVertexFormat_Snorm16x4
#define VertexFormat_Float16 WGPUVertexFormat_Float16
#define VertexFormat_Float16x2 WGPUVertexFormat_Float16x2
#define VertexFormat_Float16x4 WGPUVertexFormat_Float16x4
#define VertexFormat_Float32 WGPUVertexFormat_Float32
#define VertexFormat_Float32x2 WGPUVertexFormat_Float32x2
#define VertexFormat_Float32x3 WGPUVertexFormat_Float32x3
#define VertexFormat_Float32x4 WGPUVertexFormat_Float32x4
#define VertexFormat_Uint32 WGPUVertexFormat_Uint32
#define VertexFormat_Uint32x2 WGPUVertexFormat_Uint32x2
#define VertexFormat_Uint32x3 WGPUVertexFormat_Uint32x3
#define VertexFormat_Uint32x4 WGPUVertexFormat_Uint32x4
#define VertexFormat_Sint32 WGPUVertexFormat_Sint32
#define VertexFormat_Sint32x2 WGPUVertexFormat_Sint32x2
#define VertexFormat_Sint32x3 WGPUVertexFormat_Sint32x3
#define VertexFormat_Sint32x4 WGPUVertexFormat_Sint32x4
#define VertexFormat_Unorm10_10_10_2 WGPUVertexFormat_Unorm10_10_10_2
#define VertexFormat_Unorm8x4BGRA WGPUVertexFormat_Unorm8x4BGRA
#define VertexFormat_Force32 WGPUVertexFormat_Force32
//typedef WGPURequestAdapterStatus WGPURequestAdapterStatus;

#define WGPURequestAdapterStatus_Success WGPURequestAdapterStatus_Success
#define WGPURequestAdapterStatus_InstanceDropped WGPURequestAdapterStatus_InstanceDropped
#define WGPURequestAdapterStatus_Unavailable WGPURequestAdapterStatus_Unavailable
#define WGPURequestAdapterStatus_Error WGPURequestAdapterStatus_Error
#define WGPURequestAdapterStatus_Force32 WGPURequestAdapterStatus_Force32


//typedef WGPUBlendFactor BlendFactor;
#define BlendFactor_Undefined WGPUBlendFactor_Undefined
#define BlendFactor_Zero WGPUBlendFactor_Zero
#define BlendFactor_One WGPUBlendFactor_One
#define BlendFactor_Src WGPUBlendFactor_Src
#define BlendFactor_OneMinusSrc WGPUBlendFactor_OneMinusSrc
#define BlendFactor_SrcAlpha WGPUBlendFactor_SrcAlpha
#define BlendFactor_OneMinusSrcAlpha WGPUBlendFactor_OneMinusSrcAlpha
#define BlendFactor_Dst WGPUBlendFactor_Dst
#define BlendFactor_OneMinusDst WGPUBlendFactor_OneMinusDst
#define BlendFactor_DstAlpha WGPUBlendFactor_DstAlpha
#define BlendFactor_OneMinusDstAlpha WGPUBlendFactor_OneMinusDstAlpha
#define BlendFactor_SrcAlphaSaturated WGPUBlendFactor_SrcAlphaSaturated
#define BlendFactor_Constant WGPUBlendFactor_Constant
#define BlendFactor_OneMinusConstant WGPUBlendFactor_OneMinusConstant
#define BlendFactor_Src1 WGPUBlendFactor_Src1
#define BlendFactor_OneMinusSrc1 WGPUBlendFactor_OneMinusSrc1
#define BlendFactor_Src1Alpha WGPUBlendFactor_Src1Alpha
#define BlendFactor_OneMinusSrc1Alpha WGPUBlendFactor_OneMinusSrc1Alpha

//typedef WGPUBlendOperation BlendOperation;
#define BlendOperation_Undefined WGPUBlendOperation_Undefined
#define BlendOperation_Add WGPUBlendOperation_Add
#define BlendOperation_Subtract WGPUBlendOperation_Subtract
#define BlendOperation_ReverseSubtract WGPUBlendOperation_ReverseSubtract
#define BlendOperation_Min WGPUBlendOperation_Min
#define BlendOperation_Max WGPUBlendOperation_Max

//typedef WGPUCompositeAlphaMode WGPUCompositeAlphaMode;
#define WGPUCompositeAlphaMode_Auto WGPUCompositeAlphaMode_Auto
#define WGPUCompositeAlphaMode_Opaque WGPUCompositeAlphaMode_Opaque
#define WGPUCompositeAlphaMode_Premultiplied WGPUCompositeAlphaMode_Premultiplied
#define WGPUCompositeAlphaMode_Unpremultiplied WGPUCompositeAlphaMode_Unpremultiplied
#define WGPUCompositeAlphaMode_Inherit WGPUCompositeAlphaMode_Inherit
#define WGPUCompositeAlphaMode_Force32 WGPUCompositeAlphaMode_Force32

#endif





// -------------------- Vulkan Translation Functions --------------------

#if SUPPORT_VULKAN_BACKEND == 1

/**
 * @brief Translates custom WGPUTextureUsage flags to Vulkan's VkImageUsageFlags.
 *
 * @param usage The WGPUTextureUsage bitmask to translate.
 * @return VkImageUsageFlags The corresponding Vulkan image usage flags.
 */
static inline VkImageUsageFlags toVulkanWGPUTextureUsage(WGPUTextureUsage usage, PixelFormat format) {
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
        if (format == Depth24 || format == Depth32) { // Assuming Depth24 and Depth32 are defined enum/macro values
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

static inline VkImageAspectFlags toVulkanAspectMask(TextureAspect aspect){
    
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

/**
 * @brief This function does not have an equivalent in WebGPU
 *
 * @param type
 * @return VkDescriptorType
 */
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
static inline VkImageViewType toVulkanTextureViewDimension(TextureViewDimension dim){
    VkImageViewCreateInfo info;
    switch(dim){
        default:
        case 0:{
            rg_unreachable();
        }
        case TextureViewDimension_1D:{
            return VK_IMAGE_VIEW_TYPE_1D;
        }
        case TextureViewDimension_2D:{
            return VK_IMAGE_VIEW_TYPE_2D;
        }
        case TextureViewDimension_3D:{
            return VK_IMAGE_VIEW_TYPE_3D;
        }
        case TextureViewDimension_2DArray:{
            return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }

    }
}
static inline VkImageType toVulkanTextureDimension(TextureDimension dim){
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
static inline VkFilter toVulkanFilterMode(filterMode fm) {
    switch (fm) {
    case filter_nearest:
        return VK_FILTER_NEAREST;
    case filter_linear:
        return VK_FILTER_LINEAR;
    default:
        return VK_FILTER_NEAREST; // Default fallback
    }
}
static inline PixelFormat fromVulkanPixelFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return RGBA8;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return BGRA8;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return BGRA8_Srgb;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return RGBA8_Srgb;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return RGBA16F;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return RGBA32F;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return Depth24;
        case VK_FORMAT_D32_SFLOAT:
            return Depth32;
        default: rg_unreachable();
    }
}
static inline VkPresentModeKHR toVulkanPresentMode(PresentMode mode){
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
static inline PresentMode fromVulkanPresentMode(VkPresentModeKHR mode){
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
            return (PresentMode)~0;
    }
}
static inline VkFormat toVulkanPixelFormat(PixelFormat format) {
    switch (format) {
    case RGBA8:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case RGBA8_Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case BGRA8:
        return VK_FORMAT_B8G8R8A8_UNORM;
    case BGRA8_Srgb:
        return VK_FORMAT_B8G8R8A8_SRGB;
    case RGBA16F:
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RGBA32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Depth24:
        return VK_FORMAT_D24_UNORM_S8_UINT;
    case Depth32:
        return VK_FORMAT_D32_SFLOAT;
    case GRAYSCALE:
        assert(0 && "GRAYSCALE format not supported in Vulkan.");
        break;
    case RGB8:
        assert(0 && "RGB8 format not supported in Vulkan.");
        break;
    default:
        rg_unreachable();
    }
    return (VkFormat)0;
}

static inline VkSamplerAddressMode toVulkanAddressMode(addressMode am) {
    switch (am) {
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

static inline VkShaderStageFlagBits toVulkanShaderStage(WGPUShaderStageEnum stage) {
    switch (stage){
        case ShaderStage_Vertex:         return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage_TessControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage_TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage_Geometry:       return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage_Fragment:       return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage_Compute:        return VK_SHADER_STAGE_COMPUTE_BIT;
        case ShaderStage_RayGen:         return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case ShaderStage_Miss:           return VK_SHADER_STAGE_MISS_BIT_KHR;
        case ShaderStage_ClosestHit:     return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case ShaderStage_AnyHit:         return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case ShaderStage_Intersect:      return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case ShaderStage_Callable:       return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case ShaderStage_Task:           return VK_SHADER_STAGE_TASK_BIT_EXT;
        case ShaderStage_Mesh:           return VK_SHADER_STAGE_MESH_BIT_EXT;
        default: rg_unreachable();
    }
}

static inline VkPipelineStageFlags toVulkanPipelineStage(WGPUShaderStageEnum stage) {
    VkPipelineStageFlags ret = 0;
    if(stage & ShaderStageMask_Vertex){
        ret |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if(stage & ShaderStageMask_TessControl){
        ret |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    }
    if(stage & ShaderStageMask_TessEvaluation){
        ret |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    }
    if(stage & ShaderStageMask_Geometry){
        ret |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    }
    if(stage & ShaderStageMask_Fragment){
        ret |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if(stage & ShaderStageMask_Compute){
        ret |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if(stage & ShaderStageMask_RayGen){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_Miss){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_ClosestHit){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_AnyHit){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_Intersect){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_Callable){
        ret |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
    }
    if(stage & ShaderStageMask_Task){
        ret |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
    }
    if(stage & ShaderStageMask_Mesh){
        ret |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
    }
    return ret;
}

static inline VkShaderStageFlagBits toVulkanShaderStageBits(WGPUShaderStage stageBits) {
    VkShaderStageFlagBits ret = 0;
    //for(unsigned iter = stageBits;iter;iter &= (iter - 1)){
    //    WGPUShaderStage stage = (WGPUShaderStage)(iter & -iter);
    //    switch (stage){
    if(stageBits & ShaderStageMask_Vertex)         ret |= VK_SHADER_STAGE_VERTEX_BIT;
    if(stageBits & ShaderStageMask_TessControl)    ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    if(stageBits & ShaderStageMask_TessEvaluation) ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    if(stageBits & ShaderStageMask_Geometry)       ret |= VK_SHADER_STAGE_GEOMETRY_BIT;
    if(stageBits & ShaderStageMask_Fragment)       ret |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if(stageBits & ShaderStageMask_Compute)        ret |= VK_SHADER_STAGE_COMPUTE_BIT;
    if(stageBits & ShaderStageMask_RayGen)         ret |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    if(stageBits & ShaderStageMask_Miss)           ret |= VK_SHADER_STAGE_MISS_BIT_KHR;
    if(stageBits & ShaderStageMask_ClosestHit)     ret |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    if(stageBits & ShaderStageMask_AnyHit)         ret |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    if(stageBits & ShaderStageMask_Intersect)      ret |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    if(stageBits & ShaderStageMask_Callable)       ret |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    if(stageBits & ShaderStageMask_Task)           ret |= VK_SHADER_STAGE_TASK_BIT_EXT;
    if(stageBits & ShaderStageMask_Mesh)           ret |= VK_SHADER_STAGE_MESH_BIT_EXT;
    //        default: rg_unreachable();
    //    }
    //}
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

static inline VkFilter toVulkanTFilterMode(TFilterMode tfm) {
    switch (tfm) {
    case TFilterMode_Nearest:
        return VK_FILTER_NEAREST;
    case TFilterMode_Linear:
        return VK_FILTER_LINEAR;
    default:
        return VK_FILTER_NEAREST; // Default fallback
    }
}

static inline VkFrontFace toVulkanFrontFace(FrontFace ff) {
    switch (ff) {
    case FrontFace_CCW:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case FrontFace_CW:
        return VK_FRONT_FACE_CLOCKWISE;
    default:
        return VK_FRONT_FACE_COUNTER_CLOCKWISE; // Default fallback
    }
}

static inline VkIndexType toVulkanIndexFormat(IndexFormat ifmt) {
    switch (ifmt) {
    case IndexFormat_Uint16:
        return VK_INDEX_TYPE_UINT16;
    case IndexFormat_Uint32:
        return VK_INDEX_TYPE_UINT32;
    default:
        return VK_INDEX_TYPE_UINT16; // Default fallback
    }
}
static inline VkAttachmentStoreOp toVulkanStoreOperation(StoreOp lop) {
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
static inline VkAttachmentLoadOp toVulkanLoadOperation(LoadOp lop) {
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

static inline VkVertexInputRate toVulkanVertexStepMode(WGPUVertexStepMode vsm) {
    switch (vsm) {
    case WGPUVertexStepMode_Vertex:
        return VK_VERTEX_INPUT_RATE_VERTEX;
    case WGPUVertexStepMode_Instance:
        return VK_VERTEX_INPUT_RATE_INSTANCE;
    case WGPUVertexStepMode_None:
        // Vulkan does not have a direct equivalent for 'None'; defaulting to Vertex
        return VK_VERTEX_INPUT_RATE_VERTEX;
    default:
        return VK_VERTEX_INPUT_RATE_VERTEX; // Default fallback
    }
}

#endif // SUPPORT_VULKAN_BACKEND

// -------------------- WebGPU Translation Functions --------------------

#if SUPPORT_WGPU_BACKEND == 1
/**
 * @brief Translates custom WGPUTextureUsage flags to WebGPU's WGPUTextureUsage flags.
 *
 * @param usage The WGPUTextureUsage bitmask to translate.
 * @return WGPUTextureUsage The corresponding WebGPU texture usage flags.
 */
static inline WGPUTextureUsage toWebGPUTextureUsage(WGPUTextureUsage usage) {
    WGPUTextureUsage wgpuUsage = 0;

    // Array of all possible WGPUTextureUsage flags
    const WGPUTextureUsage allFlags[] = {WGPUTextureUsage_CopySrc, WGPUTextureUsage_CopyDst, WGPUTextureUsage_TextureBinding, WGPUTextureUsage_StorageBinding, WGPUTextureUsage_RenderAttachment, WGPUTextureUsage_TransientAttachment, WGPUTextureUsage_StorageAttachment};
    // Iterate through each dflag and map accordingly
    uint32_t remainigFlags = usage;
    while (remainigFlags != 0) {
        uint32_t flag = remainigFlags & -remainigFlags;
        switch (flag) {
        case WGPUTextureUsage_CopySrc:
            wgpuUsage |= WGPUTextureUsage_CopySrc;
            break;
        case WGPUTextureUsage_CopyDst:
            wgpuUsage |= WGPUTextureUsage_CopyDst;
            break;
        case WGPUTextureUsage_TextureBinding:
            wgpuUsage |= WGPUTextureUsage_TextureBinding;
            break;
        case WGPUTextureUsage_StorageBinding:
            wgpuUsage |= WGPUTextureUsage_StorageBinding;
            break;
        case WGPUTextureUsage_RenderAttachment:
            wgpuUsage |= WGPUTextureUsage_RenderAttachment;
            break;
        //case WGPUTextureUsage_TransientAttachment:
        //    wgpuUsage |= WGPUTextureUsage_TransientAttachment;
        //    break;
        //case WGPUTextureUsage_StorageAttachment:
        //    wgpuUsage |= WGPUTextureUsage_StorageAttachment;
        //    break;
        default:
            break;
        }
        remainigFlags = remainigFlags & (remainigFlags - 1);
    }

    return wgpuUsage;
}
static inline WGPUPrimitiveTopology toWebGPUPrimitive(PrimitiveType pt){
    switch(pt){
        case RL_LINES: return WGPUPrimitiveTopology_LineList;
        case RL_TRIANGLES: return WGPUPrimitiveTopology_TriangleList;
        case RL_TRIANGLE_STRIP: return WGPUPrimitiveTopology_TriangleStrip;
        case RL_POINTS: return WGPUPrimitiveTopology_PointList;
        case RL_QUADS: 
        default:
        rg_unreachable();
    }
}
static inline WGPUBufferUsage toWebGPUBufferUsage(WGPUBufferUsage busg) {
    WGPUBufferUsage usage = 0;

    while (busg != 0) {
        WGPUBufferUsage flag = (busg & (-busg));

        switch (flag) {
        case WGPUBufferUsage_CopySrc:
            usage |= WGPUBufferUsage_CopySrc;
            break;
        case WGPUBufferUsage_CopyDst:
            usage |= WGPUBufferUsage_CopyDst;
            break;
        case WGPUBufferUsage_Vertex:
            usage |= WGPUBufferUsage_Vertex;
            break;
        case WGPUBufferUsage_Index:
            usage |= WGPUBufferUsage_Index;
            break;
        case WGPUBufferUsage_Uniform:
            usage |= WGPUBufferUsage_Uniform;
            break;
        case WGPUBufferUsage_Storage:
            usage |= WGPUBufferUsage_Storage;
            break;
        case WGPUBufferUsage_Indirect:
            usage |= WGPUBufferUsage_Indirect;
            break;
        case WGPUBufferUsage_MapRead:
            usage |= WGPUBufferUsage_MapRead;
            break;
        case WGPUBufferUsage_MapWrite:
            usage |= WGPUBufferUsage_MapWrite;
            break;
        case WGPUBufferUsage_QueryResolve:
            usage |= WGPUBufferUsage_QueryResolve;
            break;
        default:
            break;
        }

        busg &= (busg - 1);
    }

    return usage;
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

// Translation function for filterMode to WGPUFilterMode
static inline WGPUFilterMode toWebGPUFilterMode(filterMode fm) {
    switch (fm) {
    case filter_nearest:
        return WGPUFilterMode_Nearest;
    case filter_linear:
        return WGPUFilterMode_Linear;
    default:
        return WGPUFilterMode_Nearest; // Default fallback
    }
}

// Translation function for addressMode to WGPUAddressMode
static inline WGPUAddressMode toWebGPUAddressMode(addressMode am) {
    switch (am) {
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



// Translation function for BlendOperation to WGPUBlendOperation
static inline WGPUBlendOperation toWebGPUBlendOperation(BlendOperation bo) {
    return bo;
    //switch (bo) {
    //case BlendOperation_Add:
    //    return WGPUBlendOperation_Add;
    //case BlendOperation_Subtract:
    //    return WGPUBlendOperation_Subtract;
    //case BlendOperation_ReverseSubtract:
    //    return WGPUBlendOperation_ReverseSubtract;
    //case BlendOperation_Min:
    //    return WGPUBlendOperation_Min;
    //case BlendOperation_Max:
    //    return WGPUBlendOperation_Max;
    //default:
    //    return WGPUBlendOperation_Add; // Default fallback
    //}
}

// Translation function for TFilterMode to WGPUFilterMode
static inline WGPUFilterMode toWebGPUTFilterMode(TFilterMode tfm) {
    switch (tfm) {
    case TFilterMode_Nearest:
        return WGPUFilterMode_Nearest;
    case TFilterMode_Linear:
        return WGPUFilterMode_Linear;
    default:
        return WGPUFilterMode_Nearest; // Default fallback
    }
}

// Translation function for FrontFace to WGPUFrontFace
static inline WGPUFrontFace toWebGPUFrontFace(FrontFace ff) {
    switch (ff) {
    case FrontFace_CCW:
        return WGPUFrontFace_CCW;
    case FrontFace_CW:
        return WGPUFrontFace_CW;
    default:
        return WGPUFrontFace_CCW; // Default fallback
    }
}

// Translation function for IndexFormat to WGPUIndexFormat
static inline WGPUIndexFormat toWebGPUIndexFormat(IndexFormat ifmt) {
    switch (ifmt) {
    case IndexFormat_Uint16:
        return WGPUIndexFormat_Uint16;
    case IndexFormat_Uint32:
        return WGPUIndexFormat_Uint32;
    default:
        return WGPUIndexFormat_Uint16; // Default fallback
    }
}
static inline WGPUTextureFormat toWGPUTextureFormat(PixelFormat format){
    switch(format){
        case RGBA8:return WGPUTextureFormat_RGBA8Unorm;
        case BGRA8:return WGPUTextureFormat_BGRA8Unorm;
        case RGBA16F: return WGPUTextureFormat_RGBA16Float;
        case RGBA32F: return WGPUTextureFormat_RGBA32Float;
        case Depth24: return WGPUTextureFormat_Depth24Plus;
        case Depth32: return WGPUTextureFormat_Depth32Float;
        default: return WGPUTextureFormat_Undefined;
    }
}
// Translation function for LoadOperation to WGPULoadOp
static inline WGPULoadOp toWebGPULoadOperation(LoadOp lop) {
    switch (lop) {
    case LoadOp_Load:
        return WGPULoadOp_Load;
    case LoadOp_Clear:
        return WGPULoadOp_Clear;
    //case LoadOp_ExpandResolveTexture:
    //    // WebGPU does not have a direct equivalent; choose appropriate op or handle separately
    //    return WGPULoadOp_ExpandResolveTexture; // Example fallback
    default:
        return WGPULoadOp_Load; // Default fallback
    }
}

// Translation function for VertexFormat to WGPUVertexFormat


// Translation function for VertexStepMode to WGPUVertexStepMode
static inline WGPUVertexStepMode toWebGPUVertexStepMode(VertexStepMode vsm) {
    switch (vsm) {
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
