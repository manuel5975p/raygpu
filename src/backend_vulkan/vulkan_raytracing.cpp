#include "vulkan_internals.hpp"
#include <macros_and_constants.h>
#include <vulkan/vulkan.h>
#include <wgvk.h>
static struct {
    VkDevice device;
    PFN_vkCmdBuildAccelerationStructuresKHR PFN_vkCmdBuildAccelerationStructuresKHR_ptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR PFN_vkGetAccelerationStructureBuildSizesKHR_ptr;
    PFN_vkCreateAccelerationStructureKHR PFN_vkCreateAccelerationStructureKHR_ptr;
    PFN_vkDestroyAccelerationStructureKHR PFN_vkDestroyAccelerationStructureKHR_ptr;
    PFN_vkGetAccelerationStructureDeviceAddressKHR PFN_vkGetAccelerationStructureDeviceAddressKHR_ptr;
} g_rt_table;

/**
 * Lazy-loads and forwards acceleration structure building commands
 */
extern "C" void vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount, const VkAccelerationStructureBuildGeometryInfoKHR *pInfos, const VkAccelerationStructureBuildRangeInfoKHR *const *ppBuildRangeInfos) {

    if (!g_rt_table.PFN_vkCmdBuildAccelerationStructuresKHR_ptr) {
        g_rt_table.PFN_vkCmdBuildAccelerationStructuresKHR_ptr = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(g_rt_table.device, "vkCmdBuildAccelerationStructuresKHR");
    }

    if (g_rt_table.PFN_vkCmdBuildAccelerationStructuresKHR_ptr) {
        g_rt_table.PFN_vkCmdBuildAccelerationStructuresKHR_ptr(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
    }
}

/**
 * Lazy-loads and forwards acceleration structure build size calculations
 */
extern "C" void vkGetAccelerationStructureBuildSizesKHR(
    VkDevice device, VkAccelerationStructureBuildTypeKHR buildType, const VkAccelerationStructureBuildGeometryInfoKHR *pBuildInfo, const uint32_t *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo) {

    if (!g_rt_table.PFN_vkGetAccelerationStructureBuildSizesKHR_ptr) {
        g_rt_table.PFN_vkGetAccelerationStructureBuildSizesKHR_ptr = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(g_rt_table.device, "vkGetAccelerationStructureBuildSizesKHR");
    }

    if (g_rt_table.PFN_vkGetAccelerationStructureBuildSizesKHR_ptr) {
        g_rt_table.PFN_vkGetAccelerationStructureBuildSizesKHR_ptr(g_rt_table.device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }
}

/**
 * Lazy-loads and forwards acceleration structure creation
 */
extern "C" VkResult vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkAccelerationStructureKHR *pAccelerationStructure) {

    if (!g_rt_table.PFN_vkCreateAccelerationStructureKHR_ptr) {
        g_rt_table.PFN_vkCreateAccelerationStructureKHR_ptr = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(g_rt_table.device, "vkCreateAccelerationStructureKHR");
    }

    if (g_rt_table.PFN_vkCreateAccelerationStructureKHR_ptr) {
        return g_rt_table.PFN_vkCreateAccelerationStructureKHR_ptr(g_rt_table.device, pCreateInfo, pAllocator, pAccelerationStructure);
    }

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

/**
 * Lazy-loads and forwards acceleration structure destruction
 */
extern "C" void vkDestroyAccelerationStructureKHR(VkDevice device, VkAccelerationStructureKHR accelerationStructure, const VkAllocationCallbacks *pAllocator) {

    if (!g_rt_table.PFN_vkDestroyAccelerationStructureKHR_ptr) {
        g_rt_table.PFN_vkDestroyAccelerationStructureKHR_ptr = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(g_rt_table.device, "vkDestroyAccelerationStructureKHR");
    }

    if (g_rt_table.PFN_vkDestroyAccelerationStructureKHR_ptr) {
        g_rt_table.PFN_vkDestroyAccelerationStructureKHR_ptr(g_rt_table.device, accelerationStructure, pAllocator);
    }
}

/**
 * Lazy-loads and forwards retrieval of acceleration structure device address
 */
extern "C" VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR *pInfo) {

    if (!g_rt_table.PFN_vkGetAccelerationStructureDeviceAddressKHR_ptr) {
        g_rt_table.PFN_vkGetAccelerationStructureDeviceAddressKHR_ptr = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(g_rt_table.device, "vkGetAccelerationStructureDeviceAddressKHR");
    }

    if (g_rt_table.PFN_vkGetAccelerationStructureDeviceAddressKHR_ptr) {
        return g_rt_table.PFN_vkGetAccelerationStructureDeviceAddressKHR_ptr(g_rt_table.device, pInfo);
    }

    return 0;
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

    vkGetPhysicalDeviceProperties2(device->physicalDevice, &deviceProperties2);

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
    allocateInfo.memoryTypeIndex = findMemoryType(device->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
        addressInfo.accelerationStructure = descriptor->bottomLevelAS[i];
        uint64_t blasAddress = vkGetAccelerationStructureDeviceAddressKHR(device->device, &addressInfo);

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

    vkGetAccelerationStructureBuildSizesKHR(device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &buildRangeInfo.primitiveCount, &buildSizesInfo);

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
    asAllocateInfo.memoryTypeIndex = findMemoryType(device->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device->device, &asAllocateInfo, nullptr, &impl->accelerationStructureBufferMemory);
    vkBindBufferMemory(device->device, impl->accelerationStructureBuffer, impl->accelerationStructureBufferMemory, 0);

    // Create the acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = impl->accelerationStructureBuffer;
    createInfo.offset = 0;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

    vkCreateAccelerationStructureKHR(device->device, &createInfo, nullptr, &impl->accelerationStructure);

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
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, &pBuildRangeInfo);

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

    vkDestroyAccelerationStructureKHR(impl->device, impl->accelerationStructure, nullptr);
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

    return vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
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

    vkGetPhysicalDeviceProperties2(device->physicalDevice, &deviceProperties2);

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

    vkGetAccelerationStructureBuildSizesKHR(device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &buildRangeInfo.primitiveCount, &buildSizesInfo);

    // Create buffer for acceleration structure
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizesInfo.accelerationStructureSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    vkCreateBuffer(device->device, &bufferCreateInfo, nullptr, &impl->accelerationStructureBuffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device->device, impl->accelerationStructureBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = findMemoryType(device->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device->device, &allocateInfo, nullptr, &impl->accelerationStructureBufferMemory);
    vkBindBufferMemory(device->device, impl->accelerationStructureBuffer, impl->accelerationStructureBufferMemory, 0);

    // Create the acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = impl->accelerationStructureBuffer;
    createInfo.offset = 0;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

    vkCreateAccelerationStructureKHR(device->device, &createInfo, nullptr, &impl->accelerationStructure);

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
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, &pBuildRangeInfo);

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

    vkDestroyAccelerationStructureKHR(impl->device, impl->accelerationStructure, nullptr);
    vkDestroyBuffer(impl->device, impl->accelerationStructureBuffer, nullptr);
    vkFreeMemory(impl->device, impl->accelerationStructureBufferMemory, nullptr);
    vkDestroyBuffer(impl->device, impl->scratchBuffer, nullptr);
    vkFreeMemory(impl->device, impl->scratchBufferMemory, nullptr);

    std::free(impl);
}