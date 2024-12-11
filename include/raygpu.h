#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <stdbool.h>
#include "macros_and_constants.h"
#include "mathutils.h"
#include "pipeline.h"
typedef struct vertex{
    Vector3 pos;
    Vector2 uv ;
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
    GRAYSCALE = 0x100000 //No WGPU_ equivalent
}PixelFormat;

typedef struct Image{
    PixelFormat format;
    uint32_t width, height;
    size_t rowStrideInBytes; // Does not have to match with width
                             // One reason for this is the fact that Texture to Buffer copy commands
                             // Have to have a multiple of 256 bytes as row length 
    void* data;
}Image;



//typedef struct ShaderInputs{
//    uint32_t per_vertex_count;
//    uint32_t per_vertex_sizes[8]; //In bytes
//
//    uint32_t per_instance_count;
//    uint32_t per_instance_sizes[8]; //In bytes
//
//    uint32_t uniform_count; //Also includes storage
//    enum uniform_type uniform_types[8];
//    uint32_t uniform_minsizes[8]; //In bytes
//}ShaderInputs;

typedef struct Texture{
    uint32_t width, height;
    WGPUTextureFormat format;
    WGPUTexture tex;
    WGPUTextureView view;
}Texture;

typedef struct Rectangle {
    float x, y, width, height;
} Rectangle;
typedef struct RenderTexture{
    Texture color;
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
EXTERN_C_BEGIN
    WGPUDevice GetDevice(cwoid);
    WGPUQueue GetQueue(cwoid);
EXTERN_C_END


typedef struct DescribedBuffer{
    WGPUBufferDescriptor descriptor;
    WGPUBuffer buffer;
}DescribedBuffer;

typedef struct StagingBuffer{
    DescribedBuffer gpuUsable;
    DescribedBuffer mappable;
    void* map; //Nullable
}StagingBuffer;
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
    Texture texture;      // Texture atlas containing the glyphs
    Rectangle *recs;        // Rectangles in texture for the glyphs
    GlyphInfo *glyphs;      // Glyphs info data
} Font;

extern Vector2 nextuv;
extern Vector4 nextcol;
extern StagingBuffer vboStaging;
extern vertex* vboptr;
extern vertex* vboptr_base;
//extern DescribedBuffer vbomap;

#define BLANK CLITERAL(Color){0,0,0,0}
#define BLACK CLITERAL(Color){0,0,0,255}
#define WHITE CLITERAL(Color){255,255,255,255}

enum draw_mode{
    RL_TRIANGLES, RL_TRIANGLE_STRIP, RL_QUADS
};

typedef struct full_renderstate full_renderstate;

typedef struct AttributeAndResidence{
    WGPUVertexAttribute attr;
    uint32_t bufferSlot; //Describes the actual buffer it will reside in
    WGPUVertexStepMode stepMode;
}AttributeAndResidence;

/**
 */
typedef struct VertexArray VertexArray;


