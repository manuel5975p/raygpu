#ifndef VULKAN_INTERNALS_HPP
#define VULKAN_INTERNALS_HPP
#include "small_vector.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <utility>
#include <raygpu.h>
#include <internals.hpp>
#define SUPPORT_VULKAN_BACKEND 1
#include <enum_translation.h>
inline std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>> genericVertexLayoutSetToVulkan(const VertexBufferLayoutSet& vls){
    std::pair<std::vector<VkVertexInputAttributeDescription>, std::vector<VkVertexInputBindingDescription>> ret{};

    std::vector<VkVertexInputAttributeDescription>& attribs = ret.first;
    uint32_t totalNumberOfAttributes = 0;
    
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        totalNumberOfAttributes += vls.layouts[i].attributeCount;
    }
    attribs.reserve(totalNumberOfAttributes);
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        for(uint32_t j = 0;j < vls.layouts[i].attributeCount;j++){
            VkVertexInputAttributeDescription adesc{};
            adesc.location = vls.layouts[i].attributes[j].shaderLocation;
            adesc.offset = vls.layouts[i].attributes[j].offset;
            adesc.binding = i;
            adesc.format = toVulkanVertexFormat(vls.layouts[i].attributes[j].format);
            attribs.push_back(adesc);
        }
    }


    std::vector<VkVertexInputBindingDescription>& bufferLayouts = ret.second;
    bufferLayouts.resize(vls.number_of_buffers);
    for(uint32_t i = 0;i < vls.number_of_buffers;i++){
        bufferLayouts[i].binding = i;
        bufferLayouts[i].stride = vls.layouts[i].arrayStride;
        bufferLayouts[i].inputRate = toVulkanVertexStepMode(vls.layouts[i].stepMode);
    }
    return ret;
}
struct VertexAndFragmentShaderModuleImpl;
struct DescriptorSetHandleImpl;
struct BufferHandleImpl;
struct RenderPassEncoderHandleImpl;
struct CommandBufferHandleImpl;

typedef VertexAndFragmentShaderModuleImpl* VertexAndFragmentShaderModule;
typedef DescriptorSetHandleImpl* DescriptorSetHandle;
typedef BufferHandleImpl* BufferHandle;
typedef RenderPassEncoderHandleImpl* RenderPassEncoderHandle;
typedef CommandBufferHandleImpl* CommmandBufferHandle;
using refcount_type = uint32_t;
typedef struct VertexAndFragmentShaderModuleImpl{
    VkShaderModule vModule;
    VkShaderModule fModule;
}VertexAndFragmentShaderModuleImpl;

typedef struct DescriptorSetHandleImpl{
    VkDescriptorSet set;
    VkDescriptorPool pool;
    refcount_type refCount;
}DescriptorSetHandleImpl;

typedef struct BufferHandleImpl{
    VkBuffer buffer;
    VkDeviceMemory memory;
    refcount_type refCount;
}BufferHandleImpl;

typedef struct RenderPassEncoderHandleImpl{
    VkCommandBuffer cmdBuffer;
    small_vector<BufferHandle> referencedBuffers;
    small_vector<DescriptorSetHandle> referencedDescriptorSets;
    VkPipelineLayout lastLayout;
    refcount_type refCount;
}RenderPassEncoderHandleImpl;

typedef struct CommandBufferHandleImpl{
    small_vector<RenderPassEncoderHandle> referencedRPs;
    VkCommandBuffer buffer;
    VkCommandPool pool;
}CommandBufferHandleImpl;


void ReleaseCommandBuffer(CommmandBufferHandle commandBuffer);
void ReleaseRenderPassEncoder(RenderPassEncoderHandle rpenc);
void ReleaseBuffer(BufferHandle commandBuffer);
void ReleaseDescriptorSet(DescriptorSetHandle commandBuffer);

void RenderpassEncoderDraw(RenderPassEncoderHandle rpe, uint32_t vertices, uint32_t instances, uint32_t firstvertex, uint32_t firstinstance);
void RenderpassEncoderDrawIndexed(RenderPassEncoderHandle rpe, uint32_t indices, uint32_t instances, uint32_t firstindex, uint32_t firstinstance);
void RenderPassDescriptorBindDescriptorSet(RenderPassEncoderHandle rpe, uint32_t group, DescriptorSetHandle dset);
void RenderPassDescriptorBindPipeline(RenderPassEncoderHandle rpe, uint32_t group, DescribedPipeline* pipeline);

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
    VkPipelineLayout graphicsPipelineLayout;

    VkExtent2D swapchainExtent = {0, 0};
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapchainImageFramebuffers;

    VkBuffer vbuffer;
};  extern VulkanState g_vulkanstate; 

