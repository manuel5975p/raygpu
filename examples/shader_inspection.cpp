#include <raygpu.h>
#include <internals.hpp>
#include <external/gl_corearb.h>
const char* computeSource = R"(
#version 450 core

layout(binding = 0) uniform texture2D texture0;
layout(binding = 1) uniform sampler texSampler;  

layout(binding = 2) uniform DummyUniformBuffer {
    float udata[];
}a;

layout(std430, binding = 3) writeonly buffer DummyOutputBuffer {
    float data[];
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    data[0] += texture(sampler2D(texture0, texSampler), vec2(0, 0)).x + a.udata[0];
}
)";



constexpr char vertexSourceGLSL[] = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable
// Input attributes.
layout(location = 0) in vec3 in_position;  // position
layout(location = 1) in vec2 in_uv;        // texture coordinates
layout(location = 2) in vec3 in_normal;    // normal (unused)
layout(location = 3) in vec4 in_color;     // vertex color

// Outputs to fragment shader.
layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec4 frag_color;

// Uniform block for Perspective_View matrix (binding = 0).
layout(binding = 0) uniform PerspectiveViewBlock {
    mat4 Perspective_View;
};

// Storage buffer for model matrices (binding = 3).
// Note: 'buffer' qualifier makes it a shader storage buffer.
layout(binding = 3) readonly buffer ModelMatrixBlock {
    mat4 modelMatrix[];  // Array of model matrices.
};

void main() {
    gl_PointSize = 1.0f;
    
    // Compute transformed position using instance-specific model matrix.
    gl_Position = Perspective_View * modelMatrix[gl_InstanceIndex] * vec4(in_position, 1.0);
    //gl_Position = vec4(in_position, 1.0);
    frag_uv = in_uv;
    frag_color = in_color;
}
)";

constexpr char fragmentSourceGLSL[] = R"(
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
    //outColor = vec4(frag_color.xyz,1);
}
)";



int main(){
    ShaderSources sources zeroinit;
    sources.computeSource = computeSource;
    //sources.vertexSource = vertexSourceGLSL;
    //sources.fragmentSource = fragmentSourceGLSL;
    
    auto comp = getBindingsGLSL(sources);
    for(auto [n, l] : comp){
        std::cout << n << ": " << l.location << ", " << l.type <<  "\n";
    }
}