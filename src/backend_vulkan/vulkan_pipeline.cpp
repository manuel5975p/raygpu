#include <vulkan/vulkan.h>
#define Font rlFont
#include <raygpu.h>
#undef Font
#include <internals.hpp>
#include <vulkan/vulkan_core.h>
#include "pipeline.h"
#include "vulkan_internals.hpp"
#include <spirv_reflect.h>
#include <enum_translation.h>

extern "C" DescribedShaderModule LoadShaderModuleSPIRV(ShaderSources sources){
    DescribedShaderModule ret zeroinit;
    
    for(uint32_t i = 0;i < sources.sourceCount;i++){
        VkShaderModuleCreateInfo csCreateInfo zeroinit;
        csCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        csCreateInfo.codeSize = sources.sources[i].sizeInBytes;
        csCreateInfo.pCode = (const uint32_t*)sources.sources[i].data;
        VkWriteDescriptorSet ws;
        VkShaderModule insert zeroinit;
        vkCreateShaderModule(g_vulkanstate.device->device, &csCreateInfo, nullptr, &insert);
        spv_reflect::ShaderModule module(csCreateInfo.codeSize, csCreateInfo.pCode);
        uint32_t epCount = module.GetEntryPointCount();
        for(uint32_t i = 0;i < epCount;i++){
            SpvReflectShaderStageFlagBits epStage = module.GetEntryPointShaderStage(i);
            ShaderStage stage = [](SpvReflectShaderStageFlagBits epStage){
                switch(epStage){
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
                        return ShaderStage_Vertex;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
                        return ShaderStage_Fragment;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
                        return ShaderStage_Compute;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
                        return ShaderStage_Geometry;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:
                        return ShaderStage_RayGen;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
                        return ShaderStage_ClosestHit;
                    case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:
                        return ShaderStage_Miss;
                    default:
                        TRACELOG(LOG_FATAL, "Unknown shader stage: %d", (int)epStage);
                        return ShaderStage_EnumCount;
                }
            }(epStage);
            
            ret.reflectionInfo.ep[stage].stage = stage;
            ret.stages[stage].module = insert;
            std::memset(ret.reflectionInfo.ep[stage].name, 0, sizeof(ret.reflectionInfo.ep[stage].name));
            uint32_t eplength = std::strlen(module.GetEntryPointName(i));
            rassert(eplength < 16, "Entry point name must be < 16 chars");
            std::copy(module.GetEntryPointName(i), module.GetEntryPointName(i) + eplength, ret.reflectionInfo.ep[stage].name);
            //TRACELOG(LOG_INFO, "%s : %d", module.GetEntryPointName(i), module.GetEntryPointShaderStage(i));
        }
    }

    return ret;
}

