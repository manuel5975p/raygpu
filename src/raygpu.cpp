#include "raygpu.h"

#include <cassert>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdarg>
#include <thread>
#include <unordered_map>
#include <webgpu/webgpu_cpp.h>
#include "wgpustate.inc"
#include <stb_image_write.h>
#include <stb_image.h>
#include <sinfl.h>
#include <sdefl.h>
#include "msf_gif.h"
#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__
wgpustate g_wgpustate{};

Image fbLoad zeroinit;
wgpu::Buffer readtex zeroinit;
volatile bool waitflag = false;

typedef struct GIFRecordState{
    uint64_t delayInCentiseconds;
    uint64_t lastFrameTimestamp;
    MsfGifState msf_state;
    uint64_t numberOfFrames;
    bool recording;
}GIFRecordState;

void startRecording(GIFRecordState* grst, uint64_t delay){
    if(grst->recording){
        TRACELOG(LOG_WARNING, "Already recording");
        return;
    }
    else{
        grst->numberOfFrames = 0;
    }
    msf_gif_bgra_flag = true;
    grst->msf_state = MsfGifState{};
    grst->delayInCentiseconds = delay;
    msf_gif_begin(&grst->msf_state, GetScreenWidth(), GetScreenHeight());
    grst->recording = true;
}

void addScreenshot(GIFRecordState* grst){
    #ifdef __EMSCRIPTEN__
    if(grst->numberOfFrames > 0)
        msf_gif_frame(&grst->msf_state, (uint8_t*)fbLoad.data, grst->delayInCentiseconds, 8, fbLoad.rowStrideInBytes);
    #endif
    grst->lastFrameTimestamp = NanoTime();
    Image fb = LoadImageFromTextureEx(GetActiveColorTarget(), 0);
    #ifndef __EMSCRIPTEN__
    msf_gif_frame(&grst->msf_state, (uint8_t*)fbLoad.data, grst->delayInCentiseconds, 8, fbLoad.rowStrideInBytes);
    #endif
    UnloadImage(fb);
    ++grst->numberOfFrames;
}

void endRecording(GIFRecordState* grst, const char* filename){
    MsfGifResult result = msf_gif_end(&grst->msf_state);
    
    if (result.data) {
    #ifdef __EMSCRIPTEN__
        // Use EM_ASM to execute JavaScript for downloading the GIF
        // Allocate a buffer in the Emscripten heap and copy the GIF data
        // Then create a Blob and trigger a download in the browser

        // Ensure that the data is null-terminated if necessary
        // You might need to allocate memory in the Emscripten heap
        // and copy the data there, but for simplicity, we'll pass the pointer and size
        EM_ASM({
            var filename = UTF8ToString($0);
            var dataPtr = $1;
            var dataSize = $2;
            
            // Create a Uint8Array from the Emscripten heap
            var bytes = new Uint8Array(Module.HEAPU8.buffer, dataPtr, dataSize);
            
            // Create a Blob from the byte array
            var blob = new Blob([bytes], {type: 'image/gif'});
            
            // Create a temporary anchor element to trigger the download
            var link = document.createElement('a');
            link.href = URL.createObjectURL(blob);
            link.download = filename;
            
            // Append the link to the body (required for Firefox)
            document.body.appendChild(link);
            
            // Programmatically click the link to trigger the download
            link.click();
            
            // Clean up by removing the link and revoking the object URL
            document.body.removeChild(link);
            URL.revokeObjectURL(link.href);
        }, filename, (uintptr_t)result.data, (int)result.dataSize);
    #else
        // Native file system approach
        FILE * fp = fopen(filename, "wb");
        if (fp) {
            fwrite(result.data, 1, result.dataSize, fp);
            fclose(fp);
        } else {
            // Handle file open error if necessary
            fprintf(stderr, "Failed to open file: %s\n", filename);
        }
    #endif
    }

    //Free the GIF result resources
    msf_gif_free(result);

    // Update the recording state
    grst->recording = false;

    // Optionally clear the GIFRecordState structure
    memset(grst, 0, sizeof(GIFRecordState));
}
Vector2 nextuv;
Vector3 nextnormal;
Vector4 nextcol;
vertex* vboptr;
vertex* vboptr_base;
VertexArray* renderBatchVAO;
DescribedBuffer* renderBatchVBO;
//DescribedBuffer vbomap;
#ifdef _WIN32
#define __builtin_unreachable(...)
#endif

