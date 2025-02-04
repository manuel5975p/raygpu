#include <filesystem>
#include <raygpu.h>
#include <unordered_set>
#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include <optional>
#include <chrono>


inline std::ostream& operator<<(std::ostream& ostr, const wgpu::StringView& st){
    ostr.write(st.data, st.length);
    return ostr;
}

constexpr char shaderSource[] = R"(
struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
    @location(2) normal: vec3f,
    @location(3) color: vec4f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
    @location(1) color: vec4f,
};

struct LightBuffer {
    count: u32,
    positions: array<vec3f>
};

@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var texture0: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
@group(0) @binding(4) var<storage> lights: LightBuffer;
@group(0) @binding(5) var<storage> lights2: LightBuffer;

//Can be omitted
//@group(0) @binding(3) var<storage> storig: array<vec4f>;


@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * 
                   modelMatrix[instanceIdx] *
    vec4f(in.position.xyz, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return textureSample(texture0, texSampler, in.uv).rgba * in.color;
}
)";

constexpr char vertexSourceGLSL[] = R"(#version 450

// Input attributes.
layout(location = 0) in vec3 in_position;  // position
layout(location = 1) in vec2 in_uv;        // texture coordinates
layout(location = 2) in vec3 in_normal;    // normal (unused)
layout(location = 3) in vec4 in_color;     // vertex color

// Outputs to fragment shader.
layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec4 frag_color;

// Uniform block for Perspective_View matrix (binding = 0).
layout(binding = 0) uniform PerspectiveViewBlock {
    mat4 Perspective_View;
};

// Storage buffer for model matrices (binding = 3).
// Note: 'buffer' qualifier makes it a shader storage buffer.
layout(binding = 3) buffer ModelMatrixBlock {
    mat4 modelMatrix[];  // Array of model matrices.
};

void main() {
    // Compute transformed position using instance-specific model matrix.
    gl_Position = Perspective_View * modelMatrix[gl_InstanceID] * vec4(in_position, 1.0);
    frag_uv = in_uv;
    frag_color = in_color;
}
)";

constexpr char fragmentSourceGLSL[] = R"(
#version 450
#extension GL_ARB_separate_shader_objects : enable  // Enable separate sampler objects if needed

// Inputs from vertex shader.
layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec4 frag_color;

// Output fragment color.
layout(location = 0) out vec4 outColor;

// Texture and sampler, bound separately.
layout(binding = 1) uniform texture2D texture0;  // Texture (binding = 1)
layout(binding = 2) uniform sampler texSampler;    // Sampler (binding = 2)

void main() {
    // Sample the texture using the combined sampler.
    vec4 texColor = texture(sampler2D(texture0, texSampler), frag_uv);
    outColor = texColor * frag_color;
}
)";



struct full_renderstate;
#include "wgpustate.inc"
struct webgpu_cxx_state{
    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
    //wgpu::Surface surface{};
    //full_renderstate* rstate = nullptr;
    //Texture depthTexture{};
};

void PollEvents(){
    #if SUPPORT_SDL2 != 0
    PollEvents_SDL2();
    #endif
    #if SUPPORT_GLFW != 0
    PollEvents_GLFW();
    #endif
}
void* GetActiveWindowHandle(){
    if(g_renderstate.activeSubWindow.handle)return g_renderstate.activeSubWindow.handle;
    return g_renderstate.window;
}
bool WindowShouldClose(cwoid){
    #ifdef MAIN_WINDOW_SDL2
    return g_renderstate.closeFlag;
    #elif defined(MAIN_WINDOW_GLFW)
    return WindowShouldClose_GLFW(g_wgpustate.window);
    #else
    return false;
    #endif
}



























extern "C" DescribedPipeline* LoadPipelineForVAO_Vk(const char* vsSource, const char* fsSource, const VertexArray* vao, const ResourceTypeDescriptor* uniforms, uint32_t uniformCount, RenderSettings settings);

