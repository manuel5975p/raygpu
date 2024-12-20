#include <raygpu.h>
#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include <chrono>
#include "GLFW/glfw3.h"
#ifndef __EMSCRIPTEN__
#include <emscripten/html5.h>           // Emscripten HTML5 library
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
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

// Instead of the simple uTime variable, our uniform variable is a struct
@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
@group(0) @binding(2) var grsampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
@group(0) @binding(4) var<storage> lights: LightBuffer;

//Can be omitted
//@group(0) @binding(3) var<storage> storig: array<vec4f>;


@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = Perspective_View * 
                   modelMatrix[instanceIdx] *
    vec4f(in.position.xyz /*+ storig[0].xyz * 0.3f*/, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return textureSample(gradientTexture, grsampler, in.uv).rgba * in.color;
}
)";
extern Texture depthTexture;
struct full_renderstate;
#include "wgpustate.inc"
struct webgpu_cxx_state{
    wgpu::Instance instance{};
    wgpu::Adapter adapter{};
    wgpu::Device device{};
    wgpu::Queue queue{};
    wgpu::Surface surface{};

    full_renderstate* rstate = nullptr;
    Texture depthTexture{};
};
void mcb(GLFWwindow* window, double x, double y){
    TraceLog(LOG_INFO, "Mousepos");
}
//webgpu_cxx_state* sample;
GLFWwindow* InitWindow(uint32_t width, uint32_t height, const char* title){
    g_wgpustate.instance = nullptr;
    g_wgpustate.adapter = nullptr;
    g_wgpustate.device = nullptr;
    g_wgpustate.queue = nullptr;
    webgpu_cxx_state samplev;
    webgpu_cxx_state* sample = &samplev;
    
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
        adapterOptions.compatibilityMode = bcompat(backendType);

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

    // Synchronously create the adapter
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    std::cerr << "Failed to get an adapter:" << message;
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
    wgpu::DeviceDescriptor deviceDesc = {};
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
            std::cerr << "Device lost because of " << reasonName << ": " << std::string(message.data, message.length);
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
    reqLimits.limits.maxBufferSize = 1ull << 28;
    
    deviceDesc.requiredLimits = &reqLimits;
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
        std::cerr << "Device is null\n";
        abort();
    }
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error: " << code << " - " << message;
    });

    if (!glfwInit()) {
        abort();
    }
    GLFWmonitor* mon = nullptr;
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
    GLFWwindow* window;
#ifndef __EMSCRIPTEN__
    window = glfwCreateWindow(width, height, title, mon, nullptr);
    //glfwSetWindowPos(window, 200, 1200);
    if (!window) {
        abort();
    }
    g_wgpustate.window = window;
    // Create the surface.
    sample->surface = wgpu::glfw::CreateSurfaceForWindow(sample->instance, window);
#else
    // Create the surface.
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &canvasDesc;
    sample->surface = sample->instance.CreateSurface(&surfaceDesc);
    window = glfwCreateWindow(width, height, title, mon, nullptr);
    g_wgpustate.window = window;
