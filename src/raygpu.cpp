/*
 * MIT License
 * 
 * Copyright (c) 2025 @manuel5975p
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <raygpu.h>

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
#include <deque>
#include <map>
//#include <webgpu/webgpu_cpp.h>

#include <external/stb_image_write.h>
#include <external/stb_image.h>
#include <external/sinfl.h>
#include <external/sdefl.h>
#include <internals.hpp>
#include <external/msf_gif.h>
extern "C" void ToggleFullscreenImpl(cwoid);
#ifdef __EMSCRIPTEN__
#endif  // __EMSCRIPTEN__
#include <renderstate.inc>
renderstate g_renderstate{};

ShaderSourceType detectShaderLanguage(std::string_view source){
    if(source.find("@location") != std::string::npos || source.find("@binding") != std::string::npos){
        return sourceTypeWGSL;
    }
    else if(source.find("#version") != std::string::npos){
        return sourceTypeGLSL;
    }
    else{
        return sourceTypeUnknown;
    }
}

typedef struct GIFRecordState{
    uint64_t delayInCentiseconds;
    uint64_t lastFrameTimestamp;
    MsfGifState msf_state;
    uint64_t numberOfFrames;
    bool recording;
}GIFRecordState;

void startRecording(GIFRecordState* grst, uint64_t delayInCentiseconds){
    if(grst->recording){
        TRACELOG(LOG_WARNING, "Already recording");
        return;
    }
    else{
        grst->numberOfFrames = 0;
    }
    msf_gif_bgra_flag = true;
    grst->msf_state = MsfGifState{};
    grst->delayInCentiseconds = delayInCentiseconds;
    msf_gif_begin(&grst->msf_state, GetScreenWidth(), GetScreenHeight());
    grst->recording = true;
}

void addScreenshot(GIFRecordState* grst, WGVKTexture tex){
    //#ifdef __EMSCRIPTEN__
    //if(grst->numberOfFrames > 0)
    //    msf_gif_frame(&grst->msf_state, (uint8_t*)fbLoad.data, grst->delayInCentiseconds, 8, fbLoad.rowStrideInBytes);
    //#endif
    grst->lastFrameTimestamp = NanoTime();
    Image fb = LoadImageFromTextureEx(tex, 0);
    //#ifndef __EMSCRIPTEN__
    msf_gif_frame(&grst->msf_state, (uint8_t*)fb.data, grst->delayInCentiseconds, 8, fb.rowStrideInBytes);
    //#endif
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
PrimitiveType current_drawmode;
//DescribedBuffer vbomap;
#ifdef _WIN32
#define __builtin_unreachable(...)
#endif

extern "C" VertexArray* LoadVertexArray(){
    VertexArray* ret = callocnew(VertexArray);
    new (ret) VertexArray;
    return ret;
}
extern "C" void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, VertexFormat format, uint32_t offset, VertexStepMode stepmode){
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
            RenderPassSetVertexBuffer(GetActiveRenderPass(), 
                                                i, 
                                                bufferPair.first, 
                                                0);
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
extern "C" void DrawArrays(PrimitiveType drawMode, uint32_t vertexCount){
    BindPipeline(GetActivePipeline(), drawMode);

    if(GetActivePipeline()->bindGroup.needsUpdate){
        RenderPassSetBindGroup(GetActiveRenderPass(), 0, &GetActivePipeline()->bindGroup);
    }
    RenderPassDraw(GetActiveRenderPass(), vertexCount, 1, 0, 0);
}
extern "C" void DrawArraysIndexed(PrimitiveType drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount){
    BindPipeline(GetActivePipeline(), drawMode);
    //PreparePipeline(GetActivePipeline(), VertexArray *va)
    //TRACELOG(LOG_INFO, "a oooo");
    //auto& rp = g_renderstate.renderpass.rpEncoder;  
    if(GetActivePipeline()->bindGroup.needsUpdate){
        RenderPassSetBindGroup(GetActiveRenderPass(), 0, &GetActivePipeline()->bindGroup);
    }
    RenderPassSetIndexBuffer(GetActiveRenderPass(), &indexBuffer, IndexFormat_Uint32, 0);
    RenderPassDrawIndexed(GetActiveRenderPass(), vertexCount, 1, 0, 0, 0);
}
extern "C" void DrawArraysIndexedInstanced(PrimitiveType drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount, uint32_t instanceCount){
    BindPipeline(GetActivePipeline(), drawMode);
    if(GetActivePipeline()->bindGroup.needsUpdate){
        RenderPassSetBindGroup(GetActiveRenderPass(), 0, &GetActivePipeline()->bindGroup);
    }
    RenderPassSetIndexBuffer(GetActiveRenderPass(), &indexBuffer, IndexFormat_Uint32, 0);
    RenderPassDrawIndexed(GetActiveRenderPass(), vertexCount, instanceCount, 0, 0, 0);
}
extern "C" void DrawArraysInstanced(PrimitiveType drawMode, uint32_t vertexCount, uint32_t instanceCount){
    BindPipeline(GetActivePipeline(), drawMode);
    if(GetActivePipeline()->bindGroup.needsUpdate){
        RenderPassSetBindGroup(GetActiveRenderPass(), 0, &GetActivePipeline()->bindGroup);
    }
    RenderPassDraw(GetActiveRenderPass(), vertexCount, instanceCount, 0, 0);
}


Texture GetDepthTexture(){
    return g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].depth;
}
Texture GetMultisampleColorTarget(){
    return g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].colorMultisample;
}


void drawCurrentBatch(){
    size_t vertexCount = vboptr - vboptr_base;
    //std::cout << "vcoun = " << vertexCount << "\n";
    if(vertexCount == 0)return;
    
    DescribedBuffer* vbo;
    bool allocated_via_pool = false;
    if(vertexCount < 32 && !g_renderstate.smallBufferPool.empty()){
        allocated_via_pool = true;
        vbo = g_renderstate.smallBufferPool.back();
        g_renderstate.smallBufferPool.pop_back();
        #if SUPPORT_VULKAN_BACKEND == 1
        BufferData(vbo, vboptr_base, vertexCount * sizeof(vertex));
        #else
        wgpuQueueWriteBuffer(GetQueue(), (WGPUBuffer)vbo->buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
        #endif
    }
    else{
        vbo = GenVertexBuffer(vboptr_base, vertexCount * sizeof(vertex));
    }

    BufferData(vbo, vboptr_base, vertexCount * sizeof(vertex));
    renderBatchVAO->buffers.front().first = vbo;
    SetStorageBuffer(3, g_renderstate.identityMatrix);
    UpdateBindGroup(&GetActivePipeline()->bindGroup);
    switch(current_drawmode){
        case RL_LINES:{

            //TODO: Line texturing is currently disable in all DrawLine... functions
            SetTexture(1, g_renderstate.whitePixel);
            BindPipeline(GetActivePipeline(), RL_LINES);
            BindPipelineVertexArray(GetActivePipeline(), renderBatchVAO);
            DrawArrays(RL_LINES, vertexCount);
            //wgpuRenderPassEncoderSetBindGroup(g_renderstate.renderpass.rpEncoder, 0, GetWGPUBindGroup(&GetActivePipeline()->bindGroup), 0, 0);
            //wgpuRenderPassEncoderSetVertexBuffer(g_renderstate.renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            //wgpuRenderPassEncoderDraw(g_renderstate.renderpass.rpEncoder, vertexCount, 1, 0, 0);
            
            GetActivePipeline()->bindGroup.needsUpdate = true;
        }break;
        case RL_TRIANGLE_STRIP:{
            BindPipeline(GetActivePipeline(), RL_TRIANGLE_STRIP);
            BindPipelineVertexArray(GetActivePipeline(), renderBatchVAO);
            DrawArrays(RL_TRIANGLE_STRIP, vertexCount);
            break;
        }
        
        case RL_TRIANGLES:{
            //SetTexture(1, g_renderstate.whitePixel);
            BindPipeline(GetActivePipeline(), RL_TRIANGLES);
            BindPipelineVertexArray(GetActivePipeline(), renderBatchVAO);
            DrawArrays(RL_TRIANGLES, vertexCount);
            //abort();
            //vboptr = vboptr_base;
            //BindPipeline(g_renderstate.activePipeline, WGPUPrimitiveTopology_TriangleList);
            //wgpuRenderPassEncoderSetVertexBuffer(g_renderstate.renderpass.rpEncoder, 0, vbo.buffer, 0, wgpuBufferGetSize(vbo.buffer));
            //wgpuRenderPassEncoderDraw(g_renderstate.renderpass.rpEncoder, vertexCount, 1, 0, 0);
            //if(!allocated_via_pool){
            //    wgpuBufferRelease(vbo.buffer);
            //}
            //else{
            //    g_renderstate.smallBufferRecyclingBin.push_back(vbo);
            //}
        } break;
        case RL_QUADS:{
            const size_t quadCount = vertexCount / 4;
            
            if(true || g_renderstate.quadindicesCache->size < 6 * quadCount * sizeof(uint32_t)){
                std::vector<uint32_t> indices(6 * quadCount);
                for(size_t i = 0;i < quadCount;i++){
                    indices[i * 6 + 0] = (i * 4 + 0);
                    indices[i * 6 + 1] = (i * 4 + 1);
                    indices[i * 6 + 2] = (i * 4 + 3);
                    indices[i * 6 + 3] = (i * 4 + 1);
                    indices[i * 6 + 4] = (i * 4 + 2);
                    indices[i * 6 + 5] = (i * 4 + 3);
                }
                BufferData(g_renderstate.quadindicesCache, indices.data(), 6 * quadCount * sizeof(uint32_t));
                //if(g_renderstate.quadindicesCache->buffer){
                //    wgpuBufferRelease(g_renderstate.quadindicesCache->buffer);
                //}
                //g_renderstate.quadindicesCache = GenBufferEx(indices.data(), 6 * quadCount * sizeof(uint32_t), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index);
            }
            const DescribedBuffer* ibuf = g_renderstate.quadindicesCache;
            //BindPipeline(g_renderstate.activePipeline, WGPUPrimitiveTopology_TriangleList);
            //g_renderstate.activePipeline
            BindPipelineVertexArray(GetActivePipeline(), renderBatchVAO);
            DrawArraysIndexed(RL_TRIANGLES, *ibuf, quadCount * 6);

            //wgpuQueueWriteBuffer(GetQueue(), vbo.buffer, 0, vboptr_base, vertexCount * sizeof(vertex));
            
            //wgpuRenderPassEncoderSetVertexBuffer(g_renderstate.renderpass.rpEncoder, 0, vbo.buffer, 0, vertexCount * sizeof(vertex));
            //wgpuRenderPassEncoderSetIndexBuffer (g_renderstate.renderpass.rpEncoder, ibuf.buffer, WGPUIndexFormat_Uint32, 0, quadCount * 6 * sizeof(uint32_t));
            //wgpuRenderPassEncoderDrawIndexed    (g_renderstate.renderpass.rpEncoder, quadCount * 6, 1, 0, 0, 0);
            
            //if(!allocated_via_pool){
            //    wgpuBufferRelease(vbo.buffer);
            //}
            //else{
            //    g_renderstate.smallBufferRecyclingBin.push_back(vbo);
            //}
            
        } break;
        default:break;
    }
    if(!allocated_via_pool){
        UnloadBuffer(vbo);
    }
    else{
        g_renderstate.smallBufferRecyclingBin.push_back(vbo);
    }
    vboptr = vboptr_base;
}

void LoadIdentity(cwoid){
    g_renderstate.matrixStack[g_renderstate.stackPosition].first = MatrixIdentity();
}
void PushMatrix(){
    ++g_renderstate.stackPosition;
}
void PopMatrix(){
    --g_renderstate.stackPosition;
}
Matrix GetMatrix(){
    return g_renderstate.matrixStack[g_renderstate.stackPosition].first;
}
Matrix* GetMatrixPtr(){
    return &g_renderstate.matrixStack[g_renderstate.stackPosition].first;
}
void SetMatrix(Matrix m){
    g_renderstate.matrixStack[g_renderstate.stackPosition].first = m;
}

void adaptRenderPass(DescribedRenderpass* drp, RenderSettings settings){
    drp->settings = settings;
    //drp->renderPassDesc.colorAttachments = settings.depthTest ? drp->rca : nullptr;
    //drp->renderPassDesc.depthStencilAttachment = settings.depthTest ? drp->dsa : nullptr;
}
extern "C" void BeginPipelineMode(DescribedPipeline* pipeline){
    drawCurrentBatch();
    if(!RenderSettingsComptatible(pipeline->settings, g_renderstate.renderpass.settings)){
        EndRenderpass();
        adaptRenderPass(&g_renderstate.renderpass, pipeline->settings);
        BeginRenderpass();
    }
    g_renderstate.activePipeline = pipeline;
    uint32_t location = GetUniformLocation(pipeline, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION_VIEW);
    if(location != LOCATION_NOT_FOUND){
        SetUniformBufferData(location, &g_renderstate.matrixStack[g_renderstate.stackPosition], sizeof(Matrix));
    }
    //BindPipeline(pipeline, drawMode);
}
extern "C" void EndPipelineMode(){
    drawCurrentBatch();
    if(!RenderSettingsComptatible(g_renderstate.defaultPipeline->settings, g_renderstate.renderpass.settings)){
        EndRenderpass();
        adaptRenderPass(&g_renderstate.renderpass, g_renderstate.defaultPipeline->settings);
        BeginRenderpass();
    }
    g_renderstate.activePipeline = g_renderstate.defaultPipeline;
    //BindPipeline(g_renderstate.activePipeline, g_renderstate.activePipeline->lastUsedAs);
}
extern "C" void BeginMode2D(Camera2D camera){
    drawCurrentBatch();
    Matrix mat = GetCameraMatrix2D(camera);
    mat = MatrixMultiply(ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY), mat);
    PushMatrix();
    SetMatrix(mat);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
extern "C" void EndMode2D(){
    drawCurrentBatch();
    PopMatrix();
    //g_renderstate.activeScreenMatrix = ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY);
    
    SetUniformBufferData(0, GetMatrixPtr(), sizeof(Matrix));
}
void BeginMode3D(Camera3D camera){
    drawCurrentBatch();
    Matrix mat = GetCameraMatrix3D(camera, float(g_renderstate.renderExtentX) / g_renderstate.renderExtentY);
    //g_renderstate.activeScreenMatrix = mat;
    PushMatrix();
    SetMatrix(mat);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
}
void EndMode3D(){
    drawCurrentBatch();
    
    //g_renderstate.activeScreenMatrix = ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY);
    PopMatrix();
    SetUniformBufferData(0, GetMatrixPtr(), sizeof(Matrix));
}


uint32_t GetScreenWidth (cwoid){
    return g_renderstate.width;
}
uint32_t GetScreenHeight(cwoid){
    return g_renderstate.height;
}


void BeginRenderpass(cwoid){
    BeginRenderpassEx(&g_renderstate.renderpass);
}
void EndRenderpass(cwoid){
    EndRenderpassEx(&g_renderstate.renderpass);
}
extern "C" void ClearBackground(Color clearColor){
    bool rpActive = GetActiveRenderPass() != nullptr;
    DescribedRenderpass* backup = GetActiveRenderPass();
    if(rpActive){
        EndRenderpassEx(g_renderstate.activeRenderpass);
    }
    g_renderstate.clearPass.colorClear = DColor{clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0};
    BeginRenderpassEx(&g_renderstate.clearPass);
    EndRenderpassEx(&g_renderstate.clearPass);
    if(rpActive){
        BeginRenderpassEx(backup);
    }

}
void BeginComputepass(){
    BeginComputepassEx(&g_renderstate.computepass);
}

void EndComputepass(){
    EndComputepassEx(&g_renderstate.computepass);
}

#ifdef __EMSCRIPTEN__
typedef void (*FrameCallback)(void);
typedef void (*FrameCallbackArg)(void*);
// Workaround for JSPI not working in emscripten_set_main_loop. Loosely based on this code:
// https://github.com/emscripten-core/emscripten/issues/22493#issuecomment-2330275282
// This code only works with JSPI is enabled.
// I believe -sEXPORTED_RUNTIME_METHODS=getWasmTableEntry is technically necessary to link this.
EM_JS(void, requestAnimationFrameLoopWithJSPI_impl, (FrameCallback callback), {
    var wrappedCallback = WebAssembly.promising(getWasmTableEntry(callback));
    async function tick() {
        // Start the frame callback. 'await' means we won't call
        // requestAnimationFrame again until it completes.
        //var keepLooping = await wrappedCallback();
        //if (keepLooping) requestAnimationFrame(tick);
        await wrappedCallback();
        requestAnimationFrame(tick);
    }
    requestAnimationFrame(tick);
});
EM_JS(void, requestAnimationFrameLoopWithJSPIArg_impl, (FrameCallbackArg callback, void* userData), {
    var wrappedCallback = WebAssembly.promising(getWasmTableEntry(callback));
    async function tick() {
        // Start the frame callback. 'await' means we won't call
        // requestAnimationFrame again until it completes.
        //var keepLooping = await wrappedCallback();
        //if (keepLooping) requestAnimationFrame(tick);
        await wrappedCallback(userData);
        requestAnimationFrame(tick);
    }
    requestAnimationFrame(tick);
});
//#define emscripten_set_main_loop requestAnimationFrameLoopWithJSPI
#endif
void requestAnimationFrameLoopWithJSPIArg(void (*callback)(void*), void* userData, int, int){
    #ifdef __EMSCRIPTEN__
    requestAnimationFrameLoopWithJSPIArg_impl(callback, userData);
    #else
    TRACELOG(LOG_WARNING, "requestAnimationFrame not supported outside of emscripten");
    #endif
}
void requestAnimationFrameLoopWithJSPI(void (*callback)(void), int, int){
    #ifdef __EMSCRIPTEN__
    requestAnimationFrameLoopWithJSPI_impl(callback);
    #else
    TRACELOG(LOG_WARNING, "requestAnimationFrame not supported outside of emscripten");
    #endif
}
RenderTexture headless_rtex;
void BeginDrawing(){

    ResetSyncState();
    ++g_renderstate.renderTargetStackPosition;
    
    if(g_renderstate.windowFlags & FLAG_HEADLESS){
        if(headless_rtex.texture.id){
            UnloadTexture(headless_rtex.texture);
        }
        if(headless_rtex.colorMultisample.id){
            UnloadTexture(headless_rtex.colorMultisample);
        }
        if(headless_rtex.depth.id){
            UnloadTexture(headless_rtex.depth);
        }
        

        headless_rtex = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
        g_renderstate.mainWindowRenderTarget = headless_rtex;
        //setTargetTextures(g_renderstate.rstate, headless_rtex.texture.view, headless_rtex.colorMultisample.view, headless_rtex.depth.view);

        g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition] = headless_rtex;
    }
    else{
        
        //if(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.id)
        //    UnloadTexture(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture);
        #if SUPPORT_WGPU_BACKEND == 1
        if(g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture.id)
            UnloadTexture(g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture);
        #endif

        //g_renderstate.drawmutex.lock();
        g_renderstate.renderExtentX = g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture.width;
        g_renderstate.width = g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture.width;
        g_renderstate.renderExtentY = g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture.height;
        g_renderstate.height = g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget.texture.height;
        //std::cout << g_renderstate.createdSubwindows[g_renderstate.window].surface.frameBuffer.depth.width << std::endl;
        GetNewTexture(&g_renderstate.createdSubwindows[g_renderstate.window].surface);
        g_renderstate.mainWindowRenderTarget = g_renderstate.createdSubwindows[g_renderstate.window].surface.renderTarget;
        g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition] = g_renderstate.mainWindowRenderTarget;
        //__builtin_dump_struct(&(g_renderstate.mainWindowRenderTarget.depth), printf);
    }
    BeginRenderpassEx(&g_renderstate.renderpass);
    //SetUniformBuffer(0, g_renderstate.defaultScreenMatrix);
    SetMatrix(ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY));
    SetUniformBufferData(0, GetMatrixPtr(), sizeof(Matrix));
    
    if(IsKeyPressed(KEY_F2) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) || true)){
        if(g_renderstate.grst->recording){
            EndGIFRecording();
        }
        else{
            StartGIFRecording();
        }
    }
    //EndRenderPass(&g_renderstate.clearPass);
    //BeginRenderPass(&g_renderstate.renderpass);
    //UseNoTexture();
    //updateBindGroup(g_renderstate.rstate);
}
void EndDrawing(){
    if(g_renderstate.activeRenderpass){
        drawCurrentBatch();
        EndRenderpassEx(g_renderstate.activeRenderpass);
        
    }
    if(g_renderstate.windowFlags & FLAG_STDOUT_TO_FFMPEG){
        Image img = LoadImageFromTextureEx((WGVKTexture)GetActiveColorTarget(), 0);
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
        size_t fmtsize = GetPixelSizeInBytes(img.format);
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
    if(g_renderstate.grst->recording){
        uint64_t stmp = NanoTime();
        if(stmp - g_renderstate.grst->lastFrameTimestamp > g_renderstate.grst->delayInCentiseconds * 10000000ull){
            g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.format = (PixelFormat)g_renderstate.frameBufferFormat;
            Texture fbCopy = LoadTextureEx(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.width, g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.height, (PixelFormat)g_renderstate.frameBufferFormat, false);
            BeginComputepass();
            ComputepassEndOnlyComputing();
            CopyTextureToTexture(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture, fbCopy);
            EndComputepass();
            BeginRenderpass();
            int recordingTextX = GetScreenWidth() - MeasureText("Recording", 30);
            DrawText("Recording", recordingTextX, 5, 30, Color{255,40,40,255});
            EndRenderpass();
            addScreenshot(g_renderstate.grst, (WGVKTexture)fbCopy.id);
            UnloadTexture(fbCopy);
            g_renderstate.grst->lastFrameTimestamp = stmp;
        }
        else{
            BeginRenderpass();
            int recordingTextX = GetScreenWidth() - MeasureText("Recording", 30);
            DrawText("Recording", recordingTextX, 5, 30, Color{255,40,40,255});
            EndRenderpass();
        }
        
        
    }
    
    //WGPUSurfaceTexture surfaceTexture;
    //wgpuSurfaceGetCurrentTexture(g_renderstate.surface, &surfaceTexture);
    
    
    if(!(g_renderstate.windowFlags & FLAG_HEADLESS)){
        #ifndef __EMSCRIPTEN__
        PresentSurface(&g_renderstate.mainWindow->surface);
        #endif
    }
    else{
        PostPresentSurface();
    }
    
    std::copy(g_renderstate.smallBufferRecyclingBin.begin(), g_renderstate.smallBufferRecyclingBin.end(), std::back_inserter(g_renderstate.smallBufferPool));
    g_renderstate.smallBufferRecyclingBin.clear();
    auto& ipstate = g_renderstate.input_map[g_renderstate.window];
    std::copy(ipstate.keydown.begin(), ipstate.keydown.end(), ipstate.keydownPrevious.begin());
    ipstate.mousePosPrevious = ipstate.mousePos;
    ipstate.scrollPreviousFrame = ipstate.scrollThisFrame;
    ipstate.scrollThisFrame = Vector2{0, 0};
    std::copy(ipstate.mouseButtonDown.begin(), ipstate.mouseButtonDown.end(), ipstate.mouseButtonDownPrevious.begin());
    for(auto& [_, ipstate] : g_renderstate.input_map){
        ipstate.charQueue.clear();
        ipstate.gestureAngleThisFrame = 0;
        ipstate.gestureZoomThisFrame = 1;
    }
    PollEvents();
    if(g_renderstate.wantsToggleFullscreen){
        g_renderstate.wantsToggleFullscreen = false;
        ToggleFullscreenImpl();
    }
    //if(!(g_renderstate.windowFlags & FLAG_HEADLESS))
    //    g_renderstate.drawmutex.unlock();
    uint64_t nanosecondsPerFrame = std::floor(1e9 / GetTargetFPS());
    //std::cout << nanosecondsPerFrame << "\n";
    uint64_t beginframe_stmp = g_renderstate.last_timestamps[(g_renderstate.total_frames) % 64];
    ++g_renderstate.total_frames;
    g_renderstate.last_timestamps[g_renderstate.total_frames % 64] = NanoTime();
    uint64_t elapsed = NanoTime() - beginframe_stmp;
    if(elapsed & (1ull << 63))return;
    if(!(g_renderstate.windowFlags & FLAG_VSYNC_HINT) && nanosecondsPerFrame > elapsed && GetTargetFPS() > 0)
        NanoWait(nanosecondsPerFrame - elapsed);
    
    --g_renderstate.renderTargetStackPosition;
    //std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
void StartGIFRecording(){
    startRecording(g_renderstate.grst, 4);
}
void EndGIFRecording(){
    #ifndef __EMSCRIPTEN__
    if(!g_renderstate.grst->recording)return;
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
    endRecording(g_renderstate.grst, buf);
}
void rlBegin(PrimitiveType mode){
    
    if(current_drawmode != mode){
        drawCurrentBatch();
        //assert(g_renderstate.activeRenderPass == &g_renderstate.renderpass);
        //EndRenderpassEx(&g_renderstate.renderpass);
        //BeginRenderpassEx(&g_renderstate.renderpass);
    }
    if(mode == RL_LINES){ //TODO: Fix this, why is this required? Check core_msaa and comment this out to trigger a bug
        SetTexture(1, g_renderstate.whitePixel);
    }
    current_drawmode = mode;
}
void rlEnd(){
    
}
uint32_t RoundUpToNextMultipleOf256(uint32_t x) {
    return (x + 255) & ~0xFF;
}
uint32_t RoundUpToNextMultipleOf16(uint32_t x) {
    return (x + 15) & ~0xF;
}
#ifdef __EMSCRIPTEN__

#endif 



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
    uint32_t psize = GetPixelSizeInBytes(newFormat);
    Image newimg zeroinit;
    newimg.format = newFormat;
    newimg.width = img->width;
    newimg.height = img->height;
    newimg.mipmaps = img->mipmaps;
    newimg.rowStrideInBytes = newimg.width * psize;
    newimg.data = calloc(img->width * img->height, psize);
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
    img->data = newimg.data;
    img->rowStrideInBytes = newimg.rowStrideInBytes;
    
}
Color* LoadImageColors(Image img){
    Image copy = ImageFromImage(img, Rectangle{0,0,(float)img.width, (float)img.height});
    ImageFormat(&copy, RGBA8);
    return (RGBA8Color*)copy.data;
}
void UnloadImageColors(Color* cols){
    free(cols);
}

Image LoadImageFromTexture(Texture tex){
    //#ifndef __EMSCRIPTEN__
    //auto& device = g_renderstate.device;
    return LoadImageFromTextureEx((WGVKTexture)tex.id, 0);
    //#else
    //std::cerr << "LoadImageFromTexture not supported on web\n";
    //return Image{};
    //#endif
}
void TakeScreenshot(const char* filename){
    Image img = LoadImageFromTextureEx((WGVKTexture)g_renderstate.mainWindowRenderTarget.texture.id, 0);
    SaveImage(img, filename);
    UnloadImage(img);
}


extern "C" bool IsKeyDown(int key){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydown[key];
}
extern "C" bool IsKeyPressed(int key){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydown[key] && !g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].keydownPrevious[key];
}
extern "C" int GetCharPressed(){
    int fc = 0;
    if(!g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.empty()){
        fc = g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.front();
        g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].charQueue.pop_front();
    }
    return fc;
}
int GetMouseX(cwoid){
    return (int)GetMousePosition().x;
}
int GetMouseY(cwoid){
    return (int)GetMousePosition().y;
}

float GetGesturePinchZoom(cwoid){
    return g_renderstate.input_map[g_renderstate.window].gestureZoomThisFrame;
}
float GetGesturePinchAngle(cwoid){
     
    return g_renderstate.input_map[g_renderstate.window].gestureAngleThisFrame;
}
Vector2 GetMousePosition(cwoid){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos;
}
Vector2 GetMouseDelta(cwoid){
    return Vector2{g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.x - g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.x,
                   g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.y - g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mousePos.y};
}
float GetMouseWheelMove(void){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].scrollPreviousFrame.y;
}
Vector2 GetMouseWheelMoveV(void){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].scrollPreviousFrame;
    //return Vector2{
    //    (float)(g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollX - g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollYPrevious),
    //    (float)(g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollY - g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].globalScrollYPrevious)};
}
bool IsMouseButtonPressed(int button){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button] && !g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDownPrevious[button];
}
bool IsMouseButtonDown(int button){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button];
}
bool IsMouseButtonReleased(int button){
    return !g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDown[button] && g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].mouseButtonDownPrevious[button];
}
bool IsCursorOnScreen(cwoid){
    return g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()].cursorInWindow;
}

uint64_t NanoTime(cwoid){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
double GetTime(cwoid){
    uint64_t nano_diff = NanoTime() - g_renderstate.init_timestamp;

    return double(nano_diff) * 1e-9;
}
uint32_t GetFPS(cwoid){
    auto firstzero = std::find(std::begin(g_renderstate.last_timestamps), std::end(g_renderstate.last_timestamps), int64_t(0));
    auto [minit, maxit] = std::minmax_element(std::begin(g_renderstate.last_timestamps), firstzero);
    //TRACELOG(LOG_INFO, "%d : %d", int(minit - std::begin(g_renderstate.last_timestamps)), int(maxit - std::begin(g_renderstate.last_timestamps)));
    return uint32_t(std::round((firstzero - std::begin(g_renderstate.last_timestamps) - 1.0) * 1.0e9 / (*maxit - *minit)));
}
void DrawFPS(int posX, int posY){
    char fpstext[128] = {0};
    std::snprintf(fpstext, 128, "%d FPS", GetFPS());
    double ratio = double(GetFPS()) / GetTargetFPS();
    ratio = std::max(0.0, std::min(1.0, ratio));
    uint8_t v8 = ratio * 255;
    DrawText(fpstext, posX, posY, 40, Color{uint8_t(255 - uint8_t(ratio * ratio * 255)), v8, 20, 255});
}

/*WGPUShaderModule LoadShader(const char* path) {
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
    return wgpuDeviceCreateShaderModule((WGPUDevice)GetDevice(), &shaderDesc);
}*/
extern "C" Texture3D LoadTexture3DEx(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format){
    return LoadTexture3DPro(width, height, depth, format, TextureUsage_CopyDst | TextureUsage_TextureBinding | TextureUsage_StorageBinding, 1);
}
uint32_t GetUniformLocation(DescribedPipeline* pl, const char* uniformName){
    return pl->sh.uniformLocations->GetLocation(uniformName);
}
uint32_t GetUniformLocationCompute(DescribedComputePipeline* pl, const char* uniformName){
    return pl->shaderModule.uniformLocations->GetLocation(uniformName);
}
NativeBindgroupHandle UpdateAndGetNativeBindGroup(DescribedBindGroup* bg){
    if(bg->needsUpdate){
        UpdateBindGroup(bg);
        //bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &(bg->desc));
        bg->needsUpdate = false;
    }
    return bg->bindGroup;
}
extern "C" void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va){
    auto plquart = GetPipelinesForLayout(pipeline, va->attributes);
    pipeline->quartet = plquart;
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


extern "C" Texture LoadTextureEx(uint32_t width, uint32_t height, PixelFormat format, bool to_be_used_as_rendertarget){
    return LoadTexturePro(width, height, format, (TextureUsage_RenderAttachment * to_be_used_as_rendertarget) | TextureUsage_TextureBinding | TextureUsage_CopyDst | TextureUsage_CopySrc, 1, 1);
}
Texture LoadTexture(const char* filename){
    Image img = LoadImage(filename);
    Texture tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}
Texture LoadBlankTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, RGBA8, true);
}

