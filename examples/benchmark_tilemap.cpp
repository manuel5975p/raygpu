#include <filesystem>
#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
const char wgsl[] = R"(
@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> tileTypes: array<u32>;

struct VertexInput {
    @location(0) position: vec2f,
    @location(1) offset: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) @interpolate(flat) offset: vec2i,
    @location(1) uv: vec2f
};

@vertex
fn vs_main(@builtin(instance_index) instanceID: u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let x: i32 = i32(tileTypes[instanceID] % 60);
    let y: i32 = i32(tileTypes[instanceID] / 60);
    out.offset = vec2i(x * 16, y * 16);
    out.position = Perspective_View * vec4f(in.position.xy + in.offset.xy, 0.0f, 1.0f);
    out.uv = in.position;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    var uvi: vec2i = vec2i(i32(in.uv.x * 16), i32(in.uv.y * 16));
    uvi += in.offset;
    var color = textureLoad(colDiffuse, uvi, 0);
    return color;
}
)";
Texture tilemap;
Camera2D cam;
DescribedPipeline* pl;
VertexArray* tileVAO;
DescribedBuffer* tileVBO;
DescribedBuffer* tileIBO;
DescribedBuffer* tileOffsets;
DescribedBuffer* tileTypes;
int tileCount;

void mainloop(){
    BeginDrawing();
    //ClearBackground(BLACK);
    BeginPipelineMode(pl);
    BeginMode2D(cam);
    BindPipelineVertexArray(pl, tileVAO);
    DrawArraysIndexedInstanced(WGPUPrimitiveTopology_TriangleList, *tileIBO, 6, tileCount);
    //DrawTexturePro(tilemap, Rectangle{0,0,(float)tilemap.width, (float)tilemap.height}, Rectangle{0,0,1000,1000}, Vector2{0,0}, 0.0f, WHITE);
    EndMode2D();
    EndPipelineMode();
    DrawFPS(10,10);
    cam.zoom *= std::exp(GetMouseWheelMove() / 30.0f);
    const float speed = 200.0f;
    if(IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)){
        cam.target.y -= speed * GetFrameTime() / cam.zoom;
    }
    if(IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)){
        cam.target.y += speed * GetFrameTime() / cam.zoom;
    }
    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)){
        cam.target.x -= speed * GetFrameTime() / cam.zoom;
    }
    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)){
        cam.target.x += speed * GetFrameTime() / cam.zoom;
    }
    EndDrawing();
}
int main(){
    //SetConfigFlags(FLAG_MSAA_4X_HINT);
    //SetConfigFlags(FLAG_VSYNC_HINT);
    std::filesystem::path p(".");
    std::cout << std::filesystem::absolute(p) << "\n";
    InitWindow(640, 480, "Tilemap Benchmark");
    SetTargetFPS(0);
    Image img = LoadImage("../resources/tileset.png");
    tilemap = LoadTextureFromImage(img);
    cam = Camera2D{
        .offset = Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f}, 
        .target = Vector2{0,0},
        .rotation = 0.0f, 
        .zoom = 100.0f
    };
    pl = LoadPipeline(wgsl);

    SetPipelineTexture(pl, 1, tilemap);
    SetPipelineSampler(pl, 2, LoadSampler(repeat, nearest));
    tileVAO = LoadVertexArray();
    float tileVertices [8] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };
    uint32_t tileIndices[6] = {
        0,1,2,0,2,3
    };
    tileIBO = GenIndexBuffer(tileIndices, sizeof(tileIndices));
    tileVBO = GenBuffer(tileVertices, sizeof(tileVertices));
    std::vector<Vector2> offsets;
    std::vector<uint32_t> tt;
    int w = 1000;
    int wh = w / 2;
    tileCount = 0;
    for(int i = 0;i < w;i++){
        for(int j = 0;j < w;j++){
            offsets.emplace_back(i - wh, j - wh);
            tt.emplace_back(rand() % 3 + (rand() % 3) * 60);
            ++tileCount;
        }
    }
    tileOffsets = GenBuffer(offsets.data(), offsets.size() * sizeof(Vector2));
    tileTypes = GenStorageBuffer(tt.data(), tt.size() * sizeof(uint32_t));
    VertexAttribPointer(tileVAO, tileVBO, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(tileVAO, tileOffsets, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Instance);
    SetPipelineStorageBuffer(pl, 3, tileTypes);
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}