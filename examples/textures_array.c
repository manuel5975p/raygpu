#include <raygpu.h>
const char computeSource[] =
    "@group(0) @binding(0) var output: texture_storage_2d_array<rgba8unorm, write>;\n"
    "\n"
    "@compute\n"
    "@workgroup_size(1, 1, 1)\n"
    "fn compute_main(@builtin(global_invocation_id) id: vec3<u32>) {\n"
    "    textureStore(output, id.xy, 0, vec4<f32>(0,0,0,0));\n"
    "}\n";
int main(void){
    InitWindow(800, 600, "Texture Array");

    DescribedComputePipeline* cpl = LoadComputePipeline(computeSource);
    Texture2DArray input = LoadTextureArray(100, 100, 100, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Texture2DArray output = LoadTextureArray(100, 100, 100, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    while(!WindowShouldClose()){
        BeginDrawing();

        EndDrawing();
    }
}
