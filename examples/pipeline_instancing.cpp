#include <raygpu.h>
#include <vector>
#include <random>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

const char source[] = 
R"(struct VertexInput {
    @location(0) position: vec2<f32>,
    @location(1) offset: vec2f,
    @location(2) color: u32
};
struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f
};
//@group(0) @binding(0) var<uniform> Perspective_View: mat4x4<f32>;
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy + in.offset.xy, 0.0f, 1.0f);
    out.color = vec4f(f32((in.color >> 24) & 0xff) / 255.0f, f32((in.color >> 16) & 0xff) / 255.0f, f32((in.color >> 8) & 0xff) / 255.0f, f32((in.color) & 0xff) / 255.0f);
    return out;
}
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return in.color;
})";
float size = 1.0f / 32;
float qsize = size * 0.8f;
float positions[8] = {
    0,0,
    qsize,0,
    qsize,qsize,
    0,qsize
};
std::vector<Vector2> offsets;
std::vector<Vector2> velocities;
std::vector<uint32_t> colors;
std::mt19937_64 gen(42);
DescribedPipeline* pl;
DescribedBuffer* ibo;
DescribedBuffer* posb;
DescribedBuffer* poso;
DescribedBuffer* posc;
VertexArray*     vao;
void mainloop(){
    for(size_t i = 0;i < offsets.size();i++){
        offsets[i] += velocities[i] * 0.1f * std::min(GetFrameTime(), 0.01f);
    }
    BufferData(poso, offsets.data(), offsets.size() * sizeof(Vector2));
    BeginDrawing();
    //ClearBackground(BLUE);
    BeginPipelineMode(pl);
    BindPipelineVertexArray(pl, vao);
    DrawArraysIndexedInstanced(RL_TRIANGLES, *ibo, 6, offsets.size());
    EndPipelineMode();
    DrawFPS(0, 0);
    EndDrawing();
}
int main(){
    //SetConfigFlags(FLAG_VSYNC_LOWLATENCY_HINT);
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920, 1080, "VAO");
    std::mt19937_64 gen(42);
    std::uniform_real_distribution<float> dis(0,1);
    for(float i = -1;i <= 1;i += size){
        for(float j = -1;j <= 1;j += size){
            offsets.push_back(Vector2{i, j});
            velocities.push_back(Vector2{dis(gen) - 0.5f, dis(gen) - 0.5f});
            colors.push_back((uint32_t)gen());
        }
    }
    posb = GenVertexBuffer(positions, sizeof(positions));
    poso = GenVertexBuffer(offsets.data(), offsets.size() * sizeof(Vector2));
    posc = GenVertexBuffer(colors.data(), colors.size() * sizeof(uint32_t));
    vao = LoadVertexArray();
    
    VertexAttribPointer(vao, posb, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, poso, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Instance);
    VertexAttribPointer(vao, posc, 2, WGPUVertexFormat_Uint32, 0, WGPUVertexStepMode_Instance);

    
    uint32_t trifanIndices[6] = {0,1,2,0,2,3};
    ibo = GenIndexBuffer(trifanIndices, sizeof(trifanIndices));
    pl = LoadPipeline(source);
    
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
