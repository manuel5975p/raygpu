#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdarg.h>
#include <webgpu/webgpu.h>
#include "macros_and_constants.h"

enum uniform_type{
    uniform_buffer, storage_buffer, texture2d, sampler, storage_write_buffer
};

typedef struct UniformDescriptor{
    enum uniform_type type;
    uint32_t minBindingSize;
}UniformDescriptor;


typedef struct RenderSettings{
    uint8_t depthTest;
    uint8_t faceCull;
    uint8_t sampleCount_onlyApplicableIfMoreThanOne;
    WGPUCompareFunction depthCompare;
    WGPUFrontFace frontFace;

    WGPUTextureView optionalDepthTexture; //Depth texture (pointer), applicable if depthTest != 0

}RenderSettings;

typedef struct DescribedBindGroupLayout{
    WGPUBindGroupLayoutDescriptor descriptor;
    WGPUBindGroupLayoutEntry* entries;
    WGPUBindGroupLayout layout;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    WGPUBindGroupDescriptor desc;
    WGPUBindGroupEntry* entries;
    WGPUBindGroup bindGroup;
    
    uint64_t descriptorHash;
    bool needsUpdate;

}DescribedBindGroup;

typedef struct DescribedPipelineLayout{
    WGPUPipelineLayoutDescriptor descriptor;
    WGPUPipelineLayout layout;
}DescribedPipelineLayout;

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
}DescribedPipeline;
typedef struct DescribedComputePipeline{
    WGPUComputePipelineDescriptor desc;
    WGPUComputePipeline pipeline;
    DescribedBindGroupLayout bglayout;
}DescribedComputePipeline;

typedef struct VertexArray VertexArray;
EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, pl.pipeline);
    }
    WGPUDevice GetDevice(cwoid);

    //DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);
    void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
    

    DescribedBindGroup LoadBindGroup(const DescribedPipeline* pipeline, const WGPUBindGroupEntry* entries, size_t entryCount);
    WGPUBindGroup GetWGPUBindGroup(DescribedBindGroup* bg);
    
    void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, WGPUBindGroupEntry entry);
    void UpdateBindGroup(DescribedBindGroup* bg);
    void UnloadBindGroup(DescribedBindGroup* bg);
    
    DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
    DescribedComputePipeline* LoadComputePipeline(const char* shaderCode, const UniformDescriptor* uniforms, uint32_t uniformCount);
EXTERN_C_END
#endif// PIPELINE_H
