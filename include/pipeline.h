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


#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdint.h>
#include <webgpu/webgpu.h>
#include <macros_and_constants.h>

typedef void* NativeShaderModuleHandle;
typedef void* NativeBindgroupLayoutHandle;
typedef void* NativeBindgroupHandle;
typedef void* NativePipelineLayoutHandle;
typedef void* NativeRenderPipelineHandle;
typedef void* NativeComputePipelineHandle;
typedef void* NativeSurfaceHandle;
typedef void* NativeSwapchainHandle;
typedef void* NativeBufferHandle;
typedef void* NativeMemoryHandle;
typedef void* NativeSamplerHandle;
typedef void* NativeImageHandle;
typedef void* NativeImageViewHandle;
typedef void* NativeCommandEncoderHandle;
typedef void* NativeRenderPassEncoderHandle;
typedef void* NativeComputePassEncoderHandle;

typedef enum uniform_type{
    uniform_buffer, storage_buffer, texture2d, storage_texture2d, texture_sampler, texture3d, storage_texture3d
}uniform_type;

typedef enum access_type{
    readonly, readwrite, writeonly
}access_type;

typedef enum format_or_sample_type{
    sample_f32, sample_u32, format_r32float, format_r32uint, format_rgba8unorm, format_rgba32float
}format_or_sample_type;

typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
}ResourceTypeDescriptor;

typedef struct ResourceDescriptor {
    void const * nextInChain; //hmm
    uint32_t binding;
    /*NULLABLE*/  NativeBufferHandle buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ NativeSamplerHandle sampler;
    /*NULLABLE*/ NativeImageViewHandle textureView;
} ResourceDescriptor;

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
    VertexStepMode_VertexBufferNotUsed = 0x00000000,
    VertexStepMode_Undefined = 0x00000001,
    VertexStepMode_Vertex = 0x00000002,
    VertexStepMode_Instance = 0x00000003,
    VertexStepMode_Force32 = 0x7FFFFFFF
} VertexStepMode;



//TODO: Stencil attachment
/**
 * @brief This struct handles the settings that GL handles with global functions
 * 
 * GL_DEPTH_TEST
 * GL_BLEND
 * GL_CULL_FACE
 * 
 */
typedef struct RenderSettings{
    bool depthTest;
    bool faceCull;
    uint8_t sampleCount                : 8;
    CompareFunction    depthCompare    : 8;
    BlendOperation blendOperationAlpha : 8;
    BlendFactor    blendFactorSrcAlpha : 8;
    BlendFactor    blendFactorDstAlpha : 8;
    BlendOperation blendOperationColor : 8;
    BlendFactor    blendFactorSrcColor : 8;
    BlendFactor    blendFactorDstColor : 8;
    FrontFace frontFace;

}RenderSettings;
/**
 * @brief This function determines compatibility between RenderSettings
 * @details 
 * The purpose of this function is to determine whether the attachment states of a renderpass and a pipeline is compatible.
 * For this, the multisample state and depth state need to match (and also stencil but not implemented right now) 
 * 
 * @param settings1 
 * @param settings2 
 * @return true 
 * @return false 
 */
static inline bool RenderSettingsComptatible(RenderSettings settings1, RenderSettings settings2){
    return settings1.sampleCount == settings2.sampleCount &&
           settings1.depthTest   == settings2.depthTest;
}

typedef struct DescribedBindGroupLayout{
    NativeBindgroupLayoutHandle layout;
    uint32_t entryCount;
    ResourceTypeDescriptor* entries;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    //Cached handles
    NativeBindgroupHandle bindGroup;
    NativeBindgroupLayoutHandle layout;
    int needsUpdate; //Cached handles valid?

    //Description: entryCount and actual entries
    uint32_t entryCount;
    ResourceDescriptor* entries;


    uint64_t releaseOnClear; //bitset
    uint64_t descriptorHash; //currently unused
    
}DescribedBindGroup;

typedef struct DescribedPipelineLayout{
    NativePipelineLayoutHandle layout;

    uint32_t bindgroupCount;
    DescribedBindGroupLayout bindgroupLayouts[4]; //4 is a reasonable max
}DescribedPipelineLayout;