#endif
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height){
        //wgpuSurfaceRelease(g_wgpustate.surface);
        //g_wgpustate.surface = wgpu::glfw::CreateSurfaceForWindow(g_wgpustate.instance, window).MoveToCHandle();
        //while(!g_wgpustate.drawmutex.try_lock());
        g_wgpustate.drawmutex.lock();
        std::cout << "Size callbacked " << width << " " << height << "\n";
        WGPUSurfaceCapabilities capabilities;
        wgpuSurfaceGetCapabilities(g_wgpustate.surface, g_wgpustate.adapter, &capabilities);
        WGPUSurfaceConfiguration config = {};
        config.alphaMode = WGPUCompositeAlphaMode_Opaque;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = g_wgpustate.device;
        config.format = (WGPUTextureFormat)capabilities.formats[0];
        config.presentMode = !!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) ? WGPUPresentMode_Fifo : WGPUPresentMode_Immediate;
        config.width = width;
        config.height = height;
        g_wgpustate.width = width;
        g_wgpustate.height = height;
        wgpuTextureViewRelease(depthTexture.view);
        wgpuTextureRelease(depthTexture.id);
        depthTexture = LoadDepthTexture(width, height);
        wgpuSurfaceConfigure(g_wgpustate.surface, &config);
        Matrix newcamera = ScreenMatrix(width, height);
        BufferData(&g_wgpustate.defaultScreenMatrix, &newcamera, sizeof(Matrix));

        setTargetTextures(g_wgpustate.rstate, g_wgpustate.rstate->color, depthTexture.view);
        //updateRenderPassDesc(g_wgpustate.rstate);

        //TODO wtf is this?
        g_wgpustate.rstate->renderpass.dsa->view = depthTexture.view;
        g_wgpustate.drawmutex.unlock();
    });
    
    //#ifndef __EMSCRIPTEN__
    wgpu::SurfaceCapabilities capabilities;
    sample->surface.GetCapabilities(sample->adapter, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = sample->device;
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
    config.presentMode = !!(g_wgpustate.windowFlags & FLAG_VSYNC_HINT) ? wgpu::PresentMode::Fifo : wgpu::PresentMode::Immediate;

    config.width = width;
    config.height = height;
    g_wgpustate.width = width;
    g_wgpustate.height = height;
    sample->surface.Configure(&config);
    //#endif
    g_wgpustate.adapter = sample->adapter.MoveToCHandle();
    g_wgpustate.device = sample->device.MoveToCHandle();
    g_wgpustate.queue = sample->queue.MoveToCHandle();
    g_wgpustate.instance = sample->instance.MoveToCHandle();
    g_wgpustate.surface = sample->surface.MoveToCHandle();
    g_wgpustate.frameBufferFormat = (WGPUTextureFormat)config.format;
    //std::cout << "Supported Framebuffer Format: 0x" << std::hex << (WGPUTextureFormat)config.format << std::dec << "\n";
    
    TraceLog(LOG_INFO, "Loading whitepixel texture");
    g_wgpustate.whitePixel = LoadTextureFromImage(GenImageChecker(Color{255,255,255,255}, Color{255,255,255,255}, 16, 16, 0));
    TraceLog(LOG_INFO, "Loaded whitepixel texture");

    WGPUShaderModule tShader = LoadShaderFromMemory(shaderSource);
    //RenderTexture rtex = LoadRenderTexture(width, height);
    g_wgpustate.rstate = new full_renderstate;
    

    float data[16] = {0};
    //std::fill(data, data + 16, 1.0f);
    
    /*ShaderInputs shaderInputs{};
    auto arraySetter = [](uint32_t (&dat)[8], std::initializer_list<uint32_t> arg){
        for(auto it = arg.begin();it != arg.end();it++){
            dat[it - arg.begin()] = *it;
        }
    };
    auto uarraySetter = [](uniform_type (&dat)[8], std::initializer_list<uniform_type> arg){
        for(auto it = arg.begin();it != arg.end();it++){
            dat[it - arg.begin()] = *it;
        }
    };*/
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset){
        g_wgpustate.scrollThisFrame.x += xoffset;
        g_wgpustate.scrollThisFrame.y += yoffset;
    });
    auto cpcallback = [](GLFWwindow* window, double x, double y){
        g_wgpustate.mousePos = Vector2{float(x), float(y)};
    };
    #ifndef __EMSCRIPTEN__
    glfwSetCursorPosCallback(window, cpcallback);
    #else
    auto EmscriptenMouseCallback = [](int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData){
        //(*((decltype(cpcallback)*)userData))(nullptr, mouseEvent->screenX, mouseEvent->screenY);
        (*((decltype(cpcallback)*)userData))(nullptr, mouseEvent->clientX, mouseEvent->clientY);
        TRACELOG(LOG_INFO, "clientX: %d", mouseEvent->canvasX);
        return true;
    };
    //emscripten_set_click_callback("#canvas", NULL, 1, EmscriptenMouseCallback);
    emscripten_set_mousemove_callback("#canvas", &cpcallback, 1, EmscriptenMouseCallback);
    #endif // __EMSCRIPTEN__
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
        if(action == GLFW_PRESS){
            g_wgpustate.mouseButtonDown[button] = 1;
        }
        else if(action == GLFW_RELEASE){
            g_wgpustate.mouseButtonDown[button] = 0;
        }
    });
    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* window, int key, int scancode, int action, int mods){
            if(action == GLFW_PRESS){
                g_wgpustate.keydown[key] = 1;
            }else if(action == GLFW_RELEASE){
                g_wgpustate.keydown[key] = 0;
            }
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
                glfwSetWindowShouldClose(window, true);
            }
        }
    );
    glfwSetCharCallback(window, [](GLFWwindow* window, unsigned int codePoint){
        g_wgpustate.charQueue.push_back((int)codePoint);
    });
    glfwSetCursorEnterCallback(window, [](GLFWwindow* window, int entered){
        g_wgpustate.cursorInWindow = entered;
    });
    
    //shaderInputs.per_vertex_count = 3;
    //shaderInputs.per_instance_count = 0;
    
    //shaderInputs.uniform_count = 4;
    UniformDescriptor uniforms[4] = {
        UniformDescriptor{uniform_buffer, 64},
        UniformDescriptor{texture2d, 0},
        UniformDescriptor{sampler, 0},
        UniformDescriptor{storage_buffer, 64}
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
    depthTexture = LoadTexturePro(width,
                                  height, 
                                  WGPUTextureFormat_Depth24Plus, 
                                  WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc, 
                                  (g_wgpustate.windowFlags & FLAG_MSAA_4X_HINT) ? 4 : 1
    );
    init_full_renderstate(g_wgpustate.rstate, shaderSource, attrs, 4, uniforms, sizeof(uniforms) / sizeof(UniformDescriptor), colorTexture.view, depthTexture.view);
    g_wgpustate.rstate->renderExtentX = width;
    g_wgpustate.rstate->renderExtentY = height;
    {
        int wposx, wposy;
        glfwGetWindowPos(g_wgpustate.window, &wposx, &wposy);
        g_wgpustate.windowPosition = Rectangle{
            (float)wposx,
            (float)wposy,
            (float)GetScreenWidth(),
            (float)GetScreenHeight()
        };
    }

    LoadFontDefault();
    for(size_t i = 0;i < 100;i++){
        g_wgpustate.smallBufferPool.push_back(GenBuffer(nullptr, sizeof(vertex) * 10));
    }
    WGPUCommandEncoderDescriptor cedesc{};
    cedesc.label = STRVIEW("Global Command Encoder");
    g_wgpustate.rstate->renderpass.cmdEncoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device, &cedesc);
    Matrix m = ScreenMatrix(width, height);
    static_assert(sizeof(Matrix) == 64, "non 4 byte floats? or what");
    g_wgpustate.defaultScreenMatrix = GenUniformBuffer(&m, sizeof(Matrix));
    SetUniformBuffer(0, &g_wgpustate.defaultScreenMatrix);
    SetTexture(1, g_wgpustate.whitePixel);
    Matrix iden = MatrixIdentity();
    SetStorageBufferData(3, &iden, 64);

    WGPUSamplerDescriptor samplerDesc{};
    samplerDesc.addressModeU = WGPUAddressMode_Repeat;
    samplerDesc.addressModeV = WGPUAddressMode_Repeat;
    samplerDesc.addressModeW = WGPUAddressMode_Repeat;
    samplerDesc.magFilter    = WGPUFilterMode_Nearest;
    samplerDesc.minFilter    = WGPUFilterMode_Linear;
    samplerDesc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    samplerDesc.compare      = WGPUCompareFunction_Undefined;
    samplerDesc.lodMinClamp  = 0.0f;
    samplerDesc.lodMaxClamp  = 1.0f;
    samplerDesc.maxAnisotropy = 1;


    WGPUSampler sampler = wgpuDeviceCreateSampler(g_wgpustate.device, &samplerDesc);
    SetSampler(2, sampler);
    g_wgpustate.init_timestamp = NanoTime();
    TraceLog(LOG_INFO, "oof");
    #ifndef __EMSCRIPTEN__
    if(g_wgpustate.windowFlags & FLAG_VSYNC_HINT){
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
void CloseWindow(cwoid){
    glfwSetWindowShouldClose(g_wgpustate.window, GLFW_TRUE);
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