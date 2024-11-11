#include "raygpu.h"
#include <cassert>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>
#include "wgpustate.inc"
#include <webgpu/webgpu_cpp.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__
wgpustate g_wgpustate;
Texture depthTexture; //TODO: uhhh move somewhere
WGPUDevice GetDevice(){
    return g_wgpustate.device;
}
WGPUQueue GetQueue(){
    return g_wgpustate.queue;
}
void rlColor4f(float r, float g, float b, float alpha){
    g_wgpustate.nextcol.x = r;
    g_wgpustate.nextcol.y = g;
    g_wgpustate.nextcol.z = b;
    g_wgpustate.nextcol.w = alpha;

}
void rlTexCoord2f(float u, float v){
    g_wgpustate.nextuv.x = u;
    g_wgpustate.nextuv.y = v;
}

void rlVertex2f(float x, float y){
    g_wgpustate.current_vertices.push_back(vertex{{x, y, 0}, g_wgpustate.nextuv, g_wgpustate.nextcol});
}
void rlVertex3f(float x, float y, float z){
    g_wgpustate.current_vertices.push_back(vertex{{x, y, z}, g_wgpustate.nextuv, g_wgpustate.nextcol});
}

void drawCurrentBatch(){
    if(g_wgpustate.current_vertices.size() == 0)return;
    
    updatePipeline(g_wgpustate.rstate, g_wgpustate.current_drawmode);
    switch(g_wgpustate.current_drawmode){
        case RL_TRIANGLES: [[fallthrough]];
        case RL_TRIANGLE_STRIP:{
            updateVertexBuffer(g_wgpustate.rstate, g_wgpustate.current_vertices.data(), g_wgpustate.current_vertices.size() * sizeof(vertex));
            size_t vcount = g_wgpustate.current_vertices.size();
            g_wgpustate.rstate->executeRenderpass([vcount](WGPURenderPassEncoder renderPass){
                wgpuRenderPassEncoderDraw(renderPass, vcount, 1, 0, 0);
            });
        } break;
        case RL_QUADS:{
            updateVertexBuffer(g_wgpustate.rstate, g_wgpustate.current_vertices.data(), g_wgpustate.current_vertices.size() * sizeof(vertex));
            WGPUBufferDescriptor bd{};
            bd.mappedAtCreation = false;
            bd.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
            bd.size = sizeof(uint32_t) * 6 * (g_wgpustate.current_vertices.size() / 4);
            WGPUBuffer ibuf = wgpuDeviceCreateBuffer(g_wgpustate.device, &bd);
            std::vector<uint32_t> indices;
            indices.reserve(6 * (g_wgpustate.current_vertices.size() / 4));
            for(size_t i = 0;i < (g_wgpustate.current_vertices.size() / 4);i++){
                indices.push_back(i * 4+0);
                indices.push_back(i * 4+1);
                indices.push_back(i * 4+3);
                indices.push_back(i * 4+1);
                indices.push_back(i * 4+3);
                indices.push_back(i * 4+2);
            }
            size_t bytesize = indices.size() * sizeof(uint32_t);
            size_t vcount = indices.size();
            wgpuQueueWriteBuffer(g_wgpustate.queue, ibuf, 0, indices.data(), indices.size() * sizeof(uint32_t));
            g_wgpustate.rstate->executeRenderpass([ibuf, bytesize, vcount](WGPURenderPassEncoder renderPass){
                wgpuRenderPassEncoderSetIndexBuffer(renderPass, ibuf, WGPUIndexFormat_Uint32, 0, bytesize);
                wgpuRenderPassEncoderDrawIndexed(renderPass, vcount, 1, 0, 0, 0);
            });
            wgpuBufferRelease(ibuf);
        } break;
        default:break;
    }
    g_wgpustate.current_vertices.clear();
}
uint32_t GetScreenWidth (cwoid){
    return g_wgpustate.width;
}
uint32_t GetScreenHeight(cwoid){
    return g_wgpustate.height;
}
void BeginDrawing(){
    glfwPollEvents();
    g_wgpustate.drawmutex.lock();
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);
    g_wgpustate.currentSurfaceTexture = surfaceTexture;
    WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
    g_wgpustate.currentSurfaceTextureView = nextTexture;
    setTargetTextures(g_wgpustate.rstate, nextTexture, g_wgpustate.rstate->depth);
    UseNoTexture();
    updateBindGroup(g_wgpustate.rstate);
    
}
void EndDrawing(){
    drawCurrentBatch();
    #ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(g_wgpustate.surface);
    #endif // __EMSCRIPTEN__
    //WGPUSurfaceTexture surfaceTexture;
    //wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);

    wgpuTextureRelease(g_wgpustate.currentSurfaceTexture.texture);
    wgpuTextureViewRelease(g_wgpustate.currentSurfaceTextureView);
    g_wgpustate.last_timestamps[g_wgpustate.total_frames % 32] = NanoTime();
    ++g_wgpustate.total_frames;
    g_wgpustate.drawmutex.unlock();
}
void rlBegin(draw_mode mode){
    if(g_wgpustate.current_drawmode != mode){
        drawCurrentBatch();
    }
    g_wgpustate.current_drawmode = mode;
}
void rlEnd(){

}
Image LoadImageFromTexture(Texture tex){
    #ifndef __EMSCRIPTEN__
    auto& device = g_wgpustate.device;
    Image ret {tex.format, tex.width, tex.height, nullptr};
    WGPUBufferDescriptor b{};
    b.mappedAtCreation = false;
    b.size = size_t(std::ceil(4.0 * tex.width / 256.0) * 256) * tex.height;
    b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    
    wgpu::Buffer readtex = wgpuDeviceCreateBuffer(device, &b);
    
    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = STRVIEW("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
    WGPUImageCopyTexture tbsource;
    tbsource.texture = tex.tex;
    tbsource.mipLevel = 0;
    tbsource.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    tbsource.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUImageCopyBuffer tbdest;
    tbdest.buffer = readtex.Get();
    tbdest.layout.offset = 0;
    tbdest.layout.bytesPerRow = std::ceil(4.0 * tex.width / 256.0) * 256;
    tbdest.layout.rowsPerImage = tex.height;

    WGPUExtent3D copysize{tex.width, tex.height, 1};
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &copysize);
    
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit(g_wgpustate.queue, 1, &command);
    wgpuCommandBufferRelease(command);
    #ifdef WEBGPU_BACKEND_DAWN
    wgpuDeviceTick(device);
    #endif
    std::pair<Image*, wgpu::Buffer*> ibpair{&ret, &readtex};
    auto onBuffer2Mapped = [&ibpair](wgpu::MapAsyncStatus status, const char* userdata){
        //std::cout << "Backcalled: " << status << std::endl;
        if(status != wgpu::MapAsyncStatus::Success){
            std::cout << userdata << std::endl;
        }
        assert(status == wgpu::MapAsyncStatus::Success);
        //std::pair<Image*, wgpu::Buffer*>* rei = (std::pair<Image*, wgpu::Buffer*>*)userdata;
        std::pair<Image*, wgpu::Buffer*>* rei = &ibpair;
        const void* map = wgpuBufferGetConstMappedRange(rei->second->Get(), 0, wgpuBufferGetSize(rei->second->Get()));
        rei->first->data = std::realloc(rei->first->data, wgpuBufferGetSize(rei->second->Get()));
        std::memcpy(rei->first->data, map, wgpuBufferGetSize(rei->second->Get()));
        rei->second->Unmap();
        wgpuBufferRelease(rei->second->Get());
        rei->second = nullptr;
        //wgpuBufferDestroy(*rei->second);
    };

    
    
    #ifndef __EMSCRIPTEN__
    WGPUFutureWaitInfo winfo{};
    //wgpu::BufferMapCallbackInfo cbinfo{};
    //cbinfo.callback = onBuffer2Mapped;
    //cbinfo.userdata = &ibpair;
    //cbinfo.mode = wgpu::CallbackMode::WaitAnyOnly;
    wgpu::Future fut = readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * tex.width / 256.0) * 256) * tex.height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped);
    winfo.future = fut;
    wgpuInstanceWaitAny(g_wgpustate.instance, 1, &winfo, 1000000000);
    #else 
    readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * tex.width / 256.0) * 256) * tex.height, onBuffer2Mapped, &ibpair);
    emscripten_sleep(20);
    #endif
    //wgpuBufferMapAsyncF(readtex, WGPUMapMode_Read, 0, 4 * tex.width * tex.height, onBuffer2Mapped);
    /*while(ibpair.first->data == nullptr){
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        WGPUCommandBufferDescriptor cmdBufferDescriptor2{};
        cmdBufferDescriptor.label = "Command buffer";
        WGPUCommandEncoderDescriptor commandEncoderDesc2{};
        commandEncoderDesc.label = "Command Encode2";
        WGPUCommandEncoder encoder2 = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
        WGPUCommandBuffer command2 = wgpuCommandEncoderFinish(encoder2, &cmdBufferDescriptor);
        wgpuQueueSubmit(g_wgpustate.queue, 1, &command2);
        wgpuCommandBufferRelease(command);
        #ifdef WEBGPU_BACKEND_DAWN
        wgpuDeviceTick(device);
        #endif
    }
    #ifdef WEBGPU_BACKEND_DAWN
    wgpuInstanceProcessEvents(g_wgpustate.instance);
    wgpuDeviceTick(device);
    #endif*/
    //readtex.Destroy();
    return ret;
    #else
    std::cerr << "LoadImageFromTexture not supported on web\n";
    return Image{tex.format, 0, 0, nullptr};
    #endif
}
Texture LoadTextureFromImage(Image img){
    Texture ret;
    WGPUTextureDescriptor desc = {
    nullptr,
    WGPUStringView{nullptr, 0},
    WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
    WGPUTextureDimension_2D,
    WGPUExtent3D{img.width, img.height, 1},
    img.format,
    1,1,1,nullptr};
        
    desc.viewFormats = &img.format;
    ret.tex = wgpuDeviceCreateTexture(g_wgpustate.device, &desc);
    WGPUTextureViewDescriptor vdesc{};
    vdesc.arrayLayerCount = 0;
    vdesc.aspect = WGPUTextureAspect_All;
    vdesc.format = desc.format;
    vdesc.dimension = WGPUTextureViewDimension_2D;
    vdesc.baseArrayLayer = 0;
    vdesc.arrayLayerCount = 1;
    vdesc.baseMipLevel = 0;
    vdesc.mipLevelCount = 1;
        
    WGPUImageCopyTexture destination;
    destination.texture = ret.tex;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUTextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * img.width;
    source.rowsPerImage = img.height;
    //wgpuQueueWriteTexture()
    wgpuQueueWriteTexture(g_wgpustate.queue, &destination, img.data,  4 * img.width * img.height, &source, &desc.size);
    ret.view = wgpuTextureCreateView(ret.tex, &vdesc);
    return ret;
}
uint64_t NanoTime(cwoid){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
double GetTime(cwoid){
    uint64_t nano_diff = NanoTime() - g_wgpustate.init_timestamp;

    return double(nano_diff) * 1e-9;
}
uint32_t GetFPS(cwoid){
    auto [minit, maxit] = std::minmax_element(std::begin(g_wgpustate.last_timestamps), std::end(g_wgpustate.last_timestamps));
    return uint32_t(std::round(32e9 / (*maxit - *minit)));
}
WGPUShaderModule LoadShaderFromMemory(const char* shaderSource) {
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    shaderCodeDesc.code = WGPUStringView{shaderSource, strlen(shaderSource)};
    shaderCodeDesc.code = WGPUStringView{shaderSource, strlen(shaderSource)};
    WGPUShaderModuleDescriptor shaderDesc{};
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    #ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    #endif
    return wgpuDeviceCreateShaderModule(g_wgpustate.device, &shaderDesc);
}
WGPUShaderModule LoadShader(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
    shaderCodeDesc.code = WGPUStringView{shaderSource.c_str(), shaderSource.size()};
    WGPUShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif
    return wgpuDeviceCreateShaderModule(g_wgpustate.device, &shaderDesc);
}
Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = 1;
    tDesc.format = format;
    tDesc.usage  = (WGPUTextureUsage_RenderAttachment * to_be_used_as_rendertarget) | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;

    WGPUTextureViewDescriptor textureViewDesc{};
    textureViewDesc.aspect = ((format == WGPUTextureFormat_Depth24Plus) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    Texture ret;
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.tex = wgpuDeviceCreateTexture(g_wgpustate.device, &tDesc);
    ret.view = wgpuTextureCreateView(ret.tex, &textureViewDesc);
    return ret;
}

Texture LoadTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_RGBA8Unorm, true);
}

