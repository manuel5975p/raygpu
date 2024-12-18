#include <raygpu.h>
#include <vector>
#include <random>
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
constexpr bool msaa = true;
int main(){
    if(msaa)
        SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1920 * 2, 1080 * 2, "VAO");
    //SetTargetFPS(100000);

    float size = 1.0f / 1024;
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
    for(float i = -1;i <= 1;i += size){
        for(float j = -1;j <= 1;j += size){
            offsets.push_back(Vector2{i, j});
            velocities.push_back(Vector2{(float)drand48() - 0.5f, (float)drand48() - 0.5f});
            colors.push_back((uint32_t)gen());
        }
    }
    DescribedBuffer posb = GenBuffer(positions, sizeof(positions));
    DescribedBuffer poso = GenBuffer(offsets.data(), offsets.size() * sizeof(Vector2));
    DescribedBuffer posc = GenBuffer(colors.data(), colors.size() * sizeof(uint32_t));
    VertexArray* vao = LoadVertexArray();

    VertexAttribPointer(vao, &posb, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &poso, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Instance);
    VertexAttribPointer(vao, &posc, 2, WGPUVertexFormat_Uint32, 0, WGPUVertexStepMode_Instance);

    RenderSettings settings zeroinit;
    settings.depthTest = 1;
    settings.sampleCount_onlyApplicableIfMoreThanOne = msaa ? 4 : 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    uint32_t trifanIndices[6] = {0,1,2,0,2,3};
    DescribedBuffer ibo = GenIndexBuffer(trifanIndices, sizeof(trifanIndices));
    DescribedPipeline* pl = LoadPipelineForVAO(source, vao, nullptr, 0, settings);
    while(!WindowShouldClose()){
        for(size_t i = 0;i < offsets.size();i++){
            offsets[i] += velocities[i] * 0.0001f;
        }
        BufferData(&poso, offsets.data(), offsets.size() * sizeof(Vector2));
        BeginDrawing();
        ClearBackground(BLANK);
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        BindVertexArray(pl, vao);
        DrawArraysIndexedInstanced(ibo, 6, offsets.size());
        EndPipelineMode();
        DrawFPS(0, 0);
        EndDrawing();
    }
}