EXTERN_C_BEGIN
    GLFWwindow* InitWindow(uint32_t width, uint32_t height);
    uint32_t GetScreenWidth (cwoid);
    uint32_t GetScreenHeight(cwoid);


    /**
     * @brief Get the time elapsed since InitWindow() in seconds since 
     * 
     * @return double
     */
    double GetTime(cwoid);
    /**
     * @brief Return the unix timestamp in nanosecond precision
     *
     * (implemented with high_resolution_clock::now().time_since_epoch())
     * @return uint64_t 
     */
    uint64_t NanoTime(cwoid);

    uint32_t GetFPS(cwoid);
    void ClearBackground(Color clearColor);
    void BeginDrawing(cwoid);
    void EndDrawing(cwoid);
    DescribedRenderpass LoadRenderpass(WGPUTextureView color, WGPUTextureView depth);
    DescribedRenderpass LoadRenderpassEx(WGPUTextureView color, WGPUTextureView depth, RenderSettings settings);
    void UpdateRenderpass(DescribedRenderpass* rp, RenderSettings newSettings);
    void UnloadRenderpass(DescribedRenderpass rp);
    
    void BeginRenderpassEx(DescribedRenderpass* renderPass);
    void EndRenderpassEx(DescribedRenderpass* renderPass);
    void BeginPipelineMode(DescribedPipeline* pipeline);
    void EndPipelineMode();
    void* LoadFileData(const char *fileName, size_t *dataSize);
    Texture LoadTextureFromImage(Image img);
    Image LoadImageFromTexture(Texture tex);
    Image LoadImage(const char* filename);
    Image ImageFromImage(Image img, Rectangle rec);
    void UnloadImage(Image img);
    void UnloadTexture(Texture tex);
    Image LoadImageFromMemory(const void* data, size_t dataSize);
    Image GenImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount);
    void SaveImage(Image img, const char* filepath);

    void DrawFPS(int posX, int posY);                                                     // Draw current FPS
    void DrawText(const char *text, int posX, int posY, int fontSize, Color color);       // Draw text (using default font)
    void DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
    void DrawTextPro(Font font, const char *text, Vector2 position, Vector2 origin, float rotation, float fontSize, float spacing, Color tint); // Draw text using Font and pro parameters (rotation)
    void DrawTextCodepoint(Font font, int codepoint, Vector2 position, float fontSize, Color tint); // Draw one character (codepoint)
    void DrawTextCodepoints(Font font, const int *codepoints, int codepointCount, Vector2 position, float fontSize, float spacing, Color tint); // Draw multiple character (codepoint)
    int GetCodepointNext(const char *text, int *codepointSize);
    unsigned int TextLength(const char *text);
    // Text font info functions
    void SetTextLineSpacing(int spacing);                                                 // Set vertical line spacing when drawing with line-breaks
    int MeasureText(const char *text, int fontSize);                                      // Measure string width for default font
    Vector2 MeasureTextEx(Font font, const char *text, float fontSize, float spacing);    // Measure string size for Font
    int GetGlyphIndex(Font font, int codepoint);                                          // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
    GlyphInfo GetGlyphInfo(Font font, int codepoint);                                     // Get glyph font info data for a codepoint (unicode character), fallback to '?' if not found
    Rectangle GetGlyphAtlasRec(Font font, int codepoint);                                 // Get glyph rectangle in font atlas for a codepoint (unicode character), fallback to '?' if not found
    typedef enum {
        FONT_DEFAULT = 0,               // Default font generation, anti-aliased
        FONT_BITMAP,                    // Bitmap font generation, no anti-aliasing
        FONT_SDF                        // SDF font generation, requires external shader
    } FontType;

    #define TRACELOG(...)
    Font LoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);
    Font LoadFontFromImage(Image img, Color key, int firstchar);
    Font LoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount);
    GlyphInfo *LoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type);
    Image GenImageFontAtlas(const GlyphInfo *glyphs, Rectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod);
    void UseTexture(Texture tex);
    void UseNoTexture(cwoid);
    inline void rlColor4f(float r, float g, float b, float alpha){
        nextcol.x = r;
        nextcol.y = g;
        nextcol.z = b;
        nextcol.w = alpha;
    }
    inline void rlTexCoord2f(float u, float v){
        nextuv.x = u;
        nextuv.y = v;
    }

    inline void rlVertex2f(float x, float y){
        *(vboptr++) = CLITERAL(vertex){{x, y, 0}, nextuv, nextcol};
    }
    inline void rlVertex3f(float x, float y, float z){
        *(vboptr++) = CLITERAL(vertex){{x, y, z}, nextuv, nextcol};
    }
    void rlBegin(enum draw_mode mode);
    void rlEnd(cwoid);

    void BeginTextureMode(RenderTexture rtex);
    void EndTextureMode(cwoid);

    void BindPipeline(DescribedPipeline* pipeline);

    WGPUShaderModule LoadShaderFromMemory(const char* shaderSource);
    WGPUShaderModule LoadShader(const char* path);

    DescribedPipeline* LoadPipelineEx(const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);

    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
    Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget);
    Texture LoadTexture(uint32_t width, uint32_t height);
    Texture LoadDepthTexture(uint32_t width, uint32_t height);
    RenderTexture LoadRenderTexture(uint32_t width, uint32_t height);
    
    StagingBuffer GenStagingBuffer(size_t size, WGPUBufferUsage usage);
    void UpdateStagingBuffer(StagingBuffer* buffer);
    void RecreateStagingBuffer(StagingBuffer* buffer);
    //void MapStagingBuffer(size_t size, WGPUBufferUsage usage);
    void UnloadStagingBuffer(StagingBuffer* buf);
    
    DescribedBuffer GenBuffer(const void* data, size_t size);
    DescribedBuffer GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage);
    void BufferData(DescribedBuffer* buffer, const void* data, size_t size);
    void ResizeBuffer(DescribedBuffer* buffer, size_t newSize);
    void ResizeBufferAndConserve(DescribedBuffer* buffer, size_t newSize);
    void BindVertexBuffer(const DescribedBuffer* buffer);

    void SetTexture       (uint32_t index, Texture tex);
    void SetSampler       (uint32_t index, WGPUSampler sampler);
    void SetUniformBuffer (uint32_t index, const void* data, size_t size);
    void SetStorageBuffer (uint32_t index, const void* data, size_t size);
    
    void init_full_renderstate (full_renderstate* state, const char* shaderSource, const AttributeAndResidence* attribs, uint32_t attribCount, const UniformDescriptor* uniforms, uint32_t uniform_count, WGPUTextureView c, WGPUTextureView d);
    //void updateVertexBuffer    (full_renderstate* state, const void* data, size_t size);
    //void setStateTexture       (full_renderstate* state, uint32_t index, Texture tex);
    //void setStateSampler       (full_renderstate* state, uint32_t index, WGPUSampler sampler);
    //void setStateUniformBuffer (full_renderstate* state, uint32_t index, const void* data, size_t size);
    //void setStateStorageBuffer (full_renderstate* state, uint32_t index, const void* data, size_t size);
    //void updateBindGroup       (full_renderstate* state);
    void updatePipeline        (full_renderstate* state, enum draw_mode drawmode);
    //void updateRenderPassDesc  (full_renderstate* state);
    void setTargetTextures     (full_renderstate* state, WGPUTextureView c, WGPUTextureView d);

    VertexArray* LoadVertexArray(cwoid);
    void VertexAttribPointer(VertexArray* array, DescribedBuffer* buffer, uint32_t attribLocation, WGPUVertexFormat format, uint32_t offset, WGPUVertexStepMode stepmode);
    void EnableVertexAttribArray(VertexArray* array, uint32_t attribLocation);
    
    void PreparePipeline(DescribedPipeline* pipeline, VertexArray* va);
    void BindVertexArray(DescribedPipeline* pipeline, VertexArray* va);
    
    void DrawArrays(uint32_t vertexCount);
    void DrawTexturePro(Texture texture, Rectangle source, Rectangle dest, Vector2 origin, float rotation, Color tint);

    
    inline uint32_t attributeSize(WGPUVertexFormat fmt){
        switch(fmt){
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
    DescribedPipeline* currentPipeline;

    DescribedRenderpass clearPass;
    DescribedRenderpass renderpass;
    DescribedRenderpass* activeRenderPass;

    //WGPUBindGroupLayoutDescriptor bglayoutdesc;
    //WGPUBindGroupLayout bglayout;
    //WGPURenderPipelineDescriptor pipelineDesc;
    
    
    //#ifdef __cplusplus
    //template<typename callable>
    //void executeRenderpass(callable&& c){
    //    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    //    commandEncoderDesc.label = STRVIEW("Command Encoder");
    //    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
    //    
    //    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    //    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline.pipeline);
    //    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, GetWGPUBindGroup(&pipeline.bindGroup), 0, 0);
    //    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, this->vbo, 0, wgpuBufferGetSize(vbo));
    //    c(renderPass);
    //    //if constexpr(std::is_invocable_v<callable, WGPURenderPassEncoder>){
    //    //}
    //    //else if constexpr(std::is_invocable_v<callable, WGPURenderPassEncoder, WGPUCommandEncoder>){
    //    //    c(renderPass, encoder);
    //    //}
    //    wgpuRenderPassEncoderEnd(renderPass);
    //    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    //    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    //    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    //    wgpuQueueSubmit(GetQueue(), 1, &command);
    //    wgpuCommandEncoderRelease(encoder);
    //    wgpuCommandBufferRelease(command);
    //}
    //template<typename callable>
    //void executeRenderpassPlain(callable&& c){
    //    WGPUCommandEncoderDescriptor commandEncoderDesc = {};
    //    commandEncoderDesc.label = STRVIEW("Command Encoder");
    //    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(GetDevice(), &commandEncoderDesc);
    //    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    //    c(renderPass);
    //    wgpuRenderPassEncoderEnd(renderPass);
    //    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    //    cmdBufferDescriptor.label = STRVIEW("Command buffer");
    //    WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    //    wgpuQueueSubmit(GetQueue(), 1, &command);
    //    wgpuCommandEncoderRelease(encoder);
    //    wgpuCommandBufferRelease(command);
    //}
    //#endif
}full_renderstate;
typedef struct wgpustate wgpustate;
extern wgpustate g_wgpustate;

