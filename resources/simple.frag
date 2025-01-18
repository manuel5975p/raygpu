#version 430 core
//#extension clip_distances: disable

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec4 somethingelse;
layout(binding = 0) uniform texture2D tex0;
layout(binding = 1) uniform sampler smp0;
void main() {
    //outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
    outColor = texture(sampler2D(tex0, smp0), uv); // Texture sample
}
