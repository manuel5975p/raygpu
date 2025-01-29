#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <format>
#define SUPPORT_VULKAN_BACKEND 1
#include <raygpu.h>
#include <small_vector.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

#include "vulkan_internals.hpp"
#include <internals.hpp>
#include <wgpustate.inc>

VulkanState g_vulkanstate{};


constexpr char vsSource[] = R"(#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inOff;
layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_PointSize = 1.0f;
    gl_Position = vec4(inPos + inOff, 0.0f, 1.0f);//vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
)";
constexpr char fsSource[] = R"(#version 450
#extension GL_EXT_samplerless_texture_functions: enable
layout(location = 0) in vec3 fragColor;
layout(binding = 0) uniform texture2D texture0;
layout(binding = 0) uniform sampler2D sampler0;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0,1,0,1);
    //outColor.xy += gl_FragCoord.xy / 1000.0f;
    outColor = texelFetch(texture0, ivec2(gl_FragCoord.xy / 40.0f), 0);
    //outColor = 0.3f * texelFetch(texture0, ivec2(gl_FragCoord.xy / 100.0f), 0);
    //outColor.y += gl_FragCoord.x * 0.001f;
    //outColor = vec4(fragColor, 1.0);
})";

std::pair<std::vector<uint32_t>, std::vector<uint32_t>> glsl_to_spirv(const char *vs, const char *fs);
/*void BeginRenderpassEx_Vk(DescribedRenderpass* renderPass){
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
}*/

//void EndRenderpassEx(DescribedRenderpass *renderPass){ 
//    vkEndCommandBuffer((VkCommandBuffer)renderPass->cmdEncoder);
//    
//}

inline uint64_t nanoTime() {
    using namespace std;
    using namespace chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}

// Global Vulkan state structure


DescribedSampler LoadSampler_Vk(addressMode amode, filterMode fmode, filterMode mipmapFilter, float maxAnisotropy){
    auto vkamode = [](addressMode a){
        switch(a){
            case addressMode::clampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case addressMode::mirrorRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case addressMode::repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    };
    DescribedSampler ret{};// = callocnew(DescribedSampler);
    VkSamplerCreateInfo sci{};
    sci.compareEnable = VK_FALSE;
    sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.addressModeU = vkamode(amode);
    sci.addressModeV = vkamode(amode);
    sci.addressModeW = vkamode(amode);
    
    sci.mipmapMode = ((fmode == filter_linear) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);

    sci.anisotropyEnable = false;
    sci.maxAnisotropy = maxAnisotropy;
    sci.magFilter = ((fmode == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
    sci.minFilter = ((fmode == filter_linear) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);

    ret.magFilter = fmode;
    ret.minFilter = fmode;
    ret.addressModeU = amode;
    ret.addressModeV = amode;
    ret.addressModeW = amode;
    ret.maxAnisotropy = maxAnisotropy;
    ret.minFilter = mipmapFilter;
    ret.compare = CompareFunction_Undefined;//huh??
    VkResult scr = vkCreateSampler(g_vulkanstate.device, &sci, nullptr, (VkSampler*)&ret.sampler);
    if(scr != VK_SUCCESS){
        throw std::runtime_error("Sampler creation failed: " + std::to_string(scr));
    }
    return ret;
}
DescribedBindGroupLayout LoadBindGroupLayout_Vk(const ResourceTypeDescriptor* descs, uint32_t uniformCount){
    DescribedBindGroupLayout retv{};
    DescribedBindGroupLayout* ret = &retv;
    VkDescriptorSetLayout layout{};
    VkDescriptorSetLayoutCreateInfo lci{};
    lci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lci.bindingCount = uniformCount;
    
    small_vector<VkDescriptorSetLayoutBinding> entries(uniformCount);
    lci.pBindings = entries.data();
    for(uint32_t i = 0;i < uniformCount;i++){
        switch(descs[i].type){
            case storage_texture2d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }break;
            case storage_texture3d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            }break;
            case storage_buffer:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }break;
            case uniform_buffer:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            }break;
            case texture2d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }break;
            case texture3d:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            }break;
            case texture_sampler:{
                entries[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            }break;
        }
        entries[i].descriptorCount = 1;
        entries[i].binding = descs[i].location;
        entries[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    ret->entries = (ResourceTypeDescriptor*)std::calloc(uniformCount, sizeof(ResourceTypeDescriptor));
    ret->entryCount = uniformCount;
    std::memcpy(ret->entries, descs, uniformCount * sizeof(ResourceTypeDescriptor));
    VkResult createResult = vkCreateDescriptorSetLayout(g_vulkanstate.device, &lci, nullptr, (VkDescriptorSetLayout*)&ret->layout);
    return retv;
}
extern "C" void UnloadBuffer_Vk(DescribedBuffer* buf){
    BufferHandle handle = (BufferHandle)buf->buffer;
    wgvkReleaseBuffer(handle);
    std::free(buf);
}

