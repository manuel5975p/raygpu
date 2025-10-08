#ifndef RAYGPU_H
#define RAYGPU_H
#include <config.h>
#if SUPPORT_WGPU_BACKEND == 1
    //#include <webgpu/webgpu.h>
#else
    //#include <webgpu/webgpu.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <macros_and_constants.h>
#include <mathutils.h>
#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
    #include <emscripten/emscripten.h>
    #if defined(ASSUME_EM_ASYNCIFY)
        #define emscripten_set_main_loop requestAnimationFrameLoopWithJSPI
        #define emscripten_set_main_loop_arg requestAnimationFrameLoopWithJSPIArg
    #endif
#endif
#if defined(__GNUC__) || defined(__clang__)
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define UNLIKELY(x) (x)
#endif
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

// Forward declarations
struct WGPUTextureImpl;
struct WGPUTextureViewImpl;
struct WGPUBufferImpl;
struct WGPUBindGroupImpl;
struct WGPUBindGroupLayoutImpl;
struct WGPUPipelineLayoutImpl;
struct WGPUBufferImpl;
struct WGPUFutureImpl;
struct WGPURenderPassEncoderImpl;
struct WGPUComputePassEncoderImpl;
struct WGPURenderBundleImpl;
struct WGPURenderBundleEncoderImpl;
struct WGPUCommandEncoderImpl;
struct WGPUCommandBufferImpl;
struct WGPUTextureImpl;
struct WGPUTextureViewImpl;
struct WGPUQueueImpl;
struct WGPUQuerySetImpl;
struct WGPUInstanceImpl;
struct WGPUAdapterImpl;
struct WGPUDeviceImpl;
struct WGPUSurfaceImpl;
struct WGPUShaderModuleImpl;
struct WGPURenderPipelineImpl;
struct WGPUComputePipelineImpl;
struct WGPUTopLevelAccelerationStructureImpl;
struct WGPUBottomLevelAccelerationStructureImpl;
struct WGPURaytracingPipelineImpl;
struct WGPURaytracingPassEncoderImpl;

// Typedefs for WGPU handles
typedef struct WGPUSurfaceImpl* WGPUSurface;
typedef struct WGPUBindGroupLayoutImpl* WGPUBindGroupLayout;
typedef struct WGPUPipelineLayoutImpl* WGPUPipelineLayout;
typedef struct WGPUBindGroupImpl* WGPUBindGroup;
typedef struct WGPUBufferImpl* WGPUBuffer;
typedef struct WGPUQueueImpl* WGPUQueue;
typedef struct WGPUQuerySetImpl* WGPUQuerySet;
typedef struct WGPUInstanceImpl* WGPUInstance;
typedef struct WGPUAdapterImpl* WGPUAdapter;
typedef struct WGPUDeviceImpl* WGPUDevice;
typedef struct WGPURenderPassEncoderImpl* WGPURenderPassEncoder;
typedef struct WGPUComputePassEncoderImpl* WGPUComputePassEncoder;
typedef struct WGPURenderBundleImpl* WGPURenderBundle;
typedef struct WGPURenderBundleEncoderImpl* WGPURenderBundleEncoder;
typedef struct WGPUCommandBufferImpl* WGPUCommandBuffer;
typedef struct WGPUCommandEncoderImpl* WGPUCommandEncoder;
typedef struct WGPUTextureImpl* WGPUTexture;
typedef struct WGPUTextureViewImpl* WGPUTextureView;
typedef struct WGPUSamplerImpl* WGPUSampler;
typedef struct WGPUFenceImpl* WGPUFence;
typedef struct WGPURenderPipelineImpl* WGPURenderPipeline;
typedef struct WGPUShaderModuleImpl* WGPUShaderModule;
typedef struct WGPUComputePipelineImpl* WGPUComputePipeline;
typedef struct WGPURayTracingAccelerationContainerImpl* WGPURayTracingAccelerationContainer;
typedef struct WGPURayTracingShaderBindingTableImpl* WGPURayTracingShaderBindingTable;
typedef struct WGPURaytracingPipelineImpl* WGPURaytracingPipeline;
typedef struct WGPURaytracingPassEncoderImpl* WGPURaytracingPassEncoder;

static inline uint64_t ROT_BYTES(uint64_t V, uint8_t C) {
 return ((V << C) | ((V) >> ((64 - C) & 63)));
}

typedef uint64_t RGFlags;

// RG Enums, replacing WGPU equivalents for the API

typedef enum RGShaderStageEnum{
    RGShaderStageEnum_Vertex,
    RGShaderStageEnum_Fragment,
    RGShaderStageEnum_Compute,
    RGShaderStageEnum_EnumCount,
    RGShaderStageEnum_Force32 = 0x7FFFFFFF
} RGShaderStageEnum;

typedef enum RGLoadOp {
    RGLoadOp_Undefined = 0x00000000,
    RGLoadOp_Clear = 0x00000001,
    RGLoadOp_Load = 0x00000002,
    RGLoadOp_Force32 = 0x7FFFFFFF
} RGLoadOp;

typedef enum RGStoreOp {
    RGStoreOp_Undefined = 0x00000000,
    RGStoreOp_Store = 0x00000001,
    RGStoreOp_Discard = 0x00000002,
    RGStoreOp_Force32 = 0x7FFFFFFF
} RGStoreOp;
typedef enum RGPresentMode {
    RGPresentMode_Undefined = 0,
    RGPresentMode_Fifo,
    RGPresentMode_FifoRelaxed,
    RGPresentMode_Immediate,
    RGPresentMode_Mailbox,
} RGPresentMode;

typedef enum RGVertexStepMode {
    RGVertexStepMode_Vertex = 0x00000000,
    RGVertexStepMode_Instance = 0x00000001,
    RGVertexStepMode_VertexBufferNotUsed = 0x00000002,
    RGVertexStepMode_Force32 = 0x7FFFFFFF
} RGVertexStepMode;

typedef enum RGVertexFormat {
    RGVertexFormat_Undefined = 0x00000000,
    RGVertexFormat_Uint8x2 = 0x00000001,
    RGVertexFormat_Uint8x4 = 0x00000002,
    RGVertexFormat_Sint8x2 = 0x00000003,
    RGVertexFormat_Sint8x4 = 0x00000004,
    RGVertexFormat_Unorm8x2 = 0x00000005,
    RGVertexFormat_Unorm8x4 = 0x00000006,
    RGVertexFormat_Snorm8x2 = 0x00000007,
    RGVertexFormat_Snorm8x4 = 0x00000008,
    RGVertexFormat_Uint16x2 = 0x00000009,
    RGVertexFormat_Uint16x4 = 0x0000000A,
    RGVertexFormat_Sint16x2 = 0x0000000B,
    RGVertexFormat_Sint16x4 = 0x0000000C,
    RGVertexFormat_Unorm16x2 = 0x0000000D,
    RGVertexFormat_Unorm16x4 = 0x0000000E,
    RGVertexFormat_Snorm16x2 = 0x0000000F,
    RGVertexFormat_Snorm16x4 = 0x00000010,
    RGVertexFormat_Float16x2 = 0x00000011,
    RGVertexFormat_Float16x4 = 0x00000012,
    RGVertexFormat_Float32 = 0x00000013,
    RGVertexFormat_Float32x2 = 0x00000014,
    RGVertexFormat_Float32x3 = 0x00000015,
    RGVertexFormat_Float32x4 = 0x00000016,
    RGVertexFormat_Uint32 = 0x00000017,
    RGVertexFormat_Uint32x2 = 0x00000018,
    RGVertexFormat_Uint32x3 = 0x00000019,
    RGVertexFormat_Uint32x4 = 0x0000001A,
    RGVertexFormat_Sint32 = 0x0000001B,
    RGVertexFormat_Sint32x2 = 0x0000001C,
    RGVertexFormat_Sint32x3 = 0x0000001D,
    RGVertexFormat_Sint32x4 = 0x0000001E,
    RGVertexFormat_Float16,
    RGVertexFormat_Uint16,
    RGVertexFormat_Sint16,
    RGVertexFormat_Force32 = 0x7FFFFFFF
} RGVertexFormat;


/// Yes, those need to be macros, otherwise they 
/// can't be used as `case` expressions in `switch` statements 
typedef RGFlags RGBufferUsage;
#define RGBufferUsage_None 0x00000000
#define RGBufferUsage_MapRead 0x00000001
#define RGBufferUsage_MapWrite 0x00000002
#define RGBufferUsage_CopySrc 0x00000004
#define RGBufferUsage_CopyDst 0x00000008
#define RGBufferUsage_Index 0x00000010
#define RGBufferUsage_Vertex 0x00000020
#define RGBufferUsage_Uniform 0x00000040
#define RGBufferUsage_Storage 0x00000080
#define RGBufferUsage_Indirect 0x00000100
#define RGBufferUsage_QueryResolve 0x00000200

typedef RGFlags RGTextureUsage;

#define RGTextureUsage_None 0x00000000
#define RGTextureUsage_CopySrc 0x00000001
#define RGTextureUsage_CopyDst 0x00000002
#define RGTextureUsage_TextureBinding 0x00000004
#define RGTextureUsage_StorageBinding 0x00000008
#define RGTextureUsage_RenderAttachment 0x00000010

typedef RGFlags RGShaderStage;

#define RGShaderStage_None 0x00000000
#define RGShaderStage_Vertex 0x00000001
#define RGShaderStage_Fragment 0x00000002
#define RGShaderStage_Compute 0x00000004

typedef enum PixelFormat {
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8_SRGB,
    PIXELFORMAT_UNCOMPRESSED_B8G8R8A8,
    PIXELFORMAT_UNCOMPRESSED_B8G8R8A8_SRGB,
    PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,
    PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
    PIXELFORMAT_DEPTH_24_PLUS = 0x28, //WGPUTextureFormat_Depth24Plus,
    PIXELFORMAT_DEPTH_32_FLOAT = 0x2A, //WGPUTextureFormat_Depth32Float,
    GRAYSCALE = 0x100000, // No WGPU_ equivalent
    RGB8 = 0x100001, // No WGPU_ equivalent
    PixelFormat_Force32 = 0x7FFFFFFF
} PixelFormat;

