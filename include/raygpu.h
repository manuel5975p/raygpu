#ifndef RAYGPU_H
#define RAYGPU_H
#include <webgpu/webgpu.h>
#ifdef __cplusplus
#include <webgpu/webgpu_cpp.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include "macros_and_constants.h"
#include "mathutils.h"
#include "pipeline.h"
#ifdef __cplusplus
#include <vector>
#include <cassert>
#include <unordered_map>
#endif
#define RL_FREE free
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif
static inline uint64_t ROT_BYTES(uint64_t V, uint8_t C) {
    return ((V << C) | ((V) >> (64 - C)));
}
typedef struct vertex{
    Vector3 pos;
    Vector2 uv ;
    Vector3 normal ;
    Vector4 col;
}vertex;
typedef struct Color{
    uint8_t r, g, b, a;
} Color;
typedef struct BGRAColor{
    uint8_t b, g, r, a;
} BGRAColor;


typedef enum PixelFormat{
    RGBA8 = WGPUTextureFormat_RGBA8Unorm,
    BGRA8 = WGPUTextureFormat_BGRA8Unorm,
    GRAYSCALE = 0x100000, //No WGPU_ equivalent
    RGB8 = 0x100000,
}PixelFormat;

typedef struct Image{
    PixelFormat format;
    uint32_t width, height;
    size_t rowStrideInBytes; // Does not have to match with width
                             // One reason for this is the fact that Texture to Buffer copy commands
                             // Have to have a multiple of 256 bytes as row length 
    void* data;
    int mipmaps; // Unused
}Image;

typedef struct Texture{
    WGPUTexture id;
    WGPUTextureView view;
    uint32_t width, height;
    WGPUTextureFormat format;
    uint32_t sampleCount;
}Texture;

typedef Texture Texture2D;

typedef struct Rectangle {
    float x, y, width, height;
} Rectangle;

typedef struct RenderTexture{
    Texture color;
    Texture colorMultisample;
    Texture depth;
}RenderTexture;



typedef struct DescribedRenderpass{
    RenderSettings settings;
    WGPURenderPassDescriptor renderPassDesc;
    WGPURenderPassColorAttachment* rca;
    WGPURenderPassDepthStencilAttachment* dsa;
    WGPUCommandEncoder cmdEncoder;
    WGPURenderPassEncoder rpEncoder; //Only non-null when in renderpass
}DescribedRenderpass;

typedef struct DescribedComputePass{
    WGPUCommandEncoder cmdEncoder;
    WGPUComputePassEncoder cpEncoder;
    WGPUComputePassDescriptor desc;
}DescribedComputepass;
EXTERN_C_BEGIN
    WGPUInstance GetInstance(cwoid);
    WGPUAdapter  GetAdapter (cwoid);
    WGPUDevice   GetDevice  (cwoid);
    WGPUQueue    GetQueue   (cwoid);
    WGPUSurface  GetSurface (cwoid);
EXTERN_C_END

#ifdef __cplusplus
    wgpu::Instance& GetCXXInstance();
    wgpu::Adapter&  GetCXXAdapter ();
    wgpu::Device&   GetCXXDevice  ();
    wgpu::Queue&    GetCXXQueue   ();
    wgpu::Surface&  GetCXXSurface ();
#endif


typedef struct DescribedBuffer{
    WGPUBufferDescriptor descriptor;
    WGPUBuffer buffer;
}DescribedBuffer;

