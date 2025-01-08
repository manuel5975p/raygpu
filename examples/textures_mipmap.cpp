#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
constexpr char computeCode[] = R"(
@group(0) @binding(0) var previousMipLevel: texture_2d<f32>;
@group(0) @binding(1) var nextMipLevel: texture_storage_2d<rgba8unorm, write>;

@compute @workgroup_size(8, 8)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    let offset = vec2<u32>(0, 1);
    let color = (
        textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)
    ) * 0.25;
    textureStore(nextMipLevel, id.xy, color);
}
)";
DescribedComputePipeline* cpl;
Texture loadMip(uint32_t width, uint32_t height, uint32_t sampleCount, WGPUTextureFormat format, WGPUTextureUsage usage){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = 2;
    tDesc.sampleCount = sampleCount;
    tDesc.format = format;
    tDesc.usage = usage | WGPUTextureUsage_CopyDst| WGPUTextureUsage_CopySrc | WGPUTextureUsage_StorageBinding  | WGPUTextureUsage_TextureBinding;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;

    
    Texture ret;
    ret.id = wgpuDeviceCreateTexture(GetDevice(), &tDesc);
    
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.sampleCount = sampleCount;


    WGPUImageCopyTexture destination;
    destination.texture = ret.id;
    destination.origin = { 0, 0, 0 };
    destination.aspect = WGPUTextureAspect_All;
    destination.mipLevel = 0;
    WGPUTextureDataLayout source;
    source.offset = 0;
    source.bytesPerRow = 4 * ret.width;
    source.rowsPerImage = ret.height;

    Image img = GenImageChecker(RED, BLUE, width, height, 20);

    wgpuQueueWriteTexture(GetQueue(), &destination, img.data, (size_t)(4 * width * height), &source, &tDesc.size);




    WGPUTextureViewDescriptor textureViewDesc{};

    textureViewDesc.aspect = WGPUTextureAspect_All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;


    ret.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    Texture dummy = ret;

    textureViewDesc.baseMipLevel = 1;
    dummy.view = wgpuTextureCreateView(ret.id, &textureViewDesc);
    BeginComputepass();

    SetBindgroupTexture(&cpl->bindGroup, 0, ret);
    SetBindgroupTexture(&cpl->bindGroup, 1, dummy);
    BindComputePipeline(cpl);
    DispatchCompute(width / 8, height / 8, 1);
    EndComputepass();
    return ret;
}
void mainloop(){
    BeginDrawing();
    ClearBackground(GREEN);
    DrawText("Hello there!",200, 200, 30, BLACK);
    EndDrawing();
}

int main(){
    InitWindow(800, 600, "Title");
    cpl = LoadComputePipeline(computeCode);
    auto what = loadMip(1000, 1000, 1, WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_TextureBinding);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else 
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}