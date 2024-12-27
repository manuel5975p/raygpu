#include "raygpu.h"
#include <GLFW/glfw3.h>
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
#include <sdefl.h>

Vector2 nextuv;
Vector3 nextnormal;
Vector4 nextcol;
vertex* vboptr;
vertex* vboptr_base;
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
    return callocnew(VertexArray);
}
extern "C" void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, WGPUVertexFormat format, uint32_t offset, WGPUVertexStepMode stepmode){
    array->add(buffer, attribLocation, format, offset, stepmode);
}

extern "C" void BindVertexArray(DescribedPipeline* pipeline, VertexArray* va){
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
extern "C" void DrawArrays(uint32_t vertexCount){
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderDraw(rp, vertexCount, 1, 0, 0);
}
extern "C" void DrawArraysIndexed(DescribedBuffer indexBuffer, uint32_t vertexCount){
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderSetIndexBuffer(rp, indexBuffer.buffer, WGPUIndexFormat_Uint32, 0, indexBuffer.descriptor.size);
    wgpuRenderPassEncoderDrawIndexed(rp, vertexCount, 1, 0, 0, 0);
}
extern "C" void DrawArraysIndexedInstanced(DescribedBuffer indexBuffer, uint32_t vertexCount, uint32_t instanceCount){
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderSetIndexBuffer(rp, indexBuffer.buffer, WGPUIndexFormat_Uint32, 0, indexBuffer.descriptor.size);
    wgpuRenderPassEncoderDrawIndexed(rp, vertexCount, instanceCount, 0, 0, 0);
}
extern "C" void DrawArraysInstanced(uint32_t vertexCount, uint32_t instanceCount){
    auto& rp = g_wgpustate.rstate->renderpass.rpEncoder;
    if(g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate){
        wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->activeRenderpass->rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, nullptr);
    }
    wgpuRenderPassEncoderDraw(rp, vertexCount, instanceCount, 0, 0);
}
Texture depthTexture; //TODO: uhhh move somewhere
WGPUDevice GetDevice(){
    return g_wgpustate.device;
}
Texture GetDepthTexture(){
    return depthTexture;
}
WGPUQueue GetQueue(){
    return g_wgpustate.queue;
}


void drawCurrentBatch(){
    size_t vertexCount = vboptr - vboptr_base;
    //std::cout << "vcoun = " << vertexCount << "\n";
    if(vertexCount == 0)return;
    //std::cout << "vboptr reset" << std::endl;
    
    UpdateBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup);
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
            //TODO: Line texturing is currently disable in all DrawLine... functions
            SetTexture(1, g_wgpustate.whitePixel);
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_LineList);
            
            wgpuRenderPassEncoderSetBindGroup(g_wgpustate.rstate->renderpass.rpEncoder, 0, GetWGPUBindGroup(&g_wgpustate.rstate->activePipeline->bindGroup), 0, 0);
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            wgpuRenderPassEncoderDraw(g_wgpustate.rstate->renderpass.rpEncoder, vertexCount, 1, 0, 0);
            if(!allocated_via_pool){
                wgpuBufferRelease(vbo.buffer);
            }
            else{
                g_wgpustate.smallBufferRecyclingBin.push_back(vbo);
            }
            g_wgpustate.rstate->activePipeline->bindGroup.needsUpdate = true;
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
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
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
            BindPipeline(g_wgpustate.rstate->activePipeline, WGPUPrimitiveTopology_TriangleList);
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
    g_wgpustate.rstate->activePipeline = pipeline;
    SetUniformBufferData(0, &g_wgpustate.activeScreenMatrix, sizeof(Matrix));
    BindPipeline(pipeline, drawMode);
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
    SetUniformBuffer(0, &g_wgpustate.defaultScreenMatrix);
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
    SetUniformBuffer(0, &g_wgpustate.defaultScreenMatrix);
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
extern "C" void BindComputePipeline(DescribedComputePipeline* pipeline){
    wgpuComputePassEncoderSetPipeline(g_wgpustate.rstate->computepass.cpEncoder, pipeline->pipeline);
    wgpuComputePassEncoderSetBindGroup (g_wgpustate.rstate->computepass.cpEncoder, 0, GetWGPUBindGroup(&pipeline->bindGroup), 0, 0);
}
void CopyBufferToBuffer(DescribedBuffer* source, DescribedBuffer* dest, size_t count){
    wgpuCommandEncoderCopyBufferToBuffer(g_wgpustate.rstate->computepass.cmdEncoder, source->buffer, 0, dest->buffer, 0, count);
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
    g_wgpustate.rstate->activeRenderpass = renderPass;
}