typedef struct DescribedSampler{
    WGPUSampler sampler;
    WGPUSamplerDescriptor desc;
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
    int vertexCount;            // Number of vertices stored in arrays
    int triangleCount;          // Number of triangles stored (indexed or not)

    // Vertex attributes data
    float *vertices;            // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;           // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;        // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;             // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float *tangents;          // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
    float *colors;              // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    uint32_t *indices;          // Vertex indices (in case vertex data comes indexed)

    // Animation vertex data (not supported yet)
    float *animVertices;        // Animated vertex positions (after bones transformations)
    float *animNormals;         // Animated normals (after bones transformations)
    unsigned char *boneIds;     // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning) (shader-location = 6)
    float *boneWeights;         // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)
    Matrix *boneMatrices;       // Bones animated transformation matrices
    int boneCount;              // Number of bones

    // WebGPU identifiers
    VertexArray* vao;
    DescribedBuffer** vbos;
    DescribedBuffer* ibo;              //Index buffer object, optional
    DescribedBuffer* boneMatrixBuffer; //Storage buffer
    WGPUVertexFormat boneIDFormat;    //Either WGPUVertexFormat_Uint8 or -16;
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

typedef struct Material{
    int id;
    MaterialMap* maps;
    DescribedPipeline* pipeline;
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
    Vector3 min;            // Minimum vertex box-corner
    Vector3 max;            // Maximum vertex box-corner
} BoundingBox;
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
externcvar VertexArray* renderBatchVAO;
externcvar DescribedBuffer* renderBatchVBO;
externcvar char telegrama_render[];
externcvar size_t telegrama_render_size;

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
#define LIGHTGRAY  CLITERAL(Color){ 200, 200, 200, 255 }   // Light Gray
#define GRAY       CLITERAL(Color){ 130, 130, 130, 255 }   // Gray
#define DARKGRAY   CLITERAL(Color){ 80, 80, 80, 255 }      // Dark Gray
#define YELLOW     CLITERAL(Color){ 253, 249, 0, 255 }     // Yellow
#define GOLD       CLITERAL(Color){ 255, 203, 0, 255 }     // Gold
#define ORANGE     CLITERAL(Color){ 255, 161, 0, 255 }     // Orange
#define PINK       CLITERAL(Color){ 255, 109, 194, 255 }   // Pink
#define RED        CLITERAL(Color){ 230, 41, 55, 255 }     // Red
#define MAROON     CLITERAL(Color){ 190, 33, 55, 255 }     // Maroon
#define GREEN      CLITERAL(Color){ 0, 228, 48, 255 }      // Green
#define LIME       CLITERAL(Color){ 0, 158, 47, 255 }      // Lime
#define DARKGREEN  CLITERAL(Color){ 0, 117, 44, 255 }      // Dark Green
#define SKYBLUE    CLITERAL(Color){ 102, 191, 255, 255 }   // Sky Blue
#define BLUE       CLITERAL(Color){ 0, 121, 241, 255 }     // Blue
#define DARKBLUE   CLITERAL(Color){ 0, 82, 172, 255 }      // Dark Blue
#define PURPLE     CLITERAL(Color){ 200, 122, 255, 255 }   // Purple
#define VIOLET     CLITERAL(Color){ 135, 60, 190, 255 }    // Violet
#define DARKPURPLE CLITERAL(Color){ 112, 31, 126, 255 }    // Dark Purple
#define BEIGE      CLITERAL(Color){ 211, 176, 131, 255 }   // Beige
#define BROWN      CLITERAL(Color){ 127, 106, 79, 255 }    // Brown
#define DARKBROWN  CLITERAL(Color){ 76, 63, 47, 255 }      // Dark Brown

#define WHITE      CLITERAL(Color){ 255, 255, 255, 255 }   // White
#define BLACK      CLITERAL(Color){ 0, 0, 0, 255 }         // Black
#define BLANK      CLITERAL(Color){ 0, 0, 0, 0 }           // Blank (Transparent)
#define MAGENTA    CLITERAL(Color){ 255, 0, 255, 255 }     // Magenta
#define RAYWHITE   CLITERAL(Color){ 245, 245, 245, 255 }   // @raysan's own White (raylib logo)

#endif


typedef enum WindowFlag{
    FLAG_STDOUT_TO_FFMPEG   = 0x02000000,   // Redirect tracelog to stderr and dump frames into stdout
                                            // Made for <program> | ffmpeg -f rawvideo -pix_fmt bgra -s 1920x1080 -i - output.mp4
    FLAG_HEADLESS           = 0x01000000,   // Disable ALL windowing stuff (Runnable without Desktop Server)
    FLAG_VSYNC_HINT         = 0x00000040,   // Set to try enabling V-Sync on GPU (i.e. PresentMode::Fifo for the surface)
    FLAG_MSAA_4X_HINT       = 0x00000020,   // Forcefully Hint (actually force) 4x multisampling for the default color buffer and pipeline
    FLAG_WINDOW_RESIZABLE   = 0x00000004,
    FLAG_FULLSCREEN_MODE    = 0x00000002,   // Set to run program in fullscreen
} WindowFlag;
typedef enum draw_mode{
    RL_TRIANGLES, RL_TRIANGLE_STRIP, RL_QUADS, RL_LINES
}draw_mode;
typedef enum filterMode{
    nearest = WGPUFilterMode_Nearest,
    linear = WGPUFilterMode_Linear,
}filterMode;
typedef enum adressMode{
    repeat = WGPUAddressMode_Repeat, clampToEdge = WGPUAddressMode_ClampToEdge, mirrorRepeat = WGPUAddressMode_MirrorRepeat,
}addressMode;
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
 * 
 */
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
    maxInterStageShaderComponents,
    maxInterStageShaderVariables,
    maxColorAttachments,
    maxColorAttachmentBytesPerSample,
    maxComputeWorkgroupStorageSize,
    maxComputeInvocationsPerWorkgroup,
    maxComputeWorkgroupSizeX,
    maxComputeWorkgroupSizeY,
    maxComputeWorkgroupSizeZ,
    maxComputeWorkgroupsPerDimension,
    maxStorageBuffersInVertexStage,
    maxStorageTexturesInVertexStage,
    maxStorageBuffersInFragmentStage,
    maxStorageTexturesInFragmentStage
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
    MATERIAL_MAP_ALBEDO = 0,        // Albedo material (same as: MATERIAL_MAP_DIFFUSE)
    MATERIAL_MAP_METALNESS,         // Metalness material (same as: MATERIAL_MAP_SPECULAR)
    MATERIAL_MAP_NORMAL,            // Normal material
    MATERIAL_MAP_ROUGHNESS,         // Roughness material
    MATERIAL_MAP_OCCLUSION,         // Ambient occlusion material
    MATERIAL_MAP_EMISSION,          // Emission material
    MATERIAL_MAP_HEIGHT,            // Heightmap material
    MATERIAL_MAP_CUBEMAP,           // Cubemap material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_IRRADIANCE,        // Irradiance material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_PREFILTER,         // Prefilter material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    MATERIAL_MAP_BRDF               // Brdf material
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
#define MOUSE_LEFT_BUTTON MOUSE_BUTTON_LEFT
#define MOUSE_RIGHT_BUTTON MOUSE_BUTTON_RIGHT

typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} MouseButton;