typedef enum TextureFilter {
    TEXTURE_FILTER_POINT = 0x00000000,
    TEXTURE_FILTER_BILINEAR = 0x00000001,
    TFilterMode_Force32 = 0x7FFFFFFF
} TextureFilter;

typedef enum FrontFace {
    FrontFace_Undefined = 0x00000000,
    FrontFace_CCW = 0x00000001,
    FrontFace_CW = 0x00000002,
    FrontFace_Force32 = 0x7FFFFFFF
} FrontFace;


typedef enum ShaderUniformDataType{
    SHADER_UNIFORM_FLOAT = 0,
    SHADER_UNIFORM_VEC2,
    SHADER_UNIFORM_VEC3,
    SHADER_UNIFORM_VEC4,
    SHADER_UNIFORM_INT,
    SHADER_UNIFORM_IVEC2,
    SHADER_UNIFORM_IVEC3,
    SHADER_UNIFORM_IVEC4,
    SHADER_UNIFORM_SAMPLER2D
} ShaderUniformDataType;

typedef enum IndexFormat {
    IndexFormat_Undefined = 0x00000000,
    IndexFormat_Uint16 = 0x00000001,
    IndexFormat_Uint32 = 0x00000002,
    IndexFormat_Force32 = 0x7FFFFFFF
} IndexFormat;

typedef enum TextureWrap {
    TEXTURE_WRAP_CLAMP = 0x1,
    TEXTURE_WRAP_REPEAT = 0x2,
    TEXTURE_WRAP_MIRROR_REPEAT = 0x3,
    TEXTURE_WRAP_MIRROR_CLAMP = 0x4,
} TextureWrap;

typedef enum PrimitiveType{
    RL_TRIANGLES,
    RL_TRIANGLE_STRIP,
    RL_QUADS,
    RL_LINES,
    RL_POINTS
}PrimitiveType;

typedef struct vertex{
    Vector3 pos;
    Vector2 uv ;
    Vector3 normal;
    Vector4 col;
}vertex;

typedef struct RGBA8Color{
    uint8_t r, g, b, a;
} RGBA8Color;

typedef RGBA8Color Color;

typedef struct BGRA8Color{
    uint8_t b, g, r, a;
} BGRA8Color;

typedef struct RGBA16FColor {
    uint16_t r, g, b, a;
}RGBA16FColor;

typedef struct RGBA32FColor{
    float r, g, b, a;
}RGBA32FColor;

typedef struct Image{
    void* data;
    uint32_t width;
    uint32_t height;
    int mipmaps;
    PixelFormat format;
    size_t rowStrideInBytes; // Does not have to match with width
                             // One reason for this is the fact that Texture to Buffer copy commands
                             // Have to have a multiple of 256 bytes as row length
}Image;

#ifndef MAX_MIP_LEVELS
#define MAX_MIP_LEVELS 8
#endif

typedef struct Texture2D{
    WGPUTexture id;
    WGPUTextureView view;
    WGPUTextureView mipViews[MAX_MIP_LEVELS];
    uint32_t width, height;
    PixelFormat format;
    uint32_t sampleCount;
    uint32_t mipmaps;
}Texture2D;

typedef Texture2D Texture;

typedef struct Texture3D{
    WGPUTexture id;
    WGPUTextureView view;
    uint32_t width, height, depth;
    PixelFormat format;
    uint32_t sampleCount;
}Texture3D;

typedef struct Texture2DArray{
    WGPUTexture id;
    WGPUTextureView view;
    uint32_t width, height, layerCount;
    PixelFormat format;
    uint32_t sampleCount;
}Texture2DArray;

typedef struct Rectangle {
    float x, y, width, height;
} Rectangle;

typedef struct RenderTexture{
    union{
        Texture texture; // texture == colorAttachments[0]
        Texture colorAttachments[MAX_COLOR_ATTACHMENTS];
    };
    Texture colorMultisample;
    Texture depth;
    uint32_t colorAttachmentCount;
}RenderTexture;

typedef enum RGBlendFactor {
    RGBlendFactor_Undefined = 0x00000000,
    RGBlendFactor_Zero = 0x00000001,
    RGBlendFactor_One = 0x00000002,
    RGBlendFactor_Src = 0x00000003,
    RGBlendFactor_OneMinusSrc = 0x00000004,
    RGBlendFactor_SrcAlpha = 0x00000005,
    RGBlendFactor_OneMinusSrcAlpha = 0x00000006,
    RGBlendFactor_Dst = 0x00000007,
    RGBlendFactor_OneMinusDst = 0x00000008,
    RGBlendFactor_DstAlpha = 0x00000009,
    RGBlendFactor_OneMinusDstAlpha = 0x0000000A,
    RGBlendFactor_SrcAlphaSaturated = 0x0000000B,
    RGBlendFactor_Constant = 0x0000000C,
    RGBlendFactor_OneMinusConstant = 0x0000000D,
    RGBlendFactor_Src1 = 0x0000000E,
    RGBlendFactor_OneMinusSrc1 = 0x0000000F,
    RGBlendFactor_Src1Alpha = 0x00000010,
    RGBlendFactor_OneMinusSrc1Alpha = 0x00000011,
    RGBlendFactor_Force32 = 0x7FFFFFFF
} RGBlendFactor;

typedef enum RGBlendOperation {
    RGBlendOperation_Undefined = 0x00000000,
    RGBlendOperation_Add = 0x00000001,
    RGBlendOperation_Subtract = 0x00000002,
    RGBlendOperation_ReverseSubtract = 0x00000003,
    RGBlendOperation_Min = 0x00000004,
    RGBlendOperation_Max = 0x00000005,
    RGBlendOperation_Force32 = 0x7FFFFFFF
} RGBlendOperation;

typedef enum RGFrontFace {
    RGFrontFace_Undefined = 0x00000000,
    RGFrontFace_CCW = 0x00000001,
    RGFrontFace_CW = 0x00000002,
    RGFrontFace_Force32 = 0x7FFFFFFF
} RGFrontFace;

typedef enum RGCompareFunction {
    RGCompareFunction_Undefined = 0x00000000,
    RGCompareFunction_Never = 0x00000001,
    RGCompareFunction_Less = 0x00000002,
    RGCompareFunction_Equal = 0x00000003,
    RGCompareFunction_LessEqual = 0x00000004,
    RGCompareFunction_Greater = 0x00000005,
    RGCompareFunction_NotEqual = 0x00000006,
    RGCompareFunction_GreaterEqual = 0x00000007,
    RGCompareFunction_Always = 0x00000008,
    RGCompareFunction_Force32 = 0x7FFFFFFF
} RGCompareFunction;

typedef struct RGBlendComponent{
    RGBlendOperation operation;
    RGBlendFactor srcFactor;
    RGBlendFactor dstFactor;
}RGBlendComponent;

typedef struct RGBlendState{
    RGBlendComponent color;
    RGBlendComponent alpha;
}RGBlendState;
typedef struct RGColor{
    double r,g,b,a;
}RGColor;

/**
 * @brief This struct handles the settings that GL handles with global functions
 *
 * GL_DEPTH_TEST
 * GL_BLEND
 * GL_CULL_FACE
 * */
typedef struct RenderSettings{
    Bool32 depthTest;
    Bool32 faceCull;
    uint32_t sampleCount;
    uint32_t lineWidth;
    RGBlendState blendState;
    RGFrontFace frontFace;
    RGCompareFunction depthCompare;
}RenderSettings;

typedef enum uniform_type {
    uniform_type_undefined,
    uniform_buffer,
    storage_buffer,
    texture2d,
    texture2d_array,
    storage_texture2d,
    texture_sampler,
    texture3d,
    storage_texture3d,
    storage_texture2d_array,
    acceleration_structure,
    combined_image_sampler,
    uniform_type_enumcount,
    uniform_type_force32 = 0x7fffffff} uniform_type;

typedef enum access_type {
    readonly,
    readwrite,
    writeonly
} access_type;

typedef enum format_or_sample_type {
    we_dont_know,
    sample_f32,
    sample_u32,
    sample_i32,
    format_r32float,
    format_r32uint,
    format_rgba8unorm,
    format_rgba32float
} format_or_sample_type;

typedef struct ResourceTypeDescriptor{
    uniform_type type;
    uint32_t minBindingSize;
    uint32_t location; //only for @binding attribute in bindgroup 0
    //Applicable for storage buffers and textures
    access_type access;
    format_or_sample_type fstype;
    RGShaderStage visibility;
}ResourceTypeDescriptor;

typedef struct ResourceDescriptor {
    void const * nextInChain; //hmm
    uint32_t binding;
    /*NULLABLE*/ WGPUBuffer buffer;
    uint64_t offset;
    uint64_t size;
    /*NULLABLE*/ WGPUSampler sampler;
    /*NULLABLE*/ WGPUTextureView textureView;
#if SUPPORT_VULKAN_BACKEND == 1
    /*NULLABLE*/ WGPURayTracingAccelerationContainer accelerationStructure;
#endif
} ResourceDescriptor;


typedef struct DescribedBindGroupLayout{
    void* layout;
    uint32_t entryCount;
    ResourceTypeDescriptor* entries;
}DescribedBindGroupLayout;

typedef struct DescribedBindGroup{
    //Cached handles
    WGPUBindGroup bindGroup;
    const DescribedBindGroupLayout* layout;
    int needsUpdate; //Cached handles valid?

    //Description: entryCount and actual entries
    uint32_t entryCount;
    ResourceDescriptor* entries;
    uint64_t descriptorHash; //currently unused
}DescribedBindGroup;

typedef struct RGVertexAttribute {
    RGVertexFormat format;
    uint64_t offset;
    uint32_t shaderLocation;
} RGVertexAttribute;


typedef struct AttributeAndResidence{
    RGVertexAttribute attr;
    uint32_t bufferSlot; //Describes the actual buffer it will reside in
    RGVertexStepMode stepMode;
    uint32_t enabled;
}AttributeAndResidence;

