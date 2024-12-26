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
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
//Can be omitted
//@group(0) @binding(3) var<storage> storig: array<vec4f>;


@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * 
                   //modelMatrix[instanceIdx] *
    vec4f(in.position.xyz, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return /*textureSample(gradientTexture, grsampler, in.uv).rgba */ in.color;
})";
int main(){
    InitWindow(800, 600, "Shader Loading");
    
    RenderSettings settings zeroinit;
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    vertex vaodata[3] = {
        vertex{.pos = Vector3{0,0,0}, .uv = Vector2{0,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{100,0,0}, .uv = Vector2{1,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{0,100,0}, .uv = Vector2{0,1}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
    };
    DescribedBuffer buf = GenBuffer(vaodata, sizeof(vaodata));
    VertexArray* vao = LoadVertexArray();
    VertexAttribPointer(vao, &buf, 0, WGPUVertexFormat_Float32x3, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &buf, 1, WGPUVertexFormat_Float32x2, sizeof(float) * 2, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &buf, 2, WGPUVertexFormat_Float32x3, sizeof(float) * 5, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &buf, 3, WGPUVertexFormat_Float32x4, sizeof(float) * 8, WGPUVertexStepMode_Vertex);

    DescribedPipeline* pl = LoadPipelineForVAO(shaderSource, vao, settings);
    DescribedSampler smp = LoadSampler(repeat, nearest);
    Texture tex = LoadTextureFromImage(GenImageColor(RED, 10, 10));
    Matrix scr = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    SetPipelineUniformBufferData(pl, GetUniformLocation(pl, "Perspective_View"), &scr, sizeof(Matrix));
    SetPipelineTexture          (pl, GetUniformLocation(pl, "colDiffuse"), tex);
    SetPipelineSampler          (pl, GetUniformLocation(pl, "texSampler"), smp.sampler);
    
    while(!WindowShouldClose()){
        
        BeginDrawing();
        DrawFPS(0, 0);
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        
        
        BindVertexArray(pl, vao);
        DrawArrays(3);
        EndPipelineMode();
        EndDrawing();
    }
}