typedef struct AttributeAndResidence{
    WGPUVertexAttribute attr;
    uint32_t bufferSlot; //Describes the actual buffer it will reside in
    WGPUVertexStepMode stepMode;
    bool enabled;
}AttributeAndResidence;

typedef struct SubWindow{
    void* handle;
    WGPUSurface surface;
    RenderTexture frameBuffer;
}SubWindow;

typedef struct full_renderstate full_renderstate;
typedef struct GLFWwindow GLFWwindow;
#ifdef __cplusplus
namespace std{
    template<>
    struct hash<AttributeAndResidence>{
        size_t operator()(const AttributeAndResidence& res)const noexcept{
            uint64_t attrh = ROT_BYTES(res.attr.shaderLocation * uint64_t(41), 31) ^ ROT_BYTES(res.attr.offset, 11);
            attrh *= 111;
            attrh ^= ROT_BYTES(res.attr.format, 48) * uint64_t(44497);
            uint64_t v = ROT_BYTES(res.bufferSlot * uint64_t(756839), 13) ^ ROT_BYTES(attrh * uint64_t(1171), 47);
            v ^= ROT_BYTES(res.enabled * uint64_t(2976221), 23);
            return v;
        }
    };
    template<>
    struct hash<std::vector<AttributeAndResidence>>{
        size_t operator()(const std::vector<AttributeAndResidence>& res)const noexcept{
            uint64_t hv = 0;
            for(const auto& aar: res){
                hv ^= hash<AttributeAndResidence>{}(aar);
                hv = ROT_BYTES(hv, 7);
            }
            return hv;
        }
    };
}
struct attributeVectorCompare{
    inline bool operator()(const std::vector<AttributeAndResidence>& a, const std::vector<AttributeAndResidence>& b)const noexcept{
        #ifndef NDEBUG
        int prevloc = -1;

        for(size_t i = 0;i < a.size();i++){
            assert((int)a[i].attr.shaderLocation > prevloc && "std::vector<AttributeAndResidence> not sorted");
            prevloc = a[i].attr.shaderLocation;
        }
        prevloc = -1;
        for(size_t i = 0;i < b.size();i++){
            assert((int)b[i].attr.shaderLocation > prevloc && "std::vector<AttributeAndResidence> not sorted");
            prevloc = b[i].attr.shaderLocation;
        }
        #endif
        if(a.size() != b.size()){
            return false;
        }
        for(size_t i = 0;i < a.size();i++){
            if(
                   a[i].bufferSlot != b[i].bufferSlot 
                || a[i].enabled != b[i].enabled
                || a[i].stepMode != b[i].stepMode 
                || a[i].attr.format != b[i].attr.format 
                || a[i].attr.offset != b[i].attr.offset 
                || a[i].attr.shaderLocation != b[i].attr.shaderLocation
            ){
                return false;
            }

        }
        return true;
    }
};
typedef struct VertexStateToPipelineMap{
    std::unordered_map<std::vector<AttributeAndResidence>, PipelineTriplet, std::hash<std::vector<AttributeAndResidence>>, attributeVectorCompare> pipelines;

}VertexStateToPipelineMap;
#endif

#ifdef __cplusplus
/**
 * @brief Get the Bindings object, returning a map from 
 * Uniform name -> UniformDescriptor (type and minimum size and binding location)
 * 
 * @param shaderSource 
 * @return std::unordered_map<std::string, std::pair<uint32_t, UniformDescriptor>> 
 */
std::unordered_map<std::string, UniformDescriptor> getBindings(const char* shaderSource);
/**
 * @brief returning a map from 
 * Attribute name -> Attribute format (vec2f, vec3f, etc.) and binding location
 * @param shaderSource 
 * @return std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> 
 */
std::unordered_map<std::string, std::pair<WGPUVertexFormat, uint32_t>> getAttributes(const char* shaderSource);
#endif

