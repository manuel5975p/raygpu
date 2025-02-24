#include <raygpu.h>
#include <internals.hpp>
const char* computeSource = R"(
#version 450 core
#extension GL_EXT_samplerless_texture_functions : enable
layout(binding = 1) uniform texture2D texture0;


//layout(binding = 0) uniform mat4 exposure2;
//layout(binding = 2) uniform sampler2D modelToWorldMatrix;
//layout (std140, binding = 5) uniform imput{
//    float inExposure;
//};

layout(std430, binding = 7) writeonly buffer ExposureBuffer {
    mat4 exposure;
    float data[];
};
layout(binding = 4) uniform UniformExposureBuffer {
    mat4 uniformExposure;
};
layout(local_size_x = 32, local_size_y = 32, local_size_z = 32) in;
void main() {
    exposure[0][0] = texelFetch(texture0, ivec2(0,0), 0).x;
    exposure += uniformExposure;
    //data[0] = texelFetch(texture0, ivec2(0,0), 0).x;
}
)";
int main(){
    ShaderSources sources zeroinit;
    sources.computeSource = computeSource;
    auto comp = getBindingsGLSL(sources);
    for(auto [n, l] : comp){
        std::cout << n << ": " << l.location << "\n";
    }
}