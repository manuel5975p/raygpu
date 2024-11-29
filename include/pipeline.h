#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdarg.h>
#include <webgpu/webgpu.h>
#include "macros_and_constants.h"

typedef struct Pipeline{
    WGPUBindGroupLayoutDescriptor bglDesc;
    WGPUBindGroupLayout bgl;
    WGPUVertexBufferLayout* vbLayouts;
    size_t vbLayoutCount;
    WGPURenderPipeline pipeline;
}Pipeline;

typedef struct BindGroup{
    WGPUBindGroupDescriptor desc;
    WGPUBindGroupEntry* entries;
    WGPUBindGroup bindGroup;
}BindGroup;
EXTERN_C_BEGIN
    inline void UsePipeline(WGPURenderPassEncoder rpEncoder, Pipeline pl){
        wgpuRenderPassEncoderSetPipeline(rpEncoder, pl.pipeline);
    }
    WGPUDevice GetDevice(cwoid);

    inline BindGroup LoadBindGroup(const Pipeline* pipeline, const WGPUBindGroupEntry* entries, size_t entryCount){
        BindGroup ret zeroinit;
        WGPUBindGroupEntry* rentries = (WGPUBindGroupEntry*) calloc(entryCount, sizeof(WGPUBindGroupEntry));
        memcpy(rentries, entries, entryCount * sizeof(WGPUBindGroupEntry));
        ret.entries = rentries;
        ret.desc.layout;
        ret.desc.entries = ret.entries;
        ret.bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &ret.desc);
        return ret;
    }
    inline void UpdateBindGroup(BindGroup* bg, size_t index, WGPUBindGroupEntry entry){
        bg->entries[index] = entry;

        //TODO don't release and recreate here
        wgpuBindGroupRelease(bg->bindGroup);
        bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
    }
    inline void UnloadBindGroup(BindGroup* bg){
        free(bg->entries);
        wgpuBindGroupRelease(bg->bindGroup);
    }

EXTERN_C_END
#endif// PIPELINE_H
