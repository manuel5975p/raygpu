

#include <raygpu.h>

constexpr char firstPassSource[] = R"(

const filtersize = 100;

@group(0) @binding(0) var input: texture_2d<f32>;
@group(0) @binding(1) var output: texture_storage_2d<rgba32float, write>;

@compute @workgroup_size(8,8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    var accum: vec4<f32> = vec4<f32>(0.0f,0.0f,0.0f,0.0f);
    let ldval = textureLoad(input, id.xy, 0);
    for(var i : i32 = 0;i < filtersize;i++){
        let dist: f32 = f32(i) - f32(filtersize) / 2.0f;
        let scale = exp(-dist * dist / f32(filtersize * filtersize / 4.0f));
        accum += (2.0f / filtersize) * scale * textureLoad(input, vec2<i32>(i32(id.x) + (i - filtersize / 2), i32(id.y)), 0);
    }
    textureStore(output, id.xy, accum);
}
)";
constexpr char secondPassSource[] = R"(

const filtersize = 100;

@group(0) @binding(0) var original: texture_2d<f32>;
@group(0) @binding(1) var input: texture_2d<f32>;
@group(0) @binding(2) var output: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(8,8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    var accum: vec4<f32> = vec4<f32>(0.0f,0.0f,0.0f,0.0f);
    let ldval = textureLoad(input, id.xy, 0);
    let orig = textureLoad(original, id.xy, 0);
    for(var i : i32 = 0;i < filtersize;i++){
        let dist: f32 = f32(i) - f32(filtersize) / 2.0f;
        let scale = exp(-dist * dist / f32(filtersize * filtersize / 4.0f));
        accum += (2.0f / filtersize) * scale * textureLoad(input, vec2<i32>(i32(id.x), i32(id.y) + (i - filtersize / 2)), 0);
    }
    textureStore(output, id.xy, orig + accum);
}
)";
DescribedComputePipeline* firstPassPipeline;
DescribedComputePipeline* secondPassPipeline;

const uint32_t width = 800;
const uint32_t height = 800;


int main(){
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(width, height, "Bloom");
    firstPassPipeline = LoadComputePipeline(firstPassSource);
    secondPassPipeline = LoadComputePipeline(secondPassSource);
    RenderTexture rtex = LoadRenderTexture(width, height);

    Texture intermediary = LoadTexturePro(width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, RGTextureUsage_TextureBinding | RGTextureUsage_StorageBinding, 1, 1);
    Texture output = LoadTexturePro(width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, RGTextureUsage_TextureBinding | RGTextureUsage_StorageBinding, 1, 1);

    SetBindgroupTexture(&firstPassPipeline->bindGroup, 0, rtex.texture);
    SetBindgroupTexture(&firstPassPipeline->bindGroup, 1, intermediary);
    SetBindgroupTexture(&secondPassPipeline->bindGroup, 0, rtex.texture);
    SetBindgroupTexture(&secondPassPipeline->bindGroup, 1, intermediary);
    SetBindgroupTexture(&secondPassPipeline->bindGroup, 2, output);


    while(!WindowShouldClose()){
        BeginTextureMode(rtex);
        ClearBackground(BLANK);
        DrawCircleV(GetMousePosition(), height / 10.f, WHITE);
        EndTextureMode();
        BeginComputepass();
        BindComputePipeline(firstPassPipeline);
        DispatchCompute(width / 8, height / 8, 1);
        BindComputePipeline(secondPassPipeline);
        DispatchCompute(width / 8, height / 8, 1);
        EndComputepass();
        BeginDrawing();
        DrawTexture(output, 0, 0, WHITE);
        DrawFPS(5, 5);
        EndDrawing();
    }
}