void* InitWindow(uint32_t width, uint32_t height, const char* title){
    #if FORCE_HEADLESS == 1
    g_renderstate.windowFlags |= FLAG_HEADLESS;
    #endif
    //TODO: fix this, preferrably set correct format in InitWindow_SDL or GLFW
    g_renderstate.frameBufferFormat = BGRA8;
    if(g_renderstate.windowFlags & FLAG_STDOUT_TO_FFMPEG){
        if(IsATerminal(stdout)){
            TRACELOG(LOG_ERROR, "Refusing to pipe video output to terminal");
            TRACELOG(LOG_ERROR, "Try <program> | ffmpeg -y -f rawvideo -pix_fmt rgba -s %ux%u -r 60 -i - -vf format=yuv420p out.mp4", width, height);
            exit(1);
        }
        SetTraceLogFile(stderr);
    }
    TRACELOG(LOG_INFO, "Hello!");
    std::string working_dir = std::filesystem::absolute(std::filesystem::current_path()).string();
    if(!working_dir.ends_with('/')){
        working_dir += '/';
    }
    TRACELOG(LOG_INFO, "Working directory: %s", working_dir.c_str());
    g_renderstate.last_timestamps[0] = NanoTime();
    
    InitBackend();
    g_renderstate.width = width;
    g_renderstate.height = height;
    
    g_renderstate.whitePixel = LoadTextureFromImage(GenImageChecker(Color{255,255,255,255}, Color{255,255,255,255}, 1, 1, 0));
    TraceLog(LOG_INFO, "Loaded whitepixel texture");

    //DescribedShaderModule tShader = LoadShaderModuleFromMemory(shaderSource);
    
    Matrix identity = MatrixIdentity();
    g_renderstate.identityMatrix = GenStorageBuffer(&identity, sizeof(Matrix));

    g_renderstate.grst = (GIFRecordState*)calloc(1, 160);



    

    //void* window = nullptr;
    if(!(g_renderstate.windowFlags & FLAG_HEADLESS)){
        #if SUPPORT_GLFW == 1 || SUPPORT_SDL2 == 1
        #ifdef MAIN_WINDOW_GLFW
        SubWindow createdWindow = InitWindow_GLFW(width, height, title);
        #else
        Initialize_SDL2();
        SubWindow createdWindow = InitWindow_SDL2(width, height, title);
        #endif
        g_renderstate.window = (GLFWwindow*)createdWindow.handle;
        auto it = g_renderstate.createdSubwindows.find(g_renderstate.window);
        g_renderstate.mainWindow = &it->second;
        //g_wgpustate.surface = wgpu::Surface((WGPUSurface)g_wgpustate.createdSubwindows[glfwWin.handle].surface.surface);
        #endif
        

        
        
        
        //#ifndef __EMSCRIPTEN__
        
        //std::cout << "Supported Framebuffer Format: 0x" << std::hex << (WGPUTextureFormat)config.format << std::dec << "\n";        
    }else{
        g_renderstate.frameBufferFormat = BGRA8;
    }
    


    ResourceTypeDescriptor uniforms[4] = {
        ResourceTypeDescriptor{uniform_buffer, 64, 0, readonly, format_or_sample_type(0)},
        ResourceTypeDescriptor{texture2d, 0, 1      , readonly, format_or_sample_type(0)},
        ResourceTypeDescriptor{texture_sampler, 0, 2, readonly, format_or_sample_type(0)},
        ResourceTypeDescriptor{storage_buffer, 64, 3, readonly, format_or_sample_type(0)}
    };

    AttributeAndResidence attrs[4] = {
        AttributeAndResidence{VertexAttribute{VertexFormat_Float32x3, 0 * sizeof(float), 0}, 0, VertexStepMode_Vertex, true},
        AttributeAndResidence{VertexAttribute{VertexFormat_Float32x2, 3 * sizeof(float), 1}, 0, VertexStepMode_Vertex, true},
        AttributeAndResidence{VertexAttribute{VertexFormat_Float32x3, 5 * sizeof(float), 2}, 0, VertexStepMode_Vertex, true},
        AttributeAndResidence{VertexAttribute{VertexFormat_Float32x4, 8 * sizeof(float), 3}, 0, VertexStepMode_Vertex, true},
    };
    
    //arraySetter(shaderInputs.per_vertex_sizes, {3,2,4});
    //arraySetter(shaderInputs.uniform_minsizes, {64, 0, 0, 0});
    //uarraySetter(shaderInputs.uniform_types, {uniform_buffer, texture2d, sampler, storage_buffer});
    auto colorTexture = LoadTextureEx(width, height, (PixelFormat)g_renderstate.frameBufferFormat, true);
    //g_wgpustate.mainWindowRenderTarget.texture = colorTexture;
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT)
        g_renderstate.mainWindowRenderTarget.colorMultisample = LoadTexturePro(width, height, (PixelFormat)g_renderstate.frameBufferFormat, TextureUsage_RenderAttachment | TextureUsage_TextureBinding | TextureUsage_CopyDst | TextureUsage_CopySrc, 4, 1);
    auto depthTexture = LoadTexturePro(width,
                                  height, 
                                  Depth32, 
                                  WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                                  (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                                  1
    );
    g_renderstate.mainWindowRenderTarget.depth = depthTexture;
    //init_full_renderstate(g_renderstate.rstate, shaderSource, attrs, 4, uniforms, sizeof(uniforms) / sizeof(ResourceTypeDescriptor), (WGPUTextureView)colorTexture.view, (WGPUTextureView)g_renderstate.mainWindowRenderTarget.depth.view);
    TRACELOG(LOG_INFO, "Renderstate inited");
    g_renderstate.renderExtentX = width;
    g_renderstate.renderExtentY = height;
    

    LoadFontDefault();
    for(size_t i = 0;i < 512;i++){
        g_renderstate.smallBufferPool.push_back(GenVertexBuffer(nullptr, sizeof(vertex) * 32));
    }

    vboptr = (vertex*)std::calloc(10000, sizeof(vertex));
    vboptr_base = vboptr;
    renderBatchVBO = GenVertexBuffer(nullptr, 10000 * sizeof(vertex));
    
    renderBatchVAO = LoadVertexArray();
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 0, VertexFormat_Float32x3, 0 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 1, VertexFormat_Float32x2, 3 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 2, VertexFormat_Float32x3, 5 * sizeof(float), VertexStepMode_Vertex);
    VertexAttribPointer(renderBatchVAO, renderBatchVBO, 3, VertexFormat_Float32x4, 8 * sizeof(float), VertexStepMode_Vertex);


    g_renderstate.renderpass = LoadRenderpassEx(GetDefaultSettings(), false, DColor{}, false, 0.0f);

    g_renderstate.clearPass = LoadRenderpassEx(GetDefaultSettings(), true, DColor{0,0,0,1}, true, 1.0f);
    //}
    //state->clearPass.rca->clearValue = WGPUColor{1.0, 0.4, 0.2, 1.0};
    //state->clearPass.rca->loadOp = WGPULoadOp_Clear;
    //state->clearPass.rca->storeOp = WGPUStoreOp_Store;
    //state->activeRenderpass = nullptr;
    #if SUPPORT_VULKAN_BACKEND == 1
    g_renderstate.defaultPipeline = LoadPipelineForVAO_Vk(vertexSourceGLSL, fragmentSourceGLSL, renderBatchVAO, uniforms, sizeof(uniforms) / sizeof(ResourceTypeDescriptor), GetDefaultSettings());
    #else
    g_renderstate.defaultPipeline = LoadPipelineForVAOEx(shaderSource, renderBatchVAO, uniforms, sizeof(uniforms) / sizeof(ResourceTypeDescriptor), GetDefaultSettings());
    #endif
    g_renderstate.activePipeline = g_renderstate.defaultPipeline;
    g_renderstate.quadindicesCache = callocnew(DescribedBuffer);    //WGPUBufferDescriptor vbmdesc{};
    g_renderstate.quadindicesCache->usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Index;
    //WGPUCommandEncoderDescriptor cedesc{};
    //cedesc.label = STRVIEW("Global Command Encoder");
    //g_renderstate.renderpass.cmdEncoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device.Get(), &cedesc);
    Matrix m = ScreenMatrix(width, height);
    static_assert(sizeof(Matrix) == 64, "non 4 byte floats? or what");
    //g_wgpustate.defaultScreenMatrix = GenUniformBuffer(&m, sizeof(Matrix));
    //SetUniformBuffer(0, g_wgpustate.defaultScreenMatrix);
    SetTexture(1, g_renderstate.whitePixel);
    Matrix iden = MatrixIdentity();
    SetStorageBufferData(3, &iden, 64);

    WGPUSamplerDescriptor samplerDesc{};
    samplerDesc.addressModeU = WGPUAddressMode_Repeat;
    samplerDesc.addressModeV = WGPUAddressMode_Repeat;
    samplerDesc.addressModeW = WGPUAddressMode_Repeat;
    samplerDesc.magFilter    = WGPUFilterMode_Linear;
    samplerDesc.minFilter    = WGPUFilterMode_Linear;
    samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    samplerDesc.compare      = WGPUCompareFunction_Undefined;
    samplerDesc.lodMinClamp  = 0.0f;
    samplerDesc.lodMaxClamp  = 1.0f;
    samplerDesc.maxAnisotropy = 1;


    DescribedSampler sampler = LoadSampler(repeat, filter_linear);
    g_renderstate.defaultSampler = sampler;
    SetSampler(2, sampler);
    g_renderstate.init_timestamp = NanoTime();
    #ifndef __EMSCRIPTEN__
    if((g_renderstate.windowFlags & FLAG_VSYNC_HINT))
        SetTargetFPS(60);
    //if(!(g_wgpustate.windowFlags & FLAG_HEADLESS) && (g_wgpustate.windowFlags & FLAG_VSYNC_HINT)){
    //    auto rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
    //    SetTargetFPS(rate);
    //}
    else
        SetTargetFPS(0);
    
    #endif
    #ifndef __EMSCRIPTEN__
    return nullptr;
    #else
    return nullptr;
    #endif

}
extern "C" SubWindow OpenSubWindow(uint32_t width, uint32_t height, const char* title){
    #ifdef MAIN_WINDOW_GLFW
    return OpenSubWindow_GLFW(width, height, title);
    #elif defined(MAIN_WINDOW_SDL2)
    return OpenSubWindow_SDL2(width, height, title);
    #endif
    return SubWindow zeroinit;
}
extern "C" void ToggleFullscreenImpl(){
    #ifdef MAIN_WINDOW_GLFW
    ToggleFullscreen_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    ToggleFullscreen_SDL2();
    #endif
}
extern "C" void ToggleFullscreen(){
    g_renderstate.wantsToggleFullscreen = true;
}
Vector2 GetTouchPosition(int index){
    #ifdef MAIN_WINDOW_GLFW
    return GetTouchPointCount_GLFW(index);
    #elif defined(MAIN_WINDOW_SDL2)
    return GetTouchPosition_SDL2(index);
    #else
    return Vector2{0, 0};
    #endif
}
int GetTouchPointCount(cwoid){
    #ifdef MAIN_WINDOW_GLFW
    return GetTouchPointCount_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    return GetTouchPointCount_SDL2();
    #else
    return 0;
    #endif
}
uint32_t GetMonitorWidth(cwoid){
    #ifdef MAIN_WINDOW_GLFW
    return GetMonitorWidth_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    return GetMonitorWidth_SDL2();
    #else
    return 0;
    #endif
}
void SetWindowShouldClose(){
    #ifdef MAIN_WINDOW_GLFW
    return SetWindowShouldClose_GLFW(g_wgpustate.window);
    #elif defined(MAIN_WINDOW_SDL2)
    g_renderstate.closeFlag = true;
    #endif
}
uint32_t GetMonitorHeight(cwoid){
    #ifdef MAIN_WINDOW_GLFW
    return GetMonitorHeight_GLFW();
    #elif defined(MAIN_WINDOW_SDL2)
    return GetMonitorHeight_SDL2();
    #else
    return 0;
    #endif
}

