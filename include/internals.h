// begin file include/internals.hpp
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




#ifndef INTERNALS_HPP_INCLUDED
#define INTERNALS_HPP_INCLUDED
#include "config.h"
#if SUPPORT_VULKAN_BACKEND == 1
    #include <wgvk.h>
#endif
#include "raygpu.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
















#include <webgpu/webgpu.h>

// Note: Flag enums like RGBufferUsage are defined with values matching WGPU, so a direct cast is sufficient.
// The functions are provided for consistency and type safety.

static inline WGPUBlendFactor RG_to_WGPU_BlendFactor(RGBlendFactor factor) {
    switch (factor) {
        case RGBlendFactor_Zero: return WGPUBlendFactor_Zero;
        case RGBlendFactor_One: return WGPUBlendFactor_One;
        case RGBlendFactor_Src: return WGPUBlendFactor_Src;
        case RGBlendFactor_OneMinusSrc: return WGPUBlendFactor_OneMinusSrc;
        case RGBlendFactor_SrcAlpha: return WGPUBlendFactor_SrcAlpha;
        case RGBlendFactor_OneMinusSrcAlpha: return WGPUBlendFactor_OneMinusSrcAlpha;
        case RGBlendFactor_Dst: return WGPUBlendFactor_Dst;
        case RGBlendFactor_OneMinusDst: return WGPUBlendFactor_OneMinusDst;
        case RGBlendFactor_DstAlpha: return WGPUBlendFactor_DstAlpha;
        case RGBlendFactor_OneMinusDstAlpha: return WGPUBlendFactor_OneMinusDstAlpha;
        case RGBlendFactor_SrcAlphaSaturated: return WGPUBlendFactor_SrcAlphaSaturated;
        case RGBlendFactor_Constant: return WGPUBlendFactor_Constant;
        case RGBlendFactor_OneMinusConstant: return WGPUBlendFactor_OneMinusConstant;
        case RGBlendFactor_Src1: return WGPUBlendFactor_Src1;
        case RGBlendFactor_OneMinusSrc1: return WGPUBlendFactor_OneMinusSrc1;
        case RGBlendFactor_Src1Alpha: return WGPUBlendFactor_Src1Alpha;
        case RGBlendFactor_OneMinusSrc1Alpha: return WGPUBlendFactor_OneMinusSrc1Alpha;
        default: return WGPUBlendFactor_Undefined;
    }
}

static inline WGPUBlendOperation RG_to_WGPU_BlendOperation(RGBlendOperation op) {
    switch (op) {
        case RGBlendOperation_Add: return WGPUBlendOperation_Add;
        case RGBlendOperation_Subtract: return WGPUBlendOperation_Subtract;
        case RGBlendOperation_ReverseSubtract: return WGPUBlendOperation_ReverseSubtract;
        case RGBlendOperation_Min: return WGPUBlendOperation_Min;
        case RGBlendOperation_Max: return WGPUBlendOperation_Max;
        default: return WGPUBlendOperation_Undefined;
    }
}

static inline WGPUCompareFunction RG_to_WGPU_CompareFunction(RGCompareFunction func) {
    switch (func) {
        case RGCompareFunction_Never: return WGPUCompareFunction_Never;
        case RGCompareFunction_Less: return WGPUCompareFunction_Less;
        case RGCompareFunction_Equal: return WGPUCompareFunction_Equal;
        case RGCompareFunction_LessEqual: return WGPUCompareFunction_LessEqual;
        case RGCompareFunction_Greater: return WGPUCompareFunction_Greater;
        case RGCompareFunction_NotEqual: return WGPUCompareFunction_NotEqual;
        case RGCompareFunction_GreaterEqual: return WGPUCompareFunction_GreaterEqual;
        case RGCompareFunction_Always: return WGPUCompareFunction_Always;
        default: return WGPUCompareFunction_Undefined;
    }
}

static inline WGPUFrontFace RG_to_WGPU_FrontFace(RGFrontFace face) {
    switch (face) {
        case RGFrontFace_CCW: return WGPUFrontFace_CCW;
        case RGFrontFace_CW: return WGPUFrontFace_CW;
        default: return WGPUFrontFace_Undefined;
    }
}

static inline WGPUIndexFormat RG_to_WGPU_IndexFormat(IndexFormat format) {
    switch (format) {
        case IndexFormat_Uint16: return WGPUIndexFormat_Uint16;
        case IndexFormat_Uint32: return WGPUIndexFormat_Uint32;
        default: return WGPUIndexFormat_Undefined;
    }
}

static inline WGPULoadOp RG_to_WGPU_LoadOp(RGLoadOp op) {
    switch (op) {
        case RGLoadOp_Clear: return WGPULoadOp_Clear;
        case RGLoadOp_Load: return WGPULoadOp_Load;
        default: return WGPULoadOp_Undefined;
    }
}

static inline WGPUStoreOp RG_to_WGPU_StoreOp(RGStoreOp op) {
    switch (op) {
        case RGStoreOp_Store: return WGPUStoreOp_Store;
        case RGStoreOp_Discard: return WGPUStoreOp_Discard;
        default: return WGPUStoreOp_Undefined;
    }
}

static inline WGPUVertexStepMode RG_to_WGPU_VertexStepMode(RGVertexStepMode mode) {
    switch (mode) {
        case RGVertexStepMode_Vertex: return WGPUVertexStepMode_Vertex;
        case RGVertexStepMode_Instance: return WGPUVertexStepMode_Instance;
        case RGVertexStepMode_VertexBufferNotUsed: return WGPUVertexStepMode_Undefined;
        default: return (WGPUVertexStepMode)0; // Or some other default
    }
}

static inline WGPUBufferUsage RG_to_WGPU_BufferUsage(RGBufferUsage usage) {
    WGPUBufferUsage wgpu_usage = WGPUBufferUsage_None;
    if (usage & RGBufferUsage_MapRead)       wgpu_usage |= WGPUBufferUsage_MapRead;
    if (usage & RGBufferUsage_MapWrite)      wgpu_usage |= WGPUBufferUsage_MapWrite;
    if (usage & RGBufferUsage_CopySrc)       wgpu_usage |= WGPUBufferUsage_CopySrc;
    if (usage & RGBufferUsage_CopyDst)       wgpu_usage |= WGPUBufferUsage_CopyDst;
    if (usage & RGBufferUsage_Index)         wgpu_usage |= WGPUBufferUsage_Index;
    if (usage & RGBufferUsage_Vertex)        wgpu_usage |= WGPUBufferUsage_Vertex;
    if (usage & RGBufferUsage_Uniform)       wgpu_usage |= WGPUBufferUsage_Uniform;
    if (usage & RGBufferUsage_Storage)       wgpu_usage |= WGPUBufferUsage_Storage;
    if (usage & RGBufferUsage_Indirect)      wgpu_usage |= WGPUBufferUsage_Indirect;
    if (usage & RGBufferUsage_QueryResolve)  wgpu_usage |= WGPUBufferUsage_QueryResolve;
    return wgpu_usage;
}

static inline WGPUTextureUsage RG_to_WGPU_TextureUsage(RGTextureUsage usage) {
    WGPUTextureUsage wgpu_usage = WGPUTextureUsage_None;
    if (usage & RGTextureUsage_CopySrc)          wgpu_usage |= WGPUTextureUsage_CopySrc;
    if (usage & RGTextureUsage_CopyDst)          wgpu_usage |= WGPUTextureUsage_CopyDst;
    if (usage & RGTextureUsage_TextureBinding)   wgpu_usage |= WGPUTextureUsage_TextureBinding;
    if (usage & RGTextureUsage_StorageBinding)   wgpu_usage |= WGPUTextureUsage_StorageBinding;
    if (usage & RGTextureUsage_RenderAttachment) wgpu_usage |= WGPUTextureUsage_RenderAttachment;
    return wgpu_usage;
}

static inline WGPUShaderStage RG_to_WGPU_ShaderStage(RGShaderStage stage) {
    WGPUShaderStage wgpu_stage = WGPUShaderStage_None;
    if (stage & RGShaderStage_Vertex)   wgpu_stage |= WGPUShaderStage_Vertex;
    if (stage & RGShaderStage_Fragment) wgpu_stage |= WGPUShaderStage_Fragment;
    if (stage & RGShaderStage_Compute)  wgpu_stage |= WGPUShaderStage_Compute;
    return wgpu_stage;
}