typedef struct DescribedPipelineLayout{
    WGPUPipelineLayout layout;
    uint32_t bindgroupCount;
    DescribedBindGroupLayout bindgroupLayouts[4]; //4 is a reasonable max
}DescribedPipelineLayout;

typedef struct DescribedRenderpass{
    RenderSettings settings;
    RGLoadOp colorLoadOp;
    RGStoreOp colorStoreOp;
    RGLoadOp depthLoadOp;
    RGStoreOp depthStoreOp;
    RGColor colorClear;
    float depthClear;
    RenderTexture renderTarget;
    WGPUCommandEncoder cmdEncoder;
    WGPURenderPassEncoder rpEncoder;
    void* VkRenderPass;
}DescribedRenderpass;

typedef struct DescribedComputePass{
    WGPUCommandEncoder cmdEncoder;
    WGPUComputePassEncoder cpEncoder;
    //WGPUComputePassDescriptor desc; <-- By omitting this we lose timestampwrites
}DescribedComputepass;



typedef struct DescribedBuffer{
    RGBufferUsage usage;
    uint64_t size;
    WGPUBuffer buffer;
}DescribedBuffer;

typedef struct DescribedSampler{
    WGPUSampler sampler;
    TextureWrap addressModeU;
    TextureWrap addressModeV;
    TextureWrap addressModeW;
    TextureFilter magFilter;
    TextureFilter minFilter;
    TextureFilter mipmapFilter;
    float lodMinClamp;
    float lodMaxClamp;
    RGCompareFunction compare;
    float maxAnisotropy;
}DescribedSampler;

typedef struct StagingBuffer{
    DescribedBuffer gpuUsable;
    DescribedBuffer mappable;
    void* map; //Nullable
}StagingBuffer;

typedef struct VertexArray VertexArray;
typedef struct GIFRecordState GIFRecordState;

// GlyphInfo, font characters glyphs info
typedef struct GlyphInfo {
    int value;              // Character value (Unicode)
    int offsetX;            // Character offset X when drawing
    int offsetY;            // Character offset Y when drawing
    int advanceX;           // Character advance position X
    Image image;            // Character image data
} GlyphInfo;

// Font, font texture and GlyphInfo array data
typedef struct Font {
    int baseSize;           // Base size (default chars height)
    int glyphCount;         // Number of glyph characters
    int glyphPadding;       // Padding around the glyph characters
    Texture texture;        // Texture atlas containing the glyphs
    Rectangle *recs;        // Rectangles in texture for the glyphs
    GlyphInfo *glyphs;      // Glyphs info data
} Font;

typedef struct Mesh {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)

    // Vertex attributes data
    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float *tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
    uint8_t *colors;          // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    uint32_t *indices;        // Vertex indices (in case vertex data comes indexed)

    // Animation vertex data (not supported yet)
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    unsigned char *boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning) (shader-location = 6)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)
    Matrix *boneMatrices;   // Bones animated transformation matrices
    int boneCount;          // Number of bones

    // WebGPU identifiers
    VertexArray* vao;
    DescribedBuffer** vbos;
    DescribedBuffer* ibo; //Index buffer object, optional
    DescribedBuffer* boneMatrixBuffer; //Storage buffer
    RGVertexFormat boneIDFormat; //Either RGVertexFormat_Uint8 or Uint16;
} Mesh;

typedef struct BoneInfo {
    char name[32];          // Bone name
    int parent;             // Bone parent
} BoneInfo;

typedef struct MaterialMap{
    Texture texture;
    Color color;
    float value;
}MaterialMap;

typedef struct Shader {
    unsigned int id;        // Pipeline
    int *locs;              // Shader locations array (RL_MAX_SHADER_LOCATIONS)
} Shader;

typedef struct Material{
    int id;
    MaterialMap* maps;
    Shader shader;
}Material;

typedef struct Model {
    Matrix transform;       // Local transform matrix

    int meshCount;          // Number of meshes
    int materialCount;      // Number of materials
    Mesh *meshes;           // Meshes array
    Material *materials;    // Materials array
    int *meshMaterial;      // Mesh material number

    // Animation data
    int boneCount;          // Number of bones
    BoneInfo *bones;        // Bones information (skeleton)
    Transform *bindPose;    // Bones base transformation (pose)
} Model;

// ModelAnimation
typedef struct ModelAnimation {
    int boneCount;          // Number of bones
    int frameCount;         // Number of animation frames
    BoneInfo *bones;        // Bones information (skeleton)
    Transform **framePoses; // Poses array by frame
    char name[32];          // Animation name
} ModelAnimation;

typedef struct BoundingBox {
    Vector3 min;    // Minimum vertex box-corner
    Vector3 max;    // Maximum vertex box-corner
} BoundingBox;

typedef struct StringToUniformMap StringToUniformMap;
typedef struct StringToAttributeMap StringToAttributeMap;
typedef struct VertexStateToPipelineMap VertexStateToPipelineMap;

typedef enum ShaderSourceType{
    sourceTypeUnknown = 0,
    sourceTypeSPIRV = 1,
    sourceTypeWGSL = 2,
    sourceTypeGLSL = 3,
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

#ifdef __cplusplus
#define externcvar extern "C"
#else
#define externcvar extern
#endif
externcvar Vector2 nextuv;
externcvar Vector3 nextnormal;
externcvar Vector4 nextcol;
externcvar StagingBuffer vboStaging; //unused
externcvar vertex* vboptr;
externcvar vertex* vboptr_base;
#if SUPPORT_VULKAN_BACKEND == 1
externcvar WGPUBuffer vbo_buf;
#endif
externcvar VertexArray* renderBatchVAO;
externcvar DescribedBuffer* renderBatchVBO;
externcvar PrimitiveType current_drawmode;
externcvar char telegrama_render1[];
externcvar char telegrama_render2[];
externcvar char telegrama_render3[];
externcvar size_t telegrama_render_size1;
externcvar size_t telegrama_render_size2;
externcvar size_t telegrama_render_size3;
//extern DescribedBuffer vbomap;

#ifdef __cplusplus
constexpr uint32_t LOCATION_NOT_FOUND = 0x0500;//GL_INVALID_ENUM;
constexpr Color LIGHTGRAY{ 200, 200, 200, 255 };
constexpr Color GRAY{ 130, 130, 130, 255 };
constexpr Color DARKGRAY{ 80, 80, 80, 255 };
constexpr Color YELLOW{ 253, 249, 0, 255 };
constexpr Color GOLD{ 255, 203, 0, 255 };
constexpr Color ORANGE{ 255, 161, 0, 255 };
constexpr Color PINK{ 255, 109, 194, 255 };
constexpr Color RED{ 230, 41, 55, 255 };
constexpr Color MAROON{ 190, 33, 55, 255 };
constexpr Color GREEN{ 0, 228, 48, 255 };
constexpr Color LIME{ 0, 158, 47, 255 };
constexpr Color DARKGREEN{ 0, 117, 44, 255 };
constexpr Color SKYBLUE{ 102, 191, 255, 255 };
constexpr Color BLUE{ 0, 121, 241, 255 };
constexpr Color DARKBLUE{ 0, 82, 172, 255 };
constexpr Color PURPLE{ 200, 122, 255, 255 };
constexpr Color VIOLET{ 135, 60, 190, 255 };
constexpr Color DARKPURPLE{112, 31, 126, 255 };
constexpr Color BEIGE{ 211, 176, 131, 255 };
constexpr Color BROWN{ 127, 106, 79, 255 };
constexpr Color DARKBROWN{ 76, 63, 47, 255 };
constexpr Color WHITE{ 255, 255, 255, 255 };
constexpr Color BLACK{ 0, 0, 0, 255 };
constexpr Color BLANK{ 0, 0, 0, 0 };
constexpr Color MAGENTA{ 255, 0, 255, 255 };
constexpr Color RAYWHITE{ 245, 245, 245, 255 };
#else // No C++
#define LOCATION_NOT_FOUND 0x0500
#define LIGHTGRAY CLITERAL(Color){ 200, 200, 200, 255 }    // Light Gray
#define GRAY CLITERAL(Color){ 130, 130, 130, 255 }          // Gray
#define DARKGRAY CLITERAL(Color){ 80, 80, 80, 255 }         // Dark Gray
#define YELLOW CLITERAL(Color){ 253, 249, 0, 255 }          // Yellow
#define GOLD CLITERAL(Color){ 255, 203, 0, 255 }            // Gold
#define ORANGE CLITERAL(Color){ 255, 161, 0, 255 }          // Orange
#define PINK CLITERAL(Color){ 255, 109, 194, 255 }          // Pink
#define RED CLITERAL(Color){ 230, 41, 55, 255 }             // Red
#define MAROON CLITERAL(Color){ 190, 33, 55, 255 }          // Maroon
#define GREEN CLITERAL(Color){ 0, 228, 48, 255 }            // Green
#define LIME CLITERAL(Color){ 0, 158, 47, 255 }             // Lime
#define DARKGREEN CLITERAL(Color){ 0, 117, 44, 255 }        // Dark Green
#define SKYBLUE CLITERAL(Color){ 102, 191, 255, 255 }       // Sky Blue
#define BLUE CLITERAL(Color){ 0, 121, 241, 255 }            // Blue
#define DARKBLUE CLITERAL(Color){ 0, 82, 172, 255 }         // Dark Blue
#define PURPLE CLITERAL(Color){ 200, 122, 255, 255 }        // Purple
#define VIOLET CLITERAL(Color){ 135, 60, 190, 255 }         // Violet
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }     // Dark Purple
#define BEIGE CLITERAL(Color){ 211, 176, 131, 255 }         // Beige
#define BROWN CLITERAL(Color){ 127, 106, 79, 255 }          // Brown
#define DARKBROWN CLITERAL(Color){ 76, 63, 47, 255 }        // Dark Brown
#define WHITE CLITERAL(Color){ 255, 255, 255, 255 }         // White
#define BLACK CLITERAL(Color){ 0, 0, 0, 255 }               // Black
#define BLANK CLITERAL(Color){ 0, 0, 0, 0 }                 // Blank (Transparent)
#define MAGENTA CLITERAL(Color){ 255, 0, 255, 255 }         // Magenta
#define RAYWHITE CLITERAL(Color){ 245, 245, 245, 255 }      // @raysan's own White (raylib logo)
#endif

