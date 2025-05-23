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
    vec4 eye;
    vec4 target;
    vec4 up;
    vec4 fovY;
} camera;

// Ray payload - will be passed to closest hit or miss shader
layout(location = 0) rayPayloadEXT vec4 payload;

void main() {
    // Get the current pixel coordinate
    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2 d = inUV * 2.0 - 1.0;

    // Calculate ray origin and direction using camera matrices
    vec3 origin = camera.eye.xyz;
    vec3 target = camera.target.xyz;
    vec3 direction = normalize(target - origin);
    vec3 left = cross(normalize(camera.up.xyz), direction);
    vec3 realup = normalize(cross(direction, left));
    float factor = tan(camera.fovY.x * 0.5f);
    vec3 raydirection = normalize(direction + factor * d.x * left + factor * d.y * realup);

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
        raydirection.xyz,     // Ray direction
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
    payload = vec4(1.0,float(gl_InstanceID),0.0,1.0);
}
)";
constexpr char rmissSource[] = R"(#version 460
#extension GL_EXT_ray_tracing : require

// Ray payload
layout(location = 0) rayPayloadInEXT vec4 payload;

void main(){
    // Sky color based on ray direction
    vec3 dir = normalize(gl_WorldRayDirectionEXT);
    
    // Simple gradient for sky
    float t = 0.5 * (dir.y + 1.0);
    vec3 skyColor = mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t);
    
    // Write sky color to payload
    payload = vec4(skyColor, 1.0f);
})";
Matrix padCamera(Camera3D cam){
    Matrix ret zeroinit;
    ret.data[0] = cam.position.x;
    ret.data[1] = cam.position.y;
    ret.data[2] = cam.position.z;

    ret.data[4] = cam.target.x;
    ret.data[5] = cam.target.y;
    ret.data[6] = cam.target.z;

    ret.data[8] = cam.up.x;
    ret.data[9] = cam.up.y;
    ret.data[10] = cam.up.z;
    
    ret.data[12] = cam.fovy;
    ret.data[13] = cam.fovy;
    ret.data[14] = cam.fovy;

    return ret;
}
int main(){
    RequestAdapterType(SOFTWARE_RENDERER);
    InitWindow(800, 800, "HWRT");
    
    WGPUBottomLevelAccelerationStructureDescriptor blasdesc zeroinit;

    WGPUBufferDescriptor bdesc1 zeroinit;
    
    Camera3D cam{
        .position = Vector3{0,0,-4},
        .target = Vector3{0,0,0},
        .up = Vector3{0,1,0},
        .fovy = 1.f,
    };
    //Matrix persplookat[2] = {
    //    MatrixLookAt(cam.position, cam.target, cam.up),
    //    MatrixPerspective(cam.fovy * DEG2RAD, 1.0f, 0.01f, 100.0f),
    //};
    //
    //persplookat[0] = MatrixIdentity();//MatrixInvert(persplookat[0]);
    //persplookat[1] = MatrixIdentity();//MatrixInvert(persplookat[1]);
    float vertexData[9] = { 0,-1, 1,
        -1, 1, 1,
         1, 1, 1};
    bdesc1.size = sizeof(vertexData);
    bdesc1.usage = BufferUsage_MapWrite | BufferUsage_CopyDst | BufferUsage_ShaderDeviceAddress | BufferUsage_AccelerationStructureInput;
    WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bdesc1);
    
    wgpuQueueWriteBuffer((WGPUQueue)g_vulkanstate.queue, vertexBuffer, 0, vertexData, sizeof(vertexData));

    blasdesc.vertexBuffer = vertexBuffer;
    blasdesc.vertexCount = 3;
    blasdesc.vertexStride = 12;
    WGPUBottomLevelAccelerationStructure blas = wgpuDeviceCreateBottomLevelAccelerationStructure((WGPUDevice)GetDevice(), &blasdesc);
    
    WGPUTopLevelAccelerationStructureDescriptor tlasdesc zeroinit;
    WGPUBottomLevelAccelerationStructure blases[3] = {blas, blas, blas};
    tlasdesc.bottomLevelAS = blases;
    tlasdesc.blasCount = 3;
    VkTransformMatrixKHR matrix[3] zeroinit;
    for(uint32_t i = 0;i < 3;i++){
        matrix[i].matrix[0][0] = 1;
        matrix[i].matrix[1][1] = 1;
        matrix[i].matrix[2][2] = 1;
        matrix[i].matrix[0][3] = float(i);
    }
    
    tlasdesc.transformMatrices = matrix;
    
    WGPUTopLevelAccelerationStructure tlas = wgpuDeviceCreateTopLevelAccelerationStructure((WGPUDevice)GetDevice(), &tlasdesc);
    Texture2D storageTex = LoadTexturePro(50, 50, RGBA8, TextureUsage_StorageBinding | TextureUsage_CopySrc | TextureUsage_TextureBinding, 1, 1);
    
    Matrix camPadded = padCamera(cam);
    DescribedBuffer* uniformBuffer = GenBufferEx(&camPadded, sizeof(Matrix), BufferUsage_CopyDst | BufferUsage_Uniform);
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
    bgentries[1].textureView = (WGPUTextureView)storageTex.view;
    bgentries[1].binding = 1;
    bgentries[2].buffer = (WGPUBuffer)uniformBuffer->buffer;
    bgentries[2].binding = 2;
    bgentries[2].size = uniformBuffer->size;
    DescribedBindGroup rtbg = LoadBindGroup(&drtpl->bglayout, bgentries, 3);
    UpdateBindGroup(&rtbg);
    WGPURaytracingPipeline rtpl = drtpl->pipeline;
    WGPUCommandEncoderDescriptor cedecs zeroinit;
    WGPUCommandEncoder cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &cedecs);
    
    WGPURaytracingPassEncoder rtEncoder = wgpuCommandEncoderBeginRaytracingPass(cmdEncoder);
    wgpuRaytracingPassEncoderSetPipeline(rtEncoder, rtpl);
    wgpuRaytracingPassEncoderSetBindGroup(rtEncoder, 0, (WGPUBindGroup)rtbg.bindGroup);
    wgpuRaytracingPassEncoderTraceRays(rtEncoder, 50, 50, 1);
    WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(cmdEncoder);
    
    //vkCmdBindPipeline(cmdEncoder->buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rtpl->raytracingPipeline);
    //vkCmdBindDescriptorSets(cmdEncoder->buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, (VkPipelineLayout)drtpl->layout, 0, 1, &reinterpret_cast<WGPUBindGroup>(rtbg.bindGroup)->set, 0, nullptr);
    
    

    //vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData) s;
    while(!WindowShouldClose()){
        BeginDrawing();
        if(GetFrameCount() == 0){
            wgpuQueueSubmit(g_vulkanstate.queue, 1, &cmdBuffer);
            wgpuReleaseCommandEncoder(cmdEncoder);
            wgpuReleaseCommandBuffer(cmdBuffer);
        }
        ClearBackground(DARKGRAY);
        DrawFPS(5, 5);
        DrawTexturePro(storageTex, Rectangle{0,0,(float)storageTex.width, (float)storageTex.height}, Rectangle{100,100,400,400}, Vector2{0, 0}, 0.0f, WHITE);
        EndDrawing();
    }
}


