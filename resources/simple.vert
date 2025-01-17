#version 430 core
//#extension clip_distances: disable

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec2 uv;
layout(location = 1) out vec4 somethingelse;
void main() {
    somethingelse = vec4(1,0.5,0.2,1);
    uv = abs(vec2(inPosition.x, inPosition.y));
    gl_Position = vec4(inPosition, 1.0);
}

