#ifndef TINT_C_API_H
#define TINT_C_API_H

#define SUPPORT_VULKAN_BACKEND 1
#include <wgvk_structs_impl.h>

RGAPI WGPUReflectionInfo reflectionInfo_wgsl_sync(WGPUStringView wgslSource);

typedef struct tc_SpirvBlob{
    size_t codeSize;
    uint32_t* code;
}tc_SpirvBlob;

RGAPI tc_SpirvBlob wgslToSpirv(const WGPUShaderSourceWGSL* source);
RGAPI void reflectionInfo_wgsl_free(WGPUReflectionInfo* reflectionInfo);

#endif