const std::unordered_map<WGPUFeatureName, std::string> featureSpellingTable = [](){
    std::unordered_map<WGPUFeatureName, std::string> ret;
    #ifdef __EMSCRIPTEN__
    ret[WGPUFeatureName_DepthClipControl] = "DepthClipControl";
    ret[WGPUFeatureName_Depth32FloatStencil8] = "Depth32FloatStencil8";
    ret[WGPUFeatureName_TimestampQuery] = "TimestampQuery";
    ret[WGPUFeatureName_TextureCompressionBC] = "TextureCompressionBC";
    ret[WGPUFeatureName_TextureCompressionETC2] = "TextureCompressionETC2";
    ret[WGPUFeatureName_TextureCompressionASTC] = "TextureCompressionASTC";
    ret[WGPUFeatureName_IndirectFirstInstance] = "IndirectFirstInstance";
    ret[WGPUFeatureName_ShaderF16] = "ShaderF16";
    ret[WGPUFeatureName_RG11B10UfloatRenderable] = "RG11B10UfloatRenderable";
    ret[WGPUFeatureName_BGRA8UnormStorage] = "BGRA8UnormStorage";
    ret[WGPUFeatureName_Float32Filterable] = "Float32Filterable";
    ret[WGPUFeatureName_Float32Blendable] = "Float32Blendable";
    ret[WGPUFeatureName_Subgroups] = "Subgroups";
    ret[WGPUFeatureName_SubgroupsF16] = "SubgroupsF16";
    ret[WGPUFeatureName_Force32] = "Force32";
    #else
    ret[WGPUFeatureName_DepthClipControl] = "DepthClipControl";
    ret[WGPUFeatureName_Depth32FloatStencil8] = "Depth32FloatStencil8";
    ret[WGPUFeatureName_TimestampQuery] = "TimestampQuery";
    ret[WGPUFeatureName_TextureCompressionBC] = "TextureCompressionBC";
    ret[WGPUFeatureName_TextureCompressionETC2] = "TextureCompressionETC2";
    ret[WGPUFeatureName_TextureCompressionASTC] = "TextureCompressionASTC";
    ret[WGPUFeatureName_IndirectFirstInstance] = "IndirectFirstInstance";
    ret[WGPUFeatureName_ShaderF16] = "ShaderF16";
    ret[WGPUFeatureName_RG11B10UfloatRenderable] = "RG11B10UfloatRenderable";
    ret[WGPUFeatureName_BGRA8UnormStorage] = "BGRA8UnormStorage";
    ret[WGPUFeatureName_Float32Filterable] = "Float32Filterable";
    ret[WGPUFeatureName_Float32Blendable] = "Float32Blendable";
    ret[WGPUFeatureName_Subgroups] = "Subgroups";
    ret[WGPUFeatureName_SubgroupsF16] = "SubgroupsF16";
    ret[WGPUFeatureName_DawnInternalUsages] = "DawnInternalUsages";
    ret[WGPUFeatureName_DawnMultiPlanarFormats] = "DawnMultiPlanarFormats";
    ret[WGPUFeatureName_DawnNative] = "DawnNative";
    ret[WGPUFeatureName_ChromiumExperimentalTimestampQueryInsidePasses] = "ChromiumExperimentalTimestampQueryInsidePasses";
    ret[WGPUFeatureName_ImplicitDeviceSynchronization] = "ImplicitDeviceSynchronization";
    ret[WGPUFeatureName_ChromiumExperimentalImmediateData] = "ChromiumExperimentalImmediateData";
    ret[WGPUFeatureName_TransientAttachments] = "TransientAttachments";
    ret[WGPUFeatureName_MSAARenderToSingleSampled] = "MSAARenderToSingleSampled";
    ret[WGPUFeatureName_DualSourceBlending] = "DualSourceBlending";
    ret[WGPUFeatureName_D3D11MultithreadProtected] = "D3D11MultithreadProtected";
    ret[WGPUFeatureName_ANGLETextureSharing] = "ANGLETextureSharing";
    ret[WGPUFeatureName_PixelLocalStorageCoherent] = "PixelLocalStorageCoherent";
    ret[WGPUFeatureName_PixelLocalStorageNonCoherent] = "PixelLocalStorageNonCoherent";
    ret[WGPUFeatureName_Unorm16TextureFormats] = "Unorm16TextureFormats";
    ret[WGPUFeatureName_Snorm16TextureFormats] = "Snorm16TextureFormats";
    ret[WGPUFeatureName_MultiPlanarFormatExtendedUsages] = "MultiPlanarFormatExtendedUsages";
    ret[WGPUFeatureName_MultiPlanarFormatP010] = "MultiPlanarFormatP010";
    ret[WGPUFeatureName_HostMappedPointer] = "HostMappedPointer";
    ret[WGPUFeatureName_MultiPlanarRenderTargets] = "MultiPlanarRenderTargets";
    ret[WGPUFeatureName_MultiPlanarFormatNv12a] = "MultiPlanarFormatNv12a";
    ret[WGPUFeatureName_FramebufferFetch] = "FramebufferFetch";
    ret[WGPUFeatureName_BufferMapExtendedUsages] = "BufferMapExtendedUsages";
    ret[WGPUFeatureName_AdapterPropertiesMemoryHeaps] = "AdapterPropertiesMemoryHeaps";
    ret[WGPUFeatureName_AdapterPropertiesD3D] = "AdapterPropertiesD3D";
    ret[WGPUFeatureName_AdapterPropertiesVk] = "AdapterPropertiesVk";
    ret[WGPUFeatureName_R8UnormStorage] = "R8UnormStorage";
    ret[WGPUFeatureName_Norm16TextureFormats] = "Norm16TextureFormats";
    ret[WGPUFeatureName_MultiPlanarFormatNv16] = "MultiPlanarFormatNv16";
    ret[WGPUFeatureName_MultiPlanarFormatNv24] = "MultiPlanarFormatNv24";
    ret[WGPUFeatureName_MultiPlanarFormatP210] = "MultiPlanarFormatP210";
    ret[WGPUFeatureName_MultiPlanarFormatP410] = "MultiPlanarFormatP410";
    ret[WGPUFeatureName_SharedTextureMemoryVkDedicatedAllocation] = "SharedTextureMemoryVkDedicatedAllocation";
    ret[WGPUFeatureName_SharedTextureMemoryAHardwareBuffer] = "SharedTextureMemoryAHardwareBuffer";
    ret[WGPUFeatureName_SharedTextureMemoryDmaBuf] = "SharedTextureMemoryDmaBuf";
    ret[WGPUFeatureName_SharedTextureMemoryOpaqueFD] = "SharedTextureMemoryOpaqueFD";
    ret[WGPUFeatureName_SharedTextureMemoryZirconHandle] = "SharedTextureMemoryZirconHandle";
    ret[WGPUFeatureName_SharedTextureMemoryDXGISharedHandle] = "SharedTextureMemoryDXGISharedHandle";
    ret[WGPUFeatureName_SharedTextureMemoryD3D11Texture2D] = "SharedTextureMemoryD3D11Texture2D";
    ret[WGPUFeatureName_SharedTextureMemoryIOSurface] = "SharedTextureMemoryIOSurface";
    ret[WGPUFeatureName_SharedTextureMemoryEGLImage] = "SharedTextureMemoryEGLImage";
    ret[WGPUFeatureName_SharedFenceVkSemaphoreOpaqueFD] = "SharedFenceVkSemaphoreOpaqueFD";
    ret[WGPUFeatureName_SharedFenceSyncFD] = "SharedFenceSyncFD";
    ret[WGPUFeatureName_SharedFenceVkSemaphoreZirconHandle] = "SharedFenceVkSemaphoreZirconHandle";
    ret[WGPUFeatureName_SharedFenceDXGISharedHandle] = "SharedFenceDXGISharedHandle";
    ret[WGPUFeatureName_SharedFenceMTLSharedEvent] = "SharedFenceMTLSharedEvent";
    ret[WGPUFeatureName_SharedBufferMemoryD3D12Resource] = "SharedBufferMemoryD3D12Resource";
    ret[WGPUFeatureName_StaticSamplers] = "StaticSamplers";
    ret[WGPUFeatureName_YCbCrVulkanSamplers] = "YCbCrVulkanSamplers";
    ret[WGPUFeatureName_ShaderModuleCompilationOptions] = "ShaderModuleCompilationOptions";
    ret[WGPUFeatureName_DawnLoadResolveTexture] = "DawnLoadResolveTexture";
    ret[WGPUFeatureName_DawnPartialLoadResolveTexture] = "DawnPartialLoadResolveTexture";
    ret[WGPUFeatureName_MultiDrawIndirect] = "MultiDrawIndirect";
    ret[WGPUFeatureName_ClipDistances] = "ClipDistances";
    ret[WGPUFeatureName_DawnTexelCopyBufferRowAlignment] = "DawnTexelCopyBufferRowAlignment";
    ret[WGPUFeatureName_FlexibleTextureViews] = "FlexibleTextureViews";
    ret[WGPUFeatureName_Force32] = "Force32";
    #endif
    return ret;
}();

