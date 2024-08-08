#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP
#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <iostream>
#include <webgpu/webgpu.h>
#include <cstdlib>
#include "raygpu.h"

extern wgpustate g_wgpustate;


struct vertex_buffer{
    WGPUVertexBufferLayout layout;
    WGPUBuffer buffer;
    std::vector<WGPUVertexAttribute> attribs;
    size_t size;

    vertex_buffer(const std::vector<vertex>& data, const std::vector<uint32_t>& attribute_sizes) : layout{}, buffer{}{
        using dtype = typename std::remove_reference_t<decltype(data)>::value_type;
        attribs.resize(attribute_sizes.size());
        uint32_t offset = 0;
        for(uint32_t i = 0;i < attribs.size();i++){
            attribs[i].shaderLocation = i;
            attribs[i].format = f32format(attribute_sizes[i]);
            attribs[i].offset = offset * sizeof(float);
            offset += attribute_sizes[i];
        }
        layout.attributeCount = attribs.size();
        layout.attributes = attribs.data();
        layout.arrayStride = sizeof(dtype);
        layout.stepMode = WGPUVertexStepMode_Vertex;

        WGPUBufferDescriptor bd;
        bd.mappedAtCreation = false;
        bd.size = data.size() * sizeof(dtype);
        size = bd.size;
        bd.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        buffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bd);
        wgpuQueueWriteBuffer(g_wgpustate.queue, buffer, 0, data.data(), data.size() * sizeof(dtype));
    }
};
struct uniform_buffer{
    WGPUBuffer buffer;
    size_t size;
    uniform_buffer(std::vector<float> data){
        WGPUBufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float) * data.size();
        size = sizeof(float) * data.size();
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        bufferDesc.mappedAtCreation = false;
        buffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    }
};


inline Image LoadImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){
    Image ret{WGPUTextureFormat_RGBA8Unorm, width, height, std::calloc(width * height, sizeof(Color))};
    for(uint32_t i = 0;i < height;i++){
        for(uint32_t j = 0;j < width;j++){
            const size_t index = size_t(i) * width + j;
            const size_t ic = i * checkerCount / height;
            const size_t jc = j * checkerCount / width;
            static_cast<Color*>(ret.data)[index] = ((ic ^ jc) & 1) ? a : b;
        }
    }
    return ret;
}



inline Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = 1;
    tDesc.format = format;
    tDesc.usage  = (WGPUTextureUsage_RenderAttachment * to_be_used_as_rendertarget) | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
    tDesc.viewFormatCount = 0;
    tDesc.viewFormats = nullptr;

    WGPUTextureViewDescriptor textureViewDesc{};
    textureViewDesc.aspect = ((format == WGPUTextureFormat_Depth24Plus) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    return Texture(tDesc, textureViewDesc);
}

inline Texture LoadTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_RGBA8Unorm, true);
}

inline Texture LoadDepthTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_Depth24Plus, true);
}

inline RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    return RenderTexture{.color = LoadTexture(width, height), .depth = LoadDepthTexture(width, height)};
}

struct render_pipeline{
    WGPURenderPipeline pl;
    WGPURenderPipelineDescriptor desc;
};

struct render_pass{
    WGPURenderPassDescriptor desc;
    WGPUTextureView color, depth;
    WGPURenderPassColorAttachment colorAttachment;
    WGPURenderPassDepthStencilAttachment depthAttachment;
    render_pass(WGPUTextureView c, WGPUTextureView d): color(c), depth(d){

    }
};

struct ass{
    union{
        float x;
        int y;
    };
};



};
#endif
