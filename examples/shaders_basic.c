#include <raygpu.h>
const char fragSourceGLSL[] = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable  // Enable separate sampler objects if needed

// Inputs from vertex shader.
layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec4 frag_color;

// Output fragment color.
layout(location = 0) out vec4 outColor;

// Texture and sampler, bound separately.
layout(binding = 1) uniform texture2D texture0;  // Texture (binding = 1)
layout(binding = 2) uniform sampler texSampler;    // Sampler (binding = 2)

void main() {
    // Sample the texture using the combined sampler.
    vec4 texColor = texture(sampler2D(texture0, texSampler), frag_uv);
    outColor = texColor * frag_color;
}
)";
int main(){
    InitWindow(800, 800, "Shaders example");
    Shader colorInverter = LoadShaderFromMemory(NULL, fragSourceGLSL);
    
    Matrix matrix = GetMatrix();

    DescribedBuffer* matrixbuffer = GenUniformBuffer(&matrix, sizeof(Matrix));
    DescribedBuffer* matrixbuffers = GenStorageBuffer(&matrix, sizeof(Matrix));
    Texture tex = GetDefaultTexture();
    DescribedSampler sampler = LoadSampler(repeat, filter_linear);

   

    while(!WindowShouldClose()){
        BeginDrawing();
        BeginPipelineMode(colorInverter.id);
        SetPipelineUniformBuffer(colorInverter.id, 0, matrixbuffer);
        SetPipelineTexture(colorInverter.id, 1, tex);
        SetPipelineSampler(colorInverter.id, 2, sampler);
        SetPipelineStorageBuffer(colorInverter.id, 3, matrixbuffers);
        DrawRectangle(200,200,200,200,RED);
        EndPipelineMode();
        EndDrawing();
    }
}