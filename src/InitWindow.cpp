#include <raygpu.h>
#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include <chrono>
#include "GLFW/glfw3.h"
#ifndef __EMSCRIPTEN__
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
// Configurable scaling factors
constexpr float PIXEL_SCALE = 1.0f;
constexpr float LINE_SCALE = 20.0f;  // Adjust based on typical line height
constexpr float PAGE_SCALE = 800.0f; // Adjust based on typical page height
// Function to calculate scaling based on deltaMode
float calculateScrollScale(int deltaMode) {
    switch(deltaMode) {
        case DOM_DELTA_PIXEL:
            return PIXEL_SCALE;
        case DOM_DELTA_LINE:
            return LINE_SCALE;
        case DOM_DELTA_PAGE:
            return PAGE_SCALE;
        default:
            return PIXEL_SCALE; // Fallback to pixel scale
    }
}
#endif  // 
#ifdef _WIN32
#define __builtin_unreachable(...)
#endif
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
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
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
    return textureSample(colDiffuse, texSampler, in.uv).rgba * in.color;
}
)";
wgpu::Limits* limitsToBeRequested = nullptr;

void setlimit(wgpu::Limits& limits, LimitType limit, uint64_t value){
    switch(limit){
        case maxTextureDimension1D:limits.maxTextureDimension1D = value;break;
        case maxTextureDimension2D:limits.maxTextureDimension2D = value;break;
        case maxTextureDimension3D:limits.maxTextureDimension3D = value;break;
        case maxTextureArrayLayers:limits.maxTextureArrayLayers = value;break;
        case maxBindGroups:limits.maxBindGroups = value;break;
        case maxBindGroupsPlusVertexBuffers:limits.maxBindGroupsPlusVertexBuffers = value;break;
        case maxBindingsPerBindGroup:limits.maxBindingsPerBindGroup = value;break;
        case maxDynamicUniformBuffersPerPipelineLayout:limits.maxDynamicUniformBuffersPerPipelineLayout = value;break;
        case maxDynamicStorageBuffersPerPipelineLayout:limits.maxDynamicStorageBuffersPerPipelineLayout = value;break;
        case maxSampledTexturesPerShaderStage:limits.maxSampledTexturesPerShaderStage = value;break;
        case maxSamplersPerShaderStage:limits.maxSamplersPerShaderStage = value;break;
        case maxStorageBuffersPerShaderStage:limits.maxStorageBuffersPerShaderStage = value;break;
        case maxStorageTexturesPerShaderStage:limits.maxStorageTexturesPerShaderStage = value;break;
        case maxUniformBuffersPerShaderStage:limits.maxUniformBuffersPerShaderStage = value;break;
        case maxUniformBufferBindingSize:limits.maxUniformBufferBindingSize = value;break;
        case maxStorageBufferBindingSize:limits.maxStorageBufferBindingSize = value;break;
        case minUniformBufferOffsetAlignment:limits.minUniformBufferOffsetAlignment = value;break;
        case minStorageBufferOffsetAlignment:limits.minStorageBufferOffsetAlignment = value;break;
        case maxVertexBuffers:limits.maxVertexBuffers = value;break;
        case maxBufferSize:limits.maxBufferSize = value;break;
        case maxVertexAttributes:limits.maxVertexAttributes = value;break;
        case maxVertexBufferArrayStride:limits.maxVertexBufferArrayStride = value;break;
        case maxInterStageShaderComponents:limits.maxInterStageShaderComponents = value;break;
        case maxInterStageShaderVariables:limits.maxInterStageShaderVariables = value;break;
        case maxColorAttachments:limits.maxColorAttachments = value;break;
        case maxColorAttachmentBytesPerSample:limits.maxColorAttachmentBytesPerSample = value;break;
        case maxComputeWorkgroupStorageSize:limits.maxComputeWorkgroupStorageSize = value;break;
        case maxComputeInvocationsPerWorkgroup:limits.maxComputeInvocationsPerWorkgroup = value;break;
        case maxComputeWorkgroupSizeX:limits.maxComputeWorkgroupSizeX = value;break;
        case maxComputeWorkgroupSizeY:limits.maxComputeWorkgroupSizeY = value;break;
        case maxComputeWorkgroupSizeZ:limits.maxComputeWorkgroupSizeZ = value;break;
        case maxComputeWorkgroupsPerDimension:limits.maxComputeWorkgroupsPerDimension = value;break;
        case maxStorageBuffersInVertexStage:limits.maxStorageBuffersInVertexStage = value;break;
        case maxStorageTexturesInVertexStage:limits.maxStorageTexturesInVertexStage = value;break;
        case maxStorageBuffersInFragmentStage:limits.maxStorageBuffersInFragmentStage = value;break;
        case maxStorageTexturesInFragmentStage:limits.maxStorageTexturesInFragmentStage = value;break;
    }
}
extern "C" void RequestLimit(LimitType limit, uint64_t value){
    if(limitsToBeRequested == nullptr){
        limitsToBeRequested = new wgpu::Limits;
    }
    setlimit(*limitsToBeRequested, limit, value);
}

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
extern const std::unordered_map<std::string, int> emscriptenToGLFWKeyMap;
void glfwKeyCallback (GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        g_wgpustate.input_map[window].keydown[key] = 1;
    }else if(action == GLFW_RELEASE){
        g_wgpustate.input_map[window].keydown[key] = 0;
    }
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        EndGIFRecording();
        glfwSetWindowShouldClose(window, true);
    }
}
#ifdef __EMSCRIPTEN__
EM_BOOL EmscriptenKeydownCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData){
    if(keyEvent->repeat)return 0;
    uint32_t modifier = 0;
    if(keyEvent->ctrlKey)
        modifier |= GLFW_MOD_CONTROL;
    if(keyEvent->shiftKey)
        modifier |= GLFW_MOD_SHIFT;
    if(keyEvent->altKey)
        modifier |= GLFW_MOD_ALT;
    glfwKeyCallback(g_wgpustate.window, emscriptenToGLFWKeyMap.at(keyEvent->code), emscriptenToGLFWKeyMap.at(keyEvent->code), GLFW_PRESS, modifier);
    //__builtin_dump_struct(keyEvent, printf);
    //printf("Pressed %u\n", keyEvent->which);
    return 0;
}
EM_BOOL EmscriptenKeyupCallback(int eventType, const EmscriptenKeyboardEvent *keyEvent, void *userData){
    if(keyEvent->repeat)return 1;
    uint32_t modifier = 0;
    if(keyEvent->ctrlKey)
        modifier |= GLFW_MOD_CONTROL;
    if(keyEvent->shiftKey)
        modifier |= GLFW_MOD_SHIFT;
    if(keyEvent->altKey)
        modifier |= GLFW_MOD_ALT;
    glfwKeyCallback(g_wgpustate.window, emscriptenToGLFWKeyMap.at(keyEvent->code), emscriptenToGLFWKeyMap.at(keyEvent->code), GLFW_RELEASE, modifier);
    //printf("Released %u\n", keyEvent->which);
    return 1;
}
#endif// __EMSCRIPTEN__



