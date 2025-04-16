#include <raygpu.h>
const char vertexSourceGLSL[] = R"(
#version 450
layout(location = 1) in vec2 position;

void main() {
    gl_Position = vec4(position, 0, 1);
}
)";
const char fragmentSourceGLSL[] = R"(
#version 450
layout(location = 0) out vec4 fragColor;
layout(binding = 5) uniform color {
    vec4 colorval;
};
void main() {
    fragColor = colorval;
}
)";
int main(){
    InitWindow(800, 800, "Shaders example");
    Shader colorInverter = LoadShaderFromMemory(vertexSourceGLSL, fragmentSourceGLSL);

    Matrix matrix = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    Matrix identity = MatrixIdentity();
    DescribedBuffer* matrixbuffer = GenUniformBuffer(&matrix, sizeof(Matrix));
    DescribedBuffer* matrixbuffers = GenStorageBuffer(&identity, sizeof(Matrix));
    
    Texture tex = LoadTexture(TextFormat("%s/tileset.png",FindDirectory("resources", 3)));
    DescribedSampler sampler = LoadSampler(repeat, filter_linear);
    
    float vertices[6] = {0,-0.5,-0.5,0.5,0.5,0.5};
    DescribedBuffer* buffer = GenVertexBuffer(vertices, sizeof(vertices));
    VertexArray* vao = LoadVertexArray();
    VertexAttribPointer(vao, buffer, 1, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    EnableVertexAttribArray(vao, 0);
    float color[4] = {1,1,1,1};
    SetPipelineUniformBufferData(colorInverter.id, 5, color, sizeof(color));
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(BLACK);
        BeginPipelineMode(colorInverter.id);
        BindVertexArray(vao);
        DrawArrays(RL_TRIANGLES, 3);
        EndPipelineMode();
        DrawFPS(5, 5);
        EndDrawing();
    }
}