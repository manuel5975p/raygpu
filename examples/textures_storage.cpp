#include <raygpu.h>
constexpr char shaderCode[] = R"(
@group(0) @binding(0) var tex: texture_storage_2d<r32uint, read_write>;
@group(0) @binding(1) var<uniform> scale: f32;
@group(0) @binding(2) var<uniform> center: vec2<f32>;
fn mandelIter(p: vec2f, c: vec2f) -> vec2f{
    let newr: f32 = p.x * p.x - p.y * p.y;
    let newi: f32 = 2 * p.x * p.y;

    return vec2f(newr + c.x, newi + c.y);
}
const maxiters = 10000;
fn mandelBailout(z: vec2f) -> u32{
    var zn = z;
    var i: u32;
    for(i = 0;i < maxiters;i = i + 1){
        zn = mandelIter(zn, z);
        if(zn.x * zn.x + zn.y * zn.y > 4.0f){
            break;
        }
    }
    return i;
}
@compute
@workgroup_size(16, 16, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let ld: vec4<u32> = textureLoad(tex, id.xy);
    let mandelSample: vec2f = (vec2f(id.xy) - 640.0f) * scale + center;
    let iters = mandelBailout(mandelSample);
    if(iters == maxiters){
        textureStore(tex, id.xy, vec4<u32>(0xff000000, 0, 0, 0));
    }
    else{
        let inorm: f32 = f32(iters);
        let colorSpace = 3.0f * log(inorm + 1) / log(f32(maxiters));
        let colorr: u32 = u32((0.5f * sin(10.0f * colorSpace) + 0.5f) * 255.0f);
        let colorg: u32 = u32((0.5f * sin(4.0f * colorSpace) + 0.5f) * 255.0f);
        textureStore(tex, id.xy, vec4<u32>(colorr | (colorg << 8) | 0xff000000, 0, 0, 0));
    }
}
)";
int main(){
    //SetConfigFlags();
    InitWindow(1280, 1280, "Storage Texture");
    SetTargetFPS(0);
    Texture storageTex = LoadTexturePro(1280, 1280, WGPUTextureFormat_R32Uint, WGPUTextureUsage_CopySrc | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding, 1);
    Texture tex = LoadTexturePro(1280, 1280, WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, 1);
    DescribedComputePipeline* pl = LoadComputePipeline(shaderCode);
    WGPUImageCopyTexture source{};

    SetBindgroupTexture(&pl->bindGroup, 0, storageTex);
    float scale = 1.0f / 500.0f;
    Vector2 center{0,0};
    while(!WindowShouldClose()){
        scale /= std::exp(GetMouseWheelMove() * 1e-1);
        if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
            center.y += (scale) * 0.2f;
        }
        if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
            center.y -= (scale) * 0.2f;
        }
        if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
            center.x -= (scale) * 0.2f;
        }
        if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
            center.x += (scale) * 0.2f;
        }
        SetBindgroupUniformBufferData(&pl->bindGroup, 1, &scale, sizeof(float));
        SetBindgroupUniformBufferData(&pl->bindGroup, 2, &center, sizeof(Vector2));
        BeginComputepass();
        BindComputePipeline(pl);
        DispatchCompute(1280 / 16, 1280 / 16, 1);
        ComputepassEndOnlyComputing();
        CopyTextureToTexture(storageTex, tex);

        EndComputepass();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(tex, Rectangle(0,0,1280,1280), Rectangle(0,0,1280,1280), Vector2{0,0}, 0.0f, WHITE);
        //DrawFPS(5, 5);
        EndDrawing();
    }
}