extern "C" WGVKRenderPipeline createSingleRenderPipe(const ModifiablePipelineState& mst, const DescribedShaderModule& shaderModule, const DescribedBindGroupLayout& bglayout, const DescribedPipelineLayout& pllayout){
    TRACELOG(LOG_INFO, "Generating new single pipeline");
    WGVKRenderPipeline ret = callocnewpp(WGVKRenderPipelineImpl);
    auto& settings = mst;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = (VkShaderModule)shaderModule.stages[ShaderStage_Vertex].module;
    vertShaderStageInfo.pName = shaderModule.reflectionInfo.ep[ShaderStage_Vertex].name;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = (VkShaderModule)shaderModule.stages[ShaderStage_Fragment].module;
    fragShaderStageInfo.pName = shaderModule.reflectionInfo.ep[ShaderStage_Fragment].name;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex Input Setup
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    auto vls = getBufferLayoutRepresentation(mst.vertexAttributes.data(), mst.vertexAttributes.size());
    auto [vad, vbd] = genericVertexLayoutSetToVulkan(vls);

    vertexInputInfo.vertexBindingDescriptionCount = vbd.size();
    vertexInputInfo.vertexAttributeDescriptionCount = vad.size();
    vertexInputInfo.pVertexAttributeDescriptions = vad.data();
    vertexInputInfo.pVertexBindingDescriptions = vbd.data();

    // Input Assembly Setup
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = toVulkanPrimitive(mst.primitiveType);
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissor Setup
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    VkRect2D scissor{0, 0, g_vulkanstate.surface.surfaceConfig.width, g_vulkanstate.surface.surfaceConfig.height};
    VkViewport fullView{
        0.0f, 
        (float)g_vulkanstate. surface.surfaceConfig.height, 
        (float)g_vulkanstate. surface.surfaceConfig.width, 
        -((float)g_vulkanstate.surface.surfaceConfig.height), 
        0.0f, 
        1.0f
    };
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &fullView;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = (float)mst.settings.lineWidth;

    // Incorporate face culling from RenderSettings
    // TODO: Cull modes
    rasterizer.cullMode = VK_CULL_MODE_NONE; 
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;

    // Depth Stencil State Setup
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = settings.settings.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = settings.settings.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = toVulkanCompareFunction((CompareFunction)settings.settings.depthCompare);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    // Multisampling State Setup
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    
    // Map sampleCount from RenderSettings to VkSampleCountFlagBits
    switch (settings.settings.sampleCount) {
        case 1: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; break;
        case 2: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT; break;
        case 4: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT; break;
        case 8: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT; break;
        case 16: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT; break;
        case 32: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT; break;
        case 64: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT; break;
        default: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; break;
    }

    // Color Blend Attachment Setup
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    // Enable blending based on whether blend operations are set
    bool blendingEnabled = 
        settings.settings.blendState.alpha.operation != BlendOperation_Add || 
        settings.settings.blendState.alpha.srcFactor != BlendFactor_One ||
        settings.settings.blendState.alpha.dstFactor != BlendFactor_Zero ||
        settings.settings.blendState.alpha.operation != BlendOperation_Add ||
        settings.settings.blendState.alpha.srcFactor != BlendFactor_One ||
        settings.settings.blendState.alpha.dstFactor != BlendFactor_Zero;

    colorBlendAttachment.blendEnable = blendingEnabled ? VK_TRUE : VK_FALSE;

    if (blendingEnabled) {
        // Configure blending for color
        colorBlendAttachment.srcColorBlendFactor = toVulkanBlendFactor   (settings.settings.blendState.color.srcFactor);
        colorBlendAttachment.dstColorBlendFactor = toVulkanBlendFactor   (settings.settings.blendState.color.dstFactor);
        colorBlendAttachment.colorBlendOp =        toVulkanBlendOperation(settings.settings.blendState.color.operation);
        
        // Configure blending for alpha
        colorBlendAttachment.srcAlphaBlendFactor = toVulkanBlendFactor   (settings.settings.blendState.alpha.srcFactor);
        colorBlendAttachment.dstAlphaBlendFactor = toVulkanBlendFactor   (settings.settings.blendState.alpha.dstFactor);
        colorBlendAttachment.alphaBlendOp =        toVulkanBlendOperation(settings.settings.blendState.alpha.operation);
    }

    // Color Blending State Setup
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
    // Dynamic State Setup (optional based on RenderSettings)
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR,
        //VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, 
        //VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, 
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    
    // You can make dynamic states configurable via RenderSettings if needed
    dynamicState.dynamicStateCount = static_cast<uint32_t>(2);
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Pipeline Layout Setup
    
    
    // Graphics Pipeline Creation
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = settings.settings.depthTest ? &depthStencil : nullptr; // Enable depth stencil if needed
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = pllayout.layout->layout;

    RenderPassLayout rpLayout zeroinit;
    rpLayout.colorAttachmentCount = 1;
    rpLayout.depthAttachmentPresent = settings.settings.depthTest;
    
    rpLayout.colorAttachments[0].format = toVulkanPixelFormat(BGRA8);
    rpLayout.colorAttachments[0].loadop = LoadOp_Load;
    rpLayout.colorAttachments[0].storeop = StoreOp_Store;
    rpLayout.colorAttachments[0].sampleCount = settings.settings.sampleCount;
    
    if(rpLayout.colorAttachments[0].sampleCount > 1){
        rpLayout.colorResolveAttachments[0].format = toVulkanPixelFormat(BGRA8);
        rpLayout.colorResolveAttachments[0].loadop = LoadOp_Load;
        rpLayout.colorResolveAttachments[0].storeop = StoreOp_Store;
        rpLayout.colorResolveAttachments[0].sampleCount = 1;
    }
    else{
        //rpLayout.colorResolveIndex = VK_ATTACHMENT_UNUSED;
    }

    if(settings.settings.depthTest){
        rpLayout.depthAttachment.format = toVulkanPixelFormat(Depth32);
        rpLayout.depthAttachment.loadop = LoadOp_Load;
        rpLayout.depthAttachment.storeop = StoreOp_Store;
        rpLayout.depthAttachment.sampleCount = settings.settings.sampleCount;
    }
    
    
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    VkPipelineRenderingCreateInfo rci zeroinit;
    rci.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rci.colorAttachmentCount = 1;
    VkFormat colorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    rci.pColorAttachmentFormats = &colorAttachmentFormat;
    rci.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    pipelineInfo.pNext = &rci;
    #else
    LayoutedRenderPass rp = LoadRenderPassFromLayout(g_vulkanstate.device, rpLayout);
    pipelineInfo.renderPass = rp.renderPass;
    pipelineInfo.subpass = 0;
    #endif
    
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    ret->dynamicStates = dynamicStates;
    ret->layout = pllayout.layout;
    wgvkPipelineLayoutAddRef(pllayout.layout);
    if (vkCreateGraphicsPipelines(g_vulkanstate.device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->renderPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Trianglelist pipiline creation failed");
    }
    return ret;
}

/*
extern "C" RenderPipelineQuartet GetPipelinesForLayoutSet(DescribedPipeline* ret, const VertexBufferLayoutSet vls){

    TRACELOG(LOG_INFO, "Generating new pipelines");

    auto& settings = ret->state;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = (VkShaderModule)ret->shaderModule.stages[ShaderStage_Vertex].module;
    vertShaderStageInfo.pName = ret->shaderModule.reflectionInfo.ep[ShaderStage_Vertex].name;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = (VkShaderModule)ret->shaderModule.stages[ShaderStage_Fragment].module;
    fragShaderStageInfo.pName = ret->shaderModule.reflectionInfo.ep[ShaderStage_Fragment].name;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex Input Setup
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    
    auto [vad, vbd] = genericVertexLayoutSetToVulkan(vls);

    vertexInputInfo.vertexBindingDescriptionCount = vbd.size();
    vertexInputInfo.vertexAttributeDescriptionCount = vad.size();
    vertexInputInfo.pVertexAttributeDescriptions = vad.data();
    vertexInputInfo.pVertexBindingDescriptions = vbd.data();

    // Input Assembly Setup
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport and Scissor Setup
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    VkRect2D scissor{0, 0, g_vulkanstate.surface.surfaceConfig.width, g_vulkanstate.surface.surfaceConfig.height};
    VkViewport fullView{
        0.0f, 
        (float)g_vulkanstate. surface.surfaceConfig.height, 
        (float)g_vulkanstate. surface.surfaceConfig.width, 
        -((float)g_vulkanstate.surface.surfaceConfig.height), 
        0.0f, 
        1.0f
    };
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &fullView;

    // Rasterization State Setup
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;

    // Incorporate face culling from RenderSettings
    if (settings.faceCull) {
        rasterizer.cullMode = VK_CULL_MODE_NONE;//VK_CULL_MODE_BACK_BIT; // You can make this configurable
    } else {
        rasterizer.cullMode = VK_CULL_MODE_NONE;
    }

    // Set front face based on RenderSettings
    rasterizer.frontFace = toVulkanFrontFace(settings.frontFace);

    rasterizer.depthBiasEnable = VK_FALSE;

    // Depth Stencil State Setup
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = settings.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = settings.depthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = toVulkanCompareFunction((CompareFunction)settings.depthCompare);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    // Multisampling State Setup
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    
    // Map sampleCount from RenderSettings to VkSampleCountFlagBits
    switch (settings.sampleCount) {
        case 1: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; break;
        case 2: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT; break;
        case 4: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT; break;
        case 8: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT; break;
        case 16: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT; break;
        case 32: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT; break;
        case 64: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT; break;
        default: multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; break;
    }

    // Color Blend Attachment Setup
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    // Enable blending based on whether blend operations are set
    bool blendingEnabled = 
        settings.blendOperationAlpha != BlendOperation_Add || 
        settings.blendFactorSrcAlpha != BlendFactor_One ||
        settings.blendFactorDstAlpha != BlendFactor_Zero ||
        settings.blendOperationColor != BlendOperation_Add ||
        settings.blendFactorSrcColor != BlendFactor_One ||
        settings.blendFactorDstColor != BlendFactor_Zero;

    colorBlendAttachment.blendEnable = blendingEnabled ? VK_TRUE : VK_FALSE;

    if (blendingEnabled) {
        // Configure blending for color
        colorBlendAttachment.srcColorBlendFactor = toVulkanBlendFactor(settings.blendFactorSrcColor);
        colorBlendAttachment.dstColorBlendFactor = toVulkanBlendFactor(settings.blendFactorDstColor);
        colorBlendAttachment.colorBlendOp =        toVulkanBlendOperation(settings.blendOperationColor);
        
        // Configure blending for alpha
        colorBlendAttachment.srcAlphaBlendFactor = toVulkanBlendFactor(settings.blendFactorSrcAlpha);
        colorBlendAttachment.dstAlphaBlendFactor = toVulkanBlendFactor(settings.blendFactorDstAlpha);
        colorBlendAttachment.alphaBlendOp =        toVulkanBlendOperation(settings.blendOperationAlpha);
    }

    // Color Blending State Setup
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
    
    // Dynamic State Setup (optional based on RenderSettings)
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR,
        //VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY, 
        //VK_DYNAMIC_STATE_VERTEX_INPUT_EXT, 
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    
    // You can make dynamic states configurable via RenderSettings if needed
    dynamicState.dynamicStateCount = static_cast<uint32_t>(2);
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Pipeline Layout Setup
    
    
    // Graphics Pipeline Creation
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = settings.depthTest ? &depthStencil : nullptr; // Enable depth stencil if needed
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = (VkPipelineLayout)ret->layout.layout;

    RenderPassLayout rpLayout zeroinit;
    rpLayout.colorAttachmentCount = 1;
    rpLayout.depthAttachmentPresent = settings.depthTest;
    
    rpLayout.colorAttachments[0].format = toVulkanPixelFormat(BGRA8);
    rpLayout.colorAttachments[0].loadop = LoadOp_Load;
    rpLayout.colorAttachments[0].storeop = StoreOp_Store;
    rpLayout.colorAttachments[0].sampleCount = settings.sampleCount;
    
    if(settings.depthTest){
        rpLayout.depthAttachment.format = toVulkanPixelFormat(Depth32);
        rpLayout.depthAttachment.loadop = LoadOp_Load;
        rpLayout.depthAttachment.storeop = StoreOp_Store;
        rpLayout.depthAttachment.sampleCount = settings.sampleCount;
    }
    if(settings.sampleCount > 1){
        rpLayout.colorAttachments[1].format = toVulkanPixelFormat(BGRA8);
        rpLayout.colorAttachments[1].loadop = LoadOp_Load;
        rpLayout.colorAttachments[1].storeop = StoreOp_Store;
        rpLayout.colorAttachments[1].sampleCount = 1;
        rpLayout.colorResolveIndex = 1;
    }
    else{
        rpLayout.colorResolveIndex = VK_ATTACHMENT_UNUSED;
    }
    #if VULKAN_USE_DYNAMIC_RENDERING == 1
    VkPipelineRenderingCreateInfo rci zeroinit;
    rci.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    rci.colorAttachmentCount = 1;
    VkFormat colorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;
    rci.pColorAttachmentFormats = &colorAttachmentFormat;
    rci.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;
    pipelineInfo.pNext = &rci;
    #else
    VkRenderPass rp = LoadRenderPassFromLayout(g_vulkanstate.device, rpLayout);
    pipelineInfo.renderPass = rp;
    pipelineInfo.subpass = 0;
    #endif
    
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    RenderPipelineQuartet quartet{
        .pipeline_TriangleList = callocnew(WGVKRenderPipelineImpl),
        .pipeline_TriangleStrip = callocnew(WGVKRenderPipelineImpl),
        .pipeline_LineList = callocnew(WGVKRenderPipelineImpl),
        .pipeline_PointList = callocnew(WGVKRenderPipelineImpl)
    };
    quartet.pipeline_TriangleList->layout = pipelineInfo.layout;
    quartet.pipeline_TriangleStrip->layout = pipelineInfo.layout;
    quartet.pipeline_LineList->layout = pipelineInfo.layout;
    quartet.pipeline_PointList->layout = pipelineInfo.layout;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&quartet.pipeline_TriangleList->renderPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&quartet.pipeline_TriangleStrip->renderPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&quartet.pipeline_LineList->renderPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&quartet.pipeline_PointList->renderPipeline) != VK_SUCCESS) {
        TRACELOG(LOG_FATAL, "Trianglelist pipiline creation failed");
    }
    ret->createdPipelines->pipelines.emplace(vls, quartet);

    return quartet;
}
*/

//extern "C" void UpdatePipeline(DescribedPipeline *pl){
//    pl->quartet = GetPipelinesForLayoutSet(pl, pl->vertexLayout);
//}

//extern "C" RenderPipelineQuartet GetPipelinesForLayout(DescribedPipeline *ret, const std::vector<AttributeAndResidence>& attribs){
//    
//    VertexBufferLayoutSet layoutset = getBufferLayoutRepresentation(attribs.data(), attribs.size());
//
//    auto ait = ret->createdPipelines->pipelines.find(layoutset);
//    if(ait != ret->createdPipelines->pipelines.end()){
//        UnloadBufferLayoutSet(layoutset);
//        return ait->second;
//    }
//    return GetPipelinesForLayoutSet(ret, layoutset);
//}

extern "C" DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    ShaderSources sources = dualStage(shaderSource, sourceTypeWGSL, ShaderStage_Vertex, ShaderStage_Fragment);
    
    DescribedShaderModule mod = LoadShaderModule(sources);
    //std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>> attribs = getAttributes(shaderSource);
    return LoadPipelineMod(mod, attribs, attribCount, uniforms, uniformCount, settings);
}
extern "C" DescribedPipeline* LoadPipeline(const char* shaderSource){
    ShaderSources sources = dualStage(shaderSource, sourceTypeWGSL, ShaderStage_Vertex, ShaderStage_Fragment);
    std::unordered_map<std::string, std::pair<VertexFormat, uint32_t>> attribs = getAttributesWGSL(sources);
    std::vector<AttributeAndResidence> allAttribsInOneBuffer;

    allAttribsInOneBuffer.reserve(attribs.size());
    uint32_t offset = 0;
    for(const auto& [name, attr] : attribs){
        const auto& [format, location] = attr;
        allAttribsInOneBuffer.push_back(AttributeAndResidence{
            .attr = VertexAttribute{
                .nextInChain = nullptr,
                .format = format,
                .offset = offset,
                .shaderLocation = location
            },
            .bufferSlot = 0,
            .stepMode = VertexStepMode_Vertex,
            .enabled = true}
        );
        offset += attributeSize(format);
    }
    

    auto bindings = getBindings(sources);

    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    return LoadPipelineEx(shaderSource, allAttribsInOneBuffer.data(), allAttribsInOneBuffer.size(), values.data(), values.size(), GetDefaultSettings());
}

