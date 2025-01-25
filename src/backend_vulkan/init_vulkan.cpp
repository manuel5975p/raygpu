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
#include <format>
#define SUPPORT_VULKAN_BACKEND 1
#include <raygpu.h>
#include <small_vector.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan_internals.hpp"
#include <internals.hpp>
VulkanState g_vulkanstate;


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
    //outColor = vec4(0,1,0,1);
    //outColor.xy += gl_FragCoord.xy / 1000.0f;
    outColor = texelFetch(texture0, ivec2(gl_FragCoord.xy / 10.0f), 0);
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
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
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

DescribedBuffer* GenBufferEx_Vk(const void *data, size_t size, BufferUsage usage){
    VkBufferUsageFlags vusage = toVulkanBufferUsage(usage);
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    VkBuffer vertexBuffer{};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = vusage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_vulkanstate.device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
    VkDeviceMemory vertexBufferMemory;
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, vertexBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device, vertexBuffer, vertexBufferMemory, 0);
    void* mapdata;
    vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mapdata);
    memcpy(mapdata, data, (size_t)bufferInfo.size);
    vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    ret->buffer = callocnewpp(BufferHandleImpl);
    ((BufferHandle)ret->buffer)->buffer = vertexBuffer;
    ((BufferHandle)ret->buffer)->memory = vertexBufferMemory;
    ((BufferHandle)ret->buffer)->refCount = 1;
    //vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, size, void **ppData)
    ret->size = bufferInfo.size;
    ret->usage = usage;
    void* mdata;
    vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &mdata);
    memcpy(mdata, data, (size_t)bufferInfo.size);
    vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    return ret;
}
// Function to find a suitable memory type
VkBuffer createVertexBuffer() {
    VkBuffer vertexBuffer{};
    std::vector<float> vertices{-1,-1,0,1,0,0,0,1,0};
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = sizeof(float) * vertices.size();
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_vulkanstate.device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }
    VkDeviceMemory vertexBufferMemory;
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(g_vulkanstate.device, vertexBuffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(g_vulkanstate.device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate vertex buffer memory!");
    }
    vkBindBufferMemory(g_vulkanstate.device, vertexBuffer, vertexBufferMemory, 0);
    void* data;
    vkMapMemory(g_vulkanstate.device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t) bufferInfo.size);
    vkUnmapMemory(g_vulkanstate.device, vertexBufferMemory);
    return vertexBuffer;
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
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(g_vulkanstate.physicalDevice, &memProperties);
    //std::cout << std::endl;
    //for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    //    std::cout << "type: " << memProperties.memoryTypes[i].propertyFlags << "\n";
    //}
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    //std::cout << std::endl;

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


/*
void createGraphicsPipeline(DescribedBindGroupLayout* setLayout) {
    VkShaderModuleCreateInfo vcinfo{};
    VkShaderModuleCreateInfo fcinfo{};
    //auto vsSpirv = readFile("../resources/hvk.vert.spv");
    //auto fsSpirv = readFile("../resources/hvk.frag.spv");
    auto [vsSpirv, fsSpirv] = glsl_to_spirv(vsSource, fsSource);
    DescribedShaderModule shaderModule = LoadShaderModuleFromSPIRV_Vk(vsSpirv.data(), vsSpirv.size() * sizeof(uint32_t), fsSpirv.data(), fsSpirv.size() * sizeof(uint32_t));

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = ((VertexAndFragmentShaderModule*)shaderModule.shaderModule)->vModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = ((VertexAndFragmentShaderModule*)shaderModule.shaderModule)->fModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VkVertexInputBindingDescription vbd{};
    VkVertexInputAttributeDescription vad{};
    vbd.binding = 0;
    vbd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vbd.stride = 12;
    vad.binding = 0;
    vad.format = VK_FORMAT_R32G32B32_SFLOAT;
    vad.location = 0;
    vad.offset = 0;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = 1;
    vertexInputInfo.pVertexAttributeDescriptions = &vad;
    vertexInputInfo.pVertexBindingDescriptions = &vbd;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    VkRect2D scissor{0, 0, g_vulkanstate.swapchainExtent.width, g_vulkanstate.swapchainExtent.height};
    VkViewport fullView{0, ((float)g_vulkanstate.swapchainExtent.height), (float)g_vulkanstate.swapchainExtent.width, -((float)g_vulkanstate.swapchainExtent.height), 0.0f, 1.0f};
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &fullView;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional


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
    colorBlending.blendConstants[0] = 1.0f;
    colorBlending.blendConstants[1] = 1.0f;
    colorBlending.blendConstants[2] = 1.0f;
    colorBlending.blendConstants[3] = 1.0f;
    
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, 
        VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, 
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 0; //static_cast<uint32_t>(dynamicStates.size());
    
    dynamicState.pDynamicStates = dynamicStates.data();
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)&setLayout->layout;
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
    pipelineInfo.pDepthStencilState = &depthStencil;
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
    g_vulkanstate.graphicsPipelineLayout = pipelineLayout;
}*/