VkDescriptorSetLayout loadBindGroupLayout(){
    
    VkDescriptorSetLayout ret{};
    VkDescriptorSetLayoutCreateInfo reti{};
    VkDescriptorSetLayoutBinding tex0b{};
    tex0b.binding = 0;
    tex0b.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    tex0b.descriptorCount = 1;
    tex0b.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    reti.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    reti.bindingCount = 1;
    reti.pBindings = &tex0b;
    vkCreateDescriptorSetLayout(g_vulkanstate.device, &reti, nullptr, &ret);
    return ret;
}
VkDescriptorSet loadBindGroup(VkDescriptorSetLayout layout, Texture tex){
    VkDescriptorPool descriptorPool{};
    VkDescriptorPoolCreateInfo pci{};
    pci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pci.poolSizeCount = 1;
    pci.maxSets = 1;
    VkDescriptorPoolSize size{};
    size.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    size.descriptorCount = 1;
    pci.pPoolSizes = &size;
    vkCreateDescriptorPool(g_vulkanstate.device, &pci, nullptr, &descriptorPool);
    VkDescriptorSet ret{};
    VkDescriptorSetAllocateInfo reti{};
    reti.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    reti.descriptorSetCount = 1;
    reti.descriptorPool = descriptorPool;
    reti.pSetLayouts = &layout;
    VkResult ur = vkAllocateDescriptorSets(g_vulkanstate.device, &reti, &ret);
    if(ur == VK_SUCCESS){
        std::cout << "Successfully allocated desciptor set\n";
    }
    else{
        throw "sdjfhjskd\n";
    }    
    VkWriteDescriptorSet s{};
    s.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    VkDescriptorImageInfo imageinfo{};
    imageinfo.imageView = (VkImageView)tex.view;
    imageinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    s.dstSet = ret;
    s.dstBinding = 0;
    s.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    s.pImageInfo = &imageinfo;
    s.descriptorCount = 1;
    vkUpdateDescriptorSets(g_vulkanstate.device, 1, &s, 0, nullptr);
    return ret;
}
memory_types discoverMemoryTypes(VkPhysicalDevice physicalDevice) {
    memory_types ret{~0u, ~0u};
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    //std::cout << std::endl;
    //for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    //    std::cout << "type: " << memProperties.memoryTypes[i].propertyFlags << "\n";
    //}
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            ret.hostVisibleCoherent = i;
        }
        if((memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT){
            ret.deviceLocal = i;
        }
    }
    return ret;
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



