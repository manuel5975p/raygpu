#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
const char wgsl[] = R"(
@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * vec4f(in.position.xyz, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let color = textureLoad(colDiffuse, vec2i(i32(in.position.x) % 128, i32(in.position.y) % 128), 0);
    return color;
}
)";
Texture tilemap;
Camera2D cam;
DescribedPipeline* pl;
void mainloop(){
    BeginDrawing();
    BeginPipelineMode(pl);
    BeginMode2D(cam);
    DrawTexturePro(tilemap, Rectangle{0,0,(float)tilemap.width, (float)tilemap.height}, Rectangle{0,0,1000,1000}, Vector2{0,0}, 0.0f, WHITE);
    EndMode2D();
    EndPipelineMode();
    DrawFPS(10,10);
    EndDrawing();
}
int main(){
    InitWindow(1000, 1000, "Tilemap Benchmark");
    Image img = LoadImage("../resources/tileset.png");
    tilemap = LoadTextureFromImage(img);
    cam = Camera2D{
        .offset = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f}, 
        .target = Vector2{0,0}, 
        .rotation = 0.0f, 
        .zoom = 1.0f
    };
    AttributeAndResidence attrs[2] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 0, 0}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 8, 1}, 0, WGPUVertexStepMode_Vertex, true},
    };
    pl = LoadPipeline(wgsl, attrs, 2);
    SetPipelineSampler(pl, 2, LoadSampler(repeat, nearest));
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}