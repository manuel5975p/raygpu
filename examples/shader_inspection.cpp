#include <raygpu.h>
#include <internals.hpp>
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
constexpr char shaderSourceWGSL[] = R"(
    struct VertexInput {
        @location(0) position: vec3f,
        @location(1) uv: vec2f,
        @location(2) normal: vec3f,
        @location(3) color: vec4f,
    };
    
    struct VertexOutput {
        @builtin(position) position: vec4f,
        @location(0) uv: vec2f,
        @location(1) color: vec4f,
    };
    
    struct LightBuffer {
        count: u32,
        positions: array<vec3f>
    };
    
    @group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
    @group(0) @binding(1) var texture0: texture_2d<f32>;
    @group(0) @binding(2) var texSampler: sampler;
    @group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
    @group(0) @binding(4) var<storage> lights: LightBuffer;
    @group(0) @binding(5) var<storage> lights2: LightBuffer;
    
    //Can be omitted
    //@group(0) @binding(3) var<storage> storig: array<vec4f>;
    
    
    @vertex
    fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
        var out: VertexOutput;
        out.position = Perspective_View * 
                       modelMatrix[instanceIdx] *
        vec4f(in.position.xyz, 1.0f);
        out.color = in.color;
        out.uv = in.uv;
        return out;
    }
    
    @fragment
    fn fs_main(in: VertexOutput) -> @location(0) vec4f {
        return textureSample(texture0, texSampler, in.uv).rgba * in.color;
    }
    )";


int main(){
    InitWindow(800, 800, "as");
    ShaderSources sources zeroinit;
    sources.language = sourceTypeGLSL;
    sources.sourceCount = 2;
    sources.sources[0].data = vertexSourceGLSL;
    sources.sources[0].sizeInBytes = std::strlen(vertexSourceGLSL);
    sources.sources[0].stageMask = ShaderStageMask_Vertex;
    
    sources.sources[1].data = fragmentSourceGLSL;
    sources.sources[1].sizeInBytes = std::strlen(fragmentSourceGLSL);
    sources.sources[1].stageMask = ShaderStageMask_Fragment;
    
    ShaderSources wgslSources zeroinit;
    wgslSources.language = sourceTypeWGSL;
    wgslSources.sourceCount = 1;
    sources.sources[0].data = shaderSourceWGSL;
    sources.sources[0].sizeInBytes = std::strlen(shaderSourceWGSL);
    sources.sources[0].stageMask = ShaderStageMask(ShaderStageMask_Vertex | ShaderStageMask_Fragment);
    //sources.computeSource = computeSource;
    auto comp = getBindingsGLSL(sources);
    auto wgcomp = getBindings(wgslSources);
    for(auto [n, l] : wgcomp){
        std::cout << n << ": " << l.location << ", " << l.type <<  "\n";
    }
    exit(0);
    while(!WindowShouldClose()){
        BeginDrawing();
        EndDrawing();
    }
    
}
