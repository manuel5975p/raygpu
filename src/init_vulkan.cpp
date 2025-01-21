#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>
#include <raygpu.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
void BeginRenderpassEx_Vk(DescribedRenderpass* renderPass){
    VkCommandBuffer cmd = (VkCommandBuffer)renderPass->cmdEncoder;
    vkResetCommandBuffer(cmd, 0);
    VkCommandBufferBeginInfo binfo{};
    binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &binfo);
    VkAttachmentDescription attachment;
    VkAttachmentDescriptionFlags adflags = 0;
    
    attachment.flags = adflags;
    VkRenderPassCreateInfo rpcinfo{};
    rpcinfo.attachmentCount = 1;
    rpcinfo.pAttachments = &attachment;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_D24_UNORM_S8_UINT;

    VkFramebufferCreateInfo fbcinfo{};
    VkRenderPassBeginInfo rpinfo{};
    rpinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;



}
void EndRenderpassEx(DescribedRenderpass *renderPass){ 
    vkEndCommandBuffer((VkCommandBuffer)renderPass->cmdEncoder);
    
}

inline uint64_t nanoTime() {
    using namespace std;
    using namespace chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
// Global Vulkan state structure
struct VulkanState {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    // Separate queues for clarity
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE; // Example for an additional queue type

    // Separate queue family indices
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    uint32_t computeFamily = UINT32_MAX; // Example for an additional queue type

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat swapchainImageFormat = VK_FORMAT_UNDEFINED;

    VkRenderPass renderPass;
    VkPipeline graphicsPipeline;

    VkExtent2D swapchainExtent = {0, 0};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainImageFramebuffers;
} g_vulkanstate;

// Function to find a suitable memory type
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

// Function to create a Vulkan buffer
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_vulkanstate.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    vkBindBufferMemory(g_vulkanstate.device, buffer, bufferMemory, 0);
}

