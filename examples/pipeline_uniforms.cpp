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
//Can be omitted
//@group(0) @binding(3) var<storage> storig: array<vec4f>;


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
constexpr char shaderSource2[] = R"(
// Vertex shader in WGSL
const PI = 3.1415926;
struct VertexInput {
    @location(0) vertex_position: vec3<f32>,
    @location(1) vertex_texcoord: vec2<f32>,
    @location(2) vertex_normal: vec3<f32>,
    @location(3) vertex_tangent: vec3<f32>,
    @location(4) vertex_color: vec4<f32>,
};

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) frag_position: vec3<f32>,
    @location(1) frag_texcoord: vec2<f32>,
    @location(2) frag_color: vec4<f32>,
    @location(3) frag_normal: vec3<f32>,
    @location(4) tbn0: vec3<f32>, // First column of the TBN matrix
    @location(5) tbn1: vec3<f32>, // Second column of the TBN matrix
    @location(6) tbn2: vec3<f32>, // Third column of the TBN matrix
};

@group(0) @binding(0) var<uniform> mvp: mat4x4<f32>;
@group(0) @binding(1) var<uniform> mat_model: mat4x4<f32>;
fn inverse(m: mat3x3<f32>) -> mat3x3<f32> {
    let det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
              m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
              m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

    let inv_det = 1.0 / det;

    let adj = mat3x3<f32>(
        vec3<f32>(m[1][1] * m[2][2] - m[1][2] * m[2][1],
                  m[0][2] * m[2][1] - m[0][1] * m[2][2],
                  m[0][1] * m[1][2] - m[0][2] * m[1][1]),
        vec3<f32>(m[1][2] * m[2][0] - m[1][0] * m[2][2],
                  m[0][0] * m[2][2] - m[0][2] * m[2][0],
                  m[0][2] * m[1][0] - m[0][0] * m[1][2]),
        vec3<f32>(m[1][0] * m[2][1] - m[1][1] * m[2][0],
                  m[0][1] * m[2][0] - m[0][0] * m[2][1],
                  m[0][0] * m[1][1] - m[0][1] * m[1][0])
    );

    return adj * inv_det;
}
@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;

    let vertex_binormal = normalize(cross(input.vertex_normal, input.vertex_tangent));
    let normal_matrix = transpose(inverse(mat3x3<f32>(mat_model[0].xyz, mat_model[1].xyz, mat_model[2].xyz)));

    output.frag_position = (mat_model * vec4<f32>(input.vertex_position, 1.0)).xyz;
    output.frag_texcoord = input.vertex_texcoord * 2.0;
    output.frag_color = input.vertex_color;
    output.frag_normal = normalize(normal_matrix * input.vertex_normal);

    let frag_tangent = normalize(normal_matrix * input.vertex_tangent);
    let frag_tangent_adjusted = normalize(frag_tangent - dot(frag_tangent, output.frag_normal) * output.frag_normal);
    let frag_binormal = cross(output.frag_normal, frag_tangent_adjusted);
    let tbn = transpose(mat3x3<f32>(frag_tangent_adjusted, frag_binormal, output.frag_normal));
    output.tbn0 = tbn[0];
    output.tbn1 = tbn[1];
    output.tbn2 = tbn[2];

    output.position = mvp * vec4<f32>(input.vertex_position, 1.0);

    return output;
}

// Fragment shader in WGSL
struct Light {
    enabled: i32,
    light_type: i32,
    position: vec3<f32>,
    light_target: vec3<f32>,
    color: vec4<f32>,
    intensity: f32,
};

@group(0) @binding(2) var<uniform> num_of_lights: i32;
@group(0) @binding(3) var<storage, read> lights: array<Light, 4>;

@group(0) @binding(4) var<uniform> view_pos: vec3<f32>;
@group(0) @binding(5) var<uniform> albedo_color: vec4<f32>;
@group(0) @binding(6) var<uniform> emissive_color: vec4<f32>;
@group(0) @binding(7) var<uniform> normal_value: f32;
@group(0) @binding(8) var<uniform> metallic_value: f32;
@group(0) @binding(9) var<uniform> roughness_value: f32;
@group(0) @binding(10) var<uniform> ao_value: f32;
@group(0) @binding(11) var<uniform> emissive_power: f32;
@group(0) @binding(12) var<uniform> ambient_color: vec3<f32>;
@group(0) @binding(13) var<uniform> ambient: f32;

@group(0) @binding(14) var albedo_map: texture_2d<f32>;
@group(0) @binding(15) var mra_map: texture_2d<f32>;
@group(0) @binding(16) var normal_map: texture_2d<f32>;
@group(0) @binding(17) var emissive_map: texture_2d<f32>;

fn schlick_fresnel(h_dot_v: f32, refl: vec3<f32>) -> vec3<f32> {
    return refl + (1.0 - refl) * pow(1.0 - h_dot_v, 5.0);
}

