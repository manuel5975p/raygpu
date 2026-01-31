#include <raygpu.h>
const char shaderSource1[] = R"(
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


@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View *
                   //modelMatrix[instanceIdx] *
    vec4f(in.position.xyz, 1.0f);
    out.color = in.color;
    out.uv.x = pow(in.uv.x, in.uv.y);
    out.uv.y = pow(in.uv.y, in.uv.x);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return /*textureSample(gradientTexture, grsampler, in.uv).rgba */ in.color;
})";
int main(){
    InitWindow(800, 600, "Shader Loading");
    vertex vaodata[3] = {
        vertex{.pos = Vector3{0,0,0}, .uv = Vector2{0,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{100,0,0}, .uv = Vector2{1,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{0,100,0}, .uv = Vector2{0,1}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
    };
    DescribedBuffer* buf = GenVertexBuffer(vaodata, sizeof(vaodata));
    VertexArray* vao = LoadVertexArray();
    VertexAttribPointer(vao, buf, 0, RGVertexFormat_Float32x3, sizeof(float) * 0, RGVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 1, RGVertexFormat_Float32x2, sizeof(float) * 3, RGVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 2, RGVertexFormat_Float32x3, sizeof(float) * 5, RGVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 3, RGVertexFormat_Float32x4, sizeof(float) * 8, RGVertexStepMode_Vertex);

    Shader pl = LoadPipeline(shaderSource1);
    PrepareShader(pl, vao);
    DescribedSampler smp = LoadSampler(TEXTURE_WRAP_REPEAT, TEXTURE_FILTER_POINT);
    Texture tex = LoadTextureFromImage(GenImageColor(10, 10, RED));
    Matrix scr = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    SetShaderUniformBufferData(pl, GetUniformLocation(pl, "Perspective_View"), &scr, sizeof(Matrix));
    SetShaderTexture          (pl, GetUniformLocation(pl, "colDiffuse"), tex);
    SetShaderSampler          (pl, GetUniformLocation(pl, "texSampler"), smp);

    while(!WindowShouldClose()){
        BeginDrawing();
        DrawFPS(0, 0);
        BeginShaderMode(pl);
        BindShaderVertexArray(pl, vao);
        DrawArrays(RL_TRIANGLES, 3);
        EndShaderMode();
        EndDrawing();
    }
}
