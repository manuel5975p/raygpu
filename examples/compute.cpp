#include <algorithm>
#include <raygpu.h>
#include <vector>
#include <random>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
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

@compute
@workgroup_size(64, 1, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    //outputPosBuffer[id] = posBuffer[id] + velBuffer[id];
    posBuffer[id.x * 16 + id.y] = posBuffer[id.x * 16 + id.y] + velBuffer[id.x * 16 + id.y];
    //posBuffer[id.x] = posBuffer[id.x] + velBuffer[id.x];
})";
DescribedPipeline *rpl;
DescribedComputePipeline* firstPassPipeline;
VertexArray* vao;
DescribedBuffer* quad;
DescribedBuffer* positions;
DescribedBuffer* velocities;
DescribedBuffer* positionsnew;

constexpr bool headless = false;

constexpr size_t parts = (1 << 23);
void mainloop(void){
    BeginDrawing();
    BeginComputepass();
    BindComputePipeline(firstPassPipeline);
    DispatchCompute(parts / 64 / 16, 16, 1);
    EndComputepass();
    ClearBackground(BLACK);

    BeginPipelineMode(rpl);
    BindPipelineVertexArray(rpl, vao);
    DrawArraysInstanced(WGPUPrimitiveTopology_TriangleStrip, 4, parts);
    EndPipelineMode();

    DrawFPS(10, 10);
    EndDrawing();
    if(false && headless){
        char b[64] = {0};

        snprintf(b, 64, "frame%04d.bmp", (int)GetFrameCount());
        TRACELOG(LOG_INFO, "Saving screenshot %s", b);
        TakeScreenshot(b);
    }
}
int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetConfigFlags(FLAG_STDOUT_TO_FFMPEG);
    //if(headless)
    //    SetConfigFlags(FLAG_HEADLESS);
    //RequestLimit(maxBufferSize, 1ull << 30);
    InitWindow(2560, 1440, "Compute Shader");
    SetTargetFPS(0);
    
    
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
    //Vertex Coordinates for a square
    float quadpos[8] = {
        0,0,
        1,0,
        0,1,
        1,1
    };
    for(int i = 0;i < 8;i++){
        quadpos[i] *= 0.003f;
    }
    quad = GenBufferEx(quadpos, sizeof(quadpos), BufferUsage_Vertex | BufferUsage_CopyDst);

    // GenBufferEx is required for Vertex and Storage Buffer Usage
    positions = GenBufferEx(pos.data(), pos.size() * sizeof(Vector2),  BufferUsage_Vertex | BufferUsage_Storage | BufferUsage_CopyDst);
    velocities = GenBufferEx(vel.data(), vel.size() * sizeof(Vector2), BufferUsage_Vertex | BufferUsage_Storage | BufferUsage_CopySrc | BufferUsage_CopyDst);

    firstPassPipeline = LoadComputePipeline(computeSource);
    (*firstPassPipeline)["posBuffer"] = positions;
    (*firstPassPipeline)["velBuffer"] = velocities;

    //SetBindgroupStorageBuffer(&cpl->bindGroup, GetUniformLocationCompute(cpl, "posBuffer"), positions);
    //SetBindgroupStorageBuffer(&cpl->bindGroup, GetUniformLocationCompute(cpl, "velBuffer"), velocities);

    vao = LoadVertexArray();
    VertexAttribPointer(vao, quad, 0, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, positions, 1, VertexFormat_Float32x2, 0, VertexStepMode_Instance);
    rpl = LoadPipeline(wgsl);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
    return 0;
}