typedef struct VertexArray{
    std::vector<AttributeAndResidence> attributes;
    std::vector<std::pair<DescribedBuffer*, WGPUVertexStepMode>> buffers;
    void add(DescribedBuffer* buffer, uint32_t shaderLocation, 
                              WGPUVertexFormat fmt, uint32_t offset, 
                              WGPUVertexStepMode stepmode) {
        // Search for existing attribute by shaderLocation
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            // Attribute exists, update it
            size_t existingBufferSlot = it->bufferSlot;
            auto& existingBufferPair = buffers[existingBufferSlot];

            // Check if the buffer is the same
            if (existingBufferPair.first->buffer != buffer->buffer) {
                // Attempting to update to a new buffer
                // Check if the new buffer is already in buffers
                auto bufferIt = std::find_if(buffers.begin(), buffers.end(),
                                            [buffer, stepmode](const std::pair<DescribedBuffer*, WGPUVertexStepMode>& b) {
                                                return b.first->buffer == buffer->buffer && b.second == stepmode;
                                            });

                if (bufferIt != buffers.end()) {
                    // Reuse existing buffer slot
                    it->bufferSlot = std::distance(buffers.begin(), bufferIt);
                } else {
                    uint32_t pufferIndex = bufferIt - buffers.begin();
                    auto attribIt = std::find_if(attributes.begin(), attributes.end(), [&](const AttributeAndResidence& attr){
                        return attr.bufferSlot == it->bufferSlot && attr.attr.shaderLocation != it->attr.shaderLocation;
                    });
                    //This buffer slot is unused otherwise, so reuse
                    if(attribIt == attributes.end()){
                        buffers[it->bufferSlot] = {buffer, stepmode};
                    }
                    // Add new buffer
                    else{
                        it->bufferSlot = buffers.size();
                        buffers.emplace_back(buffer, stepmode);
                    }
                }
            }

            // Update the rest of the attribute properties
            it->stepMode = stepmode;
            it->attr.format = fmt;
            it->attr.offset = offset;
            it->enabled = true;

            std::cout << "Attribute at shader location " << shaderLocation << " updated.\n";
        } else {
            // Attribute does not exist, add as new
            AttributeAndResidence insert{};
            insert.enabled = true;
            bool bufferFound = false;

            for (size_t i = 0; i < buffers.size(); ++i) {
                auto& existingBufferPair = buffers[i];
                if (existingBufferPair.first->buffer == buffer->buffer) {
                    if (existingBufferPair.second == stepmode) {
                        // Reuse existing buffer slot
                        insert.bufferSlot = i;
                        bufferFound = true;
                        break;
                    } else {
                        TRACELOG(LOG_FATAL, "Mixed step modes for the same buffer are not implemented");
                        // Handle mixed step modes as per your application's requirements
                        // For now, we'll exit to indicate the limitation
                        exit(EXIT_FAILURE);
                    }
                }
            }

            if (!bufferFound) {
                // Add new buffer
                insert.bufferSlot = buffers.size();
                buffers.emplace_back(buffer, stepmode);
            }

            // Set the attribute properties
            insert.stepMode = stepmode;
            insert.attr.format = fmt;
            insert.attr.offset = offset;
            insert.attr.shaderLocation = shaderLocation;
            attributes.emplace_back(insert);

            TRACELOG(LOG_DEBUG,  "New attribute added at shader location %u", shaderLocation);
        }
        std::sort(attributes.begin(), attributes.end(), [](const AttributeAndResidence& a, const AttributeAndResidence& b){
            return a.attr.shaderLocation < b.attr.shaderLocation;
        });
    }
    void add_old(DescribedBuffer* buffer, uint32_t shaderLocation, WGPUVertexFormat fmt, uint32_t offset, WGPUVertexStepMode stepmode){
        AttributeAndResidence insert{};
        for(size_t i = 0;i < buffers.size();i++){
            auto& _buffer = buffers[i];
            if(_buffer.first->buffer == buffer->buffer){
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
        std::sort(attributes.begin(), attributes.end(), [](const AttributeAndResidence& a, const AttributeAndResidence& b){
            return a.attr.shaderLocation < b.attr.shaderLocation;
        });
    }

    /**
     * Enables an attribute based on shaderLocation.
     *
     * @param shaderLocation The shader location identifier to enable.
     * @return true if the attribute was found and enabled, false otherwise.
     */
    bool enableAttribute(uint32_t shaderLocation) {
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            if (!it->enabled) {
                it->enabled = true;
            }
            return true;
        }

        TRACELOG(LOG_WARNING, "Attribute with shader location %u not found.", shaderLocation);
        return false;
    }

    /**
     * Disables an attribute based on shaderLocation.
     *
     * @param shaderLocation The shader location identifier to disable.
     * @return true if the attribute was found and disabled, false otherwise.
     */
    bool disableAttribute(uint32_t shaderLocation) {
        auto it = std::find_if(attributes.begin(), attributes.end(),
                               [shaderLocation](const AttributeAndResidence& ar) {
                                   return ar.attr.shaderLocation == shaderLocation;
                               });

        if (it != attributes.end()) {
            if (it->enabled) {
                it->enabled = false;
                std::cout << "Attribute at shader location " << shaderLocation << " disabled.\n";
            } else {
                std::cout << "Attribute at shader location " << shaderLocation << " is already disabled.\n";
            }
            return true;
        }

        std::cerr << "Attribute with shader location " << shaderLocation << " not found.\n";
        return false;
    }
}VertexArray;
namespace std{
    template<>
    struct hash<VertexArray>{
        inline size_t operator()(const VertexArray& va)const noexcept{
            size_t hashValue = 0;

            // Hash the attributes
            for (const auto& attrRes : va.attributes) {
                // Hash bufferSlot
                size_t bufferSlotHash = std::hash<size_t>()(attrRes.bufferSlot);
                hashValue ^= bufferSlotHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash stepMode
                size_t stepModeHash = std::hash<int>()(static_cast<int>(attrRes.stepMode));
                hashValue ^= stepModeHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash shaderLocation
                size_t shaderLocationHash = std::hash<uint32_t>()(attrRes.attr.shaderLocation);
                hashValue ^= shaderLocationHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash format
                size_t formatHash = std::hash<int>()(static_cast<int>(attrRes.attr.format));
                hashValue ^= formatHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash offset
                size_t offsetHash = std::hash<uint32_t>()(attrRes.attr.offset);
                hashValue ^= offsetHash;
                hashValue = ROT_BYTES(hashValue, 5);

                // Hash enabled flag
                size_t enabledHash = std::hash<bool>()(attrRes.enabled);
                hashValue ^= enabledHash;
                hashValue = ROT_BYTES(hashValue, 5);
            }

            // Hash the buffers (excluding DescribedBuffer* pointers)
            for (const auto& bufferPair : va.buffers) {
                // Only hash the WGPUVertexStepMode, not the buffer pointer
                size_t stepModeHash = std::hash<uint64_t>()(static_cast<uint64_t>(bufferPair.second));
                hashValue ^= stepModeHash;
                hashValue = ROT_BYTES(hashValue, 5);
            }

            return hashValue;
        }
    };
}
extern "C" VertexArray* LoadVertexArray(){
    VertexArray* ret = callocnew(VertexArray);
    new (ret) VertexArray;
    return ret;
}
extern "C" void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, WGPUVertexFormat format, uint32_t offset, WGPUVertexStepMode stepmode){
    array->add(buffer, attribLocation, format, offset, stepmode);
}
void BindVertexArray(VertexArray* va){
    BindPipelineVertexArray(GetActivePipeline(), va);
}
extern "C" void BindPipelineVertexArray(DescribedPipeline* pipeline, VertexArray* va){
    PreparePipeline(pipeline, va);
    // Iterate over each buffer
    for(unsigned i = 0; i < va->buffers.size(); i++){
        bool shouldBind = false;

        // Check if any enabled attribute uses this buffer
        for(const auto& attr : va->attributes){
            if(attr.bufferSlot == i){
                if(attr.enabled){
                    shouldBind = true;
                    break;
                }
                else{
                }
            }
        }

        if(shouldBind){
            auto& bufferPair = va->buffers[i];
            // Bind the buffer
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 
                                                i, 
                                                bufferPair.first->buffer, 
                                                0, 
                                                bufferPair.first->descriptor.size);
        } else {
            TRACELOG(LOG_DEBUG, "Buffer slot %u not bound (no enabled attributes use it).", i);
        }
    }
    
}
extern "C" void EnableVertexAttribArray(VertexArray* array, uint32_t attribLocation){
    array->enableAttribute(attribLocation);
    return;
}
extern "C" void DisableVertexAttribArray(VertexArray* array, uint32_t attribLocation){
    array->disableAttribute(attribLocation);
    return;
}
extern "C" void DrawArrays(WGPUPrimitiveTopology drawMode, uint32_t vertexCount){
    BindPipeline(GetActivePipeline(), drawMode);
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderDraw(rp, vertexCount, 1, 0, 0);
}
extern "C" void DrawArraysIndexed(WGPUPrimitiveTopology drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount){
    BindPipeline(GetActivePipeline(), drawMode);
    //PreparePipeline(GetActivePipeline(), VertexArray *va)
    //TRACELOG(LOG_INFO, "a oooo");
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderSetIndexBuffer(rp, indexBuffer.buffer, WGPUIndexFormat_Uint32, 0, indexBuffer.descriptor.size);
    wgpuRenderPassEncoderDrawIndexed(rp, vertexCount, 1, 0, 0, 0);
}
extern "C" void DrawArraysIndexedInstanced(WGPUPrimitiveTopology drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount, uint32_t instanceCount){
    BindPipeline(GetActivePipeline(), drawMode);
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderSetIndexBuffer(rp, indexBuffer.buffer, WGPUIndexFormat_Uint32, 0, indexBuffer.descriptor.size);
    wgpuRenderPassEncoderDrawIndexed(rp, vertexCount, instanceCount, 0, 0, 0);
}
extern "C" void DrawArraysInstanced(WGPUPrimitiveTopology drawMode, uint32_t vertexCount, uint32_t instanceCount){
    BindPipeline(GetActivePipeline(), drawMode);
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderDraw(rp, vertexCount, instanceCount, 0, 0);
}
WGPUInstance GetInstance(){
    return g_wgpustate.instance.Get();
}
WGPUDevice GetDevice(){
    return g_wgpustate.device.Get();
}
Texture GetDepthTexture(){
    return g_wgpustate.currentDefaultRenderTarget.depth;
}
Texture GetIntermediaryColorTarget(){
    return g_wgpustate.currentDefaultRenderTarget.colorMultisample;
}
WGPUQueue GetQueue(){
    return g_wgpustate.queue.Get();
}
WGPUAdapter GetAdapter(){
    return g_wgpustate.adapter.Get();
}
WGPUSurface GetSurface(){
    return g_wgpustate.surface.Get();
}
wgpu::Instance& GetCXXInstance(){
    return g_wgpustate.instance;
}
wgpu::Adapter&  GetCXXAdapter (){
    return g_wgpustate.adapter;
}
wgpu::Device&   GetCXXDevice  (){
    return g_wgpustate.device;
}
wgpu::Queue&    GetCXXQueue   (){
    return g_wgpustate.queue;
}
wgpu::Surface&  GetCXXSurface (){
    return g_wgpustate.surface;
}


void drawCurrentBatch(){
    size_t vertexCount = vboptr - vboptr_base;
    //std::cout << "vcoun = " << vertexCount << "\n";
    if(vertexCount == 0)return;
    
    DescribedBuffer* vbo;
    bool allocated_via_pool = false;
    if(vertexCount < 32 && !g_wgpustate.smallBufferPool.empty()){
        allocated_via_pool = true;
        vbo = g_wgpustate.smallBufferPool.back();
        g_wgpustate.smallBufferPool.pop_back();
        wgpuQueueWriteBuffer(GetQueue(), vbo->buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
    }
    else{
        vbo = GenBuffer(vboptr_base, vertexCount * sizeof(vertex));
    }

    BufferData(vbo, vboptr_base, vertexCount * sizeof(vertex));
    renderBatchVAO->buffers.front().first = vbo;
    SetStorageBuffer(3, g_wgpustate.identityMatrix);
    UpdateBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup);
    switch(g_wgpustate.current_drawmode){
        case RL_LINES:{

            //TODO: Line texturing is currently disable in all DrawLine... functions
            SetTexture(1, g_wgpustate.whitePixel);
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_LineList);
            BindPipelineVertexArray(g_wgpustate.rstate->activePipeline, renderBatchVAO);
            DrawArrays(WGPUPrimitiveTopology_LineList, vertexCount);
            //wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->renderpass.rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, 0);
            //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            //wgpuRenderPassEncoderDraw(g_wgpustate.rstate->renderpass.rpEncoder, vertexCount, 1, 0, 0);
            
            g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate = true;
        }break;
        case RL_TRIANGLE_STRIP:{
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
            BindPipelineVertexArray(g_wgpustate.rstate->activePipeline, renderBatchVAO);
            DrawArrays(WGPUPrimitiveTopology_TriangleStrip, vertexCount);
            break;
        }
        
        case RL_TRIANGLES:{
            //SetTexture(1, g_wgpustate.whitePixel);
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
            BindPipelineVertexArray(g_wgpustate.rstate->activePipeline, renderBatchVAO);
            DrawArrays(WGPUPrimitiveTopology_TriangleList, vertexCount);
            //abort();
            //vboptr = vboptr_base;
            //BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
            //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            //wgpuRenderPassEncoderDraw(g_wgpustate.rstate->renderpass.rpEncoder, vertexCount, 1, 0, 0);
            //if(!allocated_via_pool){
            //    wgpuBufferRelease(vbo.buffer);
            //}
            //else{
            //    g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            //}
        } break;
        case RL_QUADS:{
            const size_t quadCount = vertexCount / 4;
            
            if(g_wgpustate.quadindicesCache->descriptor.size < 6 * quadCount * sizeof(uint32_t)){
                std::vector<uint32_t> indices(6 * quadCount);
                for(size_t i = 0;i < quadCount;i++){
                    indices[i * 6 + 0] = (i * 4 + 0);
                    indices[i * 6 + 1] = (i * 4 + 1);
                    indices[i * 6 + 2] = (i * 4 + 3);
                    indices[i * 6 + 3] = (i * 4 + 1);
                    indices[i * 6 + 4] = (i * 4 + 2);
                    indices[i * 6 + 5] = (i * 4 + 3);
                }
                BufferData(g_wgpustate.quadindicesCache, indices.data(), 6 * quadCount * sizeof(uint32_t));
                //if(g_wgpustate.quadindicesCache->buffer){
                //    wgpuBufferRelease(g_wgpustate.quadindicesCache->buffer);
                //}
                //g_wgpustate.quadindicesCache = GenBufferEx(indices.data(), 6 * quadCount * sizeof(uint32_t), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
            }
            const DescribedBuffer* ibuf = g_wgpustate.quadindicesCache;
            //BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
            //g_wgpustate.rstate->activePipeline
            BindPipelineVertexArray(g_wgpustate.rstate->activePipeline, renderBatchVAO);
            DrawArraysIndexed(WGPUPrimitiveTopology_TriangleList, *ibuf, quadCount * 6);

            //wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            
            //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, vertexCount * sizeof(vertex));
            //wgpuRenderPassEncoderSetIndexBuffer (g_wgpustate.rstate->renderpass.rpEncoder, ibuf.buffer, WGPUIndexFormat_Uint32, 0, quadCount * 6 * sizeof(uint32_t));
            //wgpuRenderPassEncoderDrawIndexed    (g_wgpustate.rstate->renderpass.rpEncoder, quadCount * 6, 1, 0, 0, 0);
            
            //if(!allocated_via_pool){
            //    wgpuBufferRelease(vbo.buffer);
            //}
            //else{
            //    g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            //}
            
        } break;
        default:break;
    }
    if(!allocated_via_pool){
        UnloadBuffer(vbo);
    }
    else{
        g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
    }
    vboptr = vboptr_base;
}