fn ggx_distribution(n_dot_h: f32, roughness: f32) -> f32 {
    let a = roughness * roughness;
    let d = n_dot_h * n_dot_h * (a - 1.0) + 1.0;
    return a / max(PI * d * d, 0.0000001);
}

fn geom_smith(n_dot_v: f32, n_dot_l: f32, roughness: f32) -> f32 {
    let r = roughness + 1.0;
    let k = (r * r) / 8.0;
    let ggx1 = n_dot_v / (n_dot_v * (1.0 - k) + k);
    let ggx2 = n_dot_l / (n_dot_l * (1.0 - k) + k);
    return ggx1 * ggx2;
}

fn compute_pbr(input: VertexOutput) -> vec3<f32> {
    var albedo = textureSample(albedo_map, sampler, input.frag_texcoord).rgb;
    albedo = albedo_color.rgb * albedo;

    var metallic = clamp(metallic_value, 0.0, 1.0);
    var roughness = clamp(roughness_value, 0.0, 1.0);
    var ao = clamp(ao_value, 0.0, 1.0);

    var N = normalize(input.frag_normal);

    var V = normalize(view_pos - input.frag_position);

    let emissive = vec3<f32>(0.0);
    let base_refl = mix(vec3<f32>(0.04), albedo.rgb, metallic);

    var light_accum = vec3<f32>(0.0);
    for (var i = 0; i < num_of_lights; i = i + 1) {
        if (lights[i].enabled == 0) {
            continue;
        }

        let L = normalize(lights[i].position - input.frag_position);
        let H = normalize(V + L);
        let dist = length(lights[i].position - input.frag_position);
        let attenuation = 1.0 / (dist * dist * 0.23);
        let radiance = lights[i].color.rgb * lights[i].intensity * attenuation;

        let n_dot_v = max(dot(N, V), 0.0000001);
        let n_dot_l = max(dot(N, L), 0.0000001);
        let h_dot_v = max(dot(H, V), 0.0);
        let n_dot_h = max(dot(N, H), 0.0);

        let D = ggx_distribution(n_dot_h, roughness);
        let G = geom_smith(n_dot_v, n_dot_l, roughness);
        let F = schlick_fresnel(h_dot_v, base_refl);

        let spec = (D * G * F) / (4.0 * n_dot_v * n_dot_l);

        let kD = vec3<f32>(1.0) - F;
        kD *= 1.0 - metallic;

        light_accum += ((kD * albedo.rgb / PI + spec) * radiance * n_dot_l);
    }

    let ambient_final = (ambient_color + albedo) * ambient * 0.5;
    return ambient_final + light_accum * ao + emissive;
}

@fragment
fn fs_main(input: FragmentInput) -> @location(0) vec4<f32> {
    var color = compute_pbr(input);

    color = pow(color, color + vec3<f32>(1.0));
    color = pow(color, vec3<f32>(1.0 / 2.2));

    return vec4<f32>(color, 1.0);
}
)";
int main(){
    InitWindow(800, 600, "Shader Loading");
    vertex vaodata[3] = {
        vertex{.pos = Vector3{0,0,0}, .uv = Vector2{0,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{100,0,0}, .uv = Vector2{1,0}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
        vertex{.pos = Vector3{0,100,0}, .uv = Vector2{0,1}, .normal = Vector3{0,0,1}, .col = Vector4{1,1,1,1}},
    };
    DescribedBuffer* buf = GenBuffer(vaodata, sizeof(vaodata));
    VertexArray* vao = LoadVertexArray();
    VertexAttribPointer(vao, buf, 0, WGPUVertexFormat_Float32x3, sizeof(float) * 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 1, WGPUVertexFormat_Float32x2, sizeof(float) * 2, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 2, WGPUVertexFormat_Float32x3, sizeof(float) * 5, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, buf, 3, WGPUVertexFormat_Float32x4, sizeof(float) * 8, WGPUVertexStepMode_Vertex);

    DescribedPipeline* pl = LoadPipelineForVAO(shaderSource1, vao);
    DescribedSampler smp = LoadSampler(repeat, nearest);
    Texture tex = LoadTextureFromImage(GenImageColor(RED, 10, 10));
    Matrix scr = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    SetPipelineUniformBufferData(pl, GetUniformLocation(pl, "Perspective_View"), &scr, sizeof(Matrix));
    SetPipelineTexture          (pl, GetUniformLocation(pl, "colDiffuse"), tex);
    SetPipelineSampler          (pl, GetUniformLocation(pl, "texSampler"), smp);
    
    while(!WindowShouldClose()){
        
        BeginDrawing();
        DrawFPS(0, 0);
        BeginPipelineMode(pl);
        
        
        BindPipelineVertexArray(pl, vao);
        DrawArrays(WGPUPrimitiveTopology_TriangleList, 3);
        EndPipelineMode();
        EndDrawing();
    }
}
