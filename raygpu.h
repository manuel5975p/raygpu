#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include "mathutils.h"
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
    uniform_type uniform_types[8];
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

extern "C"{
    WGPUDevice GetDevice();
    WGPUQueue GetQueue();
}
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
        commandEncoderDesc.label = "Command Encoder";
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
        wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bg, 0, 0);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, this->vbo, 0, wgpuBufferGetSize(vbo));
        c(renderPass);
        wgpuRenderPassEncoderEnd(renderPass);
        WGPUCommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        wgpuQueueSubmit(GetQueue(), 1, &command);
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(command);
    }
    #endif
}full_renderstate;

enum draw_mode{
    RL_TRIANGLES, RL_TRIANGLE_STRIP, RL_QUADS
};

extern "C"{
    GLFWwindow* InitWindow(uint32_t width, uint32_t height);
    void BeginDrawing();
    void EndDrawing();
    Texture LoadTextureFromImage(Image img);
    Image LoadImageFromTexture(Texture tex);

    
    void UseTexture(Texture tex);
    void UseNoTexture();
    void rlColor4f(float r, float g, float b, float alpha);
    void rlTexCoord2f(float u, float v);
    void rlVertex2f(float x, float y);
    void rlVertex3f(float x, float y, float z);
    void rlBegin(draw_mode mode);
    void rlEnd();

    void BeginTextureMode(RenderTexture rtex);
    void EndTextureMode();

    WGPUShaderModule LoadShaderFromMemory(const char* shaderSource);
    WGPUShaderModule LoadShader(const char* path);
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
    void updatePipeline        (full_renderstate* state, draw_mode drawmode);
    void updateRenderPassDesc  (full_renderstate* state);
    void setTargetTextures     (full_renderstate* state, WGPUTextureView c, WGPUTextureView d);

    void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);

    Image LoadImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount);
}
struct wgpustate;
extern wgpustate g_wgpustate;

