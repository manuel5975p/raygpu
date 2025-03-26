#include <raygpu.h>
#include <wgvk.h>
#include "../src/backend_vulkan/vulkan_internals.hpp"
//#include "../src/backend_vulkan/vulkan_internals.hpp"
constexpr const char raygenSource[] = R"(#version 460
#extension GL_EXT_ray_tracing : require

// Binding for acceleration structure
layout(binding = 0) uniform accelerationStructureEXT topLevelAS;
// Output image
layout(binding = 1, rgba8) uniform image2D image;
// Camera uniform buffer
layout(binding = 2) uniform CameraProperties {
    mat4 viewInverse;
    mat4 projInverse;
} camera;

// Ray payload - will be passed to closest hit or miss shader
layout(location = 0) rayPayloadEXT vec4 payload;

void main() {
    // Get the current pixel coordinate
    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2 d = inUV * 2.0 - 1.0;

    // Calculate ray origin and direction using camera matrices
    vec4 origin = camera.viewInverse * vec4(0, 0, 0, 1);
    vec4 target = camera.projInverse * vec4(d.x, d.y, 1, 1);
    vec4 direction = camera.viewInverse * vec4(normalize(target.xyz), 0);

    payload = vec4(target.yx, 0.3f, 1);
    // Initialize payload

    // Trace ray
    traceRayEXT(
        topLevelAS,           // Acceleration structure
        gl_RayFlagsOpaqueEXT, // Ray flags
        0xFF,                 // Cull mask
        0,                    // sbtRecordOffset
        0,                    // sbtRecordStride
        0,                    // missIndex
        origin.xyz,           // Ray origin
        0.001,                // Min ray distance
        direction.xyz,        // Ray direction
        100.0,                // Max ray distance
        0                     // Payload location
    );
    
    // Write result to output image
    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(payload.xyz, 1.0f));
    //imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(vec2(gl_LaunchIDEXT.xy * 0.01f),1,1));
}
)";
constexpr char rchitSource[] = R"(#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

// Bind scene descriptors
//layout(binding = 3) buffer SceneDesc { vec4 data[]; } sceneDesc;
//layout(binding = 4) uniform sampler2D textures[];

// Ray payload
layout(location = 0) rayPayloadInEXT vec4 payload;

// Hit attributes from intersection
hitAttributeEXT vec2 attribs;

// Shader record buffer index
//layout(binding = 4) uniform _ShaderRecordBuffer {
//    int materialID;
//} shaderRecordBuffer;

void main(){
    // Basic surface color (replace with your material system)
    vec3 hitColor = vec3(0.7, 0.7, 0.7);
    
    // Get hit triangle vertices
    int primitiveID = gl_PrimitiveID;
    int materialID = 0;//shaderRecordBuffer.materialID;
    
    // Simple diffuse shading based on normal
    vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    
    // Calculate surface normal using barycentric coordinates
    // (In a real implementation, you would use vertex data)
    vec3 normal = normalize(vec3(0, 1, 0)); // Simplified normal
    
    // Direction to light (hardcoded for simplicity)
    vec3 lightDir = normalize(vec3(1, 1, 1));
    
    // Simple diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.2);
    
    // Set final color
    //payload = vec4(hitColor * diffuse, 1.0);
    payload = vec4(1.0,0.0,0.0,1.0);
}
)";
constexpr char rmissSource[] = R"(#version 460
#extension GL_EXT_ray_tracing : require

// Ray payload
layout(location = 0) rayPayloadEXT vec4 payload;

