#include "vulkan_internals.hpp"
#include <macros_and_constants.h>
#include <vulkan/vulkan.h>
#include <wgvk.h>

#define RTFunctions \
X(CreateRayTracingPipelinesKHR) \
X(CmdBuildAccelerationStructuresKHR) \
X(GetAccelerationStructureBuildSizesKHR) \
X(CreateAccelerationStructureKHR) \
X(DestroyAccelerationStructureKHR) \
X(GetAccelerationStructureDeviceAddressKHR) \
X(GetRayTracingShaderGroupHandlesKHR)


#define X(A) PFN_vk##A fulk##A;
RTFunctions
#undef X
extern "C" void raytracing_LoadDeviceFunctions(VkDevice device){
    #define X(A) fulk##A = (PFN_vk##A)vkGetDeviceProcAddr(device, "vk" #A);
    RTFunctions
    #undef X
}

struct triangle {
    vertex v1, v2, v3;
};

typedef struct WGVKBottomLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
} WGVKBottomLevelAccelerationStructureImpl;
typedef WGVKBottomLevelAccelerationStructureImpl *WGVKBottomLevelAccelerationStructure;

typedef struct WGVKTopLevelAccelerationStructureImpl {
    VkDevice device;
    VkAccelerationStructureKHR accelerationStructure;
    VkDeviceMemory accelerationStructureMemory;
    VkBuffer scratchBuffer;
    VkDeviceMemory scratchBufferMemory;
    VkBuffer accelerationStructureBuffer;
    VkDeviceMemory accelerationStructureBufferMemory;
    VkBuffer instancesBuffer;
    VkDeviceMemory instancesBufferMemory;
} WGVKTopLevelAccelerationStructureImpl;

/**
 * Creates a Vulkan top level acceleration structure for ray tracing
 *
 * Encapsulates instance data and references to BLASes to enable scene traversal
 */
