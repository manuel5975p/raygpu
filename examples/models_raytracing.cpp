#include <raygpu.h>
#include <wgvk.h>
//#include "../src/backend_vulkan/vulkan_internals.hpp"
constexpr const char raygenSource[] = R"(
#version 460 core
#extension GL_EXT_ray_tracing : enable
layout(location = 0) rayPayloadEXT vec4 payload;
layout(binding = 0, set = 0) uniform accelerationStructureEXT acc;
layout(binding = 1, rgba32f) uniform image2D img;
layout(binding = 1, set = 0) uniform rayParams
{
    vec3 rayOrigin;
    vec3 rayDir;
    uint sbtOffset;
    uint sbtStride;
    uint missIndex;
};
void main() {
    vec3 dire = vec3(0,0,1);
    traceRayEXT(acc, gl_RayFlagsOpaqueEXT, 0xff, sbtOffset,
                sbtStride, missIndex, rayOrigin, 0.0,
                dire,
                100.0f, 0 /* payload */);
    vec4 imgColor = payload;// + vec4(blendColor) ;
    imageStore(img, ivec2(gl_LaunchIDEXT), payload);
}
)";
int main(){
    RequestAdapterType(SOFTWARE_RENDERER);
    InitWindow(800, 800, "HWRT");
    
    WGVKBottomLevelAccelerationStructureDescriptor blasdesc zeroinit;

    WGVKBufferDescriptor bdesc1 zeroinit;
    bdesc1.size = 12;
    bdesc1.usage = BufferUsage_CopyDst | BufferUsage_ShaderDeviceAddress | BufferUsage_AccelerationStructureInput;
    WGVKBuffer vertexBuffer = wgvkDeviceCreateBuffer((WGVKDevice)GetDevice(), &bdesc1);
    blasdesc.vertexBuffer = vertexBuffer;
    blasdesc.vertexCount = 3;
    blasdesc.vertexStride = 4;
    WGVKBottomLevelAccelerationStructure blas = wgvkDeviceCreateBottomLevelAccelerationStructure((WGVKDevice)GetDevice(), &blasdesc);
    
    WGVKTopLevelAccelerationStructureDescriptor tlasdesc zeroinit;
    tlasdesc.bottomLevelAS = &blas;
    tlasdesc.blasCount = 1;
    VkTransformMatrixKHR matrix zeroinit;
    matrix.matrix[0][0] = 1;
    matrix.matrix[1][1] = 1;
    matrix.matrix[2][2] = 1;
    tlasdesc.transformMatrices = &matrix;
    wgvkDeviceCreateTopLevelAccelerationStructure((WGVKDevice)GetDevice(), &tlasdesc);
    
    ShaderSources sources zeroinit;
    sources.language = sourceTypeGLSL;
    sources.sourceCount = 1;
    sources.sources[0].data = raygenSource;
    sources.sources[0].sizeInBytes = sizeof(raygenSource) - 1;
    sources.sources[0].stageMask = ShaderStageMask_RayGen;
    DescribedShaderModule rt_module = LoadShaderModule(sources);

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawFPS(5, 5);
        EndDrawing();
    }
}