typedef enum WindowFlag{
    FLAG_STDOUT_TO_FFMPEG = 0x02000000,      // Redirect tracelog to stderr and dump frames into stdout
                                            // Made for <program> | ffmpeg -f rawvideo -pix_fmt bgra -s 1920x1080 -i - output.mp4
    FLAG_HEADLESS = 0x01000000,             // Disable ALL windowing stuff (Runnable without Desktop Server)
    FLAG_VSYNC_LOWLATENCY_HINT = 0x00100000, // Set to try enabling Lowlatency tearless mailbox mode
    FLAG_VSYNC_HINT = 0x00000040,           // Set to try enabling V-Sync on GPU (i.e. PresentMode::Fifo for the surface)
    FLAG_MSAA_4X_HINT = 0x00000020,         // Forcefully Hint (actually force) 4x multisampling for the default color buffer and pipeline
    FLAG_WINDOW_RESIZABLE = 0x00000004,
    FLAG_FULLSCREEN_MODE = 0x00000002,      // Set to run program in fullscreen
} WindowFlag;

typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    KEY_BACK            = 4,        // Key: Android back button
    KEY_MENU            = 5,        // Key: Android menu button
    KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} KeyboardKey;


/**
 * @brief Limit enum to describe WebGPU requested device limits
 * */
typedef enum LimitType{
    maxTextureDimension1D,
    maxTextureDimension2D,
    maxTextureDimension3D,
    maxTextureArrayLayers,
    maxBindGroups,
    maxBindGroupsPlusVertexBuffers,
    maxBindingsPerBindGroup,
    maxDynamicUniformBuffersPerPipelineLayout,
    maxDynamicStorageBuffersPerPipelineLayout,
    maxSampledTexturesPerShaderStage,
    maxSamplersPerShaderStage,
    maxStorageBuffersPerShaderStage,
    maxStorageTexturesPerShaderStage,
    maxUniformBuffersPerShaderStage,
    maxUniformBufferBindingSize,
    maxStorageBufferBindingSize,
    minUniformBufferOffsetAlignment,
    minStorageBufferOffsetAlignment,
    maxVertexBuffers,
    maxBufferSize,
    maxVertexAttributes,
    maxVertexBufferArrayStride,
    maxInterStageShaderVariables,
    maxColorAttachments,
    maxColorAttachmentBytesPerSample,
    maxComputeWorkgroupStorageSize,
    maxComputeInvocationsPerWorkgroup,
    maxComputeWorkgroupSizeX,
    maxComputeWorkgroupSizeY,
    maxComputeWorkgroupSizeZ,
    maxComputeWorkgroupsPerDimension,
    maxImmediateSize,
}LimitType;

typedef enum {
    LOG_ALL = 0,        // Display all logs
    LOG_TRACE,          // Trace logging, intended for internal use only
    LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    LOG_INFO,           // Info logging, used for program execution info
    LOG_WARNING,        // Warning logging, used on recoverable failures
    LOG_ERROR,          // Error logging, used on unrecoverable failures
    LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    LOG_NONE            // Disable logging
} TraceLogLevel;


typedef enum {
    SHADER_LOC_VERTEX_POSITION = 0, // Shader location: vertex attribute: position
    SHADER_LOC_VERTEX_TEXCOORD01,   // Shader location: vertex attribute: texcoord01
    SHADER_LOC_VERTEX_TEXCOORD02,   // Shader location: vertex attribute: texcoord02
    SHADER_LOC_VERTEX_NORMAL,       // Shader location: vertex attribute: normal
    SHADER_LOC_VERTEX_TANGENT,      // Shader location: vertex attribute: tangent
    SHADER_LOC_VERTEX_COLOR,        // Shader location: vertex attribute: color
    SHADER_LOC_MATRIX_MVP,          // Shader location: matrix uniform: model-view-projection
    SHADER_LOC_MATRIX_VIEW,         // Shader location: matrix uniform: view (camera transform)
    SHADER_LOC_MATRIX_PROJECTION,   // Shader location: matrix uniform: projection
    SHADER_LOC_MATRIX_MODEL,        // Shader location: matrix uniform: model (transform)
    SHADER_LOC_MATRIX_NORMAL,       // Shader location: matrix uniform: normal
    SHADER_LOC_VECTOR_VIEW,         // Shader location: vector uniform: view
    SHADER_LOC_COLOR_DIFFUSE,       // Shader location: vector uniform: diffuse color
    SHADER_LOC_COLOR_SPECULAR,      // Shader location: vector uniform: specular color
    SHADER_LOC_COLOR_AMBIENT,       // Shader location: vector uniform: ambient color
    SHADER_LOC_MAP_ALBEDO,          // Shader location: sampler2d texture: albedo (same as: SHADER_LOC_MAP_DIFFUSE)
    SHADER_LOC_MAP_METALNESS,       // Shader location: sampler2d texture: metalness (same as: SHADER_LOC_MAP_SPECULAR)
    SHADER_LOC_MAP_NORMAL,          // Shader location: sampler2d texture: normal
    SHADER_LOC_MAP_ROUGHNESS,       // Shader location: sampler2d texture: roughness
    SHADER_LOC_MAP_OCCLUSION,       // Shader location: sampler2d texture: occlusion
    SHADER_LOC_MAP_EMISSION,        // Shader location: sampler2d texture: emission
    SHADER_LOC_MAP_HEIGHT,          // Shader location: sampler2d texture: height
    SHADER_LOC_MAP_CUBEMAP,         // Shader location: samplerCube texture: cubemap
    SHADER_LOC_MAP_IRRADIANCE,      // Shader location: samplerCube texture: irradiance
    SHADER_LOC_MAP_PREFILTER,       // Shader location: samplerCube texture: prefilter
    SHADER_LOC_MAP_BRDF,            // Shader location: sampler2d texture: brdf
    SHADER_LOC_VERTEX_BONEIDS,      // Shader location: vertex attribute: boneIds
    SHADER_LOC_VERTEX_BONEWEIGHTS,  // Shader location: vertex attribute: boneWeights
    SHADER_LOC_BONE_MATRICES,       // Shader location: array of matrices uniform: boneMatrices
    SHADER_LOC_VERTEX_INSTANCE_TX   // Shader location: vertex attribute: instanceTransform
} ShaderLocationIndex;

#define SHADER_LOC_MAP_DIFFUSE      SHADER_LOC_MAP_ALBEDO
#define SHADER_LOC_MAP_SPECULAR     SHADER_LOC_MAP_METALNESS

typedef enum {
    MATERIAL_MAP_ALBEDO = 0,     // Albedo material (same as: MATERIAL_MAP_DIFFUSE)
    MATERIAL_MAP_METALNESS,      // Metalness material (same as: MATERIAL_MAP_SPECULAR)
    MATERIAL_MAP_NORMAL,         // Normal material
    MATERIAL_MAP_ROUGHNESS,      // Roughness material
    MATERIAL_MAP_OCCLUSION,      // Ambient occlusion material
    MATERIAL_MAP_EMISSION,       // Emission material
    MATERIAL_MAP_HEIGHT,         // Heightmap material
    MATERIAL_MAP_CUBEMAP,        // Cubemap material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_IRRADIANCE,     // Irradiance material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_PREFILTER,      // Prefilter material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_BRDF,           // Brdf material
    MAX_MATERIAL_MAPS            // Amount of maps
} MaterialMapIndex;

#define MATERIAL_MAP_DIFFUSE      MATERIAL_MAP_ALBEDO
#define MATERIAL_MAP_SPECULAR     MATERIAL_MAP_METALNESS

typedef enum {
    FONT_DEFAULT = 0,   // Default font generation, anti-aliased
    FONT_BITMAP,        // Bitmap font generation, no anti-aliasing
    FONT_SDF            // SDF font generation, requires external shader
} FontType;

// Required for some backwards compatibility, e.g. raygui.h
#define MOUSE_MIDDLE_BUTTON MOUSE_BUTTON_MIDDLE
#define MOUSE_LEFT_BUTTON   MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON  MOUSE_BUTTON_RIGHT

typedef enum MouseButton{
    MOUSE_BUTTON_LEFT       = 0,    // Mouse button left
    MOUSE_BUTTON_RIGHT      = 1,    // Mouse button right
    MOUSE_BUTTON_MIDDLE     = 2,    // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE       = 3,    // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA      = 4,    // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD    = 5,    // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK       = 6,    // Mouse button back (advanced mouse device)
} MouseButton;

typedef enum BackendType {
    BackendType_Undefined = 0x00000000,
    BackendType_Null = 0x00000001,
    BackendType_WebGPU = 0x00000002,
    BackendType_D3D11 = 0x00000003,
    BackendType_D3D12 = 0x00000004,
    BackendType_Metal = 0x00000005,
    BackendType_Vulkan = 0x00000006,
    BackendType_OpenGL = 0x00000007,
    BackendType_OpenGLES = 0x00000008,
    BackendType_Force32 = 0x7FFFFFFF
} BackendType;

typedef enum {
    BLEND_ALPHA = 0,                // Blend textures considering alpha (default)
    BLEND_ADDITIVE,                 // Blend textures adding colors
    BLEND_MULTIPLIED,               // Blend textures multiplying colors
    BLEND_ADD_COLORS,               // Blend textures adding colors (alternative)
    BLEND_SUBTRACT_COLORS,          // Blend textures subtracting colors (alternative)
    BLEND_ALPHA_PREMULTIPLY,        // Blend premultiplied textures considering alpha
    BLEND_CUSTOM,                   // Blend textures using custom src/dst factors (use rlSetBlendFactors())
    BLEND_CUSTOM_SEPARATE,          // Blend textures using custom rgb/alpha separate src/dst factors (use rlSetBlendFactorsSeparate())
    INVALID_BLEND_MODE = 0x8FFFFFFF
} rlBlendMode;

typedef enum AdapterType{
    DISCRETE_GPU,
    INTEGRATED_GPU,
    SOFTWARE_RENDERER,
}AdapterType;

