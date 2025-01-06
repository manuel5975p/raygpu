#include <raygpu.h>
const char source[] = 
R"(struct VertexInput {
    @location(0) position: vec2f,
    @location(1) uv: vec2f,
    @location(2) normal: vec3f,
    @location(3) color: vec4f
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec4f
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xy, 0.0f, 1.0f);
    out.color = in.color;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color.xyz,1);
}
)";
int main(){
    InitWindow(1200, 800, "VAO");
    //UniformDescriptor uniforms[1] = {
    //    (UniformDescriptor){
    //        .minBindingSize = 16,
    //        .type = uniform_buffer
    //    }
    //};
    float positions[6] = {
        0,0,
        1,0,
        0,1
    };
    float positions2[6] = {
        0,0,
        1,0.2,
        -0.4,0.5
    };
    float uvs[6] = {
        0,0,
        1,0,
        0,1
    };
    float normals[9] = {
        0,1,1,
        1,0,1,
        1,1,0
    };
    float colors[12] = {
        0,1,1,1,
        1,0,1,1,
        1,1,0,1
    };
    Camera3D cam =  CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){2,3,4},
        .target =   CLITERAL(Vector3){0,0,0},
        .up =       CLITERAL(Vector3){0,1,0},
        .fovy = 1.0f
    };
    DescribedBuffer* posb = GenBuffer(positions, sizeof(positions));
    DescribedBuffer* posu = GenBuffer(uvs, sizeof(uvs));
    DescribedBuffer* posn = GenBuffer(normals, sizeof(normals));
    DescribedBuffer* posc = GenBuffer(colors, sizeof(colors));
    DescribedBuffer* posb2 = GenBuffer(positions2, sizeof(positions2));
    DescribedBuffer* posu2 = GenBuffer(uvs, sizeof(uvs));
    DescribedBuffer* posn2 = GenBuffer(normals, sizeof(normals));
    DescribedBuffer* posc2 = GenBuffer(colors, sizeof(colors));
    VertexArray* vao = LoadVertexArray();

    VertexAttribPointer(vao, posb, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posu, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posn, 2, WGPUVertexFormat_Float32x3, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posc, 3, WGPUVertexFormat_Float32x4, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posb2, 0, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posu2, 1, WGPUVertexFormat_Float32x2, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posn, 2, WGPUVertexFormat_Float32x3, 0, WGPUVertexStepMode_Vertex);
    VertexAttribPointer(vao, posc2, 3, WGPUVertexFormat_Float32x4, 0, WGPUVertexStepMode_Vertex);

    RenderSettings settings zeroinit;

    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;

    //DescribedPipeline* pl = LoadPipelineEx(source, nullptr, 0, nullptr, 0, settings);
    //DescribedPipeline* pl = DefaultPipeline();
    DescribedPipeline* pl = LoadPipelineForVAOEx(source, vao, nullptr, 0, settings);
    PreparePipeline(pl, vao);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLANK);
        UseNoTexture();
        BeginMode3D(cam);
        BeginPipelineMode(pl);
        BindVertexArray(pl, vao);
        DrawArrays(WGPUPrimitiveTopology_TriangleList, 3);
        EndPipelineMode();
        EndMode3D();
        DrawFPS(0, 0);
        EndDrawing();
    }
}