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

#include "macros_and_constants.h"
#if SUPPORT_WGPU_BACKEND == 1
#include <webgpu/webgpu.h>
#endif
#if SUPPORT_VULKAN_BACKEND == 1
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#endif
#include <wgvk.h>
// ------------------------- Enum Definitions -------------------------



//typedef enum ShaderStage{
//    ShaderStage_Vertex,
//    ShaderStage_TessControl,
//    ShaderStage_TessEvaluation,
//    ShaderStage_Geometry,
//    ShaderStage_Fragment,
//    ShaderStage_Compute,
//    ShaderStage_RayGen,
//    ShaderStage_RayGenNV = ShaderStage_RayGen,
//    ShaderStage_Intersect,
//    ShaderStage_IntersectNV = ShaderStage_Intersect,
//    ShaderStage_AnyHit,
//    ShaderStage_AnyHitNV = ShaderStage_AnyHit,
//    ShaderStage_ClosestHit,
//    ShaderStage_ClosestHitNV = ShaderStage_ClosestHit,
//    ShaderStage_Miss,
//    ShaderStage_MissNV = ShaderStage_Miss,
//    ShaderStage_Callable,
//    ShaderStage_CallableNV = ShaderStage_Callable,
//    ShaderStage_Task,
//    ShaderStage_TaskNV = ShaderStage_Task,
//    ShaderStage_Mesh,
//    ShaderStage_MeshNV = ShaderStage_Mesh,
//    ShaderStage_EnumCount
//}ShaderStage;
//
//typedef enum ShaderStageMask{
//    ShaderStageMask_Vertex = (1u << ShaderStage_Vertex),
//    ShaderStageMask_TessControl = (1u << ShaderStage_TessControl),
//    ShaderStageMask_TessEvaluation = (1u << ShaderStage_TessEvaluation),
//    ShaderStageMask_Geometry = (1u << ShaderStage_Geometry),
//    ShaderStageMask_Fragment = (1u << ShaderStage_Fragment),
//    ShaderStageMask_Compute = (1u << ShaderStage_Compute),
//    ShaderStageMask_RayGen = (1u << ShaderStage_RayGen),
//    ShaderStageMask_RayGenNV = (1u << ShaderStage_RayGenNV),
//    ShaderStageMask_Intersect = (1u << ShaderStage_Intersect),
//    ShaderStageMask_IntersectNV = (1u << ShaderStage_IntersectNV),
//    ShaderStageMask_AnyHit = (1u << ShaderStage_AnyHit),
//    ShaderStageMask_AnyHitNV = (1u << ShaderStage_AnyHitNV),
//    ShaderStageMask_ClosestHit = (1u << ShaderStage_ClosestHit),
//    ShaderStageMask_ClosestHitNV = (1u << ShaderStage_ClosestHitNV),
//    ShaderStageMask_Miss = (1u << ShaderStage_Miss),
//    ShaderStageMask_MissNV = (1u << ShaderStage_MissNV),
//    ShaderStageMask_Callable = (1u << ShaderStage_Callable),
//    ShaderStageMask_CallableNV = (1u << ShaderStage_CallableNV),
//    ShaderStageMask_Task = (1u << ShaderStage_Task),
//    ShaderStageMask_TaskNV = (1u << ShaderStage_TaskNV),
//    ShaderStageMask_Mesh = (1u << ShaderStage_Mesh),
//    ShaderStageMask_MeshNV = (1u << ShaderStage_MeshNV),
//    ShaderStageMask_EnumCount = (1u << ShaderStage_EnumCount)
//}ShaderStageMask;



#ifdef sadfasd
typedef WGPUDeviceLostReason WGVKDeviceLostReason;
typedef WGPUErrorType WGVKErrorType;
typedef WGPUVertexFormat VertexFormat;
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
typedef WGPURequestAdapterStatus WGVKRequestAdapterStatus;

#define WGVKRequestAdapterStatus_Success WGVKRequestAdapterStatus_Success
#define WGVKRequestAdapterStatus_InstanceDropped WGVKRequestAdapterStatus_InstanceDropped
#define WGVKRequestAdapterStatus_Unavailable WGVKRequestAdapterStatus_Unavailable
#define WGVKRequestAdapterStatus_Error WGVKRequestAdapterStatus_Error
#define WGVKRequestAdapterStatus_Force32 WGVKRequestAdapterStatus_Force32