extern "C" void UpdatePipelineWithNewLayout(DescribedPipeline* ret, const std::vector<AttributeAndResidence>& attributes){
    // Shader Stage Setup
    ret->state.vertexAttributes = attributes;
    ret->activePipeline = ret->pipelineCache.getOrCreate(ret->state, ret->shaderModule, ret->bglayout, ret->layout);
    //VertexBufferLayoutSet layoutset = getBufferLayoutRepresentation(attributes.data(), attributes.size());
    //auto it = ret->createdPipelines->pipelines.find(layoutset);
    //if(it != ret->createdPipelines->pipelines.end()){
    //    UnloadBufferLayoutSet(layoutset);
    //    ret->quartet = it->second;
    //    return;
    //}

    //RenderPipelineQuartet quartet = GetPipelinesForLayout(ret, attributes);

    //ret->quartet = quartet;
    //ret->createdPipelines->pipelines[layoutset] = ret->quartet;
    //UnloadBufferLayoutSet(layoutset);
}

extern "C" DescribedPipeline* LoadPipelineMod(DescribedShaderModule mod, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* ret = callocnewpp(DescribedPipeline);
    ret->state.settings = settings;
    ret->state.vertexAttributes = std::vector<AttributeAndResidence>(attribs, attribs + attribCount); 
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, false);
    ret->shaderModule = mod;
    //auto [spirV, spirF] = glsl_to_spirv(vsSource, fsSource);
    //ret->sh = LoadShaderModuleFromSPIRV_Vk(spirV.data(), spirV.size() * 4, spirF.data(), spirF.size() * 4);
    
    WGVKPipelineLayoutDescriptor pldesc zeroinit;
    pldesc.bindGroupLayoutCount = 1;
    WGVKBindGroupLayout bgls[1] = {ret->bglayout.layout};
    pldesc.bindGroupLayouts = bgls;

    ret->layout.layout = wgvkDeviceCreatePipelineLayout(g_vulkanstate.device, &pldesc);
    std::vector<ResourceDescriptor> bge(uniformCount);

    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = ResourceDescriptor{};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge.data(), bge.size());
    
    //ret->vertexLayout = getBufferLayoutRepresentation(attribs, attribCount);
    
    //UpdatePipeline(ret);//, std::vector<AttributeAndResidence>{attribs, attribs + attribCount});    
    
    return ret;

}




DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    
    ShaderSources sources zeroinit;
    sources.sourceCount = 2;
    sources.sources[0].data = vsSource;
    sources.sources[0].sizeInBytes = std::strlen(vsSource);
    sources.sources[0].stageMask = ShaderStageMask_Vertex;
    
    sources.sources[1].data = fsSource;
    sources.sources[1].sizeInBytes = std::strlen(fsSource);
    sources.sources[1].stageMask = ShaderStageMask_Fragment;

    DescribedShaderModule sh = LoadShaderModule(sources);
    return LoadPipelineMod(sh, vao->attributes.data(), vao->attributes.size(), uniforms, uniformCount, settings);
    
}

extern "C" DescribedPipeline* LoadPipelineForVAOEx(ShaderSources sources, VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    //detectShaderLanguage()
    
    DescribedShaderModule module = LoadShaderModule(sources);
    
    DescribedPipeline* pl = LoadPipelineMod(module, vao->attributes.data(), vao->attributes.size(), uniforms, uniformCount, settings);
    //DescribedPipeline* pl = LoadPipelineEx(shaderSource, nullptr, 0, uniforms, uniformCount, settings);
    PreparePipeline(pl, vao);
    return pl;
}

extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t location, ResourceDescriptor entry){

    WGVKBindGroup bgImpl = (WGVKBindGroup)bg->bindGroup;
    uint32_t index = ~0u;
    for(uint32_t i = 0;i < bg->entryCount;i++){
        if(bg->entries[i].binding == location){
            index = i; break;
        }
    }
    rassert(index != ~0u, "No entry was found with given location");

    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    if(bg->entries[index].buffer){
        WGVKBuffer wBuffer = (WGVKBuffer)bg->entries[index].buffer;
        wgvkBufferRelease(wBuffer);
    }
    else if(bg->entries[index].textureView){
        //TODO: currently not the case anyway, but this is nadinöf
        wgvkReleaseTextureView((WGVKTextureView)bg->entries[index].textureView);
    }
    else if(bg->entries[index].sampler){
        // TODO
    }
    if(entry.buffer){
        wgvkBufferAddRef((WGVKBuffer)entry.buffer);
    }
    else if(entry.textureView){
        wgvkTextureViewAddRef((WGVKTextureView)entry.textureView);
    }
    else if(entry.sampler){
        // TODO
    }
    else{
        TRACELOG(LOG_FATAL, "Invalid ResourceDescriptor");
    }

    bg->entries[index] = entry;
    //bg->descriptorHash ^= bgEntryHash(bg->entries[index]);
    WGVKBindGroup wB = (WGVKBindGroup)bg->bindGroup;
    if(bg->bindGroup && wB->refCount > 1){
        wgvkBindGroupRelease(wB);
        bg->bindGroup = nullptr;
    }
    //else if(!bg->needsUpdate && bg->bindGroup){
    //    g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //    bg->bindGroup = nullptr;
    //}
    bg->needsUpdate = true;
    
    //bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
}

