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
#include <wgvk.h>
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
    WGVKBindGroup bindGroup;
    const DescribedBindGroupLayout* layout;
    int needsUpdate; //Cached handles valid?

    //Description: entryCount and actual entries
    uint32_t entryCount;
    ResourceDescriptor* entries;


    uint64_t descriptorHash; //currently unused
    
}DescribedBindGroup;

typedef struct WGVKBlendComponent {
    BlendOperation operation;
    BlendFactor srcFactor;
    BlendFactor dstFactor;
} WGVKBlendComponent;

typedef struct WGVKBlendState {
    WGVKBlendComponent color;
    WGVKBlendComponent alpha;
} WGVKBlendState;

typedef struct VertexAttribute {
    void* nextInChain;
    VertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
}VertexAttribute;
typedef struct AttributeAndResidence{
    VertexAttribute attr;
    uint32_t bufferSlot; //Describes the actual buffer it will reside in
    VertexStepMode stepMode;
    uint32_t enabled;
}AttributeAndResidence;



typedef struct DescribedPipelineLayout{
    NativePipelineLayoutHandle layout;

    uint32_t bindgroupCount;
    DescribedBindGroupLayout bindgroupLayouts[4]; //4 is a reasonable max
}DescribedPipelineLayout;

typedef struct RenderPipelineQuartet{
    WGVKRenderPipeline pipeline_TriangleList;
    WGVKRenderPipeline pipeline_TriangleStrip;
    WGVKRenderPipeline pipeline_LineList;
    WGVKRenderPipeline pipeline_PointList;
}RenderPipelineQuartet;

/**
 * @brief Hashmap: std::string -> UniformDescriptor, only visible for C++
 */
typedef struct StringToUniformMap StringToUniformMap;
typedef struct StringToAttributeMap StringToAttributeMap;
/**
 * @brief Hashmap: std::pair<std::vector<AttributeAndResidence>, WGPUPrimitiveTopology> -> PipelineTriplet, only visible for C++
 */
typedef struct VertexStateToPipelineMap VertexStateToPipelineMap;
typedef enum ShaderSourceType{
    sourceTypeUnknown = 0,
    sourceTypeSPIRV   = 1,
    sourceTypeWGSL    = 2,
    sourceTypeGLSL    = 3,
}ShaderSourceType;





typedef struct DescribedPipeline DescribedPipeline;
typedef struct DescribedComputePipeline DescribedComputePipeline;
typedef struct DescribedRaytracingPipeline DescribedRaytracingPipeline;
typedef struct DescribedShaderModule DescribedShaderModule;

typedef struct DescribedBuffer DescribedBuffer;
#ifdef __cplusplus
struct UniformAccessor{
    uint32_t index;
    DescribedBindGroup* bindgroup;
    void operator=(DescribedBuffer* buf);
};
#endif


typedef struct VertexArray VertexArray;
EXTERN_C_BEGIN

    //inline void UsePipeline(WGPURenderPassEncoder rpEncoder, DescribedPipeline pl){
    //    wgpuRenderPassEncoderSetPipeline(rpEncoder, (WGPURenderPipeline)pl.quartet.pipeline_TriangleList);
    //}
    

    //DescribedBindGroupLayout LoadBindGroupLayout(const UniformDescriptor* uniforms, uint32_t uniformCount);
    void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
    

    DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const ResourceDescriptor* entries, size_t entryCount);
    WGVKBindGroup UpdateAndGetNativeBindGroup(DescribedBindGroup* bg);
    
    void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);
    void UpdateBindGroup(DescribedBindGroup* bg);
    void UnloadBindGroup(DescribedBindGroup* bg);
    
    DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
    DescribedComputePipeline* LoadComputePipeline(const char* shaderCode);
    DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount);

    DescribedRaytracingPipeline* LoadRaytracingPipeline(const DescribedShaderModule* shaderModule); 

    /**
     * @brief Loads a shader given vertex and fragment GLSL source.
     * 
     * @param vertexSource 
     * @param fragmentSource 
     * @return Shader 
     */
    //Shader LoadShaderFromMemory(const char* vertexSource, const char* fragmentSource);
EXTERN_C_END
#endif// PIPELINE_H