// Structure to hold swapchain support details
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}
VkImage createVkImageFromRGBA8(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue queue, const uint8_t *data, uint32_t width, uint32_t height, VkDeviceMemory &imageMemory) {
    // Lambda to find suitable memory type
    auto findMemoryType = [&](uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);
        for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
            if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        throw std::runtime_error("Failed to find suitable memory type!");
    };

    // Create staging buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = width * height * 4;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer stagingBuffer;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
        throw std::runtime_error("Failed to create staging buffer!");

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDeviceMemory stagingMemory;
    if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate staging buffer memory!");

    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    // Map and copy data to staging buffer
    void *mapped;
    vkMapMemory(device, stagingMemory, 0, bufferInfo.size, 0, &mapped);
    std::memcpy(mapped, data, static_cast<size_t>(bufferInfo.size));
    vkUnmapMemory(device, stagingMemory);

    // Create image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VkImage image;
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        throw std::runtime_error("Failed to create image!");

    vkGetImageMemoryRequirements(device, image, &memReq);

    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate image memory!");

    vkBindImageMemory(device, image, imageMemory, 0);

    // Begin command buffer
    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdAllocInfo.commandPool = commandPool;
    cmdAllocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &cmdAllocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    // Transition image layout to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.levelCount = barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Copy buffer to image
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmdBuffer, stagingBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Transition image layout to SHADER_READ_ONLY_OPTIMAL
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmdBuffer);

    // Submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    // Cleanup staging resources
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    return image;
}
VkDescriptorSet LoadBindGroup_VK() {
    VkDescriptorPoolCreateInfo poold{};
    poold.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poold.poolSizeCount = 1;
    VkDescriptorPoolSize size{};
    size.descriptorCount = 1000;
    size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poold.pPoolSizes = &size;
    VkDescriptorPool pl;

    // vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkImage *pImage)
    // vkGetDeviceImageMemoryRequirements(VkDevice device, const VkDeviceImageMemoryRequirements *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    VkDescriptorSetAllocateInfo ai{};

    // vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets)
    return nullptr;
}
VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;

        if (vkCreateShaderModule(g_vulkanstate.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
void createGraphicsPipeline() {
    VkShaderModuleCreateInfo vcinfo{};
    VkShaderModuleCreateInfo fcinfo{};
    auto vssource = readFile("../resources/hvk.vert.spv");
    auto fssource = readFile("../resources/hvk.frag.spv");
    VkShaderModule vertShaderModule = createShaderModule(vssource);
    VkShaderModule fragShaderModule = createShaderModule(fssource);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    VkRect2D scissor{0, 0, g_vulkanstate.swapchainExtent.width, g_vulkanstate.swapchainExtent.height};
    VkViewport fullView{0, 0, (float)g_vulkanstate.swapchainExtent.width, (float)g_vulkanstate.swapchainExtent.height, 0.0f, 1.0f};
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &fullView;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 1; //static_cast<uint32_t>(dynamicStates.size());
    
    dynamicState.pDynamicStates = dynamicStates.data();
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};
    if (vkCreatePipelineLayout(g_vulkanstate.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
        
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = g_vulkanstate.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
    else{
        std::cout << "Successfully initialized graphics pipeline\n";
    }
    g_vulkanstate.graphicsPipeline = graphicsPipeline;
}
void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = g_vulkanstate.renderPass;
    renderPassInfo.framebuffer = g_vulkanstate.swapchainImageFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = g_vulkanstate.swapchainExtent;

    VkClearValue clearColor = {{{0.0f, 1.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanstate.graphicsPipeline);
    vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) g_vulkanstate.swapchainExtent.width;
    viewport.height = (float) g_vulkanstate.swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    //vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = VkOffset2D{int(g_vulkanstate.swapchainExtent.width / 2.5f), int(g_vulkanstate.swapchainExtent.height / 2.5f)};
    scissor.extent = g_vulkanstate.swapchainExtent;
    //vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

// Function to initialize GLFW and create a window
GLFWwindow *initWindow(uint32_t width, uint32_t height, const char *title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We don't want OpenGL
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // We don't want OpenGL
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        if(key == GLFW_KEY_ESCAPE){
            glfwSetWindowShouldClose(window, true);
        }
    });
    return window;
}

// Function to create Vulkan instance
void createInstance() {
    uint32_t requiredGLFWExtensions = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&requiredGLFWExtensions);

    if (!glfwExtensions) {
        throw std::runtime_error("Failed to get GLFW required extensions!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<const char *> validationLayers;
    for (const auto &layer : availableLayers) {
        // std::cout << "\t" << layer.layerName << " : " << layer.description << "\n";
        if (std::string(layer.layerName).find("validat") != std::string::npos) {
            std::cout << "\t[DEBUG]: Selecting layer " << layer.layerName << std::endl;
            validationLayers.push_back(layer.layerName);
        }
    }

    VkInstanceCreateInfo createInfo{};

    createInfo.enabledLayerCount = validationLayers.size();
    createInfo.ppEnabledLayerNames = validationLayers.data();

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Copy GLFW extensions to a vector
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + requiredGLFWExtensions);
    
    uint32_t instanceExtensionCount = 0;

    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    std::vector<VkExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, availableInstanceExtensions.data());
    for(auto& ext : availableInstanceExtensions){
        std::cout << ext.extensionName << ", ";
    }
    std::cout << std::endl;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    

    // (Optional) Enable validation layers here if needed

    if (vkCreateInstance(&createInfo, nullptr, &g_vulkanstate.instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance!");
    } else {
        std::cout << "Successfully created Vulkan instance\n";
    }

    

}

// Function to create window surface
void createSurface(GLFWwindow *window) {
    if (glfwCreateWindowSurface(g_vulkanstate.instance, window, nullptr, &g_vulkanstate.surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    } else {
        std::cout << "Successfully created window surface\n";
    }
}

// Function to pick a suitable physical device (GPU)
void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, devices.data());

    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }

    if (g_vulkanstate.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
picked:
    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT ext3{};
    ext3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 props2{};
    props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    props2.pNext = &ext3;
    VkPhysicalDeviceExtendedDynamicState3FeaturesEXT ext{};
    vkGetPhysicalDeviceProperties2(g_vulkanstate.physicalDevice, &props2);
    std::cout << "Extended support: " << ext3.dynamicPrimitiveTopologyUnrestricted << "\n";
    //exit(0);
    (void)0;
}

// Function to find queue families
void findQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Iterate through each queue family and check for the desired capabilities
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        // Check for graphics support
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && g_vulkanstate.graphicsFamily == UINT32_MAX) {
            g_vulkanstate.graphicsFamily = i;
        }

        // Check for presentation support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(g_vulkanstate.physicalDevice, i, g_vulkanstate.surface, &presentSupport);
        if (presentSupport && g_vulkanstate.presentFamily == UINT32_MAX) {
            g_vulkanstate.presentFamily = i;
        }

        // Example: Check for compute support
        if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && g_vulkanstate.computeFamily == UINT32_MAX) {
            g_vulkanstate.computeFamily = i;
        }

        // If all families are found, no need to continue
        if (g_vulkanstate.graphicsFamily != UINT32_MAX && g_vulkanstate.presentFamily != UINT32_MAX && g_vulkanstate.computeFamily != UINT32_MAX) {
            break;
        }
    }

    // Validate that at least graphics and present families are found
    if (g_vulkanstate.graphicsFamily == UINT32_MAX || g_vulkanstate.presentFamily == UINT32_MAX) {
        throw std::runtime_error("Failed to find required queue families!");
    }
}

