#include <external/VmaUsage.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <raygpu.h>
void zerotexture(RenderTexture* tex){
    *tex = (RenderTexture){0};
}
int main(){
    VmaVirtualBlockCreateInfo blockCreateInfo = {0};
    blockCreateInfo.size = (VkDeviceSize)1500;
    VmaVirtualBlock block;
    VkResult res = vmaCreateVirtualBlock(&blockCreateInfo, &block);
    VmaVirtualAllocationCreateInfo allocCreateInfo = {0};
    allocCreateInfo.size = 500;
    VmaVirtualAllocation alloc = {0};
    VmaVirtualAllocation alloc2 = {0};
    VmaVirtualAllocation alloc3 = {0};
    VmaVirtualAllocation alloc4 = {0};
    VmaVirtualAllocation alloc5 = {0};
    VkDeviceSize offset = 0;
    res = vmaVirtualAllocate(block, &allocCreateInfo, &alloc, &offset);
    res = vmaVirtualAllocate(block, &allocCreateInfo, &alloc2, &offset);
    if(res != VK_SUCCESS){
        printf("Allocation failed: %s\n", vkErrorString(res));
        fflush(stdout);
        abort();
    }
    vmaVirtualFree(block, alloc);
    res = vmaVirtualAllocate(block, &allocCreateInfo, &alloc3, &offset);
    printf("Allocated %" PRIu64 " bytes at offset %" PRIu64 "\n", allocCreateInfo.size, offset);
    res = vmaVirtualAllocate(block, &allocCreateInfo, &alloc4, &offset);
    if(res != VK_SUCCESS){
        printf("Allocation failed: %s\n", vkErrorString(res));
        fflush(stdout);
        abort();
    }
    printf("Allocated %" PRIu64 " bytes at offset %" PRIu64 "\n", allocCreateInfo.size, offset);
}