#include <raygpu.h>
constexpr char shaderCode[] = R"(
@group(0) @binding(0) var tex: texture_storage_2d<r32uint, read_write>;

@compute
@workgroup_size(16, 16, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    textureStore(tex, id.xy, vec4<u32>(0xff000000 | (id.x & 255) | ((id.y & 255) << 8), 0, 0, 0));

}
)";
int main(){
    InitWindow(1280, 1280, "Storage Texture");
    SetTargetFPS(0);
    //UniformDescriptor udesc[1] = {
    //    UniformDescriptor{.type = storage_texture3d,.minBindingSize = 0, .location = 0}
    //};
    Texture storageTex = LoadTexturePro(1280, 1280, WGPUTextureFormat_R32Uint, WGPUTextureUsage_CopySrc | WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding, 1);
    Texture tex = LoadTexturePro(1280, 1280, WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst, 1);
    DescribedComputePipeline* pl = LoadComputePipeline(shaderCode);
    WGPUImageCopyTexture source{};

    //source.aspect = WGPUTex
    //wgpuCommandEncoderCopyTextureToTexture(WGPUCommandEncoder commandEncoder, const WGPUImageCopyTexture *source, const WGPUImageCopyTexture *destination, const WGPUExtent3D *copySize)
    SetBindgroupTexture(&pl->bindGroup, 0, storageTex);
    while(!WindowShouldClose()){
        BeginComputepass();
        BindComputePipeline(pl);
        DispatchCompute(1280 / 16, 1280 / 16, 1);
        ComputepassEndOnlyComputing();
        CopyTextureToTexture(storageTex, tex);

        EndComputepass();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(tex, Rectangle(0,0,1280,1280), Rectangle(0,0,1280,1280), Vector2{0,0}, 0.0f, WHITE);
        DrawFPS(5,5);
        EndDrawing();
    }
}