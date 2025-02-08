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


    ret.shaderModule = callocnew(VertexAndFragmentShaderModuleImpl);
    if (vkCreateShaderModule(g_vulkanstate.device, &vscreateInfo, nullptr, &((VertexAndFragmentShaderModule)ret.shaderModule)->vModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex shader module!");
    }
    if (vkCreateShaderModule(g_vulkanstate.device, &fscreateInfo, nullptr, &((VertexAndFragmentShaderModule)ret.shaderModule)->fModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create fragment shader module!");
    }

    return ret;
}

void UpdatePipeline_Vk(DescribedPipeline* ret, const VertexArray* vao){
    // Shader Stage Setup
    auto it = ret->createdPipelines->pipelines.find(vao->attributes);
    if(it != ret->createdPipelines->pipelines.end()){
        ret->quartet = it->second;
        return;
    }
    VertexAndFragmentShaderModule vfm = ((VertexAndFragmentShaderModule)ret->sh.shaderModule);
    auto& settings = ret->settings;
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vfm->vModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = vfm->fModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex Input Setup
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexBufferLayoutSet vls = getBufferLayoutRepresentation(vao->attributes.data(), vao->attributes.size());
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
    pipelineInfo.renderPass = g_vulkanstate.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(g_vulkanstate.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->quartet.pipeline_TriangleList) != VK_SUCCESS) {
        throw std::runtime_error("Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->quartet.pipeline_TriangleStrip) != VK_SUCCESS) {
        throw std::runtime_error("Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->quartet.pipeline_LineList) != VK_SUCCESS) {
        throw std::runtime_error("Trianglelist pipiline creation failed");
    }
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    if (vkCreateGraphicsPipelines(g_vulkanstate.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, (VkPipeline*)&ret->quartet.pipeline_PointList) != VK_SUCCESS) {
        throw std::runtime_error("Trianglelist pipiline creation failed");
    }
    ret->createdPipelines->pipelines[vao->attributes] = ret->quartet;
    
}


DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings){
    DescribedPipeline* ret = callocnew(DescribedPipeline);
    ret->settings = settings;
    ret->createdPipelines = callocnewpp(VertexStateToPipelineMap);
    ret->bglayout = LoadBindGroupLayout(uniforms, uniformCount, false);
    auto [spirV, spirF] = glsl_to_spirv(vsSource, fsSource);
    
    ret->sh = LoadShaderModuleFromSPIRV_Vk(spirV.data(), spirV.size() * 4, spirF.data(), spirF.size() * 4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = (VkDescriptorSetLayout*)&(ret->bglayout.layout);
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(g_vulkanstate.device, &pipelineLayoutInfo, nullptr, (VkPipelineLayout*)&ret->layout.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
    std::vector<ResourceDescriptor> bge(uniformCount);
    for(uint32_t i = 0;i < bge.size();i++){
        bge[i] = ResourceDescriptor{};
        bge[i].binding = uniforms[i].location;
    }
    ret->bindGroup = LoadBindGroup_Vk(&ret->bglayout, bge.data(), bge.size());
    UpdatePipeline_Vk(ret, vao);    
    
    return ret;
}

extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry){
    if(index >= bg->entryCount){
        TRACELOG(LOG_WARNING, "Trying to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->entryCount);
        //return;
    }
    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    
    if(bg->releaseOnClear & (1 << index)){
        //donotcache = true;
        if(bg->entries[index].buffer){
            wgvkReleaseBuffer((WGVKBuffer)bg->entries[index].buffer);
        }
        else if(bg->entries[index].textureView){
            //Todo: currently not the case anyway, but this is nadinöf
            //vkDestroyImageView((VkImageView)bg->entries[index].textureView);
        }
        else if(bg->entries[index].sampler){
            //wgpuSamplerRelease((WGPUSampler)bg->entries[index].sampler);
        }
        bg->releaseOnClear &= ~(1 << index);
    }
    bg->entries[index] = entry;
    //bg->descriptorHash ^= bgEntryHash(bg->entries[index]);

    //TODO don't release and recreate here or find something better
    if(true /*|| donotcache*/){
        if(bg->bindGroup)
            wgvkReleaseDescriptorSet((DescriptorSetHandle)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    //else if(!bg->needsUpdate && bg->bindGroup){
    //    g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //    bg->bindGroup = nullptr;
    //}
    bg->needsUpdate = true;
    
    //bg->bindGroup = wgpuDeviceCreateBindGroup(GetDevice(), &(bg->desc));
}
DescribedBindGroup LoadBindGroup_Vk(const DescribedBindGroupLayout* layout, const ResourceDescriptor* resources, uint32_t count){
    DescribedBindGroup ret{};
    VkDescriptorPool dpool{};
    VkDescriptorSet dset{};
    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    std::unordered_map<VkDescriptorType, uint32_t> counts;
    for(uint32_t i = 0;i < layout->entryCount;i++){
        ++counts[toVulkanResourceType(layout->entries[i].type)];
    }
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(counts.size());
    for(const auto& [t, s] : counts){
        sizes.push_back(VkDescriptorPoolSize{.type = t, .descriptorCount = s});
    }

    dpci.poolSizeCount = sizes.size();
    dpci.pPoolSizes = sizes.data();
    dpci.maxSets = 1;
    vkCreateDescriptorPool(g_vulkanstate.device, &dpci, nullptr, &dpool);
    
    //VkCopyDescriptorSet copy{};
    //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    
    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = dpool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = (VkDescriptorSetLayout*)&layout->layout;
    vkAllocateDescriptorSets(g_vulkanstate.device, &dsai, &dset);
    
    ret.layout = layout;
    ret.bindGroup = callocnewpp(DescriptorSetHandleImpl);
    ((DescriptorSetHandle)ret.bindGroup)->refCount = 1;

    ((DescriptorSetHandle)ret.bindGroup)->pool = dpool;
    ((DescriptorSetHandle)ret.bindGroup)->set = dset;

    ret.entries = (ResourceDescriptor*)std::calloc(count, sizeof(ResourceDescriptor));
    ret.entryCount = count;
    std::memcpy(ret.entries, resources, count * sizeof(ResourceDescriptor));
    //small_vector<VkWriteDescriptorSet> writes(count, VkWriteDescriptorSet{});
    //small_vector<VkDescriptorBufferInfo> bufferInfos(count, VkDescriptorBufferInfo{});
    //small_vector<VkDescriptorImageInfo> imageInfos(count, VkDescriptorImageInfo{});
    //for(uint32_t i = 0;i < count;i++){
    //    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    uint32_t binding = resources[i].binding;
    //    writes[i].dstBinding = binding;
    //    writes[i].dstSet = dset;
    //    writes[i].descriptorType = toVulkanResourceType(layout->entries[i].type);
    //    writes[i].descriptorCount = 1;
    //    if(layout->entries[i].type == uniform_buffer || layout->entries->type == storage_buffer){
    //        bufferInfos[i].buffer = (VkBuffer)resources[i].buffer;
    //        bufferInfos[i].offset = resources[i].offset;
    //        bufferInfos[i].range = resources[i].size;
    //        writes[i].pBufferInfo = bufferInfos.data() + i;
    //    }
//
    //    if(layout->entries[i].type == texture2d || layout->entries[i].type == texture3d){
    //        imageInfos[i].imageView = (VkImageView)resources[i].textureView;
    //        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //        writes[i].pImageInfo = imageInfos.data() + i;
    //    }
//
    //    if(layout->entries[i].type == texture_sampler){
    //        VkSampler vksampler = (VkSampler)ret.entries[i].sampler;
    //        imageInfos[i].sampler = vksampler;
    //        writes[i].pImageInfo = imageInfos.data() + i;
    //    }
    //}
//
    //vkUpdateDescriptorSets(g_vulkanstate.device, writes.size(), writes.data(), 0, nullptr);//count, &copy);
    ret.needsUpdate = true;
    return ret;
}

void UpdateBindGroup(DescribedBindGroup* bg){
    const auto* layout = bg->layout;
    if(bg->needsUpdate == false)return;
    bg->bindGroup = callocnewpp(DescriptorSetHandleImpl);
    DescriptorSetHandle dshandle = (DescriptorSetHandle)bg->bindGroup;
    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    std::unordered_map<VkDescriptorType, uint32_t> counts;
    for(uint32_t i = 0;i < layout->entryCount;i++){
        ++counts[toVulkanResourceType(layout->entries[i].type)];
    }
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(counts.size());
    for(const auto& [t, s] : counts){
        sizes.push_back(VkDescriptorPoolSize{.type = t, .descriptorCount = s});
    }

    dpci.poolSizeCount = sizes.size();
    dpci.pPoolSizes = sizes.data();
    dpci.maxSets = 1;
    vkCreateDescriptorPool(g_vulkanstate.device, &dpci, nullptr, &((DescriptorSetHandle)bg->bindGroup)->pool);
    
    //VkCopyDescriptorSet copy{};
    //copy.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
    
    VkDescriptorSetAllocateInfo dsai{};
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = ((DescriptorSetHandle)bg->bindGroup)->pool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = (VkDescriptorSetLayout*)&layout->layout;
    vkAllocateDescriptorSets(g_vulkanstate.device, &dsai, &((DescriptorSetHandle)bg->bindGroup)->set);



    uint32_t count = bg->entryCount;
    small_vector<VkWriteDescriptorSet> writes(count, VkWriteDescriptorSet{});
    small_vector<VkDescriptorBufferInfo> bufferInfos(count, VkDescriptorBufferInfo{});
    small_vector<VkDescriptorImageInfo> imageInfos(count, VkDescriptorImageInfo{});
    
    ((DescriptorSetHandle)bg->bindGroup)->refCount = 1;
    auto& l3 = layout->entries[3];
    for(uint32_t i = 0;i < count;i++){
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uint32_t binding = bg->entries[i].binding;
        writes[i].dstBinding = binding;
        writes[i].dstSet = ((DescriptorSetHandle)bg->bindGroup)->set;
        writes[i].descriptorType = toVulkanResourceType(bg->layout->entries[i].type);
        writes[i].descriptorCount = 1;

        if(layout->entries[i].type == uniform_buffer || layout->entries[i].type == storage_buffer){
            bufferInfos[i].buffer = ((WGVKBuffer)bg->entries[i].buffer)->buffer;
            bufferInfos[i].offset = bg->entries[i].offset;
            bufferInfos[i].range =  bg->entries[i].size;
            writes[i].pBufferInfo = bufferInfos.data() + i;
        }

        if(layout->entries[i].type == texture2d || layout->entries[i].type == texture3d){
            imageInfos[i].imageView = (VkImageView)(bg->entries[i].textureView);
            imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            writes[i].pImageInfo = imageInfos.data() + i;
        }

        if(layout->entries[i].type == texture_sampler){
            VkSampler vksampler = (VkSampler)bg->entries[i].sampler;
            imageInfos[i].sampler = vksampler;
            writes[i].pImageInfo = imageInfos.data() + i;
        }
    }
    vkUpdateDescriptorSets(g_vulkanstate.device, writes.size(), writes.data(), 0, nullptr);
    bg->needsUpdate = false;
}


//TODO: actually, one would need to iterate entries to find out where .binding == binding
void SetBindGroupTexture_Vk(DescribedBindGroup* bg, uint32_t binding, Texture tex){

    bg->entries[binding].textureView = tex.view;
    if(bg->bindGroup){
        wgvkReleaseDescriptorSet((DescriptorSetHandle)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}
void SetBindGroupBuffer_Vk(DescribedBindGroup* bg, uint32_t binding, DescribedBuffer* buf){
    
    //TODO: actually, one would need to iterate entries to find out where .binding == binding
    bg->entries[binding].buffer = buf->buffer;
    if(bg->bindGroup){
        wgvkReleaseDescriptorSet((DescriptorSetHandle)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}
void SetBindGroupSampler_Vk(DescribedBindGroup* bg, uint32_t binding, DescribedSampler buf){
    
    //TODO: actually, one would need to iterate entries to find out where .binding == binding
    bg->entries[binding].sampler = buf.sampler;

    if(bg->bindGroup){
        wgvkReleaseDescriptorSet((DescriptorSetHandle)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    bg->needsUpdate = true;
}