extern "C" void BeginPipelineMode(DescribedPipeline* pipeline){
    drawCurrentBatch();
    g_wgpustate.rstate->activePipeline = pipeline;
    uint32_t location = GetUniformLocation(pipeline, "Perspective_View");
    if(location != LOCATION_NOT_FOUND){
        SetUniformBufferData(location, &g_wgpustate.activeScreenMatrix, sizeof(Matrix));
    }
    //BindPipeline(pipeline, drawMode);
}
extern "C" void EndPipelineMode(){
    drawCurrentBatch();
    g_wgpustate.rstate->activePipeline = g_wgpustate.rstate->defaultPipeline;
    BindPipeline(g_wgpustate.rstate->activePipeline, g_wgpustate.rstate->activePipeline->lastUsedAs);
}
extern "C" void BeginMode2D(Camera2D camera){
    drawCurrentBatch();
    Matrix mat = GetCameraMatrix2D(camera);
    mat = MatrixMultiply(ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY), mat);
    g_wgpustate.activeScreenMatrix = mat;
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
extern "C" void EndMode2D(){
    drawCurrentBatch();
    g_wgpustate.activeScreenMatrix = ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    SetUniformBufferData(0, &g_wgpustate.activeScreenMatrix, sizeof(Matrix));
}
void BeginMode3D(Camera3D camera){
    drawCurrentBatch();
    Matrix mat = GetCameraMatrix3D(camera, float(g_wgpustate.rstate->renderExtentX) / g_wgpustate.rstate->renderExtentY);
    g_wgpustate.activeScreenMatrix = mat;
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
void EndMode3D(){
    drawCurrentBatch();
    g_wgpustate.activeScreenMatrix = ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    SetUniformBufferData(0, &g_wgpustate.activeScreenMatrix, sizeof(Matrix));
}
extern "C" void BindPipeline(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode){
    switch(drawMode){
        case WGPUPrimitiveTopology_TriangleList:
        //std::cout << "Binding: " <<  pipeline->pipeline << "\n";
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
extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    wgpuComputePassEncoderSetPipeline(g_wgpustate.rstate->computepass.cpEncoder, pipeline->pipeline);
    wgpuComputePassEncoderSetBindGroup (g_wgpustate.rstate->computepass.cpEncoder, 0, GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);
}
extern "C" void CopyBufferToBuffer(DescribedBuffer* source, DescribedBuffer* dest, size_t count){
    wgpuCommandEncoderCopyBufferToBuffer(g_wgpustate.rstate->computepass.cmdEncoder, source->buffer, 0, dest->buffer, 0, count);
}
WGPUBuffer intermediary = 0;
extern "C" void CopyTextureToTexture(Texture source, Texture dest){
    size_t rowBytes = RoundUpToNextMultipleOf256(source.width * GetPixelSizeInBytes(source.format));
    WGPUBufferDescriptor bdesc zeroinit;
    bdesc.size = rowBytes * source.height;
    bdesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
    if(!intermediary)
        intermediary = wgpuDeviceCreateBuffer(GetDevice(), &bdesc);
    
    
    WGPUImageCopyTexture src zeroinit;
    src.texture = source.id;
    src.aspect = WGPUTextureAspect_All;
    src.mipLevel = 0;
    src.origin = WGPUOrigin3D{0, 0, 0};
    
    WGPUImageCopyBuffer bdst zeroinit;
    bdst.buffer = intermediary;
    bdst.layout.rowsPerImage = source.height;
    bdst.layout.bytesPerRow = rowBytes;
    bdst.layout.offset = 0;

    WGPUImageCopyTexture tdst zeroinit;
    tdst.texture = dest.id;
    tdst.aspect = WGPUTextureAspect_All;
    tdst.mipLevel = 0;
    tdst.origin = WGPUOrigin3D{0, 0, 0};

    WGPUExtent3D copySize zeroinit;
    copySize.width = source.width;
    copySize.height = source.height;
    copySize.depthOrArrayLayers = 1;
    
    wgpuCommandEncoderCopyTextureToBuffer(g_wgpustate.rstate->computepass.cmdEncoder, &src, &bdst, &copySize);
    wgpuCommandEncoderCopyBufferToTexture(g_wgpustate.rstate->computepass.cmdEncoder, &bdst, &tdst, &copySize);
    
    //Doesnt work unfortunately:
    //wgpuCommandEncoderCopyTextureToTexture(g_wgpustate.rstate->computepass.cmdEncoder, &src, &dst, &copySize);
    
    //wgpuBufferRelease(intermediary);
}
uint32_t GetScreenWidth (cwoid){
    return g_wgpustate.width;
}
uint32_t GetScreenHeight(cwoid){
    return g_wgpustate.height;
}

void BeginRenderpassEx(DescribedRenderpass* renderPass){
    WGPUCommandEncoderDescriptor desc{};
    desc.label = STRVIEW("another cmdencoder");
    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &desc);
    renderPass->rpEncoder = wgpuCommandEncoderBeginRenderPass(renderPass->cmdEncoder, &renderPass->renderPassDesc);
    g_wgpustate.rstate->activeRenderpass = renderPass;
}

void EndRenderpassEx(DescribedRenderpass* renderPass){
    drawCurrentBatch();
    wgpuRenderPassEncoderEnd(renderPass->rpEncoder);
    g_wgpustate.rstate->activeRenderpass = nullptr;
    auto re = renderPass->rpEncoder;
    renderPass->rpEncoder = 0;
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(renderPass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuRenderPassEncoderRelease(re);
    wgpuCommandEncoderRelease(renderPass->cmdEncoder);
    wgpuCommandBufferRelease(command);
}
void BeginRenderpass(cwoid){
    BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
}
void EndRenderpass(cwoid){
    EndRenderpassEx(&g_wgpustate.rstate->renderpass);
}
extern "C" void ClearBackground(Color clearColor){
    bool rpActive = g_wgpustate.rstate->activeRenderpass != nullptr;
    DescribedRenderpass* backup = g_wgpustate.rstate->activeRenderpass;
    if(rpActive){
        EndRenderpassEx(g_wgpustate.rstate->activeRenderpass);
    }
    g_wgpustate.rstate->clearPass.rca->clearValue = WGPUColor{clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0};
    BeginRenderpassEx(&g_wgpustate.rstate->clearPass);
    EndRenderpassEx(&g_wgpustate.rstate->clearPass);
    if(rpActive){
        BeginRenderpassEx(backup);
    }

}
void BeginComputepass(){
    BeginComputepassEx(&g_wgpustate.rstate->computepass);
}
void DispatchCompute(uint32_t x, uint32_t y, uint32_t z){
    wgpuComputePassEncoderDispatchWorkgroups(g_wgpustate.rstate->computepass.cpEncoder, x, y, z);
}
void ComputepassEndOnlyComputing(cwoid){
    wgpuComputePassEncoderEnd(g_wgpustate.rstate->computepass.cpEncoder);
    g_wgpustate.rstate->computepass.cpEncoder = nullptr;
}
void EndComputepass(){
    EndComputepassEx(&g_wgpustate.rstate->computepass);
}
void BeginComputepassEx(DescribedComputepass* computePass){
    computePass->cmdEncoder = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    g_wgpustate.rstate->computepass.cpEncoder = wgpuCommandEncoderBeginComputePass(g_wgpustate.rstate->computepass.cmdEncoder, &g_wgpustate.rstate->computepass.desc);

}
void EndComputepassEx(DescribedComputepass* computePass){
    if(computePass->cpEncoder){
        wgpuComputePassEncoderEnd(computePass->cpEncoder);
        computePass->cpEncoder = 0;
    }
    
    //TODO
    g_wgpustate.rstate->activeComputepass = nullptr;

    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(computePass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    wgpuCommandEncoderRelease(computePass->cmdEncoder);
}
RenderTexture headless_rtex;
void BeginDrawing(){
    g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] = NanoTime();
    if(g_wgpustate.windowFlags & FLAG_HEADLESS){
        UnloadTexture(headless_rtex.texture);
        if(headless_rtex.colorMultisample.id){
            UnloadTexture(headless_rtex.colorMultisample);
        }
        UnloadTexture(headless_rtex.depth);

        headless_rtex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
        setTargetTextures(g_wgpustate.rstate, headless_rtex.texture.view, headless_rtex.colorMultisample.view, headless_rtex.depth.view);

        g_wgpustate.currentDefaultRenderTarget = headless_rtex;
    }
    else{
        
        if(g_wgpustate.currentDefaultRenderTarget.texture.id)
            UnloadTexture(g_wgpustate.currentDefaultRenderTarget.texture);
        

        //g_wgpustate.drawmutex.lock();
        g_wgpustate.rstate->renderExtentX = GetScreenWidth();
        g_wgpustate.rstate->renderExtentY = GetScreenHeight();
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(g_wgpustate.surface.Get(), &surfaceTexture);
        WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture, nullptr);

        g_wgpustate.mainWindowRenderTarget.texture.id = surfaceTexture.texture;
        g_wgpustate.mainWindowRenderTarget.texture.width = GetScreenWidth();
        g_wgpustate.mainWindowRenderTarget.texture.height = GetScreenHeight();
        g_wgpustate.mainWindowRenderTarget.texture.view = nextTexture;
        
        g_wgpustate.currentDefaultRenderTarget = g_wgpustate.mainWindowRenderTarget;
        setTargetTextures(g_wgpustate.rstate, g_wgpustate.currentDefaultRenderTarget.texture.view, g_wgpustate.currentDefaultRenderTarget.colorMultisample.view, g_wgpustate.currentDefaultRenderTarget.depth.view);
    }
    BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
    SetUniformBuffer(0, g_wgpustate.defaultScreenMatrix);
    g_wgpustate.activeScreenMatrix = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    if(IsKeyPressed(KEY_F2) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || true)){
        if(g_wgpustate.grst->recording){
            EndGIFRecording();
        }
        else{
            StartGIFRecording();
        }
    }
    //EndRenderPass(&g_wgpustate.rstate->clearPass);
    //BeginRenderPass(&g_wgpustate.rstate->renderpass);
    //UseNoTexture();
    //updateBindGroup(g_wgpustate.rstate);
}
void EndDrawing(){
    if(g_wgpustate.rstate->activeRenderpass){
        drawCurrentBatch();
        EndRenderpassEx(g_wgpustate.rstate->activeRenderpass);
    }
    if(g_wgpustate.windowFlags & FLAG_STDOUT_TO_FFMPEG){
        Image img = LoadImageFromTextureEx(GetActiveColorTarget(), 0);
        if (img.format != BGRA8 && img.format != RGBA8) {
            // Handle unsupported formats or convert as necessary
            fprintf(stderr, "Unsupported pixel format for FFmpeg export.\n");
            // You might want to convert the image to a supported format here
            // For simplicity, we'll skip exporting in this case
            return;
        }
        ImageFormat(&img, RGBA8);

        // Calculate the total size of the image data to write
        size_t totalSize = img.rowStrideInBytes * img.height;

        // Write the image data to stdout (FFmpeg should be reading from stdin)
        size_t fmtsize = GetPixelSizeInBytes((WGPUTextureFormat)img.format);
        char offset[1];
        //fwrite(offset, 1, 1, stdout);
        for(size_t i = 0;i < img.height;i++){
            unsigned char* dptr = static_cast<unsigned char*>(img.data) + i * img.rowStrideInBytes;
            //for(uint32_t r = 0;r < img.width;r++){
            //    RGBA8Color c = reinterpret_cast<RGBA8Color*>(dptr)[r];
            //    std::cerr << (int)c.a << "\n";
            //    reinterpret_cast<RGBA8Color*>(dptr)[r] = RGBA8Color{c.a, c.b, c.g, c.r};
            //}
            size_t bytesWritten = fwrite(dptr, 1, img.width * fmtsize, stdout);
        }

        // Flush stdout to ensure all data is sent to FFmpeg promptly
        fflush(stdout);
        UnloadImage(img);
    }
    if(g_wgpustate.grst->recording){
        uint64_t stmp = NanoTime();
        if(stmp - g_wgpustate.grst->lastFrameTimestamp > g_wgpustate.grst->delayInCentiseconds * 10000000ull){
            addScreenshot(g_wgpustate.grst);
            g_wgpustate.grst->lastFrameTimestamp = stmp;
        }
        
        BeginRenderpass();
        int recordingTextX = GetScreenWidth() - MeasureText("Recording", 30);
        DrawText("Recording", recordingTextX, 5, 30, Color{255,40,40,255});
        EndRenderpass();
    }
    
    //WGPUSurfaceTexture surfaceTexture;
    //wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);
    
    
    if(!(g_wgpustate.windowFlags & FLAG_HEADLESS)){
        #ifndef __EMSCRIPTEN__
        g_wgpustate.surface.Present();
        #endif
    }
    uint64_t beginframe_stmp = g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64];
    ++g_wgpustate.total_frames;
    std::copy(g_wgpustate.smallBufferRecyclingBin.begin(), g_wgpustate.smallBufferRecyclingBin.end(), std::back_inserter(g_wgpustate.smallBufferPool));
    g_wgpustate.smallBufferRecyclingBin.clear();
    auto& ipstate = g_wgpustate.input_map[g_wgpustate.window];
    std::copy(ipstate.keydown.begin(), ipstate.keydown.end(), ipstate.keydownPrevious.begin());
    ipstate.mousePosPrevious = ipstate.mousePos;
    ipstate.scrollPreviousFrame = ipstate.scrollThisFrame;
    ipstate.scrollThisFrame = Vector2{0, 0};
    std::copy(ipstate.mouseButtonDown.begin(), ipstate.mouseButtonDown.end(), ipstate.mouseButtonDownPrevious.begin());
    //if(!(g_wgpustate.windowFlags & FLAG_HEADLESS))
    //    g_wgpustate.drawmutex.unlock();
    uint64_t nanosecondsPerFrame = std::floor(1e9 / GetTargetFPS());
    //std::cout << nanosecondsPerFrame << "\n";
    uint64_t elapsed = NanoTime() - beginframe_stmp;
    if(elapsed & (1ull << 63))return;
    if(!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) && nanosecondsPerFrame > elapsed && GetTargetFPS() > 0)
        NanoWait(nanosecondsPerFrame - elapsed);
    PollEvents();

    //std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondsPerFrame - elapsed));
}
void StartGIFRecording(){
    startRecording(g_wgpustate.grst, 4);
}
void EndGIFRecording(){
    #ifndef __EMSCRIPTEN__
    if(!g_wgpustate.grst->recording)return;
    char buf[128] = {0};
    for(int i = 1;i < 1000;i++){
        sprintf(buf, "recording%03d.gif", i);
        std::filesystem::path p(buf);
        if(!std::filesystem::exists(p))
            break;
    }
    #else
    constexpr char buf[] = "gifexport.gif";
    #endif
    endRecording(g_wgpustate.grst, buf);
}
void rlBegin(draw_mode mode){
    if(mode == RL_LINES){ //TODO: Fix this, why is this required? Check core_msaa and comment this out to trigger a bug
        SetTexture(1, g_wgpustate.whitePixel);
    }
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
uint32_t RoundUpToNextMultipleOf256(uint32_t x) {
    return (x + 255) & ~0xFF;
}
uint32_t RoundUpToNextMultipleOf16(uint32_t x) {
    return (x + 15) & ~0xF;
}

Image LoadImageFromTextureEx(WGPUTexture tex, uint32_t miplevel){
    size_t formatSize = GetPixelSizeInBytes(wgpuTextureGetFormat(tex));
    uint32_t width = wgpuTextureGetWidth(tex);
    uint32_t height = wgpuTextureGetHeight(tex);
    Image ret {(PixelFormat)wgpuTextureGetFormat(tex), wgpuTextureGetWidth(tex), wgpuTextureGetHeight(tex), RoundUpToNextMultipleOf256(formatSize * width), nullptr, 1};
    fbLoad = ret;
    WGPUBufferDescriptor b{};
    b.mappedAtCreation = false;
    
    b.size = RoundUpToNextMultipleOf256(formatSize * width) * height;
    b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    
    readtex = wgpu::Buffer(wgpuDeviceCreateBuffer(GetDevice(), &b));
    
    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = STRVIEW("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
    WGPUImageCopyTexture tbsource{};
    tbsource.texture = tex;
    tbsource.mipLevel = miplevel;
    tbsource.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    tbsource.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUImageCopyBuffer tbdest{};
    tbdest.buffer = readtex.Get();
    tbdest.layout.offset = 0;
    tbdest.layout.bytesPerRow = RoundUpToNextMultipleOf256(formatSize * width);
    tbdest.layout.rowsPerImage = height;

    WGPUExtent3D copysize{width / (1 << miplevel), height / (1 << miplevel), 1};
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &copysize);
    
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit(GetQueue(), 1, &command);
    wgpuCommandBufferRelease(command);
    //#ifdef WEBGPU_BACKEND_DAWN
    //wgpuDeviceTick(GetDevice());
    //#else
    //#endif
    fbLoad.format = (PixelFormat)ret.format;
    waitflag = true;
    auto onBuffer2Mapped = [](wgpu::MapAsyncStatus status, const char* userdata){
        //std::cout << "Backcalled: " << (int)status << std::endl;
        waitflag = false;
        if(status != wgpu::MapAsyncStatus::Success){
            TRACELOG(LOG_ERROR, "onBuffer2Mapped called back with error!");
        }
        assert(status == wgpu::MapAsyncStatus::Success);

        uint64_t bufferSize = readtex.GetSize();
        const void* map = readtex.GetConstMappedRange(0, bufferSize);
        fbLoad.data = std::realloc(fbLoad.data , bufferSize);
        //fbLoad.data = std::malloc(bufferSize);
        std::memcpy(fbLoad.data, map, bufferSize);
        readtex.Unmap();
        wgpuBufferRelease(readtex.MoveToCHandle());
    };

    
    
    #ifndef __EMSCRIPTEN__
    g_wgpustate.instance.WaitAny(readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped), 1000000000);
    #else
    //g_wgpustate.instance.WaitAny(readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped), 1000000000);
    readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::AllowSpontaneous, onBuffer2Mapped);
    
    //while(waitflag){
    //    
    //}
    //readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * width / 256.0) * 256) * height, onBuffer2Mapped, &ibpair);
    //wgpu::Future fut = readtex.MapAsync(wgpu::MapMode::Read, 0, RoundUpToNextMultipleOf256(formatSize * width) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped);
    //WGPUFutureWaitInfo winfo{};
    //winfo.future = fut;
    //wgpuInstanceWaitAny(g_wgpustate.instance, 1, &winfo, 1000000000);

    //while(!done){
        //emscripten_sleep(20);
    //}
    //wgpuInstanceWaitAny(g_wgpustate.instance, 1, &winfo, 1000000000);
    //emscripten_sleep(20);
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
    return fbLoad;
}
template<typename from, typename to>
to convert4(const from& fr){
    to ret;
    if constexpr(std::is_floating_point_v<decltype(ret.r)>){

    }
    else{
        ret.r = fr.r;
        ret.g = fr.g;
        ret.b = fr.b;
        ret.a = fr.a;
    }
    return ret;
}
template<typename from, typename to>
void FormatRange(const from* source, to* dest, size_t count){
    const from* fr = reinterpret_cast<const from*>(source);
    to* top = reinterpret_cast<to*>(dest);

    for(size_t i = 0;i < count;i++){
        top[i] = convert4<from, to>(fr[i]);
    }
}
template<typename from, typename to>
void FormatImage_Impl(const Image& source, Image& dest){
    for(uint32_t i = 0;i < source.height;i++){
        const from* dataptr = (const from*)(static_cast<const uint8_t*>(source.data) + source.rowStrideInBytes * i);
        to* destptr = (to*)(static_cast<uint8_t*>(dest.data) + dest.rowStrideInBytes * i);
        FormatRange<from, to>(dataptr, destptr, source.width);
    }
}
void ImageFormat(Image* img, PixelFormat newFormat){
    uint32_t psize = GetPixelSizeInBytes((WGPUTextureFormat)newFormat);
    void* newdata = calloc(img->width * img->height, psize);
    Image newimg zeroinit;
    newimg.format = newFormat;
    newimg.width = img->width;
    newimg.height = img->height;
    newimg.mipmaps = img->mipmaps;
    newimg.rowStrideInBytes = newimg.width * psize;
    newimg.data = newdata;
    switch(img->format){
        case PixelFormat::BGRA8:{
            if(newFormat == RGBA8){
                FormatImage_Impl<BGRA8Color, RGBA8Color>(*img, newimg);
            }
            if(newFormat == RGBA32F){
                FormatImage_Impl<BGRA8Color, RGBA32FColor>(*img, newimg);
            }
        }break;
        default:
        //abort();
        return;
    }
    free(img->data);
    *img = newimg;
    
}

