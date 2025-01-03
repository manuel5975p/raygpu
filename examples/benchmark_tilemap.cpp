#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
const char wgsl[] = R"(
struct VertexInput {
    @location(0) position: vec2f,
    @location(1) offset: vec2f
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy + in.offset.xy, 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.position.xy / 400.0f,1,1);
}
)";
Texture tilemap;
DescribedPipeline* pl;
void mainloop(){
    BeginDrawing();
    BeginPipelineMode(pl);
    DrawTexturePro(tilemap, Rectangle{0,0,(float)tilemap.width, (float)tilemap.height}, Rectangle{0,0,1000,1000}, Vector2{0,0}, 0.0f, WHITE);
    EndPipelineMode();
    DrawFPS(10,10);
    EndDrawing();
}
int main(){
    InitWindow(1000, 1000, "Tilemap Benchmark");
    Image img = LoadImage("../resources/tileset.png");
    tilemap = LoadTextureFromImage(img);
    AttributeAndResidence attrs[2] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 0, 0}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 8, 1}, 0, WGPUVertexStepMode_Vertex, true},
    };
    pl = LoadPipeline(wgsl, attrs, 2);

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}