typedef WGPUBlendFactor BlendFactor;
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

typedef WGPUBlendOperation BlendOperation;
#define BlendOperation_Undefined WGPUBlendOperation_Undefined
#define BlendOperation_Add WGPUBlendOperation_Add
#define BlendOperation_Subtract WGPUBlendOperation_Subtract
#define BlendOperation_ReverseSubtract WGPUBlendOperation_ReverseSubtract
#define BlendOperation_Min WGPUBlendOperation_Min
#define BlendOperation_Max WGPUBlendOperation_Max

typedef WGPUCompositeAlphaMode WGVKCompositeAlphaMode;
#define WGVKCompositeAlphaMode_Auto WGPUCompositeAlphaMode_Auto
#define WGVKCompositeAlphaMode_Opaque WGPUCompositeAlphaMode_Opaque
#define WGVKCompositeAlphaMode_Premultiplied WGPUCompositeAlphaMode_Premultiplied
#define WGVKCompositeAlphaMode_Unpremultiplied WGPUCompositeAlphaMode_Unpremultiplied
#define WGVKCompositeAlphaMode_Inherit WGPUCompositeAlphaMode_Inherit
#define WGVKCompositeAlphaMode_Force32 WGPUCompositeAlphaMode_Force32

#endif


#ifdef __cplusplus
constexpr BufferUsage BufferUsage_None = 0x0000000000000000;
constexpr BufferUsage BufferUsage_MapRead = 0x0000000000000001;
constexpr BufferUsage BufferUsage_MapWrite = 0x0000000000000002;
constexpr BufferUsage BufferUsage_CopySrc = 0x0000000000000004;
constexpr BufferUsage BufferUsage_CopyDst = 0x0000000000000008;
constexpr BufferUsage BufferUsage_Index = 0x0000000000000010;
constexpr BufferUsage BufferUsage_Vertex = 0x0000000000000020;
constexpr BufferUsage BufferUsage_Uniform = 0x0000000000000040;
constexpr BufferUsage BufferUsage_Storage = 0x0000000000000080;
constexpr BufferUsage BufferUsage_Indirect = 0x0000000000000100;
constexpr BufferUsage BufferUsage_QueryResolve = 0x0000000000000200;
constexpr BufferUsage BufferUsage_ShaderDeviceAddress = 0x0000000001000000;
constexpr BufferUsage BufferUsage_AccelerationStructureInput = 0x0000000002000000;
constexpr BufferUsage BufferUsage_AccelerationStructureStorage = 0x0000000004000000;
constexpr BufferUsage BufferUsage_ShaderBindingTable = 0x0000000008000000;

constexpr TextureUsage TextureUsage_None = 0x0000000000000000;
constexpr TextureUsage TextureUsage_CopySrc = 0x0000000000000001;
constexpr TextureUsage TextureUsage_CopyDst = 0x0000000000000002;
constexpr TextureUsage TextureUsage_TextureBinding = 0x0000000000000004;
constexpr TextureUsage TextureUsage_StorageBinding = 0x0000000000000008;
constexpr TextureUsage TextureUsage_RenderAttachment = 0x0000000000000010;
constexpr TextureUsage TextureUsage_TransientAttachment = 0x0000000000000020;
constexpr TextureUsage TextureUsage_StorageAttachment = 0x0000000000000040;

#else
#define BufferUsage_None 0x0000000000000000
#define BufferUsage_MapRead 0x0000000000000001
#define BufferUsage_MapWrite 0x0000000000000002
#define BufferUsage_CopySrc 0x0000000000000004
#define BufferUsage_CopyDst 0x0000000000000008
#define BufferUsage_Index 0x0000000000000010
#define BufferUsage_Vertex 0x0000000000000020
#define BufferUsage_Uniform 0x0000000000000040
#define BufferUsage_Storage 0x0000000000000080
#define BufferUsage_Indirect 0x0000000000000100
#define BufferUsage_QueryResolve 0x0000000000000200
#define TextureUsage_None 0x0000000000000000
#define TextureUsage_CopySrc 0x0000000000000001
#define TextureUsage_CopyDst 0x0000000000000002
#define TextureUsage_TextureBinding 0x0000000000000004
#define TextureUsage_StorageBinding 0x0000000000000008
#define TextureUsage_RenderAttachment 0x0000000000000010
#define TextureUsage_TransientAttachment 0x0000000000000020
#define TextureUsage_StorageAttachment 0x0000000000000040
#define BufferUsage_ShaderDeviceAddress 0x0000000001000000
#define BufferUsage_AccelerationStructureInput 0x0000000002000000
#define BufferUsage_AccelerationStructureStorage 0x0000000004000000
#define BufferUsage_ShaderBindingTable 0x0000000008000000
#endif


