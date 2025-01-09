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
@group(0) @binding(1) var texture0: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<vec4f>;
@vertex fn vs_main(@builtin(instance_index) instanceIdx: u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * vec4f(modelMatrix[instanceIdx].xyz + in.position, 1.0f);
    out.normal = in.normal;
    out.color = in.color;
    return out;
}
@fragment fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f((in.color * dot(in.normal, vec3f(-0.3,0.8,-0.55))).xyz, 1.0f);
    
})";
DescribedPipeline* pl;
Mesh cubeMesh;
Camera3D cam;
std::vector<Vector4> instancetransforms;
void mainloop(){
    BeginDrawing();
    ClearBackground(BLANK);
    //TRACELOG(LOG_WARNING, "Setting storig buffer");
    //TRACELOG(LOG_WARNING, "pl %ull", pl);
    //TRACELOG(LOG_WARNING, "GetActivePipeline %ull", GetActivePipeline());
    //TRACELOG(LOG_WARNING, "Setting storig buffer");

    BeginPipelineMode(pl);
    UseNoTexture();
    BeginMode3D(cam);
    BindPipelineVertexArray(pl, cubeMesh.vao);
    DrawArraysIndexedInstanced(WGPUPrimitiveTopology_TriangleList, *cubeMesh.ibo, 36, instancetransforms.size());
    //DrawMeshInstanced(cubeMesh, Material{}, instancetransforms.data(), instancetransforms.size());
    EndMode3D();
    EndPipelineMode();
    DrawFPS(0, 10);
    if(IsKeyPressed(KEY_P)){
        TakeScreenshot("screenshot.png");
    }
    if(IsKeyDown(KEY_W)){
        cam.position.y += GetFrameTime() * 0.1f;
    }
    if(IsKeyDown(KEY_S)){
        cam.position.y -= GetFrameTime() * 0.1f;
    }
    EndDrawing();
}

int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(1200, 800, "Cube Benchmark");
    SetTargetFPS(0);
    float scale = 1.0f;
    cubeMesh = GenMeshCube(scale, scale, scale);
    cam = Camera3D{
        .position = Vector3{0,70,0},
        .target = Vector3{100,0,100},
        .up = Vector3{0,1,0},
        .fovy = 50.0f
    };
    pl = LoadPipelineForVAO(shaderSource, cubeMesh.vao);
    auto smp = LoadSampler(repeat, nearest);
    SetPipelineSampler(pl, 2, smp);
    for(int i = -0;i < 2000;i++){
        for(int j = 0;j < 2000;j++){
            instancetransforms.push_back(Vector4(i, std::sin(2 * M_PI * i / 10.0f) + 15.0f * -std::cos(2 * M_PI * j / 90.0f), j));
        }
    }
    DescribedBuffer* persistent = GenStorageBuffer(instancetransforms.data(), instancetransforms.size() * sizeof(Vector4));
    SetPipelineStorageBuffer(pl, 3, persistent);
    //TRACELOG(LOG_WARNING, "OOO: %llu", (unsigned long long)persistent.buffer);
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}