Texture LoadDepthTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, Depth32, true);
}
RenderTexture LoadRenderTextureEx(uint32_t width, uint32_t height, PixelFormat colorFormat, uint32_t sampleCount){
    RenderTexture ret{
        .texture = LoadTextureEx(width, height, colorFormat, true),
        .colorMultisample = Texture{}, 
        .depth = LoadTexturePro(width, height, Depth32, TextureUsage_RenderAttachment | TextureUsage_CopySrc, sampleCount, 1)
    };
    if(sampleCount > 1){
        ret.colorMultisample = LoadTexturePro(width, height, colorFormat, TextureUsage_RenderAttachment | TextureUsage_CopySrc, sampleCount, 1);
    }
    return ret;
}



DescribedRenderpass* GetActiveRenderPass(){
    return g_renderstate.activeRenderpass;
}
DescribedPipeline* GetActivePipeline(){
    return g_renderstate.activePipeline;
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
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->size;
    UpdateBindGroupEntry(bg, index, entry);
}
void SetBindgroupStorageBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.buffer = buffer->buffer;
    entry.size = buffer->size;
    UpdateBindGroupEntry(bg, index, entry);
}

extern "C" void SetBindgroupTexture3D(DescribedBindGroup* bg, uint32_t index, Texture3D tex){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}
extern "C" void SetBindgroupTextureView(DescribedBindGroup* bg, uint32_t index, WGVKTextureView texView){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.textureView = texView;
    
    UpdateBindGroupEntry(bg, index, entry);
}
extern "C" void SetBindgroupTexture(DescribedBindGroup* bg, uint32_t index, Texture tex){
    ResourceDescriptor entry{};
    entry.binding = index;
    entry.textureView = tex.view;
    
    UpdateBindGroupEntry(bg, index, entry);
}

