#ifndef PIPELINE_H
#define PIPELINE_H
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

typedef struct Pipeline{
    WGPUBindGroupLayoutDescriptor bglDesc;
    WGPUVertexBufferLayout* vbLayouts;
    size_t vbLayoutCount;
    
    WGPURenderPipeline pipeline;
}Pipeline;
#endif// PIPELINE_H
