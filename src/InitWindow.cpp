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
#endif  // __EMSCRIPTEN__
inline std::ostream& operator<<(std::ostream& ostr, const wgpu::StringView& st){
    ostr.write(st.data, st.length);
    return ostr;
}
constexpr char shaderSource[] = R"(
struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
    @location(2) color: vec4f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
    @location(1) color: vec4f,
};

/**
 * A structure holding the value of our uniforms
 */
struct MyUniforms {
    trf: mat4x4f
};

// Instead of the simple uTime variable, our uniform variable is a struct
@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;
@group(0) @binding(1) var gradientTexture: texture_2d<f32>;
@group(0) @binding(2) var grsampler: sampler;

//Can be omitted
@group(0) @binding(3) var<storage> storig: array<vec4f>;

//@builtin(instance_index) instanceID: u32;
@vertex
fn vs_main(@builtin(instance_index) instanceIdx : u32, in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = uMyUniforms.trf * 
    vec4f(in.position.xyz /*+ storig[0].xyz * 0.3f*/, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(textureSample(gradientTexture, grsampler, in.uv).rgb, 1.0f) * in.color;
    //return vec4f(1.0f, 1.0f, 0.0f, 1.0f);
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
//webgpu_cxx_state* sample;
GLFWwindow* InitWindow(uint32_t width, uint32_t height){
    g_wgpustate.instance = nullptr;
    g_wgpustate.adapter = nullptr;
    g_wgpustate.device = nullptr;
    g_wgpustate.queue = nullptr;
    webgpu_cxx_state samplev;
    webgpu_cxx_state* sample = &samplev;
    
    // Create the toggles descriptor if not using emscripten.
    wgpu::ChainedStruct* togglesChain = nullptr;
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
    //std::cout << "Using adapter \"" << info.device << "\"\n";

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

    #ifndef __EMSCRIPTEN__
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error: " << code << " - " << message;
    });

    if (!glfwInit()) {
        abort();
    }

    // Create the test window with no client API.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(width, height, "Dawn window", nullptr, nullptr);
    if (!window) {
        abort();
    }

    // Create the surface.
    sample->surface = wgpu::glfw::CreateSurfaceForWindow(sample->instance, window);
#else
    // Create the surface.
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &canvasDesc;
    surface = instance.CreateSurface(&surfaceDesc);
#endif
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height){
        //wgpuSurfaceRelease(g_wgpustate.surface);
        //g_wgpustate.surface = wgpu::glfw::CreateSurfaceForWindow(g_wgpustate.instance, window).MoveToCHandle();
        g_wgpustate.drawmutex.lock();
        WGPUSurfaceCapabilities capabilities;
        wgpuSurfaceGetCapabilities(g_wgpustate.surface, g_wgpustate.adapter, &capabilities);
        WGPUSurfaceConfiguration config = {};
        config.alphaMode = WGPUCompositeAlphaMode_Opaque;
        config.usage = WGPUTextureUsage_RenderAttachment;
        config.device = g_wgpustate.device;
        config.format = (WGPUTextureFormat)capabilities.formats[0];
        config.presentMode = WGPUPresentMode_Immediate;

        config.width = width;
        config.height = height;
        g_wgpustate.width = width;
        g_wgpustate.height = height;
        wgpuTextureViewRelease(depthTexture.view);
        wgpuTextureRelease(depthTexture.tex);
        depthTexture = LoadDepthTexture(width, height);
        wgpuSurfaceConfigure(g_wgpustate.surface, &config);
        updateRenderPassDesc(g_wgpustate.rstate);
        g_wgpustate.drawmutex.unlock();
    });
    
    //#ifndef __EMSCRIPTEN__
    wgpu::SurfaceCapabilities capabilities;
    sample->surface.GetCapabilities(sample->adapter, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = sample->device;
    config.format = capabilities.formats[0];
    config.presentMode = wgpu::PresentMode::Immediate;

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

    
    g_wgpustate.whitePixel = LoadTextureFromImage(LoadImageChecker(Color{255,255,255,255}, Color{255,255,255,255}, 1, 1, 0));

    WGPUShaderModule tShader = LoadShaderFromMemory(shaderSource);
    RenderTexture rtex = LoadRenderTexture(width, height);
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
    
    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* window, int key, int scancode, int action, int mods){
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
                glfwSetWindowShouldClose(window, true);
            }
        }
    );
    //shaderInputs.per_vertex_count = 3;
    //shaderInputs.per_instance_count = 0;
    
    //shaderInputs.uniform_count = 4;
    UniformDescriptor desc[4] = {
        UniformDescriptor{uniform_buffer, 64},
        UniformDescriptor{texture2d, 0},
        UniformDescriptor{sampler, 0},
        UniformDescriptor{storage_buffer, 0}
    };
    AttributeAndResidence attrs[3] = {
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x3, 0, 0}, 0, WGPUVertexStepMode_Vertex},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 12, 1}, 0, WGPUVertexStepMode_Vertex},
        AttributeAndResidence{WGPUVertexAttribute{WGPUVertexFormat_Float32x4, 20, 2}, 0, WGPUVertexStepMode_Vertex},
    };
    //arraySetter(shaderInputs.per_vertex_sizes, {3,2,4});
    //arraySetter(shaderInputs.uniform_minsizes, {64, 0, 0, 0});
    //uarraySetter(shaderInputs.uniform_types, {uniform_buffer, texture2d, sampler, storage_buffer});
    
    depthTexture = rtex.depth;
    init_full_renderstate(g_wgpustate.rstate, shaderSource, attrs, 3, desc, 4, rtex.color.view, rtex.depth.view);
    WGPUCommandEncoderDescriptor cedesc{};
    cedesc.label = STRVIEW("Global Command Encoder");
    g_wgpustate.rstate->renderpass.cmdEncoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device, &cedesc);
    setStateStorageBuffer(g_wgpustate.rstate, 3, data, 64);

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
    setStateSampler(g_wgpustate.rstate, 2, sampler);
    g_wgpustate.init_timestamp = NanoTime();
    #ifndef __EMSCRIPTEN__
    return window;
    #else
    return nullptr;
    #endif
}