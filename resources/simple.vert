#version 430 core
//#extension clip_distances: disable

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec2 uv;
void main() {
    uv = abs(inPosition.xy);
    gl_Position = vec4(inPosition, 1.0);
}