const std::unordered_map<WGPUBackendType, std::string> backendTypeSpellingTable = [](){
    std::unordered_map<WGPUBackendType, std::string> map;
    map[WGPUBackendType_Undefined] = "Undefined";
    map[WGPUBackendType_Null] = "Null";
    map[WGPUBackendType_WebGPU] = "WebGPU";
    map[WGPUBackendType_D3D11] = "D3D11";
    map[WGPUBackendType_D3D12] = "D3D12";
    map[WGPUBackendType_Metal] = "Metal";
    map[WGPUBackendType_Vulkan] = "Vulkan";
    map[WGPUBackendType_OpenGL] = "OpenGL";
    map[WGPUBackendType_OpenGLES] = "OpenGLES";
    return map;
}();
const std::unordered_map<WGPUPresentMode, std::string> presentModeSpellingTable = [](){
    std::unordered_map<WGPUPresentMode, std::string> map;
    map[WGPUPresentMode_Fifo] = "WGPUPresentMode_Fifo";
    map[WGPUPresentMode_FifoRelaxed] = "WGPUPresentMode_FifoRelaxed";
    map[WGPUPresentMode_Immediate] = "WGPUPresentMode_Immediate";
    map[WGPUPresentMode_Mailbox] = "WGPUPresentMode_Mailbox";
    map[WGPUPresentMode_Force32] = "WGPUPresentMode_Force32";
    return map;
}();
const std::unordered_map<WGPUTextureFormat, std::string> textureFormatSpellingTable = [](){
    std::unordered_map<WGPUTextureFormat, std::string> map;
    map[WGPUTextureFormat_Undefined] = "WGPUTextureFormat_Undefined";
    map[WGPUTextureFormat_R8Unorm] = "WGPUTextureFormat_R8Unorm";
    map[WGPUTextureFormat_R8Snorm] = "WGPUTextureFormat_R8Snorm";
    map[WGPUTextureFormat_R8Uint] = "WGPUTextureFormat_R8Uint";
    map[WGPUTextureFormat_R8Sint] = "WGPUTextureFormat_R8Sint";
    map[WGPUTextureFormat_R16Uint] = "WGPUTextureFormat_R16Uint";
    map[WGPUTextureFormat_R16Sint] = "WGPUTextureFormat_R16Sint";
    map[WGPUTextureFormat_R16Float] = "WGPUTextureFormat_R16Float";
    map[WGPUTextureFormat_RG8Unorm] = "WGPUTextureFormat_RG8Unorm";
    map[WGPUTextureFormat_RG8Snorm] = "WGPUTextureFormat_RG8Snorm";
    map[WGPUTextureFormat_RG8Uint] = "WGPUTextureFormat_RG8Uint";
    map[WGPUTextureFormat_RG8Sint] = "WGPUTextureFormat_RG8Sint";
    map[WGPUTextureFormat_R32Float] = "WGPUTextureFormat_R32Float";
    map[WGPUTextureFormat_R32Uint] = "WGPUTextureFormat_R32Uint";
    map[WGPUTextureFormat_R32Sint] = "WGPUTextureFormat_R32Sint";
    map[WGPUTextureFormat_RG16Uint] = "WGPUTextureFormat_RG16Uint";
    map[WGPUTextureFormat_RG16Sint] = "WGPUTextureFormat_RG16Sint";
    map[WGPUTextureFormat_RG16Float] = "WGPUTextureFormat_RG16Float";
    map[WGPUTextureFormat_RGBA8Unorm] = "WGPUTextureFormat_RGBA8Unorm";
    map[WGPUTextureFormat_RGBA8UnormSrgb] = "WGPUTextureFormat_RGBA8UnormSrgb";
    map[WGPUTextureFormat_RGBA8Snorm] = "WGPUTextureFormat_RGBA8Snorm";
    map[WGPUTextureFormat_RGBA8Uint] = "WGPUTextureFormat_RGBA8Uint";
    map[WGPUTextureFormat_RGBA8Sint] = "WGPUTextureFormat_RGBA8Sint";
    map[WGPUTextureFormat_BGRA8Unorm] = "WGPUTextureFormat_BGRA8Unorm";
    map[WGPUTextureFormat_BGRA8UnormSrgb] = "WGPUTextureFormat_BGRA8UnormSrgb";
    map[WGPUTextureFormat_RGB10A2Uint] = "WGPUTextureFormat_RGB10A2Uint";
    map[WGPUTextureFormat_RGB10A2Unorm] = "WGPUTextureFormat_RGB10A2Unorm";
    map[WGPUTextureFormat_RG11B10Ufloat] = "WGPUTextureFormat_RG11B10Ufloat";
    map[WGPUTextureFormat_RGB9E5Ufloat] = "WGPUTextureFormat_RGB9E5Ufloat";
    map[WGPUTextureFormat_RG32Float] = "WGPUTextureFormat_RG32Float";
    map[WGPUTextureFormat_RG32Uint] = "WGPUTextureFormat_RG32Uint";
    map[WGPUTextureFormat_RG32Sint] = "WGPUTextureFormat_RG32Sint";
    map[WGPUTextureFormat_RGBA16Uint] = "WGPUTextureFormat_RGBA16Uint";
    map[WGPUTextureFormat_RGBA16Sint] = "WGPUTextureFormat_RGBA16Sint";
    map[WGPUTextureFormat_RGBA16Float] = "WGPUTextureFormat_RGBA16Float";
    map[WGPUTextureFormat_RGBA32Float] = "WGPUTextureFormat_RGBA32Float";
    map[WGPUTextureFormat_RGBA32Uint] = "WGPUTextureFormat_RGBA32Uint";
    map[WGPUTextureFormat_RGBA32Sint] = "WGPUTextureFormat_RGBA32Sint";
    map[WGPUTextureFormat_Stencil8] = "WGPUTextureFormat_Stencil8";
    map[WGPUTextureFormat_Depth16Unorm] = "WGPUTextureFormat_Depth16Unorm";
    map[WGPUTextureFormat_Depth24Plus] = "WGPUTextureFormat_Depth24Plus";
    map[WGPUTextureFormat_Depth24PlusStencil8] = "WGPUTextureFormat_Depth24PlusStencil8";
    map[WGPUTextureFormat_Depth32Float] = "WGPUTextureFormat_Depth32Float";
    map[WGPUTextureFormat_Depth32FloatStencil8] = "WGPUTextureFormat_Depth32FloatStencil8";
    map[WGPUTextureFormat_BC1RGBAUnorm] = "WGPUTextureFormat_BC1RGBAUnorm";
    map[WGPUTextureFormat_BC1RGBAUnormSrgb] = "WGPUTextureFormat_BC1RGBAUnormSrgb";
    map[WGPUTextureFormat_BC2RGBAUnorm] = "WGPUTextureFormat_BC2RGBAUnorm";
    map[WGPUTextureFormat_BC2RGBAUnormSrgb] = "WGPUTextureFormat_BC2RGBAUnormSrgb";
    map[WGPUTextureFormat_BC3RGBAUnorm] = "WGPUTextureFormat_BC3RGBAUnorm";
    map[WGPUTextureFormat_BC3RGBAUnormSrgb] = "WGPUTextureFormat_BC3RGBAUnormSrgb";
    map[WGPUTextureFormat_BC4RUnorm] = "WGPUTextureFormat_BC4RUnorm";
    map[WGPUTextureFormat_BC4RSnorm] = "WGPUTextureFormat_BC4RSnorm";
    map[WGPUTextureFormat_BC5RGUnorm] = "WGPUTextureFormat_BC5RGUnorm";
    map[WGPUTextureFormat_BC5RGSnorm] = "WGPUTextureFormat_BC5RGSnorm";
    map[WGPUTextureFormat_BC6HRGBUfloat] = "WGPUTextureFormat_BC6HRGBUfloat";
    map[WGPUTextureFormat_BC6HRGBFloat] = "WGPUTextureFormat_BC6HRGBFloat";
    map[WGPUTextureFormat_BC7RGBAUnorm] = "WGPUTextureFormat_BC7RGBAUnorm";
    map[WGPUTextureFormat_BC7RGBAUnormSrgb] = "WGPUTextureFormat_BC7RGBAUnormSrgb";
    map[WGPUTextureFormat_ETC2RGB8Unorm] = "WGPUTextureFormat_ETC2RGB8Unorm";
    map[WGPUTextureFormat_ETC2RGB8UnormSrgb] = "WGPUTextureFormat_ETC2RGB8UnormSrgb";
    map[WGPUTextureFormat_ETC2RGB8A1Unorm] = "WGPUTextureFormat_ETC2RGB8A1Unorm";
    map[WGPUTextureFormat_ETC2RGB8A1UnormSrgb] = "WGPUTextureFormat_ETC2RGB8A1UnormSrgb";
    map[WGPUTextureFormat_ETC2RGBA8Unorm] = "WGPUTextureFormat_ETC2RGBA8Unorm";
    map[WGPUTextureFormat_ETC2RGBA8UnormSrgb] = "WGPUTextureFormat_ETC2RGBA8UnormSrgb";
    map[WGPUTextureFormat_EACR11Unorm] = "WGPUTextureFormat_EACR11Unorm";
    map[WGPUTextureFormat_EACR11Snorm] = "WGPUTextureFormat_EACR11Snorm";
    map[WGPUTextureFormat_EACRG11Unorm] = "WGPUTextureFormat_EACRG11Unorm";
    map[WGPUTextureFormat_EACRG11Snorm] = "WGPUTextureFormat_EACRG11Snorm";
    map[WGPUTextureFormat_ASTC4x4Unorm] = "WGPUTextureFormat_ASTC4x4Unorm";
    map[WGPUTextureFormat_ASTC4x4UnormSrgb] = "WGPUTextureFormat_ASTC4x4UnormSrgb";
    map[WGPUTextureFormat_ASTC5x4Unorm] = "WGPUTextureFormat_ASTC5x4Unorm";
    map[WGPUTextureFormat_ASTC5x4UnormSrgb] = "WGPUTextureFormat_ASTC5x4UnormSrgb";
    map[WGPUTextureFormat_ASTC5x5Unorm] = "WGPUTextureFormat_ASTC5x5Unorm";
    map[WGPUTextureFormat_ASTC5x5UnormSrgb] = "WGPUTextureFormat_ASTC5x5UnormSrgb";
    map[WGPUTextureFormat_ASTC6x5Unorm] = "WGPUTextureFormat_ASTC6x5Unorm";
    map[WGPUTextureFormat_ASTC6x5UnormSrgb] = "WGPUTextureFormat_ASTC6x5UnormSrgb";
    map[WGPUTextureFormat_ASTC6x6Unorm] = "WGPUTextureFormat_ASTC6x6Unorm";
    map[WGPUTextureFormat_ASTC6x6UnormSrgb] = "WGPUTextureFormat_ASTC6x6UnormSrgb";
    map[WGPUTextureFormat_ASTC8x5Unorm] = "WGPUTextureFormat_ASTC8x5Unorm";
    map[WGPUTextureFormat_ASTC8x5UnormSrgb] = "WGPUTextureFormat_ASTC8x5UnormSrgb";
    map[WGPUTextureFormat_ASTC8x6Unorm] = "WGPUTextureFormat_ASTC8x6Unorm";
    map[WGPUTextureFormat_ASTC8x6UnormSrgb] = "WGPUTextureFormat_ASTC8x6UnormSrgb";
    map[WGPUTextureFormat_ASTC8x8Unorm] = "WGPUTextureFormat_ASTC8x8Unorm";
    map[WGPUTextureFormat_ASTC8x8UnormSrgb] = "WGPUTextureFormat_ASTC8x8UnormSrgb";
    map[WGPUTextureFormat_ASTC10x5Unorm] = "WGPUTextureFormat_ASTC10x5Unorm";
    map[WGPUTextureFormat_ASTC10x5UnormSrgb] = "WGPUTextureFormat_ASTC10x5UnormSrgb";
    map[WGPUTextureFormat_ASTC10x6Unorm] = "WGPUTextureFormat_ASTC10x6Unorm";
    map[WGPUTextureFormat_ASTC10x6UnormSrgb] = "WGPUTextureFormat_ASTC10x6UnormSrgb";
    map[WGPUTextureFormat_ASTC10x8Unorm] = "WGPUTextureFormat_ASTC10x8Unorm";
    map[WGPUTextureFormat_ASTC10x8UnormSrgb] = "WGPUTextureFormat_ASTC10x8UnormSrgb";
    map[WGPUTextureFormat_ASTC10x10Unorm] = "WGPUTextureFormat_ASTC10x10Unorm";
    map[WGPUTextureFormat_ASTC10x10UnormSrgb] = "WGPUTextureFormat_ASTC10x10UnormSrgb";
    map[WGPUTextureFormat_ASTC12x10Unorm] = "WGPUTextureFormat_ASTC12x10Unorm";
    map[WGPUTextureFormat_ASTC12x10UnormSrgb] = "WGPUTextureFormat_ASTC12x10UnormSrgb";
    map[WGPUTextureFormat_ASTC12x12Unorm] = "WGPUTextureFormat_ASTC12x12Unorm";
    map[WGPUTextureFormat_ASTC12x12UnormSrgb] = "WGPUTextureFormat_ASTC12x12UnormSrgb";
    #ifndef __EMSCRIPTEN__ //why??
    map[WGPUTextureFormat_R16Unorm] = "WGPUTextureFormat_R16Unorm";
    map[WGPUTextureFormat_RG16Unorm] = "WGPUTextureFormat_RG16Unorm";
    map[WGPUTextureFormat_RGBA16Unorm] = "WGPUTextureFormat_RGBA16Unorm";
    map[WGPUTextureFormat_R16Snorm] = "WGPUTextureFormat_R16Snorm";
    map[WGPUTextureFormat_RG16Snorm] = "WGPUTextureFormat_RG16Snorm";
    map[WGPUTextureFormat_RGBA16Snorm] = "WGPUTextureFormat_RGBA16Snorm";
    map[WGPUTextureFormat_R8BG8Biplanar420Unorm] = "WGPUTextureFormat_R8BG8Biplanar420Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm";
    map[WGPUTextureFormat_R8BG8A8Triplanar420Unorm] = "WGPUTextureFormat_R8BG8A8Triplanar420Unorm";
    map[WGPUTextureFormat_R8BG8Biplanar422Unorm] = "WGPUTextureFormat_R8BG8Biplanar422Unorm";
    map[WGPUTextureFormat_R8BG8Biplanar444Unorm] = "WGPUTextureFormat_R8BG8Biplanar444Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm";
    map[WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm] = "WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm";
    map[WGPUTextureFormat_External] = "WGPUTextureFormat_External";
    map[WGPUTextureFormat_Force32] = "WGPUTextureFormat_Force32";
    #endif
    return map;
}();
const char* TextureFormatName(WGPUTextureFormat fmt){
    auto it = textureFormatSpellingTable.find(fmt);
    if(it == textureFormatSpellingTable.end()){
        return "?? Unknown WGPUTextureFormat value ??";
    }
    return it->second.c_str();
}
extern "C" size_t GetPixelSizeInBytes(PixelFormat format) {
    switch(format){
        case PixelFormat::BGRA8:
        case PixelFormat::RGBA8:
        return 4;
        case PixelFormat::RGBA16F:
        return 8;
        case PixelFormat::RGBA32F:
        return 16;

        case PixelFormat::GRAYSCALE:
        return 2;
        case PixelFormat::RGB8:
        return 3;
        case PixelFormat::Depth24:
        case PixelFormat::Depth32:
        return 4;
    }
    /*
    switch (format) {
        case WGPUTextureFormat_Undefined:
            return 0;

        // 8-bit formats
        case WGPUTextureFormat_R8Unorm:
        case WGPUTextureFormat_R8Snorm:
        case WGPUTextureFormat_R8Uint:
        case WGPUTextureFormat_R8Sint:
            return 1;

        // 16-bit formats
        case WGPUTextureFormat_R16Uint:
        case WGPUTextureFormat_R16Sint:
        case WGPUTextureFormat_R16Float:
        case WGPUTextureFormat_RG8Unorm:
        case WGPUTextureFormat_RG8Snorm:
        case WGPUTextureFormat_RG8Uint:
        case WGPUTextureFormat_RG8Sint:
            return 2;

        // 24-bit formats
        

        // 32-bit formats
        case WGPUTextureFormat_RGB9E5Ufloat: // Packed format, typically stored in 4 bytes
        case WGPUTextureFormat_R32Float:
        case WGPUTextureFormat_R32Uint:
        case WGPUTextureFormat_R32Sint:
        case WGPUTextureFormat_RG16Uint:
        case WGPUTextureFormat_RG16Sint:
        case WGPUTextureFormat_RG16Float:
        case WGPUTextureFormat_RGBA8Unorm:
        case WGPUTextureFormat_RGBA8UnormSrgb:
        case WGPUTextureFormat_RGBA8Snorm:
        case WGPUTextureFormat_RGBA8Uint:
        case WGPUTextureFormat_RGBA8Sint:
        case WGPUTextureFormat_BGRA8Unorm:
        case WGPUTextureFormat_BGRA8UnormSrgb:
        case WGPUTextureFormat_RGB10A2Uint:
        case WGPUTextureFormat_RGB10A2Unorm:
        case WGPUTextureFormat_RG11B10Ufloat:
            return 4;

        // 64-bit formats
        case WGPUTextureFormat_RG32Float:
        case WGPUTextureFormat_RG32Uint:
        case WGPUTextureFormat_RG32Sint:
        case WGPUTextureFormat_RGBA16Uint:
        case WGPUTextureFormat_RGBA16Sint:
        case WGPUTextureFormat_RGBA16Float:
            return 8;

        // 128-bit formats
        case WGPUTextureFormat_RGBA32Float:
        case WGPUTextureFormat_RGBA32Uint:
        case WGPUTextureFormat_RGBA32Sint:
            return 16;

        // Depth and Stencil formats
        case WGPUTextureFormat_Stencil8:
            return 1;
        case WGPUTextureFormat_Depth16Unorm:
            return 2;
        case WGPUTextureFormat_Depth24Plus:
            return 3; // Typically 24 bits for depth
        case WGPUTextureFormat_Depth24PlusStencil8:
            return 4; // 24 bits depth + 8 bits stencil
        case WGPUTextureFormat_Depth32Float:
            return 4;
        case WGPUTextureFormat_Depth32FloatStencil8:
            return 5; // 32 bits depth + 8 bits stencil

        // Compressed Formats (Sizes are per block)
        case WGPUTextureFormat_BC1RGBAUnorm:
        case WGPUTextureFormat_BC1RGBAUnormSrgb:
            return 8; // 8 bytes per 4x4 block

        case WGPUTextureFormat_BC2RGBAUnorm:
        case WGPUTextureFormat_BC2RGBAUnormSrgb:
        case WGPUTextureFormat_BC3RGBAUnorm:
        case WGPUTextureFormat_BC3RGBAUnormSrgb:
        case WGPUTextureFormat_BC7RGBAUnorm:
        case WGPUTextureFormat_BC7RGBAUnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_BC4RUnorm:
        case WGPUTextureFormat_BC4RSnorm:
            return 8; // 8 bytes per 4x4 block

        case WGPUTextureFormat_BC5RGUnorm:
        case WGPUTextureFormat_BC5RGSnorm:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_BC6HRGBUfloat:
        case WGPUTextureFormat_BC6HRGBFloat:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ETC2RGB8Unorm:
        case WGPUTextureFormat_ETC2RGB8UnormSrgb:
        case WGPUTextureFormat_ETC2RGB8A1Unorm:
        case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
        case WGPUTextureFormat_ETC2RGBA8Unorm:
        case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
            return 8; // 8 bytes per 4x4 block

        case WGPUTextureFormat_EACR11Unorm:
        case WGPUTextureFormat_EACR11Snorm:
        case WGPUTextureFormat_EACRG11Unorm:
        case WGPUTextureFormat_EACRG11Snorm:
            return 8; // 8 bytes per 4x4 block

        case WGPUTextureFormat_ASTC4x4Unorm:
        case WGPUTextureFormat_ASTC4x4UnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ASTC5x4Unorm:
        case WGPUTextureFormat_ASTC5x4UnormSrgb:
        case WGPUTextureFormat_ASTC5x5Unorm:
        case WGPUTextureFormat_ASTC5x5UnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ASTC6x5Unorm:
        case WGPUTextureFormat_ASTC6x5UnormSrgb:
        case WGPUTextureFormat_ASTC6x6Unorm:
        case WGPUTextureFormat_ASTC6x6UnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ASTC8x5Unorm:
        case WGPUTextureFormat_ASTC8x5UnormSrgb:
        case WGPUTextureFormat_ASTC8x6Unorm:
        case WGPUTextureFormat_ASTC8x6UnormSrgb:
        case WGPUTextureFormat_ASTC8x8Unorm:
        case WGPUTextureFormat_ASTC8x8UnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ASTC10x5Unorm:
        case WGPUTextureFormat_ASTC10x5UnormSrgb:
        case WGPUTextureFormat_ASTC10x6Unorm:
        case WGPUTextureFormat_ASTC10x6UnormSrgb:
        case WGPUTextureFormat_ASTC10x8Unorm:
        case WGPUTextureFormat_ASTC10x8UnormSrgb:
        case WGPUTextureFormat_ASTC10x10Unorm:
        case WGPUTextureFormat_ASTC10x10UnormSrgb:
            return 16; // 16 bytes per 4x4 block

        case WGPUTextureFormat_ASTC12x10Unorm:
        case WGPUTextureFormat_ASTC12x10UnormSrgb:
        case WGPUTextureFormat_ASTC12x12Unorm:
        case WGPUTextureFormat_ASTC12x12UnormSrgb:
            return 16; // 16 bytes per 4x4 block

#ifndef __EMSCRIPTEN__
        // Additional Formats not available in Emscripten builds

        case WGPUTextureFormat_R16Unorm:
            return 2;

        case WGPUTextureFormat_RG16Unorm:
            return 4;

        case WGPUTextureFormat_RGBA16Unorm:
            return 8;

        case WGPUTextureFormat_R16Snorm:
            return 2;

        case WGPUTextureFormat_RG16Snorm:
            return 4;

        case WGPUTextureFormat_RGBA16Snorm:
            return 8;

        case WGPUTextureFormat_R8BG8Biplanar420Unorm:
            return 2; // Typically, planar formats have separate sizes for planes

        case WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm:
            return 3; // Packed format

        case WGPUTextureFormat_R8BG8A8Triplanar420Unorm:
            return 3; // Triplanar formats have separate sizes for planes

        case WGPUTextureFormat_R8BG8Biplanar422Unorm:
            return 2;

        case WGPUTextureFormat_R8BG8Biplanar444Unorm:
            return 2;

        case WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm:
            return 3;

        case WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm:
            return 3;

        case WGPUTextureFormat_External:
            return 0; // External formats may not have a defined size

        case WGPUTextureFormat_Force32:
            return 4; // Typically used to force enum size
#endif

        default:
            // Unknown format
            return 0;
    }*/
}