// Function to query swapchain support details
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    // Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // Formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // Present Modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// Function to choose the best surface format
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // If the preferred format is not available, return the first one
    return availableFormats[0];
}

// Function to choose the best present mode
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    // Fallback to FIFO which is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

// Function to choose the swap extent (resolution)
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
void createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = g_vulkanstate.swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(g_vulkanstate.device, &renderPassInfo, nullptr, &g_vulkanstate.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}
// Function to create the swapchain
void createSwapChain(GLFWwindow *window, uint32_t width, uint32_t height) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, g_vulkanstate.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = g_vulkanstate.surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; // For stereoscopic 3D applications
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queue family indices
    uint32_t queueFamilyIndices[] = {g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily};

    if (g_vulkanstate.graphicsFamily != g_vulkanstate.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(g_vulkanstate.device, &createInfo, nullptr, &g_vulkanstate.swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    } else {
        std::cout << "Successfully created swap chain\n";
    }

    // Retrieve swapchain images
    vkGetSwapchainImagesKHR(g_vulkanstate.device, g_vulkanstate.swapchain, &imageCount, nullptr);
    g_vulkanstate.swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(g_vulkanstate.device, g_vulkanstate.swapchain, &imageCount, g_vulkanstate.swapchainImages.data());

    g_vulkanstate.swapchainImageFormat = surfaceFormat.format;
    g_vulkanstate.swapchainExtent = extent;
}