typedef struct RenderPipelineQuartet{
    NativeRenderPipelineHandle pipeline_TriangleList;
    NativeRenderPipelineHandle pipeline_TriangleStrip;
    NativeRenderPipelineHandle pipeline_LineList;
    NativeRenderPipelineHandle pipeline_PointList;
}RenderPipelineQuartet;

/**
 * @brief Hashmap: std::string -> UniformDescriptor, only visible for C++
 */
typedef struct StringToUniformMap StringToUniformMap;
/**
 * @brief Hashmap: std::pair<std::vector<AttributeAndResidence>, WGPUPrimitiveTopology> -> PipelineTriplet, only visible for C++
 */
typedef struct VertexStateToPipelineMap VertexStateToPipelineMap;
typedef enum ShaderSourceType{
    sourceTypeSPIRV = 1,
    sourceTypeWGSL  = 2
}ShaderSourceType;

typedef struct DescribedShaderModule{

    /**
     * @brief source is Either binary uint32_t* for SPIR-V or printable char* for WGSL
     */
    const void* source;
    ShaderSourceType sourceType;
    size_t sourceLengthInBytes;

    NativeShaderModuleHandle shaderModule;
    StringToUniformMap* uniformLocations;
}DescribedShaderModule;

typedef struct VertexAttribute {
    VertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}VertexAttribute;

typedef struct VertexBufferLayout{
    uint64_t arrayStride;
    VertexStepMode stepMode;
    size_t attributeCount;
    VertexAttribute* attributes; //NOT owned, point to a VertexBufferLayoutSet::attributePool
}VertexBufferLayout;

typedef struct VertexBufferLayoutSet{
    uint32_t number_of_buffers;
    VertexBufferLayout* layouts;
    VertexAttribute* attributePool;
}VertexBufferLayoutSet;


typedef struct DescribedPipeline{
    RenderSettings settings;
    VertexBufferLayoutSet vertexLayout;

    //TODO: Multiple bindgrouplayouts
    DescribedShaderModule sh;
    DescribedBindGroup bindGroup;
    DescribedPipelineLayout layout;
    DescribedBindGroupLayout bglayout;

    //WGPUVertexBufferLayout* vbLayouts;
    //WGPUVertexAttribute* attributePool;
    //WGPURenderPipeline pipeline; //TriangleList
    //WGPURenderPipeline pipeline_TriangleStrip;
    //WGPURenderPipeline pipeline_LineList;
    //WGPUPrimitiveTopology lastUsedAs; //unused
    //WGPUBlendState* blendState;
    //WGPUFragmentState* fragmentState;
    //WGPUColorTargetState* colorTarget;
    //WGPUDepthStencilState* depthStencilState;
    RenderPipelineQuartet quartet;
    VertexStateToPipelineMap* createdPipelines;
}DescribedPipeline;

typedef struct Shader {
    DescribedPipeline* id;  // Pipeline
    int *locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
} Shader;



typedef struct DescribedBuffer DescribedBuffer;
#ifdef __cplusplus
struct UniformAccessor{
    uint32_t index;
    DescribedBindGroup* bindgroup;
    void operator=(DescribedBuffer* buf);
};
#endif
typedef struct DescribedComputePipeline{
    NativeComputePipelineHandle pipeline;
    NativePipelineLayoutHandle layout;
    DescribedBindGroupLayout bglayout;
    DescribedBindGroup bindGroup;
    DescribedShaderModule shaderModule;
    #ifdef __cplusplus
    UniformAccessor operator[](const char* uniformName);
    #endif
}DescribedComputePipeline;

typedef struct VertexArray VertexArray;
EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, (WGPURenderPipeline)pl.quartet.pipeline_TriangleList);
    }
    

    //DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);
    void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
    

    DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const ResourceDescriptor* entries, size_t entryCount);
    NativeBindgroupHandle GetWGPUBindGroup(DescribedBindGroup* bg);
    
    void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);
    void UpdateBindGroup(DescribedBindGroup* bg);
    void UnloadBindGroup(DescribedBindGroup* bg);
    
    DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
    DescribedComputePipeline* LoadComputePipeline(const char* shaderCode);
    DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount);
    /**
     * @brief Loads a shader given vertex and fragment GLSL source.
     * 
     * @param vertexSource 
     * @param fragmentSource 
     * @return Shader 
     */
    Shader LoadShaderFromMemory(const char* vertexSource, const char* fragmentSource);
EXTERN_C_END
#endif// PIPELINE_H
