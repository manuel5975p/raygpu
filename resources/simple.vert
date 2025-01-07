#version 430 core
//#extension clip_distances: disable

layout(location = 0) in vec3 inPosition;
void main() {
    gl_Position = vec4(inPosition, 1.0);
}