struct FullVkRenderPass{
    VkRenderPass renderPass;
    VkSemaphore signalSemaphore;
};
inline FullVkRenderPass LoadRenderPass(RenderSettings settings){
    FullVkRenderPass ret{};
    VkRenderPassCreateInfo rpci{};

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

    VkSubpassDescription subpass{};
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 2;
    rpci.pAttachments = attachments;
    
    VkAttachmentReference ca{};
    ca.attachment = 0;
    ca.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference da{};
    da.attachment = 1;
    da.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &ca;
    subpass.pDepthStencilAttachment = &da;

    rpci.pSubpasses = &subpass;
    rpci.subpassCount = 1;
    vkCreateRenderPass(g_vulkanstate.device, &rpci, nullptr, &ret.renderPass);

    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(g_vulkanstate.device, &si, nullptr, &ret.signalSemaphore);
    return ret;
}

inline RenderPassEncoderHandle BeginRenderPass_Vk(VkCommandBuffer cbuffer, FullVkRenderPass rp, VkFramebuffer fb){
    RenderPassEncoderHandle ret = callocnewpp(RenderPassEncoderHandleImpl);
    VkCommandBufferBeginInfo bbi{};
    bbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo rpbi{};
    VkClearValue clearvalues[2] = {};
    clearvalues[0].color = VkClearColorValue{};
    clearvalues[0].color.float32[0] = 1.0f;
    clearvalues[1].depthStencil = VkClearDepthStencilValue{};
    clearvalues[1].depthStencil.depth = 1.0f;
    rpbi.renderPass = rp.renderPass;
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clearvalues;
    rpbi.framebuffer = fb;
    rpbi.renderArea.extent.width = 1000;
    rpbi.renderArea.extent.height = 800;

    VkSubpassContents scontents{};
    vkBeginCommandBuffer(cbuffer, &bbi);
    vkCmdBeginRenderPass(cbuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
    ret->cmdBuffer = cbuffer;
    return ret;
}
inline void EndRenderPass_Vk(VkCommandBuffer cbuffer, FullVkRenderPass rp, VkSemaphore imageAvailableSemaphore){
    vkCmdEndRenderPass(cbuffer);
    vkEndCommandBuffer(cbuffer);
    VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    VkSubmitInfo sinfo{};
    sinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    sinfo.commandBufferCount = 1;
    sinfo.pCommandBuffers = &cbuffer;
    sinfo.waitSemaphoreCount = 1;
    sinfo.pWaitSemaphores = &imageAvailableSemaphore;
    sinfo.pWaitDstStageMask = &stageMask;
    sinfo.signalSemaphoreCount = 1;
    sinfo.pSignalSemaphores = &rp.signalSemaphore;
    VkFence fence{};
    VkFenceCreateInfo finfo{};
    finfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    if(vkCreateFence(g_vulkanstate.device, &finfo, nullptr, &fence) != VK_SUCCESS){
        throw std::runtime_error("Could not create fence");
    }
    if(vkQueueSubmit(g_vulkanstate.graphicsQueue, 1, &sinfo, fence) != VK_SUCCESS){
        throw std::runtime_error("Could not submit commandbuffer");
    }
    if(vkWaitForFences(g_vulkanstate.device, 1, &fence, VK_TRUE, ~0) != VK_SUCCESS){
        throw std::runtime_error("Could not wait for fence");
    }
    vkDestroyFence(g_vulkanstate.device, fence, nullptr);
}

extern "C" Texture LoadTexturePro_Vk(uint32_t width, uint32_t height, PixelFormat format, int usage, uint32_t sampleCount, uint32_t mipmaps, const void* data = nullptr);
extern "C" Texture LoadTextureFromImage_Vk(Image img);
extern "C" DescribedShaderModule LoadShaderModuleFromSPIRV_Vk(const uint32_t* vscode, size_t vscodeSizeInBytes, const uint32_t* fscode, size_t fscodeSizeInBytes);
extern "C" DescribedBindGroupLayout LoadBindGroupLayout_Vk(const ResourceTypeDescriptor* descs, uint32_t uniformCount);
extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
extern "C" DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count);
extern "C" void RenderPassDescriptorBindIndexBuffer(RenderPassEncoderHandle rpe, BufferHandle buffer, VkDeviceSize offset, VkIndexType indexType);
extern "C" void RenderPassDescriptorBindVertexBuffer(RenderPassEncoderHandle rpe, uint32_t binding, BufferHandle buffer, VkDeviceSize offset);
#endif