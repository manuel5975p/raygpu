#include <vulkan/vulkan.h>
#include <raygpu.h>
#include <internals.hpp>
#include "vulkan_internals.hpp"

extern "C" DescribedShaderModule LoadShaderModuleFromSPIRV_Vk(const uint32_t* code, size_t codeSizeInBytes){
    DescribedShaderModule ret{};
    ret.source = calloc(codeSizeInBytes, 1);
    std::memcpy(const_cast<void*>(ret.source), code, codeSizeInBytes);
    ret.sourceLengthInBytes = codeSizeInBytes;

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = codeSizeInBytes / sizeof(uint32_t);
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code);
    if (vkCreateShaderModule(g_vulkanstate.device, &createInfo, nullptr, (VkShaderModule*)&ret.shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return ret;
}

DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, RenderSettings settings){
    DescribedPipeline* ret = callocnew(DescribedPipeline);
    ret->settings = settings;
    auto [spirV, spirF] = glsl_to_spirv(vsSource, fsSource);
    ret->sh = LoadShaderModuleFromSPIRV_Vk(spirV.data(), spirV.size());
    ret->sh = LoadShaderModuleFromSPIRV_Vk(spirF.data(), spirF.size());
}