Image LoadImageFromTexture(Texture tex){
    //#ifndef __EMSCRIPTEN__
    //auto& device = g_wgpustate.device;
    return LoadImageFromTextureEx(tex.id, 0);
    //#else
    //std::cerr << "LoadImageFromTexture not supported on web\n";
    //return Image{};
    //#endif
}
void TakeScreenshot(const char* filename){
    Image img = LoadImageFromTextureEx(GetActiveColorTarget(), 0);
    SaveImage(img, filename);
    UnloadImage(img);
}
Texture LoadTextureFromImage(Image img){
    Texture ret zeroinit;
    ret.sampleCount = 1;
    Color* altdata = nullptr;
    if(img.format == GRAYSCALE){
        altdata = (Color*)calloc(img.width * img.height, sizeof(Color));
        for(size_t i = 0;i < img.width * img.height;i++){
            uint16_t gscv = ((uint16_t*)img.data)[i];
            ((Color*)altdata)[i].r = gscv & 255;
            ((Color*)altdata)[i].g = gscv & 255;
            ((Color*)altdata)[i].b = gscv & 255;
            ((Color*)altdata)[i].a = gscv >> 8;
        }
    }
    WGPUTextureDescriptor desc = {
        nullptr,
        WGPUStringView{nullptr, 0},
        WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
        WGPUTextureDimension_2D,
        WGPUExtent3D{img.width, img.height, 1},
        img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format,
        1,1,1,nullptr
    };
    WGPUTextureFormat resulting_tf = img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format;
    desc.viewFormats = (WGPUTextureFormat*)&resulting_tf;
    ret.id = wgpuDeviceCreateTexture(GetDevice(), &desc);
    WGPUTextureViewDescriptor vdesc{};
    vdesc.arrayLayerCount = 0;
    vdesc.aspect = WGPUTextureAspect_All;
    vdesc.format = desc.format;
    vdesc.dimension = WGPUTextureViewDimension_2D;
    vdesc.baseArrayLayer = 0;
    vdesc.arrayLayerCount = 1;
    vdesc.baseMipLevel = 0;
    vdesc.mipLevelCount = 1;
        
    WGPUImageCopyTexture destination{};
    destination.texture = ret.id;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUTextureDataLayout source{};
    source.offset = 0;
    source.bytesPerRow = 4 * img.width;
    source.rowsPerImage = img.height;
    //wgpuQueueWriteTexture()
    wgpuQueueWriteTexture(GetQueue(), &destination, altdata ? altdata : img.data, 4 * img.width * img.height, &source, &desc.size);
    ret.view = wgpuTextureCreateView(ret.id, &vdesc);
    ret.width = img.width;
    ret.height = img.height;
    if(altdata)free(altdata);
    return ret;
}

