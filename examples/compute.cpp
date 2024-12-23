#include <algorithm>
#include <raygpu.h>
#include <iostream>
#include <vector>
#include <random>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
#include <wgpustate.inc>
const char wgsl[] = R"(
struct VertexInput {
    @location(0) position: vec2f,
    @location(1) offset: vec2f
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy + in.offset.xy, 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.position.xy / 400.0f,1,1);
}
)";

const char computeSource[] = R"(
@group(0) @binding(0) var<storage,read_write> posBuffer: array<vec2<f32>>;
@group(0) @binding(1) var<storage,read> velBuffer: array<vec2<f32>>;
//@group(0) @binding(2) var<storage,read_write> outputPosBuffer: array<vec2<f32>>;
@compute
@workgroup_size(64, 1, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    //outputPosBuffer[id] = posBuffer[id] + velBuffer[id];
    //outputPosBuffer[id.x * 16 + id.y] = posBuffer[id.x * 16 + id.y] + velBuffer[id.x * 16 + id.y];
    posBuffer[id.x] = posBuffer[id.x] + velBuffer[id.x];
})";
DescribedPipeline *rpl;
DescribedComputePipeline* cpl;
VertexArray* vao;
DescribedBuffer quad;
DescribedBuffer positions;
DescribedBuffer velocities;
DescribedBuffer positionsnew;
DescribedBuffer ibo;
constexpr size_t parts = (1 << 20);
void mainloop(void){

    BeginComputepass();
    BindComputePipeline(cpl);
    DispatchCompute(parts / 64 / 1, 1, 1);
    ComputepassEndOnlyComputing();
    //CopyBufferToBuffer(&positionsnew, &positions, positions.descriptor.size);
    EndComputepass();
    BeginDrawing();
    ClearBackground(BLACK);
    BeginPipelineMode(rpl, WGPUPrimitiveTopology_TriangleList);
    BindVertexArray(rpl, vao);
    //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, buf2.buffer, 0, 256);
    DrawArraysIndexedInstanced(ibo, 6, parts);
    EndPipelineMode();
    DrawFPS(10, 10);
    TakeScreenshot("out.png");
    EndDrawing();
}
int main(){
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1600, 900, "Compute Shader");
    SetTargetFPS(100000);
    UniformDescriptor computeUniforms[] = {
        UniformDescriptor{.type = storage_write_buffer, .minBindingSize = 8},
        UniformDescriptor{.type = storage_buffer, .minBindingSize = 8},
        UniformDescriptor{.type = storage_write_buffer, .minBindingSize = 8}
    };
    cpl = LoadComputePipeline(computeSource, computeUniforms, 2);
    
    std::mt19937_64 gen(53);
    std::uniform_real_distribution<float> dis(-1,1);
    std::vector<Vector2> pos(parts), vel(parts);
    //std::generate(pos.begin(), pos.end(), [&]{
    //    return Vector2{dis(gen)*0.01f, dis(gen) * 0.01f};
    //});
    for(size_t i = 0;i < parts;i++){
        float arg = M_PI * (dis(gen) + 1);
        float mag = std::sqrt(dis(gen)) * 0.001f;

        vel[i] = Vector2{std::cos(arg) * mag, std::sin(arg) * mag};
    }
    float quadpos[8] = {0,0,1,0,1,1,0,1};
    for(int i = 0;i < 8;i++){
        quadpos[i] *= 0.005f;
    }
    quad = GenBuffer(quadpos, sizeof(quadpos));
    uint32_t indices[] = {0,1,2,0,2,3};
    ibo = GenIndexBuffer(indices, sizeof(indices));
    positions = GenBufferEx(pos.data(), pos.size() * sizeof(Vector2), WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage | WGPUBufferUsage_CopyDst);
    velocities = GenBufferEx(vel.data(), vel.size() * sizeof(Vector2), WGPUBufferUsage_Vertex | WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst);
    positionsnew = GenBufferEx(nullptr, vel.size() * sizeof(Vector2), WGPUBufferUsage_Storage | WGPUBufferUsage_CopySrc);
    
    WGPUBindGroupEntry bge[2] = {
        WGPUBindGroupEntry{},
        WGPUBindGroupEntry{}
    };
    bge[0].binding = 0;
    bge[0].buffer  = positions.buffer;
    bge[0].size    = positions.descriptor.size;

    bge[1].binding = 1;
    bge[1].buffer  = velocities.buffer;
    bge[1].size    = velocities.descriptor.size;

    cpl->bindGroup = LoadBindGroup(&cpl->bglayout, bge, 2);

    vao = LoadVertexArray();
    VertexAttribPointer(vao, &quad, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &positions, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Instance);
    RenderSettings settings{};
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_Always;
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
