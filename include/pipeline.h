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
#if SUPPORT_WGPU_BACKEND == 1
#include <webgpu/webgpu.h>
#else 
#include <wgvk.h>
#endif



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
    WGPUBool depthTest;
    WGPUBool faceCull;
    uint32_t sampleCount;
    uint32_t lineWidth;
    WGPUBlendState blendState;    
    WGPUFrontFace frontFace;
    WGPUCompareFunction depthCompare;
    #ifdef __cplusplus
    bool operator==(const RenderSettings& rs) const noexcept{
        return
               depthTest    == rs.depthTest     && 
               faceCull     == rs.faceCull      && 
               sampleCount  == rs.sampleCount   && 
               lineWidth    == rs.lineWidth     && 
               //blendState   == rs.blendState    && 
               frontFace    == rs.frontFace     && 
               depthCompare == rs.depthCompare  &&
        true;
    }
    #endif
}RenderSettings;




typedef struct DescribedBindGroupLayout{
    WGPUBindGroupLayout layout;
    uint32_t entryCount;
    WGPUBindGroupLayoutEntry* entries;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    //Cached handles
    WGPUBindGroup bindGroup;
    const DescribedBindGroupLayout* layout;
    int needsUpdate; //Cached handles valid?

    //Description: entryCount and actual entries
    uint32_t entryCount;
    WGPUBindGroupEntry* entries;


    uint64_t descriptorHash; //currently unused
    
}DescribedBindGroup;

typedef struct AttributeAndResidence{
    WGPUVertexAttribute attr;
    uint32_t bufferSlot; //Describes the actual buffer it will reside in
    WGPUVertexStepMode stepMode;
    uint32_t enabled;
}AttributeAndResidence;



typedef struct DescribedPipelineLayout{
    WGPUPipelineLayout layout;

    uint32_t bindgroupCount;
    DescribedBindGroupLayout bindgroupLayouts[4]; //4 is a reasonable max
}DescribedPipelineLayout;

typedef struct RenderPipelineQuartet{
    WGPURenderPipeline pipeline_TriangleList;
    WGPURenderPipeline pipeline_TriangleStrip;
    WGPURenderPipeline pipeline_LineList;
    WGPURenderPipeline pipeline_PointList;
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
    shaderSourceForce32 = 0x8fffffff
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

EXTERN_C_END
#endif// PIPELINE_H
