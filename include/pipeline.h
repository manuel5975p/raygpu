#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdarg.h>
#include <webgpu/webgpu.h>
#include "macros_and_constants.h"

enum uniform_type{
    uniform_buffer, storage_buffer, texture2d, sampler
};

typedef struct UniformDescriptor{
    enum uniform_type type;
    uint32_t minBindingSize;
}UniformDescriptor;



typedef struct DescribedBindGroupLayout{
    WGPUBindGroupLayoutDescriptor descriptor;
    WGPUBindGroupLayoutEntry* entries;
    WGPUBindGroupLayout layout;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    WGPUBindGroupDescriptor desc;
    WGPUBindGroupEntry* entries;
    WGPUBindGroup bindGroup;
}DescribedBindGroup;

typedef struct DescribedPipeline{
    DescribedBindGroupLayout bglayout;
    WGPUVertexBufferLayout* vbLayouts;
    size_t vbLayoutCount;
    WGPURenderPipeline pipeline;
}DescribedPipeline;

EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, pl.pipeline);
    }
    WGPUDevice GetDevice(cwoid);

    DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);



    DescribedBindGroup LoadBindGroup(const DescribedPipeline* pipeline, const WGPUBindGroupEntry* entries, size_t entryCount);
    void UpdateBindGroup(DescribedBindGroup* bg, size_t index, WGPUBindGroupEntry entry);
    void UnloadBindGroup(DescribedBindGroup* bg);

EXTERN_C_END
#endif// PIPELINE_H
