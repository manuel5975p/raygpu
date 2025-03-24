#include <raygpu.h>
#include <wgvk.h>
//#include "../src/backend_vulkan/vulkan_internals.hpp"
int main(){
    RequestAdapterType(SOFTWARE_RENDERER);
    InitWindow(800, 800, "HWRT");
    
    WGVKBottomLevelAccelerationStructureDescriptor blasdesc zeroinit;

    WGVKBufferDescriptor bdesc1 zeroinit;
    bdesc1.size = 12;
    bdesc1.usage = BufferUsage_CopyDst | BufferUsage_ShaderDeviceAddress;
    WGVKBuffer vertexBuffer = wgvkDeviceCreateBuffer((WGVKDevice)GetDevice(), &bdesc1);
    blasdesc.vertexBuffer = vertexBuffer;
    blasdesc.vertexCount = 3;
    blasdesc.vertexStride = 4;
    WGVKBottomLevelAccelerationStructure blas = wgvkDeviceCreateBottomLevelAccelerationStructure((WGVKDevice)GetDevice(), &blasdesc);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(DARKGRAY);
        DrawFPS(5, 5);
        EndDrawing();
    }
}