extern "C" void SetBindgroupSampler(DescribedBindGroup* bg, uint32_t index, DescribedSampler sampler){
    ResourceDescriptor entry{};
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

inline uint64_t bgEntryHash(const ResourceDescriptor& bge){
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}

extern "C" DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const ResourceDescriptor* entries, size_t entryCount){
    DescribedBindGroup ret zeroinit;
    ret.entries = (ResourceDescriptor*)std::calloc(entryCount, sizeof(ResourceDescriptor));
    std::memcpy(ret.entries, entries, entryCount * sizeof(ResourceDescriptor));
    ret.entryCount = entryCount;
    ret.layout = bglayout;

    ret.needsUpdate = true;
    ret.descriptorHash = 0;


    for(uint32_t i = 0;i < ret.entryCount;i++){
        ret.descriptorHash ^= bgEntryHash(ret.entries[i]);
    }
    //ret.bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &ret.desc);
    return ret;
}






inline bool operator==(const RenderSettings& a, const RenderSettings& b){
    return a.depthTest == b.depthTest &&
           a.faceCull == b.faceCull &&
           a.depthCompare == b.depthCompare &&
           a.frontFace == b.frontFace;
}
/*extern "C" void UpdateRenderpass(DescribedRenderpass* rp, RenderSettings newSettings){
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
}*/
void UnloadRenderpass(DescribedRenderpass rp){
    //if(rp.rca)free(rp.rca); //Should not happen
    //rp.rca = nullptr;

    //if(rp.dsa)free(rp.dsa);
    //rp.dsa = nullptr;
}


