#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <stdbool.h>
#include "mathutils.h"
#include "pipeline.h"
#define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define cwoid
#include <cstdlib>
#else
#include <stdlib.h>
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define cwoid void
#endif
struct vertex{
    Vector3 pos;
    Vector2 uv ;
    Vector4 col;
};
typedef struct Color{
    uint8_t r, g, b, a;
} Color;
typedef struct BGRAColor{
    uint8_t b, g, r, a;
} BGRAColor;

typedef struct Image{
    WGPUTextureFormat format;
    uint32_t width, height;
    void* data;
}Image;

enum uniform_type{
    uniform_buffer, storage_buffer, texture2d, sampler
};

typedef struct ShaderInputs{
    uint32_t per_vertex_count;
    uint32_t per_vertex_sizes[8]; //In bytes

    uint32_t per_instance_count;
    uint32_t per_instance_sizes[8]; //In bytes

    uint32_t uniform_count; //Also includes storage
    enum uniform_type uniform_types[8];
    uint32_t uniform_minsizes[8]; //In bytes
}ShaderInputs;

typedef struct Texture{
    uint32_t width, height;
    WGPUTextureFormat format;
    WGPUTexture tex;
    WGPUTextureView view;
}Texture;

typedef struct Rectangle {
    float x, y, width, height;
} Rectangle;
typedef struct RenderTexture{
    Texture color;
    Texture depth;
}RenderTexture;

EXTERN_C_BEGIN
    WGPUDevice GetDevice(cwoid);
    WGPUQueue GetQueue(cwoid);
EXTERN_C_END
typedef struct full_renderstate{
    WGPUShaderModule shader;
    WGPUTextureView color;
    WGPUTextureView depth;

    WGPURenderPipeline pipeline;
    WGPURenderPassDescriptor renderPassDesc;
    WGPURenderPassColorAttachment rca;
    WGPURenderPassDepthStencilAttachment dsa;


    WGPUBindGroupLayoutDescriptor bglayoutdesc;
    WGPUBindGroupLayout bglayout;
    WGPURenderPipelineDescriptor pipelineDesc;
    
    WGPUVertexBufferLayout vlayout;
    WGPUBuffer vbo;

    WGPUVertexAttribute attribs[8]; // Size: vlayout.attributeCount

    WGPUBindGroup bg;
    WGPUBindGroupEntry bgEntries[8]; // Size: bglayoutdesc.entryCount

    #ifdef __cplusplus
    template<typename callable>
    void executeRenderpass(callable&& c){
        WGPUCommandEncoderDescriptor commandEncoderDesc = {};
        commandEncoderDesc.label = STRVIEW("Command Encoder");
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bg, 0, 0);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, this->vbo, 0, wgpuBufferGetSize(vbo));
        c(renderPass);
        wgpuRenderPassEncoderEnd(renderPass);
        WGPUCommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = STRVIEW("Command buffer");
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        wgpuQueueSubmit(GetQueue(), 1, &command);
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(command);
    }
    template<typename callable>
    void executeRenderpassPlain(callable&& c){
        WGPUCommandEncoderDescriptor commandEncoderDesc = {};
        commandEncoderDesc.label = STRVIEW("Command Encoder");
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        c(renderPass);
        wgpuRenderPassEncoderEnd(renderPass);
        WGPUCommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = STRVIEW("Command buffer");
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        wgpuQueueSubmit(GetQueue(), 1, &command);
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(command);
    }
    #endif
}full_renderstate;

typedef struct Shader{
    WGPUShaderModule shadermodule;

    WGPUBindGroupLayoutDescriptor bglayoutdesc;
    WGPUBindGroupLayout bglayout;
    
} Shader;

enum draw_mode{
    RL_TRIANGLES, RL_TRIANGLE_STRIP, RL_QUADS
};
typedef struct UniformDescriptor{
    enum uniform_type type;
    uint32_t minBindingSize;
}UniformDescriptor;

