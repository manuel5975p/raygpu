#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
uint32_t width = 1600, height = 1600;
constexpr char shaderCode[] = R"(
@group(0) @binding(0) var tex: texture_storage_2d<rgba8unorm, write>;
@group(0) @binding(1) var<uniform> scale: f32;
@group(0) @binding(2) var<uniform> center: vec2<f32>;
fn mandelIter(p: vec2f, c: vec2f) -> vec2f{
    let newr: f32 = p.x * p.x - p.y * p.y;
    let newi: f32 = 2 * p.x * p.y;

    return vec2f(newr + c.x, newi + c.y);
}
const maxiters: i32 = 150;

struct termination {
    iterations: i32,
    overshoot: f32,
}

fn mandelBailout(z: vec2f) -> termination{
    var zn = z;
    var i: i32;
    for(i = 0;i < maxiters;i = i + 1){
        zn = mandelIter(zn, z);
        if(sqrt(zn.x * zn.x + zn.y * zn.y) > 2.0f){
            break;
        }
    }
    var ret: termination;
    ret.iterations = i;
    ret.overshoot = ((sqrt(zn.x * zn.x + zn.y * zn.y)) - 2.0f) / 2.0f;
    return ret;
}
@compute
@workgroup_size(16, 16, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    //let ld: vec4<u32> = textureLoad(tex, id.xy);
    let mandelSample: vec2f = (vec2f(id.xy) - 640.0f) * scale + center;
    let iters = mandelBailout(mandelSample);
    if(iters.iterations == maxiters){
        textureStore(tex, id.xy, vec4<f32>(0, 0, 0, 1.0f));
    }
    else{
        let inorm: f32 = f32(iters.iterations) - iters.overshoot;
        let colorSpace = log(inorm + 1.0f);//3.0f * log(inorm + 1) / log(f32(maxiters));
        let colorr: f32 = 0.5f * sin(10.0f * colorSpace) + 0.5f;
        let colorg: f32 = 0.5f * sin(4.0f * colorSpace) + 0.5f;
        let colorb: f32 = 0.05f * sin(7.0f * colorSpace) + 0.1f;
        textureStore(tex, id.xy, vec4<f32>(colorr, colorg, colorb, 1.0f));
    }
}
)";


Texture storageTex;
Texture tex;
DescribedComputePipeline* pl;
float scale = 1.0f / 500.0f;
constexpr float speed = 2000.0f;
Vector2 center{0,0};

void mainloop(){
    scale /= std::exp(GetMouseWheelMove() * 1e-1);
    if(IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)){
        center.y += scale * speed * GetFrameTime();
    }
    if(IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)){
        center.y -= scale * speed * GetFrameTime();
    }
    if(IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)){
        center.x -= scale * speed * GetFrameTime();
    }
    if(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)){
        center.x += scale * speed * GetFrameTime();
    }
    SetBindgroupUniformBufferData(&pl->bindGroup, 1, &scale, sizeof(float));
    SetBindgroupUniformBufferData(&pl->bindGroup, 2, &center, sizeof(Vector2));
    BeginComputepass();
    BindComputePipeline(pl);
    DispatchCompute(width / 16, height / 16, 1);
    ComputepassEndOnlyComputing();
    CopyTextureToTexture(storageTex, tex);
    EndComputepass();
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(tex, Rectangle(0,0,width,height), Rectangle(0,0,width,height), Vector2{0,0}, 0.0f, WHITE);
    DrawFPS(5, 5);
    EndDrawing();
}


int main(){
    //SetConfigFlags();
    InitWindow(width, height, "Storage Texture");
    SetTargetFPS(0);
    storageTex = LoadTexturePro(width, height, WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_CopySrc | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding, 1, 1);
    tex = LoadTexturePro(width, height, WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, 1, 1);
    pl = LoadComputePipeline(shaderCode);

    SetBindgroupTexture(&pl->bindGroup, 0, storageTex);
    
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
    
}