DescribedSampler LoadSampler(addressMode amode, filterMode fmode){
    return LoadSamplerEx(amode, fmode, fmode, 1.0f);
}


NativeImageHandle GetActiveColorTarget(){
    return g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.id;
}

/*void setTargetTextures(full_renderstate* state, WGPUTextureView c, WGPUTextureView cms, WGPUTextureView d){
    DescribedRenderpass* restart = nullptr;
    if(g_renderstate.activeRenderpass){
        restart = g_renderstate.activeRenderpass;
        EndRenderpassEx(g_renderstate.activeRenderpass);
    }
    state->color = c;
    state->depth = d;
    //LoadTexture
    
    WGPUTextureDescriptor desc{};
    const bool multisample = g_renderstate.windowFlags & FLAG_MSAA_4X_HINT;
    //if(colorMultisample.id == nullptr && multisample){
    //    colorMultisample = LoadTexturePro(GetScreenWidth(), GetScreenHeight(), g_renderstate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4);
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
}*/
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
    Image ret{
        .data = std::calloc(width * height, sizeof(Color)),
        .width = width, 
        .height = height,
        .mipmaps = 1,
        .format = RGBA8, 
        .rowStrideInBytes = width * 4,
    };
    for(uint32_t i = 0;i < height;i++){
        for(uint32_t j = 0;j < width;j++){
            const size_t index = size_t(i) * width + j;
            static_cast<Color*>(ret.data)[index] = a;
        }
    }
    return ret;
}
extern "C" Image GenImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){
    Image ret{
        .data = std::calloc(width * height, sizeof(Color)),
        .width = width, 
        .height = height, 
        .mipmaps = 1,
        .format = RGBA8, 
        .rowStrideInBytes = width * 4, 
    };
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
void SaveImage(Image _img, const char* filepath){
    std::string_view fp(filepath, filepath + std::strlen(filepath));
    //std::cout << img.format << "\n";
    if(_img.format == GRAYSCALE){
        if(fp.ends_with(".png")){
            stbi_write_png(filepath, _img.width, _img.height, 2, _img.data, _img.width * sizeof(uint16_t));
            return;
        }
        else{
            TRACELOG(LOG_WARNING, "Grayscale can only export to png");
        }
    }
    Image img = ImageFromImage(_img, Rectangle{0,0,(float)_img.width, (float)_img.height});
    ImageFormat(&img, PixelFormat::RGBA8);
    //size_t stride = std::ceil(img.rowStrideInBytes);
    //if(stride == 0){
    //    stride = img.width * sizeof(Color);
    //}
    if(fp.ends_with(".png")){
        stbi_write_png(filepath, img.width, img.height, 4, img.data, img.rowStrideInBytes);
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
    UnloadImage(img);
}
void UseTexture(Texture tex){
    uint32_t texture0loc = GetUniformLocation(GetActivePipeline(), RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);
    if(texture0loc == LOCATION_NOT_FOUND){
        texture0loc = 1;
        //return;
    }
    if(g_renderstate.activePipeline->bindGroup.entries[texture0loc].textureView == tex.view)return;
    drawCurrentBatch();
    SetTexture(texture0loc, tex);
    
}
void UseNoTexture(){
    uint32_t textureLocation = GetUniformLocation(GetActivePipeline(), RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);
    if(textureLocation == LOCATION_NOT_FOUND){
        textureLocation = 1;
    }
    if(g_renderstate.activePipeline->bindGroup.entries[textureLocation].textureView == g_renderstate.whitePixel.view)return;
    drawCurrentBatch();
    SetTexture(textureLocation, g_renderstate.whitePixel);
}

void BeginTextureMode(RenderTexture rtex){
    ++g_renderstate.renderTargetStackPosition;
    g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition] = rtex;
    g_renderstate.renderExtentX = rtex.texture.width;
    g_renderstate.renderExtentY = rtex.texture.height;
    //std::cout << std::format("{} x {}\n", g_renderstate.renderExtentX, g_renderstate.renderExtentY);
    //setTargetTextures(g_renderstate.rstate, rtex.texture.view, rtex.colorMultisample.view, rtex.depth.view);
    PushMatrix();
    Matrix mat = ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY);
    SetMatrix(mat);
    SetUniformBufferData(0, &mat, sizeof(Matrix));
    BeginRenderpass();
}

