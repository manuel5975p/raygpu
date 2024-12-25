#include <raygpu.h>
const char shaderSource[] = R"(
struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
    @location(2) normal: vec3<f32>,
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
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
@group(0) @binding(2) var grsampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
@group(0) @binding(4) var<storage> lights: LightBuffer;

//Can be omitted
//@group(0) @binding(3) var<storage> storig: array<vec4f>;


@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * 
                   modelMatrix[instanceIdx] *
    vec4f(in.position.xyz /*+ storig[0].xyz * 0.3f*/, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return textureSample(gradientTexture, grsampler, in.uv).rgba * in.color;
})";
int main(){
    InitWindow(800, 600, "Shader Loading");
    AttributeAndResidence attrs[4] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 0 * sizeof(float), 0}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 3 * sizeof(float), 1}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 5 * sizeof(float), 2}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x4, 8 * sizeof(float), 3}, 0, WGPUVertexStepMode_Vertex, true},
    };
    RenderSettings settings zeroinit;
    settings.depthTest = 1;
    DescribedPipeline* pl = LoadPipeline(shaderSource, attrs, 4, settings);
    while(!WindowShouldClose()){
        BeginDrawing();
        DrawFPS(0, 0);
        EndDrawing();
    }
}
