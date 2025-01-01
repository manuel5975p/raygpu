#include <raygpu.h>
#include <vector>
#include <algorithm>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
const char shaderSource[] = 
R"(struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) uv: vec2f,
    @location(2) normal: vec3f,
    @location(3) color: vec4f
};
struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
    @location(1) normal: vec3f
};
@group(0) @binding(0) var <uniform> Perspective_View: mat4x4<f32>;
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
@vertex fn vs_main(@builtin(instance_index) instanceIdx: u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * modelMatrix[instanceIdx] * vec4f(in.position.xyz, 1.0f);
    out.normal = in.normal;
    out.color = in.color;
    return out;
}
@fragment fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f((in.color * dot(in.normal, vec3f(0.2,0.8,0.45))).xyz, 1.0f);
    
})";
DescribedPipeline* pl;
Mesh cubeMesh;
Camera3D cam;
int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920, 1080, "Cube Benchmark");
    float scale = 0.04f;
    cubeMesh = GenMeshCube(scale, scale, scale);
    cam = Camera3D{
        .position = Vector3{1,2,12},
        .target = Vector3{0,0,0},
        .up = Vector3{0,1,0},
        .fovy = 1.0f
    };
    pl = LoadPipelineForVAO(shaderSource, cubeMesh.vao);
    auto smp = LoadSampler(repeat, nearest);
    SetPipelineSampler(pl, 2, smp);
    std::vector<Matrix> instancetransforms(1000000);
    std::generate(instancetransforms.begin(), instancetransforms.end(), []{
        return MatrixTranslate((drand48() - 0.5) * 10, (drand48() - 0.5) * 10, (drand48() - 0.5) * 10);
    });
    DescribedBuffer persistent = GenStorageBuffer(instancetransforms.data(), instancetransforms.size() * sizeof(Matrix));
    SetPipelineStorageBuffer(pl, 3, &persistent);
    //TRACELOG(LOG_WARNING, "OOO: %llu", (unsigned long long)persistent.buffer);
    auto mainloop = [&]{
        BeginDrawing();
        ClearBackground(BLANK);
        //TRACELOG(LOG_WARNING, "Setting storig buffer");
        //TRACELOG(LOG_WARNING, "pl %ull", pl);
        //TRACELOG(LOG_WARNING, "GetActivePipeline %ull", GetActivePipeline());
        //TRACELOG(LOG_WARNING, "Setting storig buffer");
        
        BeginPipelineMode(pl);
        UseNoTexture();
        BeginMode3D(cam);

        BindVertexArray(pl, cubeMesh.vao);
        DrawArraysIndexedInstanced(WGPUPrimitiveTopology_TriangleList, cubeMesh.ibo, 36, 1000000);

        //DrawMeshInstanced(cubeMesh, Material{}, instancetransforms.data(), instancetransforms.size());
        EndMode3D();
        EndPipelineMode();
        DrawFPS(0, 10);
        if(IsKeyPressed(KEY_P)){
            TakeScreenshot("screenshot.png");
        }
        EndDrawing();
    };

    using mainloop_type = decltype(mainloop);
    auto mainloopCaller = [](void* x){
        (*((decltype(mainloop)*)x))();
    };
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop_arg(mainloopCaller, (void*)&mainloop, 0, 0);
    #endif
}