typedef const void* char_or_uint32_pointer;
typedef struct ShaderStageSource{
    char_or_uint32_pointer data;
    uint32_t sizeInBytes;
    RGShaderStage stageMask;
}ShaderStageSource;

/**
 * @brief Generalized shader source struct. Not all members need to be set.
 *
 * @details Currently supported settings are
 * 1. vertexSource and fragmentSource set with GLSL or WGSL sources
 * 2. vertexAndFragmentSource set with WGSL source
 * 3. computeSource set with GLSL or WGSL source
 */
typedef struct ShaderSources{
    uint32_t sourceCount;
    ShaderStageSource sources[RGShaderStageEnum_EnumCount];
    //const char* vertexSource;
    //const char* fragmentSource;
    //const char* vertexAndFragmentSource;
    //const char* computeSource
    ShaderSourceType language;
}ShaderSources;

typedef struct ShaderEntryPoint{
    RGShaderStageEnum stage;
    char name[16];
}ShaderEntryPoint;


typedef struct ReflectionVertexAttribute{
    char name[MAX_VERTEX_ATTRIBUTE_NAME_LENGTH + 1];
    RGVertexFormat format;
    uint32_t location;
}ReflectionVertexAttribute;

typedef struct ReflectionFragmentOutput{
    uint32_t number_of_components;
    format_or_sample_type type;
}ReflectionFragmentOutput;

/**
 * @brief Struct describing vertex input and fragment output of a vertex-fragment pipeline
 * */
typedef struct InOutAttributeInfo{
    uint32_t vertexAttributeCount;
    ReflectionVertexAttribute vertexAttributes[MAX_VERTEX_ATTRIBUTES];
    uint32_t attachmentCount;
    ReflectionFragmentOutput attachments[MAX_COLOR_ATTACHMENTS];
}InOutAttributeInfo;


typedef struct ShaderReflectionInfo{
    ShaderEntryPoint ep[16];
    StringToUniformMap* uniforms;
    InOutAttributeInfo attributes;
}ShaderReflectionInfo;

typedef struct StageInModule{
    WGPUShaderModule module;
}StageInModule;

typedef struct DescribedShaderModule{
    StageInModule stages[16];
    ShaderReflectionInfo reflectionInfo;
}DescribedShaderModule;

typedef struct VertexBufferLayoutSet VertexBufferLayoutSet;

typedef struct DescribedComputePipeline{
    WGPUComputePipeline pipeline;
    DescribedShaderModule shaderModule;
    WGPUPipelineLayout layout;
    DescribedBindGroupLayout bglayout;
    DescribedBindGroup bindGroup;
#ifdef __cplusplus
    UniformAccessor operator[](const char* uniformName);
#endif
}DescribedComputePipeline;

#if SUPPORT_VULKAN_BACKEND == 1
typedef struct DescribedRaytracingPipeline{
    WGPURaytracingPipeline pipeline;
    WGPUPipelineLayout layout;
    DescribedBindGroupLayout bglayout;
    DescribedBindGroup bindGroup;
    DescribedShaderModule shaderModule;
}DescribedRaytracingPipeline;
#endif

typedef struct FullSurface{
    WGPUSurface surface;
    Bool32 headless;
    RGPresentMode presentMode;
    PixelFormat format;
    uint32_t width;
    uint32_t height;
    RenderTexture renderTarget;
}FullSurface;

typedef enum windowType {
    windowType_glfw,
    windowType_rgfw,
    windowType_sdl2,
    windowType_sdl3
}windowType;

typedef struct {
    float axes[16];
    Vector2 position;
} PenInputState;

#define KEYS_MAX 512
#define MOUSEBTN_MAX 16
#define TOUCH_MAX 32
#define CHARQ_MAX 256
#define PEN_MAX 16

typedef struct {
    int64_t id;
    Vector2 pos;
} TouchPoint;

typedef struct window_input_state{
    Rectangle windowPosition;

    uint8_t keydownPrevious[KEYS_MAX];
    uint8_t keydown[KEYS_MAX];

    Vector2 scrollThisFrame, scrollPreviousFrame;
    float gestureZoomThisFrame;
    float gestureAngleThisFrame;

    Vector2 mousePosPrevious;
    Vector2 mousePos;
    int cursorInWindow;

    uint8_t mouseButtonDownPrevious[MOUSEBTN_MAX];
    uint8_t mouseButtonDown[MOUSEBTN_MAX];

    TouchPoint touchPoints[TOUCH_MAX];
    size_t touchPointsCount;

    int charQueue[CHARQ_MAX];
    size_t charQueueHead, charQueueTail, charQueueCount;

    struct {
        unsigned int key;
        PenInputState value;
        int used;
    } penStates[PEN_MAX];
    size_t penStatesCount;
} window_input_state;

typedef struct RGWindowImpl{
    void* handle;
    FullSurface surface;
    windowType type;
    double width, height;
    double scaleFactor;
    window_input_state input_state; // width/height in pixels are obtained by width * scaleFactor
}RGWindowImpl;

typedef struct RGWindowImpl* SubWindow;
typedef struct GLFWwindow GLFWwindow;
typedef void(*SetupFunction)();
typedef void(*RenderFunction)();

typedef struct ProgramInfo{
    const char* windowTitle;
    int windowWidth, windowHeight;
    SetupFunction setupFunction ;
    RenderFunction renderFunction;
}ProgramInfo;

EXTERN_C_BEGIN

RGAPI void InitWindow(int width, int height, const char* title);
RGAPI void InitProgram(ProgramInfo program);
RGAPI void requestAnimationFrameLoopWithJSPI(void (*callback)(void), int /* unused */, int/* unused */);
RGAPI void requestAnimationFrameLoopWithJSPIArg(void (*callback)(void*), void* userData, int/* unused */, int/* unused */);
RGAPI void SetWindowShouldClose(cwoid);
RGAPI bool WindowShouldClose(cwoid);
RGAPI SubWindow OpenSubWindow (int width, int height, const char* title);
RGAPI SubWindow InitWindow_SDL2 (int width, int height, const char* title);
RGAPI SubWindow InitWindow_SDL3 (int width, int height, const char* title);
RGAPI void CloseSubWindow (SubWindow subWindow);
RGAPI FullSurface CompleteSurface (void* nsurface, int width, int height);
RGAPI FullSurface CreateHeadlessSurface(int width, int height, PixelFormat format);
RGAPI void ResizeSurface (FullSurface* fsurface, int width, int height);
RGAPI void GetNewTexture (FullSurface* fsurface);
RGAPI void PresentSurface (FullSurface* fsurface);
RGAPI void DummySubmitOnQueue(cwoid);
RGAPI int GetScreenWidth (cwoid); //Window width
RGAPI int GetScreenHeight (cwoid); //Window height
RGAPI int GetMonitorWidth (cwoid); //Monitor height
RGAPI int GetMonitorHeight(cwoid); //Monitor height
RGAPI int GetRenderWidth (cwoid); //Render width (e.g. Rendertexture)
RGAPI int GetRenderHeight (cwoid); //Render height (e.g. Rendertexture)
RGAPI void ToggleFullscreen(cwoid);
RGAPI int GetCurrentMonitor(void);
RGAPI bool IsKeyDown(int key);
RGAPI bool IsKeyPressed(int key);
RGAPI int GetCharPressed(cwoid);
RGAPI int GetMouseX(cwoid);
RGAPI int GetMouseY(cwoid);
RGAPI int GetTouchX(void); // Get touch position X for touch point 0 (relative to screen size)
RGAPI int GetTouchY(void); // Get touch position Y for touch point 0 (relative to screen size)
RGAPI Vector2 GetTouchPosition(int index); // Get touch position XY for a touch point index (relative to screen size)
RGAPI int GetTouchPointId(int index); // Get touch point identifier for given index
RGAPI int GetTouchPointCount(void); // Get number of touch points
RGAPI float GetGesturePinchZoom(cwoid);
RGAPI float GetGesturePinchAngle(cwoid);
RGAPI Vector2 GetMousePosition(cwoid);
RGAPI Vector2 GetMouseDelta(cwoid);
RGAPI float GetMouseWheelMove(void); // Get mouse wheel movement for X or Y, whichever is larger
RGAPI Vector2 GetMouseWheelMoveV(void); // Get mouse wheel movement for both X and Y
RGAPI bool IsMouseButtonPressed(int button);
RGAPI bool IsMouseButtonDown(int button);
RGAPI bool IsMouseButtonReleased(int button);
RGAPI void ShowCursor(cwoid);                   // Shows cursor
RGAPI void HideCursor(cwoid);                   // Hides cursor
RGAPI bool IsCursorHidden(cwoid);               // Check if cursor is not visible
RGAPI void EnableCursor(cwoid);                 // Enables cursor (unlock cursor)
RGAPI void DisableCursor(cwoid);                // Disables cursor (lock cursor)
RGAPI bool IsCursorOnScreen(cwoid);             // Check if cursor is on the screen
RGAPI void PollEvents(cwoid);
RGAPI void PollEvents_SDL2(cwoid);
RGAPI void PollEvents_SDL3(cwoid);
RGAPI void PollEvents_GLFW(cwoid);
RGAPI void PollEvents_RGFW(cwoid);
RGAPI int GetMonitorWidth_GLFW(cwoid);
RGAPI int GetMonitorWidth_SDL2(cwoid);
RGAPI int GetMonitorWidth_SDL3(cwoid);
RGAPI int GetMonitorHeight_SDL3(cwoid);
RGAPI int GetMonitorHeight_GLFW(cwoid);
RGAPI int GetMonitorHeight_SDL2(cwoid);
RGAPI int GetTouchPointCount_SDL2(cwoid);
RGAPI Vector2 GetTouchPosition_SDL2(int);
RGAPI void SetWindowShouldClose_GLFW(GLFWwindow* win);
RGAPI void Initialize_SDL2(cwoid);
RGAPI void Initialize_SDL3(cwoid);
RGAPI bool WindowShouldClose_GLFW(GLFWwindow* win);
RGAPI SubWindow InitWindow_GLFW(int width, int height, const char* title);
RGAPI SubWindow InitWindow_RGFW(int width, int height, const char* title);
RGAPI void ToggleFullscreen_GLFW(cwoid);
RGAPI void ToggleFullscreen_SDL2(cwoid);
RGAPI void ToggleFullscreen_SDL3(cwoid);
RGAPI SubWindow OpenSubWindow_GLFW(int width, int height, const char* title);
RGAPI SubWindow OpenSubWindow_SDL2(int width, int height, const char* title);
RGAPI SubWindow OpenSubWindow_SDL3(int width, int height, const char* title);