// -------------------- Vulkan Translation Functions --------------------

#if SUPPORT_VULKAN_BACKEND == 1

/**
 * @brief Translates custom TextureUsage flags to Vulkan's VkImageUsageFlags.
 *
 * @param usage The TextureUsage bitmask to translate.
 * @return VkImageUsageFlags The corresponding Vulkan image usage flags.
 */
static inline VkImageUsageFlags toVulkanTextureUsage(TextureUsage usage, PixelFormat format) {
    VkImageUsageFlags vkUsage = 0;

    // Array of all possible TextureUsage flags
    const TextureUsage allFlags[] = {TextureUsage_CopySrc, TextureUsage_CopyDst, TextureUsage_TextureBinding, TextureUsage_StorageBinding, TextureUsage_RenderAttachment, TextureUsage_TransientAttachment, TextureUsage_StorageAttachment};
    const uint32_t nFlags = sizeof(allFlags) / sizeof(TextureUsage);
    // Iterate through each flag and map accordingly
    for (uint32_t i = 0; i < nFlags; i++) {
        uint32_t flag = allFlags[i];
        if (usage & flag) {
            switch (flag) {
            case TextureUsage_CopySrc:
                vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
                break;
            case TextureUsage_CopyDst:
                vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
                break;
            case TextureUsage_TextureBinding:
                vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
                break;
            case TextureUsage_StorageBinding:
                vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
                break;
            case TextureUsage_RenderAttachment:
                if(format == Depth24 || format == Depth32){
                    vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                }else{
                    vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                }
                break;
            case TextureUsage_TransientAttachment:
                vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
                break;
            case TextureUsage_StorageAttachment:
                vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
                break;
            default:
                break;
            }
        }
    }

    return vkUsage;
}
static inline VkImageAspectFlags toVulkanAspectMask(TextureAspect aspect){
    VkImageAspectFlagBits ret;
    switch(aspect){
        case TextureAspect_All:
        return VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
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
// Inverse conversion: Vulkan usage flags -> TextureUsage flags
static inline TextureUsage fromVulkanTextureUsage(VkImageUsageFlags vkUsage) {
    TextureUsage usage = 0;
    
    // Map transfer bits
    if (vkUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        usage |= TextureUsage_CopySrc;
    if (vkUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        usage |= TextureUsage_CopyDst;
    
    // Map sampling bit
    if (vkUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
        usage |= TextureUsage_TextureBinding;
    
    // Map storage bit (ambiguous: could originate from either storage flag)
    if (vkUsage & VK_IMAGE_USAGE_STORAGE_BIT)
        usage |= TextureUsage_StorageBinding | TextureUsage_StorageAttachment;
    
    // Map render attachment bits (depth/stencil or color, both yield RenderAttachment)
    if (vkUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        usage |= TextureUsage_RenderAttachment;
    if (vkUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        usage |= TextureUsage_RenderAttachment;
    
    // Map transient attachment
    if (vkUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
        usage |= TextureUsage_TransientAttachment;
    
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
        case storage_texture2d: {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        case storage_texture2d_array: {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        case storage_texture3d: {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        case storage_buffer: {
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }
        case uniform_buffer: {
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        case texture2d: {
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        case texture2d_array: {
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        case texture3d: {
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        }
        case texture_sampler: {
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        }
        case acceleration_structure: {
            return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        }
        case combined_image_sampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case uniform_type_undefined: 
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
static inline VkBufferUsageFlags toVulkanBufferUsage(BufferUsage busg) {
    VkBufferUsageFlags usage = 0;

    while (busg != 0) {
        // Isolate the least significant set bit
        BufferUsage flag = (busg & (-busg));

        switch (flag) {
        case BufferUsage_CopySrc:
            usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case BufferUsage_CopyDst:
            usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferUsage_Vertex:
            usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            break;
        case BufferUsage_Index:
            usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            break;
        case BufferUsage_Uniform:
            usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
        case BufferUsage_Storage:
            usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            break;
        case BufferUsage_Indirect:
            usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
            break;
        case BufferUsage_MapRead:
            usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferUsage_MapWrite:
            usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            break;
        case BufferUsage_QueryResolve:
            // Handle Vulkan-specific flags for query resolve if applicable.
            break;
        case BufferUsage_ShaderDeviceAddress:
            usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
            break;
        case BufferUsage_AccelerationStructureInput:
            usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
            break;
        case BufferUsage_AccelerationStructureStorage:
            usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            break;
        case BufferUsage_ShaderBindingTable:
            usage |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
            break;
        default:
            rg_unreachable();
            break;
        }

        // Clear the least significant set bit
        busg &= (busg - 1);
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

static inline VkCompareOp toVulkanCompareFunction(CompareFunction cf) {
    switch (cf) {
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

static inline VkShaderStageFlagBits toVulkanShaderStage(ShaderStage stage) {
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
static inline VkShaderStageFlagBits toVulkanShaderStageBits(ShaderStageMask stage) {
    unsigned ret = 0;
    for(unsigned iter = stage;iter;iter &= (iter - 1)){
        ShaderStageMask stage = (ShaderStageMask)(iter & -iter);
        switch (stage){
            case ShaderStageMask_Vertex:         ret |= VK_SHADER_STAGE_VERTEX_BIT;break;
            case ShaderStageMask_TessControl:    ret |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;break;
            case ShaderStageMask_TessEvaluation: ret |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;break;
            case ShaderStageMask_Geometry:       ret |= VK_SHADER_STAGE_GEOMETRY_BIT;break;
            case ShaderStageMask_Fragment:       ret |= VK_SHADER_STAGE_FRAGMENT_BIT;break;
            case ShaderStageMask_Compute:        ret |= VK_SHADER_STAGE_COMPUTE_BIT;break;
            case ShaderStageMask_RayGen:         ret |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;break;
            case ShaderStageMask_Miss:           ret |= VK_SHADER_STAGE_MISS_BIT_KHR;break;
            case ShaderStageMask_ClosestHit:     ret |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;break;
            case ShaderStageMask_AnyHit:         ret |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;break;
            case ShaderStageMask_Intersect:      ret |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;break;
            case ShaderStageMask_Callable:       ret |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;break;
            case ShaderStageMask_Task:           ret |= VK_SHADER_STAGE_TASK_BIT_EXT;break;
            case ShaderStageMask_Mesh:           ret |= VK_SHADER_STAGE_MESH_BIT_EXT;break;
            default: rg_unreachable();
        }
    }
    return (VkShaderStageFlagBits)(ret);
}

static inline VkBlendFactor toVulkanBlendFactor(BlendFactor bf) {
    switch (bf) {
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

static inline VkBlendOp toVulkanBlendOperation(BlendOperation bo) {
    switch (bo) {
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

static inline VkFormat toVulkanVertexFormat(VertexFormat vf) {
    switch (vf) {
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

static inline VkVertexInputRate toVulkanVertexStepMode(VertexStepMode vsm) {
    switch (vsm) {
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
/**
 * @brief Translates custom TextureUsage flags to WebGPU's WGPUTextureUsage flags.
 *
 * @param usage The TextureUsage bitmask to translate.
 * @return WGPUTextureUsage The corresponding WebGPU texture usage flags.
 */
static inline WGPUTextureUsage toWebGPUTextureUsage(TextureUsage usage) {
    WGPUTextureUsage wgpuUsage = 0;

    // Array of all possible TextureUsage flags
    const TextureUsage allFlags[] = {TextureUsage_CopySrc, TextureUsage_CopyDst, TextureUsage_TextureBinding, TextureUsage_StorageBinding, TextureUsage_RenderAttachment, TextureUsage_TransientAttachment, TextureUsage_StorageAttachment};
    // Iterate through each dflag and map accordingly
    uint32_t remainigFlags = usage;
    while (remainigFlags != 0) {
        uint32_t flag = remainigFlags & -remainigFlags;
        switch (flag) {
        case TextureUsage_CopySrc:
            wgpuUsage |= WGPUTextureUsage_CopySrc;
            break;
        case TextureUsage_CopyDst:
            wgpuUsage |= WGPUTextureUsage_CopyDst;
            break;
        case TextureUsage_TextureBinding:
            wgpuUsage |= WGPUTextureUsage_TextureBinding;
            break;
        case TextureUsage_StorageBinding:
            wgpuUsage |= WGPUTextureUsage_StorageBinding;
            break;
        case TextureUsage_RenderAttachment:
            wgpuUsage |= WGPUTextureUsage_RenderAttachment;
            break;
        //case TextureUsage_TransientAttachment:
        //    wgpuUsage |= WGPUTextureUsage_TransientAttachment;
        //    break;
        //case TextureUsage_StorageAttachment:
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
static inline WGPUBufferUsage toWebGPUBufferUsage(BufferUsage busg) {
    WGPUBufferUsage usage = 0;

    while (busg != 0) {
        BufferUsage flag = (busg & (-busg));

        switch (flag) {
        case BufferUsage_CopySrc:
            usage |= WGPUBufferUsage_CopySrc;
            break;
        case BufferUsage_CopyDst:
            usage |= WGPUBufferUsage_CopyDst;
            break;
        case BufferUsage_Vertex:
            usage |= WGPUBufferUsage_Vertex;
            break;
        case BufferUsage_Index:
            usage |= WGPUBufferUsage_Index;
            break;
        case BufferUsage_Uniform:
            usage |= WGPUBufferUsage_Uniform;
            break;
        case BufferUsage_Storage:
            usage |= WGPUBufferUsage_Storage;
            break;
        case BufferUsage_Indirect:
            usage |= WGPUBufferUsage_Indirect;
            break;
        case BufferUsage_MapRead:
            usage |= WGPUBufferUsage_MapRead;
            break;
        case BufferUsage_MapWrite:
            usage |= WGPUBufferUsage_MapWrite;
            break;
        case BufferUsage_QueryResolve:
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

// Translation function for CompareFunction to WGPUCompareFunction
static inline WGPUCompareFunction toWebGPUCompareFunction(CompareFunction cf) {
    switch (cf) {
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
static inline WGPUBlendFactor toWebGPUBlendFactor(BlendFactor bf) {
    return bf;
    //switch (bf) {
    //case BlendFactor_Zero:
    //    return WGPUBlendFactor_Zero;
    //case BlendFactor_One:
    //    return WGPUBlendFactor_One;
    //case BlendFactor_Src:
    //    return WGPUBlendFactor_Src;
    //case BlendFactor_OneMinusSrc:
    //    return WGPUBlendFactor_OneMinusSrc;
    //case BlendFactor_SrcAlpha:
    //    return WGPUBlendFactor_SrcAlpha;
    //case BlendFactor_OneMinusSrcAlpha:
    //    return WGPUBlendFactor_OneMinusSrcAlpha;
    //case BlendFactor_Dst:
    //    return WGPUBlendFactor_Dst;
    //case BlendFactor_OneMinusDst:
    //    return WGPUBlendFactor_OneMinusDst;
    //case BlendFactor_DstAlpha:
    //    return WGPUBlendFactor_DstAlpha;
    //case BlendFactor_OneMinusDstAlpha:
    //    return WGPUBlendFactor_OneMinusDstAlpha;
    //case BlendFactor_SrcAlphaSaturated:
    //    return WGPUBlendFactor_SrcAlphaSaturated;
    //case BlendFactor_Constant:
    //    return WGPUBlendFactor_Constant;
    //case BlendFactor_OneMinusConstant:
    //    return WGPUBlendFactor_OneMinusConstant;
    //case BlendFactor_Src1:
    //    return WGPUBlendFactor_Src1;
    //case BlendFactor_OneMinusSrc1:
    //    return WGPUBlendFactor_OneMinusSrc1;
    //case BlendFactor_Src1Alpha:
    //    return WGPUBlendFactor_Src1Alpha;
    //case BlendFactor_OneMinusSrc1Alpha:
    //    return WGPUBlendFactor_OneMinusSrc1Alpha;
    //default:
    //    return WGPUBlendFactor_One; // Default fallback
    //}
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
static inline WGPUVertexFormat toWebGPUVertexFormat(VertexFormat vf) {
    switch (vf) {
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
    default:
        return WGPUVertexFormat_Force32; // Default fallback
    }
}

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