void main(){
    // Sky color based on ray direction
    vec3 dir = normalize(gl_WorldRayDirectionEXT);
    
    // Simple gradient for sky
    float t = 0.5 * (dir.y + 1.0);
    vec3 skyColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t);
    
    // Write sky color to payload
    payload = vec4(0, 1.0f, 1.0f, 1.0f);
})";

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
    
    WGVKTopLevelAccelerationStructure tlas = wgvkDeviceCreateTopLevelAccelerationStructure((WGVKDevice)GetDevice(), &tlasdesc);
    Texture2D storageTex = LoadTexturePro(50, 50, RGBA8, TextureUsage_StorageBinding | TextureUsage_CopySrc | TextureUsage_TextureBinding, 1, 1);
    
    Matrix inverseCamMatrices[2] = {MatrixIdentity(), MatrixIdentity()};

    DescribedBuffer* uniformBuffer = GenBufferEx(inverseCamMatrices, sizeof(Matrix) * 2, BufferUsage_CopyDst | BufferUsage_Uniform);
    ShaderSources sources zeroinit;
    sources.language = sourceTypeGLSL;
    sources.sourceCount = 3;
    sources.sources[0].data = raygenSource;
    sources.sources[0].sizeInBytes = sizeof(raygenSource) - 1;
    sources.sources[0].stageMask = ShaderStageMask_RayGen;

    sources.sources[1].data = rchitSource;
    sources.sources[1].sizeInBytes = sizeof(rchitSource) - 1;
    sources.sources[1].stageMask = ShaderStageMask_ClosestHit;

    sources.sources[2].data = rmissSource;
    sources.sources[2].sizeInBytes = sizeof(rmissSource) - 1;
    sources.sources[2].stageMask = ShaderStageMask_Miss;

    DescribedShaderModule rt_module = LoadShaderModule(sources);

    DescribedRaytracingPipeline* drtpl = LoadRaytracingPipeline(&rt_module);

    ResourceDescriptor bgentries[3] zeroinit;
    bgentries[0].accelerationStructure = tlas;
    bgentries[0].binding = 0;
    bgentries[1].textureView = (WGVKTextureView)storageTex.view;
    bgentries[1].binding = 1;
    bgentries[2].buffer = (WGVKBuffer)uniformBuffer->buffer;
    bgentries[2].binding = 2;
    bgentries[2].size = 128;
    DescribedBindGroup rtbg = LoadBindGroup(&drtpl->bglayout, bgentries, 3);
    UpdateBindGroup(&rtbg);
    WGVKRaytracingPipeline rtpl = drtpl->pipeline;
    WGVKCommandEncoderDescriptor cedecs zeroinit;
    WGVKCommandEncoder cmdEncoder = wgvkDeviceCreateCommandEncoder((WGVKDevice)GetDevice(), &cedecs);
    
    WGVKRaytracingPassEncoder rtEncoder = wgvkCommandEncoderBeginRaytracingPass(cmdEncoder);
    wgvkRaytracingPassEncoderSetPipeline(rtEncoder, rtpl);
    wgvkRaytracingPassEncoderSetBindGroup(rtEncoder, 0, (WGVKBindGroup)rtbg.bindGroup);
    wgvkRaytracingPassEncoderTraceRays(rtEncoder, 50, 50, 1);
    WGVKCommandBuffer cmdBuffer = wgvkCommandEncoderFinish(cmdEncoder);
    
    //vkCmdBindPipeline(cmdEncoder->buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtpl->raytracingPipeline);
    //vkCmdBindDescriptorSets(cmdEncoder->buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, (VkPipelineLayout)drtpl->layout, 0, 1, &reinterpret_cast<WGVKBindGroup>(rtbg.bindGroup)->set, 0, nullptr);
    
    

    //vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData) s;
    while(!WindowShouldClose()){
        BeginDrawing();
        if(GetFrameCount() == 0){
            wgvkQueueSubmit(g_vulkanstate.queue, 1, &cmdBuffer);
            wgvkReleaseCommandEncoder(cmdEncoder);
            wgvkReleaseCommandBuffer(cmdBuffer);
        }
        ClearBackground(DARKGRAY);
        DrawFPS(5, 5);
        DrawTexturePro(storageTex, Rectangle{0,0,(float)storageTex.width, (float)storageTex.height}, Rectangle{100,100,400,400}, Vector2{0, 0}, 0.0f, WHITE);
        EndDrawing();
    }
}


