#include <raygpu.h>
#include <vector>
const char source[] = 
R"(struct VertexInput {
    @location(0) position: vec2f,
    @location(1) offset: vec2f
};

struct VertexOutput {
    @builtin(position) position: vec4f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy + in.offset.xy, 0.0f, 1.0f);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(1,1,1,1);
}
)";
int main(){
    InitWindow(1920, 1080, "VAO");
    AttributeAndResidence attributes[2] = {
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x2, 
                                          .offset = 0, 
                                          .shaderLocation = 0},
            
            .bufferSlot = 0, 
            .stepMode = WGPUVertexStepMode_Vertex
        },
        CLITERAL(AttributeAndResidence){
            .attr = CLITERAL(WGPUVertexAttribute){.format = WGPUVertexFormat_Float32x2, 
                                          .offset = 0, 
                                          .shaderLocation = 1},
            
            .bufferSlot = 1,
            .stepMode = WGPUVertexStepMode_Instance
        }
    };

    float size = 1.0f / 2048;
    float qsize = size * 0.8f;
    float positions[8] = {
        0,0,
        qsize,0,
        qsize,qsize,
        0,qsize
    };
    std::vector<Vector2> offsets;
    for(float i = -1;i <= 1;i += size){
        for(float j = -1;j <= 1;j += size){
            offsets.push_back(Vector2{i, j});
        }
    }
    DescribedBuffer posb = GenBuffer(positions, sizeof(positions));
    DescribedBuffer poso = GenBuffer(offsets.data(), offsets.size() * sizeof(Vector2));
    VertexArray* vao = LoadVertexArray();

    VertexAttribPointer(vao, &posb, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, &poso, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Instance);

    RenderSettings settings zeroinit;

    settings.depthTest = 1;
    uint32_t trifanIndices[6] = {
        0,1,2,0,2,3
    };
    DescribedBuffer ibo = GenIndexBuffer(trifanIndices, sizeof(trifanIndices));
    settings.depthCompare = WGPUCompareFunction_LessEqual;

    DescribedPipeline* pl = LoadPipelineEx(source, attributes, 2, nullptr, 0, settings);
    TRACELOG(LOG_INFO, "Drawing %d triangles", (int)offsets.size());
    PreparePipeline(pl, vao);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLANK);
        UseNoTexture();
        BeginPipelineMode(pl, WGPUPrimitiveTopology_TriangleList);
        BindVertexArray(pl, vao);
        DrawArraysIndexedInstanced(ibo, 6, offsets.size());
        EndPipelineMode();
        DrawFPS(0, 0);
        EndDrawing();
    }
}