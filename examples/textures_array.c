#include <raygpu.h>
const char computeSource[] = R"(
@group(0) @binding(0) var output: texture_storage_2d_array<rgba8unorm, write>;
//@group(0) @binding(1) var<write> velBuffer: array<vec2<f32>>;

@compute
@workgroup_size(1, 1, 1)
fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {
    textureStore(output, id.xy, 0, vec4<f32>(0,0,0,0));
    //posBuffer[id.x * 16 + id.y] = posBuffer[id.x * 16 + id.y] + velBuffer[id.x * 16 + id.y];
})";
int main(void){
    InitWindow(800, 600, "Texture Array");

    DescribedComputePipeline* cpl = LoadComputePipeline(computeSource);
    Texture2DArray input = LoadTextureArray(100, 100, 100, RGBA8);
    Texture2DArray output = LoadTextureArray(100, 100, 100, RGBA8);
    while(!WindowShouldClose()){
        BeginDrawing();

        EndDrawing();
    }
}