void EndTextureMode(){
    drawCurrentBatch();

    EndRenderpassPro(GetActiveRenderPass(), true);

    --g_renderstate.renderTargetStackPosition;
    g_renderstate.renderExtentX = g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.width;
    g_renderstate.renderExtentY = g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.height;
    Matrix mat = ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY);
    //SetUniformBuffer(0, g_renderstate.defaultScreenMatrix);
    PopMatrix();
    SetUniformBufferData(0, GetMatrixPtr(), sizeof(Matrix));
    if(g_renderstate.renderTargetStackPosition >= 0){
        BeginRenderpass();
    }
}
extern "C" void BeginWindowMode(SubWindow sw){
    auto& swref = g_renderstate.createdSubwindows.at(sw.handle);
    g_renderstate.activeSubWindow = sw;
    GetNewTexture(&swref.surface);


    //WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture,nullptr);
    //wgpuTextureViewRelease(g_renderstate.activeSubWindow.frameBuffer.color.view);
    //wgpuTextureRelease(g_renderstate.activeSubWindow.frameBuffer.color.id);
    //sw.frameBuffer.texture.view = nextTexture;
    //sw.frameBuffer.texture.id = surfaceTexture.texture;
    BeginTextureMode(swref.surface.renderTarget);
    //++g_renderstate.renderTargetStackPosition;
    
    //g_renderstate.currentDefaultRenderTarget = sw.frameBuffer;
    //BeginRenderpass();
}