EXTERN_C_BEGIN
    GLFWwindow* InitWindow(uint32_t width, uint32_t height, const char* title);
    void SetWindowShouldClose(cwoid);
    bool WindowShouldClose(cwoid);
    SubWindow OpenSubWindow(uint32_t width, uint32_t height, const char* title);
    void CloseSubWindow(SubWindow subWindow);

    uint32_t GetScreenWidth (cwoid);                             //Window width
    uint32_t GetScreenHeight(cwoid);                             //Window height
    uint32_t GetMonitorWidth (cwoid);                            //Monitor height
    uint32_t GetMonitorHeight(cwoid);                            //Monitor height
    void ToggleFullscreen(cwoid);
    int GetCurrentMonitor(void);

    bool IsKeyDown(int key);
    bool IsKeyPressed(int key);
    int GetCharPressed(cwoid);

    int GetMouseX(cwoid);
    int GetMouseY(cwoid);
    Vector2 GetMousePosition(cwoid);
    Vector2 GetMouseDelta(cwoid);
    float GetMouseWheelMove(void);                          // Get mouse wheel movement for X or Y, whichever is larger
    Vector2 GetMouseWheelMoveV(void);                       // Get mouse wheel movement for both X and Y
    bool IsMouseButtonPressed(int button);
    bool IsMouseButtonDown(int button);
    bool IsMouseButtonReleased(int button);

    void ShowCursor(cwoid);                                      // Shows cursor
    void HideCursor(cwoid);                                      // Hides cursor
    bool IsCursorHidden(cwoid);                                  // Check if cursor is not visible
    void EnableCursor(cwoid);                                    // Enables cursor (unlock cursor)
    void DisableCursor(cwoid);                                   // Disables cursor (lock cursor)
    bool IsCursorOnScreen(cwoid);                                // Check if cursor is on the screen
    void PollEvents(cwoid);

    /**
     * @brief Get the time elapsed since InitWindow() in seconds since 
     * 
     * @return double
     */
    double GetTime(cwoid);
    int GetRandomValue(int min, int max);
    void SetTraceLogFile(FILE* file);
    void SetTraceLogLevel(int logLevel);
    void TraceLog(int logType, const char *text, ...);
    /**
     * @brief Return the unix timestamp in nanosecond precision
     *
     * (implemented with high_resolution_clock::now().time_since_epoch())
     * @return uint64_t 
     */
    uint64_t NanoTime(cwoid);
    void RequestLimit(LimitType limit, uint64_t value);
    void SetConfigFlags(WindowFlag flag);
    bool IsATerminal(FILE *stream);
    void SetTargetFPS(int fps);                                 // Set target FPS (maximum)
    int GetTargetFPS(cwoid);
    uint64_t GetFrameCount(cwoid);
    float GetFrameTime(cwoid);                                  // Get time in seconds for last frame drawn (delta time)
    void DrawFPS(int posX, int posY);                           // Draw current FPS
    void NanoWait(uint64_t time);
    uint32_t GetFPS(cwoid);
    void ClearBackground(Color clearColor);
    void BeginDrawing(cwoid);
    void EndDrawing(cwoid);
    void StartGIFRecording(cwoid);
    void EndGIFRecording(cwoid);
    Texture GetDepthTexture(cwoid);
    Texture GetIntermediaryColorTarget(cwoid);


    DescribedRenderpass LoadRenderpass(WGPUTextureView color, WGPUTextureView depth);
    DescribedRenderpass LoadRenderpassEx(WGPUTextureView color, WGPUTextureView depth, RenderSettings settings);
    void UpdateRenderpass(DescribedRenderpass* rp, RenderSettings newSettings);
    void UnloadRenderpass(DescribedRenderpass rp);
    
    void BeginRenderpass(cwoid);
    void EndRenderpass(cwoid);
    void BeginComputepass(cwoid);
    void BindComputePipeline(DescribedComputePipeline* cpl);
    void DispatchCompute(uint32_t x, uint32_t y, uint32_t z);
    void CopyBufferToBuffer(DescribedBuffer* source, DescribedBuffer* dest, size_t count/* in bytes*/);
    void ComputepassEndOnlyComputing(cwoid);
    void EndComputepass(cwoid);
    void BeginComputepassEx(DescribedComputepass* computePass);
    void EndComputepassEx(DescribedComputepass* computePass);
    void BeginRenderpassEx(DescribedRenderpass* renderPass);
    void EndRenderpassEx(DescribedRenderpass* renderPass);
    void BeginPipelineMode(DescribedPipeline* pipeline);
    void EndPipelineMode(cwoid);
    void BeginMode2D(Camera2D camera);
    void EndMode2D(cwoid);
    void BeginMode3D(Camera3D camera);
    void EndMode3D(cwoid);
    char *LoadFileText(const char* fileName);
    void UnloadFileText(char* content);
    void* LoadFileData(const char* fileName, size_t* dataSize);
    void UnloadFileData(void* content);
    const char* GetDirectoryPath(const char* arg);
    const char* FindDirectory(const char* directoryName, int maxOutwardSearch);
    bool IsFileExtension(const char *fileName, const char *ext);
    
    DescribedSampler LoadSampler(addressMode amode, filterMode fmode);
    void UnloadSampler(DescribedSampler sampler);

    WGPUTexture GetActiveColorTarget(cwoid);
    Texture LoadTextureFromImage(Image img);
    void ImageFormat(Image* img, PixelFormat newFormat);
    Image LoadImageFromTexture(Texture tex);
    Image LoadImageFromTextureEx(WGPUTexture tex);
    void TakeScreenshot(const char* filename);
    Image LoadImage(const char* filename);
    Image ImageFromImage(Image img, Rectangle rec);
    
    uint32_t RoundUpToNextMultipleOf256(uint32_t x);
    void UnloadImage(Image img);
    void UnloadTexture(Texture tex);
    Image LoadImageFromMemory(const char* extension, const void* data, size_t dataSize);
    Image GenImageColor(Color a, uint32_t width, uint32_t height);
    Image GenImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount);
    void SaveImage(Image img, const char* filepath);

    unsigned char *DecodeDataBase64(const unsigned char *data, int *outputSize);
    char *EncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize);    
    float TextToFloat(const char *text);
    const char *TextToLower(const char *text);
    const char **TextSplit(const char *text, char delimiter, int *count);
    unsigned char *CompressData(const unsigned char *data, int dataSize, int *compDataSize);
    unsigned char *DecompressData(const unsigned char *compData, int compDataSize, int *dataSize);
    const char *CodepointToUTF8(int codepoint, int *utf8Size);
    int TextToInteger(const char *text);
    void UnloadCodepoints(int *codepoints);
    int *LoadCodepoints(const char *text, int *count);
    int GetCodepoint(const char *text, int *codepointSize);
    int GetCodepointPrevious(const char *text, int *codepointSize);
    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
    void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
    void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
    void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
    void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)
    int GetCodepointNext(const char *text, int *codepointSize);
    unsigned int TextLength(const char *text);
    
    void SetTextLineSpacing(int spacing);                                                 // Set vertical line spacing when drawing with line-breaks
    int MeasureText(const char *text, int fontSize);                                      // Measure string width for default font
    Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);    // Measure string size for Font
    int GetGlyphIndex(Font font, int codepoint);                                          // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
    GlyphInfo GetGlyphInfo(Font font, int codepoint);                                     // Get glyph font info data for a codepoint (unicode character), fallback to '?' if not found
    Rectangle GetGlyphAtlasRec(Font font, int codepoint);                                 // Get glyph rectangle in font atlas for a codepoint (unicode character), fallback to '?' if not found
    Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);
    Font LoadFontFromImage(Image img, Color key, int firstchar);
    Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount);
    void LoadFontDefault(void);
    Font GetFontDefault(void);
    GlyphInfo* LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
    Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);
    void SetShapesTexture(Texture tex, Rectangle rec);
    void UseTexture(Texture tex);
    void UseNoTexture(cwoid);
    static Color Fade(Color col, float fade_alpha){
        float v = (1.0f - fade_alpha);
        uint8_t a = (uint8_t)roundf(col.a * v);
        return CLITERAL(Color){col.r, col.g, col.b, a};
    }
    static Color GetColor(unsigned int hexValue){
        Color color;

        color.r = (uint8_t)(hexValue >> 24) & 0xFF;
        color.g = (uint8_t)(hexValue >> 16) & 0xFF;
        color.b = (uint8_t)(hexValue >> 8) & 0xFF;
        color.a = (uint8_t)hexValue & 0xFF;

        return color;
    }
    static void rlColor4f(float r, float g, float b, float alpha){
        nextcol.x = r;
        nextcol.y = g;
        nextcol.z = b;
        nextcol.w = alpha;
    }
    static void rlColor4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a){
        nextcol.x = ((int)r) / 255.0f;
        nextcol.y = ((int)g) / 255.0f;
        nextcol.z = ((int)b) / 255.0f;
        nextcol.w = ((int)a) / 255.0f;
    }
    static void rlColor3f(float r, float g, float b){
        rlColor4f(r, g, b, 1.0f);
    }

    static void rlTexCoord2f(float u, float v){
        nextuv.x = u;
        nextuv.y = v;
    }

    static void rlVertex2f(float x, float y){
        *(vboptr++) = CLITERAL(vertex){{x, y, 0}, nextuv, nextnormal, nextcol};
    }
    static void rlNormal3f(float x, float y, float z){
        nextnormal.x = x;
        nextnormal.y = y;
        nextnormal.z = z;
    }
    static void rlVertex3f(float x, float y, float z){
        *(vboptr++) = CLITERAL(vertex){{x, y, z}, nextuv, nextnormal, nextcol};
    }
    void rlBegin(enum draw_mode mode);
    void rlEnd(cwoid);

    void BeginTextureMode(RenderTexture rtex);
    void EndTextureMode(cwoid);
    void BeginWindowMode(SubWindow sw);
    void EndWindowMode(cwoid);

    void BindPipeline(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode);
    void BindComputePipeline(DescribedComputePipeline* pipeline);

    WGPUShaderModule LoadShaderFromMemory(const char* shaderSource);
    WGPUShaderModule LoadShader(const char* path);

    DescribedPipeline* ClonePipeline(const DescribedPipeline* pl);
    DescribedPipeline* LoadPipeline(const char* shaderSource);
    DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
    DescribedPipeline* LoadPipelineForVAO(const char* shaderSource, VertexArray* vao);
    DescribedPipeline* LoadPipelineForVAOEx(const char* shaderSource, VertexArray* vao, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);
    DescribedPipeline* DefaultPipeline(cwoid);
    RenderSettings GetDefaultSettings(cwoid);
    Texture GetDefaultTexture(cwoid);
    void UnloadPipeline(DescribedPipeline* pl);

    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
    const char* TextureFormatName(WGPUTextureFormat fmt);
    size_t GetPixelSizeInBytes(WGPUTextureFormat format);
    Texture LoadBlankTexture(uint32_t width, uint32_t height);
    Texture LoadTexture(const char* filename);
    Texture LoadDepthTexture(uint32_t width, uint32_t height);
    Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget);
    Texture LoadTexturePro(uint32_t width, uint32_t height, WGPUTextureFormat format, WGPUTextureUsage usage, uint32_t sampleCount);
    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
    void UpdateTexture(Texture tex, void* data);

    StagingBuffer GenStagingBuffer(size_t size, WGPUBufferUsage usage);
    void UpdateStagingBuffer(StagingBuffer* buffer);
    void RecreateStagingBuffer(StagingBuffer* buffer);
    //void MapStagingBuffer(size_t size, WGPUBufferUsage usage);
    void UnloadStagingBuffer(StagingBuffer* buf);
    
    DescribedBuffer* GenUniformBuffer(const void* data, size_t size);
    DescribedBuffer* GenStorageBuffer(const void* data, size_t size);
    DescribedBuffer* GenIndexBuffer(const void* data, size_t size);
    DescribedBuffer* GenBuffer(const void* data, size_t size);
    DescribedBuffer* GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage);
    void UnloadBuffer(DescribedBuffer* buffer);
    void BufferData(DescribedBuffer* buffer, const void* data, size_t size);
    void ResizeBuffer(DescribedBuffer* buffer, size_t newSize);
    void ResizeBufferAndConserve(DescribedBuffer* buffer, size_t newSize);
    void BindVertexBuffer(const DescribedBuffer* buffer);

    DescribedPipeline* GetActivePipeline(cwoid);

    uint32_t GetUniformLocation(DescribedPipeline* pl, const char* uniformName);
    uint32_t GetUniformLocationCompute(DescribedComputePipeline* pl, const char* uniformName);
    void SetPipelineTexture           (DescribedPipeline* pl, uint32_t index, Texture tex);
    void SetPipelineSampler           (DescribedPipeline* pl, uint32_t index, DescribedSampler sampler);
    void SetPipelineUniformBuffer     (DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer);
    void SetPipelineStorageBuffer     (DescribedPipeline* pl, uint32_t index, DescribedBuffer* buffer);
    void SetPipelineUniformBufferData (DescribedPipeline* pl, uint32_t index, const void* data, size_t size);
    void SetPipelineStorageBufferData (DescribedPipeline* pl, uint32_t index, const void* data, size_t size);

    void SetTexture                   (uint32_t index, Texture tex);
    void SetSampler                   (uint32_t index, DescribedSampler sampler);
    void SetUniformBuffer             (uint32_t index, DescribedBuffer* buffer);
    void SetStorageBuffer             (uint32_t index, DescribedBuffer* buffer);
    void SetUniformBufferData         (uint32_t index, const void* data, size_t size);
    void SetStorageBufferData         (uint32_t index, const void* data, size_t size);
    void SetBindgroupUniformBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer);
    void SetBindgroupStorageBuffer (DescribedBindGroup* bg, uint32_t index, DescribedBuffer* buffer);
    void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size);
    void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size);
    void SetBindgroupTexture(DescribedBindGroup* bg, uint32_t index, Texture tex);
    void SetBindgroupSampler(DescribedBindGroup* bg, uint32_t index, DescribedSampler sampler);



    void init_full_renderstate (full_renderstate* state, const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniform_count, WGPUTextureView c, WGPUTextureView d);
    void updatePipeline        (full_renderstate* state, enum draw_mode drawmode);
    void setTargetTextures     (full_renderstate* state, WGPUTextureView c, WGPUTextureView colorMultisample, WGPUTextureView d);

    /**
        The functions LoadVertexArray, VertexAttribPointer, EnableVertexAttribArray, DisableVertexAttribArray
        aim to replicate the behaviour of OpenGL as closely as possible.
     */
    VertexArray* LoadVertexArray(cwoid);
    void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, WGPUVertexFormat format, uint32_t offset, WGPUVertexStepMode stepmode);
    void EnableVertexAttribArray(VertexArray* array, uint32_t attribLocation);
    void DisableVertexAttribArray(VertexArray* array, uint32_t attribLocation);

    void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va);
    void BindVertexArray(DescribedPipeline* pipeline, VertexArray* va);

    void DrawArrays                (WGPUPrimitiveTopology drawMode, uint32_t vertexCount);
    void DrawArraysInstanced       (WGPUPrimitiveTopology drawMode, uint32_t vertexCount, uint32_t instanceCount);
    void DrawArraysIndexed         (WGPUPrimitiveTopology drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount);
    void DrawArraysIndexedInstanced(WGPUPrimitiveTopology drawMode, DescribedBuffer indexBuffer, uint32_t vertexCount, uint32_t instanceCount);

    Material LoadMaterialDefault(cwoid);
    ModelAnimation *LoadModelAnimations(const char *fileName, int *animCount);
    void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame);
    void UpdateModelAnimation(Model model, ModelAnimation anim, int frame);
    Model LoadModel(const char *fileName);                                                // Load model from files (meshes and materials)
    Model LoadModelFromMesh(Mesh mesh);                                                   // Load model from generated mesh (default material)
    bool IsModelValid(Model model);                                                       // Check if a model is valid (loaded in GPU, VAO/VBOs)
    void UnloadModel(Model model);                                                        // Unload model (including meshes) from memory (RAM and/or VRAM)
    BoundingBox GetModelBoundingBox(Model model);                                         // Compute model bounding box limits (considers all meshes)

    // Model drawing functions
    void DrawModel(Model model, Vector3 position, float scale, Color tint);                            // Draw a model (with texture if set)
    void DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model with extended parameters
    void DrawModelWires(Model model, Vector3 position, float scale, Color tint);                       // Draw a model wires (with texture if set)
    void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model wires (with texture if set) with extended parameters
    void DrawModelPoints(Model model, Vector3 position, float scale, Color tint);                      // Draw a model as points
    void DrawModelPointsEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint); // Draw a model as points with extended parameters
    void DrawBoundingBox(BoundingBox box, Color color);                                                // Draw bounding box (wires)
    void DrawBillboard(Camera camera, Texture2D texture, Vector3 position, float scale, Color tint);   // Draw a billboard texture
    void DrawBillboardRec(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector2 size, Color tint); // Draw a billboard texture defined by source
    void DrawBillboardPro(Camera camera, Texture2D texture, Rectangle source, Vector3 position, Vector3 up, Vector2 size, Vector2 origin, float rotation, Color tint); // Draw a billboard texture defined by source and rotation

    // Mesh management functions
    void UploadMesh(Mesh *mesh, bool dynamic);                                            // Upload mesh vertex data in GPU and provide VAO/VBO ids
    void UpdateMeshBuffer(Mesh mesh, int index, const void *data, int dataSize, int offset); // Update mesh vertex data in GPU for a specific buffer index
    void UnloadMesh(Mesh mesh);                                                           // Unload mesh data from CPU and GPU
    void DrawMesh(Mesh mesh, Material material, Matrix transform);                        // Draw a 3d mesh with material and transform
    void DrawMeshInstanced(Mesh mesh, Material material, const Matrix *transforms, int instances); // Draw multiple mesh instances with material and different transforms
    BoundingBox GetMeshBoundingBox(Mesh mesh);                                            // Compute mesh bounding box limits
    void GenMeshTangents(Mesh *mesh);                                                     // Compute mesh tangents
    Mesh GenMeshCube(float width, float height, float length);
    //bool ExportMesh(Mesh mesh, const char *fileName);                                     // Export mesh data to file, returns true on success
    //bool ExportMeshAsCode(Mesh mesh, const char *fileName);                               // Export mesh as code file (.h) defining multiple arrays of vertex attributes


    bool CheckCollisionPointRec(Vector2 point, Rectangle rec);
    const char *TextFormat(const char *text, ...);

    void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);
    void DrawTextureV(Texture texture, Vector2 position, Color tint);                                // Draw a Texture2D with position defined as Vector2
    void DrawTextureEx(Texture texture, Vector2 position, float rotation, float scale, Color tint);  // Draw a Texture2D with extended parameters
    void DrawTextureRec(Texture texture, Rectangle source, Vector2 position, Color tint);            // Draw a part of a texture defined by a rectangle
    void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
    //void DrawTextureNPatch(Texture2D texture, NPatchInfo nPatchInfo, Rectangle dest, Vector2 origin, float rotation, Color tint); // Draws a texture (or part of it) that stretches or shrinks nicely
    void DrawPixel(int posX, int posY, Color color);
    void DrawPixelV(Vector2 position, Color color);
    void DrawGrid(int slices, float spacing);
    void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color);
    void DrawLineV(Vector2 startPos, Vector2 endPos, Color color);
    void DrawLineStrip(const Vector2 *points, int pointCount, Color color);
    void DrawLineBezier(Vector2 startPos, Vector2 endPos, float thick, Color color);
    void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
    void DrawCircle(int centerX, int centerY, float radius, Color color);
    void DrawCircleV(Vector2 center, float radius, Color color);
    void DrawCircleSector(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
    void DrawCircleSectorLines(Vector2 center, float radius, float startAngle, float endAngle, int segments, Color color);
    void DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer);
    void DrawCircleLines(int centerX, int centerY, float radius, Color color);
    void DrawCircleLinesV(Vector2 center, float radius, Color color);
    void DrawEllipse(int centerX, int centerY, float radiusH, float radiusV, Color color);
    void DrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, Color color);
    void DrawRing(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
    void DrawRingLines(Vector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, Color color);
    void DrawRectangle(int posX, int posY, int width, int height, Color color);
    void DrawRectangleV(Vector2 position, Vector2 size, Color color);
    void DrawRectangleRec(Rectangle rec, Color color);
    void DrawRectanglePro(Rectangle rec, Vector2 origin, float rotation, Color color);
    void DrawRectangleGradientV(int posX, int posY, int width, int height, Color top, Color bottom);
    void DrawRectangleGradientH(int posX, int posY, int width, int height, Color left, Color right);
    void DrawRectangleGradientEx(Rectangle rec, Color topLeft, Color bottomLeft, Color topRight, Color bottomRight);
    void DrawRectangleLines(int posX, int posY, int width, int height, Color color);
    void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
    void DrawRectangleRounded(Rectangle rec, float roundness, int segments, Color color);
    void DrawRectangleRoundedLines(Rectangle rec, float roundness, int segments, Color color);
    void DrawRectangleRoundedLinesEx(Rectangle rec, float roundness, int segments, float lineThick, Color color);
    void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
    void DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
    void DrawTriangleFan(const Vector2 *points, int pointCount, Color color);
    void DrawTriangleStrip(const Vector2 *points, int pointCount, Color color);
    void DrawPoly(Vector2 center, int sides, float radius, float rotation, Color color);
    void DrawPolyLines(Vector2 center, int sides, float radius, float rotation, Color color);
    void DrawPolyLinesEx(Vector2 center, int sides, float radius, float rotation, float lineThick, Color color);
    void DrawSplineSegmentBezierQuadratic(Vector2 p1, Vector2 c2, Vector2 p3, float thick, Color color);
    void DrawSplineSegmentBezierCubic(Vector2 p1, Vector2 c2, Vector2 c3, Vector2 p4, float thick, Color color);
    
    inline uint32_t attributeSize(WGPUVertexFormat fmt){
        switch(fmt){
            case WGPUVertexFormat_Uint8x4:
            case WGPUVertexFormat_Float32:
            case WGPUVertexFormat_Uint32:
            case WGPUVertexFormat_Sint32:
            case WGPUVertexFormat_Float16x2:
            case WGPUVertexFormat_Uint16x2:
            case WGPUVertexFormat_Sint16x2:
            return 4;
            case WGPUVertexFormat_Float32x2:
            case WGPUVertexFormat_Uint32x2:
            case WGPUVertexFormat_Sint32x2:
            case WGPUVertexFormat_Float16x4:
            case WGPUVertexFormat_Uint16x4:
            case WGPUVertexFormat_Sint16x4:
            return 8;
            case WGPUVertexFormat_Float32x3:
            case WGPUVertexFormat_Uint32x3:
            case WGPUVertexFormat_Sint32x3:
            return 12;
            case WGPUVertexFormat_Float32x4:
            case WGPUVertexFormat_Uint32x4:
            case WGPUVertexFormat_Sint32x4:
            return 16;
            case WGPUVertexFormat_Float16:
            case WGPUVertexFormat_Uint16:
            case WGPUVertexFormat_Sint16:
            return 2;
            case WGPUVertexFormat_Uint8:
            return 1;
            default:
            break;
        }
        abort();
        return 0;
    }
EXTERN_C_END
typedef struct full_renderstate{
    WGPUShaderModule shader;
    WGPUTextureView color;
    WGPUTextureView depth;

    DescribedPipeline* defaultPipeline;
    DescribedPipeline* activePipeline;

    DescribedRenderpass clearPass;
    DescribedRenderpass renderpass;
    DescribedComputepass computepass;
    DescribedRenderpass* activeRenderpass;
    DescribedComputepass* activeComputepass;

    uint32_t renderExtentX; // Dimensions of the current viewport
    uint32_t renderExtentY; // Required for camera function
}full_renderstate;
typedef struct wgpustate wgpustate;
extern wgpustate g_wgpustate;
#ifdef __cplusplus
extern const std::unordered_map<WGPUTextureFormat, std::string> textureFormatSpellingTable;
#endif
#endif
