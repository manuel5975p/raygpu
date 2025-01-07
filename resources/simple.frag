#version 430 core
//#extension clip_distances: disable

layout(location=0) out vec4 outColor;
layout(binding = 0)uniform sampler2D colDiffuse;
void main() {
    outColor = texture(colDiffuse, vec2(0.0f,0.0f)) * vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
}
