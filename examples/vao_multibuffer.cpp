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
    DescribedBuffer* posb = GenVertexBuffer(positions, sizeof(positions));
    DescribedBuffer* posu = GenVertexBuffer(uvs, sizeof(uvs));
    DescribedBuffer* posn = GenVertexBuffer(normals, sizeof(normals));
    DescribedBuffer* posc = GenVertexBuffer(colors, sizeof(colors));
    DescribedBuffer* posb2 = GenVertexBuffer(positions2, sizeof(positions2));
    DescribedBuffer* posu2 = GenVertexBuffer(uvs, sizeof(uvs));
    DescribedBuffer* posn2 = GenVertexBuffer(normals, sizeof(normals));
    DescribedBuffer* posc2 = GenVertexBuffer(colors, sizeof(colors));
    VertexArray* vao = LoadVertexArray();

    VertexAttribPointer(vao, posb,  0, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posu,  1, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posn,  2, VertexFormat_Float32x3, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posc,  3, VertexFormat_Float32x4, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posb2, 0, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posu2, 1, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posn,  2, VertexFormat_Float32x3, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, posc2, 3, VertexFormat_Float32x4, 0, VertexStepMode_Vertex);

    RenderSettings settings zeroinit;

    settings.depthTest = 1;
    settings.depthCompare = CompareFunction_LessEqual;
    AttributeAndResidence attributes[4] = {
        AttributeAndResidence{.attr = VertexAttribute{.nextInChain = 0, .format = VertexFormat_Float32x2, .offset = 0, .shaderLocation = 0}, .bufferSlot = 0, .enabled = true},
        AttributeAndResidence{.attr = VertexAttribute{.nextInChain = 0, .format = VertexFormat_Float32x2, .offset = 0, .shaderLocation = 1}, .bufferSlot = 1, .enabled = true},
        AttributeAndResidence{.attr = VertexAttribute{.nextInChain = 0, .format = VertexFormat_Float32x3, .offset = 0, .shaderLocation = 2}, .bufferSlot = 2, .enabled = true},
        AttributeAndResidence{.attr = VertexAttribute{.nextInChain = 0, .format = VertexFormat_Float32x4, .offset = 0, .shaderLocation = 3}, .bufferSlot = 3, .enabled = true}
    };
    DescribedPipeline* pl = LoadPipeline(source);
    //DescribedPipeline* pl = LoadPipelineEx(source, attributes, 4, nullptr, 0, settings);
    //DescribedPipeline* pl = DefaultPipeline();
    //DescribedPipeline* pl = LoadPipelinePro(/*source, vao, nullptr, 0, settings*/);
    PreparePipeline(pl, vao);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLANK);
        UseNoTexture();
        BeginMode3D(cam);
        BeginPipelineMode(pl);
        BindPipelineVertexArray(pl, vao);
        DrawArrays(RL_TRIANGLES, 3);
        EndPipelineMode();
        EndMode3D();
        DrawFPS(0, 0);
        EndDrawing();
    }
}