RGAPI void SetShaderValue(Shader shader, int uniformLoc, const void *value, int uniformType);
RGAPI void SetShaderValueV(Shader shader, int uniformLoc, const void *value, int uniformType, int count);
RGAPI void DrawModel(Model model, Vector3 position, float scale, Color tint);

/**
 * @brief Get the time elapsed since InitWindow() in seconds since
 *
 * @return double
 */
RGAPI double GetTime(cwoid);
RGAPI int GetRandomValue(int min, int max);
RGAPI void SetTraceLogFile(FILE* file);
RGAPI void SetTraceLogLevel(int logLevel);
RGAPI void TraceLog(int logType, const char *text, ...);

/**
 * @brief Return the unix timestamp in nanosecond precision
 *
 * (implemented with high_resolution_clock::now().time_since_epoch())
 * @return uint64_t
 */
RGAPI uint64_t NanoTime(cwoid);

/**
 * @brief This function exists to request limits that are higher than the default ones
 *
 * @details
 * By default, device limits are **very** conservative to guarantee support on most platforms
 * If a higher limit is desired, e.g. for maxTextureDimension2D, it has to be explicitly requested
 * @param limit The limit type, e.g. maxTextureDimension2D, maxBufferSize
 * @param value The new value for this limit
 */
RGAPI void RequestLimit(LimitType limit, uint64_t value);
/**
 * @brief Use a specific backend for the adapter.
 *
 * @param backend The backend to use, e.g. WGPUBackendType_Vulkan, WGPUBackendType_D3D12
 */
RGAPI void RequestBackend(BackendType backend);

/**
 * @brief Force a specific adapter type, namely WGPUAdapterType_DiscreteGPU, WGPUAdapterType_IntegratedGPU or WGPUAdapterType_CPU.
 * Not all types are guaranteed to exist.
 * @param type The adapter type
 */
RGAPI void RequestAdapterType(AdapterType type);
RGAPI void SetConfigFlags(int /* enum WindowFlag */ flag);
RGAPI bool IsATerminal(FILE *stream);
RGAPI void SetTargetFPS(int fps);               // Set target FPS (maximum)
RGAPI int GetTargetFPS(cwoid);
RGAPI uint64_t GetFrameCount(cwoid);
RGAPI float GetFrameTime(cwoid);                // Get time in seconds for last frame drawn (delta time)
RGAPI void DrawFPS(int posX, int posY);         // Draw current FPS
RGAPI void NanoWait(uint64_t time);
RGAPI uint32_t GetFPS(cwoid);

RGAPI void ClearBackground(Color clearColor);
RGAPI void BeginDrawing(cwoid);
RGAPI void EndDrawing(cwoid);
RGAPI void StartGIFRecording(cwoid);
RGAPI void EndGIFRecording(cwoid);

RGAPI Texture GetDepthTexture(cwoid);
RGAPI Texture GetMultisampleColorTarget(cwoid);
RGAPI DescribedRenderpass LoadRenderpassEx(RenderSettings settings, bool colorClear, RGColor colorClearValue, bool depthClear, float depthClearValue);
RGAPI void UnloadRenderpass(DescribedRenderpass rp);
RGAPI void BeginRenderpass(cwoid);
RGAPI void EndRenderpass(cwoid);
RGAPI void BeginComputepass(cwoid);
RGAPI void BindComputePipeline(DescribedComputePipeline* cpl);
RGAPI void DispatchCompute(uint32_t x, uint32_t y, uint32_t z);
RGAPI void CopyBufferToBuffer(DescribedBuffer* source, DescribedBuffer* dest, size_t count/* in bytes*/);
RGAPI void CopyTextureToTexture(Texture source, Texture dest);
RGAPI void ComputepassEndOnlyComputing(cwoid);
RGAPI void EndComputepass(cwoid);
RGAPI void BeginComputepassEx(DescribedComputepass* computePass);
RGAPI void EndComputepassEx(DescribedComputepass* computePass);
RGAPI void BeginRenderpassEx(DescribedRenderpass* renderPass);
RGAPI void EndRenderpassEx(DescribedRenderpass* renderPass);
RGAPI void EndRenderpassPro(DescribedRenderpass* renderPass, bool isRenderTexture);
RGAPI void BeginPipelineMode(DescribedPipeline* pipeline);
RGAPI void EndPipelineMode(cwoid);
RGAPI void BeginShaderMode(Shader shader); // Begin custom shader drawing
RGAPI void EndShaderMode(void);
RGAPI void DisableDepthTest(cwoid);
RGAPI void BeginBlendMode(rlBlendMode blendMode);
RGAPI void EndBlendMode(void);
RGAPI void BeginMode2D(Camera2D camera);
RGAPI void EndMode2D(cwoid);
RGAPI void BeginMode3D(Camera3D camera);
RGAPI void EndMode3D(cwoid);
RGAPI void LoadIdentity(cwoid);
RGAPI void PushMatrix(cwoid);
RGAPI void PopMatrix(cwoid);
RGAPI Matrix GetMatrix(cwoid);

RGAPI char* LoadFileText(const char* fileName);
RGAPI void UnloadFileText(char* content);
RGAPI void* LoadFileData(const char* fileName, size_t* dataSize);
RGAPI void UnloadFileData(void* content);
RGAPI const char* GetDirectoryPath(const char* arg);
RGAPI const char* FindDirectory(const char* directoryName, int maxOutwardSearch);
RGAPI bool IsFileExtension(const char *fileName, const char *ext);
/**
 * @brief Checks if a given path, relative or absolute, exists
 * @param path The path to check
 * @return whether the path corresponds to an existing file
 */
RGAPI bool FileExists(const char* path);

RGAPI DescribedSampler LoadSampler(TextureWrap amode, TextureFilter fmode);
RGAPI DescribedSampler LoadSamplerEx(TextureWrap amode, TextureFilter fmode, TextureFilter mipmapFilter, float maxAnisotropy);
RGAPI void UnloadSampler(DescribedSampler sampler);

RGAPI WGPUTexture GetActiveColorTarget(cwoid);
RGAPI Texture2DArray LoadTextureArray(uint32_t width, uint32_t height, uint32_t layerCount, PixelFormat format);
RGAPI void* GetActiveWindowHandle(cwoid);
RGAPI Texture LoadTextureFromImage(Image img);
RGAPI void ImageFormat(Image* img, PixelFormat newFormat);
RGAPI Image LoadImageFromTexture(Texture tex);
RGAPI Image LoadImageFromTextureEx(WGPUTexture tex, uint32_t mipLevel);
RGAPI void TakeScreenshot(const char* filename);
RGAPI Image LoadImage(const char* filename);
RGAPI Image ImageFromImage(Image img, Rectangle rec);
RGAPI Color* LoadImageColors(Image img);
RGAPI void UnloadImageColors(Color* cols);
RGAPI uint64_t RoundUpToNextMultipleOf256(uint64_t x);
RGAPI void UnloadImage(Image img);
RGAPI void UnloadTexture(Texture tex);
RGAPI Image LoadImageFromMemory(const char* extension, const void* data, size_t dataSize);
RGAPI Image GenImageColor(Color a, uint32_t width, uint32_t height);
RGAPI Image GenImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount);
RGAPI void SaveImage(Image img, const char* filepath);
RGAPI unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize);
RGAPI char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize);
RGAPI float TextToFloat(const char *text);
RGAPI const char *TextToLower(const char *text);
RGAPI const char **TextSplit(const char *text, char delimiter, int *count);
RGAPI unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize);
RGAPI unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize);
RGAPI const char *CodepointToUTF8(int codepoint, int *utf8Size);
RGAPI int TextToInteger(const char *text);
RGAPI void UnloadCodepoints(int *codepoints);
RGAPI int *LoadCodepoints(const char *text, int *count);
RGAPI int GetCodepoint(const char *text, int *codepointSize);
RGAPI int GetCodepointPrevious(const char *text, int *codepointSize);
RGAPI void DrawText(const char *text, int posX, int posY, int fontSize, Color color); // Draw text (using default font)
RGAPI void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
RGAPI void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
RGAPI void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
RGAPI void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)
RGAPI int GetCodepointNext(const char *text, int *codepointSize);
RGAPI unsigned int TextLength(const char *text);
RGAPI void SetTextLineSpacing(int spacing);                                             // Set vertical line spacing when drawing with line-breaks
RGAPI int MeasureText(const char *text, int fontSize);                                  // Measure string width for default font
RGAPI Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);  // Measure string size for Font
RGAPI int GetGlyphIndex(Font font, int codepoint);                                      // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
RGAPI GlyphInfo GetGlyphInfo(Font font, int codepoint);                                 // Get glyph font info data for a codepoint (unicode character), fallback to '?' if not found
RGAPI Rectangle GetGlyphAtlasRec(Font font, int codepoint);                             // Get glyph rectangle in font atlas for a codepoint (unicode character), fallback to '?' if not found
RGAPI Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);
RGAPI Font LoadFontFromImage(Image img, Color key, int firstchar);
RGAPI Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount);
RGAPI void LoadFontDefault(void);
RGAPI Font GetFontDefault(void);
RGAPI GlyphInfo* LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
RGAPI Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);
RGAPI void SetShapesTexture(Texture tex, Rectangle rec);
RGAPI void UseTexture(Texture tex);
RGAPI void UseNoTexture(cwoid);
RGAPI void drawCurrentBatch(cwoid);
static inline Color Fade(Color col, float fade_alpha){
    float v = (1.0f - fade_alpha);
    uint8_t a = (uint8_t)roundf(((float)col.a) * v);
    return CLITERAL(Color){col.r, col.g, col.b, a};
}