DescribedBindGroup set{};
void createRenderPass();
// Function to initialize GLFW and create a window
void ResizeCallback_Vk(GLFWwindow* win, int width, int height){
    //std::cout << std::format("Resized to {} x {}\n", width, height) << std::flush;
    wgvkSurfaceConfigure(&g_vulkanstate.surface, uint32_t(width), uint32_t(height));
    //createRenderPass();
}
GLFWwindow *initWindow(uint32_t width, uint32_t height, const char *title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We don't want OpenGL
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    //glfwSetWindowPos(window, 1500, 2100);
    glfwSetWindowSizeCallback(window, ResizeCallback_Vk);
    //glfwSetWindowSize(window, width + 100, height + 100);
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
VkInstance createInstance() {
    VkInstance ret{};

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
    //extensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    

    // (Optional) Enable validation layers here if needed
    VkResult instanceCreation = vkCreateInstance(&createInfo, nullptr, &ret);
    if (instanceCreation != VK_SUCCESS) {
        throw std::runtime_error(std::string("Failed to create Vulkan instance : ") + std::to_string(instanceCreation));
    } else {
        std::cout << "Successfully created Vulkan instance\n";
    }

    
    return ret;
}



// Function to pick a suitable physical device (GPU)
VkPhysicalDevice pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_vulkanstate.instance, &deviceCount, devices.data());
    VkPhysicalDevice ret{};
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        std::cout << "Found device: " << props.deviceName << "\n";
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            ret = device;
            goto picked;
        }
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            ret = device;
            goto picked;
        }
    }
    for (const auto &device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) {
            ret = device;
            goto picked;
        }
    }

    if (g_vulkanstate.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
picked:
    return ret;
    //VkPhysicalDeviceExtendedDynamicState3PropertiesEXT ext3{};
    //ext3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    //VkPhysicalDeviceProperties2 props2{};
    //props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    //props2.pNext = &ext3;
    //VkPhysicalDeviceExtendedDynamicState3FeaturesEXT ext{};
    //vkGetPhysicalDeviceProperties2(g_vulkanstate.physicalDevice, &props2);
    //std::cout << "Extended support: " << ext3.dynamicPrimitiveTopologyUnrestricted << "\n";
    //(void)0;
}


// Function to find queue families

QueueIndices findQueueFamilies() {
    uint32_t queueFamilyCount = 0;
    QueueIndices ret{
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX,
        UINT32_MAX
    };
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_vulkanstate.physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Iterate through each queue family and check for the desired capabilities
    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        const auto &queueFamily = queueFamilies[i];

        // Check for graphics support
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && ret.graphicsIndex == UINT32_MAX) {
            ret.graphicsIndex = i;
        }
        // Check for presentation support
        
        VkBool32 presentSupport = glfwGetPhysicalDevicePresentationSupport(g_vulkanstate.instance, g_vulkanstate.physicalDevice, i) ? VK_TRUE : VK_FALSE;
        //vkGetPhysicalDeviceSurfaceSupportKHR(g_vulkanstate.physicalDevice, i, g_vulkanstate.surface.surface, &presentSupport);
        if (presentSupport && ret.presentIndex == UINT32_MAX) {
            ret.presentIndex = i;
        }
        if(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT){
            ret.transferIndex = i;
        }

        // Example: Check for compute support
        if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && ret.computeIndex == UINT32_MAX) {
            ret.computeIndex = i;
        }

        // If all families are found, no need to continue
        if (ret.graphicsIndex != UINT32_MAX && ret.presentIndex != UINT32_MAX && ret.computeIndex != UINT32_MAX && ret.transferIndex != UINT32_MAX) {
            break;
        }
    }

    // Validate that at least graphics and present families are found
    if (ret.graphicsIndex == UINT32_MAX) {
        throw std::runtime_error("Failed to find graphics queue, probably something went wong");
    }

    return ret;
}

// Function to query swapchain support details