void InitWGPU(webgpu_cxx_state* sample){
    
    // Create the toggles descriptor if not using emscripten.
    wgpu::ChainedStruct* togglesChain = nullptr;
    wgpu::SType type;
#ifndef __EMSCRIPTEN__
    std::vector<const char*> enableToggleNames{};
    std::vector<const char*> disabledToggleNames{};

    wgpu::DawnTogglesDescriptor toggles = {};
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    togglesChain = &toggles;
#endif  // __EMSCRIPTEN__

    // Setup base adapter options with toggles.
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = togglesChain;
    auto backendType = wgpu::BackendType::Vulkan;
    auto adapterType = wgpu::AdapterType::Unknown;
    adapterOptions.backendType = backendType;
    if (backendType != wgpu::BackendType::Undefined) {
        auto bcompat = [](wgpu::BackendType backend) {
            switch (backend) {
                case wgpu::BackendType::D3D12:
                case wgpu::BackendType::Metal:
                case wgpu::BackendType::Vulkan:
                case wgpu::BackendType::WebGPU:
                case wgpu::BackendType::Null:
                    return false;
                case wgpu::BackendType::D3D11:
                case wgpu::BackendType::OpenGL:
                case wgpu::BackendType::OpenGLES:
                    return true;
                case wgpu::BackendType::Undefined:
                default:
                    __builtin_unreachable();
                    //return false;
            }
        };
        adapterOptions.featureLevel = wgpu::FeatureLevel::Undefined;

    }

    switch (adapterType) {
        case wgpu::AdapterType::CPU:
            adapterOptions.forceFallbackAdapter = true;
            break;
        case wgpu::AdapterType::DiscreteGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::IntegratedGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::LowPower;
            break;
        case wgpu::AdapterType::Unknown:
            break;
    }

#ifndef __EMSCRIPTEN__
    //dawnProcSetProcs(&dawn::native::GetProcs());

    // Create the instance with the toggles
    wgpu::InstanceDescriptor instanceDescriptor = {};
    instanceDescriptor.nextInChain = togglesChain;
    instanceDescriptor.features.timedWaitAnyEnable = true;
    
    sample->instance = wgpu::CreateInstance(&instanceDescriptor);
#else
    // Create the instance
    sample->instance = wgpu::CreateInstance(nullptr);
#endif  // __EMSCRIPTEN__

    //wgpu::WGSLFeatureName wgslfeatures[8];
    //sample->instance.EnumerateWGSLLanguageFeatures(wgslfeatures);
    //std::cout << sample->instance.HasWGSLLanguageFeature(wgpu::WGSLFeatureName::ReadonlyAndReadwriteStorageTextures);
    //exit(0);
    // Synchronously create the adapter
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    char tmp[2048] = {};
                    std::memcpy(tmp, message.data, std::min(message.length, (size_t)2047));
                    TRACELOG(LOG_FATAL, "Failed to get an adapter: %s\n", tmp);
                    return;
                }
                sample->adapter = std::move(adapter);
            }),
        UINT64_MAX);
    if (sample->adapter == nullptr) {
        std::cerr << "Adapter is null\n";
        abort();
    }
    wgpu::AdapterInfo info;
    sample->adapter.GetInfo(&info);
    std::vector<char> deviceNameCstr(info.device.length + 1, '\0');
    std::vector<char> deviceDescCstr(info.description.length + 1, '\0');
    memcpy(deviceNameCstr.data(), info.device.data, info.device.length);
    memcpy(deviceDescCstr.data(), info.description.data, info.description.length);
    TRACELOG(LOG_INFO, "Using adapter %s", deviceNameCstr.data());
    TRACELOG(LOG_INFO, "Description %s", deviceDescCstr.data());

    // Create device descriptor with callbacks and toggles
    wgpu::SupportedFeatures features;
    sample->adapter.GetFeatures(&features);
    for(size_t i = 0;i < features.featureCount;i++){
        const wgpu::FeatureName& fn = features.features[i];
        //TRACELOG(LOG_INFO, "Supports: %d", fn);
    }
    wgpu::DeviceDescriptor deviceDesc = {};
    #ifndef __EMSCRIPTEN__ //y tho
    wgpu::FeatureName fname = wgpu::FeatureName::ClipDistances;
    deviceDesc.requiredFeatures = &fname;
    deviceDesc.requiredFeatureCount = 1;
    #endif
    deviceDesc.nextInChain = togglesChain;
    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            const char* reasonName = "";
            switch (reason) {
                case wgpu::DeviceLostReason::Unknown:
                    reasonName = "Unknown";
                    break;
                case wgpu::DeviceLostReason::Destroyed:
                    reasonName = "Destroyed";
                    break;
                case wgpu::DeviceLostReason::InstanceDropped:
                    reasonName = "InstanceDropped";
                    break;
                case wgpu::DeviceLostReason::FailedCreation:
                    reasonName = "FailedCreation";
                    break;
                default:
                    __builtin_unreachable();
            }
            std::cerr << "Device lost because of " << reasonName << ": " << std::string(message.data, message.length) << std::endl;
        });
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
            const char* errorTypeName = "";
            switch (type) {
                case wgpu::ErrorType::Validation:
                    errorTypeName = "Validation";
                    break;
                case wgpu::ErrorType::OutOfMemory:
                    errorTypeName = "Out of memory";
                    break;
                case wgpu::ErrorType::Unknown:
                    errorTypeName = "Unknown";
                    break;
                case wgpu::ErrorType::DeviceLost:
                    errorTypeName = "Device lost";
                    break;
                default:
                    __builtin_unreachable();
            }
            std::cerr << errorTypeName << " error: " << std::string(message.data, message.length);
        });

    // Synchronously create the device
    wgpu::RequiredLimits reqLimits;
    if(limitsToBeRequested){
        reqLimits.limits = *limitsToBeRequested;
        deviceDesc.requiredLimits = &reqLimits;
    }
    else{
        deviceDesc.requiredLimits = nullptr;
    }
    sample->instance.WaitAny(
        sample->adapter.RequestDevice(
            &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                if (status != wgpu::RequestDeviceStatus::Success) {
                    std::cerr << "Failed to get an device: " << std::string(message.data, message.length);
                    return;
                }
                sample->device = std::move(device);
                sample->queue = sample->device.GetQueue();
            }),

        UINT64_MAX);
    if (sample->device == nullptr) {
        TRACELOG(LOG_FATAL, "Device is null");
    }
    wgpu::SupportedLimits slimits;
    
    sample->device.GetLimits(&slimits);
    TraceLog(LOG_INFO, "Supports %u bindings per bindgroup", (unsigned)slimits.limits.maxBindingsPerBindGroup);
    TraceLog(LOG_INFO, "Supports %u bindgroups", (unsigned)slimits.limits.maxBindGroups);
    TraceLog(LOG_INFO, "Supports buffers up to %llu megabytes", (unsigned long long)slimits.limits.maxBufferSize / (1000000ull));
    TraceLog(LOG_INFO, "Supports textures up to %u x %u", (unsigned)slimits.limits.maxTextureDimension2D, (unsigned)slimits.limits.maxTextureDimension2D);
    TraceLog(LOG_INFO, "Supports %u VBO slots", (unsigned)slimits.limits.maxVertexBuffers);

}