Texture LoadDepthTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_Depth24Plus, true);
}
RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    return RenderTexture{.color = LoadTextureEx(width, height, g_wgpustate.frameBufferFormat, true), .depth = LoadDepthTexture(width, height)};
}
inline WGPUVertexFormat f16format(uint32_t s){
    switch(s){
        case 1:return WGPUVertexFormat_Float16x2  ;
        case 2:return WGPUVertexFormat_Float32x2;
        case 3:return WGPUVertexFormat_Float32x3;
        case 4:return WGPUVertexFormat_Float32x4;
        default: abort();
    }
    __builtin_unreachable();
}
inline WGPUVertexFormat f32format(uint32_t s){
    switch(s){
        case 1:return WGPUVertexFormat_Float32  ;
        case 2:return WGPUVertexFormat_Float32x2;
        case 3:return WGPUVertexFormat_Float32x3;
        case 4:return WGPUVertexFormat_Float32x4;
        default: abort();
    }
    __builtin_unreachable();
} 
void init_full_renderstate(full_renderstate* state, const WGPUShaderModule sh, const ShaderInputs shader_inputs, WGPUTextureView c, WGPUTextureView d){
    state->shader = sh;
    state->color = c;
    state->depth = d;
    state->vbo = 0;
    for(uint32_t i = 0;i < 8;i++){
        state->bgEntries[i].binding = i;
    }
    {
        std::vector<WGPUBindGroupLayoutEntry> blayouts(shader_inputs.uniform_count);
        state->bglayoutdesc = WGPUBindGroupLayoutDescriptor{};
        std::memset(blayouts.data(), 0, blayouts.size() * sizeof(WGPUBindGroupLayoutEntry));
        for(size_t i = 0;i < shader_inputs.uniform_count;i++){
            blayouts[i].binding = i;
            switch(shader_inputs.uniform_types[i]){
                case uniform_buffer:
                    blayouts[i].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
                    blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                    blayouts[i].buffer.minBindingSize = shader_inputs.uniform_minsizes[i];
                break;
                case storage_buffer:{
                    blayouts[i].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
                    blayouts[i].buffer.type = WGPUBufferBindingType_ReadOnlyStorage;
                    blayouts[i].buffer.minBindingSize = 0;
                }
                break;
                case texture2d:
                    blayouts[i].visibility = WGPUShaderStage_Fragment;
                    blayouts[i].texture.sampleType = WGPUTextureSampleType_Float;
                    blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
                break;
                case sampler:
                    blayouts[i].visibility = WGPUShaderStage_Fragment;
                    blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
                break;
                default:break;
            }
        }
        state->bglayoutdesc.entryCount = shader_inputs.uniform_count;
        state->bglayoutdesc.entries = blayouts.data();
        state->bglayout = wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &state->bglayoutdesc);
    }
    state->vbo = 0;
    state->bg = 0;
    std::memset(state->bgEntries, 0, 8 * sizeof(WGPUBindGroupEntry));
    {
        uint32_t offset = 0;
        //assert(vattribute_sizes.size() <= 8);
        for(uint32_t i = 0;i < shader_inputs.per_vertex_count;i++){
            state->attribs[i].shaderLocation = i;
            state->attribs[i].format = f32format(shader_inputs.per_vertex_sizes[i]);
            state->attribs[i].offset = offset * sizeof(float);
            offset += shader_inputs.per_vertex_sizes[i];
        }
        state->vlayout.attributeCount = shader_inputs.per_vertex_count;
        state->vlayout.attributes = state->attribs;
        state->vlayout.arrayStride = sizeof(vertex);
        state->vlayout.stepMode = WGPUVertexStepMode_Vertex;  
        updatePipeline(state, RL_TRIANGLES);
        updateRenderPassDesc(state);  
    }
}
void updateVertexBuffer(full_renderstate* state, const void* data, size_t size){
    if(state->vbo){
        wgpuBufferRelease(state->vbo);
    }
    WGPUBufferDescriptor bufferDesc{};
    WGPUBufferDescriptor mapBufferDesc{};
    
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
    bufferDesc.mappedAtCreation = false;

    state->vbo = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_wgpustate.queue, state->vbo, 0, data, size);
}
void setStateTexture(full_renderstate* state, uint32_t index, Texture tex){
    state->bgEntries[index] = WGPUBindGroupEntry{};
    state->bgEntries[index].binding = index;
    state->bgEntries[index].textureView = tex.view;
    updateBindGroup(state);
}
void setStateSampler(full_renderstate* state, uint32_t index, WGPUSampler sampler){
    state->bgEntries[index] = WGPUBindGroupEntry{};
    state->bgEntries[index].binding = index;
    state->bgEntries[index].sampler = sampler;
    //updateBindGroup(state);
}
void setStateUniformBuffer(full_renderstate* state, uint32_t index, const void* data, size_t size){
    if(state->bgEntries[index].buffer){
        wgpuBufferRelease(state->bgEntries[index].buffer);
    }
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_wgpustate.queue, uniformBuffer, 0, data, size);
    
    state->bgEntries[index] = WGPUBindGroupEntry{};
    state->bgEntries[index].binding = index;
    state->bgEntries[index].buffer = uniformBuffer;
    state->bgEntries[index].offset = 0;
    state->bgEntries[index].size = size;
    updateBindGroup(state);
    
}