// Function to create image views for the swapchain images
void createImageViews(uint32_t width, uint32_t height) {
    g_vulkanstate.swapchainImageViews.resize(g_vulkanstate.swapchainImages.size());

    for (size_t i = 0; i < g_vulkanstate.swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = g_vulkanstate.swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = g_vulkanstate.swapchainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(g_vulkanstate.device, &createInfo, nullptr, &g_vulkanstate.swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image views!");
        }
    }
    g_vulkanstate.swapchainImageFramebuffers.resize(g_vulkanstate.swapchainImageViews.size());
    for (size_t i = 0; i < g_vulkanstate.swapchainImageViews.size(); i++) {
        VkFramebufferCreateInfo fbcinfo{};
        fbcinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbcinfo.attachmentCount = 1;
        fbcinfo.pAttachments = &g_vulkanstate.swapchainImageViews[i];
        fbcinfo.width = width;
        fbcinfo.height = height;
        fbcinfo.renderPass = g_vulkanstate.renderPass;
        fbcinfo.layers = 1;
        if (vkCreateFramebuffer(g_vulkanstate.device, &fbcinfo, nullptr, &g_vulkanstate.swapchainImageFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    std::cout << "Successfully created swapchain image views\n";
}

// Function to create a staging buffer (example usage)
void createStagingBuffer() {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    createBuffer(1000, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, memoryProperties, stagingBuffer, stagingBufferMemory);

    // Map and unmap memory (example)
    void *mappedData;
    vkMapMemory(g_vulkanstate.device, stagingBufferMemory, 0, 1000, 0, &mappedData);
    // If you have data to copy, use memcpy here
    // memcpy(mappedData, data, size);
    vkUnmapMemory(g_vulkanstate.device, stagingBufferMemory);

    // Cleanup staging buffer after usage would be handled elsewhere
}

// Function to create logical device and retrieve queues
void createLogicalDevice() {
    // Find queue families
    findQueueFamilies();

    // Collect unique queue families
    std::set<uint32_t> uniqueQueueFamilies; // = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

    uniqueQueueFamilies.insert(g_vulkanstate.computeFamily);
    uniqueQueueFamilies.insert(g_vulkanstate.graphicsFamily);
    uniqueQueueFamilies.insert(g_vulkanstate.presentFamily);

    // Example: Include computeFamily if it's different
    std::vector<uint32_t> queueFamilies(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());

    // Create queue create infos
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Specify device extensions
    std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
        // Add other required extensions here
    };

    // Specify device features
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT features{};
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT features3{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
    features.extendedDynamicState = VK_TRUE;
    
    
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Enable any desired device features here

    // Create device create info
    VkDeviceCreateInfo createInfo{};
    createInfo.pNext = &features;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // (Optional) Enable validation layers for device-specific debugging

    if (vkCreateDevice(g_vulkanstate.physicalDevice, &createInfo, nullptr, &g_vulkanstate.device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    } else {
        std::cout << "Successfully created logical device\n";
    }

    // Retrieve and assign queues
    vkGetDeviceQueue(g_vulkanstate.device, g_vulkanstate.graphicsFamily, 0, &g_vulkanstate.graphicsQueue);
    vkGetDeviceQueue(g_vulkanstate.device, g_vulkanstate.presentFamily, 0, &g_vulkanstate.presentQueue);

    if (g_vulkanstate.computeFamily != g_vulkanstate.graphicsFamily && g_vulkanstate.computeFamily != g_vulkanstate.presentFamily) {
        vkGetDeviceQueue(g_vulkanstate.device, g_vulkanstate.computeFamily, 0, &g_vulkanstate.computeQueue);
    } else {
        // If compute family is same as graphics or present, assign accordingly
        if (g_vulkanstate.computeFamily == g_vulkanstate.graphicsFamily) {
            g_vulkanstate.computeQueue = g_vulkanstate.graphicsQueue;
        } else if (g_vulkanstate.computeFamily == g_vulkanstate.presentFamily) {
            g_vulkanstate.computeQueue = g_vulkanstate.presentQueue;
        }
    }
    //__builtin_dump_struct(&g_vulkanstate, printf);
    // std::cin.get();

    std::cout << "Successfully retrieved queues\n";
}

// Function to initialize Vulkan (all setup steps)
void initVulkan(GLFWwindow *window) {
    createInstance();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    createSwapChain(window, width, height);
    createRenderPass();
    createImageViews(width, height);
    createGraphicsPipeline();
    createStagingBuffer();
}

// Function to run the main event loop
void mainLoop(GLFWwindow *window) {
    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
    VkResult scr = vkCreateSemaphore(g_vulkanstate.device, &sci, nullptr, &imageAvailableSemaphore);
    if (scr != VK_SUCCESS) {
        std::exit(1);
    }
    auto &device = g_vulkanstate.device;

    VkCommandPoolCreateInfo poolInfo{};
    VkCommandPool commandPool{};
    VkCommandBuffer commandBuffer{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = g_vulkanstate.graphicsFamily;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    uint64_t framecount = 0;
    uint64_t stamp = nanoTime();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto &swapChain = g_vulkanstate.swapchain;
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffer, imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(g_vulkanstate.graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkResult res = vkQueuePresentKHR(g_vulkanstate.presentQueue, &presentInfo);
        ++framecount;
        uint64_t stmp = nanoTime();
        if (stmp - stamp > 1000000000) {
            std::cout << framecount << std::endl;
            stamp = stmp;
            framecount = 0;
        }
        // break;
    }
}

// Function to clean up Vulkan and GLFW resources
void cleanup(GLFWwindow *window) {
    // Cleanup swapchain image views
    for (auto imageView : g_vulkanstate.swapchainImageViews) {
        vkDestroyImageView(g_vulkanstate.device, imageView, nullptr);
    }

    // Destroy swapchain
    if (g_vulkanstate.swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(g_vulkanstate.device, g_vulkanstate.swapchain, nullptr);
    }

    // Destroy surface
    if (g_vulkanstate.surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(g_vulkanstate.instance, g_vulkanstate.surface, nullptr);
    }

    // Destroy device
    if (g_vulkanstate.device != VK_NULL_HANDLE) {
        vkDestroyDevice(g_vulkanstate.device, nullptr);
    }

    // Destroy instance
    if (g_vulkanstate.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(g_vulkanstate.instance, nullptr);
    }

    // Destroy window
    if (window) {
        glfwDestroyWindow(window);
    }

    // Terminate GLFW
    glfwTerminate();
}

// Entry point
int main() {
    GLFWwindow *window = nullptr;

    try {
        // Initialize window
        window = initWindow(1000, 800, "Völken");

        // Initialize Vulkan
        initVulkan(window);

        // Run the main loop
        mainLoop(window);

        // Cleanup resources
        cleanup(window);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
