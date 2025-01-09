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
DescribedSampler smp;
Texture mipmappedTexture;
Camera3D cam;
constexpr float size = 100;
RenderTexture groundTruth;
Texture checker;

void mainloop(){
    BeginDrawing();
    ClearBackground(DARKGRAY);

    BeginTextureMode(groundTruth);
    ClearBackground(BLANK);
    BeginMode3D(cam);
    UseTexture(mipmappedTexture);
    rlBegin(RL_QUADS);
    rlColor3f(1.0f, 1.0f, 1.0f);
    rlTexCoord2f(0, 0);
    rlVertex3f(0, 0, 0);

    rlTexCoord2f(1, 0);
    rlVertex3f(size, 0, 0);
    
    rlTexCoord2f(1, 1);
    rlVertex3f(size, 0, size);
    
    rlTexCoord2f(0, 1);
    rlVertex3f(0, 0, size);
    
    rlEnd();
    EndMode3D();
    //DrawCircle(GetMouseX(), GetMouseY(), 1000.0f, GREEN);
    EndTextureMode();
    //DrawTexturePro(checker, Rectangle(0,0,50,50), Rectangle(0,0,100,100), Vector2{0,0}, 0.0f, WHITE);
    //DrawTexturePro(groundTruth.color, Rectangle(0,0, groundTruth.color.width, groundTruth.color.height), Rectangle(0,0,GetScreenWidth(), GetScreenHeight()), Vector2{0,0}, 0.0, WHITE);
    //DrawTexturePro(groundTruth.color, Rectangle(0,0, groundTruth.color.width, groundTruth.color.height), Rectangle(0,0,GetScreenWidth(), GetScreenHeight()), Vector2{0,0}, 0.0, WHITE);
    //DrawTexturePro(groundTruth.color, Rectangle(0,0, groundTruth.color.width, groundTruth.color.height), Rectangle(0,0,GetScreenWidth(), GetScreenHeight()), Vector2{0,0}, 0.0, WHITE);
    
    BeginMode3D(cam);
    UseTexture(mipmappedTexture);
    rlBegin(RL_QUADS);
    rlColor3f(1.0f, 1.0f, 1.0f);
    rlTexCoord2f(0, 0);
    rlVertex3f(0, 0, 0);

    rlTexCoord2f(1, 0);
    rlVertex3f(size, 0, 0);
    
    rlTexCoord2f(1, 1);
    rlVertex3f(size, 0, size);
    
    rlTexCoord2f(0, 1);
    rlVertex3f(0, 0, size);
    
    rlEnd();
    EndMode3D();
    DrawFPS(5, 5);
    EndDrawing();
    //Image gt = LoadImageFromTexture(groundTruth.color);
    //SaveImage(gt, "Samthing.png");
    //UnloadImage(gt);
}

int main(){
    //SetConfigFlags(FLAG_HEADLESS | FLAG_STDOUT_TO_FFMPEG);
    InitWindow(1920, 1080, "Title");
    OpenSubWindow(500, 500, "Controls");
    SetTargetFPS(0);
    groundTruth = LoadRenderTexture(3840, 2160);
    
    cpl = LoadComputePipeline(computeCode);
    mipmappedTexture = LoadTexturePro(
        1000, 1000,
        WGPUTextureFormat_RGBA8Unorm, WGPUTextureUsage_StorageBinding | WGPUTextureUsage_CopySrc | WGPUTextureUsage_CopyDst | WGPUTextureUsage_TextureBinding,
        1,
        10
    );
    Image img = GenImageChecker(WHITE, BLACK, 1000, 1000, 50);
    UpdateTexture(mipmappedTexture, img.data);
    GenTextureMipmaps(&mipmappedTexture);
    checker = LoadTextureFromImage(GenImageChecker(RED, BLACK, 50, 50, 5));
    WGPUSamplerDescriptor sd{};
    cam = Camera3D{
        .position = Vector3{-2,2,size / 2},
        .target = Vector3{3,0, size / 2},
        .up = Vector3{0,1,0},
        .fovy = 60.0f
    };

    smp = LoadSamplerEx(clampToEdge, linear, linear, 1000.0f);
    SetSampler(2, smp);
    
    //Image exp = LoadImageFromTextureEx(groundTruth.color.id, 0);
    //SaveImage(exp, "mip.png");
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else 
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}