static inline Color GetColor(unsigned int hexValue){
    Color color;
    color.r = (uint8_t)(hexValue >> 24) & 0xFF;
    color.g = (uint8_t)(hexValue >> 16) & 0xFF;
    color.b = (uint8_t)(hexValue >> 8) & 0xFF;
    color.a = (uint8_t)hexValue & 0xFF;
    return color;
}
#if !defined(RAYGPU_NO_INLINE_FUNCTIONS) || RAYGPU_NO_INLINE_FUNCTIONS == 0
static void rlColor4f(float r, float g, float b, float alpha){
    nextcol.x = r; nextcol.y = g; nextcol.z = b; nextcol.w = alpha;
}
static void rlColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
    nextcol.x = ((float)((int)r)) / 255.0f;
    nextcol.y = ((float)((int)g)) / 255.0f;
    nextcol.z = ((float)((int)b)) / 255.0f;
    nextcol.w = ((float)((int)a)) / 255.0f;
}
static void rlColor3f(float r, float g, float b){
    rlColor4f(r, g, b, 1.0f);
}
static void rlTexCoord2f(float u, float v){
    nextuv.x = u; nextuv.y = v;
}
static void rlVertex2f(float x, float y){
    *(vboptr++) = CLITERAL(vertex){{x, y, 0}, nextuv, nextnormal, nextcol};
    if(UNLIKELY(vboptr - vboptr_base >= RENDERBATCH_SIZE)){
        drawCurrentBatch();
    }
}
static void rlNormal3f(float x, float y, float z){
    nextnormal.x = x; nextnormal.y = y; nextnormal.z = z;
}
static void rlVertex3f(float x, float y, float z){
    *(vboptr++) = CLITERAL(vertex){{x, y, z}, nextuv, nextnormal, nextcol};
    if(UNLIKELY(vboptr - vboptr_base >= RENDERBATCH_SIZE)){
        drawCurrentBatch();
    }
}
#else
void rlColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void rlColor4f(float r, float g, float b, float alpha);
void rlColor3f(float r, float g, float b);
void rlVertex3f(float x, float y, float z);
void rlNormal3f(float x, float y, float z);
void rlVertex2f(float x, float y);
void rlTexCoord2f(float u, float v);
#endif

RGAPI void rlSetLineWidth(float lineWidth);
RGAPI void rlBegin(PrimitiveType mode);
RGAPI void rlEnd(cwoid);

RGAPI void BeginTextureAndPipelineMode(RenderTexture rtex, Shader pl);
RGAPI void EndTextureAndPipelineMode(cwoid);
RGAPI void BeginTextureMode(RenderTexture rtex);
RGAPI void EndTextureMode(cwoid);
RGAPI void BeginWindowMode(SubWindow sw);
RGAPI void EndWindowMode(cwoid);

RGAPI void BindShader(Shader shader, PrimitiveType pt);
RGAPI void BindComputePipeline(DescribedComputePipeline* pipeline);
RGAPI DescribedShaderModule LoadShaderModuleWGSL (ShaderSources sourcesWGSL);
RGAPI DescribedShaderModule LoadShaderModuleGLSL (ShaderSources sourcesGLSL);
RGAPI DescribedShaderModule LoadShaderModuleSPIRV(ShaderSources sourcesSpirv);
RGAPI DescribedShaderModule LoadShaderModule (ShaderSources source);
RGAPI const char* GetStageEntryPointName (ShaderReflectionInfo reflectionInfo, RGShaderStageEnum stage);
RGAPI uint32_t GetReflectionUniformLocation (ShaderReflectionInfo reflectionInfo, const char* name );
RGAPI uint32_t GetReflectionAttributeLocation (ShaderReflectionInfo reflectionInfo, const char* name );

RGAPI Shader LoadShader (const char *vsFileName, const char *fsFileName); // Assumes GLSL
RGAPI Shader LoadShaderSingleSource (const char* shaderSource); // Assumes WGSL
RGAPI Shader LoadShaderFromMemory (const char *vsCode , const char *fsCode ); // Assumes GLSL
RGAPI Shader LoadShaderFromMemorySPIRV(ShaderSources sources); // Obviously assumes SPIRV

RGAPI DescribedBindGroupLayout LoadBindGroupLayout(const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, bool compute);
RGAPI DescribedBindGroupLayout LoadBindGroupLayoutMod(const DescribedShaderModule* shaderModule);
//RGAPI WGPURaytracingPipeline LoadRTPipeline(const DescribedShaderModule* module);
RGAPI Shader ClonePipeline(const DescribedPipeline* pl);
RGAPI Shader ClonePipelineWithSettings(const DescribedPipeline* pl, RenderSettings settings);
RGAPI Shader LoadPipelineFromModule(DescribedShaderModule mod, const AttributeAndResidence* attribs, uint32_t attribCount, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
RGAPI Shader DefaultPipeline(cwoid);
RGAPI void UnloadBindGroupLayout(DescribedBindGroupLayout* bglayout);
RGAPI DescribedBindGroup LoadBindGroup(const DescribedBindGroupLayout* bglayout, const ResourceDescriptor* entries, size_t entryCount);
RGAPI WGPUBindGroup UpdateAndGetNativeBindGroup(DescribedBindGroup* bg);
RGAPI void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry);
RGAPI void UpdateBindGroup(DescribedBindGroup* bg);
RGAPI void UnloadBindGroup(DescribedBindGroup* bg);
RGAPI DescribedPipeline* Relayout(DescribedPipeline* pl, VertexArray* vao);
RGAPI DescribedComputePipeline* LoadComputePipeline(const char* shaderCode);
RGAPI DescribedComputePipeline* LoadComputePipelineEx(const char* shaderCode, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount);
RGAPI DescribedRaytracingPipeline* LoadRaytracingPipeline(const DescribedShaderModule* shaderModule);
RGAPI Shader DefaultShader(cwoid);
RGAPI RenderSettings GetDefaultSettings(cwoid);
RGAPI Texture GetDefaultTexture(cwoid);
RGAPI void UnloadPipeline(DescribedPipeline* pl);
RGAPI RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
RGAPI RenderTexture LoadRenderTextureEx(uint32_t width, uint32_t height, PixelFormat colorFormat, uint32_t sampleCount, uint32_t attachmentCount);
RGAPI size_t GetPixelSizeInBytes(PixelFormat format);
RGAPI Texture LoadBlankTexture(uint32_t width, uint32_t height);
RGAPI Texture LoadTexture(const char* filename);
RGAPI Texture LoadDepthTexture(uint32_t width, uint32_t height);
RGAPI Texture LoadTextureEx(uint32_t width, uint32_t height, PixelFormat format, bool to_be_used_as_rendertarget);
RGAPI Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, RGTextureUsage usage, uint32_t sampleCount, uint32_t mipmaps);
RGAPI void GenTextureMipmaps(Texture2D* tex);
RGAPI Texture3D LoadTexture3DEx(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format);
RGAPI Texture3D LoadTexture3DPro(uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, RGTextureUsage usage, uint32_t sampleCount);
RGAPI RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
RGAPI void UpdateTexture(Texture tex, void* data);
RGAPI StagingBuffer GenStagingBuffer(size_t size, RGBufferUsage usage);
RGAPI void UpdateStagingBuffer(StagingBuffer* buffer);
RGAPI void RecreateStagingBuffer(StagingBuffer* buffer);
RGAPI void MapStagingBuffer(size_t size, RGBufferUsage usage);
RGAPI void UnloadStagingBuffer(StagingBuffer* buf);
RGAPI DescribedBuffer* GenUniformBuffer(const void* data, size_t size);
RGAPI DescribedBuffer* GenStorageBuffer(const void* data, size_t size);
RGAPI DescribedBuffer* GenIndexBuffer(const void* data, size_t size);
RGAPI DescribedBuffer* GenVertexBuffer(const void* data, size_t size);
RGAPI DescribedBuffer* GenBufferEx(const void* data, size_t size, RGBufferUsage usage);
RGAPI void UnloadBuffer(DescribedBuffer* buffer);
RGAPI void BufferData(DescribedBuffer* buffer, const void* data, size_t size);
RGAPI void ResizeBuffer(DescribedBuffer* buffer, size_t newSize);
RGAPI void ResizeBufferAndConserve(DescribedBuffer* buffer, size_t newSize);
RGAPI void BindVertexBuffer(const DescribedBuffer* buffer);
RGAPI DescribedRenderpass* GetActiveRenderPass(cwoid);
RGAPI Shader GetActiveShader(cwoid);
RGAPI void RenderPassSetIndexBuffer (DescribedRenderpass* drp, DescribedBuffer* buffer, IndexFormat format, uint64_t offset);
RGAPI void RenderPassSetVertexBuffer(DescribedRenderpass* drp, uint32_t slot, DescribedBuffer* buffer, uint64_t offset);
RGAPI void RenderPassSetBindGroup (DescribedRenderpass* drp, uint32_t group, DescribedBindGroup* buffer);
RGAPI void ComputePassSetBindGroup (DescribedComputepass* drp, uint32_t group, DescribedBindGroup* buffer);
RGAPI void RenderPassDraw (DescribedRenderpass* drp, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
RGAPI void RenderPassDrawIndexed (DescribedRenderpass* drp, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t baseVertex, uint32_t firstInstance);
RGAPI uint32_t GetUniformLocation (Shader shader, const char* uniformName);
RGAPI uint32_t GetComputeShaderLocation (DescribedComputePipeline* shader, const char* uniformName);
RGAPI uint32_t GetUniformLocationCompute(Shader shader, const char* uniformName);
RGAPI uint32_t rlGetLocationUniform (const uint32_t shaderID, const char* uniformName);
RGAPI uint32_t rlGetLocationAttrib (const uint32_t shaderID, const char* attribName);

RGAPI void SetShaderTexture (Shader shader, uint32_t index, Texture tex);
RGAPI void SetShaderSampler (Shader shader, uint32_t index, DescribedSampler sampler);
RGAPI void SetShaderUniformBuffer (Shader shader, uint32_t index, DescribedBuffer* buffer);
RGAPI void SetShaderStorageBuffer (Shader shader, uint32_t index, DescribedBuffer* buffer);
RGAPI void SetShaderUniformBufferData (Shader shader, uint32_t index, const void* data, size_t size);
RGAPI void SetShaderStorageBufferData (Shader shader, uint32_t index, const void* data, size_t size);
/**
 * These functions modify the bindgroup of the currently bound pipeline,
 * i.e. the default pipeline or the one set with BeginPipelineMode
 */
RGAPI void SetTexture (uint32_t index, Texture tex);
RGAPI void SetSampler (uint32_t index, DescribedSampler sampler);
RGAPI void SetUniformBuffer (uint32_t index, DescribedBuffer* buffer);
RGAPI void SetStorageBuffer (uint32_t index, DescribedBuffer* buffer);
RGAPI void SetUniformBufferData (uint32_t index, const void* data, size_t size);
RGAPI void SetStorageBufferData (uint32_t index, const void* data, size_t size);
/**
 * These functions operate directly on a bindgroup
 */
RGAPI void SetBindgroupUniformBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer);
RGAPI void SetBindgroupStorageBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer);
RGAPI void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size);
RGAPI void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size);
RGAPI void SetBindgroupTexture3D (DescribedBindGroup* bg, uint32_t index, Texture3D tex);
RGAPI void SetBindgroupTextureView (DescribedBindGroup* bg, uint32_t index, WGPUTextureView texView);
RGAPI void SetBindgroupTexture (DescribedBindGroup* bg, uint32_t index, Texture tex);
RGAPI void SetBindgroupSampler (DescribedBindGroup* bg, uint32_t index, DescribedSampler sampler);


