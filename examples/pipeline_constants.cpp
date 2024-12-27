#include <raygpu.h>
DescribedPipeline* pl;
constexpr char shaderSource[] = R"(
const red = 0.5f;
override green = 0.5f;
const blue = 0.5f;
struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
    @location(2) normal: vec3f,
    @location(3) color: vec4f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
    @location(1) color: vec4f,
};

struct LightBuffer {
    count: u32,
    positions: array<vec3f>
};

@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var grsampler: sampler;

@vertex fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * 
    vec4f(in.position.xyz, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(red, green, blue, 1.0f);
    //return textureSample(colDiffuse, grsampler, in.uv).rgba * in.color;
}
)";
void mainloop(void){
    
}
int main(void){
    InitWindow(1200, 800, "Pipeline Constants");
    AttributeAndResidence attrs[4] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 0 * sizeof(float), 0}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 3 * sizeof(float), 1}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 5 * sizeof(float), 2}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x4, 8 * sizeof(float), 3}, 0, WGPUVertexStepMode_Vertex, true},
    };
    RenderSettings settings zeroinit;
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    pl = LoadPipeline(shaderSource, attrs, 4, settings);
    DescribedSampler smp = LoadSampler(repeat, nearest);
    SetPipelineTexture(pl, 1, GetDefaultTexture());
    SetPipelineSampler(pl, 2, smp);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKBLUE);
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        DrawRectangle(100, 100, 100, 100, WHITE);
        //DrawRectangle(100, 100, 100, 100, WHITE);
        EndPipelineMode();
        EndDrawing();
    }  
}