GLFWwindow* InitWindow(uint32_t width, uint32_t height, const char* title){
    if(g_wgpustate.windowFlags & FLAG_STDOUT_TO_FFMPEG){
        if(IsATerminal(stdout)){
            TRACELOG(LOG_ERROR, "Refusing to pipe video output to terminal");
            TRACELOG(LOG_ERROR, "Try <program> | ffmpeg -y -f rawvideo -pix_fmt rgba -s %ux%u -r 60 -i - -vf format=yuv420p out.mp4", width, height);
            exit(1);
        }
        SetTraceLogFile(stderr);
    }

    webgpu_cxx_state wg_init;
    InitWGPU(&wg_init);
    g_wgpustate.instance = std::move(wg_init.instance);
    g_wgpustate.adapter  = std::move(wg_init.adapter );
    g_wgpustate.device   = std::move(wg_init.device  );
    g_wgpustate.queue    = std::move(wg_init.queue   );
    g_wgpustate.width = width;
    g_wgpustate.height = height;

    g_wgpustate.whitePixel = LoadTextureFromImage(GenImageChecker(Color{255,255,255,255}, Color{255,255,255,255}, 1, 1, 0));
    TraceLog(LOG_INFO, "Loaded whitepixel texture");

    WGPUShaderModule tShader = LoadShaderFromMemory(shaderSource);
    g_wgpustate.rstate = new full_renderstate;
    
    Matrix identity = MatrixIdentity();
    g_wgpustate.identityMatrix = GenStorageBuffer(&identity, sizeof(Matrix));

    g_wgpustate.grst = (GIFRecordState*)calloc(1, 160);



    #ifdef __EMSCRIPTEN__
    //EMSCRIPTEN_RESULT ret = emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, EmscriptenKeyCallback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, EmscriptenKeydownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 1, EmscriptenKeyupCallback);
    //if(ret == EMSCRIPTEN_RESULT_SUCCESS)
    //    TRACELOG(LOG_INFO, "Keypress successfully registered");
    #endif

    GLFWwindow* window = nullptr;
    if(!(g_wgpustate.windowFlags & FLAG_HEADLESS)){
        if (!glfwInit()) {
            abort();
        }
        GLFWmonitor* mon = nullptr;

        glfwSetErrorCallback([](int code, const char* message) {
            std::cerr << "GLFW error: " << code << " - " << message;
        });


    #ifndef __EMSCRIPTEN__


        // Create the test window with no client API.
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, (g_wgpustate.windowFlags & FLAG_WINDOW_RESIZABLE ) ? GLFW_TRUE : GLFW_FALSE);
        //glfwWindowHint(GLFW_REFRESH_RATE, 144);

        if(g_wgpustate.windowFlags & FLAG_FULLSCREEN_MODE){
            mon = glfwGetPrimaryMonitor();
            //std::cout <<glfwGetVideoMode(mon)->refreshRate << std::endl;
            //abort();
        }
    #endif
        
    #ifndef __EMSCRIPTEN__
        window = glfwCreateWindow(width, height, title, mon, nullptr);
        //glfwSetWindowPos(window, 200, 1200);
        if (!window) {
            abort();
        }
        g_wgpustate.window = window;

        // Create the surface.
        g_wgpustate.surface = wgpu::glfw::CreateSurfaceForWindow(GetInstance(), window);
    #else
        // Create the surface.
        wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
        canvasDesc.selector = "#canvas";

        wgpu::SurfaceDescriptor surfaceDesc = {};
        surfaceDesc.nextInChain = &canvasDesc;
        g_wgpustate.surface = g_wgpustate.instance.CreateSurface(&surfaceDesc);
        window = glfwCreateWindow(width, height, title, mon, nullptr);
        g_wgpustate.window = window;
    #endif
        g_wgpustate.input_map[window] = window_input_state{};
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height){
            //wgpuSurfaceRelease(g_wgpustate.surface);
            //g_wgpustate.surface = wgpu::glfw::CreateSurfaceForWindow(g_wgpustate.instance, window).MoveToCHandle();
            //while(!g_wgpustate.drawmutex.try_lock());
            g_wgpustate.drawmutex.lock();
            TraceLog(LOG_DEBUG, "glfwSizeCallback called with %d x %d", width, height);
            wgpu::SurfaceCapabilities capabilities;
            g_wgpustate.surface.GetCapabilities(g_wgpustate.adapter, &capabilities);
            wgpu::SurfaceConfiguration config = {};
            config.alphaMode = wgpu::CompositeAlphaMode::Opaque;
            config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
            config.device = g_wgpustate.device;
            config.format = (wgpu::TextureFormat)g_wgpustate.frameBufferFormat;
            #ifdef __EMSCRIPTEN__
            config.presentMode = (wgpu::PresentMode)WGPUPresentMode_Fifo;
            #else
            config.presentMode = (wgpu::PresentMode)(!!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) ? WGPUPresentMode_Fifo : WGPUPresentMode_Immediate);
            #endif
            config.width = width;
            config.height = height;
            g_wgpustate.width = width;
            g_wgpustate.height = height;

            UnloadTexture(g_wgpustate.currentDefaultRenderTarget.colorMultisample);
            g_wgpustate.currentDefaultRenderTarget.colorMultisample = LoadTexturePro(width, height, g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 4, 1);
            UnloadTexture(g_wgpustate.currentDefaultRenderTarget.depth);
            g_wgpustate.currentDefaultRenderTarget.depth = LoadTexturePro(width,
                                      height, 
                                      WGPUTextureFormat_Depth24Plus, 
                                      WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                                      (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                                      1
            );
            g_wgpustate.surface.Configure(&config);
            Matrix newcamera = ScreenMatrix(width, height);
            BufferData(g_wgpustate.defaultScreenMatrix, &newcamera, sizeof(Matrix));

            setTargetTextures(g_wgpustate.rstate, g_wgpustate.rstate->color, g_wgpustate.currentDefaultRenderTarget.colorMultisample.view, g_wgpustate.currentDefaultRenderTarget.depth.view);
            //updateRenderPassDesc(g_wgpustate.rstate);

            //TODO wtf is this?
            g_wgpustate.rstate->renderpass.dsa->view = g_wgpustate.currentDefaultRenderTarget.depth.view;
            g_wgpustate.drawmutex.unlock();
        });

        //#ifndef __EMSCRIPTEN__
        wgpu::SurfaceCapabilities capabilities;
        GetCXXSurface().GetCapabilities(GetCXXAdapter(), &capabilities);
        wgpu::SurfaceConfiguration config = {};
        config.device = GetCXXDevice();
        config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
        //std::cout << "Supported format count: " << capabilities.formatCount << "\n";
        wgpu::TextureFormat selectedFormat = wgpu::TextureFormat::Undefined;
        int format_index = 0;
        for(format_index = 0;format_index < capabilities.formatCount;format_index++){
            if(capabilities.formats[format_index] == wgpu::TextureFormat::BGRA8Unorm ||
               capabilities.formats[format_index] == wgpu::TextureFormat::RGBA8Unorm){
                selectedFormat = (capabilities.formats[format_index]);
                break;
            }
        }
        if(format_index == capabilities.formatCount){
            TRACELOG(LOG_WARNING, "No RGBA8 / BGRA8 Unorm framebuffer format found, colors might be off"); 
            config.format = capabilities.formats[0];
        }
        else{
            config.format = selectedFormat;
        }
        TRACELOG(LOG_INFO, "Selected surface format %s", textureFormatSpellingTable.at((WGPUTextureFormat)config.format).c_str());
        #ifdef __EMSCRIPTEN__
        config.presentMode = wgpu::PresentMode::Fifo;
        #else
        config.presentMode = !!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) ? wgpu::PresentMode::Fifo : wgpu::PresentMode::Immediate;
        #endif
        config.width = width;
        config.height = height;
        GetCXXSurface().Configure(&config);
        g_wgpustate.frameBufferFormat = (WGPUTextureFormat)config.format;
        //std::cout << "Supported Framebuffer Format: 0x" << std::hex << (WGPUTextureFormat)config.format << std::dec << "\n";



        auto scrollCallback = [](GLFWwindow* window, double xoffset, double yoffset){
            g_wgpustate.input_map[window].scrollThisFrame.x += xoffset;
            g_wgpustate.input_map[window].scrollThisFrame.y += yoffset;
        };
        #ifndef __EMSCRIPTEN__
        glfwSetScrollCallback(window, scrollCallback);
        #else
        



    auto EmscriptenWheelCallback = [](int eventType, const EmscriptenWheelEvent* wheelEvent, void *userData) -> EM_BOOL {
        // Calculate scaling based on deltaMode
        float scaleX = calculateScrollScale(wheelEvent->deltaMode);
        float scaleY = calculateScrollScale(wheelEvent->deltaMode);
        
        // Optionally clamp the delta values to prevent excessive scrolling
        double deltaX = std::clamp(wheelEvent->deltaX * scaleX, -100.0, 100.0) / 100.0f;
        double deltaY = std::clamp(wheelEvent->deltaY * scaleY, -100.0, 100.0) / 100.0f;
        
        std::cout << "wheel: deltaX = " << deltaX << ", deltaY = " << deltaY << std::endl;
        
        // Invoke the original scroll callback with scaled deltas
        auto originalCallback = reinterpret_cast<decltype(scrollCallback)*>(userData);
        (*originalCallback)(nullptr, deltaX, deltaY);
        
        return EM_TRUE; // Indicate that the event was handled
    };
        emscripten_set_wheel_callback("#canvas", &scrollCallback, 1, EmscriptenWheelCallback);
        #endif

        auto cpcallback = [](GLFWwindow* window, double x, double y){
            g_wgpustate.input_map[window].mousePos = Vector2{float(x), float(y)};
        };
        #ifndef __EMSCRIPTEN__
        glfwSetCursorPosCallback(window, cpcallback);
        #else
        auto EmscriptenMouseCallback = [](int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
            (*((decltype(cpcallback)*)userData))(nullptr, mouseEvent->targetX, mouseEvent->targetY);
            return true;
        };


        emscripten_set_mousemove_callback("#canvas", &cpcallback, 1, EmscriptenMouseCallback);
        #endif // __EMSCRIPTEN__
        auto clickcallback = [](GLFWwindow* window, int button, int action, int mods){
            if(action == GLFW_PRESS){
                g_wgpustate.input_map[window].mouseButtonDown[button] = 1;
            }
            else if(action == GLFW_RELEASE){
                g_wgpustate.input_map[window].mouseButtonDown[button] = 0;
            }
        };
        #ifdef __EMSCRIPTEN__
        auto EmscriptenMousedownClickCallback = [](int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
            (*((decltype(clickcallback)*)userData))(nullptr, mouseEvent->button, GLFW_PRESS, 0);
            return true;
        };
        auto EmscriptenMouseupClickCallback = [](int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
            (*((decltype(clickcallback)*)userData))(nullptr, mouseEvent->button, GLFW_RELEASE, 0);
            return true;
        };
        emscripten_set_mousedown_callback("#canvas", &clickcallback, 1, EmscriptenMousedownClickCallback);
        emscripten_set_mouseup_callback("#canvas", &clickcallback, 1, EmscriptenMouseupClickCallback);
        #else
        glfwSetMouseButtonCallback(window, clickcallback);
        #endif

        
        #ifndef __EMSCRIPTEN__
        glfwSetKeyCallback(
            g_wgpustate.window, 
            glfwKeyCallback
        );
        #else

        #endif
        #ifndef __EMSCRIPTEN__
        glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codePoint){
            g_wgpustate.charQueue.push_back((int)codePoint);
        });
        glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered){
            g_wgpustate.input_map[window].cursorInWindow = entered;
        });
        #endif
    }else{
        g_wgpustate.frameBufferFormat = WGPUTextureFormat_BGRA8Unorm;
    }
    


    UniformDescriptor uniforms[4] = {
        UniformDescriptor{uniform_buffer, 64, 0, readonly, format_or_sample_type(0)},
        UniformDescriptor{texture2d, 0, 1      , readonly, format_or_sample_type(0)},
        UniformDescriptor{sampler, 0, 2        , readonly, format_or_sample_type(0)},
        UniformDescriptor{storage_buffer, 64, 3, readonly, format_or_sample_type(0)}
    };

    AttributeAndResidence attrs[4] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 0 * sizeof(float), 0}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 3 * sizeof(float), 1}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 5 * sizeof(float), 2}, 0, WGPUVertexStepMode_Vertex, true},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x4, 8 * sizeof(float), 3}, 0, WGPUVertexStepMode_Vertex, true},
    };
    
    //arraySetter(shaderInputs.per_vertex_sizes, {3,2,4});
    //arraySetter(shaderInputs.uniform_minsizes, {64, 0, 0, 0});
    //uarraySetter(shaderInputs.uniform_types, {uniform_buffer, texture2d, sampler, storage_buffer});
    
    auto colorTexture = LoadTextureEx(width, height, g_wgpustate.frameBufferFormat, true);
    g_wgpustate.mainWindowRenderTarget.colorMultisample = LoadTexturePro(width, height, g_wgpustate.frameBufferFormat, WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 4, 1);
    g_wgpustate.mainWindowRenderTarget.depth = LoadTexturePro(width,
                                  height, 
                                  WGPUTextureFormat_Depth24Plus, 
                                  WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                                  (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1,
                                  1
    );
    init_full_renderstate(g_wgpustate.rstate, shaderSource, attrs, 4, uniforms, sizeof(uniforms) / sizeof(UniformDescriptor), colorTexture.view, g_wgpustate.currentDefaultRenderTarget.depth.view);
    g_wgpustate.rstate->renderExtentX = width;
    g_wgpustate.rstate->renderExtentY = height;
    if(!(g_wgpustate.windowFlags & FLAG_HEADLESS)){
        int wposx, wposy;
        glfwGetWindowPos(g_wgpustate.window, &wposx, &wposy);
        g_wgpustate.input_map[window].windowPosition = Rectangle{
            (float)wposx,
            (float)wposy,
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        };
    }

    LoadFontDefault();
    for(size_t i = 0;i < 512;i++){
        g_wgpustate.smallBufferPool.push_back(GenBuffer(nullptr, sizeof(vertex) * 32));
    }
    WGPUCommandEncoderDescriptor cedesc{};
    cedesc.label = STRVIEW("Global Command Encoder");
    g_wgpustate.rstate->renderpass.cmdEncoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device.Get(), &cedesc);
    Matrix m = ScreenMatrix(width, height);
    static_assert(sizeof(Matrix) == 64, "non 4 byte floats? or what");
    g_wgpustate.defaultScreenMatrix = GenUniformBuffer(&m, sizeof(Matrix));
    SetUniformBuffer(0, g_wgpustate.defaultScreenMatrix);
    SetTexture(1, g_wgpustate.whitePixel);
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


    DescribedSampler sampler = LoadSampler(repeat, linear);
    g_wgpustate.defaultSampler = sampler;
    SetSampler(2, sampler);
    g_wgpustate.init_timestamp = NanoTime();
    #ifndef __EMSCRIPTEN__
    if(!(g_wgpustate.windowFlags & FLAG_HEADLESS) && (g_wgpustate.windowFlags & FLAG_VSYNC_HINT)){
        auto rate = glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate;
        SetTargetFPS(rate);
    }
    else
        SetTargetFPS(60);
    #endif
    #ifndef __EMSCRIPTEN__
    return window;
    #else
    return window;
    #endif
}
uint32_t GetMonitorWidth (cwoid){
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        glfwInit();
        return GetMonitorWidth();
    }
    return mode->width;
}
uint32_t GetMonitorHeight(cwoid){
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(mode == nullptr){
        glfwInit();
        return GetMonitorWidth();
    }
    return mode->height;
}
void ShowCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void HideCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}
bool IsCursorHidden(cwoid){
    return glfwGetInputMode(g_wgpustate.window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN;
}
void EnableCursor(cwoid){
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void DisableCursor(cwoid){
    #ifndef __EMSCRIPTEN__
    glfwSetInputMode(g_wgpustate.window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    #endif
}
void PollEvents(){
    glfwPollEvents();
}
bool WindowShouldClose(cwoid){
    return glfwWindowShouldClose(g_wgpustate.window);
}
void* GetActiveWindowHandle(){
    if(g_wgpustate.activeSubWindow.handle)return g_wgpustate.activeSubWindow.handle;
    return g_wgpustate.window;
}
void ToggleFullscreen(){
    #ifdef __EMSCRIPTEN__
    //platform.ourFullscreen = true;

    bool enterFullscreen = false;

    const bool wasFullscreen = EM_ASM_INT( { if (document.fullscreenElement) return 1; }, 0);
    if (wasFullscreen)
    {
        if (g_wgpustate.windowFlags & FLAG_FULLSCREEN_MODE) enterFullscreen = false;
        //else if (CORE.Window.flags & FLAG_BORDERLESS_WINDOWED_MODE) enterFullscreen = true;
        else
        {
            const int canvasWidth = EM_ASM_INT( { return document.getElementById('canvas').width; }, 0);
            const int canvasStyleWidth = EM_ASM_INT( { return parseInt(document.getElementById('canvas').style.width); }, 0);
            if (canvasStyleWidth > canvasWidth) enterFullscreen = false;
            else enterFullscreen = true;
        }

        EM_ASM(document.exitFullscreen(););

        //CORE.Window.fullscreen = false;
        g_wgpustate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        //CORE.Window.flags &= ~FLAG_BORDERLESS_WINDOWED_MODE;
    }
    else enterFullscreen = true;

    if (enterFullscreen){
        EM_ASM(
            setTimeout(function()
            {
                Module.requestFullscreen(false, false);
            }, 100);
        );
        g_wgpustate.windowFlags |= FLAG_FULLSCREEN_MODE;
    }
    TRACELOG(LOG_DEBUG, "Tagu fullscreen");
    #else //Other than emscripten
    GLFWmonitor* monitor = glfwGetWindowMonitor(g_wgpustate.window);
    if(monitor){
        //We need to exit fullscreen
        g_wgpustate.windowFlags &= ~FLAG_FULLSCREEN_MODE;
        glfwSetWindowMonitor(g_wgpustate.window, NULL, g_wgpustate.input_map[g_wgpustate.window].windowPosition.x, g_wgpustate.input_map[g_wgpustate.window].windowPosition.y, g_wgpustate.input_map[g_wgpustate.window].windowPosition.width, g_wgpustate.input_map[g_wgpustate.window].windowPosition.height, GLFW_DONT_CARE);
    }
    else{
        //We need to enter fullscreen
        int xpos, ypos;
        int xs, ys;
        glfwGetWindowPos(g_wgpustate.window, &xpos, &ypos);
        glfwGetWindowSize(g_wgpustate.window, &xs, &ys);
        g_wgpustate.input_map[g_wgpustate.window].windowPosition = Rectangle{float(xpos), float(ypos), float(xs), float(ys)};
        int monitorCount = 0;
        int monitorIndex = GetCurrentMonitor();
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        // Use current monitor, so we correctly get the display the window is on
        GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;
        auto vm = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(g_wgpustate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    }
    
    
    #endif
    //wgpuTextureViewRelease(depthTexture.view);
    //wgpuTextureRelease(depthTexture.tex);
    //auto vm = glfwGetVideoMode(glfwGetPrimaryMonitor());
    //glfwSetWindowMonitor(g_wgpustate.window, glfwGetPrimaryMonitor(), 0, 0, vm->width, vm->height, vm->refreshRate);
    //depthTexture = LoadDepthTexture(1920, 1200);
}
void SetWindowShouldClose(cwoid){
    glfwSetWindowShouldClose(g_wgpustate.window, GLFW_TRUE);
}
int GetCurrentMonitor(){
    int index = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount >= 1)
    {
        if (glfwGetWindowMonitor(g_wgpustate.window) != nullptr){
            // Get the handle of the monitor that the specified window is in full screen on
            monitor = glfwGetWindowMonitor(g_wgpustate.window);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    index = i;
                    break;
                }
            }
        }
        else
        {
            // In case the window is between two monitors, we use below logic
            // to try to detect the "current monitor" for that window, note that
            // this is probably an overengineered solution for a very side case
            // trying to match SDL behaviour

            int closestDist = 0x7FFFFFFF;

            // Window center position
            int wcx = 0;
            int wcy = 0;

            glfwGetWindowPos(g_wgpustate.window, &wcx, &wcy);
            wcx += (int)GetScreenWidth()/2;
            wcy += (int)GetScreenHeight()/2;

            for (int i = 0; i < monitorCount; i++)
            {
                // Monitor top-left position
                int mx = 0;
                int my = 0;

                monitor = monitors[i];
                glfwGetMonitorPos(monitor, &mx, &my);
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                if (mode)
                {
                    const int right = mx + mode->width - 1;
                    const int bottom = my + mode->height - 1;

                    if ((wcx >= mx) &&
                        (wcx <= right) &&
                        (wcy >= my) &&
                        (wcy <= bottom))
                    {
                        index = i;
                        break;
                    }

                    int xclosest = wcx;
                    if (wcx < mx) xclosest = mx;
                    else if (wcx > right) xclosest = right;

                    int yclosest = wcy;
                    if (wcy < my) yclosest = my;
                    else if (wcy > bottom) yclosest = bottom;

                    int dx = wcx - xclosest;
                    int dy = wcy - yclosest;
                    int dist = (dx*dx) + (dy*dy);
                    if (dist < closestDist)
                    {
                        index = i;
                        closestDist = dist;
                    }
                }
                else TRACELOG(LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
            }
        }
    }

    return index;
}

extern "C" SubWindow OpenSubWindow(uint32_t width, uint32_t height, const char* title){
    SubWindow ret{};
    #ifndef __EMSCRIPTEN__
    ret.handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
    wgpu::Surface secondSurface = wgpu::glfw::CreateSurfaceForWindow(GetInstance(), (GLFWwindow*)ret.handle);
    wgpu::SurfaceCapabilities capabilities;
    secondSurface.GetCapabilities(GetCXXAdapter(), &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = GetCXXDevice();
    config.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    config.format = (wgpu::TextureFormat)g_wgpustate.frameBufferFormat;
    config.presentMode = wgpu::PresentMode::Immediate;
    config.width = width;
    config.height = height;
    secondSurface.Configure(&config);
    ret.surface = secondSurface.MoveToCHandle();
    ret.frameBuffer = LoadRenderTexture(config.width, config.height);
    #endif
    return ret;
}

extern "C" void CloseSubWindow(SubWindow subWindow){
    glfwWindowShouldClose((GLFWwindow*)subWindow.handle);
}


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
extern "C" size_t GetPixelSizeInBytes(WGPUTextureFormat format) {
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
        case WGPUTextureFormat_RGB9E5Ufloat:
            return 4; // Packed format, typically stored in 4 bytes

        // 32-bit formats
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
    }
}
const std::unordered_map<std::string, int> emscriptenToGLFWKeyMap = {
    // Alphabet Keys
    {"KeyA", GLFW_KEY_A},
    {"KeyB", GLFW_KEY_B},
    {"KeyC", GLFW_KEY_C},
    {"KeyD", GLFW_KEY_D},
    {"KeyE", GLFW_KEY_E},
    {"KeyF", GLFW_KEY_F},
    {"KeyG", GLFW_KEY_G},
    {"KeyH", GLFW_KEY_H},
    {"KeyI", GLFW_KEY_I},
    {"KeyJ", GLFW_KEY_J},
    {"KeyK", GLFW_KEY_K},
    {"KeyL", GLFW_KEY_L},
    {"KeyM", GLFW_KEY_M},
    {"KeyN", GLFW_KEY_N},
    {"KeyO", GLFW_KEY_O},
    {"KeyP", GLFW_KEY_P},
    {"KeyQ", GLFW_KEY_Q},
    {"KeyR", GLFW_KEY_R},
    {"KeyS", GLFW_KEY_S},
    {"KeyT", GLFW_KEY_T},
    {"KeyU", GLFW_KEY_U},
    {"KeyV", GLFW_KEY_V},
    {"KeyW", GLFW_KEY_W},
    {"KeyX", GLFW_KEY_X},
    {"KeyY", GLFW_KEY_Y},
    {"KeyZ", GLFW_KEY_Z},

    // Number Keys
    {"Digit0", GLFW_KEY_0},
    {"Digit1", GLFW_KEY_1},
    {"Digit2", GLFW_KEY_2},
    {"Digit3", GLFW_KEY_3},
    {"Digit4", GLFW_KEY_4},
    {"Digit5", GLFW_KEY_5},
    {"Digit6", GLFW_KEY_6},
    {"Digit7", GLFW_KEY_7},
    {"Digit8", GLFW_KEY_8},
    {"Digit9", GLFW_KEY_9},

    // Function Keys
    {"F1", GLFW_KEY_F1},
    {"F2", GLFW_KEY_F2},
    {"F3", GLFW_KEY_F3},
    {"F4", GLFW_KEY_F4},
    {"F5", GLFW_KEY_F5},
    {"F6", GLFW_KEY_F6},
    {"F7", GLFW_KEY_F7},
    {"F8", GLFW_KEY_F8},
    {"F9", GLFW_KEY_F9},
    {"F10", GLFW_KEY_F10},
    {"F11", GLFW_KEY_F11},
    {"F12", GLFW_KEY_F12},

    // Arrow Keys
    {"ArrowUp", GLFW_KEY_UP},
    {"ArrowDown", GLFW_KEY_DOWN},
    {"ArrowLeft", GLFW_KEY_LEFT},
    {"ArrowRight", GLFW_KEY_RIGHT},

    // Control Keys
    {"Enter", GLFW_KEY_ENTER},
    {"Escape", GLFW_KEY_ESCAPE},
    {"Space", GLFW_KEY_SPACE},
    {"Tab", GLFW_KEY_TAB},
    {"ShiftLeft", GLFW_KEY_LEFT_SHIFT},
    {"ShiftRight", GLFW_KEY_RIGHT_SHIFT},
    {"ControlLeft", GLFW_KEY_LEFT_CONTROL},
    {"ControlRight", GLFW_KEY_RIGHT_CONTROL},
    {"AltLeft", GLFW_KEY_LEFT_ALT},
    {"AltRight", GLFW_KEY_RIGHT_ALT},
    {"CapsLock", GLFW_KEY_CAPS_LOCK},
    {"Backspace", GLFW_KEY_BACKSPACE},
    {"Delete", GLFW_KEY_DELETE},
    {"Insert", GLFW_KEY_INSERT},
    {"Home", GLFW_KEY_HOME},
    {"End", GLFW_KEY_END},
    {"PageUp", GLFW_KEY_PAGE_UP},
    {"PageDown", GLFW_KEY_PAGE_DOWN},

    // Numpad Keys
    {"Numpad0", GLFW_KEY_KP_0},
    {"Numpad1", GLFW_KEY_KP_1},
    {"Numpad2", GLFW_KEY_KP_2},
    {"Numpad3", GLFW_KEY_KP_3},
    {"Numpad4", GLFW_KEY_KP_4},
    {"Numpad5", GLFW_KEY_KP_5},
    {"Numpad6", GLFW_KEY_KP_6},
    {"Numpad7", GLFW_KEY_KP_7},
    {"Numpad8", GLFW_KEY_KP_8},
    {"Numpad9", GLFW_KEY_KP_9},
    {"NumpadDecimal", GLFW_KEY_KP_DECIMAL},
    {"NumpadDivide", GLFW_KEY_KP_DIVIDE},
    {"NumpadMultiply", GLFW_KEY_KP_MULTIPLY},
    {"NumpadSubtract", GLFW_KEY_KP_SUBTRACT},
    {"NumpadAdd", GLFW_KEY_KP_ADD},
    {"NumpadEnter", GLFW_KEY_KP_ENTER},
    {"NumpadEqual", GLFW_KEY_KP_EQUAL},

    // Punctuation and Symbols
    {"Backquote", GLFW_KEY_GRAVE_ACCENT},
    {"Minus", GLFW_KEY_MINUS},
    {"Equal", GLFW_KEY_EQUAL},
    {"BracketLeft", GLFW_KEY_LEFT_BRACKET},
    {"BracketRight", GLFW_KEY_RIGHT_BRACKET},
    {"Backslash", GLFW_KEY_BACKSLASH},
    {"Semicolon", GLFW_KEY_SEMICOLON},
    {"Quote", GLFW_KEY_APOSTROPHE},
    {"Comma", GLFW_KEY_COMMA},
    {"Period", GLFW_KEY_PERIOD},
    {"Slash", GLFW_KEY_SLASH},

    // Additional Keys (Add more as needed)
    {"PrintScreen", GLFW_KEY_PRINT_SCREEN},
    {"ScrollLock", GLFW_KEY_SCROLL_LOCK},
    {"Pause", GLFW_KEY_PAUSE},
    {"ContextMenu", GLFW_KEY_MENU},
    {"IntlBackslash", GLFW_KEY_UNKNOWN}, // Example of an unmapped key
    // ... add other keys as necessary
};