extern "C" bool IsKeyDown(int key){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydown[key];
}
extern "C" bool IsKeyPressed(int key){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydown[key] && !g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydownPrevious[key];
}
extern "C" int GetCharPressed(){
    int fc = 0;
    if(!g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.empty()){
        fc = g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.front();
        g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.pop_front();
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
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos;
}
Vector2 GetMouseDelta(cwoid){
    return Vector2{g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.x - g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.x,
                   g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.y - g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.y};
}
float GetMouseWheelMove(void){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].scrollPreviousFrame.y;
}
Vector2 GetMouseWheelMoveV(void){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].scrollPreviousFrame;
    //return Vector2{
    //    (float)(g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollX - g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollYPrevious),
    //    (float)(g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollY - g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollYPrevious)};
}
bool IsMouseButtonPressed(int button){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button] && !g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDownPrevious[button];
}
bool IsMouseButtonDown(int button){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button];
}
bool IsMouseButtonReleased(int button){
    return !g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button] && g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDownPrevious[button];
}
bool IsCursorOnScreen(cwoid){
    return g_wgpustate.input_map[(GLFWwindow*)GetActiveWindowHandle()].cursorInWindow;
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
    return wgpuDeviceCreateShaderModule(GetDevice(), &shaderDesc);
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
    return wgpuDeviceCreateShaderModule(GetDevice(), &shaderDesc);
}
Texture3D LoadTexture3DEx(uint32_t width, uint32_t height, uint32_t depth, WGPUTextureFormat format){
    return LoadTexture3DPro(width, height, depth, format, WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_StorageBinding, 1);
}
Texture3D LoadTexture3DPro(uint32_t width, uint32_t height, uint32_t depth, WGPUTextureFormat format, WGPUTextureUsage usage, uint32_t sampleCount){
    Texture3D ret zeroinit;
    ret.width = width;
    ret.height = height;
    ret.depth = depth;
    ret.sampleCount = sampleCount;
    ret.format = format;

    WGPUTextureDescriptor tDesc zeroinit;
    tDesc.dimension = WGPUTextureDimension_3D;
    tDesc.size = {width, height, depth};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = sampleCount;
    tDesc.format = format;
    tDesc.usage  = usage;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;
    
    WGPUTextureViewDescriptor textureViewDesc zeroinit;
    textureViewDesc.aspect = ((format == WGPUTextureFormat_Depth24Plus) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_3D;
    textureViewDesc.format = tDesc.format;

    ret.id = wgpuDeviceCreateTexture(GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    
    return ret;
}
constexpr char mipmapComputerSource[] = R"(
@group(0) @binding(0) var previousMipLevel: texture_2d<f32>;
@group(0) @binding(1) var nextMipLevel: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(8, 8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let offset = vec2<u32>(0, 1);
    
    let color = (
        textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)
    ) * 0.25;
    textureStore(nextMipLevel, id.xy, color);
}
)";
void GenTextureMipmaps(Texture2D* tex){
    static DescribedComputePipeline* cpl = LoadComputePipeline(mipmapComputerSource);
    BeginComputepass();
    
    for(int i = 0;i < tex->mipmaps - 1;i++){
        SetBindgroupTextureView(&cpl->bindGroup, 0, tex->mipViews[i    ]);
        SetBindgroupTextureView(&cpl->bindGroup, 1, tex->mipViews[i + 1]);
        if(i == 0){
            BindComputePipeline(cpl);
        }
        wgpuComputePassEncoderSetBindGroup (g_wgpustate.rstate->computepass.cpEncoder, 0, GetWGPUBindGroup(&cpl->bindGroup), 0, 0);
        uint32_t divisor = (1 << i) * 8;
        DispatchCompute((tex->width + divisor - 1) & -(divisor) / 8, (tex->height + divisor - 1) & -(divisor) / 8, 1);
    }
    EndComputepass();
}
Texture LoadTexturePro(uint32_t width, uint32_t height, WGPUTextureFormat format, WGPUTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = mipmaps;
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
    textureViewDesc.mipLevelCount = mipmaps;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    Texture ret zeroinit;
    ret.id = wgpuDeviceCreateTexture(GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.sampleCount = sampleCount;
    ret.mipmaps = mipmaps;
    if(mipmaps > 1){
        for(uint32_t i = 0;i < mipmaps;i++){
            textureViewDesc.baseMipLevel = i;
            textureViewDesc.mipLevelCount = 1;
            ret.mipViews[i] = wgpuTextureCreateView(ret.id, &textureViewDesc);
        }
    }
    return ret;
}
Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget){
    return LoadTexturePro(width, height, format, (WGPUTextureUsage_RenderAttachment * to_be_used_as_rendertarget) | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 1, 1);
}
Texture LoadTexture(const char* filename){
    Image img = LoadImage(filename);
    Texture tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}
