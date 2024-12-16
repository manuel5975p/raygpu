#include "raygpu.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdarg>
#include <thread>
#include "wgpustate.inc"
#include <webgpu/webgpu_cpp.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__
wgpustate g_wgpustate;
#include <stb_image_write.h>
#include <stb_image.h>
#include <sinfl.h>

Vector2 nextuv;
Vector4 nextcol;
vertex* vboptr;
vertex* vboptr_base;
//DescribedBuffer vbomap;


typedef struct VertexArray{
    std::vector<AttributeAndResidence> attributes;
    std::vector<std::pair<DescribedBuffer, WGPUVertexStepMode>> buffers;
    void add(DescribedBuffer buffer, uint32_t shaderLocation, WGPUVertexFormat fmt, uint32_t offset, WGPUVertexStepMode stepmode){
        AttributeAndResidence insert{};
        for(size_t i = 0;i < buffers.size();i++){
            auto& _buffer = buffers[i];
            if(_buffer.first.buffer == buffer.buffer){
                if(_buffer.second == stepmode){
                    insert.bufferSlot = i;
                    insert.stepMode = stepmode;
                    insert.attr.format = fmt;
                    insert.attr.offset = offset;
                    insert.attr.shaderLocation = shaderLocation;
                    attributes.push_back(insert);
                    return;
                }
                else{
                    std::cerr << "Mixed stepmodes not implemented yet\n";
                    exit(1);
                }
            }
        }
        insert.bufferSlot = buffers.size();
        buffers.push_back({buffer, stepmode});
        insert.stepMode = stepmode;
        insert.attr.format = fmt;
        insert.attr.offset = offset;
        insert.attr.shaderLocation = shaderLocation;
        attributes.push_back(insert);
    }
}VertexArray;
extern "C" VertexArray* LoadVertexArray(){
    return new VertexArray;
}
extern "C" void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, WGPUVertexFormat format, uint32_t offset, WGPUVertexStepMode stepmode){
    array->add(*buffer, attribLocation, format, offset, stepmode);
}
extern "C" void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va){
    pipeline->vbLayouts = (WGPUVertexBufferLayout*) malloc(va->buffers.size() * sizeof(WGPUVertexBufferLayout));
    
    pipeline->descriptor.vertex.buffers = pipeline->vbLayouts;
    pipeline->descriptor.vertex.bufferCount = va->buffers.size();

    //LoadPipelineEx
    uint32_t attribCount = va->attributes.size();
    auto& attribs = va->attributes;
    uint32_t maxslot = 0;
    for(size_t i = 0;i < attribCount;i++){
        maxslot = std::max(maxslot, attribs[i].bufferSlot);
    }
    const uint32_t number_of_buffers = maxslot + 1;
    std::vector<std::vector<WGPUVertexAttribute>> buffer_to_attributes(number_of_buffers);
    //WGPUVertexBufferLayout* vbLayouts = new WGPUVertexBufferLayout[number_of_buffers];
    std::vector<uint32_t> strides  (number_of_buffers, 0);
    std::vector<uint32_t> attrIndex(number_of_buffers, 0);
    for(size_t i = 0;i < attribCount;i++){
        buffer_to_attributes[attribs[i].bufferSlot].push_back(attribs[i].attr);
        strides[attribs[i].bufferSlot] += attributeSize(attribs[i].attr.format);
    }
    
    for(size_t i = 0;i < number_of_buffers;i++){
        pipeline->vbLayouts[i].attributes = buffer_to_attributes[i].data();
        pipeline->vbLayouts[i].attributeCount = buffer_to_attributes[i].size();
        pipeline->vbLayouts[i].arrayStride = strides[i];
        pipeline->vbLayouts[i].stepMode = va->buffers[i].second;
    }
    wgpuRenderPipelineRelease(pipeline->pipeline);
    pipeline->pipeline = wgpuDeviceCreateRenderPipeline(GetDevice(), &pipeline->descriptor);
}
extern "C" void BindVertexArray(DescribedPipeline* pipeline, VertexArray* va){
    for(size_t i = 0;i < va->buffers.size();i++){
        auto& firstbuffer = va->buffers[0].first.buffer;
        wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, va->buffers[0].first.buffer, 0, va->buffers[0].first.descriptor.size);
    }
}
extern "C" void EnableVertexAttribArray(VertexArray* array, uint32_t attribLocation){
    return;
}
extern "C" void DrawArrays(uint32_t vertexCount){
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    wgpuRenderPassEncoderDraw(rp, vertexCount, 1, 0, 0);
}
Texture depthTexture; //TODO: uhhh move somewhere
WGPUDevice GetDevice(){
    return g_wgpustate.device;
}
WGPUQueue GetQueue(){
    return g_wgpustate.queue;
}


