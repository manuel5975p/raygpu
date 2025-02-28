/*
 * MIT License
 * 
 * Copyright (c) 2025 @manuel5975p
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#ifndef PIPELINE_H
#define PIPELINE_H
#include <stdint.h>
#include <macros_and_constants.h>
#include <enum_translation.h>

typedef void* NativeShaderModuleHandle;
typedef void* NativeBindgroupLayoutHandle;
typedef void* NativeBindgroupHandle;
typedef void* NativePipelineLayoutHandle;
typedef void* NativeRenderPipelineHandle;
typedef void* NativeComputePipelineHandle;
typedef void* NativeSurfaceHandle;
typedef void* NativeSwapchainHandle;
typedef void* NativeBufferHandle;
typedef void* NativeMemoryHandle;
typedef void* NativeSamplerHandle;
typedef void* NativeImageHandle;
typedef void* NativeImageViewHandle;
typedef void* NativeCommandEncoderHandle;
typedef void* NativeRenderPassEncoderHandle;
typedef void* NativeComputePassEncoderHandle;



typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0

    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
}ResourceTypeDescriptor;

typedef struct ResourceDescriptor {
    void const * nextInChain; //hmm
    uint32_t binding;
    /*NULLABLE*/  NativeBufferHandle buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ NativeSamplerHandle sampler;
    /*NULLABLE*/ NativeImageViewHandle textureView;
} ResourceDescriptor;

typedef struct BufferDescriptor{
    BufferUsage usage;
    uint64_t size;
}BufferDescriptor;


//TODO: Stencil attachment
/**
 * @brief This struct handles the settings that GL handles with global functions
 * 
 * GL_DEPTH_TEST
 * GL_BLEND
 * GL_CULL_FACE
 * 
 */
typedef struct RenderSettings{
    bool depthTest;
    bool faceCull;
    uint8_t sampleCount                : 8;
    CompareFunction    depthCompare    : 8;
    BlendOperation blendOperationAlpha : 8;
    BlendFactor    blendFactorSrcAlpha : 8;
    BlendFactor    blendFactorDstAlpha : 8;
    BlendOperation blendOperationColor : 8;
    BlendFactor    blendFactorSrcColor : 8;
    BlendFactor    blendFactorDstColor : 8;
    FrontFace frontFace;

}RenderSettings;


/**
 * @brief This function determines compatibility between RenderSettings
 * @details 
 * The purpose of this function is to determine whether the attachment states of a renderpass and a pipeline is compatible.
 * For this, the multisample state and depth state need to match (and also stencil but not implemented right now) 
 * 
 * @param settings1 
 * @param settings2 
 * @return true 
 * @return false 
 */
static inline bool RenderSettingsComptatible(RenderSettings settings1, RenderSettings settings2){
    return settings1.sampleCount == settings2.sampleCount &&
           settings1.depthTest   == settings2.depthTest;
}

typedef struct DescribedBindGroupLayout{
    NativeBindgroupLayoutHandle layout;
    uint32_t entryCount;
    ResourceTypeDescriptor* entries;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    //Cached handles
    NativeBindgroupHandle bindGroup;
    const DescribedBindGroupLayout* layout;
    int needsUpdate; //Cached handles valid?

    //Description: entryCount and actual entries
    uint32_t entryCount;
    ResourceDescriptor* entries;


    uint64_t descriptorHash; //currently unused
    
}DescribedBindGroup;

typedef struct DescribedPipelineLayout{
    NativePipelineLayoutHandle layout;

    uint32_t bindgroupCount;
    DescribedBindGroupLayout bindgroupLayouts[4]; //4 is a reasonable max
}DescribedPipelineLayout;

typedef struct RenderPipelineQuartet{
    NativeRenderPipelineHandle pipeline_TriangleList;
    NativeRenderPipelineHandle pipeline_TriangleStrip;
    NativeRenderPipelineHandle pipeline_LineList;
    NativeRenderPipelineHandle pipeline_PointList;
}RenderPipelineQuartet;

/**
 * @brief Hashmap: std::string -> UniformDescriptor, only visible for C++
 */
