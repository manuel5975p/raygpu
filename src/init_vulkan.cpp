#include <iostream>
#include <set>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cassert>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

    VkExtent2D swapchainExtent = { 0, 0 };
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainImageFramebuffers;
} g_vulkanstate;

// Function to find a suitable memory type
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

// Function to create a Vulkan buffer
void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
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
        renderPassInfo.renderArea.extent = VkExtent2D{1000, 800};

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            /*vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);*/

            //vkCmdDraw(commandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

// Function to initialize GLFW and create a window
GLFWwindow* initWindow(uint32_t width, uint32_t height, const char* title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We don't want OpenGL
    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        throw std::runtime_error("Failed to create GLFW window!");
    }

    return window;
}

// Function to create Vulkan instance
void createInstance() {
    uint32_t requiredGLFWExtensions = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&requiredGLFWExtensions);

    if (!glfwExtensions) {
        throw std::runtime_error("Failed to get GLFW required extensions!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;


    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::vector<const char*> validationLayers;
    for (const auto& layer : availableLayers) {
        //std::cout << "\t" << layer.layerName << " : " << layer.description << "\n";
        if(std::string(layer.layerName).find("validat") != std::string::npos){
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
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + requiredGLFWExtensions);
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
void createSurface(GLFWwindow* window) {
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

    // Select the first discrete GPU
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
            g_vulkanstate.physicalDevice = device;
            goto picked;
        }
    }

    if (g_vulkanstate.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
picked:
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
        const auto& queueFamily = queueFamilies[i];

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
        if (g_vulkanstate.graphicsFamily != UINT32_MAX &&
            g_vulkanstate.presentFamily != UINT32_MAX &&
            g_vulkanstate.computeFamily != UINT32_MAX) {
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
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    // If the preferred format is not available, return the first one
    return availableFormats[0];
}

// Function to choose the best present mode
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    // Fallback to FIFO which is guaranteed to be available
    return VK_PRESENT_MODE_FIFO_KHR;
}

// Function to choose the swap extent (resolution)
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::max(capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}
void createRenderPass(){
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
void createSwapChain(GLFWwindow* window, uint32_t width, uint32_t height) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(g_vulkanstate.physicalDevice, g_vulkanstate.surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
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
    uint32_t queueFamilyIndices[] = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

    if (g_vulkanstate.graphicsFamily != g_vulkanstate.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
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
    for(size_t i = 0;i < g_vulkanstate.swapchainImageViews.size();i++){
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
    void* mappedData;
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
    std::set<uint32_t> uniqueQueueFamilies;// = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

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
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
        // Add other required extensions here
    };

    // Specify device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Enable any desired device features here

    // Create device create info
    VkDeviceCreateInfo createInfo{};
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

    if (g_vulkanstate.computeFamily != g_vulkanstate.graphicsFamily &&
        g_vulkanstate.computeFamily != g_vulkanstate.presentFamily) {
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
    //std::cin.get();

    std::cout << "Successfully retrieved queues\n";
}

// Function to initialize Vulkan (all setup steps)
void initVulkan(GLFWwindow* window) {
    createInstance();
    createSurface(window);
    pickPhysicalDevice();
    createLogicalDevice();
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    createSwapChain(window, width, height);
    createRenderPass();
    createImageViews(width, height);
    createStagingBuffer();
}

// Function to run the main event loop
void mainLoop(GLFWwindow* window) {
    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore imageAvailableSemaphore;
    VkResult scr = vkCreateSemaphore(g_vulkanstate.device, &sci, nullptr, &imageAvailableSemaphore);
    if(scr != VK_SUCCESS){
        std::exit(1);
    }
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        uint32_t imageIndex;
        VkResult res = vkAcquireNextImageKHR(g_vulkanstate.device, g_vulkanstate.swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if(res == VK_SUCCESS){
            std::cout << "Successfully got image: " << imageIndex << "\n";
        }
        
        VkCommandPoolCreateInfo cpinfo{};
        cpinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cpinfo.queueFamilyIndex = g_vulkanstate.graphicsFamily;
        VkCommandPool cpool;
        vkCreateCommandPool(g_vulkanstate.device, &cpinfo, nullptr, &cpool);
        VkCommandBufferAllocateInfo cbinfo{};
        cbinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cbinfo.commandBufferCount = 1;
        cbinfo.commandPool = cpool;
        VkCommandBuffer cmdbuffer;
        vkAllocateCommandBuffers(g_vulkanstate.device, &cbinfo, &cmdbuffer);
        VkCommandBufferBeginInfo binfo{};
        binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //vkBeginCommandBuffer(cmdbuffer, &binfo);
        //vkEndCommandBuffer(cmdbuffer);
        VkSemaphoreCreateInfo sci{};
        sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkSemaphore semafor;
        vkCreateSemaphore(g_vulkanstate.device, &sci, nullptr, &semafor);
        VkPresentInfoKHR pinfo{};
        pinfo.pImageIndices = &imageIndex;
        VkResult results;
        pinfo.pResults = &results;
        pinfo.pSwapchains = &g_vulkanstate.swapchain;
        pinfo.swapchainCount = 1;
        pinfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        

        recordCommandBuffer(cmdbuffer, imageIndex);
        VkSubmitInfo sin{};
        sin.commandBufferCount = 1;
        sin.pCommandBuffers = &cmdbuffer;
        //sin.signalSemaphoreCount = 1;
        //sin.pSignalSemaphores = &semafor;
        pinfo.waitSemaphoreCount = 1;
        pinfo.pWaitSemaphores = &imageAvailableSemaphore;
        sin.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkFenceCreateInfo fcrinfo{};
        fcrinfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        VkFence fence{};
        vkCreateFence(g_vulkanstate.device, &fcrinfo, nullptr, &fence);

        VkResult qsr = vkQueueSubmit(g_vulkanstate.graphicsQueue, 1, &sin, fence);
        VkResult wfr = vkWaitForFences(g_vulkanstate.device, 1, &fence, VK_TRUE, ~0ull);

        //std::cout << qsr << ", " << wfr << std::endl;
        //std::cin.get();
        VkResult presentResult = vkQueuePresentKHR(g_vulkanstate.presentQueue, &pinfo);
        VkSemaphoreWaitInfo winfo{};
        winfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        winfo.pSemaphores = &imageAvailableSemaphore;
        winfo.semaphoreCount = 1;
        uint64_t sleepvalue = ~0ull;
        winfo.pValues = &sleepvalue;
        winfo.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
        //vkWaitSemaphores(g_vulkanstate.device, &winfo, ~0ull);
        if(presentResult == VK_SUCCESS){
            std::cout << "Sucessfully presented the image\n";
        }
        std::cin.get();
        //break;
    }
}

// Function to clean up Vulkan and GLFW resources
void cleanup(GLFWwindow* window) {
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
    GLFWwindow* window = nullptr;

    try {
        // Initialize window
        window = initWindow(1000, 800, "Völken");

        // Initialize Vulkan
        initVulkan(window);

        // Run the main loop
        mainLoop(window);

        // Cleanup resources
        cleanup(window);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