static inline WGPUVertexFormat RG_to_WGPU_VertexFormat(RGVertexFormat format) {
    switch(format) {
        case RGVertexFormat_Uint8x2: return WGPUVertexFormat_Uint8x2;
        case RGVertexFormat_Uint8x4: return WGPUVertexFormat_Uint8x4;
        case RGVertexFormat_Sint8x2: return WGPUVertexFormat_Sint8x2;
        case RGVertexFormat_Sint8x4: return WGPUVertexFormat_Sint8x4;
        case RGVertexFormat_Unorm8x2: return WGPUVertexFormat_Unorm8x2;
        case RGVertexFormat_Unorm8x4: return WGPUVertexFormat_Unorm8x4;
        case RGVertexFormat_Snorm8x2: return WGPUVertexFormat_Snorm8x2;
        case RGVertexFormat_Snorm8x4: return WGPUVertexFormat_Snorm8x4;
        case RGVertexFormat_Uint16x2: return WGPUVertexFormat_Uint16x2;
        case RGVertexFormat_Uint16x4: return WGPUVertexFormat_Uint16x4;
        case RGVertexFormat_Sint16x2: return WGPUVertexFormat_Sint16x2;
        case RGVertexFormat_Sint16x4: return WGPUVertexFormat_Sint16x4;
        case RGVertexFormat_Unorm16x2: return WGPUVertexFormat_Unorm16x2;
        case RGVertexFormat_Unorm16x4: return WGPUVertexFormat_Unorm16x4;
        case RGVertexFormat_Snorm16x2: return WGPUVertexFormat_Snorm16x2;
        case RGVertexFormat_Snorm16x4: return WGPUVertexFormat_Snorm16x4;
        case RGVertexFormat_Float16x2: return WGPUVertexFormat_Float16x2;
        case RGVertexFormat_Float16x4: return WGPUVertexFormat_Float16x4;
        case RGVertexFormat_Float32: return WGPUVertexFormat_Float32;
        case RGVertexFormat_Float32x2: return WGPUVertexFormat_Float32x2;
        case RGVertexFormat_Float32x3: return WGPUVertexFormat_Float32x3;
        case RGVertexFormat_Float32x4: return WGPUVertexFormat_Float32x4;
        case RGVertexFormat_Uint32: return WGPUVertexFormat_Uint32;
        case RGVertexFormat_Uint32x2: return WGPUVertexFormat_Uint32x2;
        case RGVertexFormat_Uint32x3: return WGPUVertexFormat_Uint32x3;
        case RGVertexFormat_Uint32x4: return WGPUVertexFormat_Uint32x4;
        case RGVertexFormat_Sint32: return WGPUVertexFormat_Sint32;
        case RGVertexFormat_Sint32x2: return WGPUVertexFormat_Sint32x2;
        case RGVertexFormat_Sint32x3: return WGPUVertexFormat_Sint32x3;
        case RGVertexFormat_Sint32x4: return WGPUVertexFormat_Sint32x4;
        default: return (WGPUVertexFormat)~0;
    }
}
static inline RGPresentMode WGPU_to_RG_PresentMode(WGPUPresentMode pm){
    switch(pm){
        case WGPUPresentMode_Undefined: return RGPresentMode_Undefined;
        case WGPUPresentMode_Fifo: return RGPresentMode_Fifo;
        case WGPUPresentMode_FifoRelaxed: return RGPresentMode_FifoRelaxed;
        case WGPUPresentMode_Immediate: return RGPresentMode_Immediate;
        case WGPUPresentMode_Mailbox: return RGPresentMode_Mailbox;
    }
}

static inline WGPUPresentMode RG_to_WGPU_PresentMode(RGPresentMode pm){
    switch(pm){
        case RGPresentMode_Undefined: return WGPUPresentMode_Undefined;
        case RGPresentMode_Fifo: return WGPUPresentMode_Fifo;
        case RGPresentMode_FifoRelaxed: return WGPUPresentMode_FifoRelaxed;
        case RGPresentMode_Immediate: return WGPUPresentMode_Immediate;
        case RGPresentMode_Mailbox: return WGPUPresentMode_Mailbox;
    }
}






















static inline uint32_t bitcount32(uint32_t x){
    #ifdef _MSC_VER
    return __popcnt(x);
    #elif defined(__GNUC__) 
    return __builtin_popcount(x);
    #else
    return std::popcount(x);
    #endif
}

static inline uint32_t bitcount64(uint64_t x){
    #ifdef _MSC_VER
    return __popcnt64(x);
    #elif defined(__GNUC__) 
    return __builtin_popcountll(x);
    #else
    return std::popcount(x);
    #endif
}

static inline size_t hashAttributeAndResidence(const AttributeAndResidence* res){
    uint64_t attrh = ROT_BYTES(res->attr.shaderLocation * ((uint64_t)41), 31) ^ ROT_BYTES(res->attr.offset, 11);
    attrh *= 111;
    attrh ^= ROT_BYTES(res->attr.format, 48) * ((uint64_t)44497);
    uint64_t v = ROT_BYTES(res->bufferSlot * ((uint64_t)756839), 13) ^ ROT_BYTES(attrh * ((uint64_t)1171), 47);
    v ^= ROT_BYTES(res->enabled * ((uint64_t)2976221), 23);
    return v;
}
static inline size_t hashVectorOfAttributeAndResidence(const AttributeAndResidence* attribs, uint32_t count){
    uint64_t hv = 0;
    for(uint32_t i = 0;i < count;i++){
        hv ^= hashAttributeAndResidence(attribs + i);
        hv = ROT_BYTES(hv, 7);
    }
    return hv;
}
/**
 * @brief Get the Bindings object, returning a map from 
 * Uniform name -> UniformDescriptor (type and minimum size and binding location)
 * 
 * @param shaderSource
 */
RGAPI StringToUniformMap* getBindingsWGSL(ShaderSources source);
RGAPI StringToUniformMap* getBindingsGLSL(ShaderSources source);
RGAPI StringToUniformMap* getBindings    (ShaderSources source);





/**
 * @brief returning a map from 
 * Attribute name -> Attribute format (vec2f, vec3f, etc.) and attribute location
 * @param shaderSource 
 * @return std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> 
 */
RGAPI InOutAttributeInfo getAttributesWGSL(ShaderSources sources);
RGAPI InOutAttributeInfo getAttributesGLSL(ShaderSources sources);
RGAPI InOutAttributeInfo getAttributes    (ShaderSources sources);

RGAPI DescribedBuffer* UpdateVulkanRenderbatch();
void PushUsedBuffer(void* nativeBuffer);

typedef struct VertexBufferLayout{
    uint64_t arrayStride;
    RGVertexStepMode stepMode;
    size_t attributeCount;
    RGVertexAttribute* attributes; //NOT owned, points into data owned by VertexBufferLayoutSet::attributePool with an offset
}VertexBufferLayout;

typedef struct VertexBufferLayoutSet{
    uint32_t number_of_buffers;
    VertexBufferLayout* layouts;    
    RGVertexAttribute* attributePool;
}VertexBufferLayoutSet;