Texture LoadBlankTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_RGBA8Unorm, true);
}

Texture LoadDepthTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_Depth24Plus, true);
}
RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    RenderTexture ret{
        .texture = LoadTextureEx(width, height, g_wgpustate.frameBufferFormat, true),
        .colorMultisample = Texture{}, 
        .depth = LoadTexturePro(width, height, WGPUTextureFormat_Depth24Plus, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1, 1)
    };
    if(g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT){
        ret.colorMultisample = LoadTexturePro(width, height, g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4, 1);
    }
    return ret;
}
void UpdateTexture(Texture tex, void* data){
    WGPUImageCopyTexture destination{};
    destination.texture = tex.id;
    destination.aspect = WGPUTextureAspect_All;
    destination.mipLevel = 0;
    destination.origin = WGPUOrigin3D{0,0,0};

    WGPUTextureDataLayout source{};
    source.offset = 0;
    source.bytesPerRow = 4 * tex.width;
    source.rowsPerImage = tex.height;
    WGPUExtent3D writeSize{};
    writeSize.depthOrArrayLayers = 1;
    writeSize.width = tex.width;
    writeSize.height = tex.height;
    wgpuQueueWriteTexture(GetQueue(), &destination, data, tex.width * tex.height * 4, &source, &writeSize);
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
DescribedPipeline* GetActivePipeline(){
    return g_wgpustate.rstate->activePipeline;
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
    state->computepass = DescribedComputepass{};
    state->renderpass = LoadRenderpassEx(c, d, settings);
    state->renderpass.renderPassDesc.label = STRVIEW("g_wgpustate::render_pass");
    state->clearPass = LoadRenderpassEx(c, d, settings);
    state->clearPass.renderPassDesc.label = STRVIEW("g_wgpustate::clear_pass");
    if(state->clearPass.dsa){
        state->clearPass.dsa->depthClearValue = 1.0;
        state->clearPass.dsa->depthLoadOp = WGPULoadOp_Clear;
        state->clearPass.dsa->depthStoreOp = WGPUStoreOp_Store;
    }

    state->clearPass.rca->clearValue = WGPUColor{1.0, 0.4, 0.2, 1.0};
    state->clearPass.rca->loadOp = WGPULoadOp_Clear;
    state->clearPass.rca->storeOp = WGPUStoreOp_Store;
    state->activeRenderpass = nullptr;

    state->defaultPipeline = LoadPipelineEx(shaderSource, attribs, attribCount, uniforms, uniform_count, settings);
    
    state->activePipeline = state->defaultPipeline;
    g_wgpustate.quadindicesCache = callocnew(DescribedBuffer);    //WGPUBufferDescriptor vbmdesc{};
    g_wgpustate.quadindicesCache->descriptor.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    //vbmdesc.mappedAtCreation = true;
    //vbmdesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_MapWrite;
    //vbmdesc.size = (1 << 22) * sizeof(vertex);
    //vbomap.buffer = wgpuDeviceCreateBuffer(GetDevice(), &vbmdesc);
    //vbomap.descriptor = vbmdesc;
    vboptr_base = nullptr;
    //vboptr = (vertex*)wgpuBufferGetMappedRange(vbomap.buffer, 0, vbmdesc.size);
    vboptr = (vertex*)calloc(10000, sizeof(vertex));
    renderBatchVBO = GenBuffer(nullptr, 10000 * sizeof(vertex));
    
    renderBatchVAO = LoadVertexArray();
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 0, WGPUVertexFormat_Float32x3, 0 * sizeof(float), WGPUVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 1, WGPUVertexFormat_Float32x2, 3 * sizeof(float), WGPUVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 2, WGPUVertexFormat_Float32x3, 5 * sizeof(float), WGPUVertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 3, WGPUVertexFormat_Float32x4, 8 * sizeof(float), WGPUVertexStepMode_Vertex);
    vboptr_base = vboptr;
    state->defaultPipeline->lastUsedAs = WGPUPrimitiveTopology_TriangleList;
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

void SetTexture                   (uint32_t index, Texture tex){
    SetPipelineTexture(GetActivePipeline(), index, tex);
}
void SetSampler                   (uint32_t index, DescribedSampler sampler){
    SetPipelineSampler (GetActivePipeline(), index, sampler);
}
void SetUniformBuffer             (uint32_t index, DescribedBuffer* buffer){
    SetPipelineUniformBuffer (GetActivePipeline(), index, buffer);
}
void SetStorageBuffer             (uint32_t index, DescribedBuffer* buffer){
    SetPipelineStorageBuffer(GetActivePipeline(), index, buffer);
}
void SetUniformBufferData         (uint32_t index, const void* data, size_t size){
    SetPipelineUniformBufferData(GetActivePipeline(), index, data, size);
}
void SetStorageBufferData         (uint32_t index, const void* data, size_t size){
    SetPipelineStorageBufferData(GetActivePipeline(), index, data, size);
}



void SetBindgroupUniformBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(bg, index, entry);
}
void SetBindgroupStorageBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(bg, index, entry);
}
void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    //drawCurrentBatch();
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer(GetQueue(), uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
}
void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    WGPUBindGroupEntry entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer(GetQueue(), uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
}
extern "C" void SetBindgroupTexture3D(DescribedBindGroup* bg, uint32_t index, Texture3D tex){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}
extern "C" void SetBindgroupTextureView(DescribedBindGroup* bg, uint32_t index, WGPUTextureView texView){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.textureView = texView;
    
    UpdateBindGroupEntry(bg, index, entry);
}
extern "C" void SetBindgroupTexture(DescribedBindGroup* bg, uint32_t index, Texture tex){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}

extern "C" void SetBindgroupSampler(DescribedBindGroup* bg, uint32_t index, DescribedSampler sampler){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.sampler = sampler.sampler;
    UpdateBindGroupEntry(bg, index, entry);
}




void SetPipelineUniformBuffer(DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer){
    SetBindgroupUniformBuffer(&pl->bindGroup, index, buffer);
}

void SetPipelineStorageBuffer(DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer){
    SetBindgroupStorageBuffer(&pl->bindGroup, index, buffer);
}

void SetPipelineUniformBufferData(DescribedPipeline* pl, uint32_t index, const void* data, size_t size){
    SetBindgroupUniformBufferData(&pl->bindGroup, index, data, size);
}

void SetPipelineStorageBufferData(DescribedPipeline* pl, uint32_t index, const void* data, size_t size){
    SetBindgroupStorageBufferData(&pl->bindGroup, index, data, size);
}

extern "C" void SetPipelineTexture(DescribedPipeline* pl, uint32_t index, Texture tex){
    SetBindgroupTexture(&pl->bindGroup, index, tex);
}

extern "C" void SetPipelineSampler(DescribedPipeline* pl, uint32_t index, DescribedSampler sampler){
    SetBindgroupSampler(&pl->bindGroup, index, sampler);
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

    size_t smaller = std::min<uint32_t>(newSize, buffer->descriptor.size);
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
    ret.rca->depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
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
        .sampleCount_onlyApplicableIfMoreThanOne = 1,
        .depthCompare = WGPUCompareFunction_LessEqual, //Not applicable anyway
        .frontFace = WGPUFrontFace_CCW, //Not applicable anyway
        .optionalDepthTexture = depth,
    });    
}
DescribedSampler LoadSamplerEx(addressMode amode, filterMode fmode, filterMode mipmapFilter, float maxAnisotropy){
    DescribedSampler ret zeroinit;
    ret.desc.magFilter    = (WGPUFilterMode)fmode;
    ret.desc.minFilter    = (WGPUFilterMode)fmode;
    ret.desc.mipmapFilter = (WGPUMipmapFilterMode)fmode;
    ret.desc.compare      = WGPUCompareFunction_Undefined;
    ret.desc.lodMinClamp  = 0.0f;
    ret.desc.lodMaxClamp  = 10.0f;
    ret.desc.maxAnisotropy = maxAnisotropy;
    ret.desc.addressModeU = (WGPUAddressMode)amode;
    ret.desc.addressModeV = (WGPUAddressMode)amode;
    ret.desc.addressModeW = (WGPUAddressMode)amode;
    ret.sampler = wgpuDeviceCreateSampler(GetDevice(), &ret.desc);
    return ret;
}
DescribedSampler LoadSampler(addressMode amode, filterMode fmode){
    return LoadSamplerEx(amode, fmode, fmode, 1.0f);
}
void UnloadSampler(DescribedSampler sampler){
    wgpuSamplerRelease(sampler.sampler);
}

WGPUTexture GetActiveColorTarget(){
    return g_wgpustate.currentDefaultRenderTarget.texture.id;
}

