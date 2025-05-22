#version 450

// Input attributes.
layout(location = 0) in vec3 in_position;  // position
layout(location = 1) in vec2 in_uv;        // texture coordinates
layout(location = 2) in vec3 in_normal;    // normal (unused)
layout(location = 3) in vec4 in_color;     // vertex color

// Outputs to fragment shader.
layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec4 frag_color;
layout(location = 2) out vec3 out_normal;
// Uniform block for Perspective_View matrix (binding = 0).
layout(binding = 0) uniform Perspective_View {
    mat4 pvmatrix;
};

// Storage buffer for model matrices (binding = 3).
// Note: 'buffer' qualifier makes it a shader storage buffer.
layout(binding = 3) readonly buffer modelMatrix {
    mat4 modelMatrices[];  // Array of model matrices.
};

void main() {
    gl_PointSize = 1.0f;
    
    // Compute transformed position using instance-specific model matrix.
    gl_Position = pvmatrix * modelMatrices[gl_InstanceIndex] * vec4(in_position, 1.0);
    //gl_Position = vec4(0.001f * vec4(in_position, 1.0).xy - vec2(1, 1), 0, 1);
    //gl_Position = vec4(in_position, 1.0);
    frag_uv = in_uv;
    frag_color = in_color;
    out_normal = in_normal;
}