void EndRenderpassEx(DescribedRenderpass* renderPass){
    drawCurrentBatch();
    wgpuRenderPassEncoderEnd(renderPass->rpEncoder);
    g_wgpustate.rstate->activeRenderpass = nullptr;
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
void BeginDrawing(){
    g_wgpustate.last_timestamps[g_wgpustate.total_frames % 64] = NanoTime();
    
    g_wgpustate.drawmutex.lock();
    g_wgpustate.rstate->renderExtentX = GetScreenWidth();
    g_wgpustate.rstate->renderExtentY = GetScreenHeight();
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
    if(g_wgpustate.rstate->activeRenderpass){
        drawCurrentBatch();
        EndRenderpassEx(g_wgpustate.rstate->activeRenderpass);
    }
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
    g_wgpustate.scrollPreviousFrame = g_wgpustate.scrollThisFrame;
    g_wgpustate.scrollThisFrame = Vector2{0, 0};
    std::copy(g_wgpustate.mouseButtonDown.begin(), g_wgpustate.mouseButtonDown.end(), g_wgpustate.mouseButtonDownPrevious.begin());
    g_wgpustate.drawmutex.unlock();
    uint64_t nanosecondsPerFrame = std::floor(1e9 / GetTargetFPS());
    //std::cout << nanosecondsPerFrame << "\n";
    uint64_t elapsed = NanoTime() - beginframe_stmp;
    if(elapsed & (1ull << 63))return;
    if(!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) && nanosecondsPerFrame > elapsed && GetTargetFPS() > 0)
        NanoWait(nanosecondsPerFrame - elapsed);
    glfwPollEvents();

    //std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondsPerFrame - elapsed));
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
Image LoadImageFromTextureEx(WGPUTexture tex){
    Image ret {(PixelFormat)wgpuTextureGetFormat(tex), wgpuTextureGetWidth(tex), wgpuTextureGetHeight(tex), size_t(std::ceil(4.0 * wgpuTextureGetWidth(tex) / 256.0) * 256), nullptr, 1};
    WGPUBufferDescriptor b{};
    b.mappedAtCreation = false;
    uint32_t width = wgpuTextureGetWidth(tex);
    uint32_t height = wgpuTextureGetHeight(tex);
    b.size = size_t(std::ceil(4.0 * width / 256.0) * 256) * height;
    b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
    
    wgpu::Buffer readtex = wgpuDeviceCreateBuffer(GetDevice(), &b);
    
    WGPUCommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = STRVIEW("Command Encoder");
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
    WGPUImageCopyTexture tbsource{};
    tbsource.texture = tex;
    tbsource.mipLevel = 0;
    tbsource.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    tbsource.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUImageCopyBuffer tbdest{};
    tbdest.buffer = readtex.Get();
    tbdest.layout.offset = 0;
    tbdest.layout.bytesPerRow = std::ceil(4.0 * wgpuTextureGetWidth(tex) / 256.0) * 256;
    tbdest.layout.rowsPerImage = height;

    WGPUExtent3D copysize{width, height, 1};
    wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &copysize);
    
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);
    wgpuQueueSubmit(g_wgpustate.queue, 1, &command);
    wgpuCommandBufferRelease(command);
    //#ifdef WEBGPU_BACKEND_DAWN
    //wgpuDeviceTick(GetDevice());
    //#else
    //#endif
    std::pair<Image*, wgpu::Buffer*> ibpair{&ret, &readtex};
    auto onBuffer2Mapped = [&ibpair](wgpu::MapAsyncStatus status, const char* userdata){
        //std::cout << "Backcalled: " << status << std::endl;
        if(status != wgpu::MapAsyncStatus::Success){
            TRACELOG(LOG_ERROR, "onBuffer2Mapped called back with error!");
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
    wgpu::Future fut = readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * wgpuTextureGetWidth(tex) / 256.0) * 256) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped);
    winfo.future = fut;
    wgpuInstanceWaitAny(g_wgpustate.instance, 1, &winfo, 1000000000);
    #else 
    //readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * width / 256.0) * 256) * height, onBuffer2Mapped, &ibpair);
    readtex.MapAsync(wgpu::MapMode::Read, 0, size_t(std::ceil(4.0 * wgpuTextureGetWidth(tex) / 256.0) * 256) * height, wgpu::CallbackMode::WaitAnyOnly, onBuffer2Mapped);
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
    return ret;
}
Image LoadImageFromTexture(Texture tex){
    //#ifndef __EMSCRIPTEN__
    //auto& device = g_wgpustate.device;
    return LoadImageFromTextureEx(tex.id);
    //#else
    //std::cerr << "LoadImageFromTexture not supported on web\n";
    //return Image{};
    //#endif
}
void TakeScreenshot(const char* filename){
    Image img = LoadImageFromTextureEx(g_wgpustate.currentSurfaceTexture.texture);
    SaveImage(img, filename);
    UnloadImage(img);
}
Texture LoadTextureFromImage(Image img){
    Texture ret;
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
    return g_wgpustate.scrollPreviousFrame.y;
}
Vector2 GetMouseWheelMoveV(void){
    return g_wgpustate.scrollPreviousFrame;
    //return Vector2{
    //    (float)(g_wgpustate.globalScrollX - g_wgpustate.globalScrollYPrevious),
    //    (float)(g_wgpustate.globalScrollY - g_wgpustate.globalScrollYPrevious)};
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
    #ifndef __EMSCRIPTEN__
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    #endif
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

    state->activePipeline = LoadPipelineEx(shaderSource, attribs, attribCount, uniforms, uniform_count, settings);
    state->defaultPipeline = state->activePipeline;
    //WGPUBufferDescriptor vbmdesc{};
    //vbmdesc.mappedAtCreation = true;
    //vbmdesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_MapWrite;
    //vbmdesc.size = (1 << 22) * sizeof(vertex);
    //vbomap.buffer = wgpuDeviceCreateBuffer(GetDevice(), &vbmdesc);
    //vbomap.descriptor = vbmdesc;
    vboptr_base = nullptr;
    //vboptr = (vertex*)wgpuBufferGetMappedRange(vbomap.buffer, 0, vbmdesc.size);
    vboptr = (vertex*)calloc(10000, sizeof(vertex));
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
void SetSampler                   (uint32_t index, WGPUSampler sampler){
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



void SetPipelineUniformBuffer (DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
}
void SetPipelineStorageBuffer (DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->descriptor.size;
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
}

void SetPipelineUniformBufferData (DescribedPipeline* pl, uint32_t index, const void* data, size_t size){
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
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
}
void SetPipelineStorageBufferData (DescribedPipeline* pl, uint32_t index, const void* data, size_t size){
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
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
}
extern "C" void SetPipelineTexture(DescribedPipeline* pl, uint32_t index, Texture tex){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
}
extern "C" void SetPipelineSampler(DescribedPipeline* pl, uint32_t index, WGPUSampler sampler){
    WGPUBindGroupEntry entry{};
    entry.binding = index;
    entry.sampler = sampler;
    UpdateBindGroupEntry(&pl->bindGroup, index, entry);
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
DescribedSampler LoadSampler(addressMode amode, filterMode fmode){
    DescribedSampler ret zeroinit;
    ret.desc.magFilter    = (WGPUFilterMode)fmode;
    ret.desc.minFilter    = (WGPUFilterMode)fmode;
    ret.desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    ret.desc.compare      = WGPUCompareFunction_Undefined;
    ret.desc.lodMinClamp  = 0.0f;
    ret.desc.lodMaxClamp  = 1.0f;
    ret.desc.maxAnisotropy = 1;
    ret.desc.addressModeU = (WGPUAddressMode)amode;
    ret.desc.addressModeV = (WGPUAddressMode)amode;
    ret.desc.addressModeW = (WGPUAddressMode)amode;
    ret.sampler = wgpuDeviceCreateSampler(GetDevice(), &ret.desc);
    return ret;
}
void UnloadSampler(DescribedSampler sampler){
    wgpuSamplerRelease(sampler.sampler);
}

WGPUTexture GetActiveColorTarget(){
    return g_wgpustate.currentSurfaceTexture.texture;
}
WGPUTexture cres = 0;
WGPUTextureView cresv = 0;
void setTargetTextures(full_renderstate* state, WGPUTextureView c, WGPUTextureView d){
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
    std::string fn_cpp(filename);
    const char* extbegin = fn_cpp.data() + fn_cpp.rfind("."); 
    void* data = LoadFileData(filename, &size);
    if(size != 0){
        Image ld =  LoadImageFromMemory(extbegin, data, size);
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
    
    BGRAColor* cols = (BGRAColor*)img.data; 
    Color* ocols = (Color*)calloc(stride * img.height, sizeof(Color));
    for(size_t i = 0;i < (stride / sizeof(Color)) * img.height;i++){
        ocols[i].r = cols[i].r;
        ocols[i].g = cols[i].g;
        ocols[i].b = cols[i].b;
        ocols[i].a = 255;
    }
    if(fp.ends_with(".png")){
        stbi_write_png(filepath, img.width, img.height, 4, ocols, img.width * sizeof(Color));
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
    if(g_wgpustate.rstate->activePipeline->bindGroup.entries[1].textureView == g_wgpustate.whitePixel.view)return;

    drawCurrentBatch();
    SetTexture(1, g_wgpustate.whitePixel);
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
    g_wgpustate.rstate->renderExtentX = rtex.color.width;
    g_wgpustate.rstate->renderExtentY = rtex.color.height;
    setTargetTextures(g_wgpustate.rstate, rtex.color.view, rtex.depth.view);
    Matrix mat = ScreenMatrix(g_wgpustate.rstate->renderExtentX, g_wgpustate.rstate->renderExtentY);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
void EndTextureMode(){
    drawCurrentBatch();
    g_wgpustate.rstate->renderExtentX = GetScreenWidth();
    g_wgpustate.rstate->renderExtentY = GetScreenHeight();
    setTargetTextures(g_wgpustate.rstate, g_wgpustate.currentSurfaceTextureView, depthTexture.view);
    SetUniformBuffer(0, &g_wgpustate.defaultScreenMatrix);
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
int GetRandomValue(int min, int max){
    int w = (max - min);
    int v = rand() & w;
    return v + min;
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
DescribedBuffer GenIndexBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
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
//TODO: this is bad
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
void TraceLog(int logType, const char *text, ...){
    // Message has level below current threshold, don't emit
    //if (logType < logTypeLevel) return;

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
        case LOG_FATAL: strcpy(buffer, "FATAL: "); break;
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
    vprintf(buffer, args);
    fflush(stdout);

    va_end(args);

    if (logType == LOG_FATAL) exit(EXIT_FAILURE);  // If fatal logging, exit program

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

char telegrama_render[] = "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";
size_t telegrama_render_size = sizeof(telegrama_render);