#include <raygpu.h>
constexpr char shaderCode[] = R"(
@group(0) @binding(0) var tex: texture_storage_3d<r32float, read_write>;

@compute
@workgroup_size(64, 1, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    textureStore(tex, id, vec4<f32>(1.0f,1.0f,1.0f,1.0f));
}
)";
int main(){
    InitWindow(800, 800, "Storage Texture");
    UniformDescriptor udesc[1] = {
        UniformDescriptor{.type = storage_texture3d,.minBindingSize = 0, .location = 0}
    };
    Texture3D tex = LoadTexture3DEx(100,100,100,WGPUTextureFormat_R32Float);
    DescribedComputePipeline* pl = LoadComputePipelineEx(shaderCode, udesc, 1);
    SetBindgroupTexture3D(&pl->bindGroup, 0, tex);
    while(!WindowShouldClose()){
        BeginComputepass();
        BindComputePipeline(pl);
        EndComputepass();

        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        EndDrawing();
        break;
    }
}