void UpdateBindGroup(DescribedBindGroup* bg){
    const auto* layout = bg->layout;
    const auto* layoutlayout = layout->layout;
    rassert(layout != nullptr, "DescribedBindGroupLayout is nullptr");
    rassert(layoutlayout != nullptr, "WGVKBindGroupLayout is nullptr");
    if(bg->needsUpdate == false)return;
    WGVKBindGroupDescriptor bgdesc zeroinit;

    bgdesc.entryCount = bg->entryCount;
    bgdesc.entries = bg->entries;
    WGVKBindGroupLayout wvl = (WGVKBindGroupLayout)bg->layout->layout;
    bgdesc.layout = wvl;
    std::vector<ResourceTypeDescriptor> ldtypes(wvl->entries, wvl->entries + wvl->entryCount);
    //bgdesc.entries 
    if(bg->bindGroup && ((WGVKBindGroup)bg->bindGroup)->refCount == 1){  
        WGVKBindGroup writeTo = (WGVKBindGroup)bg->bindGroup;      
        wgvkWriteBindGroup(g_vulkanstate.device, writeTo, &bgdesc);
    }
    else{
        if(bg->bindGroup){
            TRACELOG(LOG_WARNING, "Weird. This shouldn't be the case");
            wgvkBindGroupRelease((WGVKBindGroup)bg->bindGroup);
        }
        bg->bindGroup = wgvkDeviceCreateBindGroup(g_vulkanstate.device, &bgdesc);
    }
    bg->needsUpdate = false;
}