void setStateStorageBuffer(full_renderstate* state, uint32_t index, const void* data, size_t size){
    if(state->bgEntries[index].buffer){
        wgpuBufferRelease(state->bgEntries[index].buffer);
    }
    WGPUBufferDescriptor bufferDesc{};
    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer storageBuffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_wgpustate.queue, storageBuffer, 0, data, size);
    
    state->bgEntries[index] = WGPUBindGroupEntry{};
    state->bgEntries[index].binding = index;
    state->bgEntries[index].buffer = storageBuffer;
    state->bgEntries[index].offset = 0;
    state->bgEntries[index].size = size;
    updateBindGroup(state);
}
void SetTexture       (uint32_t index, Texture tex){
    setStateTexture(g_wgpustate.rstate, index, tex);
}
void SetSampler       (uint32_t index, WGPUSampler sampler){
    setStateSampler(g_wgpustate.rstate, index, sampler);
}
void SetUniformBuffer (uint32_t index, const void* data, size_t size){
    setStateUniformBuffer(g_wgpustate.rstate, index, data, size);
}
void SetStorageBuffer (uint32_t index, const void* data, size_t size){
    setStateStorageBuffer(g_wgpustate.rstate, index, data, size);
}
void updateBindGroup(full_renderstate* state){
    WGPUBindGroupDescriptor bgdesc{};
    bgdesc.entryCount = state->bglayoutdesc.entryCount;

    for(uint32_t i = 0;i < state->bglayoutdesc.entryCount;i++){
        if(state->bgEntries[i].binding != i){
            return;
        }
    }
    bgdesc.entries = state->bgEntries;
    bgdesc.layout = state->bglayout;
    if(state->bg)wgpuBindGroupRelease(state->bg);
    state->bg = wgpuDeviceCreateBindGroup(g_wgpustate.device, &bgdesc);
}
void updatePipeline(full_renderstate* state, draw_mode drawmode){
    state->pipelineDesc = WGPURenderPipelineDescriptor{};
    state->pipelineDesc.vertex.bufferCount = 1;
    state->pipelineDesc.vertex.buffers = &state->vlayout;
    state->pipelineDesc.vertex.module = state->shader;
    state->pipelineDesc.vertex.entryPoint = STRVIEW("vs_main");
    state->pipelineDesc.vertex.constantCount = 0;
    state->pipelineDesc.vertex.constants = nullptr;
    switch(drawmode){
        case draw_mode::RL_QUADS:{
            state->pipelineDesc.primitive.topology =         WGPUPrimitiveTopology_TriangleList;
            state->pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        }break;
        case draw_mode::RL_TRIANGLES:{
            state->pipelineDesc.primitive.topology =         WGPUPrimitiveTopology_TriangleList;
            state->pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;

        }break;
        case draw_mode::RL_TRIANGLE_STRIP:{
            state->pipelineDesc.primitive.topology =         WGPUPrimitiveTopology_TriangleStrip;
            state->pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Uint32;
        }break;
    }
    
    state->pipelineDesc.primitive.frontFace =        WGPUFrontFace_CCW;
    state->pipelineDesc.primitive.cullMode =         WGPUCullMode_None;
    WGPUFragmentState fragmentState{};
    state->pipelineDesc.fragment = &fragmentState;
    fragmentState.module = state->shader;
    fragmentState.entryPoint = STRVIEW("fs_main");
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    WGPUBlendState blendState{};
    blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blendState.color.operation = WGPUBlendOperation_Add;
    blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
    blendState.alpha.dstFactor = WGPUBlendFactor_One;
    blendState.alpha.operation = WGPUBlendOperation_Add;
    WGPUColorTargetState colorTarget{};
    colorTarget.format = g_wgpustate.frameBufferFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = WGPUColorWriteMask_All;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    // We setup a depth buffer state for the render pipeline
    WGPUDepthStencilState depthStencilState{};
    // Keep a fragment only if its depth is lower than the previously blended one
    depthStencilState.depthCompare = WGPUCompareFunction_Less;
    // Each time a fragment is blended into the target, we update the value of the Z-buffer
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
    // Store the format in a variable as later parts of the code depend on it
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
    depthStencilState.format = depthTextureFormat;
    // Deactivate the stencil alltogether
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
    depthStencilState.stencilBack.compare = WGPUCompareFunction_Always;
    state->pipelineDesc.depthStencil = &depthStencilState;
    state->pipelineDesc.multisample.count = 1;
    state->pipelineDesc.multisample.mask = ~0u;
    state->pipelineDesc.multisample.alphaToCoverageEnabled = false;
    // Create a bind group layout
    // Create the pipeline layout
    WGPUPipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = 1;
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&state->bglayout;
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(g_wgpustate.device, &layoutDesc);
    state->pipelineDesc.layout = layout;
    state->pipeline = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &state->pipelineDesc);
    wgpuPipelineLayoutRelease(layout);
}
void updateRenderPassDesc(full_renderstate* state){
    state->renderPassDesc = WGPURenderPassDescriptor{};
    state->rca = WGPURenderPassColorAttachment{};
    state->rca.view = state->color;
    state->rca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    #ifdef __EMSCRIPTEN__
    state->rca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    #endif
    //std::cout << rca.depthSlice << "\n";
    state->rca.resolveTarget = nullptr;
    state->rca.loadOp =  WGPULoadOp_Load;
    state->rca.storeOp = WGPUStoreOp_Store;
    state->rca.clearValue = WGPUColor{ 0.01, 0.01, 0.2, 1.0 };
    state->renderPassDesc.colorAttachmentCount = 1;
    state->renderPassDesc.colorAttachments = &state->rca;
    // We now add a depth/stencil attachment:
    state->dsa = WGPURenderPassDepthStencilAttachment{};
    // The view of the depth texture
    state->dsa.view = state->depth;
    //dsa.depthSlice = 0;
    // The initial value of the depth buffer, meaning "far"
    state->dsa.depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    state->dsa.depthLoadOp = WGPULoadOp_Clear;
    state->dsa.depthStoreOp = WGPUStoreOp_Store;
    // we could turn off writing to the depth buffer globally here
    state->dsa.depthReadOnly = false;
    // Stencil setup, mandatory but unused
    state->dsa.stencilClearValue = 0;
    #ifdef WEBGPU_BACKEND_WGPU
    state->dsa.stencilLoadOp =  WGPULoadOp_Load;
    state->dsa.stencilStoreOp = WGPUStoreOp_Store;
    #else
    state->dsa.stencilLoadOp = WGPULoadOp_Undefined;
    state->dsa.stencilStoreOp = WGPUStoreOp_Undefined;
    #endif
    state->dsa.stencilReadOnly = true;
    state->renderPassDesc.depthStencilAttachment = &state->dsa;
    state->renderPassDesc.timestampWrites = nullptr;
}
void setTargetTextures(full_renderstate* state, WGPUTextureView c, WGPUTextureView d){
    state->color = c;
    state->depth = d;
    updateRenderPassDesc(state);
}
Image LoadImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){
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
void UseTexture(Texture tex){
    if(g_wgpustate.activeTexture.tex != tex.tex){
        drawCurrentBatch();
    }

    g_wgpustate.activeTexture = tex;
    setStateTexture(g_wgpustate.rstate, 1, tex);
}
void UseNoTexture(){
    if(g_wgpustate.activeTexture.tex != g_wgpustate.whitePixel.tex){
        drawCurrentBatch();
    }
    setStateTexture(g_wgpustate.rstate, 1, g_wgpustate.whitePixel);
    
}
/*template<typename callable>
void executeRenderpass(callable&& c){
    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    commandEncoderDesc.label = "Command Encoder";
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device, &commandEncoderDesc);
    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bg, 0, 0);
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, this->vbo, 0, wgpuBufferGetSize(vbo));
    c(renderPass);
    wgpuRenderPassEncoderEnd(renderPass);
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = "Command buffer";
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(g_wgpustate.queue, 1, &command);
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(command);
}*/


void BeginTextureMode(RenderTexture rtex){
    setTargetTextures(g_wgpustate.rstate, rtex.color.view, rtex.depth.view);
}
void EndTextureMode(){
    drawCurrentBatch();
    setTargetTextures(g_wgpustate.rstate, g_wgpustate.currentSurfaceTextureView, depthTexture.view);
}