#include <raygpu.h>
constexpr char shaderSource[] = R"(
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
    @location(2) worldPos: vec3f,
    @location(3) worldNormal: vec3f,
};

struct LightBuffer {
    count: u32,
    positions: array<vec4f>
};


@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var texture0: texture_2d<f32>;
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
    vec4f(in.position.xyz, 1.0f);
    out.color = in.color;
    out.worldNormal = (modelMatrix[instanceIdx] * vec4f(in.normal.xyz, 0)).xyz;
    out.worldPos = (Perspective_View * 
                   modelMatrix[instanceIdx] *
    vec4f(in.position.xyz, 1.0f)).xyz;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    var illum: f32 = 0;
    for(var i : u32 = 0;i < lights.count;i++){
        let dist : vec3f = lights.positions[i].xyz - in.worldPos; 
        let prod = max(0.0f, dot(in.worldNormal, dist));
        illum += 100.0f * prod / dot(dist, dist);
    }
    return textureSample(texture0, grsampler, in.uv).rgba * in.color * (illum + 0.1f);
}
)";
int main(){
    constexpr bool msaa = false;
    if(msaa)
        SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Lights Example");
    struct LightBuffer{
        uint32_t count = 2;
        float pos[8] = {100, 0, 100, 0,
                        0, 40, 0, 0};
    }libuf;
    Matrix iden = MatrixIdentity();
    DescribedBuffer* idenbuffer = GenStorageBuffer(&iden, sizeof(Matrix));
    DescribedBuffer* libufs = GenStorageBuffer(&libuf, sizeof(LightBuffer));
    std::string resourceDirectoryPath = FindDirectory("resources", 3);
    Model churchModel = LoadModel((resourceDirectoryPath + "/church.obj").c_str());
    Texture cdif = LoadTextureFromImage(LoadImage((resourceDirectoryPath + "/church_diffuse.png").c_str()));
    std::cout << churchModel.meshCount << std::endl;
    Mesh churchMesh = churchModel.meshes[0];

    ResourceTypeDescriptor uniforms[5] = {
        CLITERAL(ResourceTypeDescriptor){uniform_buffer, 64, 0, readonly, format_or_sample_type(0)},
        CLITERAL(ResourceTypeDescriptor){texture2d, 0, 1,       readonly, format_or_sample_type(0)},
        CLITERAL(ResourceTypeDescriptor){texture_sampler, 0, 2,         readonly, format_or_sample_type(0)},
        CLITERAL(ResourceTypeDescriptor){storage_buffer, 64, 3, readonly, format_or_sample_type(0)},
        CLITERAL(ResourceTypeDescriptor){storage_buffer, 32, 4, readonly, format_or_sample_type(0)}
    };
    
    RenderSettings settings zeroinit;
    settings.depthTest = 1;
    settings.sampleCount = msaa ? 4 : 1;
    settings.depthCompare = CompareFunction_LessEqual;
    DescribedPipeline* pl = LoadPipelineForVAO(shaderSource, churchMesh.vao);
    Camera3D cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){20,20,30},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 40.0f
    };
    //WGPUSamplerDescriptor samplerDesc{};
    //samplerDesc.addressModeU = WGPUAddressMode_Repeat;
    //samplerDesc.addressModeV = WGPUAddressMode_Repeat;
    //samplerDesc.addressModeW = WGPUAddressMode_Repeat;
    //samplerDesc.magFilter    = WGPUFilterMode_Nearest;
    //samplerDesc.minFilter    = WGPUFilterMode_Linear;
    //samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    //samplerDesc.compare      = WGPUCompareFunction_Undefined;
    //samplerDesc.lodMinClamp  = 0.0f;
    //samplerDesc.lodMaxClamp  = 1.0f;
    //samplerDesc.maxAnisotropy = 1;
    //WGPUSampler sampler = wgpuDeviceCreateSampler(GetDevice(), &samplerDesc);

    DescribedSampler sampler = LoadSampler(repeat, filter_linear);
    Material defMaterial = LoadMaterialDefault();
    defMaterial.maps[MATERIAL_MAP_ALBEDO].texture = cdif;
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLANK);
        BeginPipelineMode(pl);
        SetSampler(2, sampler);
        SetStorageBuffer(3, idenbuffer);
        SetStorageBuffer(4, libufs);
        UseTexture(cdif);
        BeginMode3D(cam);
        DrawMesh(churchMesh, defMaterial, MatrixIdentity());
        EndMode3D();
        EndPipelineMode();
        DrawFPS(0,0);
        EndDrawing();
    }
}