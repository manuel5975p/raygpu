#include <vulkan/vulkan.h>
#include <raygpu.h>
#include <internals.hpp>
#include "vulkan_internals.hpp"
extern "C" DescribedShaderModule LoadShaderModuleFromSPIRV_Vk(const uint32_t* vscode, size_t vscodeSizeInBytes, const uint32_t* fscode, size_t fscodeSizeInBytes){
    
    DescribedShaderModule ret{};
    ret.source = calloc(vscodeSizeInBytes, 1);
    std::memcpy(const_cast<void*>(ret.source), vscode, vscodeSizeInBytes);
    ret.sourceLengthInBytes = vscodeSizeInBytes;

    VkShaderModuleCreateInfo vscreateInfo{};
    vscreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vscreateInfo.codeSize = vscodeSizeInBytes;
    vscreateInfo.pCode = reinterpret_cast<const uint32_t*>(vscode);

    VkShaderModuleCreateInfo fscreateInfo{};
    fscreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fscreateInfo.codeSize = fscodeSizeInBytes;
    fscreateInfo.pCode = reinterpret_cast<const uint32_t*>(fscode);
    



    ret.shaderModule = callocnew(VertexAndFragmentShaderModule);
    if (vkCreateShaderModule(g_vulkanstate.device, &vscreateInfo, nullptr, &((VertexAndFragmentShaderModule*)ret.shaderModule)->vModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex shader module!");
    }
    if (vkCreateShaderModule(g_vulkanstate.device, &fscreateInfo, nullptr, &((VertexAndFragmentShaderModule*)ret.shaderModule)->fModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fragment shader module!");
    }

    return ret;
}

DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* ret = callocnew(DescribedPipeline);
    ret->settings = settings;
    ret->bglayout = LoadBindGroupLayout_Vk(uniforms, uniformCount);
    auto [spirV, spirF] = glsl_to_spirv(vsSource, fsSource);
    
    ret->sh = LoadShaderModuleFromSPIRV_Vk(spirV.data(), spirV.size() * 4, spirF.data(), spirF.size() * 4);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = ((VertexAndFragmentShaderModule*)ret->sh.shaderModule)->vModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = ((VertexAndFragmentShaderModule*)ret->sh.shaderModule)->fModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexBufferLayoutSet vls = getBufferLayoutRepresentation(vao->attributes.data(), vao->attributes.size());
    auto [vad, vbd] = genericVertexLayoutSetToVulkan(vls);

    vertexInputInfo.vertexBindingDescriptionCount = vbd.size();
    vertexInputInfo.vertexAttributeDescriptionCount = vad.size();
    vertexInputInfo.pVertexAttributeDescriptions = vad.data();
    vertexInputInfo.pVertexBindingDescriptions = vbd.data();

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
    dynamicState.dynamicStateCount = 1; //static_cast<uint32_t>(dynamicStates.size());
    
    dynamicState.pDynamicStates = dynamicStates.data();
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)&(ret->bglayout.layout);
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
    
    vkCreateGraphicsPipelines(g_vulkanstate.device, nullptr, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->quartet.pipeline_TriangleList);
    return ret;
}