void drawCurrentBatch(){
    size_t vertexCount = vboptr - vboptr_base;
    //std::cout << "vcoun = " << vertexCount << "\n";
    if(vertexCount == 0)return;
    //std::cout << "vboptr reset" << std::endl;
    
    UpdateBindGroup(&g_wgpustate.rstate->currentPipeline->bindGroup);
    switch(g_wgpustate.current_drawmode){
        case RL_LINES:{
            DescribedBuffer vbo;
            bool allocated_via_pool = false;
            if(vertexCount <= 8 && !g_wgpustate.smallBufferPool.empty()){
                allocated_via_pool = true;
                vbo = g_wgpustate.smallBufferPool.back();
                g_wgpustate.smallBufferPool.pop_back();
                wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            }
            else{
                vbo = GenBuffer(vboptr_base, vertexCount * sizeof(vertex));
            }
            vboptr = vboptr_base;
            BindPipeline(g_wgpustate.rstate->currentPipeline, WGPUPrimitiveTopology_LineList);
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            wgpuRenderPassEncoderDraw(g_wgpustate.rstate->renderpass.rpEncoder, vertexCount, 1, 0, 0);
            if(!allocated_via_pool){
                wgpuBufferRelease(vbo.buffer);
            }
            else{
                g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            }
        }break;
        case RL_TRIANGLES: [[fallthrough]];
        case RL_TRIANGLE_STRIP:{
            DescribedBuffer vbo;
            bool allocated_via_pool = false;
            if(vertexCount <= 8 && !g_wgpustate.smallBufferPool.empty()){
                allocated_via_pool = true;
                vbo = g_wgpustate.smallBufferPool.back();
                g_wgpustate.smallBufferPool.pop_back();
                wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            }
            else{
                vbo = GenBuffer(vboptr_base, vertexCount * sizeof(vertex));
            }
            vboptr = vboptr_base;
            BindPipeline(g_wgpustate.rstate->currentPipeline, WGPUPrimitiveTopology_TriangleList);
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            wgpuRenderPassEncoderDraw(g_wgpustate.rstate->renderpass.rpEncoder, vertexCount, 1, 0, 0);
            if(!allocated_via_pool){
                wgpuBufferRelease(vbo.buffer);
            }
            else{
                g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            }
        } break;
        case RL_QUADS:{
            DescribedBuffer vbo;
            bool allocated_via_pool = false;
            if(vertexCount < 8 && !g_wgpustate.smallBufferPool.empty()){
                allocated_via_pool = true;
                vbo = g_wgpustate.smallBufferPool.back();
                g_wgpustate.smallBufferPool.pop_back();
                wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            }
            else{
                vbo = GenBuffer(vboptr_base, vertexCount * sizeof(vertex));
            }
            const size_t quadCount = vertexCount / 4;
            
            if(g_wgpustate.quadindicesCache.descriptor.size < 6 * quadCount * sizeof(uint32_t)){
                std::vector<uint32_t> indices(6 * quadCount);
                for(size_t i = 0;i < quadCount;i++){
                    indices[i * 6 + 0] = (i * 4 + 0);
                    indices[i * 6 + 1] = (i * 4 + 1);
                    indices[i * 6 + 2] = (i * 4 + 3);
                    indices[i * 6 + 3] = (i * 4 + 1);
                    indices[i * 6 + 4] = (i * 4 + 2);
                    indices[i * 6 + 5] = (i * 4 + 3);
                }
                if(g_wgpustate.quadindicesCache.buffer)wgpuBufferRelease(g_wgpustate.quadindicesCache.buffer);
                g_wgpustate.quadindicesCache = GenBufferEx(indices.data(), 6 * quadCount * sizeof(uint32_t), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
            }
            const DescribedBuffer& ibuf = g_wgpustate.quadindicesCache;
            BindPipeline(g_wgpustate.rstate->currentPipeline, WGPUPrimitiveTopology_TriangleList);
            //wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, vertexCount * sizeof(vertex));
            wgpuRenderPassEncoderSetIndexBuffer (g_wgpustate.rstate->renderpass.rpEncoder, ibuf.buffer, WGPUIndexFormat_Uint32, 0, quadCount * 6 * sizeof(uint32_t));
            wgpuRenderPassEncoderDrawIndexed    (g_wgpustate.rstate->renderpass.rpEncoder, quadCount * 6, 1, 0, 0, 0);
            
            if(!allocated_via_pool){
                wgpuBufferRelease(vbo.buffer);
            }
            else{
                g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            }
            vboptr = vboptr_base;
            
        } break;
        default:break;
    }
    vboptr = vboptr_base;
}

extern "C" void BeginPipelineMode(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode){
    drawCurrentBatch();
    g_wgpustate.rstate->currentPipeline = pipeline;
    BindPipeline(pipeline, drawMode);
}
extern "C" void EndPipelineMode(){
    drawCurrentBatch();
    g_wgpustate.rstate->currentPipeline = g_wgpustate.rstate->defaultPipeline;
    BindPipeline(g_wgpustate.rstate->currentPipeline, g_wgpustate.rstate->currentPipeline->lastUsedAs);
}
extern "C" void BindPipeline(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode){
    
    switch(drawMode){
        case WGPUPrimitiveTopology_TriangleList:
        wgpuRenderPassEncoderSetPipeline (g_wgpustate.rstate->renderpass.rpEncoder, pipeline->pipeline);
        break;
        case WGPUPrimitiveTopology_TriangleStrip:
        wgpuRenderPassEncoderSetPipeline (g_wgpustate.rstate->renderpass.rpEncoder, pipeline->pipeline_TriangleStrip);
        break;
        case WGPUPrimitiveTopology_LineList:
        wgpuRenderPassEncoderSetPipeline (g_wgpustate.rstate->renderpass.rpEncoder, pipeline->pipeline_LineList);
        break;
        default:
            assert(false && "Unsupported Drawmode");
            abort();
    }
    pipeline->lastUsedAs = drawMode;
    wgpuRenderPassEncoderSetBindGroup (g_wgpustate.rstate->renderpass.rpEncoder, 0, GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);

}


uint32_t GetScreenWidth (cwoid){
    return g_wgpustate.width;
}
uint32_t GetScreenHeight(cwoid){
    return g_wgpustate.height;
}
uint32_t GetMonitorWidth (cwoid){
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        glfwInit();
        return GetMonitorWidth();
    }
    return mode->width;
}
uint32_t GetMonitorHeight(cwoid){
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        glfwInit();
        return GetMonitorWidth();
    }
    return mode->height;
}
void BeginRenderpassEx(DescribedRenderpass* renderPass){
    WGPUCommandEncoderDescriptor desc{};
    desc.label = STRVIEW("another cmdencoder");
    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &desc);
    renderPass->rpEncoder = wgpuCommandEncoderBeginRenderPass(renderPass->cmdEncoder, &renderPass->renderPassDesc);
    g_wgpustate.rstate->activeRenderPass = renderPass;
}

