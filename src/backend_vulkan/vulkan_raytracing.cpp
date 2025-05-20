#include "vulkan_internals.hpp"
#include <macros_and_constants.h>
#include <vulkan/vulkan.h>
#include <wgvk.h>
#include <wgvk_structs_impl.h>

#define X(A) PFN_vk##A fulk##A;
RTFunctions
#undef X
    extern "C" void
    raytracing_LoadDeviceFunctions(VkDevice device) {
#define X(A) fulk##A = (PFN_vk##A)vkGetDeviceProcAddr(device, "vk" #A);
    RTFunctions
#undef X
}

struct triangle {
    vertex v1, v2, v3;
};
static inline uint32_t findMemoryType(WGVKAdapter adapter, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    

    [[unlikely]] if (adapter->memProperties.memoryTypeCount == 0){
        vkGetPhysicalDeviceMemoryProperties(adapter->physicalDevice, &adapter->memProperties);
    }

    for (uint32_t i = 0; i < adapter->memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (adapter->memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    assert(false && "failed to find suitable memory type!");
    return ~0u;
}

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
    
    //VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProperties = {};
    //accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
    //VkPhysicalDeviceProperties2 deviceProperties2 zeroinit;
    //deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    //deviceProperties2.pNext = &accelerationStructureProperties;
    //vkGetPhysicalDeviceProperties2(device->adapter->physicalDevice, &deviceProperties2);

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
    allocateInfo.memoryTypeIndex = findMemoryType(device->adapter, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
    asAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
    allocateInfo.memoryTypeIndex = findMemoryType(device->adapter, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
    scratchBufferAllocateInfo.memoryTypeIndex = findMemoryType(device->adapter, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

std::pair<DescribedBindGroupLayout, VkPipelineLayout> CreateRTPipelineLayout(WGVKDevice device, const DescribedShaderModule *module) {
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

    VkResult result = vkCreatePipelineLayout(device->device, &pipelineLayoutInfo, nullptr, &layout);

    if (result != VK_SUCCESS) {
        // Handle error
        return {};
    }

    return {dbgl, layout};
}

/**
 * Loads a ray tracing pipeline from shader modules.
 *
 * Creates a VkRayTracingPipelineKHR with appropriate stage and group info
 * derived from reflection data in the module.
 */
WGVKRaytracingPipeline LoadRTPipeline(const DescribedShaderModule *module) {
    WGVKRaytracingPipeline pipeline = callocnewpp(WGVKRaytracingPipelineImpl);
    const WGVKDevice device = g_vulkanstate.device;
    // Create shader stages from modules
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    for (uint32_t i = 0; i < 16; i++) {
        if (module->stages[i].module != VK_NULL_HANDLE) {
            const ShaderEntryPoint &entryPoint = module->reflectionInfo.ep[i];
            VkPipelineShaderStageCreateInfo stageInfo zeroinit;
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.stage = toVulkanShaderStage(entryPoint.stage);
            stageInfo.module = (VkShaderModule)module->stages[i].module;
            stageInfo.pName = entryPoint.name;
            shaderStages.push_back(stageInfo);
        }
    }

    // Create shader groups (general, triangles hit, procedural hit)
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups(3);

    uint32_t raygenIndex = VK_SHADER_UNUSED_KHR, missIndex = VK_SHADER_UNUSED_KHR, hitIndex = VK_SHADER_UNUSED_KHR;
    for (size_t i = 0; i < shaderStages.size(); ++i) {
        auto stage = shaderStages[i].stage;
        if (stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR)
            raygenIndex = i;
        else if (stage == VK_SHADER_STAGE_MISS_BIT_KHR)
            missIndex = i;
        else if (stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
            hitIndex = i;
    }

    // Set up groups in correct order
    // 1. Raygen group
    shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
    shaderGroups[0].generalShader = raygenIndex;
    shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

    // 2. Miss group
    shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    shaderGroups[1].generalShader = missIndex;
    shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;

    // 3. Hit group
    shaderGroups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    shaderGroups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    shaderGroups[2].generalShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[2].closestHitShader = hitIndex;
    shaderGroups[2].anyHitShader = VK_SHADER_UNUSED_KHR;
    shaderGroups[2].intersectionShader = VK_SHADER_UNUSED_KHR;

    // Setup ray tracing pipeline libraries (if needed)
    VkPipelineLibraryCreateInfoKHR libraryInfo = {};
    libraryInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR;
    libraryInfo.libraryCount = 0;
    libraryInfo.pLibraries = nullptr;

    // Set up pipeline layout from uniforms
    auto [bindgrouplayout, pipelineLayout] = CreateRTPipelineLayout(device, module);
    pipeline->layout = pipelineLayout;
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
    pipelineInfo.maxPipelineRayRecursionDepth = 2; // Get from reflection info if available
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.pDynamicState = &dynamicState;

    // Create the ray tracing pipeline
    VkResult result = fulkCreateRayTracingPipelinesKHR(g_vulkanstate.device->device, // Need to get this from elsewhere
                                                       VK_NULL_HANDLE,               // Deferred operation handle
                                                       VK_NULL_HANDLE,               // Pipeline cache (optional)
                                                       1,                            // Create info count
                                                       &pipelineInfo,                // Create info
                                                       nullptr,                      // Allocator
                                                       &pipeline->raytracingPipeline // Output pipeline handle
    );

    if (result != VK_SUCCESS) {
        // Handle error
        rg_trap();
        return VK_NULL_HANDLE;
    }

    const size_t sgHandleSize = device->adapter->rayTracingPipelineProperties.shaderGroupHandleSize;

    std::vector<char> shaderGroupHandles_Buffer(shaderGroups.size() * sgHandleSize);
    fulkGetRayTracingShaderGroupHandlesKHR(device->device, pipeline->raytracingPipeline, 0, shaderGroups.size(), shaderGroups.size() * sgHandleSize, shaderGroupHandles_Buffer.data());
    WGVKBufferDescriptor bdesc zeroinit;
    bdesc.usage = BufferUsage_MapWrite | BufferUsage_ShaderDeviceAddress | BufferUsage_ShaderBindingTable;
    bdesc.size = sgHandleSize;
    pipeline->raygenBindingTable = wgvkDeviceCreateBuffer(device, &bdesc);
    pipeline->missBindingTable = wgvkDeviceCreateBuffer(device, &bdesc);
    pipeline->hitBindingTable = wgvkDeviceCreateBuffer(device, &bdesc);
    wgvkQueueWriteBuffer(device->queue, pipeline->raygenBindingTable, 0, shaderGroupHandles_Buffer.data() + sgHandleSize * 0, sgHandleSize);
    wgvkQueueWriteBuffer(device->queue, pipeline->missBindingTable, 0, shaderGroupHandles_Buffer.data() + sgHandleSize * 1, sgHandleSize);
    wgvkQueueWriteBuffer(device->queue, pipeline->hitBindingTable, 0, shaderGroupHandles_Buffer.data() + sgHandleSize * 2, sgHandleSize);

    return pipeline;
}
DescribedRaytracingPipeline *LoadRaytracingPipeline(const DescribedShaderModule *shaderModule) {
    DescribedRaytracingPipeline *ret = callocnewpp(DescribedRaytracingPipeline);
    ret->pipeline = LoadRTPipeline(shaderModule);
    auto [bgl, pll] = CreateRTPipelineLayout(g_vulkanstate.device, shaderModule);
    ret->bglayout = bgl;
    ret->layout = pll;

    return ret;
}

WGVKRaytracingPassEncoder wgvkCommandEncoderBeginRaytracingPass(WGVKCommandEncoder enc) {
    WGVKRaytracingPassEncoder rtEncoder = callocnewpp(WGVKRaytracingPassEncoderImpl);
    rtEncoder->refCount = 2;
    rtEncoder->device = enc->device;
    rtEncoder->cmdBuffer = enc->buffer;
    rtEncoder->cmdEncoder = enc;
    WGVKRaytracingPassEncoderSet_add(&enc->referencedRTs, rtEncoder);
    return rtEncoder;
}
RGAPI void wgvkRaytracingPassEncoderSetPipeline(WGVKRaytracingPassEncoder rte, WGVKRaytracingPipeline raytracingPipeline) {
    rte->device->functions.vkCmdBindPipeline(rte->cmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, raytracingPipeline->raytracingPipeline);
    rte->lastLayout = raytracingPipeline->layout;
    rte->lastPipeline = raytracingPipeline;
}

//static inline void transitionToAppropriateLayoutCallback1(void* texture_, ImageViewUsageRecord* record, void* rpe_){
//    WGVKRenderPassEncoder rpe = (WGVKRenderPassEncoder)rpe_;
//    WGVKTexture texture = (WGVKTexture)texture_;
//
//    if(record->lastLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
//        initializeOrTransition(rpe->cmdEncoder, texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//    else if(record->lastLayout == VK_IMAGE_LAYOUT_GENERAL){
//        initializeOrTransition(rpe->cmdEncoder, texture, VK_IMAGE_LAYOUT_GENERAL);
//    }
//}
//
RGAPI void wgvkRaytracingPassEncoderSetBindGroup(WGVKRaytracingPassEncoder rte, uint32_t groupIndex, WGVKBindGroup bindGroup) {

}
//    vkCmdBindDescriptorSets(rte->cmdBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rte->lastLayout, 0, 1, &bindGroup->set, 0, nullptr);
//    ru_trackBindGroup(&rte->resourceUsage, bindGroup);
//    ImageViewUsageRecordMap_for_each(&bindGroup->resourceUsage.referencedTextureViews, transitionToAppropriateLayoutCallback1, rte);
//}
void wgvkCommandEncoderEndRaytracingPass(WGVKRaytracingPassEncoder commandEncoder) {}
void wgvkRaytracingPassEncoderTraceRays(WGVKRaytracingPassEncoder cpe, uint32_t width, uint32_t height, uint32_t depth) {
    VkStridedDeviceAddressRegionKHR rgR zeroinit;
    rgR.deviceAddress = cpe->lastPipeline->raygenBindingTable->address;
    rgR.stride = 32;
    rgR.size = 32;
    VkStridedDeviceAddressRegionKHR rmR zeroinit;
    rmR.deviceAddress = cpe->lastPipeline->missBindingTable->address;
    rmR.stride = 32;
    rmR.size = 32;
    VkStridedDeviceAddressRegionKHR rhR zeroinit;
    rhR.deviceAddress = cpe->lastPipeline->hitBindingTable->address;
    rhR.stride = 32;
    rhR.size = 32;
    VkStridedDeviceAddressRegionKHR callableShaderSbtEntry zeroinit;
    fulkCmdTraceRaysKHR(cpe->cmdBuffer, &rgR, &rmR, &rhR, &callableShaderSbtEntry, width, height, depth);
}