typedef struct AttributeAndResidence{
    WGPUVertexAttribute attr;
    uint32_t bufferSlot;
    WGPUVertexStepMode stepMode;
}AttributeAndResidence;


EXTERN_C_BEGIN
    GLFWwindow* InitWindow(uint32_t width, uint32_t height);
    uint32_t GetScreenWidth (cwoid);
    uint32_t GetScreenHeight(cwoid);


    /**
     * @brief Get the time elapsed since InitWindow() in seconds since 
     * 
     * @return double
     */
    double GetTime(cwoid);
    /**
     * @brief Return the unix timestamp in nanosecond precision
     *
     * (implemented with high_resolution_clock::now().time_since_epoch())
     * @return uint64_t 
     */
    uint64_t NanoTime(cwoid);

    uint32_t GetFPS(cwoid);
    void BeginDrawing(cwoid);
    void EndDrawing(cwoid);
    Texture LoadTextureFromImage(Image img);
    Image LoadImageFromTexture(Texture tex);

    
    void UseTexture(Texture tex);
    void UseNoTexture(cwoid);
    void rlColor4f(float r, float g, float b, float alpha);
    void rlTexCoord2f(float u, float v);
    void rlVertex2f(float x, float y);
    void rlVertex3f(float x, float y, float z);
    void rlBegin(enum draw_mode mode);
    void rlEnd(cwoid);

    void BeginTextureMode(RenderTexture rtex);
    void EndTextureMode(cwoid);

    WGPUShaderModule LoadShaderFromMemory(const char* shaderSource);
    WGPUShaderModule LoadShader(const char* path);

    Pipeline LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, WGPUBindGroupLayout* layout);

    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
    Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget);
    Texture LoadTexture(uint32_t width, uint32_t height);
    Texture LoadDepthTexture(uint32_t width, uint32_t height);
    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);

    void SetTexture       (uint32_t index, Texture tex);
    void SetSampler       (uint32_t index, WGPUSampler sampler);
    void SetUniformBuffer (uint32_t index, const void* data, size_t size);
    void SetStorageBuffer (uint32_t index, const void* data, size_t size);
    
    void init_full_renderstate (full_renderstate* state, const WGPUShaderModule sh, const ShaderInputs shader_inputs, WGPUTextureView c, WGPUTextureView d);
    void updateVertexBuffer    (full_renderstate* state, const void* data, size_t size);
    void setStateTexture       (full_renderstate* state, uint32_t index, Texture tex);
    void setStateSampler       (full_renderstate* state, uint32_t index, WGPUSampler sampler);
    void setStateUniformBuffer (full_renderstate* state, uint32_t index, const void* data, size_t size);
    void setStateStorageBuffer (full_renderstate* state, uint32_t index, const void* data, size_t size);
    void updateBindGroup       (full_renderstate* state);
    void updatePipeline        (full_renderstate* state, enum draw_mode drawmode);
    void updateRenderPassDesc  (full_renderstate* state);
    void setTargetTextures     (full_renderstate* state, WGPUTextureView c, WGPUTextureView d);

    void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);

    Image LoadImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount);

    inline uint32_t attributeSize(WGPUVertexFormat fmt){
        switch(fmt){
            case WGPUVertexFormat_Float32:
            case WGPUVertexFormat_Uint32:
            case WGPUVertexFormat_Sint32:
            return 4;
            case WGPUVertexFormat_Float32x2:
            case WGPUVertexFormat_Uint32x2:
            case WGPUVertexFormat_Sint32x2:
            return 8;
            case WGPUVertexFormat_Float32x3:
            case WGPUVertexFormat_Uint32x3:
            case WGPUVertexFormat_Sint32x3:
            return 12;
            case WGPUVertexFormat_Float32x4:
            case WGPUVertexFormat_Uint32x4:
            case WGPUVertexFormat_Sint32x4:
            return 16;
            
            default:break;
        }
        abort();
        return 0;
    }
EXTERN_C_END
typedef struct wgpustate wgpustate;
extern wgpustate g_wgpustate;

