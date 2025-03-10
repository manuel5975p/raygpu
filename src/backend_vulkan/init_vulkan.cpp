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
//#define SUPPORT_VULKAN_BACKEND 1
#include <raygpu.h>
//#include <small_vector.hpp>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ShaderLang.h>

#include "vulkan_internals.hpp"
#include <internals.hpp>
#include <wgpustate.inc>




constexpr char vsSource[] = R"(#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inColor;
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;
layout(std140, binding = 0) uniform Perspective_View {
    mat4 transform; // 4x4 matrix
};
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
    gl_Position = transform * vec4(inPos, 1.0f);
    //gl_Position = vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
    fragColor = inColor;
    fragUV = inUV;
}
)";
constexpr char fsSource[] = R"(#version 450
//#extension GL_EXT_samplerless_texture_functions: enable
layout(location = 0) in vec4 fragColor;


layout(binding = 1) uniform texture2D texture0;
layout(binding = 2) uniform sampler sampler0;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 fragUV;

void main() {
    //outColor = vec4(0,1,0,1);
    outColor = fragColor * texture(sampler2D(texture0, sampler0), fragUV);
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
// Function to initialize GLFW and create a window
void ResizeCallback_Vk(GLFWwindow* win, int width, int height){
    
    //std::cout << std::format("Resized to {} x {}\n", width, height) << std::flush;
    
    
    ResizeSurface(&g_vulkanstate.surface, width, height);
    //createRenderPass();
}
/*void* InitWindow(uint32_t width, uint32_t height, const char *title) {
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
}*/



// Function to initialize Vulkan (all setup steps)  
void initVulkan(GLFWwindow *window) {
    InitBackend();
    SurfaceConfiguration config{};
    config.presentMode = PresentMode_Fifo;
    g_vulkanstate.surface = LoadSurface(window, config);
    
    ResourceTypeDescriptor types[3] = {};
    types[0].type = uniform_buffer;
    types[0].location = 0;
    types[1].type = texture2d;
    types[1].location = 1;
    types[2].type = texture_sampler;
    types[2].location = 2;

    renderBatchVBO = GenBufferEx(nullptr, 10000 * sizeof(vertex), BufferUsage_Vertex | BufferUsage_CopyDst);
    renderBatchVAO = LoadVertexArray();
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 0, VertexFormat_Float32x3, 0 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 1, VertexFormat_Float32x2, 3 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 2, VertexFormat_Float32x3, 5 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 3, VertexFormat_Float32x4,  8 * sizeof(float), VertexStepMode_Vertex);
    
    g_vulkanstate.defaultPipeline = LoadPipelineForVAO_Vk(vsSource, fsSource, renderBatchVAO, types, sizeof(types) / sizeof(ResourceTypeDescriptor), GetDefaultSettings());
    //createSurface(window);
    //int width, height;
    //glfwGetWindowSize(window, &width, &height);
    vboptr = (vertex*)std::calloc(10000, sizeof(vertex));
    vboptr_base = vboptr;
    std::cerr << "Surface created?\n";
}
Texture bwite{};
Texture rgreen{};
// Function to run the main event loop
void mainLoop(GLFWwindow *window) {
    float posdata[6] = {0,0,1,0,0,1};
    float offsets[4] = {-0.4f,-0.3,0.3,.2f};
    VertexArray* vao = LoadVertexArray();
    DescribedBuffer* vbo = GenBufferEx(posdata, sizeof(posdata), BufferUsage_Vertex | WGPUBufferUsage_CopyDst);
    DescribedBuffer* inst_bo = GenBufferEx(offsets, sizeof(offsets), BufferUsage_Vertex | WGPUBufferUsage_CopyDst);
    VertexAttribPointer(vao, vbo, 0, VertexFormat_Float32x2, 0, VertexStepMode_Vertex);
    VertexAttribPointer(vao, inst_bo, 1, VertexFormat_Float32x2, 0, VertexStepMode_Instance);
    
    //VertexBufferLayoutSet layoutset = getBufferLayoutRepresentation(vao->attributes.data(), vao->attributes.size());
    //auto pair_of_dings = genericVertexLayoutSetToVulkan(layoutset);
    //vkCmdSetVertexInputEXT(VkCommandBuffer commandBuffer, uint32_t vertexBindingDescriptionCount, const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
    //BindVertexArray(VertexArray *va)

    Image img = GenImageChecker(BLACK, WHITE, 100, 100, 20);
    bwite = LoadTextureFromImage(img);
    img = GenImageChecker(WHITE, WHITE, 100, 100, 10);
    rgreen = LoadTextureFromImage(img);
    
    DescribedSampler sampler = LoadSamplerEx(repeat, filter_nearest, filter_nearest, 10.0f);
    //set = loadBindGroup(layout, goof);
    Matrix iden = MatrixScale(0.5,0.5,0.5);
    DescribedBuffer* idbuffer = GenBufferEx(&iden, sizeof(iden), BufferUsage_CopyDst | BufferUsage_Uniform);

    
    SetBindgroupUniformBuffer(&g_vulkanstate.defaultPipeline->bindGroup, 0, idbuffer);
    SetBindgroupTexture(&g_vulkanstate.defaultPipeline->bindGroup, 1, bwite);
    SetBindgroupSampler(&g_vulkanstate.defaultPipeline->bindGroup, 2, sampler);
    //set = LoadBindGroup_Vk(&layout, textureandsampler, 2);
    
    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
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

    
    uint64_t framecount = 0;
    uint64_t stamp = nanoTime();
    uint64_t noell = 0;
    //g_renderstate.renderpass = LoadRenderPass(GetDefaultSettings());
    Texture depthTex = LoadTexturePro(g_vulkanstate.surface.surfaceConfig.width, g_vulkanstate.surface.surfaceConfig.height, Depth32, TextureUsage_RenderAttachment, 1, 1);


    while (WindowShouldClose()) {
        BeginDrawing();
        WGVKSurface wgs = ((WGVKSurface)g_vulkanstate.surface.surface);
        auto &swapChain = ((WGVKSurface)g_vulkanstate.surface.surface)->swapchain;
        GetNewTexture(&g_vulkanstate.surface);

        
        
        //VkResult acquireResult = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        //if(acquireResult != VK_SUCCESS){
        //    std::cerr << "acquireResult is " << acquireResult << std::endl;
        //}
        
        vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
        //TODO: Unload
        VkFramebuffer imgfb{};
        VkFramebufferCreateInfo fbci{};
        fbci.renderPass = g_vulkanstate.renderPass;
        fbci.layers = 1;
        fbci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbci.width = wgs->width;
        fbci.height = wgs->height;
        fbci.attachmentCount = 2;
        //if(depthTex.width != wgs->width || depthTex.height != g_vulkanstate.surface.height){
        //    UnloadTexture_Vk(depthTex);
        //    depthTex = LoadTexturePro_Vk(g_vulkanstate.surface.width, g_vulkanstate.surface.height, Depth32, TextureUsage_RenderAttachment, 1, 1);
        //}
        VkImageView fbimages[2] = {
            (VkImageView)g_vulkanstate.surface.renderTarget.texture.view,
            (VkImageView)g_vulkanstate.surface.renderTarget.depth.view
        };
        fbci.pAttachments = fbimages;
        vkCreateFramebuffer(g_vulkanstate.device, &fbci, nullptr, &imgfb);
        WGVKRenderPassEncoder encoder = nullptr;// = BeginRenderPass_Vk(commandBuffer, &g_renderstate.renderpass, imgfb);
        VkViewport viewport{
            0.0f, 
            (float) g_vulkanstate.surface.surfaceConfig.height, 
            (float) g_vulkanstate.surface.surfaceConfig.width, 
            -(float)g_vulkanstate.surface.surfaceConfig.height, 
            0.0f, 
            1.0f
        };
        vkCmdSetViewport(encoder->cmdBuffer, 0, 1, &viewport);
        VkRect2D scissorRect{.offset = {0, 0}, .extent = {g_vulkanstate.surface.surfaceConfig.width, g_vulkanstate.surface.surfaceConfig.height}};
        vkCmdSetScissor(encoder->cmdBuffer, 0, 1, &scissorRect);
        
        //recordCommandBuffer(commandBuffer, imageIndex);
        //SetBindgroupTexture_Vk(&set, 0, bwite);
        //UpdateBindGroup_Vk(&set);
        wgvkRenderPassEncoderBindPipeline(encoder, g_vulkanstate.defaultPipeline);
        UpdateBindGroup(&g_vulkanstate.defaultPipeline->bindGroup);
        wgvkRenderPassEncoderBindDescriptorSet(encoder, 0, (DescriptorSetHandle)g_vulkanstate.defaultPipeline->bindGroup.bindGroup);
        
        //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)dpl->layout.layout, 0, 1, &((DescriptorSetHandle)set.bindGroup)->set, 0, 0);
        //vkCmdSetPrimitiveTopology(commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //BindVertexArray_Vk(encoder, vao);
        //wgvkRenderPassEncoderBindVertexBuffer(encoder, 0, (BufferHandle)vbo->buffer, 0);
        //wgvkRenderPassEncoderBindVertexBuffer(encoder, 1, (BufferHandle)inst_bo->buffer, 0);
        //wgvkRenderpassEncoderDraw(encoder, 3, 1, 0, 0);
        //SetBindgroupTexture_Vk(&set, 0, rgreen);
        //UpdateBindGroup_Vk(&set);
        //wgvkRenderPassEncoderBindDescriptorSet(encoder, 0, (DescriptorSetHandle)g_vulkanstate.defaultPipeline->bindGroup.bindGroup);
        //wgvkRenderpassEncoderDraw(encoder, 3, 1, 0, 1);
        rlColor4ub(255, 255, 255, 255);
        rlTexCoord2f(0, 0);
        rlVertex2f(0, 0);
        rlTexCoord2f(1, 0);
        rlVertex2f(1, 0);
        rlTexCoord2f(0, 1);
        rlVertex2f(0, 1);
        rlTexCoord2f(0, 0);
        rlVertex2f(-1, 0);
        rlTexCoord2f(1, 0);
        rlVertex2f(0, 0);
        rlTexCoord2f(0, 1);
        rlVertex2f(-1, 1);
        drawCurrentBatch();
        //vkCmdEndRenderPass(commandBuffer);
        //EndRenderPass(commandBuffer, &g_renderstate.renderpass);
        //std::cout << ((BufferHandle)vbo->buffer)->refCount << "\n";
        vkWaitForFences(device, 1, &g_vulkanstate.syncState.renderFinishedFence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &g_vulkanstate.syncState.renderFinishedFence);
        wgvkReleaseRenderPassEncoder(encoder);
        //std::cout << ((BufferHandle)vbo->buffer)->refCount << "\n";
        
        PresentSurface(&g_vulkanstate.surface);
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
        EndDrawing();
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
    //if (window) {
    //    glfwDestroyWindow(window);
    //}

    // Terminate GLFW
    //glfwTerminate();
}

int main() {
    InitWindow(1200, 800, "Vulkan Fanschter");
    /*GLFWwindow *window = nullptr;

    try {
        window = (GLFWwindow*)InitWindow(800, 600, "Völken");

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
    }*/

    return EXIT_SUCCESS;
}