void EndRenderpassEx(DescribedRenderpass* renderPass){
    wgpuRenderPassEncoderEnd(renderPass->rpEncoder);
    g_wgpustate.rstate->activeRenderPass = nullptr;
    renderPass->rpEncoder = 0;
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(renderPass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(renderPass->cmdEncoder);
}
void BeginRenderpass(cwoid){
    BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
}
void EndRenderpass(cwoid){
    EndRenderpassEx(&g_wgpustate.rstate->renderpass);
}
extern "C" void ClearBackground(Color clearColor){
    bool rpActive = g_wgpustate.rstate->activeRenderPass != nullptr;
    DescribedRenderpass* backup = g_wgpustate.rstate->activeRenderPass;
    if(rpActive){
        EndRenderpassEx(g_wgpustate.rstate->activeRenderPass);
    }
    g_wgpustate.rstate->clearPass.rca->clearValue = WGPUColor{clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0};
    BeginRenderpassEx(&g_wgpustate.rstate->clearPass);
    EndRenderpassEx(&g_wgpustate.rstate->clearPass);
    if(rpActive){
        BeginRenderpassEx(backup);
    }

}
void BeginDrawing(){
    g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] = NanoTime();
    glfwPollEvents();
    g_wgpustate.drawmutex.lock();
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);
    g_wgpustate.currentSurfaceTexture = surfaceTexture;
    WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
    g_wgpustate.currentSurfaceTextureView = nextTexture;
    setTargetTextures(g_wgpustate.rstate, nextTexture, g_wgpustate.rstate->depth);
    BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
    SetUniformBuffer(0,&g_wgpustate.defaultScreenMatrix);
    //EndRenderPass(&g_wgpustate.rstate->clearPass);
    //BeginRenderPass(&g_wgpustate.rstate->renderpass);
    //UseNoTexture();
    //updateBindGroup(g_wgpustate.rstate);
}
void EndDrawing(){
    drawCurrentBatch();
    EndRenderpassEx(&g_wgpustate.rstate->renderpass);
    #ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(g_wgpustate.surface);
    #endif // __EMSCRIPTEN__
    //WGPUSurfaceTexture surfaceTexture;
    //wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);

    
    wgpuTextureRelease(g_wgpustate.currentSurfaceTexture.texture);
    wgpuTextureViewRelease(g_wgpustate.currentSurfaceTextureView);
    uint64_t beginframe_stmp = g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64];
    ++g_wgpustate.total_frames;
    std::copy(g_wgpustate.smallBufferRecyclingBin.begin(), g_wgpustate.smallBufferRecyclingBin.end(), std::back_inserter(g_wgpustate.smallBufferPool));
    g_wgpustate.smallBufferRecyclingBin.clear();
    std::copy(g_wgpustate.keydown.begin(), g_wgpustate.keydown.end(), g_wgpustate.keydownPrevious.begin());
    g_wgpustate.mousePosPrevious = g_wgpustate.mousePos;
    g_wgpustate.globalScrollXPrevious = g_wgpustate.globalScrollX;
    g_wgpustate.globalScrollYPrevious = g_wgpustate.globalScrollY;
    std::copy(g_wgpustate.mouseButtonDown.begin(), g_wgpustate.mouseButtonDown.end(), g_wgpustate.mouseButtonDownPrevious.begin());
    
    g_wgpustate.drawmutex.unlock();
    uint64_t nanosecondsPerFrame = std::floor(1e9 / GetTargetFPS());
    //std::cout << nanosecondsPerFrame << "\n";
    uint64_t elapsed = NanoTime() - beginframe_stmp;
    if(elapsed & (1ull << 63))return;
    if(!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) && nanosecondsPerFrame > elapsed)
        NanoWait(nanosecondsPerFrame - elapsed);
    //std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondsPerFrame - elapsed));
}
void rlBegin(draw_mode mode){
    if(g_wgpustate.current_drawmode != mode){
        drawCurrentBatch();
        //assert(g_wgpustate.rstate->activeRenderPass == &g_wgpustate.rstate->renderpass);
        //EndRenderpassEx(&g_wgpustate.rstate->renderpass);
        //BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
    }
    g_wgpustate.current_drawmode = mode;
}
void rlEnd(){
    
}
Image LoadImageFromTexture(Texture tex){
    #ifndef __EMSCRIPTEN__
    auto& device = g_wgpustate.device;
    Image ret {(PixelFormat)tex.format, tex.width, tex.height, size_t(std::ceil(4.0 * tex.width / 256.0) * 256), nullptr, 1};
    WGPUBufferDescriptor b{};
    b.mappedAtCreation = false;
    b.size = size_t(std::ceil(4.0 * tex.width / 256.0) * 256) * tex.height;
    b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    
    wgpu::Buffer readtex = wgpuDeviceCreateBuffer(device, &b);
    
    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = STRVIEW("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
    WGPUImageCopyTexture tbsource;
    tbsource.texture = tex.id;
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
        uint64_t bufferSize = wgpuBufferGetSize(rei->second->Get());
        const void* map = wgpuBufferGetConstMappedRange(rei->second->Get(), 0, bufferSize);
        //rei->first->data = std::realloc(rei->first->data, bufferSize);
        rei->first->data = std::malloc(bufferSize);
        std::memcpy(rei->first->data, map, bufferSize);
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
    Color* altdata = nullptr;
    if(img.format == GRAYSCALE){
        altdata = (Color*)calloc(img.width * img.height, sizeof(Color));
    }
    WGPUTextureDescriptor desc = {
    nullptr,
    WGPUStringView{nullptr, 0},
    WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
    WGPUTextureDimension_2D,
    WGPUExtent3D{img.width, img.height, 1},
        img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format,
    1,1,1,nullptr};
        
    desc.viewFormats = (WGPUTextureFormat*)&img.format;
    ret.id = wgpuDeviceCreateTexture(g_wgpustate.device, &desc);
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
    destination.texture = ret.id;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUTextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * img.width;
    source.rowsPerImage = img.height;
    //wgpuQueueWriteTexture()
    wgpuQueueWriteTexture(g_wgpustate.queue, &destination, altdata ? altdata : img.data, 4 * img.width * img.height, &source, &desc.size);
    ret.view = wgpuTextureCreateView(ret.id, &vdesc);
    ret.width = img.width;
    ret.height = img.height;
    if(altdata)free(altdata);
    return ret;
}
void ToggleFullscreen(){
    //wgpuTextureViewRelease(depthTexture.view);
    //wgpuTextureRelease(depthTexture.tex);
    auto vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowMonitor(g_wgpustate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    //depthTexture = LoadDepthTexture(1920, 1200);
}
extern "C" bool IsKeyDown(int key){
    return g_wgpustate.keydown[key];
}
extern "C" bool IsKeyPressed(int key){
    return g_wgpustate.keydown[key] && !g_wgpustate.keydownPrevious[key];
}
extern "C" int GetCharPressed(){
    int fc = 0;
    if(!g_wgpustate.charQueue.empty()){
        fc = g_wgpustate.charQueue.front();
        g_wgpustate.charQueue.pop_front();
    }
    return fc;
}
int GetMouseX(cwoid){
    return (int)GetMousePosition().x;
}
int GetMouseY(cwoid){
    return (int)GetMousePosition().y;
}
Vector2 GetMousePosition(cwoid){
    return g_wgpustate.mousePos;
}
Vector2 GetMouseDelta(cwoid){
    return Vector2{g_wgpustate.mousePos.x - g_wgpustate.mousePos.x,
                   g_wgpustate.mousePos.y - g_wgpustate.mousePos.y};
}
float GetMouseWheelMove(void){
    return (float)(g_wgpustate.globalScrollY - g_wgpustate.globalScrollYPrevious);
}
Vector2 GetMouseWheelMoveV(void){
    
    return Vector2{
        (float)(g_wgpustate.globalScrollX - g_wgpustate.globalScrollYPrevious),
        (float)(g_wgpustate.globalScrollY - g_wgpustate.globalScrollYPrevious)};
}
bool IsMouseButtonPressed(int button){
    return g_wgpustate.mouseButtonDown[button] && !g_wgpustate.mouseButtonDownPrevious[button];
}
bool IsMouseButtonDown(int button){
    return g_wgpustate.mouseButtonDown[button];
}
bool IsMouseButtonReleased(int button){
    return !g_wgpustate.mouseButtonDown[button] && g_wgpustate.mouseButtonDownPrevious[button];
}


void ShowCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void HideCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
bool IsCursorHidden(cwoid){
    return glfwGetInputMode(g_wgpustate.window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
}
void EnableCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void DisableCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
}
bool IsCursorOnScreen(cwoid){
    return g_wgpustate.cursorInWindow;
}

bool WindowShouldClose(cwoid){
    return glfwWindowShouldClose(g_wgpustate.window);
}

uint64_t NanoTime(cwoid){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
double GetTime(cwoid){
    uint64_t nano_diff = NanoTime() - g_wgpustate.init_timestamp;

    return double(nano_diff) * 1e-9;
}
uint32_t GetFPS(cwoid){
    auto firstzero = std::find(std::begin(g_wgpustate.last_timestamps), std::end(g_wgpustate.last_timestamps), int64_t(0));
    auto [minit, maxit] = std::minmax_element(std::begin(g_wgpustate.last_timestamps), firstzero);
    return uint32_t(std::round((firstzero - std::begin(g_wgpustate.last_timestamps) - 1.0) * 1.0e9 / (*maxit - *minit)));
}
void DrawFPS(int posX, int posY){
    char fpstext[128] = {0};
    std::snprintf(fpstext, 128, "%d FPS", GetFPS());
    double ratio = double(GetFPS()) / GetTargetFPS();
    ratio = std::max(0.0, std::min(1.0, ratio));
    uint8_t v8 = ratio * 255;
    DrawText(fpstext, posX, posY, 40, Color{uint8_t(255 - uint8_t(ratio * ratio * 255)), v8, 20, 255});
}
WGPUShaderModule LoadShaderFromMemory(const char* shaderSource) {
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderSourceWGSL;
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
Texture LoadTexturePro(uint32_t width, uint32_t height, WGPUTextureFormat format, WGPUTextureUsage usage, uint32_t sampleCount){
     WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = sampleCount;
    tDesc.format = format;
    tDesc.usage  = usage;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;

    WGPUTextureViewDescriptor textureViewDesc{};
    char potlabel[128]; 
    if(format == WGPUTextureFormat_Depth24Plus){
        int len = snprintf(potlabel, 128, "Depftex %d x %d", width, height);
        textureViewDesc.label.data = potlabel;
        textureViewDesc.label.length = len;
    }
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
    ret.id = wgpuDeviceCreateTexture(g_wgpustate.device, &tDesc);
    ret.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    return ret;
}
Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget){
    return LoadTexturePro(width, height, format, (WGPUTextureUsage_RenderAttachment * to_be_used_as_rendertarget) | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 1);
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
        case 1:return WGPUVertexFormat_Float16  ;
        case 2:return WGPUVertexFormat_Float16x2;
      //case 3:return WGPUVertexFormat_Float16x3;
        case 4:return WGPUVertexFormat_Float16x4;
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

void init_full_renderstate(full_renderstate* state, const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniform_count, WGPUTextureView c, WGPUTextureView d){
    //state->shader = sh;
    state->color = c;
    state->depth = d;
    RenderSettings settings{};
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    settings.optionalDepthTexture = d;
    settings.sampleCount_onlyApplicableIfMoreThanOne = (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1;
    state->renderpass = LoadRenderpassEx(c, d, settings);
    state->renderpass.renderPassDesc.label = STRVIEW("g_wgpustate::render_pass");
    state->clearPass = LoadRenderpassEx(c, d, settings);
    state->clearPass.renderPassDesc.label = STRVIEW("g_wgpustate::clear_pass");
    state->clearPass.dsa->depthClearValue = 1.0;
    state->clearPass.dsa->depthLoadOp = WGPULoadOp_Clear;
    state->clearPass.dsa->depthStoreOp = WGPUStoreOp_Store;

    state->clearPass.rca->clearValue = WGPUColor{1.0, 0.4, 0.2, 1.0};
    state->clearPass.rca->loadOp = WGPULoadOp_Clear;
    state->clearPass.rca->storeOp = WGPUStoreOp_Store;
    state->activeRenderPass = nullptr;

    state->currentPipeline = LoadPipelineEx(shaderSource, attribs, attribCount, uniforms, uniform_count, settings);
    state->defaultPipeline = state->currentPipeline;
    //WGPUBufferDescriptor vbmdesc{};
    //vbmdesc.mappedAtCreation = true;
    //vbmdesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_MapWrite;
    //vbmdesc.size = (1 << 22) * sizeof(vertex);
    //vbomap.buffer = wgpuDeviceCreateBuffer(GetDevice(), &vbmdesc);
    //vbomap.descriptor = vbmdesc;
    vboptr_base = nullptr;
    //vboptr = (vertex*)wgpuBufferGetMappedRange(vbomap.buffer, 0, vbmdesc.size);
    vboptr = (vertex*)calloc(1000000, sizeof(vertex));
    vboptr_base = vboptr;
    //std::cout << "VBO Punkter: " << vboptr << "\n";
    //std::cout << "Mapped: " << vboptr <<"\n";
    //exit(0);
    /*{
        std::vector<WGPUBindGroupLayoutEntry> blayouts(shader_inputs.uniform_count);
        state->pipeline.bglayout = DescribedBindGroupLayout{};
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
        state->pipeline.bglayout.descriptor.entryCount = shader_inputs.uniform_count;
        state->pipeline.bglayout.descriptor.entries = blayouts.data();
        state->pipeline.bglayout.descriptor.label = STRVIEW("GlobalRenderState_BindGroupLayout");
        state->pipeline.bglayout.layout = wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &state->pipeline.bglayout.descriptor);
    }*/
    
    //state->bg = 0;
    //std::memset(state->bgEntries, 0, 8 * sizeof(WGPUBindGroupEntry));
    /*{
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
    }*/

    //WGPUBindGroupEntry hacc[8] = {};
    //memset(hacc, 0, sizeof(hacc));
    //for(uint32_t i = 0;i < 8;i++){
    //    hacc[i] = WGPUBindGroupEntry{};
    //    hacc[i].binding = i;
    //    hacc[i].buffer = 0;
    //}
    //state->pipeline.bindGroup = LoadBindGroup(&state->pipeline, hacc, 4);
    //state->currentBindGroup.entries = (WGPUBindGroupEntry*)calloc(8, sizeof(WGPUBindGroupEntry));// = LoadBindGroup(&state->pipeline, nullptr, 0);
    //state->currentBindGroup.desc.entries = state->currentBindGroup.entries;
    //state->currentBindGroup.desc.entryCount = 4;
    //state->currentBindGroup.desc.layout = state->pipeline.bglayout.layout;
    
}
void SetUniformBuffer (uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}
void SetStorageBuffer (uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}

void SetUniformBufferData (uint32_t index, const void* data, size_t size){
    drawCurrentBatch();
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_wgpustate.queue, uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}
void SetStorageBufferData (uint32_t index, const void* data, size_t size){
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    wgpuQueueWriteBuffer(g_wgpustate.queue, uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}
extern "C" void SetTexture(uint32_t index, Texture tex){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}
extern "C" void SetSampler(uint32_t index, WGPUSampler sampler){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.sampler = sampler;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, index, entry);
}
void ResizeBuffer(DescribedBuffer* buffer, size_t newSize){
    if(newSize == buffer->descriptor.size)return;

    DescribedBuffer newbuffer{};
    newbuffer.descriptor = buffer->descriptor;
    newbuffer.descriptor.size = newSize;
    
    newbuffer.buffer = wgpuDeviceCreateBuffer(GetDevice(), &newbuffer.descriptor);
    wgpuBufferRelease(buffer->buffer);
    *buffer = newbuffer;
}
void ResizeBufferAndConserve(DescribedBuffer* buffer, size_t newSize){
    if(newSize == buffer->descriptor.size)return;

    size_t smaller = std::min(newSize, buffer->descriptor.size);
    DescribedBuffer newbuffer{};
    newbuffer.descriptor = buffer->descriptor;
    newbuffer.descriptor.size = newSize;
    newbuffer.buffer = wgpuDeviceCreateBuffer(GetDevice(), &newbuffer.descriptor);
    WGPUCommandEncoderDescriptor edesc{};
    WGPUCommandBufferDescriptor bdesc{};
    auto enc = wgpuDeviceCreateCommandEncoder(GetDevice(), &edesc);
    wgpuCommandEncoderCopyBufferToBuffer(enc, buffer->buffer, 0, newbuffer.buffer, 0, smaller);
    auto buf = wgpuCommandEncoderFinish(enc, &bdesc);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    wgpuBufferRelease(buffer->buffer);
    *buffer = newbuffer;
}


WGPURenderPassDepthStencilAttachment* defaultDSA(WGPUTextureView depth){
    WGPURenderPassDepthStencilAttachment* dsa = (WGPURenderPassDepthStencilAttachment*)calloc(1, sizeof(WGPURenderPassDepthStencilAttachment));
    // The view of the depth texture
    dsa->view = depth;
    //dsa.depthSlice = 0;
    // The initial value of the depth buffer, meaning "far"
    dsa->depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    dsa->depthLoadOp = WGPULoadOp_Load;
    dsa->depthStoreOp = WGPUStoreOp_Store;
    // we could turn off writing to the depth buffer globally here
    dsa->depthReadOnly = false;
    // Stencil setup, mandatory but unused
    dsa->stencilClearValue = 0;
    #ifdef WEBGPU_BACKEND_WGPU
    dsa.stencilLoadOp =  WGPULoadOp_Load;
    dsa.stencilStoreOp = WGPUStoreOp_Store;
    #else
    dsa->stencilLoadOp = WGPULoadOp_Undefined;
    dsa->stencilStoreOp = WGPUStoreOp_Undefined;
    #endif
    dsa->stencilReadOnly = true;
    return dsa;
}
extern "C" DescribedRenderpass LoadRenderpassEx(WGPUTextureView color, WGPUTextureView depth, RenderSettings settings){
    DescribedRenderpass ret{};
    ret.settings = settings;
    ret.renderPassDesc = WGPURenderPassDescriptor{};
    ret.rca = (WGPURenderPassColorAttachment*)calloc(1, sizeof(WGPURenderPassColorAttachment));
    ret.rca->view = color;
    ret.rca->depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    #ifdef __EMSCRIPTEN__
    ret.rca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    #endif
    //std::cout << rca.depthSlice << "\n";
    ret.rca->resolveTarget = nullptr;
    ret.rca->loadOp =  WGPULoadOp_Load;
    ret.rca->storeOp = WGPUStoreOp_Store;
    ret.rca->clearValue = WGPUColor{ 0.01, 0.01, 0.2, 1.0 };
    ret.renderPassDesc.colorAttachmentCount = 1;
    ret.renderPassDesc.colorAttachments = ret.rca;
    // We now add a depth/stencil attachment, if desired:
    ret.dsa = nullptr;
    if(settings.depthTest){
        ret.dsa = defaultDSA(depth);
    }
    ret.renderPassDesc.depthStencilAttachment = ret.dsa;
    ret.renderPassDesc.timestampWrites = nullptr;
    return ret;
}
inline bool operator==(const RenderSettings& a, const RenderSettings& b){
    return a.depthTest == b.depthTest &&
           a.faceCull == b.faceCull &&
           a.depthCompare == b.depthCompare &&
           a.frontFace == b.frontFace;
}
extern "C" void UpdateRenderpass(DescribedRenderpass* rp, RenderSettings newSettings){
    if(rp->settings == newSettings){
        return;
    }
    if(!rp->settings.depthTest && newSettings.depthTest){
        rp->dsa = defaultDSA(newSettings.optionalDepthTexture);
        rp->renderPassDesc.depthStencilAttachment = rp->dsa;
    }
    if(rp->settings.depthTest && !newSettings.depthTest){
        free(rp->dsa);
        rp->renderPassDesc.depthStencilAttachment = nullptr;
    }
}
void UnloadRenderpass(DescribedRenderpass rp){
    if(rp.rca)free(rp.rca); //Should not happen
    rp.rca = nullptr;

    if(rp.dsa)free(rp.dsa);
    rp.dsa = nullptr;
}
extern "C" DescribedRenderpass LoadRenderpass(WGPUTextureView color, WGPUTextureView depth){
    return LoadRenderpassEx(color, depth, RenderSettings{
        .depthTest = false,
        .faceCull = false,
        .depthCompare = WGPUCompareFunction_LessEqual, //Not applicable anyway
        .frontFace = WGPUFrontFace_CCW, //Not applicable anyway
        .optionalDepthTexture = depth,
    });    
}
WGPUTexture cres = 0;
WGPUTextureView cresv = 0;

void setTargetTextures(full_renderstate* state, WGPUTextureView c, WGPUTextureView d){
    state->color = c;
    state->depth = d;
    //LoadTexture
    
    WGPUTextureDescriptor desc{};
    const bool multisample = g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT;
    if(!cres && multisample){
        WGPUTextureDescriptor tDesc{};
        tDesc.format = g_wgpustate.frameBufferFormat;
        tDesc.size = WGPUExtent3D{GetScreenWidth(), GetScreenHeight(), 1};
        tDesc.usage = WGPUTextureUsage_RenderAttachment;
        tDesc.dimension = WGPUTextureDimension_2D;
        tDesc.mipLevelCount = 1;
        tDesc.sampleCount = 4;
        tDesc.usage  = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc;
        tDesc.viewFormatCount = 1;
        tDesc.viewFormats = &tDesc.format;
        cres = wgpuDeviceCreateTexture(g_wgpustate.device, &tDesc);
        WGPUTextureViewDescriptor textureViewDesc{};
        textureViewDesc.aspect = WGPUTextureAspect_All;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = 1;
        textureViewDesc.dimension = WGPUTextureViewDimension_2D;
        textureViewDesc.format = tDesc.format;
        cresv = wgpuTextureCreateView(cres, &textureViewDesc);
    }
    state->renderpass.rca->resolveTarget = multisample ? c : nullptr;
    state->renderpass.rca->view = multisample ? cresv : c;
    if(state->renderpass.settings.depthTest){
        state->renderpass.dsa->view = d;
    }

    //TODO: Not hardcode every renderpass here
    state->clearPass.rca->view = multisample ? cresv : c;
    if(state->clearPass.settings.depthTest){
        state->clearPass.dsa->view = d;
    }
    //updateRenderPassDesc(state);
    if(g_wgpustate.rstate->renderpass.rpEncoder){
        EndRenderpassEx(&g_wgpustate.rstate->renderpass);
    }
    BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
}
extern "C" char* LoadFileText(const char *fileName) {
    std::ifstream file(fileName, std::ios::ate);
    if (!file.is_open()) {
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        return nullptr;
    }

    if (!file.read(static_cast<char*>(buffer), size)) {
        free(buffer);
        return nullptr;
    }
    buffer[size] = '\0';
    return buffer;
}
extern "C" void UnloadFileText(char* content){
    free((void*)content);
}
extern "C" void UnloadFileData(void* content){
    free((void*)content);
}
extern "C" void* LoadFileData(const char *fileName, size_t *dataSize) {
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        *dataSize = 0;
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    void* buffer = malloc(size);
    if (!buffer) {
        *dataSize = 0;
        return nullptr;
    }

    if (!file.read(static_cast<char*>(buffer), size)) {
        free(buffer);
        *dataSize = 0;
        return nullptr;
    }

    *dataSize = static_cast<size_t>(size);
    return buffer;
}
extern "C" Image LoadImage(const char* filename){
    size_t size;
    void* data = LoadFileData(filename, &size);
    if(size != 0){
        Image ld =  LoadImageFromMemory(data, size);
        free(data);
        return ld;
    }
    return Image{};
}
void UnloadTexture(Texture tex){
    if(tex.view)
        wgpuTextureViewRelease(tex.view);
    if(tex.id)
        wgpuTextureRelease(tex.id);
    
}
void UnloadImage(Image img){
    free(img.data);
}
extern "C" Image LoadImageFromMemory(const void* data, size_t dataSize){
    Image image;
    uint32_t comp;
    image.data = stbi_load_from_memory((stbi_uc*)data, dataSize, (int*)&image.width, (int*)&image.height, (int*)&comp, 0);
    image.format = RGBA8;
    return image;
}
extern "C" Image GenImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){
    Image ret{RGBA8, width, height, width * 4, std::calloc(width * height, sizeof(Color)), 1};
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
void SaveImage(Image img, const char* filepath){
    //std::cout << img.format << "\n";
    size_t stride = std::ceil(4.0 * img.width / 256.0) * 256;
    std::string_view fp(filepath, filepath + std::strlen(filepath));
    BGRAColor* cols = (BGRAColor*)img.data; 
    Color* ocols = (Color*)calloc(stride * img.height, sizeof(Color));
    for(size_t i = 0;i < (stride / sizeof(Color)) * img.height;i++){
        ocols[i].r = cols[i].r;
        ocols[i].g = cols[i].g;
        ocols[i].b = cols[i].b;
        ocols[i].a = 255;
    }
    if(fp.ends_with(".png")){
        stbi_write_png(filepath, img.width, img.height, 4, ocols, stride);
    }
    else if(fp.ends_with(".jpg")){
        stbi_write_jpg(filepath, img.width, img.height, 4, ocols, stride);
    }
    else if(fp.ends_with(".bmp")){
        //if(row)
        //std::cerr << "Careful with bmp!" << filepath << "\n";
        stbi_write_bmp(filepath, img.width, img.height, 4, ocols);
    }
    else{
        std::cerr << "Unrecognized image format in filename " << filepath << "\n";
    }
}
void UseTexture(Texture tex){
    if(g_wgpustate.rstate->currentPipeline->bindGroup.entries[1].textureView == tex.view)return;
    drawCurrentBatch();
    SetTexture(1, tex);
    //WGPUBindGroupEntry entry{};
    //entry.binding = 1;
    //entry.textureView = tex.view;
    //UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, 1, entry);
    //if(g_wgpustate.activeTexture.tex != tex.tex){
    //    drawCurrentBatch();
    //    g_wgpustate.activeTexture = tex;
    //    setStateTexture(g_wgpustate.rstate, 1, tex);
    //}
    
}
void UseNoTexture(){
    if(g_wgpustate.rstate->currentPipeline->bindGroup.entries[1].textureView == g_wgpustate.whitePixel.view)return;

    drawCurrentBatch();
    WGPUBindGroupEntry entry{};
    entry.binding = 1;
    entry.textureView = g_wgpustate.whitePixel.view;
    UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, 1, entry);
    //if(g_wgpustate.activeTexture.tex != g_wgpustate.whitePixel.tex){
    //    drawCurrentBatch();
    //    setStateTexture(g_wgpustate.rstate, 1, g_wgpustate.whitePixel);
    //}
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

extern "C" StagingBuffer GenStagingBuffer(size_t size, WGPUBufferUsage usage){
    StagingBuffer ret{};
    ret.mappable.descriptor = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = WGPUStringView{},
        .usage = WGPUBufferUsage_MapWrite | WGPUBufferUsage_CopySrc,
        .size = size,
        .mappedAtCreation = true
    };
    ret.gpuUsable.descriptor = WGPUBufferDescriptor{
        .nextInChain = nullptr,
        .label = WGPUStringView{},
        .usage = usage,
        .size = size,
        .mappedAtCreation = false
    };
    ret.gpuUsable.buffer = wgpuDeviceCreateBuffer(GetDevice(), &ret.gpuUsable.descriptor);
    ret.mappable.buffer = wgpuDeviceCreateBuffer(GetDevice(), &ret.mappable.descriptor);
    ret.map = wgpuBufferGetMappedRange(ret.mappable.buffer, 0, size);
    return ret;
}
void RecreateStagingBuffer(StagingBuffer* buffer){
    wgpuBufferRelease(buffer->gpuUsable.buffer);
    buffer->gpuUsable.buffer = wgpuDeviceCreateBuffer(GetDevice(), &buffer->gpuUsable.descriptor);
}

void UpdateStagingBuffer(StagingBuffer* buffer){
    wgpuBufferUnmap(buffer->mappable.buffer);
    WGPUCommandEncoderDescriptor arg{};
    WGPUCommandBufferDescriptor arg2{};
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(GetDevice(), &arg);
    wgpuCommandEncoderCopyBufferToBuffer(enc, buffer->mappable.buffer, 0, buffer->gpuUsable.buffer, 0,buffer->mappable.descriptor.size);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, &arg2);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    
    wgpu::Buffer b(buffer->mappable.buffer);
    WGPUFuture f = b.MapAsync(wgpu::MapMode::Write, 0, b.GetSize(), wgpu::CallbackMode::WaitAnyOnly, [](wgpu::MapAsyncStatus status, wgpu::StringView message){});
    wgpuCommandEncoderRelease(enc);
    wgpuCommandBufferRelease(buf);
    WGPUFutureWaitInfo winfo{f, 0};
    wgpuInstanceWaitAny(g_wgpustate.instance, 1, &winfo, UINT64_MAX);
    buffer->mappable.buffer = b.MoveToCHandle();
    buffer->map = (vertex*)wgpuBufferGetMappedRange(buffer->mappable.buffer, 0, buffer->mappable.descriptor.size);
}
//StagingBuffer MapStagingBuffer(size_t size, WGPUBufferUsage usage){
//
//}
void UnloadStagingBuffer(StagingBuffer* buf){
    wgpuBufferRelease(buf->gpuUsable.buffer);
    wgpuBufferUnmap(buf->mappable.buffer);
    wgpuBufferRelease(buf->mappable.buffer);
}
extern "C" DescribedBuffer GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage){
    DescribedBuffer ret{};
    ret.descriptor.size = size;
    ret.descriptor.mappedAtCreation = false;
    ret.descriptor.usage = usage;
    ret.buffer = wgpuDeviceCreateBuffer(GetDevice(), &ret.descriptor);
    if(data != nullptr){
        wgpuQueueWriteBuffer(GetQueue(), ret.buffer, 0, data, size);
    }
    return ret;
}
extern "C" DescribedBuffer GenBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex);
}
DescribedBuffer GenUniformBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
}
DescribedBuffer GenStorageBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage);
}

extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->descriptor.size >= size){
        wgpuQueueWriteBuffer(GetQueue(), buffer->buffer, 0, data, size);
    }
    else{
        wgpuBufferRelease(buffer->buffer);
        *buffer = GenBuffer(data, size);
    }
}
extern "C" void SetTargetFPS(int fps){
    g_wgpustate.targetFPS = fps;
}
extern "C" int GetTargetFPS(){
    return g_wgpustate.targetFPS;
}
extern "C" float GetFrameTime(){
    if(g_wgpustate.total_frames == 0){
        return 0.0f;
    }
    return 1.0e-9f * (g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] - g_wgpustate.last_timestamps[(g_wgpustate.total_frames - 1) % 64]);
}
extern "C" void SetConfigFlags(WindowFlag flag){
    g_wgpustate.windowFlags |= flag;
}
void NanoWaitImpl(uint64_t stmp){
    for(;;){
        uint64_t now = NanoTime();
        if(now >= stmp)break;
        if(stmp - now > 2500000){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else if(stmp - now > 150000){
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }else{
            for(;NanoTime() < stmp;){}
            break;
        }
    }
}

extern "C" void NanoWait(uint64_t time){
    NanoWaitImpl(NanoTime() + time);
    return;
}

unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize)
{
    unsigned char *data = NULL;

    // Decompress data from a valid DEFLATE stream
    data = (unsigned char *)calloc((20)*1024*1024, 1);
    int length = sinflate(data, 20*1024*1024, compData, compDataSize);

    // WARNING: RL_REALLOC can make (and leave) data copies in memory, be careful with sensitive compressed data!
    // TODO: Use a different approach, create another buffer, copy data manually to it and wipe original buffer memory
    unsigned char *temp = (unsigned char *)realloc(data, length);

    if (temp != NULL) data = temp;
    else TRACELOG(LOG_WARNING, "SYSTEM: Failed to re-allocate required decompression memory");

    *dataSize = length;

    TRACELOG(LOG_INFO, "SYSTEM: Decompress data: Comp. size: %i -> Original size: %i", compDataSize, *dataSize);

    return data;
}
// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;

    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }

    return latestMatch;
}
const char *GetDirectoryPath(const char *filePath)
{
    /*
    // NOTE: Directory separator is different in Windows and other platforms,
    // fortunately, Windows also support the '/' separator, that's the one should be used
    #if defined(_WIN32)
        char separator = '\\';
    #else
        char separator = '/';
    #endif
    */
    const char *lastSlash = NULL;
    static char dirPath[2048] = { 0 };
    memset(dirPath, 0, 2048);

    // In case provided path does not contain a root drive letter (C:\, D:\) nor leading path separator (\, /),
    // we add the current directory path to dirPath
    if ((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/'))
    {
        // For security, we set starting path to current directory,
        // obtained path will be concatenated to this
        dirPath[0] = '.';
        dirPath[1] = '/';
    }

    lastSlash = strprbrk(filePath, "\\/");
    if (lastSlash)
    {
        if (lastSlash == filePath)
        {
            // The last and only slash is the leading one: path is in a root directory
            dirPath[0] = filePath[0];
            dirPath[1] = '\0';
        }
        else
        {
            // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
            char *dirPathPtr = dirPath;
            if ((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/')) dirPathPtr += 2;     // Skip drive letter, "C:"
            memcpy(dirPathPtr, filePath, strlen(filePath) - (strlen(lastSlash) - 1));
            dirPath[strlen(filePath) - strlen(lastSlash) + (((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/'))? 2 : 0)] = '\0';  // Add '\0' manually
        }
    }

    return dirPath;
}