void setTargetTextures(full_renderstate* state, WGPUTextureView c, WGPUTextureView cms, WGPUTextureView d){
    DescribedRenderpass* restart = nullptr;
    if(g_wgpustate.rstate->activeRenderpass){
        restart = g_wgpustate.rstate->activeRenderpass;
        EndRenderpassEx(g_wgpustate.rstate->activeRenderpass);
    }
    state->color = c;
    state->depth = d;
    //LoadTexture
    
    WGPUTextureDescriptor desc{};
    const bool multisample = g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT;
    //if(colorMultisample.id == nullptr && multisample){
    //    colorMultisample = LoadTexturePro(GetScreenWidth(), GetScreenHeight(), g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4);
    //}
    state->renderpass.rca->resolveTarget = multisample ? c : nullptr;
    state->renderpass.rca->view = multisample ? cms : c;
    if(state->renderpass.settings.depthTest){
        state->renderpass.dsa->view = d;
    }

    //TODO: Not hardcode every renderpass here
    state->clearPass.rca->view = multisample ? cms : c;
    if(state->clearPass.settings.depthTest){
        state->clearPass.dsa->view = d;
    }
    //updateRenderPassDesc(state);
    if(restart)
        BeginRenderpassEx(restart);
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
        TRACELOG(LOG_ERROR, "Failed to open file %s", fileName);
        return nullptr;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    void* buffer = malloc(size);
    if (!buffer) {
        *dataSize = 0;
        TRACELOG(LOG_ERROR, "Failed to load file %s", fileName);
        return nullptr;
    }

    if (!file.read(static_cast<char*>(buffer), size)) {
        free(buffer);
        *dataSize = 0;
        TRACELOG(LOG_ERROR, "Failed to load file %s", fileName);
        return nullptr;
    }

    *dataSize = static_cast<size_t>(size);
    return buffer;
}
extern "C" Image LoadImage(const char* filename){
    size_t size;
    std::string fn_cpp(filename);
    const char* extbegin = fn_cpp.data() + fn_cpp.rfind("."); 
    void* data = LoadFileData(filename, &size);

    if(size != 0){
        Image ld =  LoadImageFromMemory(extbegin, data, size);
        free(data);
        return ld;
    }
    else{
        
    }
    return Image{};
}
void UnloadTexture(Texture tex){
    for(uint32_t i = 0;i < tex.mipmaps;i++){
        if(tex.mipViews[i]){
            wgpuTextureViewRelease(tex.mipViews[i]);
            tex.mipViews[i] = nullptr;
        }
    }
    if(tex.view){
        wgpuTextureViewRelease(tex.view);
        tex.view = nullptr;
    }
    if(tex.id){
        wgpuTextureRelease(tex.id);
        tex.id = nullptr;
    }
    
}
void UnloadImage(Image img){
    free(img.data);
    img.data = nullptr;
}
extern "C" Image LoadImageFromMemory(const char* extension, const void* data, size_t dataSize){
    Image image;
    uint32_t comp;
    image.data = stbi_load_from_memory((stbi_uc*)data, dataSize, (int*)&image.width, (int*)&image.height, (int*)&comp, 0);
    if(comp == 4){
        image.format = RGBA8;
    }else if(comp == 3){
        image.format = RGB8;
    }
    return image;
}
extern "C" Image GenImageColor(Color a, uint32_t width, uint32_t height){
    Image ret{RGBA8, width, height, width * 4, std::calloc(width * height, sizeof(Color)), 1};
    for(uint32_t i = 0;i < height;i++){
        for(uint32_t j = 0;j < width;j++){
            const size_t index = size_t(i) * width + j;
            static_cast<Color*>(ret.data)[index] = a;
        }
    }
    return ret;
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
    std::string_view fp(filepath, filepath + std::strlen(filepath));
    //std::cout << img.format << "\n";
    if(img.format == GRAYSCALE){
        if(fp.ends_with(".png")){
            stbi_write_png(filepath, img.width, img.height, 2, img.data, img.width * sizeof(uint16_t));
            return;
        }
        else{
            TRACELOG(LOG_WARNING, "Grayscale can only export to png");
        }
    }
    size_t stride = std::ceil(img.rowStrideInBytes);
    if(stride == 0){
        stride = img.width * sizeof(Color);
    }
    ImageFormat(&img, PixelFormat::RGBA8);
    if(fp.ends_with(".png")){
        stbi_write_png(filepath, img.width, img.height, 4, img.data, stride);
    }
    else if(fp.ends_with(".jpg")){
        stbi_write_jpg(filepath, img.width, img.height, 4, img.data, 100);
    }
    else if(fp.ends_with(".bmp")){
        //if(row)
        //std::cerr << "Careful with bmp!" << filepath << "\n";
        stbi_write_bmp(filepath, img.width, img.height, 4, img.data);
    }
    else{
        TRACELOG(LOG_ERROR, "Unrecognized image format in filename %s", filepath);
    }
}
void UseTexture(Texture tex){
    if(GetUniformLocation(GetActivePipeline(), "colDiffuse") == LOCATION_NOT_FOUND){
        return;
    }
    if(g_wgpustate.rstate->activePipeline->bindGroup.entries[1].textureView == tex.view)return;
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
    uint32_t colDiffuseLocation = GetUniformLocation(GetActivePipeline(), "colDiffuse");
    if(colDiffuseLocation != LOCATION_NOT_FOUND){
        if(g_wgpustate.rstate->activePipeline->bindGroup.entries[colDiffuseLocation].textureView == g_wgpustate.whitePixel.view)return;
        drawCurrentBatch();
        SetTexture(colDiffuseLocation, g_wgpustate.whitePixel);
        //uint32_t samplerLocation = GetUniformLocation(GetActivePipeline(), "texSampler");
        //if(samplerLocation != LOCATION_NOT_FOUND)
        //    SetSampler(samplerLocation, g_wgpustate.defaultSampler);
    }
    //WGPUBindGroupEntry entry{};
    //entry.binding = 1;
    //entry.textureView = g_wgpustate.whitePixel.view;
    //UpdateBindGroupEntry(&g_wgpustate.rstate->currentPipeline->bindGroup, 1, entry);
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
    g_wgpustate.rstate->renderExtentX = rtex.texture.width;
    g_wgpustate.rstate->renderExtentY = rtex.texture.height;
    //std::cout << std::format("{} x {}\n", g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    setTargetTextures(g_wgpustate.rstate, rtex.texture.view, rtex.colorMultisample.view, rtex.depth.view);
    Matrix mat = ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
void EndTextureMode(){
    drawCurrentBatch();
    g_wgpustate.rstate->renderExtentX = g_wgpustate.currentDefaultRenderTarget.texture.width;
    g_wgpustate.rstate->renderExtentY = g_wgpustate.currentDefaultRenderTarget.texture.height;
    setTargetTextures(g_wgpustate.rstate, 
                    g_wgpustate.currentDefaultRenderTarget.texture.view, 
                    g_wgpustate.currentDefaultRenderTarget.colorMultisample.view,
                    g_wgpustate.currentDefaultRenderTarget.depth.view);
    Matrix mat = ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
    //SetUniformBuffer(0, g_wgpustate.defaultScreenMatrix);
}
extern "C" void BeginWindowMode(SubWindow sw){
    g_wgpustate.activeSubWindow = sw;
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(sw.surface, &surfaceTexture);


    WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture,nullptr);
    //wgpuTextureViewRelease(g_wgpustate.activeSubWindow.frameBuffer.color.view);
    //wgpuTextureRelease(g_wgpustate.activeSubWindow.frameBuffer.color.id);
    sw.frameBuffer.texture.view = nextTexture;
    sw.frameBuffer.texture.id = surfaceTexture.texture;
    g_wgpustate.currentDefaultRenderTarget = sw.frameBuffer;
    BeginTextureMode(sw.frameBuffer);
    BeginRenderpass();
}
extern "C" void EndWindowMode(){
    EndRenderpass();
    EndTextureMode();
    g_wgpustate.currentDefaultRenderTarget = g_wgpustate.mainWindowRenderTarget;
    wgpuSurfacePresent(g_wgpustate.activeSubWindow.surface);
    g_wgpustate.activeSubWindow = SubWindow zeroinit;
    return;

    //Bad implementation:
    /*drawCurrentBatch();
    g_wgpustate.rstate->renderExtentX = GetScreenWidth();
    g_wgpustate.rstate->renderExtentY = GetScreenHeight();
    auto state = g_wgpustate.rstate;
    auto& c = g_wgpustate.currentScreenTextureView;
    auto& cms = colorMultisample.view; 
    auto d = depthTexture.view;
    state->color = c;
    state->depth = d;
    //LoadTexture
    
    WGPUTextureDescriptor desc{};
    const bool multisample = g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT;
    //if(colorMultisample.id == nullptr && multisample){
    //    colorMultisample = LoadTexturePro(GetScreenWidth(), GetScreenHeight(), g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4);
    //}
    state->renderpass.rca->resolveTarget = multisample ? c : nullptr;
    state->renderpass.rca->view = multisample ? cms : c;
    if(state->renderpass.settings.depthTest){
        state->renderpass.dsa->view = d;
    }

    //TODO: Not hardcode every renderpass here
    state->clearPass.rca->view = multisample ? cms : c;
    if(state->clearPass.settings.depthTest){
        state->clearPass.dsa->view = d;
    }
    wgpuSurfacePresent(g_wgpustate.activeSubWindow.surface);*/
    
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
    wgpuInstanceWaitAny(g_wgpustate.instance.Get(), 1, &winfo, UINT64_MAX);
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
int GetRandomValue(int min, int max){
    int w = (max - min);
    int v = rand() & w;
    return v + min;
}
extern "C" DescribedBuffer* GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage){
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    ret->descriptor.size = size;
    ret->descriptor.mappedAtCreation = false;
    ret->descriptor.usage = usage;
    ret->buffer = wgpuDeviceCreateBuffer(GetDevice(), &ret->descriptor);
    if(data != nullptr){
        wgpuQueueWriteBuffer(GetQueue(), ret->buffer, 0, data, size);
    }
    return ret;
}
extern "C" DescribedBuffer* GenBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex);
}
DescribedBuffer* GenIndexBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
}
DescribedBuffer* GenUniformBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform);
}
DescribedBuffer* GenStorageBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage);
}
void UnloadBuffer(DescribedBuffer* buffer){
    wgpuBufferRelease(buffer->buffer);
    free(buffer);
}