void createRenderPass() {
    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    colorAttachment.format = g_vulkanstate.surface.swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription& depthAttachment = attachments[1];
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;



    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    

    if (vkCreateRenderPass(g_vulkanstate.device, &renderPassInfo, nullptr, &g_vulkanstate.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
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
std::pair<VkDevice, WGVKQueue> createLogicalDevice(VkPhysicalDevice physicalDevice, QueueIndices indices) {
    // Find queue families
    QueueIndices qind = findQueueFamilies();
    std::pair<VkDevice, WGVKQueue> ret{};
    // Collect unique queue families
    std::set<uint32_t> uniqueQueueFamilies; // = { g_vulkanstate.graphicsFamily, g_vulkanstate.presentFamily };

    uniqueQueueFamilies.insert(indices.computeIndex);
    uniqueQueueFamilies.insert(indices.graphicsIndex);
    uniqueQueueFamilies.insert(indices.presentIndex);

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
    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDeviceExtendedDynamicState3PropertiesEXT props{};
    props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_3_PROPERTIES_EXT;
    props.dynamicPrimitiveTopologyUnrestricted = VK_TRUE;
    
    
    VkPhysicalDeviceFeatures deviceFeatures{};


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    //createInfo.pNext = &features;
    //features.pNext = &props;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    // (Optional) Enable validation layers for device-specific debugging

    if (vkCreateDevice(g_vulkanstate.physicalDevice, &createInfo, nullptr, &ret.first) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device!");
    } else {
        std::cout << "Successfully created logical device\n";
    }

    // Retrieve and assign queues
    vkGetDeviceQueue(ret.first, indices.graphicsIndex, 0, &ret.second.graphicsQueue);
    vkGetDeviceQueue(ret.first, indices.presentIndex, 0, &ret.second.presentQueue);

    if (indices.computeIndex != indices.graphicsIndex && indices.computeIndex != indices.presentIndex) {
        vkGetDeviceQueue(ret.first, indices.computeIndex, 0, &ret.second.computeQueue);
    } else {
        // If compute Index is same as graphics or present, assign accordingly
        if (indices.computeIndex == indices.graphicsIndex) {
            ret.second.computeQueue = ret.second.graphicsQueue;
        } else if (indices.computeIndex == indices.presentIndex) {
            ret.second.computeQueue = ret.second.presentQueue;
        }
    }
    //__builtin_dump_struct(&g_vulkanstate, printf);
    // std::cin.get();

    std::cout << "Successfully retrieved queues\n";

    return ret;
}

// Function to initialize Vulkan (all setup steps)
DescribedBindGroupLayout layout;
void initVulkan(GLFWwindow *window) {
    g_vulkanstate.instance = createInstance();
    g_vulkanstate.physicalDevice = pickPhysicalDevice();
    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice, &g_vulkanstate.memProperties);
    g_vulkanstate.memoryTypes = discoverMemoryTypes(g_vulkanstate.physicalDevice);
    QueueIndices queues = findQueueFamilies();
    g_vulkanstate.graphicsFamily = queues.graphicsIndex;
    g_vulkanstate.computeFamily = queues.computeIndex;
    g_vulkanstate.presentFamily = queues.presentIndex;
    auto device_and_queues = createLogicalDevice(g_vulkanstate.physicalDevice, queues);
    g_vulkanstate.device = device_and_queues.first;
    g_vulkanstate.queue = device_and_queues.second;
    
    ResourceTypeDescriptor types[2] = {};
    types[0].type = texture2d;
    types[0].location = 0;
    types[1].type = texture_sampler;
    types[1].location = 1;

    g_vulkanstate.defaultPipeline = LoadPipelineForVAO_Vk(vsSource, fsSource, renderBatchVAO, types, 2, GetDefaultSettings());
    //createSurface(window);
    //int width, height;
    //glfwGetWindowSize(window, &width, &height);
    g_vulkanstate.surface = LoadSurface(window);
    std::cerr << "Surface created?\n";
    //createSwapChain(window, width, height);
    createRenderPass();
    //createImageViews(width, height);
    //ResourceTypeDescriptor types[2] = {};
    //types[0].type = texture2d;
    //types[0].location = 0;
    //types[1].type = texture_sampler;
    //types[1].location = 1;
//
    //layout = LoadBindGroupLayout_Vk(types, 2);

    //createGraphicsPipeline(&layout);
    createStagingBuffer();
}
Texture bwite{};
Texture rgreen{};
// Function to run the main event loop
void mainLoop(GLFWwindow *window) {
    float posdata[6] = {0,0,1,0,0,1};
    float offsets[4] = {-0.4f,-0.3,0.3,.2f};
    VertexArray* vao = LoadVertexArray();
    DescribedBuffer* vbo = GenBufferEx_Vk(posdata, sizeof(posdata), BufferUsage_Vertex | WGPUBufferUsage_CopyDst);
    DescribedBuffer* inst_bo = GenBufferEx_Vk(offsets, sizeof(offsets), BufferUsage_Vertex | WGPUBufferUsage_CopyDst);
    VertexAttribPointer(vao, vbo, 0, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, inst_bo, 1, VertexFormat_Float32x2, 0, VertexStepMode_Instance);
    renderBatchVBO = GenBufferEx_Vk(nullptr, 10000 * sizeof(vertex), BufferUsage_Vertex | BufferUsage_CopyDst);
    
    renderBatchVAO = LoadVertexArray();
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 0, VertexFormat_Float32x3, 0 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 1, VertexFormat_Float32x2, 3 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 2, VertexFormat_Float32x3, 5 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 3, VertexFormat_Unorm8x4,  8 * sizeof(float), VertexStepMode_Vertex);
    //VertexBufferLayoutSet layoutset = getBufferLayoutRepresentation(vao->attributes.data(), vao->attributes.size());
    //auto pair_of_dings = genericVertexLayoutSetToVulkan(layoutset);
    //vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
    //BindVertexArray(VertexArray *va)

    Image img = GenImageChecker(WHITE, BLACK, 100, 100, 10);
    bwite = LoadTextureFromImage_Vk(img);
    img = GenImageChecker(RED, GREEN, 100, 100, 10);
    rgreen = LoadTextureFromImage_Vk(img);
    
    DescribedSampler sampler = LoadSampler_Vk(repeat, filter_linear, filter_linear, 10.0f);
    //set = loadBindGroup(layout, goof);

    

    ResourceDescriptor textureandsampler[2] = {};

    textureandsampler[0].textureView = rgreen.view;
    textureandsampler[0].binding = 0;
    textureandsampler[1].sampler = sampler.sampler;
    textureandsampler[1].binding = 1;
    
    set = LoadBindGroup_Vk(&layout, textureandsampler, 2);
    
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
    uint64_t noell = 0;
    FullVkRenderPass renderpass = LoadRenderPass(GetDefaultSettings());
    Texture depthTex = LoadTexturePro_Vk(g_vulkanstate.surface.width, g_vulkanstate.surface.height, Depth32, TextureUsage_RenderAttachment, 1, 1);
    vkResetFences(device, 1, &inFlightFence);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto &swapChain = g_vulkanstate.surface.swapchain;
        

        uint32_t imageIndex;
        VkResult acquireResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if(acquireResult != VK_SUCCESS){
            std::cerr << "acquireResult is " << acquireResult << std::endl;
        }
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        //TODO: Unload
        VkFramebuffer imgfb{};
        VkFramebufferCreateInfo fbci{};
        fbci.renderPass = g_vulkanstate.renderPass;
        fbci.layers = 1;
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.width = g_vulkanstate.surface.width;
        fbci.height = g_vulkanstate.surface.height;
        fbci.attachmentCount = 2;
        if(depthTex.width != g_vulkanstate.surface.width || depthTex.height != g_vulkanstate.surface.height){
            UnloadTexture_Vk(depthTex);
            depthTex = LoadTexturePro_Vk(g_vulkanstate.surface.width, g_vulkanstate.surface.height, Depth32, TextureUsage_RenderAttachment, 1, 1);
        }
        VkImageView fbimages[2] = {
            g_vulkanstate.surface.imageViews[imageIndex],
            (VkImageView)depthTex.view
        };
        fbci.pAttachments = fbimages;
        vkCreateFramebuffer(g_vulkanstate.device, &fbci, nullptr, &imgfb);
        RenderPassEncoderHandle encoder = BeginRenderPass_Vk(commandBuffer, renderpass, imgfb);
        VkViewport viewport{
            0.0f, 
            (float) g_vulkanstate.surface.height, 
            (float) g_vulkanstate.surface.width, 
            -(float)g_vulkanstate.surface.height, 
            0.0f, 
            1.0f
        };
        vkCmdSetViewport(encoder->cmdBuffer, 0, 1, &viewport);
        VkRect2D scissorRect{.offset = {0,0}, .extent = {g_vulkanstate.surface.width, g_vulkanstate.surface.height}};
        vkCmdSetScissor(encoder->cmdBuffer, 0, 1, &scissorRect);
        //recordCommandBuffer(commandBuffer, imageIndex);
        SetBindgroupTexture_Vk(&set, 0, bwite);
        UpdateBindGroup_Vk(&set);
        wgvkRenderPassEncoderBindPipeline(encoder, g_vulkanstate.defaultPipeline);
        wgvkRenderPassEncoderBindDescriptorSet(encoder, 0, (DescriptorSetHandle)set.bindGroup);
        
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)dpl->layout.layout, 0, 1, &((DescriptorSetHandle)set.bindGroup)->set, 0, 0);
        //vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        BindVertexArray_Vk(encoder, vao);
        //wgvkRenderPassEncoderBindVertexBuffer(encoder, 0, (BufferHandle)vbo->buffer, 0);
        //wgvkRenderPassEncoderBindVertexBuffer(encoder, 1, (BufferHandle)inst_bo->buffer, 0);
        wgvkRenderpassEncoderDraw(encoder, 3, 1, 0, 0);
        SetBindgroupTexture_Vk(&set, 0, rgreen);
        UpdateBindGroup_Vk(&set);
        wgvkRenderPassEncoderBindDescriptorSet(encoder, 0, (DescriptorSetHandle)set.bindGroup);
        wgvkRenderpassEncoderDraw(encoder, 3, 1, 0, 1);

        //vkCmdEndRenderPass(commandBuffer);
        EndRenderPass_Vk(commandBuffer, renderpass, imageAvailableSemaphore, inFlightFence);
        //std::cout << ((BufferHandle)vbo->buffer)->refCount << "\n";
        vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFence);
        wgvkReleaseRenderPassEncoder(encoder);
        //std::cout << ((BufferHandle)vbo->buffer)->refCount << "\n";
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        VkSemaphore waiton[2] = {
            renderpass.signalSemaphore,
            imageAvailableSemaphore
        };
        presentInfo.pWaitSemaphores = waiton;

        VkSwapchainKHR swapChains[] = {swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        VkResult presentRes = vkQueuePresentKHR(g_vulkanstate.queue.presentQueue, &presentInfo);
        if(presentRes != VK_SUCCESS){
            std::cerr << "presentRes is " << presentRes << std::endl;
        }
        vkDestroyFramebuffer(g_vulkanstate.device, imgfb, nullptr);
        vkDeviceWaitIdle(g_vulkanstate.device);
        ++framecount;
        uint64_t stmp = nanoTime();
        if (stmp - stamp > 1000000000) {
            std::cout << framecount << std::endl;
            stamp = stmp;
            framecount = 0;
        }
        // break;
    }
    vkQueueWaitIdle(g_vulkanstate.queue.graphicsQueue);
    if(true)
        exit(0);
}