DescribedBindGroup set{};
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

    VkClearValue clearvalues[2];
    clearvalues[0].color = VkClearColorValue{0.0f, 1.0f, 0.0f, 1.0f};
    clearvalues[1].depthStencil = VkClearDepthStencilValue{1.0f, 0u};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearvalues;
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanstate.graphicsPipeline);
    //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_vulkanstate.graphicsPipelineLayout, 0, 1, (VkDescriptorSet*)(&set->bindGroup), 0, nullptr);
    //vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
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
    VkDeviceSize offsets{};
    //vkCmdBindVertexBuffers(commandBuffer, 0, 1, &g_vulkanstate.vbuffer, &offsets);
    //vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    
}

// Function to initialize GLFW and create a window
GLFWwindow *initWindow(uint32_t width, uint32_t height, const char *title) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We don't want OpenGL
    //glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // We don't want OpenGL
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
    //extensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    

    // (Optional) Enable validation layers here if needed
    VkResult instanceCreation = vkCreateInstance(&createInfo, nullptr, &g_vulkanstate.instance);
    if (instanceCreation != VK_SUCCESS) {
        throw std::runtime_error(std::string("Failed to create Vulkan instance : ") + std::to_string(instanceCreation));
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
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return availablePresentMode;
        }
    }
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
    VkAttachmentDescription attachments[2] = {};

    VkAttachmentDescription& colorAttachment = attachments[0];
    colorAttachment = VkAttachmentDescription{};
    colorAttachment.format = g_vulkanstate.swapchainImageFormat;
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
        Texture depthTexture = LoadTexturePro_Vk(width, height, Depth32, WGPUTextureUsage_RenderAttachment, 1, 1);
        std::array<VkImageView, 2> attachments{
            g_vulkanstate.swapchainImageViews[i],
            (VkImageView)depthTexture.view
        };

        VkFramebufferCreateInfo fbcinfo{};
        fbcinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbcinfo.attachmentCount = 2;
        fbcinfo.pAttachments = attachments.data();
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
DescribedBindGroupLayout layout;
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
    ResourceTypeDescriptor types[2] = {};
    types[0].type = texture2d;
    types[0].location = 0;
    types[1].type = texture_sampler;
    types[1].location = 1;

    layout = LoadBindGroupLayout_Vk(types, 2);

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

    ResourceTypeDescriptor types[2] = {};
    types[0].type = texture2d;
    types[0].location = 0;
    types[1].type = texture_sampler;
    types[1].location = 1;

    ResourceDescriptor textureandsampler[2] = {};

    textureandsampler[0].textureView = rgreen.view;
    textureandsampler[0].binding = 0;
    textureandsampler[1].sampler = sampler.sampler;
    textureandsampler[1].binding = 1;
    DescribedPipeline* dpl = LoadPipelineForVAO_Vk(vsSource, fsSource, vao, types, 2, GetDefaultSettings());

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
    g_vulkanstate.vbuffer = createVertexBuffer();
    uint64_t framecount = 0;
    uint64_t stamp = nanoTime();
    uint64_t noell = 0;
    FullVkRenderPass renderpass = LoadRenderPass(GetDefaultSettings());
    vkResetFences(device, 1, &inFlightFence);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        auto &swapChain = g_vulkanstate.swapchain;
        

        uint32_t imageIndex;
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        
        RenderPassEncoderHandle encoder = BeginRenderPass_Vk(commandBuffer, renderpass, g_vulkanstate.swapchainImageFramebuffers[imageIndex]);
        //recordCommandBuffer(commandBuffer, imageIndex);
        SetBindgroupTexture_Vk(&set, 0, bwite);
        UpdateBindGroup_Vk(&set);
        wgvkRenderPassEncoderBindPipeline(encoder, dpl);
        wgvkRenderPassEncoderBindDescriptorSet(encoder, 0, (DescriptorSetHandle)set.bindGroup);
        
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)dpl->layout.layout, 0, 1, &((DescriptorSetHandle)set.bindGroup)->set, 0, 0);
        //vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        wgvkRenderPassEncoderBindVertexBuffer(encoder, 0, (BufferHandle)vbo->buffer, 0);
        wgvkRenderPassEncoderBindVertexBuffer(encoder, 1, (BufferHandle)inst_bo->buffer, 0);
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
    vkQueueWaitIdle(g_vulkanstate.graphicsQueue);
    if(true)
        exit(0);
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

int main() {
    GLFWwindow *window = nullptr;

    try {
        window = initWindow(1000, 800, "Völken");

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