extern "C" void EndWindowMode(){
    
    { //This is an inlined EndTextureMode that passes false for EndRenderpassPro
        drawCurrentBatch();

        EndRenderpassPro(GetActiveRenderPass(), false);

        --g_renderstate.renderTargetStackPosition;
        g_renderstate.renderExtentX = g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.width;
        g_renderstate.renderExtentY = g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.height;
        Matrix mat = ScreenMatrix(g_renderstate.renderExtentX, g_renderstate.renderExtentY);
        //SetUniformBuffer(0, g_renderstate.defaultScreenMatrix);
        PopMatrix();
        SetUniformBufferData(0, GetMatrixPtr(), sizeof(Matrix));
        if(g_renderstate.renderTargetStackPosition >= 0){
            BeginRenderpass();
        }
    }
    
    //EndRenderpass();
    //EndTextureMode();
    //g_renderstate.currentDefaultRenderTarget = g_renderstate.mainWindowRenderTarget;
    PresentSurface(&g_renderstate.activeSubWindow.surface);
    auto& ipstate = g_renderstate.input_map[(GLFWwindow*)GetActiveWindowHandle()];
    std::copy(ipstate.keydown.begin(), ipstate.keydown.end(), ipstate.keydownPrevious.begin());
    ipstate.mousePosPrevious = ipstate.mousePos;
    ipstate.scrollPreviousFrame = ipstate.scrollThisFrame;
    ipstate.scrollThisFrame = Vector2{0, 0};
    std::copy(ipstate.mouseButtonDown.begin(), ipstate.mouseButtonDown.end(), ipstate.mouseButtonDownPrevious.begin());
    
    g_renderstate.activeSubWindow = SubWindow zeroinit;
    
    return;

    //Bad implementation:
    /*drawCurrentBatch();
    g_renderstate.renderExtentX = GetScreenWidth();
    g_renderstate.renderExtentY = GetScreenHeight();
    auto state = g_renderstate.rstate;
    auto& c = g_renderstate.currentScreenTextureView;
    auto& cms = colorMultisample.view; 
    auto d = depthTexture.view;
    state->color = c;
    state->depth = d;
    //LoadTexture
    
    WGPUTextureDescriptor desc{};
    const bool multisample = g_renderstate.windowFlags & FLAG_MSAA_4X_HINT;
    //if(colorMultisample.id == nullptr && multisample){
    //    colorMultisample = LoadTexturePro(GetScreenWidth(), GetScreenHeight(), g_renderstate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4);
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
    wgpuSurfacePresent(g_renderstate.activeSubWindow.surface);*/
    
}