extern "C" WGVKTopLevelAccelerationStructure wgvkDeviceCreateTopLevelAccelerationStructure(WGVKDevice device, const WGVKTopLevelAccelerationStructureDescriptor *descriptor) {
    WGVKTopLevelAccelerationStructureImpl *impl = callocnewpp(WGVKTopLevelAccelerationStructureImpl);
    impl->device = device->device;

    const size_t cacheIndex = device->submittedFrames % framesInFlight;
    // Get acceleration structure properties
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties2 zeroinit;
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &accelerationStructureProperties;

    vkGetPhysicalDeviceProperties2(device->adapter->physicalDevice, &deviceProperties2);

    // Create instance buffer to hold instance data
    const size_t instanceSize = sizeof(VkAccelerationStructureInstanceKHR);
    const VkDeviceSize instanceBufferSize = instanceSize * descriptor->blasCount;

    VkBufferCreateInfo instanceBufferCreateInfo = {};
    instanceBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    instanceBufferCreateInfo.size = instanceBufferSize;
    instanceBufferCreateInfo.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    vkCreateBuffer(device->device, &instanceBufferCreateInfo, nullptr, &impl->instancesBuffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->device, impl->instancesBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(device->adapter->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    allocateInfo.pNext = &memoryAllocateFlagsInfo;

    vkAllocateMemory(device->device, &allocateInfo, nullptr, &impl->instancesBufferMemory);
    vkBindBufferMemory(device->device, impl->instancesBuffer, impl->instancesBufferMemory, 0);

    // Map memory and create instances
    VkAccelerationStructureInstanceKHR *instanceData;
    vkMapMemory(device->device, impl->instancesBufferMemory, 0, instanceBufferSize, 0, (void **)&instanceData);

    for (uint32_t i = 0; i < descriptor->blasCount; i++) {

        // Get the device address of the BLAS

        VkAccelerationStructureDeviceAddressInfoKHR addressInfo zeroinit;
        addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        addressInfo.accelerationStructure = descriptor->bottomLevelAS[i]->accelerationStructure;
        uint64_t blasAddress = fulkGetAccelerationStructureDeviceAddressKHR(device->device, &addressInfo);

        VkAccelerationStructureInstanceKHR &instance = instanceData[i];
        // Set transform matrix (identity if not provided)
        if (descriptor->transformMatrices) {
            std::memcpy(&instance.transform, &descriptor->transformMatrices[i], sizeof(VkTransformMatrixKHR));
        } else {
            // Identity matrix
            std::memset(&instance.transform, 0, sizeof(VkTransformMatrixKHR));
            instance.transform.matrix[0][0] = 1.0f;
            instance.transform.matrix[1][1] = 1.0f;
            instance.transform.matrix[2][2] = 1.0f;
        }

        // Set instance data
        instance.instanceCustomIndex = descriptor->instanceCustomIndexes ? descriptor->instanceCustomIndexes[i] : 0;
        instance.mask = 0xFF; // Visible to all ray types by default
        instance.instanceShaderBindingTableRecordOffset = descriptor->instanceShaderBindingTableRecordOffsets ? descriptor->instanceShaderBindingTableRecordOffsets[i] : 0;
        instance.flags = descriptor->instanceFlags ? descriptor->instanceFlags[i] : VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        instance.accelerationStructureReference = blasAddress;
    }

    vkUnmapMemory(device->device, impl->instancesBufferMemory);

    // Create acceleration structure geometry
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureGeometryInstancesDataKHR instancesData = {};
    instancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instancesData.arrayOfPointers = VK_FALSE;

    VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = impl->instancesBuffer;
    instancesData.data.deviceAddress = vkGetBufferDeviceAddress(device->device, &bufferDeviceAddressInfo);

    accelerationStructureGeometry.geometry.instances = instancesData;

    // Build ranges
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {};
    buildRangeInfo.primitiveCount = descriptor->blasCount; // Number of instances
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;

    // Create acceleration structure build geometry info
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &accelerationStructureGeometry;

    // Get required build sizes
    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    fulkGetAccelerationStructureBuildSizesKHR(device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &buildRangeInfo.primitiveCount, &buildSizesInfo);

    // Create buffer for acceleration structure
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizesInfo.accelerationStructureSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vkCreateBuffer(device->device, &bufferCreateInfo, nullptr, &impl->accelerationStructureBuffer);

    vkGetBufferMemoryRequirements(device->device, impl->accelerationStructureBuffer, &memoryRequirements);

    VkMemoryAllocateInfo asAllocateInfo = {};
    asAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    asAllocateInfo.allocationSize = memoryRequirements.size;
    asAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    
    VkMemoryAllocateFlagsInfoKHR infoKhr zeroinit;
    infoKhr.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
    infoKhr.flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    asAllocateInfo.pNext = &infoKhr;

    vkAllocateMemory(device->device, &asAllocateInfo, nullptr, &impl->accelerationStructureBufferMemory);
    vkBindBufferMemory(device->device, impl->accelerationStructureBuffer, impl->accelerationStructureBufferMemory, 0);

    // Create the acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = impl->accelerationStructureBuffer;
    createInfo.offset = 0;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    fulkCreateAccelerationStructureKHR(device->device, &createInfo, nullptr, &impl->accelerationStructure);

    // Create scratch buffer
    VkBufferCreateInfo scratchBufferCreateInfo = {};
    scratchBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    scratchBufferCreateInfo.size = buildSizesInfo.buildScratchSize;
    scratchBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vkCreateBuffer(device->device, &scratchBufferCreateInfo, nullptr, &impl->scratchBuffer);

    vkGetBufferMemoryRequirements(device->device, impl->scratchBuffer, &memoryRequirements);

    VkMemoryAllocateInfo scratchBufferAllocateInfo = {};
    scratchBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    scratchBufferAllocateInfo.allocationSize = memoryRequirements.size;
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateFlagsInfo scratchMemFlags = {};
    scratchMemFlags.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    scratchMemFlags.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    scratchBufferAllocateInfo.pNext = &scratchMemFlags;

    vkAllocateMemory(device->device, &scratchBufferAllocateInfo, nullptr, &impl->scratchBufferMemory);
    vkBindBufferMemory(device->device, impl->scratchBuffer, impl->scratchBufferMemory, 0);

    // Update build geometry info with scratch buffer
    VkBufferDeviceAddressInfo scratchAddressInfo = {};
    scratchAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    scratchAddressInfo.buffer = impl->scratchBuffer;
    buildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(device->device, &scratchAddressInfo);
    buildGeometryInfo.dstAccelerationStructure = impl->accelerationStructure;

    // Build the acceleration structure
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = device->frameCaches[cacheIndex].commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->device, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;
    vkGetDeviceProcAddr(device->device, "vkCmdBuildAccelerationStructuresKHR");

    fulkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, &pBuildRangeInfo);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->queue->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    return impl;
}

/**
 * Destroys and cleans up top level acceleration structure resources
 *
 * Releases all memory and destroys Vulkan objects associated with the TLAS
 */
void wgvkDestroyTopLevelAccelerationStructure(WGVKTopLevelAccelerationStructureImpl *impl) {
    if (!impl)
        return;

    fulkDestroyAccelerationStructureKHR(impl->device, impl->accelerationStructure, nullptr);
    vkDestroyBuffer(impl->device, impl->accelerationStructureBuffer, nullptr);
    vkFreeMemory(impl->device, impl->accelerationStructureBufferMemory, nullptr);
    vkDestroyBuffer(impl->device, impl->scratchBuffer, nullptr);
    vkFreeMemory(impl->device, impl->scratchBufferMemory, nullptr);
    vkDestroyBuffer(impl->device, impl->instancesBuffer, nullptr);
    vkFreeMemory(impl->device, impl->instancesBufferMemory, nullptr);

    std::free(impl);
}

/**
 * Gets the device address of an acceleration structure
 *
 * Required for shader table binding to reference the acceleration structure
 */
uint64_t wgvkGetAccelerationStructureDeviceAddress(VkDevice device, VkAccelerationStructureKHR accelerationStructure) {
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
    addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.accelerationStructure = accelerationStructure;

    return fulkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
}

/**
 * Creates a Vulkan acceleration structure for ray tracing
 *
 * Manages memory allocation and buffer creation needed for BLAS creation
 */

extern "C" WGVKBottomLevelAccelerationStructure wgvkDeviceCreateBottomLevelAccelerationStructure(WGVKDevice device, const WGVKBottomLevelAccelerationStructureDescriptor *descriptor) {
    WGVKBottomLevelAccelerationStructureImpl *impl = callocnewpp(WGVKBottomLevelAccelerationStructureImpl);
    impl->device = device->device;

    const size_t cacheIndex = device->submittedFrames % framesInFlight;
    // Get acceleration structure properties
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties2 zeroinit;
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &accelerationStructureProperties;

    vkGetPhysicalDeviceProperties2(device->adapter->physicalDevice, &deviceProperties2);

    // Create acceleration structure geometry
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {};
    accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;

    VkAccelerationStructureGeometryTrianglesDataKHR trianglesData = {};
    trianglesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    trianglesData.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    VkBufferDeviceAddressInfo bdai1{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, descriptor->vertexBuffer->buffer};
    vkGetBufferDeviceAddress(device->device, &bdai1);

    trianglesData.vertexData.deviceAddress = vkGetBufferDeviceAddress(device->device, &bdai1);
    trianglesData.vertexStride = descriptor->vertexStride;
    trianglesData.maxVertex = descriptor->vertexCount - 1;

    if (descriptor->indexBuffer) {
        trianglesData.indexType = VK_INDEX_TYPE_UINT32;
        VkBufferDeviceAddressInfo bdai2{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, descriptor->indexBuffer->buffer};
        trianglesData.indexData.deviceAddress = vkGetBufferDeviceAddress(device->device, &bdai2);
    } else {
        trianglesData.indexType = VK_INDEX_TYPE_NONE_KHR;
    }

    accelerationStructureGeometry.geometry.triangles = trianglesData;

    // Build ranges
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {};
    if (descriptor->indexBuffer) {
        buildRangeInfo.primitiveCount = descriptor->indexCount / 3;
    } else {
        buildRangeInfo.primitiveCount = descriptor->vertexCount / 3;
    }
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;

    // Create acceleration structure build geometry info
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &accelerationStructureGeometry;

    // Get required build sizes
    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {};
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    fulkGetAccelerationStructureBuildSizesKHR(device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &buildRangeInfo.primitiveCount, &buildSizesInfo);
    // Create buffer for acceleration structure
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizesInfo.accelerationStructureSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vkCreateBuffer(device->device, &bufferCreateInfo, nullptr, &impl->accelerationStructureBuffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->device, impl->accelerationStructureBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(device->adapter->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VkMemoryAllocateFlagsInfoKHR infoKhr zeroinit;
    infoKhr.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
    infoKhr.flags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
    allocateInfo.pNext = &infoKhr;
    vkAllocateMemory(device->device, &allocateInfo, nullptr, &impl->accelerationStructureBufferMemory);
    vkBindBufferMemory(device->device, impl->accelerationStructureBuffer, impl->accelerationStructureBufferMemory, 0);

    // Create the acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = impl->accelerationStructureBuffer;
    createInfo.offset = 0;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    fulkCreateAccelerationStructureKHR(device->device, &createInfo, nullptr, &impl->accelerationStructure);

    // Create scratch buffer
    VkBufferCreateInfo scratchBufferCreateInfo = {};
    scratchBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    scratchBufferCreateInfo.size = buildSizesInfo.buildScratchSize;
    scratchBufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;

    vkCreateBuffer(device->device, &scratchBufferCreateInfo, nullptr, &impl->scratchBuffer);

    vkGetBufferMemoryRequirements(device->device, impl->scratchBuffer, &memoryRequirements);

    VkMemoryAllocateInfo scratchBufferAllocateInfo = {};
    scratchBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    scratchBufferAllocateInfo.allocationSize = memoryRequirements.size;
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    scratchBufferAllocateInfo.pNext = &memoryAllocateFlagsInfo;

    vkAllocateMemory(device->device, &scratchBufferAllocateInfo, nullptr, &impl->scratchBufferMemory);
    vkBindBufferMemory(device->device, impl->scratchBuffer, impl->scratchBufferMemory, 0);

    // Update build geometry info with scratch buffer
    VkBufferDeviceAddressInfo binfo3{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, NULL, impl->scratchBuffer};
    buildGeometryInfo.scratchData.deviceAddress = vkGetBufferDeviceAddress(device->device, &binfo3);
    buildGeometryInfo.dstAccelerationStructure = impl->accelerationStructure;

    // Build the acceleration structure
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = device->frameCaches[cacheIndex].commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->device, &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo zeroinit;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    const VkAccelerationStructureBuildRangeInfoKHR *pBuildRangeInfo = &buildRangeInfo;
    fulkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, &pBuildRangeInfo);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->queue->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    return impl;
}

/**
 * Destroys and cleans up acceleration structure resources
 */
void wgvkDestroyAccelerationStructure(WGVKBottomLevelAccelerationStructure impl) {
    if (!impl)
        return;

    fulkDestroyAccelerationStructureKHR(impl->device, impl->accelerationStructure, nullptr);
    vkDestroyBuffer(impl->device, impl->accelerationStructureBuffer, nullptr);
    vkFreeMemory(impl->device, impl->accelerationStructureBufferMemory, nullptr);
    vkDestroyBuffer(impl->device, impl->scratchBuffer, nullptr);
    vkFreeMemory(impl->device, impl->scratchBufferMemory, nullptr);

    std::free(impl);
}
/**
 * Creates a pipeline layout from uniform reflection data.
 */
 VkPipelineLayout CreateRTPipelineLayout(WGVKDevice device, const DescribedShaderModule* module) {
    VkPipelineLayout layout = VK_NULL_HANDLE;
    
    // Create descriptor set layouts from uniform map
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<ResourceTypeDescriptor> flat;
    DescribedBindGroupLayout dbgl = LoadBindGroupLayoutMod(module);

    descriptorSetLayouts.push_back(reinterpret_cast<WGVKBindGroupLayout>(dbgl.layout)->layout);
    
    // Parse uniform map to create descriptor sets
    // Would need to iterate through uniforms and create appropriate bindings
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    
    // Add push constants if needed
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    
    VkResult result = vkCreatePipelineLayout(
        device->device,
        &pipelineLayoutInfo,
        nullptr,
        &layout
    );
    
    if (result != VK_SUCCESS) {
        // Handle error
        return VK_NULL_HANDLE;
    }
    
    return layout;
}

/**
 * Loads a ray tracing pipeline from shader modules.
 * 
 * Creates a VkRayTracingPipelineKHR with appropriate stage and group info
 * derived from reflection data in the module.
 */
 WGVKRaytracingPipeline LoadRTPipeline(const DescribedShaderModule* module) {
    WGVKRaytracingPipeline pipeline = callocnewpp(WGVKRaytracingPipelineImpl);
    
    // Create shader stages from modules
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (uint32_t i = 0; i < 16; i++) {
        if(module->stages[i].module != VK_NULL_HANDLE){
            const ShaderEntryPoint& entryPoint = module->reflectionInfo.ep[i];
            VkPipelineShaderStageCreateInfo stageInfo zeroinit;
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = toVulkanShaderStage(entryPoint.stage);
            stageInfo.module = (VkShaderModule)module->stages[i].module;
            stageInfo.pName = entryPoint.name;
            shaderStages.push_back(stageInfo);
        }
    }
    
    // Create shader groups (general, triangles hit, procedural hit)
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    
    // Group configuration would be derived from reflection info
    // We need to match stages with their intended group type and index
    for (size_t i = 0; i < shaderStages.size(); ++i) {
        auto stage = shaderStages[i].stage;
        
        VkRayTracingShaderGroupCreateInfoKHR group = {};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        
        // Determine group type based on shader stage
        if (stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR || 
            stage == VK_SHADER_STAGE_MISS_BIT_KHR || 
            stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
            group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
            group.generalShader = i;
            group.closestHitShader = VK_SHADER_UNUSED_KHR;
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = VK_SHADER_UNUSED_KHR;
        } 
        else if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR || 
                 stage == VK_SHADER_STAGE_ANY_HIT_BIT_KHR) {
            group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
            group.generalShader = VK_SHADER_UNUSED_KHR;
            
            if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
                group.closestHitShader = i;
            else
                group.closestHitShader = VK_SHADER_UNUSED_KHR;
                
            if (stage == VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
                group.anyHitShader = i;
            else
                group.anyHitShader = VK_SHADER_UNUSED_KHR;
                
            group.intersectionShader = VK_SHADER_UNUSED_KHR;
        }
        else if (stage == VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
            group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
            group.generalShader = VK_SHADER_UNUSED_KHR;
            group.intersectionShader = i;
            
            // The closest hit and any hit shaders would need to be set separately
            // based on reflection info binding
            group.closestHitShader = VK_SHADER_UNUSED_KHR;
            group.anyHitShader = VK_SHADER_UNUSED_KHR;
        }
        
        shaderGroups.push_back(group);
    }
    
    // Setup ray tracing pipeline libraries (if needed)
    VkPipelineLibraryCreateInfoKHR libraryInfo = {};
    libraryInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
    libraryInfo.libraryCount = 0;
    libraryInfo.pLibraries = nullptr;
    
    // Set up pipeline layout from uniforms
    VkPipelineLayout pipelineLayout = CreateRTPipelineLayout(g_vulkanstate.device, module);
    
    // Configure dynamic state if needed
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    
    // Create the ray tracing pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    pipelineInfo.pGroups = shaderGroups.data();
    pipelineInfo.maxPipelineRayRecursionDepth = 5; // Get from reflection info if available
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.pDynamicState = &dynamicState;
    
    // Create the ray tracing pipeline
    VkResult result = fulkCreateRayTracingPipelinesKHR(
        g_vulkanstate.device->device,             // Need to get this from elsewhere
        VK_NULL_HANDLE,                           // Deferred operation handle
        VK_NULL_HANDLE,                           // Pipeline cache (optional)
        1,                                        // Create info count
        &pipelineInfo,                            // Create info
        nullptr,                                  // Allocator
        &pipeline->raytracingPipeline             // Output pipeline handle
    );
    
    if (result != VK_SUCCESS) {
        // Handle error
        rg_trap();
        return VK_NULL_HANDLE;
    }
    std::vector<char> kakbuffer(shaderGroups.size() * g_vulkanstate.device->adapter->rayTracingPipelineProperties.shaderGroupHandleSize);
    fulkGetRayTracingShaderGroupHandlesKHR(g_vulkanstate.device->device, pipeline->raytracingPipeline, 0, 3, shaderGroups.size() * g_vulkanstate.device->adapter->rayTracingPipelineProperties.shaderGroupHandleSize, kakbuffer.data());
    WGVKBufferDescriptor bdesc zeroinit;
    bdesc.usage = BufferUsage_MapWrite;
    bdesc.size = 32;
    WGVKBuffer buffer1 = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bdesc);
    WGVKBuffer buffer2 = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bdesc);
    WGVKBuffer buffer3 = wgvkDeviceCreateBuffer(g_vulkanstate.device, &bdesc);

    return pipeline;
}