typedef struct StringToUniformMap StringToUniformMap;
/**
 * @brief Hashmap: std::pair<std::vector<AttributeAndResidence>, WGPUPrimitiveTopology> -> PipelineTriplet, only visible for C++
 */
typedef struct VertexStateToPipelineMap VertexStateToPipelineMap;
typedef enum ShaderSourceType{
    sourceTypeUnknown = 0,
    sourceTypeSPIRV   = 1,
    sourceTypeWGSL    = 2,
    sourceTypeGLSL    = 3,
}ShaderLanguage;
typedef struct StageInModule{
    const char* entryPoint;
    NativeShaderModuleHandle module;
}StageInModule;

typedef struct DescribedShaderModule{
    StageInModule stages[16];
    StringToUniformMap* uniformLocations;
}DescribedShaderModule;

typedef struct VertexAttribute {
    void* nextInChain;
    VertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}VertexAttribute;

typedef struct VertexBufferLayout{
    uint64_t arrayStride;
    VertexStepMode stepMode;
    size_t attributeCount;
    VertexAttribute* attributes; //NOT owned, point to a VertexBufferLayoutSet::attributePool
}VertexBufferLayout;

typedef struct VertexBufferLayoutSet{
    uint32_t number_of_buffers;
    VertexBufferLayout* layouts;
    VertexAttribute* attributePool;
}VertexBufferLayoutSet;


typedef struct DescribedPipeline{
    RenderSettings settings;
    VertexBufferLayoutSet vertexLayout;

    //TODO: Multiple bindgrouplayouts
    DescribedShaderModule sh;
    DescribedBindGroup bindGroup;
    DescribedPipelineLayout layout;
    DescribedBindGroupLayout bglayout;

    //WGPUVertexBufferLayout* vbLayouts;
    //WGPUVertexAttribute* attributePool;
    //WGPURenderPipeline pipeline; //TriangleList
    //WGPURenderPipeline pipeline_TriangleStrip;
    //WGPURenderPipeline pipeline_LineList;
    //WGPUPrimitiveTopology lastUsedAs; //unused
    //WGPUBlendState* blendState;
    //WGPUFragmentState* fragmentState;
    //WGPUColorTargetState* colorTarget;
    //WGPUDepthStencilState* depthStencilState;
    RenderPipelineQuartet quartet;
    VertexStateToPipelineMap* createdPipelines;
}DescribedPipeline;

typedef struct Shader {
    DescribedPipeline* id;  // Pipeline
    int *locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
} Shader;



typedef struct DescribedBuffer DescribedBuffer;
#ifdef __cplusplus
struct UniformAccessor{
    uint32_t index;
    DescribedBindGroup* bindgroup;
    void operator=(DescribedBuffer* buf);
};
#endif
typedef struct DescribedComputePipeline{
    NativeComputePipelineHandle pipeline;
    NativePipelineLayoutHandle layout;
    DescribedBindGroupLayout bglayout;
    DescribedBindGroup bindGroup;
    DescribedShaderModule shaderModule;
    #ifdef __cplusplus
    UniformAccessor operator[](const char* uniformName);
    #endif
}DescribedComputePipeline;

typedef struct VertexArray VertexArray;
EXTERN_C_BEGIN

    //inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
    //    wgpuRenderPassEncoderSetPipeline(rpEncoder, (WGPURenderPipeline)pl.quartet.pipeline_TriangleList);
    //}
    

    //DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);
    void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
    

    DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const ResourceDescriptor* entries, size_t entryCount);
    NativeBindgroupHandle UpdateAndGetNativeBindGroup(DescribedBindGroup* bg);
    
    void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);
    void UpdateBindGroup(DescribedBindGroup* bg);
    void UnloadBindGroup(DescribedBindGroup* bg);
    
    DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
    DescribedComputePipeline* LoadComputePipeline(const char* shaderCode);
    DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount);
    /**
     * @brief Loads a shader given vertex and fragment GLSL source.
     * 
     * @param vertexSource 
     * @param fragmentSource 
     * @return Shader 
     */
    Shader LoadShaderFromMemory(const char* vertexSource, const char* fragmentSource);
EXTERN_C_END
#endif// PIPELINE_H