extern "C" void ResizeSurface(FullSurface* fsurface, uint32_t newWidth, uint32_t newHeight){
    fsurface->surfaceConfig.width = newWidth;
    fsurface->surfaceConfig.height = newHeight;
    fsurface->renderTarget.colorMultisample.width = newWidth;
    fsurface->renderTarget.colorMultisample.height = newHeight;
    fsurface->renderTarget.texture.width = newWidth;
    fsurface->renderTarget.texture.height = newHeight;
    fsurface->renderTarget.depth.width = newWidth;
    fsurface->renderTarget.depth.height = newHeight;
    WGPUTextureFormat format = (WGPUTextureFormat)fsurface->surfaceConfig.format;
    WGPUSurfaceConfiguration wsconfig{};
    wsconfig.device = (WGPUDevice)fsurface->surfaceConfig.device;
    wsconfig.width = newWidth;
    wsconfig.height = newHeight;
    wsconfig.format = format;
    wsconfig.viewFormatCount = 1;
    wsconfig.viewFormats = &format;
    wsconfig.alphaMode = WGPUCompositeAlphaMode_Opaque;
    wsconfig.presentMode = (WGPUPresentMode)fsurface->surfaceConfig.presentMode;
    wsconfig.usage = WGPUTextureUsage_CopySrc | WGPUTextureUsage_RenderAttachment;
    wgpuSurfaceConfigure((WGPUSurface)fsurface->surface, &wsconfig);
    fsurface->surfaceConfig.width = newWidth;
    fsurface->surfaceConfig.height = newHeight;
    //UnloadTexture(fsurface->frameBuffer.texture);
    //fsurface->frameBuffer.texture = Texture zeroinit;
    UnloadTexture(fsurface->renderTarget.colorMultisample);
    UnloadTexture(fsurface->renderTarget.depth);
    if(g_renderstate.windowFlags & FLAG_MSAA_4X_HINT){
        fsurface->renderTarget.colorMultisample = LoadTexturePro(newWidth, newHeight, fsurface->surfaceConfig.format, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_CopySrc, 4, 1);
    }
    fsurface->renderTarget.depth = LoadTexturePro(newWidth,
                           newHeight, 
                           Depth32, 
                           WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                           (g_renderstate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                           1
    );
}