static inline PixelFormat fromWGPUPixelFormat(WGPUTextureFormat format) {
    switch (format) {
        case WGPUTextureFormat_RGBA8Unorm:      return PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        case WGPUTextureFormat_RGBA8UnormSrgb:  return PIXELFORMAT_UNCOMPRESSED_R8G8B8A8_SRGB;
        case WGPUTextureFormat_BGRA8Unorm:      return PIXELFORMAT_UNCOMPRESSED_B8G8R8A8;
        case WGPUTextureFormat_BGRA8UnormSrgb:  return PIXELFORMAT_UNCOMPRESSED_B8G8R8A8_SRGB;
        case WGPUTextureFormat_RGBA16Float:     return PIXELFORMAT_UNCOMPRESSED_R16G16B16A16;
        case WGPUTextureFormat_RGBA32Float:     return PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        case WGPUTextureFormat_Depth24Plus:     return PIXELFORMAT_DEPTH_24_PLUS;
        case WGPUTextureFormat_Depth32Float:    return PIXELFORMAT_DEPTH_32_FLOAT;
        default:
            rg_unreachable();
    }
    return (PixelFormat)(-1); // Unreachable but silences compiler warnings
}
static inline WGPUTextureFormat toWGPUPixelFormat(PixelFormat format) {
    switch (format) {
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            return WGPUTextureFormat_RGBA8Unorm;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8_SRGB:
            return WGPUTextureFormat_RGBA8UnormSrgb;
        case PIXELFORMAT_UNCOMPRESSED_B8G8R8A8:
            return WGPUTextureFormat_BGRA8Unorm;
        case PIXELFORMAT_UNCOMPRESSED_B8G8R8A8_SRGB:
            return WGPUTextureFormat_BGRA8UnormSrgb;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            return WGPUTextureFormat_RGBA16Float;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            return WGPUTextureFormat_RGBA32Float;
        case PIXELFORMAT_DEPTH_24_PLUS:
            return WGPUTextureFormat_Depth24Plus;
        case PIXELFORMAT_DEPTH_32_FLOAT:
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

typedef struct {
    uint64_t x64;
} xorshiftstate_c99;

static inline void init_xorshiftstate(xorshiftstate_c99* state, uint64_t seed) {
    state->x64 = seed;
}
static inline void update_xorshiftstate(xorshiftstate_c99* state, uint64_t value) {
    state->x64 ^= (state->x64 << 13);
    state->x64 ^= (state->x64 >> 7);
    state->x64 ^= (state->x64 << 17);
    state->x64 ^= value;
}

static inline uint64_t hash_bytes(const void* bytes, size_t count) {
    xorshiftstate_c99 xsstate;
    init_xorshiftstate(&xsstate, 0x324234fff1f1);

    size_t i = 0;
    for (i = 0; i + sizeof(uint64_t) <= count; i += sizeof(uint64_t)) {
        uint64_t chunk;
        memcpy(&chunk, (const char*)bytes + i, sizeof(uint64_t));
        update_xorshiftstate(&xsstate, chunk);
    }

    if (count % sizeof(uint64_t) != 0) {
        uint64_t remaining_chunk = 0;
        memcpy(&remaining_chunk, (const char*)bytes + i, count % sizeof(uint64_t));
        update_xorshiftstate(&xsstate, remaining_chunk);
    }
    return xsstate.x64;
}

static inline size_t hashVertexBufferLayoutSet(const VertexBufferLayoutSet* set){
    xorshiftstate_c99 xsstate = {((uint64_t)0x1919846573) * ((uint64_t)set->number_of_buffers << 14)};
    for(uint32_t i = 0;i < set->number_of_buffers;i++){
        for(uint32_t j = 0;j < set->layouts[i].attributeCount;j++){
            update_xorshiftstate(&xsstate, (set->layouts[i].attributes[j].offset << 32) | set->layouts[i].attributes[j].shaderLocation);
        }
    }
    return xsstate.x64;
}

#define DEFINE_QUICKSORT_FUNCTION(type, compare_expr)                                      \
static inline int quicksort_less_##type(const type* left, const type* right) {             \
    return (compare_expr);                                                                 \
}                                                                                          \
static void quickSort_##type(type* begin, type* end) {                                     \
    if (!begin || !end || begin >= end) return;                                            \
    enum { QS_THRESH = 8, STACK_MAX = 64 };                                                \
    type* stack[STACK_MAX * 2];                                                            \
    int top = 0;                                                                           \
    stack[top++] = begin;                                                                  \
    stack[top++] = end;                                                                    \
    while (top) {                                                                          \
        type* hi = stack[--top];                                                           \
        type* lo = stack[--top];                                                           \
        ptrdiff_t n = hi - lo;                                                             \
        if (n <= 1) continue;                                                              \
        if (n <= QS_THRESH) {                                                              \
            for (type* p = lo + 1; p < hi; ++p) {                                          \
                type key = *p;                                                             \
                type* q = p;                                                               \
                while (q > lo && quicksort_less_##type(&key, q - 1)) {                     \
                    *q = *(q - 1);                                                         \
                    --q;                                                                   \
                }                                                                          \
                *q = key;                                                                  \
            }                                                                              \
            continue;                                                                      \
        }                                                                                  \
        type pivot = lo[n >> 1];                                                           \
        type* i = lo;                                                                      \
        type* j = hi - 1;                                                                  \
        for (;;) {                                                                         \
            while (quicksort_less_##type(i, &pivot)) ++i;                                  \
            while (quicksort_less_##type(&pivot, j)) --j;                                  \
            if (i > j) break;                                                              \
            { type tmp = *i; *i = *j; *j = tmp; }                                          \
            ++i;                                                                           \
            --j;                                                                           \
            if (i > j) break;                                                              \
        }                                                                                  \
        ptrdiff_t left_n  = (j + 1) - lo;                                                  \
        ptrdiff_t right_n = hi - i;                                                        \
        if (left_n > right_n) {                                                            \
            if (left_n  > 1) { stack[top++] = lo; stack[top++] = j + 1; }                  \
            if (right_n > 1) { stack[top++] = i;  stack[top++] = hi;   }                   \
        } else {                                                                           \
            if (right_n > 1) { stack[top++] = i;  stack[top++] = hi;   }                   \
            if (left_n  > 1) { stack[top++] = lo; stack[top++] = j + 1; }                  \
        }                                                                                  \
    }                                                                                      \
}

DEFINE_QUICKSORT_FUNCTION(ResourceTypeDescriptor, left->location < right->location)

//static inline bool vblayoutVectorCompare(const VertexBufferLayoutSet& a, const VertexBufferLayoutSet& b){
//    
//    if(a.number_of_buffers != b.number_of_buffers)return false;
//    for(uint32_t i = 0;i < a.number_of_buffers;i++){
//        if(a.layouts[i].attributeCount != b.layouts[i].attributeCount)return false;
//        for(uint32_t j = 0;j < a.layouts[i].attributeCount;j++){
//            if(
//                a.layouts[i].attributes[j].format != b.layouts[i].attributes[j].format
//             || a.layouts[i].attributes[j].shaderLocation != b.layouts[i].attributes[j].shaderLocation
//             || a.layouts[i].attributes[j].offset != b.layouts[i].attributes[j].offset
//            )return false;
//        }
//    }
//    return true;
//}


typedef struct ColorAttachmentState{
    PixelFormat attachmentFormats[MAX_COLOR_ATTACHMENTS];
    uint32_t colorAttachmentCount;
    #ifdef __cplusplus
    bool operator==(const ColorAttachmentState& other)const noexcept{
        if(colorAttachmentCount == other.colorAttachmentCount){
            for(uint32_t i = 0;i < colorAttachmentCount;i++){
                if(attachmentFormats[i] != other.attachmentFormats[i]){
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    #endif
}ColorAttachmentState;

static inline bool ColorAttachmentState_eq(const ColorAttachmentState* a, const ColorAttachmentState* b){
    if(a->colorAttachmentCount == b->colorAttachmentCount){
        for(uint32_t i = 0;i < a->colorAttachmentCount;i++){
            if(a->attachmentFormats[i] != b->attachmentFormats[i]){
                return false;
            }
        }
        return true;
    }
    return false;
}

typedef struct ModifiablePipelineState{
    AttributeAndResidence* vertexAttributes;
    uint32_t vertexAttributeCount;
    PrimitiveType primitiveType;
    RenderSettings settings;
    ColorAttachmentState colorAttachmentState;
}ModifiablePipelineState;

static ModifiablePipelineState ModifiablePipelineState_copy(const ModifiablePipelineState mpst_){
    const ModifiablePipelineState* mpst = &mpst_;
    ModifiablePipelineState ret = {
        .vertexAttributes = (AttributeAndResidence*)RL_CALLOC(mpst->vertexAttributeCount, sizeof(AttributeAndResidence)),
        .vertexAttributeCount = mpst->vertexAttributeCount,
        .primitiveType = mpst->primitiveType,
        .settings = mpst->settings,
        .colorAttachmentState = mpst->colorAttachmentState
    };
    memcpy(ret.vertexAttributes, mpst->vertexAttributes, mpst->vertexAttributeCount * sizeof(AttributeAndResidence));
    return ret;
}
static void ModifiablePipelineState_free(ModifiablePipelineState mpst){
    free(mpst.vertexAttributes);
}
static WGPURenderPipeline RenderPipelineCopy(const WGPURenderPipeline rpl){
    wgpuRenderPipelineAddRef(rpl);
    return rpl;
}
static void RenderPipeline_free(WGPURenderPipeline mpst){
    wgpuRenderPipelineRelease(mpst);
}

static inline bool vertexArCompare(const AttributeAndResidence* a, uint32_t aCount, const AttributeAndResidence* b, uint32_t bCount){
    
    #ifndef NDEBUG
    int prevloc = -1;
    for(size_t i = 0;i < aCount;i++){
        assert((int)a[i].attr.shaderLocation > prevloc && "AttributeAndResidence* not sorted");
        prevloc = a[i].attr.shaderLocation;
    }
    prevloc = -1;
    for(size_t i = 0;i < bCount;i++){
        assert((int)b[i].attr.shaderLocation > prevloc && "AttributeAndResidence* not sorted");
        prevloc = b[i].attr.shaderLocation;
    }
    #endif
    if(aCount != bCount)return false;

    for(size_t i = 0;i < aCount;i++){
        if(
               a[i].bufferSlot != b[i].bufferSlot 
            || a[i].enabled != b[i].enabled
            || a[i].stepMode != b[i].stepMode 
            || a[i].attr.format != b[i].attr.format 
            || a[i].attr.offset != b[i].attr.offset 
            || a[i].attr.shaderLocation != b[i].attr.shaderLocation
        ){
            return false;
        }
    }
    return true;
}

static inline bool WGPUBlendState_eq(const WGPUBlendState* a, const WGPUBlendState* b){
    return 
    
    a->alpha.dstFactor == b->alpha.dstFactor &&
    a->alpha.srcFactor == b->alpha.srcFactor &&
    a->alpha.operation == b->alpha.operation &&
    a->color.operation == b->color.operation &&
    a->color.operation == b->color.operation &&
    a->color.operation == b->color.operation &&
    true;
}
static inline bool RGBlendState_eq(const RGBlendState* a, const RGBlendState* b){
    return 
    
    a->alpha.dstFactor == b->alpha.dstFactor &&
    a->alpha.srcFactor == b->alpha.srcFactor &&
    a->alpha.operation == b->alpha.operation &&
    a->color.operation == b->color.operation &&
    a->color.operation == b->color.operation &&
    a->color.operation == b->color.operation &&
    true;
}

static inline bool RenderSettings_eq(const RenderSettings* a, const RenderSettings* b){
    return
        a->depthTest    == b->depthTest     && 
        a->faceCull     == b->faceCull      && 
        a->sampleCount  == b->sampleCount   && 
        a->lineWidth    == b->lineWidth     && 
        RGBlendState_eq(&a->blendState, &b->blendState)    && 
        a->frontFace    == b->frontFace     && 
        a->depthCompare == b->depthCompare  &&
    true;
}

static inline bool ModifiablePipelineState_eq(const ModifiablePipelineState msp1_, const ModifiablePipelineState msp2_){
    const ModifiablePipelineState* msp1 = &msp1_;
    const ModifiablePipelineState* msp2 = &msp2_;
    return vertexArCompare(msp1->vertexAttributes, msp1->vertexAttributeCount, msp2->vertexAttributes, msp2->vertexAttributeCount)
    && msp1->primitiveType == msp2->primitiveType
    && RenderSettings_eq(&msp1->settings, &msp2->settings)
    && ColorAttachmentState_eq(&msp1->colorAttachmentState, &msp2->colorAttachmentState);
}

static inline size_t hashModifiablePipelineState(ModifiablePipelineState mfps_){
    ModifiablePipelineState* mfps = &mfps_;
    size_t ret = hashVectorOfAttributeAndResidence(mfps->vertexAttributes, mfps->vertexAttributeCount) ^ hash_bytes(&mfps->settings, sizeof(RenderSettings)) ^ ROT_BYTES(mfps->primitiveType, 17);
    for(uint32_t i = 0;i < mfps->colorAttachmentState.colorAttachmentCount;i++){
        ret = ROT_BYTES(mfps->primitiveType, 3) ^ ((size_t)mfps->colorAttachmentState.attachmentFormats[i]);
    }
    return ret;
}


#ifndef PHM_INLINE_CAPACITY
#define PHM_INLINE_CAPACITY 3
#endif

#ifndef PHM_INITIAL_HEAP_CAPACITY
#define PHM_INITIAL_HEAP_CAPACITY 8
#endif

#ifndef PHM_LOAD_FACTOR_NUM
#define PHM_LOAD_FACTOR_NUM 3
#endif

#ifndef PHM_LOAD_FACTOR_DEN
#define PHM_LOAD_FACTOR_DEN 4
#endif

#ifndef PHM_HASH_MULTIPLIER
#define PHM_HASH_MULTIPLIER 0x9E3779B97F4A7C15ULL
#endif
#ifndef PHM_EMPTY_SLOT_KEY
#define PHM_EMPTY_SLOT_KEY NULL
#endif
#ifndef PHM_DELETED_SLOT_KEY
#define PHM_DELETED_SLOT_KEY ((void*)0xFFFFFFFFFFFFFFFF)
#endif

#define RG_DEFINE_GENERIC_HASH_MAP(SCOPE, Name, KeyType, ValueType, KeyHashFunc, KeyCmpFunc, KeyEmptyVal, KeyCopyFunc, ValueCopyFunc, KeyDeleteFunc, ValueDeleteFunc) \
    typedef struct Name##_kv_pair {                                                                                              \
        KeyType key;                                                                                                             \
        ValueType value;                                                                                                         \
    } Name##_kv_pair;                                                                                                            \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        uint64_t current_capacity;                                                                                               \
        KeyType empty_key_sentinel;                                                                                              \
        Name##_kv_pair *table;                                                                                                   \
    } Name;                                                                                                                      \
    static inline uint64_t Name##_hash_key_internal(KeyType key) { return KeyHashFunc(key); }                                    \
    static inline uint64_t Name##_round_up_to_power_of_2(uint64_t v) {                                                           \
        if (v == 0)                                                                                                              \
            return 0;                                                                                                            \
        v--;                                                                                                                     \
        v |= v >> 1;                                                                                                             \
        v |= v >> 2;                                                                                                             \
        v |= v >> 4;                                                                                                             \
        v |= v >> 8;                                                                                                             \
        v |= v >> 16;                                                                                                            \
        v |= v >> 32;                                                                                                            \
        v++;                                                                                                                     \
        return v;                                                                                                                \
    }                                                                                                                            \
    static void Name##_insert_entry(                                                                                             \
        Name##_kv_pair *table, uint64_t capacity, KeyType key, ValueType value, KeyType empty_key_val_param) {                   \
        assert(!KeyCmpFunc(key, empty_key_val_param) && capacity > 0 && (capacity & (capacity - 1)) == 0);                       \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(table[index].key, empty_key_val_param)) {                                                             \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        table[index].key = key;                                                                                                  \
        table[index].value = value;                                                                                              \
    }                                                                                                                            \
    static Name##_kv_pair *Name##_find_slot(Name *map, KeyType key) {                                                            \
        assert(map->table != NULL && map->current_capacity > 0);                                                                 \
        uint64_t cap_mask = map->current_capacity - 1;                                                                           \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(map->table[index].key, map->empty_key_sentinel) && !KeyCmpFunc(map->table[index].key, key)) {         \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        return &map->table[index];                                                                                               \
    }                                                                                                                            \
    static void Name##_grow(Name *map);                                                                                          \
    SCOPE void Name##_init(Name *map) {                                                                                          \
        map->current_size = 0;                                                                                                   \
        map->current_capacity = 0;                                                                                               \
        map->empty_key_sentinel = (KeyEmptyVal);                                                                                 \
        map->table = NULL;                                                                                                       \
    }                                                                                                                            \
    static void Name##_grow(Name *map) {                                                                                         \
        uint64_t old_capacity = map->current_capacity;                                                                           \
        Name##_kv_pair *old_table = map->table;                                                                                  \
        uint64_t new_capacity;                                                                                                   \
        if (old_capacity == 0) {                                                                                                 \
            new_capacity = (PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8;                                      \
        } else {                                                                                                                 \
            if (old_capacity >= (UINT64_MAX / 2))                                                                                \
                new_capacity = UINT64_MAX;                                                                                       \
            else                                                                                                                 \
                new_capacity = old_capacity * 2;                                                                                 \
        }                                                                                                                        \
        new_capacity = Name##_round_up_to_power_of_2(new_capacity);                                                              \
        if (new_capacity == 0 && old_capacity == 0 && ((PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8) > 0) {   \
            new_capacity = (UINT64_C(1) << 63);                                                                                  \
        }                                                                                                                        \
        if (new_capacity == 0 || (new_capacity <= old_capacity && old_capacity > 0)) {                                           \
            return;                                                                                                              \
        }                                                                                                                        \
        Name##_kv_pair *new_table = (Name##_kv_pair *)malloc(new_capacity * sizeof(Name##_kv_pair));                             \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
        for (uint64_t i = 0; i < new_capacity; ++i) {                                                                            \
            new_table[i].key = map->empty_key_sentinel;                                                                          \
        }                                                                                                                        \
        if (old_table && map->current_size > 0) {                                                                                \
            uint64_t rehashed_count = 0;                                                                                         \
            for (uint64_t i = 0; i < old_capacity; ++i) {                                                                        \
                if (!KeyCmpFunc(old_table[i].key, map->empty_key_sentinel)) {                                                    \
                    Name##_insert_entry(new_table, new_capacity, old_table[i].key, old_table[i].value, map->empty_key_sentinel); \
                    rehashed_count++;                                                                                            \
                    if (rehashed_count == map->current_size)                                                                     \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        if (old_table)                                                                                                           \
            free(old_table);                                                                                                     \
        map->table = new_table;                                                                                                  \
        map->current_capacity = new_capacity;                                                                                    \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_put(Name *map, KeyType key, ValueType value) {                                                              \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 ||                                                                                        \
            (map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) {                      \
            uint64_t old_cap = map->current_capacity;                                                                            \
            Name##_grow(map);                                                                                                    \
            if (map->current_capacity == old_cap && old_cap > 0) {                                                               \
                if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)                \
                    return 0;                                                                                                    \
            } else if (map->current_capacity == 0)                                                                               \
                return 0;                                                                                                        \
            else if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)               \
                return 0;                                                                                                        \
        }                                                                                                                        \
        assert(map->current_capacity > 0 && map->table != NULL);                                                                 \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        if (KeyCmpFunc(slot->key, map->empty_key_sentinel)) {                                                                    \
            slot->key = KeyCopyFunc(key);                                                                                        \
            slot->value = ValueCopyFunc(value);                                                                                  \
            map->current_size++;                                                                                                 \
            return 1;                                                                                                            \
        } else {                                                                                                                 \
            assert(KeyCmpFunc(slot->key, key));                                                                                  \
            ValueDeleteFunc(slot->value);                                                                                        \
            slot->value = ValueCopyFunc(value);                                                                                  \
            return 0;                                                                                                            \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE ValueType *Name##_get(Name *map, KeyType key) {                                                                        \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 || map->table == NULL)                                                                    \
            return NULL;                                                                                                         \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        return (KeyCmpFunc(slot->key, key)) ? &slot->value : NULL;                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_erase(Name* map, KeyType key) {                                                                             \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 || map->table == NULL)                                                                    \
            return 0;                                                                                                            \
        Name##_kv_pair* slot = Name##_find_slot(map, key);                                                                        \
        if (KeyCmpFunc(slot->key, key)) {                                                                                        \
            KeyDeleteFunc(slot->key);                                                                                            \
            ValueDeleteFunc(slot->value);                                                                                        \
            slot->key = map->empty_key_sentinel;                                                                                 \
            map->current_size--;                                                                                                 \
            return 1;                                                                                                            \
        }                                                                                                                        \
        return 0;                                                                                                                \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *map, void (*callback)(KeyType key, ValueType * value, void *user_data), void *user_data) {  \
        if (map->current_capacity > 0 && map->table != NULL && map->current_size > 0) {                                          \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (!KeyCmpFunc(map->table[i].key, map->empty_key_sentinel)) {                                                   \
                    callback(map->table[i].key, &map->table[i].value, user_data);                                                \
                    if (++count == map->current_size)                                                                            \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *map) {                                                                                          \
        if (map->table != NULL) {                                                                                                \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (!KeyCmpFunc(map->table[i].key, map->empty_key_sentinel)) {                                                   \
                    KeyDeleteFunc(map->table[i].key);                                                                            \
                    ValueDeleteFunc(map->table[i].value);                                                                        \
                }                                                                                                                \
            }                                                                                                                    \
            free(map->table);                                                                                                    \
        }                                                                                                                        \
        Name##_init(map);                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            Name##_free(dest);                                                                                                   \
        *dest = *source;                                                                                                         \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_clear(Name *map) {                                                                                         \
        if (map->table != NULL && map->current_capacity > 0) {                                                                   \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (!KeyCmpFunc(map->table[i].key, map->empty_key_sentinel)) {                                                   \
                    KeyDeleteFunc(map->table[i].key);                                                                            \
                    ValueDeleteFunc(map->table[i].value);                                                                        \
                    map->table[i].key = map->empty_key_sentinel;                                                                 \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        map->current_size = 0;                                                                                                   \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            Name##_free(dest);                                                                                                   \
        Name##_init(dest);                                                                                                       \
        if (source->table != NULL && source->current_capacity > 0) {                                                             \
            dest->table = (Name##_kv_pair *)malloc(source->current_capacity * sizeof(Name##_kv_pair));                           \
            if (!dest->table) {                                                                                                  \
                Name##_init(dest);                                                                                               \
                return;                                                                                                          \
            }                                                                                                                    \
            dest->current_capacity = source->current_capacity;                                                                   \
            dest->empty_key_sentinel = source->empty_key_sentinel;                                                               \
            for (uint64_t i = 0; i < source->current_capacity; ++i) {                                                            \
                if (!KeyCmpFunc(source->table[i].key, source->empty_key_sentinel)) {                                             \
                    dest->table[i].key = KeyCopyFunc(source->table[i].key);                                                      \
                    dest->table[i].value = ValueCopyFunc(source->table[i].value);                                                \
                } else {                                                                                                         \
                    dest->table[i].key = source->empty_key_sentinel;                                                             \
                }                                                                                                                \
            }                                                                                                                    \
            dest->current_size = source->current_size;                                                                           \
        }                                                                                                                        \
    }

RG_DEFINE_GENERIC_HASH_MAP(static inline, PipelineHashMap, ModifiablePipelineState, WGPURenderPipeline, hashModifiablePipelineState, ModifiablePipelineState_eq, CLITERAL(ModifiablePipelineState){0}, ModifiablePipelineState_copy, RenderPipelineCopy, ModifiablePipelineState_free, RenderPipeline_free) 


RGAPI WGPURenderPipeline createSingleRenderPipe(const ModifiablePipelineState* mst, const DescribedShaderModule* shaderModule, const DescribedBindGroupLayout* bglayout, const DescribedPipelineLayout* pllayout);

static WGPURenderPipeline PipelineHashMap_getOrCreate(PipelineHashMap* cacheMap, const ModifiablePipelineState* mst, const DescribedShaderModule* shaderModule, const DescribedBindGroupLayout* bglayout, const DescribedPipelineLayout* pllayout){
    WGPURenderPipeline* pl = PipelineHashMap_get(cacheMap, *mst);
    if(pl){
        return *pl;
    }
    else{
        WGPURenderPipeline toEmplace = createSingleRenderPipe(mst, shaderModule, bglayout, pllayout);
        PipelineHashMap_put(cacheMap, *mst, toEmplace);
        return toEmplace;
    }
}
typedef struct ShaderImpl{
    PipelineHashMap pipelineCache;
    DescribedShaderModule shaderModule;
    ModifiablePipelineState state;
    DescribedBindGroup bindGroup;
    DescribedPipelineLayout layout;
    DescribedBindGroupLayout bglayout;
}ShaderImpl;
externcvar ShaderImpl* allocatedShaderIDs_shc;
RGAPI uint32_t getNextShaderID_shc();
RGAPI ShaderImpl* GetShaderImpl(Shader shader);
RGAPI ShaderImpl* GetShaderImplByID(uint32_t id);
typedef struct DescribedPipeline{
    WGPURenderPipeline activePipeline;
    
}DescribedPipeline;

/**
 * @brief Get the Buffer Layout representation compatible with WebGPU or Vulkan
 * 
 * @return VertexBufferLayoutSet 
 */

static inline VertexBufferLayoutSet getBufferLayoutRepresentation2(const AttributeAndResidence* attributes, const uint32_t number_of_attribs, const uint32_t number_of_buffers) {
    VertexBufferLayoutSet result;
    result.number_of_buffers = number_of_buffers;
    result.layouts = NULL;
    result.attributePool = NULL;

    if (number_of_buffers > 0) {
        result.layouts = (VertexBufferLayout*)RL_CALLOC(number_of_buffers, sizeof(VertexBufferLayout));
    }
    if (number_of_attribs > 0) {
        result.attributePool = (RGVertexAttribute*)RL_CALLOC(number_of_attribs, sizeof(RGVertexAttribute));
    }

    if ((number_of_buffers > 0 && result.layouts == NULL) || (number_of_attribs > 0 && result.attributePool == NULL)) {
        free(result.layouts);
        free(result.attributePool);
        result.number_of_buffers = 0;
        result.layouts = NULL;
        result.attributePool = NULL;
        return result;
    }

    uint32_t* attribute_counts = (uint32_t*)RL_CALLOC(number_of_buffers, sizeof(uint32_t));
    if (number_of_buffers > 0 && attribute_counts == NULL) {
        free(result.layouts);
        free(result.attributePool);
        result.number_of_buffers = 0;
        result.layouts = NULL;
        result.attributePool = NULL;
        return result;
    }

    for (uint32_t i = 0; i < number_of_attribs; ++i) {
        if (attributes[i].bufferSlot < number_of_buffers) {
            attribute_counts[attributes[i].bufferSlot]++;
        }
    }

    RGVertexAttribute* current_pool_pointer = result.attributePool;
    for (uint32_t i = 0; i < number_of_buffers; ++i) {
        result.layouts[i].attributeCount = attribute_counts[i];
        result.layouts[i].attributes = current_pool_pointer;
        result.layouts[i].stepMode = RGVertexStepMode_VertexBufferNotUsed;
        current_pool_pointer += attribute_counts[i];
    }

    memset(attribute_counts, 0, number_of_buffers * sizeof(uint32_t));

    for (uint32_t i = 0; i < number_of_attribs; ++i) {
        uint32_t slot = attributes[i].bufferSlot;
        if (slot < number_of_buffers) {
            VertexBufferLayout* layout = &result.layouts[slot];
            
            uint32_t index_in_buffer = attribute_counts[slot];
            layout->attributes[index_in_buffer] = attributes[i].attr;
            attribute_counts[slot]++;

            layout->arrayStride += attributeSize(attributes[i].attr.format);

            if (layout->stepMode != RGVertexStepMode_VertexBufferNotUsed && layout->stepMode != attributes[i].stepMode) {
            }
            layout->stepMode = attributes[i].stepMode;
        }
    }

    free(attribute_counts);

    return result;
}
const char* copyString(const char* str);
//static inline void UnloadBufferLayoutSet(VertexBufferLayoutSet set){
//    std::free(set.layouts);
//    std::free(set.attributePool);
//}

static inline uint32_t getReflectionAttributeLocation(const InOutAttributeInfo* attributes, const char* name){
    if(name == NULL){
        return LOCATION_NOT_FOUND;
    }
    for(uint32_t i = 0;i < attributes->vertexAttributeCount;i++){
        int equal = 1;
        for(size_t j = 0;j < MAX_VERTEX_ATTRIBUTE_NAME_LENGTH;j++){
            if(name[j] == '\0'){
                if(attributes->vertexAttributes[i].name[j] != '\0'){
                    equal = 0;
                }
                break;
            }
            else if(attributes->vertexAttributes[i].name[j] != name[j]){
                equal = 0;
                break;
            }
        }
        if(equal){
            return attributes->vertexAttributes[i].location;
        }
    }
    return LOCATION_NOT_FOUND;
}

static inline ShaderSources singleStage(const char* code, ShaderSourceType language, RGShaderStageEnum stage){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 1;
    sources.sources[0].data = code;
    sources.sources[0].sizeInBytes = strlen(code);
    sources.sources[0].stageMask = (RGShaderStage)(1u << stage);
    return sources;
}

static inline ShaderSources dualStage(const char* code, ShaderSourceType language, RGShaderStageEnum stage1, RGShaderStageEnum stage2){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 1;
    sources.sources[0].data = code;
    sources.sources[0].sizeInBytes = strlen(code);
    sources.sources[0].stageMask = ((RGShaderStage)((1u << (uint32_t)(stage1)) | (1u << (uint32_t)(stage2))));
    return sources;
}


static inline ShaderSources dualStageDualSource(const char* code1, const char* code2, ShaderSourceType language, RGShaderStageEnum stage1, RGShaderStageEnum stage2){
    ShaderSources sources zeroinit;
    sources.language = language;
    sources.sourceCount = 2;
    sources.sources[0].data = code1;
    sources.sources[0].sizeInBytes = strlen(code1);
    sources.sources[0].stageMask = ((RGShaderStage)(1ull << (uint32_t)(+stage1)));

    sources.sources[1].data = code2;
    sources.sources[1].sizeInBytes = strlen(code2);
    sources.sources[1].stageMask = ((RGShaderStage)((1ull << (uint32_t)(+stage2))));
 
    return sources;
}

#define VLA_STACK_SIZE (1 << 20)
#define VLA_ALLOCATION_COUNT (64)
typedef struct VLAStack{
    uint32_t currentAllocationIndex;
    uint32_t currentDataOffset;
    uint32_t allocationSizes[VLA_ALLOCATION_COUNT];
    char data[VLA_STACK_SIZE];
}VLAStack;

static inline void* VLAStack_alloc(VLAStack* stack, uint32_t size){
    rassert(stack->currentDataOffset + size < VLA_STACK_SIZE, "VLAStack is out of memory");
    rassert(stack->currentAllocationIndex < VLA_STACK_SIZE - 1, "VLAStack has too many allocations");
    void* ret = (void*)(stack->data + stack->currentDataOffset);
    stack->currentDataOffset += size;
    stack->allocationSizes[stack->currentAllocationIndex++] = size;
    return ret;
}

static inline void VLAStack_free(VLAStack* stack, void* data){
    uint32_t size = stack->allocationSizes[stack->currentAllocationIndex - 1];
    rassert(stack->data + (stack->currentDataOffset - size) == data, "Corrupted");
    stack->currentDataOffset -= size;
    stack->currentAllocationIndex--;
}

extern VLAStack g_vlastack;

void detectShaderLanguage(ShaderSources* sources);
RGAPI ShaderSourceType detectShaderLanguageSingle(const void* sourceptr, size_t size);
StringToUniformMap* getBindingsGLSL(ShaderSources source);
typedef struct EntryPointSet{
    char names[RGShaderStageEnum_EnumCount][MAX_SHADER_ENTRYPOINT_NAME_LENGTH + 1];
}EntryPointSet;
DescribedShaderModule LoadShaderModule(ShaderSources source);

RGAPI EntryPointSet                                           getEntryPointsWGSL (const char* shaderSourceWGSL);
RGAPI InOutAttributeInfo                                      getAttributesSPIRV (ShaderSources sources);
RGAPI StringToUniformMap*                                     getBindingsSPIRV   (ShaderSources sources);
RGAPI EntryPointSet                                           getEntryPointsSPIRV(const uint32_t* shaderSourceSPIRV, uint32_t wordCount);

static inline VertexBufferLayoutSet getBufferLayoutRepresentation(const AttributeAndResidence* attributes, const uint32_t number_of_attribs){
    uint32_t maxslot = 0;
    for(size_t i = 0;i < number_of_attribs;i++){
        maxslot = std_max_u32(maxslot, attributes[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    return getBufferLayoutRepresentation2(attributes, number_of_attribs, number_of_buffers);
}
typedef struct BufferEntry{
    DescribedBuffer* buffer;
    RGVertexStepMode stepMode;
} BufferEntry;

typedef struct VertexArray{
    AttributeAndResidence* attributes;
    size_t attributes_count;
    size_t attributes_capacity;

    BufferEntry* buffers;
    size_t buffers_count;
    size_t buffers_capacity;
} VertexArray;

static void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        RL_FREE(pointer);
        return NULL;
    }
    void* result = RL_REALLOC(pointer, newSize);
    if (result == NULL) {
        TRACELOG(LOG_FATAL, "realloc: Out of Memory!");
    }
    return result;
}

static void VertexArray_Init(VertexArray* vao) {
    vao->attributes = NULL;
    vao->attributes_count = 0;
    vao->attributes_capacity = 0;
    vao->buffers = NULL;
    vao->buffers_count = 0;
    vao->buffers_capacity = 0;
}

static void VertexArray_Destroy(VertexArray* vao) {
    free(vao->attributes);
    free(vao->buffers);
    VertexArray_Init(vao); // Reset to a clean state
}

static int compareAttributes(const void* a, const void* b) {
    const AttributeAndResidence* attrA = (const AttributeAndResidence*)a;
    const AttributeAndResidence* attrB = (const AttributeAndResidence*)b;
    if (attrA->attr.shaderLocation < attrB->attr.shaderLocation) return -1;
    if (attrA->attr.shaderLocation > attrB->attr.shaderLocation) return 1;
    return 0;
}

static void VertexArray_add(VertexArray* vao, DescribedBuffer* buffer, uint32_t shaderLocation,
                            RGVertexFormat fmt, uint32_t offset, RGVertexStepMode stepmode) {
    // Search for existing attribute by shaderLocation
    AttributeAndResidence* it = NULL;
    for (size_t i = 0; i < vao->attributes_count; ++i) {
        if (vao->attributes[i].attr.shaderLocation == shaderLocation) {
            it = &vao->attributes[i];
            break;
        }
    }

    if (it != NULL) {
        // Attribute exists, update it
        size_t existingBufferSlot = it->bufferSlot;
        BufferEntry* existingBufferPair = &vao->buffers[existingBufferSlot];

        // Check if the buffer is the same
        if (existingBufferPair->buffer->buffer != buffer->buffer) {
            // Attempting to update to a new buffer
            // Check if the new buffer is already in buffers
            int bufferIt_idx = -1;
            for (size_t i = 0; i < vao->buffers_count; ++i) {
                if (vao->buffers[i].buffer->buffer == buffer->buffer && vao->buffers[i].stepMode == stepmode) {
                    bufferIt_idx = (int)i;
                    break;
                }
            }

            if (bufferIt_idx != -1) {
                // Reuse existing buffer slot
                it->bufferSlot = bufferIt_idx;
            } else {
                // Check if the old buffer slot is now unused by other attributes
                bool is_slot_used_by_others = false;
                for(size_t i = 0; i < vao->attributes_count; ++i) {
                    if (vao->attributes[i].bufferSlot == it->bufferSlot && vao->attributes[i].attr.shaderLocation != it->attr.shaderLocation) {
                        is_slot_used_by_others = true;
                        break;
                    }
                }
                
                // This buffer slot is unused otherwise, so reuse
                if (!is_slot_used_by_others) {
                    vao->buffers[it->bufferSlot].buffer = buffer;
                    vao->buffers[it->bufferSlot].stepMode = stepmode;
                } else {
                    // Add new buffer
                    it->bufferSlot = vao->buffers_count;
                    if (vao->buffers_count >= vao->buffers_capacity) {
                        size_t old_capacity = vao->buffers_capacity;
                        vao->buffers_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
                        vao->buffers = (BufferEntry*)reallocate(vao->buffers, old_capacity * sizeof(BufferEntry), vao->buffers_capacity * sizeof(BufferEntry));
                    }
                    vao->buffers[vao->buffers_count].buffer = buffer;
                    vao->buffers[vao->buffers_count].stepMode = stepmode;
                    vao->buffers_count++;
                }
            }
        }

        // Update the rest of the attribute properties
        it->stepMode = stepmode;
        it->attr.format = fmt;
        it->attr.offset = offset;
        it->enabled = true;

        TRACELOG(LOG_DEBUG, "Attribute at shader location %u updated.", shaderLocation);
    } else {
        // Attribute does not exist, add as new
        AttributeAndResidence insert = {};
        insert.enabled = true;
        bool bufferFound = false;

        for (size_t i = 0; i < vao->buffers_count; ++i) {
            if (vao->buffers[i].buffer->buffer == buffer->buffer) {
                if (vao->buffers[i].stepMode == stepmode) {
                    // Reuse existing buffer slot
                    insert.bufferSlot = i;
                    bufferFound = true;
                    break;
                } else {
                    TRACELOG(LOG_FATAL, "Mixed step modes for the same buffer are not implemented");
                    exit(EXIT_FAILURE);
                }
            }
        }

        if (!bufferFound) {
            // Add new buffer
            insert.bufferSlot = vao->buffers_count;
            if (vao->buffers_count >= vao->buffers_capacity) {
                size_t old_capacity = vao->buffers_capacity;
                vao->buffers_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
                vao->buffers = (BufferEntry*)reallocate(vao->buffers, old_capacity * sizeof(BufferEntry), vao->buffers_capacity * sizeof(BufferEntry));
            }
            vao->buffers[vao->buffers_count].buffer = buffer;
            vao->buffers[vao->buffers_count].stepMode = stepmode;
            vao->buffers_count++;
        }

        // Set the attribute properties
        insert.stepMode = stepmode;
        insert.attr.format = fmt;
        insert.attr.offset = offset;
        insert.attr.shaderLocation = shaderLocation;

        // Add the new attribute to the dynamic array
        if (vao->attributes_count >= vao->attributes_capacity) {
            size_t old_capacity = vao->attributes_capacity;
            vao->attributes_capacity = old_capacity < 8 ? 8 : old_capacity * 2;
            vao->attributes = (AttributeAndResidence*)reallocate(vao->attributes, old_capacity * sizeof(AttributeAndResidence), vao->attributes_capacity * sizeof(AttributeAndResidence));
        }
        vao->attributes[vao->attributes_count] = insert;
        vao->attributes_count++;


        TRACELOG(LOG_DEBUG,  "New attribute added at shader location %u", shaderLocation);
    }

    // Sort attributes based on shaderLocation
    qsort(vao->attributes, vao->attributes_count, sizeof(AttributeAndResidence), compareAttributes);
}


static bool VertexArray_enableAttribute(VertexArray* vao, uint32_t shaderLocation) {
    for (size_t i = 0; i < vao->attributes_count; ++i) {
        if (vao->attributes[i].attr.shaderLocation == shaderLocation) {
            if (!vao->attributes[i].enabled) {
                vao->attributes[i].enabled = true;
            }
            return true;
        }
    }

    TRACELOG(LOG_WARNING, "Attribute with shader location %u not found.", shaderLocation);
    return false;
}

static bool VertexArray_disableAttribute(VertexArray* vao, uint32_t shaderLocation) {
    for (size_t i = 0; i < vao->attributes_count; ++i) {
        if (vao->attributes[i].attr.shaderLocation == shaderLocation) {
            if (vao->attributes[i].enabled) {
                vao->attributes[i].enabled = false;
                TRACELOG(LOG_DEBUG, "Attribute at shader location %u disabled.", shaderLocation);
            } else {
                TRACELOG(LOG_DEBUG, "Attribute at shader location %u is already disabled", shaderLocation);
            }
            return true;
        }
    }
    TRACELOG(LOG_WARNING, "Attribute at shader location %u does not exist", shaderLocation);
    return false;
}
#define DEFINE_GENERIC_HASH_MAP(SCOPE, Name, KeyType, ValueType, KeyHashFunc, KeyCmpFunc, KeyEmptyVal)                           \
    typedef struct Name##_kv_pair {                                                                                              \
        KeyType key;                                                                                                             \
        ValueType value;                                                                                                         \
    } Name##_kv_pair;                                                                                                            \
    typedef struct Name {                                                                                                        \
        uint64_t current_size;                                                                                                   \
        uint64_t current_capacity;                                                                                               \
        KeyType empty_key_sentinel;                                                                                              \
        Name##_kv_pair *table;                                                                                                   \
    } Name;                                                                                                                      \
    static inline uint64_t Name##_hash_key_internal(KeyType key) { return KeyHashFunc(key); }                                    \
    static inline uint64_t Name##_round_up_to_power_of_2(uint64_t v) {                                                           \
        if (v == 0)                                                                                                              \
            return 0;                                                                                                            \
        v--;                                                                                                                     \
        v |= v >> 1;                                                                                                             \
        v |= v >> 2;                                                                                                             \
        v |= v >> 4;                                                                                                             \
        v |= v >> 8;                                                                                                             \
        v |= v >> 16;                                                                                                            \
        v |= v >> 32;                                                                                                            \
        v++;                                                                                                                     \
        return v;                                                                                                                \
    }                                                                                                                            \
    static void Name##_insert_entry(                                                                                             \
        Name##_kv_pair *table, uint64_t capacity, KeyType key, ValueType value, KeyType empty_key_val_param) {                   \
        assert(!KeyCmpFunc(key, empty_key_val_param) && capacity > 0 && (capacity & (capacity - 1)) == 0);                       \
        uint64_t cap_mask = capacity - 1;                                                                                        \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(table[index].key, empty_key_val_param)) {                                                             \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        table[index].key = key;                                                                                                  \
        table[index].value = value;                                                                                              \
    }                                                                                                                            \
    static Name##_kv_pair *Name##_find_slot(Name *map, KeyType key) {                                                            \
        assert(map->table != NULL && map->current_capacity > 0);                                                                 \
        uint64_t cap_mask = map->current_capacity - 1;                                                                           \
        uint64_t index = Name##_hash_key_internal(key) & cap_mask;                                                               \
        while (!KeyCmpFunc(map->table[index].key, map->empty_key_sentinel) && !KeyCmpFunc(map->table[index].key, key)) {         \
            index = (index + 1) & cap_mask;                                                                                      \
        }                                                                                                                        \
        return &map->table[index];                                                                                               \
    }                                                                                                                            \
    static void Name##_grow(Name *map);                                                                                          \
    SCOPE void Name##_init(Name *map) {                                                                                          \
        map->current_size = 0;                                                                                                   \
        map->current_capacity = 0;                                                                                               \
        map->empty_key_sentinel = (KeyEmptyVal);                                                                                 \
        map->table = NULL;                                                                                                       \
    }                                                                                                                            \
    static void Name##_grow(Name *map) {                                                                                         \
        uint64_t old_capacity = map->current_capacity;                                                                           \
        Name##_kv_pair *old_table = map->table;                                                                                  \
        uint64_t new_capacity;                                                                                                   \
        if (old_capacity == 0) {                                                                                                 \
            new_capacity = (PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8;                                      \
        } else {                                                                                                                 \
            if (old_capacity >= (UINT64_MAX / 2))                                                                                \
                new_capacity = UINT64_MAX;                                                                                       \
            else                                                                                                                 \
                new_capacity = old_capacity * 2;                                                                                 \
        }                                                                                                                        \
        new_capacity = Name##_round_up_to_power_of_2(new_capacity);                                                              \
        if (new_capacity == 0 && old_capacity == 0 && ((PHM_INITIAL_HEAP_CAPACITY > 0) ? PHM_INITIAL_HEAP_CAPACITY : 8) > 0) {   \
            new_capacity = (UINT64_C(1) << 63);                                                                                  \
        }                                                                                                                        \
        if (new_capacity == 0 || (new_capacity <= old_capacity && old_capacity > 0)) {                                           \
            return;                                                                                                              \
        }                                                                                                                        \
        Name##_kv_pair *new_table = (Name##_kv_pair *)malloc(new_capacity * sizeof(Name##_kv_pair));                             \
        if (!new_table)                                                                                                          \
            return;                                                                                                              \
        for (uint64_t i = 0; i < new_capacity; ++i) {                                                                            \
            new_table[i].key = map->empty_key_sentinel;                                                                          \
        }                                                                                                                        \
        if (old_table && map->current_size > 0) {                                                                                \
            uint64_t rehashed_count = 0;                                                                                         \
            for (uint64_t i = 0; i < old_capacity; ++i) {                                                                        \
                if (!KeyCmpFunc(old_table[i].key, map->empty_key_sentinel)) {                                                    \
                    Name##_insert_entry(new_table, new_capacity, old_table[i].key, old_table[i].value, map->empty_key_sentinel); \
                    rehashed_count++;                                                                                            \
                    if (rehashed_count == map->current_size)                                                                     \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
        if (old_table)                                                                                                           \
            free(old_table);                                                                                                     \
        map->table = new_table;                                                                                                  \
        map->current_capacity = new_capacity;                                                                                    \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE int Name##_put(Name *map, KeyType key, ValueType value) {                                                              \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 ||                                                                                        \
            (map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM) {                      \
            uint64_t old_cap = map->current_capacity;                                                                            \
            Name##_grow(map);                                                                                                    \
            if (map->current_capacity == old_cap && old_cap > 0) {                                                               \
                if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)                \
                    return 0;                                                                                                    \
            } else if (map->current_capacity == 0)                                                                               \
                return 0;                                                                                                        \
            else if ((map->current_size + 1) * PHM_LOAD_FACTOR_DEN >= map->current_capacity * PHM_LOAD_FACTOR_NUM)               \
                return 0;                                                                                                        \
        }                                                                                                                        \
        assert(map->current_capacity > 0 && map->table != NULL);                                                                 \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        if (KeyCmpFunc(slot->key, map->empty_key_sentinel)) {                                                                    \
            slot->key = key;                                                                                                     \
            slot->value = value;                                                                                                 \
            map->current_size++;                                                                                                 \
            return 1;                                                                                                            \
        } else {                                                                                                                 \
            assert(KeyCmpFunc(slot->key, key));                                                                                  \
            slot->value = value;                                                                                                 \
            return 0;                                                                                                            \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE ValueType *Name##_get(Name *map, KeyType key) {                                                                        \
        assert(!KeyCmpFunc(key, map->empty_key_sentinel));                                                                       \
        if (map->current_capacity == 0 || map->table == NULL)                                                                    \
            return NULL;                                                                                                         \
        Name##_kv_pair *slot = Name##_find_slot(map, key);                                                                       \
        return (KeyCmpFunc(slot->key, key)) ? &slot->value : NULL;                                                               \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_for_each(Name *map, void (*callback)(KeyType key, ValueType * value, void *user_data), void *user_data) {  \
        if (map->current_capacity > 0 && map->table != NULL && map->current_size > 0) {                                          \
            uint64_t count = 0;                                                                                                  \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                if (!KeyCmpFunc(map->table[i].key, map->empty_key_sentinel)) {                                                   \
                    callback(map->table[i].key, &map->table[i].value, user_data);                                                \
                    if (++count == map->current_size)                                                                            \
                        break;                                                                                                   \
                }                                                                                                                \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_free(Name *map) {                                                                                          \
        if (map->table != NULL)                                                                                                  \
            free(map->table);                                                                                                    \
        Name##_init(map);                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_move(Name *dest, Name *source) {                                                                           \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table);                                                                                                   \
        *dest = *source;                                                                                                         \
        Name##_init(source);                                                                                                     \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_clear(Name *map) {                                                                                         \
        map->current_size = 0;                                                                                                   \
        if (map->table != NULL && map->current_capacity > 0) {                                                                   \
            for (uint64_t i = 0; i < map->current_capacity; ++i) {                                                               \
                map->table[i].key = map->empty_key_sentinel;                                                                     \
            }                                                                                                                    \
        }                                                                                                                        \
    }                                                                                                                            \
                                                                                                                                 \
    SCOPE void Name##_copy(Name *dest, const Name *source) {                                                                     \
        if (dest == source)                                                                                                      \
            return;                                                                                                              \
        if (dest->table != NULL)                                                                                                 \
            free(dest->table);                                                                                                   \
        Name##_init(dest);                                                                                                       \
        dest->current_size = source->current_size;                                                                               \
        if (source->table != NULL && source->current_capacity > 0) {                                                             \
            dest->table = (Name##_kv_pair *)malloc(source->current_capacity * sizeof(Name##_kv_pair));                           \
            if (!dest->table) {                                                                                                  \
                Name##_init(dest);                                                                                               \
                return;                                                                                                          \
            }                                                                                                                    \
            memcpy(dest->table, source->table, source->current_capacity * sizeof(Name##_kv_pair));                               \
            dest->current_capacity = source->current_capacity;                                                                   \
        }                                                                                                                        \
    }

typedef struct BindingIdentifier{
    uint32_t length;
    char name[MAX_BINDING_NAME_LENGTH + 1];
}BindingIdentifier;

static inline BindingIdentifier BIfromCString(const char* c_str){
    size_t len = strlen(c_str);
    BindingIdentifier identifier = {0, 0};
    if(len > MAX_BINDING_NAME_LENGTH){
        return identifier;
    }
    identifier.length = (uint32_t)len;
    memcpy(identifier.name, c_str, MAX_BINDING_NAME_LENGTH);
    return identifier;
}

static inline size_t hashBindingIdentifier(const BindingIdentifier ident){
    return hash_bytes(ident.name, ident.length);
}
static inline void deleteBindingIdentifier(const BindingIdentifier ident){}
static inline void deleteResourceType(const ResourceTypeDescriptor ident){}
static inline BindingIdentifier copyBindingIdentifier(const BindingIdentifier ident){return ident;}
static inline ResourceTypeDescriptor copyResourceType(const ResourceTypeDescriptor rtype){return rtype;}


static inline bool hashBindingCompare(const BindingIdentifier a, const BindingIdentifier b){
    if(a.length != b.length){
        return false;
    }
    for(uint32_t i = 0;i < a.length;i++){
        if(a.name[i] != b.name[i]){
            return false;
        }
    }
    return true;
}

// SCOPE, Name, KeyType, ValueType, KeyHashFunc, KeyCmpFunc, KeyEmptyVal, KeyCopyFunc, ValueCopyFunc, KeyDeleteFunc, ValueDeleteFunc

RG_DEFINE_GENERIC_HASH_MAP(
    static inline,
    StringToUniformMap,
    BindingIdentifier,
    ResourceTypeDescriptor,
    hashBindingIdentifier,
    hashBindingCompare,
    CLITERAL(BindingIdentifier){0},
    copyBindingIdentifier,
    copyResourceType,
    deleteBindingIdentifier,
    deleteResourceType
)


static inline size_t hashVertexArray(const VertexArray va){
    size_t hashValue = 0;
    // Hash the attributes
    for (size_t i = 0;i < va.attributes_count;i++) {
        AttributeAndResidence* attrRes = &va.attributes[i];
        // Hash bufferSlot
        size_t bufferSlotHash = (attrRes->bufferSlot);
        hashValue ^= bufferSlotHash;
        hashValue = ROT_BYTES(hashValue, 5);
        // Hash stepMode
        size_t stepModeHash = (int)(attrRes->stepMode);
        hashValue ^= stepModeHash;
        hashValue = ROT_BYTES(hashValue, 5);
        // Hash shaderLocation
        size_t shaderLocationHash = (attrRes->attr.shaderLocation);
        hashValue ^= shaderLocationHash;
        hashValue = ROT_BYTES(hashValue, 5);
        // Hash format
        size_t formatHash = ((int)(attrRes->attr.format));
        hashValue ^= formatHash;
        hashValue = ROT_BYTES(hashValue, 5);
        // Hash offset
        size_t offsetHash = (attrRes->attr.offset);
        hashValue ^= offsetHash;
        hashValue = ROT_BYTES(hashValue, 5);
        // Hash enabled flag
        size_t enabledHash = (attrRes->enabled);
        hashValue ^= enabledHash;
        hashValue = ROT_BYTES(hashValue, 5);
    }
    // Hash the buffers (excluding DescribedBuffer* pointers)
    for (size_t i = 0;i < va.buffers_count;i++) {
        // Only hash the WGPUVertexStepMode, not the buffer pointer
        size_t stepModeHash = (uint64_t)(va.buffers[i].stepMode);
        hashValue ^= stepModeHash;
        hashValue = ROT_BYTES(hashValue, 5);
    }
    return hashValue;
}



// Shader compilation utilities

/**
 * WGSL compilation is single code only because an arbitrary set of entrypoints can be placed there anyway.
 */

/**
 * @brief Compiles Vertex and Fragment shaders sources from GLSL (Vulkan rules) to SPIRV
 * Returns _two_ spirv blobs containing the respective modules.
 */

//std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char* vs, const char* fs);
//std::vector<uint32_t> wgsl_to_spirv(const char* anything);
//std::vector<uint32_t> glsl_to_spirv(const char* cs);
ShaderSources wgsl_to_spirv(ShaderSources sources);
ShaderSources glsl_to_spirv(ShaderSources sources);
RGAPI void UpdatePipeline(DescribedPipeline* pl);
RGAPI void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle);
RGAPI void ResetSyncState(cwoid);
RGAPI void CharCallback(void* window, unsigned int codePoint);
struct RGFW_window;

RGAPI WGPUSurface RGFW_GetWGPUSurface(void* instance, struct RGFW_window* window);
RGAPI WGPUSurface CreateSurfaceForWindow(SubWindow window);
RGAPI WGPUSurface CreateSurfaceForWindow_SDL2(void* windowHandle);
RGAPI WGPUSurface CreateSurfaceForWindow_SDL3(void* windowHandle);
RGAPI WGPUSurface CreateSurfaceForWindow_GLFW(void* windowHandle);
RGAPI WGPUSurface CreateSurfaceForWindow_RGFW(void* windowHandle);

struct InitContext_Impl;
typedef void* (*ContinuationPoint)(struct InitContext_Impl);
typedef struct InitContext_Impl{
    const char* windowTitle;
    int windowWidth, windowHeight;
    ContinuationPoint continuationPoint;
    void(*finalContinuationPoint)(SetupFunction, RenderFunction);
    void* wgpustate;
    SetupFunction setupFunction;
    RenderFunction renderFunction;
}InitContext_Impl;

static inline WGPUFilterMode toWGPUFilterMode(TextureFilter fm){
    switch(fm){
        case TEXTURE_FILTER_BILINEAR: return WGPUFilterMode_Linear;
        default: return WGPUFilterMode_Nearest;
    }
}
static inline WGPUMipmapFilterMode toWGPUMipmapFilterMode(TextureFilter fm){
    switch(fm){
        case TEXTURE_FILTER_BILINEAR: return WGPUMipmapFilterMode_Linear;
        default: return WGPUMipmapFilterMode_Nearest;
    }
}
static inline WGPUAddressMode toWGPUAddressMode(TextureWrap am){
    switch(am){
        case TEXTURE_WRAP_CLAMP: return WGPUAddressMode_ClampToEdge;
        case TEXTURE_WRAP_REPEAT: return WGPUAddressMode_Repeat;
        case TEXTURE_WRAP_MIRROR_REPEAT: return WGPUAddressMode_MirrorRepeat;
        case TEXTURE_WRAP_MIRROR_CLAMP: return WGPUAddressMode_ClampToEdge;
        default: return WGPUAddressMode_Repeat;
    }
}



#endif

// end file include/internals.hpp