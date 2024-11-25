#ifndef PIPELINE_H
#define PIPELINE_H
#include <webgpu/webgpu.h>
#include "macros_and_constants.h"

typedef struct Pipeline{
    WGPUBindGroupLayoutDescriptor bglDesc;
    WGPUVertexBufferLayout* vbLayouts;
    size_t vbLayoutCount;

    WGPURenderPipeline pipeline;
}Pipeline;
EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, Pipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, pl.pipeline);
    }
EXTERN_C_END
#endif// PIPELINE_H
