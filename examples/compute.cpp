#include <raygpu.h>
#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <wgpustate.inc>
const char wgsl[] = R"(
struct VertexInput {
    @location(0) position: vec2f
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy, 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.position.xy / 400.0f,1,1);
}
)";
DescribedPipeline *rpl;
DescribedComputePipeline* cpl;
VertexArray* vao;
DescribedBindGroup bg;
DescribedBuffer buf1;
DescribedBuffer buf2;
void mainloop(void){
    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    WGPUComputePassEncoder cpenc = wgpuCommandEncoderBeginComputePass(enc, nullptr);

    wgpuComputePassEncoderSetPipeline(cpenc, cpl->pipeline);
    wgpuComputePassEncoderSetBindGroup(cpenc, 0, bg.bindGroup, 0, nullptr);
    wgpuComputePassEncoderDispatchWorkgroups(cpenc, 8, 8, 1);
    wgpuComputePassEncoderEnd(cpenc);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    wgpuComputePassEncoderRelease(cpenc);
    wgpuCommandBufferRelease(buf);
    wgpuCommandEncoderRelease(enc);
    BeginDrawing();
    ClearBackground(BLACK);
    BeginPipelineMode(rpl, WGPUPrimitiveTopology_TriangleList);
    BindVertexArray(rpl, vao);
    //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, buf2.buffer, 0, 256);
    DrawArrays(3);
    EndPipelineMode();
    DrawFPS(10, 10);
    EndDrawing();
}
int main(){
    InitWindow(1280, 720, "Compute Shader");
    UniformDescriptor computeUniforms[] = {
        UniformDescriptor{.type = storage_buffer, .minBindingSize = 256},
        UniformDescriptor{.type = storage_write_buffer, .minBindingSize = 256}
    };
    cpl = LoadComputePipeline(R"(
@group(0) @binding(0) var<storage,read> inputBuffer: array<f32,64>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32,64>;
@compute
@workgroup_size(8, 8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    // Apply the function f to the buffer element at index id.x:
    outputBuffer[id.x] = 3.0f * (inputBuffer[id.x]);
}
    )", computeUniforms, 2);

    float data[64] = {0.1f, 0.1f, 0.3f, 0.1f, 0.3f, 0.3f};

    buf1 = GenBufferEx(data, 256, WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst);
    buf2 = GenBufferEx(data, 256, WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst);
    WGPUBindGroupEntry bge[2] = {
        WGPUBindGroupEntry{},
        WGPUBindGroupEntry{}
    };
    bge[0].binding = 0;
    bge[0].buffer = buf1.buffer;
    bge[0].size = buf1.descriptor.size;

    bge[1].binding = 1;
    bge[1].buffer = buf2.buffer;
    bge[1].size = buf2.descriptor.size;
    WGPUBindGroupDescriptor bgd{};
    bgd.entries = bge;
    bgd.entryCount = 2;
    bgd.layout = cpl->bglayout.layout;
    bg = LoadBindGroup(&cpl->bglayout, bge, 2);
    

    WGPUCommandEncoder enc = wgpuDeviceCreateCommandEncoder(GetDevice(), nullptr);
    WGPUComputePassEncoder cpenc = wgpuCommandEncoderBeginComputePass(enc, nullptr);
    wgpuComputePassEncoderSetPipeline(cpenc, cpl->pipeline);
    wgpuComputePassEncoderSetBindGroup(cpenc, 0, GetWGPUBindGroup(&bg), 0, nullptr);
    wgpuComputePassEncoderDispatchWorkgroups(cpenc, 8, 8, 1);
    wgpuComputePassEncoderEnd(cpenc);
    WGPUCommandBuffer buf = wgpuCommandEncoderFinish(enc, nullptr);
    wgpuQueueSubmit(GetQueue(), 1, &buf);
    wgpuComputePassEncoderRelease(cpenc);
    wgpuCommandBufferRelease(buf);
    wgpuCommandEncoderRelease(enc);
    vao = LoadVertexArray();
    VertexAttribPointer(vao, &buf2, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    RenderSettings settings{};
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    rpl = LoadPipelineForVAO(wgsl, vao, 0, 0, settings);



    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
    return 0;
}