int GetRandomValue(int min, int max){
    int w = (max - min);
    int v = rand() & w;
    return v + min;
}

extern "C" DescribedBuffer* GenVertexBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, BufferUsage_CopyDst | BufferUsage_Vertex);
}
DescribedBuffer* GenIndexBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, BufferUsage_CopyDst | BufferUsage_Index);
}
DescribedBuffer* GenUniformBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, BufferUsage_CopySrc | BufferUsage_CopyDst | BufferUsage_Uniform);
}
DescribedBuffer* GenStorageBuffer(const void* data, size_t size){
    return GenBufferEx(data, size, BufferUsage_CopySrc | BufferUsage_CopyDst | BufferUsage_Storage);
}



extern "C" void SetTargetFPS(int fps){
    g_renderstate.targetFPS = fps;
}
extern "C" int GetTargetFPS(){
    return g_renderstate.targetFPS;
}
extern "C" uint64_t GetFrameCount(){
    return g_renderstate.total_frames;
}
//TODO: this is bad
extern "C" float GetFrameTime(){
    if(g_renderstate.total_frames <= 1){
        return 0.0f;
    }
    if(g_renderstate.last_timestamps[g_renderstate.total_frames % 64] - g_renderstate.last_timestamps[(g_renderstate.total_frames - 1) % 64] < 0){
        return 1.0e-9f * (g_renderstate.last_timestamps[(g_renderstate.total_frames - 1) % 64] - g_renderstate.last_timestamps[(g_renderstate.total_frames - 2) % 64]);
    } 
    return 1.0e-9f * (g_renderstate.last_timestamps[g_renderstate.total_frames % 64] - g_renderstate.last_timestamps[(g_renderstate.total_frames - 1) % 64]);
}
extern "C" void SetConfigFlags(int /* enum WindowFlag */ flag){
    g_renderstate.windowFlags |= flag;
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
RenderSettings GetDefaultSettings(){
    RenderSettings ret zeroinit;
    ret.faceCull = 1;
    ret.frontFace = FrontFace_CCW;
    ret.depthTest = 1;
    ret.depthCompare = CompareFunction_LessEqual;
    ret.sampleCount = (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1;

    ret.blendFactorSrcAlpha = BlendFactor_One;
    ret.blendFactorDstAlpha = BlendFactor_OneMinusSrcAlpha;
    ret.blendOperationAlpha = BlendOperation_Add;

    ret.blendFactorSrcColor = BlendFactor_SrcAlpha;
    ret.blendFactorDstColor = BlendFactor_OneMinusSrcAlpha;
    ret.blendOperationColor = BlendOperation_Add;
    
    return ret;
}
FILE* tracelogFile = stdout;
int tracelogLevel = LOG_INFO;
void SetTraceLogFile(FILE* file){
    tracelogFile = file;
}
void SetTraceLogLevel(int logLevel){
    tracelogLevel = logLevel;
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
size_t telegrama_render_size1 = sizeof(telegrama_render1);
size_t telegrama_render_size2 = sizeof(telegrama_render2);
size_t telegrama_render_size3 = sizeof(telegrama_render3);
