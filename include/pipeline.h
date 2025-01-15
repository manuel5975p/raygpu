#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdarg.h>
#include <webgpu/webgpu.h>
#include "macros_and_constants.h"

typedef enum uniform_type{
    uniform_buffer, storage_buffer, texture2d, storage_texture2d, sampler, texture3d, storage_texture3d
}uniform_type;

typedef enum access_type{
    readonly, readwrite, writeonly
}access_type;

typedef enum format_or_sample_type{
    sample_f32, sample_u32, format_r32float, format_r32uint, format_rgba8unorm
}format_or_sample_type;

typedef struct UniformDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
}UniformDescriptor;

//TODO: Stencil attachment
typedef struct RenderSettings{
    bool depthTest;
    bool faceCull;
    uint8_t sampleCount;
    WGPUCompareFunction depthCompare;
    WGPUFrontFace frontFace;

    WGPUBlendOperation blendOperationAlpha;
    WGPUBlendFactor    blendFactorSrcAlpha;
    WGPUBlendFactor    blendFactorDstAlpha;

    WGPUBlendOperation blendOperationColor;
    WGPUBlendFactor    blendFactorSrcColor;
    WGPUBlendFactor    blendFactorDstColor;
}RenderSettings;

static inline bool RenderSettingsComptatible(RenderSettings settings1, RenderSettings settings2){
    return settings1.sampleCount == settings2.sampleCount &&
           settings1.depthTest == settings2.depthTest;
}

typedef struct DescribedBindGroupLayout{
    WGPUBindGroupLayoutDescriptor descriptor;
    WGPUBindGroupLayoutEntry* entries;
    WGPUBindGroupLayout layout;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    WGPUBindGroupDescriptor desc;
    WGPUBindGroupEntry* entries;
    uint64_t releaseOnClear;
    WGPUBindGroup bindGroup;
    
    uint64_t descriptorHash;
    int needsUpdate;

}DescribedBindGroup;

typedef struct DescribedPipelineLayout{
    WGPUPipelineLayoutDescriptor descriptor;
    WGPUPipelineLayout layout;
}DescribedPipelineLayout;

typedef struct PipelineTriplet{
    WGPURenderPipeline pipeline; //TriangleList
    WGPURenderPipeline pipeline_TriangleStrip;
    WGPURenderPipeline pipeline_LineList;
}PipelineTriplet;

/**
 * @brief Hashmap: std::string -> UniformDescriptor, only visible for C++
 */
typedef struct StringToUniformMap StringToUniformMap;
/**
 * @brief Hashmap: std::pair<std::vector<AttributeAndResidence>, WGPUPrimitiveTopology> -> WGPURenderPipeline, only visible for C++
 */
typedef struct VertexStateToPipelineMap VertexStateToPipelineMap;



typedef struct DescribedPipeline{
    RenderSettings settings;

    //TODO: Multiple bindgrouplayouts
    WGPUShaderModule sh;
    DescribedBindGroup bindGroup;
    DescribedPipelineLayout layout;
    DescribedBindGroupLayout bglayout;
    WGPURenderPipelineDescriptor descriptor;
    WGPUVertexBufferLayout* vbLayouts;
    WGPUVertexAttribute* attributePool;
    WGPURenderPipeline pipeline; //TriangleList
    WGPURenderPipeline pipeline_TriangleStrip;
    WGPURenderPipeline pipeline_LineList;
    WGPUPrimitiveTopology lastUsedAs;

    WGPUBlendState* blendState;
    // WGPUVertexState not required as it's a nonpointer member of WGPURenderPipelineDescriptor
    WGPUFragmentState* fragmentState;
    WGPUColorTargetState* colorTarget;
    WGPUDepthStencilState* depthStencilState;

    StringToUniformMap* uniformLocations;
    VertexStateToPipelineMap* createdPipelines;
}DescribedPipeline;
typedef struct DescribedBuffer DescribedBuffer;
#ifdef __cplusplus
struct UniformAccessor{
    uint32_t index;
    DescribedBindGroup* bindgroup;
    void operator=(DescribedBuffer* buf);
};
#endif
typedef struct DescribedComputePipeline{
    WGPUComputePipelineDescriptor desc;
    WGPUComputePipeline pipeline;
    DescribedBindGroupLayout bglayout;
    DescribedBindGroup bindGroup;
    StringToUniformMap* uniformLocations;
    #ifdef __cplusplus
    UniformAccessor operator[](const char* uniformName);
    #endif
}DescribedComputePipeline;

typedef struct VertexArray VertexArray;
EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, pl.pipeline);
    }
    

    //DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);
    void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
    

    DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const WGPUBindGroupEntry* entries, size_t entryCount);
    WGPUBindGroup GetWGPUBindGroup(DescribedBindGroup* bg);
    
    void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, WGPUBindGroupEntry entry);
    void UpdateBindGroup(DescribedBindGroup* bg);
    void UnloadBindGroup(DescribedBindGroup* bg);
    
    DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
    DescribedComputePipeline* LoadComputePipeline(const char* shaderCode);
    DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const UniformDescriptor* uniforms, uint32_t uniformCount);
EXTERN_C_END
#endif// PIPELINE_H