/** The functions LoadVertexArray, VertexAttribPointer, EnableVertexAttribArray, DisableVertexAttribArray
 * aim to replicate the behaviour of OpenGL as closely as possible.
 */
RGAPI VertexArray* LoadVertexArray (cwoid);
RGAPI void VertexAttribPointer (VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, RGVertexFormat format, uint32_t offset, RGVertexStepMode stepmode);
RGAPI void EnableVertexAttribArray (VertexArray* array, uint32_t attribLocation);
RGAPI void DisableVertexAttribArray (VertexArray* array, uint32_t attribLocation);

RGAPI void PrepareShader (Shader pipeline, VertexArray* va);
RGAPI void BindShaderVertexArray (Shader pipeline, VertexArray* va);
RGAPI void BindVertexArray (VertexArray* va);
RGAPI void DrawArrays (PrimitiveType drawMode, uint32_t vertexCount);
RGAPI void DrawArraysInstanced (PrimitiveType drawMode, uint32_t vertexCount, uint32_t instanceCount);
RGAPI void DrawArraysIndexed (PrimitiveType drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount);
RGAPI void DrawArraysIndexedInstanced(PrimitiveType drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount, uint32_t instanceCount);

RGAPI Material LoadMaterialDefault(cwoid);
RGAPI ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount);
RGAPI void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame);
RGAPI void UpdateModelAnimation(Model model, ModelAnimation anim, int frame);
RGAPI Model LoadModel(const char *fileName);                    // Load model from files (meshes and materials)
RGAPI Model LoadModelFromMesh(Mesh mesh);                       // Load model from generated mesh (default material)
RGAPI bool IsModelValid(Model model);                           // Check if a model is valid (loaded in GPU, VAO/VBOs)
RGAPI void UnloadModel(Model model);                            // Unload model (including meshes) from memory (RAM and/or VRAM)
RGAPI BoundingBox GetModelBoundingBox(Model model);             // Compute model bounding box limits (considers all meshes)

// Model drawing functions
void DrawModel(Model model, Vector3 position, float scale, Color tint);                                   // Draw a model (with texture if set)
void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model with extended parameters
void DrawModelWires(Model model, Vector3 position, float scale, Color tint);                              // Draw a model wires (with texture if set)
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model wires (with texture if set) with extended parameters
void DrawModelPoints(Model model, Vector3 position, float scale, Color tint);                             // Draw a model as points
void DrawModelPointsEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model as points with extended parameters
void DrawBoundingBox(BoundingBox box, Color color);                                                       // Draw bounding box (wires)
void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float scale, Color tint);          // Draw a billboard texture
void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint); // Draw a billboard texture defined by source
void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint); // Draw a billboard texture defined by source and rotation

// Mesh management functions
RGAPI void UploadMesh(Mesh *mesh, bool dynamic);        // Upload mesh vertex data in GPU and provide VAO/VBO ids
RGAPI void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset); // Update mesh vertex data in GPU for a specific buffer index
RGAPI void UnloadMesh(Mesh mesh);                       // Unload mesh data from CPU and GPU
RGAPI void DrawMesh(Mesh mesh, Material material, Matrix transform); // Draw a 3d mesh with material and transform
RGAPI void DrawMeshInstanced(Mesh mesh, Material material, const Matrix *transforms, int instances); // Draw multiple mesh instances with material and different transforms
RGAPI BoundingBox GetMeshBoundingBox(Mesh mesh);        // Compute mesh bounding box limits
RGAPI void GenMeshTangents(Mesh *mesh);                 // Compute mesh tangents
RGAPI Mesh GenMeshCube(float width, float height, float length);
RGAPI Mesh GenMeshPoly(int sides, float radius);
RGAPI Mesh GenMeshPlane(float width, float length, int resX, int resZ);
RGAPI Mesh GenMeshSphere(float radius, int rings, int slices);
RGAPI Mesh GenMeshHemiSphere(float radius, int rings, int slices);
RGAPI Mesh GenMeshCylinder(float radius, float height, int slices);
RGAPI Mesh GenMeshCone(float radius, float height, int slices);
RGAPI Mesh GenMeshTorus(float radius, float size, int radSeg, int sides);
RGAPI Mesh GenMeshKnot(float radius, float size, int radSeg, int sides);
RGAPI Mesh GenMeshHeightmap(Image heightmap, Vector3 size);
RGAPI Mesh GenMeshCubicmap(Image cubicmap, Vector3 cubeSize);
//bool ExportMesh(Mesh mesh, const char *fileName);         // Export mesh data to file, returns true on success
//bool ExportMeshAsCode(Mesh mesh, const char *fileName);   // Export mesh as code file (.h) defining multiple arrays of vertex attributes

RGAPI bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
RGAPI const char *TextFormat(const char *text, ...);
RGAPI void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
RGAPI void DrawTexture(Texture texture, int posX, int posY, Color tint);
RGAPI void DrawTextureV(Texture texture, Vector2 position, Color tint); // Draw a Texture2D with position defined as Vector2
RGAPI void DrawTextureEx(Texture texture, Vector2 position, float rotation, float scale, Color tint); // Draw a Texture2D with extended parameters
RGAPI void DrawTextureRec(Texture texture, Rectangle source, Vector2 position, Color tint); // Draw a part of a texture defined by a rectangle
RGAPI void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
RGAPI void DrawPixel(int posX, int posY, Color color);
RGAPI void DrawPixelV(Vector2 position, Color color);
RGAPI void DrawGrid(int slices, float spacing);
RGAPI void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
RGAPI void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
RGAPI void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);
RGAPI void DrawLineStrip(const Vector2 *points, int pointCount, Color color);
RGAPI void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);
RGAPI void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
RGAPI void DrawCircle(int centerX, int centerY, float radius, Color color);
RGAPI void DrawCircleV(Vector2 center, float radius, Color color);
RGAPI void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
RGAPI void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
RGAPI void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer);
RGAPI void DrawCircleLines(int centerX, int centerY, float radius, Color color);
RGAPI void DrawCircleLinesV(Vector2 center, float radius, Color color);
RGAPI void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);
RGAPI void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);
RGAPI void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
RGAPI void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
RGAPI void DrawRectangle(int posX, int posY, int width, int height, Color color);
RGAPI void DrawRectangleV(Vector2 position, Vector2 size, Color color);
RGAPI void DrawRectangleRec(Rectangle rec, Color color);
RGAPI void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);
RGAPI void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom);
RGAPI void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right);
RGAPI void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight);
RGAPI void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
RGAPI void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
RGAPI void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
RGAPI void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);
RGAPI void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color);
RGAPI void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
RGAPI void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
RGAPI void DrawTriangleFan(const Vector2 *points, int pointCount, Color color);
RGAPI void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color);
RGAPI void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);
RGAPI void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);
RGAPI void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color);
RGAPI void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color);
RGAPI void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color);

RGAPI void* GetInstance(cwoid);
RGAPI WGPUAdapter GetAdapter (cwoid);
RGAPI WGPUDevice GetDevice (cwoid);
RGAPI WGPUQueue GetQueue (cwoid);
RGAPI void* GetSurface (cwoid);

static inline uint32_t attributeSize(const RGVertexFormat fmt){
    switch(fmt){
        case RGVertexFormat_Uint8x4:
        case RGVertexFormat_Unorm8x4:
        case RGVertexFormat_Float32:
        case RGVertexFormat_Uint32:
        case RGVertexFormat_Sint32:
        case RGVertexFormat_Float16x2:
        case RGVertexFormat_Uint16x2:
        case RGVertexFormat_Sint16x2:
            return 4;
        case RGVertexFormat_Float32x2:
        case RGVertexFormat_Uint32x2:
        case RGVertexFormat_Sint32x2:
        case RGVertexFormat_Float16x4:
        case RGVertexFormat_Uint16x4:
        case RGVertexFormat_Sint16x4:
            return 8;
        case RGVertexFormat_Float32x3:
        case RGVertexFormat_Uint32x3:
        case RGVertexFormat_Sint32x3:
            return 12;
        case RGVertexFormat_Float32x4:
        case RGVertexFormat_Uint32x4:
        case RGVertexFormat_Sint32x4:
            return 16;
        case RGVertexFormat_Float16:
        case RGVertexFormat_Uint16:
        case RGVertexFormat_Sint16:
            return 2;
        //case RGVertexFormat_Uint8: // This is not a real format
        //    return 1;
        default:
            break;
    }
    abort();
    return 0;
}

const char* vkErrorString(int code);

EXTERN_C_END

typedef struct renderstate renderstate;
externcvar renderstate g_renderstate;

#endif