extern "C" void BufferData(DescribedBuffer* buffer, const void* data, size_t size){
    if(buffer->descriptor.size >= size){
        wgpuQueueWriteBuffer(GetQueue(), buffer->buffer, 0, data, size);
    }
    else{
        if(buffer->buffer)
            wgpuBufferRelease(buffer->buffer);
        buffer->descriptor.size = size;
        buffer->buffer = wgpuDeviceCreateBuffer(GetDevice(), &(buffer->descriptor));
        wgpuQueueWriteBuffer(GetQueue(), buffer->buffer, 0, data, size);
    }
}
extern "C" void SetTargetFPS(int fps){
    g_wgpustate.targetFPS = fps;
}
extern "C" int GetTargetFPS(){
    return g_wgpustate.targetFPS;
}
extern "C" uint64_t GetFrameCount(){
    return g_wgpustate.total_frames;
}
//TODO: this is bad
extern "C" float GetFrameTime(){
    if(g_wgpustate.total_frames == 0){
        return 0.0f;
    }
    if(g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] - g_wgpustate.last_timestamps[(g_wgpustate.total_frames - 1) % 64] < 0){
        return 1.0e-9f * (g_wgpustate.last_timestamps[(g_wgpustate.total_frames - 1) % 64] - g_wgpustate.last_timestamps[(g_wgpustate.total_frames - 2) % 64]);
    } 
    return 1.0e-9f * (g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] - g_wgpustate.last_timestamps[(g_wgpustate.total_frames - 1) % 64]);
}
extern "C" void SetConfigFlags(int /* enum WindowFlag */ flag){
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
FILE* tracelogFile = stdout;
int tracelogLevel = LOG_INFO;
void SetTraceLogFile(FILE* file){
    tracelogFile = file;
}
void SetTraceLogLevel(int logLevel){
    logLevel = logLevel;
}
void TraceLog(int logType, const char *text, ...){
    // Message has level below current threshold, don't emit
    if(logType < tracelogLevel)return;

    va_list args;
    va_start(args, text);

    //if (traceLog){
    //    traceLog(logType, text, args);
    //    va_end(args);
    //    return;
    //}
    constexpr size_t MAX_TRACELOG_MSG_LENGTH = 2048;
    char buffer[MAX_TRACELOG_MSG_LENGTH] = { 0 };
    int needs_reset = 0;
    switch (logType)
    {
        case LOG_TRACE: strcpy(buffer, "TRACE: "); break;
        case LOG_DEBUG: strcpy(buffer, "DEBUG: "); break;
        case LOG_INFO: strcpy(buffer, TERMCTL_GREEN "INFO: "); needs_reset = 1;break;
        case LOG_WARNING: strcpy(buffer, TERMCTL_YELLOW "WARNING: ");needs_reset = 1; break;
        case LOG_ERROR: strcpy(buffer, TERMCTL_RED "ERROR: ");needs_reset = 1; break;
        case LOG_FATAL: strcpy(buffer, TERMCTL_RED "FATAL: "); break;
        default: break;
    }
    size_t offset_now = strlen(buffer);
    
    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (MAX_TRACELOG_MSG_LENGTH - 12))? textSize : (MAX_TRACELOG_MSG_LENGTH - 12));
    if(needs_reset){
        strcat(buffer, TERMCTL_RESET "\n");
    }
    else{
        strcat(buffer, "\n");
    }
    vfprintf(tracelogFile, buffer, args);
    fflush  (tracelogFile);

    va_end(args);
    // If fatal logging, exit program
    if (logType == LOG_FATAL){
        fputs(TERMCTL_RED "Exiting due to fatal error!\n" TERMCTL_RESET, tracelogFile);
        exit(EXIT_FAILURE); 
    }

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

unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize)
{
    #define COMPRESSION_QUALITY_DEFLATE  8

    unsigned char *compData = NULL;

    // Compress data and generate a valid DEFLATE stream
    struct sdefl *sdefl = (struct sdefl *)calloc(1, sizeof(struct sdefl));   // WARNING: Possible stack overflow, struct sdefl is almost 1MB
    int bounds = sdefl_bound(dataSize);
    compData = (unsigned char *)calloc(bounds, 1);

    *compDataSize = sdeflate(sdefl, compData, data, dataSize, COMPRESSION_QUALITY_DEFLATE);   // Compression level 8, same as stbiw
    RL_FREE(sdefl);

    TRACELOG(LOG_INFO, "SYSTEM: Compress data: Original size: %i -> Comp. size: %i", dataSize, *compDataSize);

    return compData;
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;

    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }

    return latestMatch;
}
template<typename callable>
    requires(std::is_invocable_r_v<bool, callable, const std::filesystem::path&>)
std::optional<std::filesystem::path> breadthFirstSearch(const std::filesystem::path& paf, int depth, callable lambda){
    namespace fs = std::filesystem;
    std::deque<std::pair<fs::path, int>> deq;
    deq.emplace_back(paf, 0);
    while(!deq.empty()){
        auto [elem, d] = deq.front();
        deq.pop_front();
        if(lambda(elem)){
            return elem;
        }
        if(fs::is_directory(elem) && d < depth){
            fs::directory_iterator dirit(elem);
            for(auto& paf : dirit){
                deq.emplace_back(paf, d + 1);
            }
        }
    }
    return std::nullopt;
}
const char* FindDirectory(const char* directoryName, int maxOutwardSearch){
    static char dirPaff[2048] = {0};
    namespace fs = std::filesystem;

    fs::path searchPath(".");
    std::optional<fs::path> path = std::nullopt;
    for(int i = 0;i < maxOutwardSearch;i++){
        auto pathopt = breadthFirstSearch(searchPath, 2, [directoryName](const fs::path& p){
            //TRACELOG(LOG_WARNING, "%s", p.string().c_str());
            return p.filename().string() == directoryName;
        });
        if(pathopt){
            path = std::move(pathopt);
            break;
        }
        searchPath /= fs::path("..");
    }
    //for(int i = 0;i < maxOutwardSearch;i++, searchPath /= ".."){
    //    fs::recursive_directory_iterator iter(searchPath);
    //    for(auto& entry : iter){
    //        if(entry.path().filename().string() == directoryName){
    //            if(entry.is_directory()){
    //                strcpy(dirPaff, entry.path().string().c_str());
    //                goto end;
    //            }else{
    //                TRACELOG(LOG_WARNING, "Found file %s, but it's not a directory", entry.path().c_str());
    //            }
    //        }
    //    }
    //}
    //abort();
    if(!path.has_value()){
        TRACELOG(LOG_WARNING, "Directory %s not found", directoryName);
        return nullptr;
    }
    std::string val = path.value().string();
    *std::copy(val.begin(), val.end(), dirPaff) = '\0';
    return dirPaff;
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
char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize)
{
    static const unsigned char base64encodeTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    static const int modTable[] = { 0, 2, 1 };

    *outputSize = 4*((dataSize + 2)/3);

    char *encodedData = (char *)malloc(*outputSize);

    if (encodedData == NULL) return NULL;   // Security check

    for (int i = 0, j = 0; i < dataSize;)
    {
        unsigned int octetA = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetB = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetC = (i < dataSize)? (unsigned char)data[i++] : 0;

        unsigned int triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        encodedData[j++] = base64encodeTable[(triple >> 3*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 2*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 1*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 0*6) & 0x3F];
    }

    for (int i = 0; i < modTable[dataSize%3]; i++) encodedData[*outputSize - 1 - i] = '=';  // Padding character

    return encodedData;
}

// Decode Base64 string data
unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize)
{
    static const unsigned char base64decodeTable[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    // Get output size of Base64 input data
    int outSize = 0;
    for (int i = 0; data[4*i] != 0; i++)
    {
        if (data[4*i + 3] == '=')
        {
            if (data[4*i + 2] == '=') outSize += 1;
            else outSize += 2;
        }
        else outSize += 3;
    }

    // Allocate memory to store decoded Base64 data
    unsigned char *decodedData = (unsigned char *)malloc(outSize);

    for (int i = 0; i < outSize/3; i++)
    {
        unsigned char a = base64decodeTable[(int)data[4*i]];
        unsigned char b = base64decodeTable[(int)data[4*i + 1]];
        unsigned char c = base64decodeTable[(int)data[4*i + 2]];
        unsigned char d = base64decodeTable[(int)data[4*i + 3]];

        decodedData[3*i] = (a << 2) | (b >> 4);
        decodedData[3*i + 1] = (b << 4) | (c >> 2);
        decodedData[3*i + 2] = (c << 6) | d;
    }

    if (outSize%3 == 1)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        decodedData[outSize - 1] = (a << 2) | (b >> 4);
    }
    else if (outSize%3 == 2)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        unsigned char c = base64decodeTable[(int)data[4*n + 2]];
        decodedData[outSize - 2] = (a << 2) | (b >> 4);
        decodedData[outSize - 1] = (b << 4) | (c >> 2);
    }

    *outputSize = outSize;
    return decodedData;
}
#include "telegrama_render_literal.inc"
size_t telegrama_render_size = sizeof(telegrama_render);