// Function to clean up Vulkan and GLFW resources
void cleanup(GLFWwindow *window) {
    // Cleanup swapchain image views
    //for (auto imageView : g_vulkanstate.swapchainImageViews) {
    //    vkDestroyImageView(g_vulkanstate.device, imageView, nullptr);
    //}
//
    //// Destroy swapchain
    //if (g_vulkanstate.swapchain != VK_NULL_HANDLE) {
    //    vkDestroySwapchainKHR(g_vulkanstate.device, g_vulkanstate.swapchain, nullptr);
    //}
//
    //// Destroy surface
    //if (g_vulkanstate.surface != VK_NULL_HANDLE) {
    //    vkDestroySurfaceKHR(g_vulkanstate.instance, g_vulkanstate.surface, nullptr);
    //}
//
    //// Destroy device
    //if (g_vulkanstate.device != VK_NULL_HANDLE) {
    //    vkDestroyDevice(g_vulkanstate.device, nullptr);
    //}
//
    //// Destroy instance
    //if (g_vulkanstate.instance != VK_NULL_HANDLE) {
    //    vkDestroyInstance(g_vulkanstate.instance, nullptr);
    //}

    // Destroy window
    if (window) {
        glfwDestroyWindow(window);
    }

    // Terminate GLFW
    glfwTerminate();
}

int main() {
    GLFWwindow *window = nullptr;

    try {
        window = initWindow(400, 300, "Völken");

        initVulkan(window);
        
        mainLoop(window);

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