//TODO: actually, one would need to iterate entries to find out where .binding == binding
void SetBindGroupTexture_Vk(DescribedBindGroup* bg, uint32_t binding, Texture tex){

    bg->entries[binding].textureView = (WGVKTextureView)tex.view;
    if(bg->bindGroup){
        wgvkBindGroupRelease((WGVKBindGroup)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}
void SetBindGroupBuffer_Vk(DescribedBindGroup* bg, uint32_t binding, DescribedBuffer* buf){
    
    //TODO: actually, one would need to iterate entries to find out where .binding == binding
    bg->entries[binding].buffer = (WGVKBuffer)buf->buffer;
    if(bg->bindGroup){
        wgvkBindGroupRelease((WGVKBindGroup)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}
void SetBindGroupSampler_Vk(DescribedBindGroup* bg, uint32_t binding, DescribedSampler buf){
    
    //TODO: actually, one would need to iterate entries to find out where .binding == binding
    bg->entries[binding].sampler = buf.sampler;

    if(bg->bindGroup){
        wgvkBindGroupRelease((WGVKBindGroup)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}


extern "C" DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount){
    DescribedComputePipeline* ret = callocnew(DescribedComputePipeline);
    ShaderSources sources = singleStage(shaderCode, sourceTypeWGSL, ShaderStage_Compute);
    DescribedShaderModule computeShaderModule = LoadShaderModule(sources);
    
    
    DescribedBindGroupLayout bgl = LoadBindGroupLayout(uniforms, uniformCount, true);
    VkPipelineLayoutCreateInfo lci zeroinit;
    lci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    lci.setLayoutCount = 1;
    lci.pSetLayouts = &(reinterpret_cast<WGVKBindGroupLayout>(bgl.layout)->layout);
    VkPipelineLayout layout zeroinit;
    VkResult pipelineCreationResult = vkCreatePipelineLayout(g_vulkanstate.device->device, &lci, nullptr, &layout);
    VkPipelineShaderStageCreateInfo computeStage zeroinit;
    computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeStage.module = (VkShaderModule)computeShaderModule.stages[ShaderStage_Compute].module;
    
    
    computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computeStage.pName = computeShaderModule.reflectionInfo.ep[ShaderStage_Compute].name;
    
    VkComputePipelineCreateInfo cpci zeroinit;
    cpci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    cpci.layout = layout;
    cpci.stage = computeStage;
    vkCreateComputePipelines(g_vulkanstate.device->device, nullptr, 1, &cpci, nullptr, (VkPipeline*)&ret->pipeline);
    ret->bglayout = bgl;
    std::vector<ResourceDescriptor> bge(uniformCount);
    
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = ResourceDescriptor{};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup(&ret->bglayout, bge.data(), bge.size());
    ret->shaderModule = computeShaderModule;
    ret->layout = layout;
    return ret;
}

extern "C" DescribedComputePipeline* LoadComputePipeline(const char* shaderCode){
    ShaderSources source = singleStage(shaderCode, sourceTypeWGSL, ShaderStage_Compute);
    std::unordered_map<std::string, ResourceTypeDescriptor> bindings = getBindings(source);
    std::vector<ResourceTypeDescriptor> values;
    values.reserve(bindings.size());
    for(const auto& [x,y] : bindings){
        values.push_back(y);
    }
    std::sort(values.begin(), values.end(),[](const ResourceTypeDescriptor& x, const ResourceTypeDescriptor& y){
        return x.location < y.location;
    });
    return LoadComputePipelineEx(shaderCode, values.data(), values.size());
}

//DescribedShaderModule LoadShaderModuleWGSL(ShaderSources sourcesWGSL){
//    ShaderSources spirv = wgsl_to_spirv(sourcesWGSL);
//    DescribedShaderModule mod = LoadShaderModuleSPIRV(spirv);
//    mod.reflectionInfo.uniforms = callocnewpp(StringToUniformMap);
//    mod.reflectionInfo.uniforms->uniforms = getBindings(sourcesWGSL);
//    return mod;
//}
