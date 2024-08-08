#include "raygpu.h"
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
    out.position = uMyUniforms.trf * vec4f(in.position.xyz + storig[0].xyz * 0.3f, 1.0f);
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(textureSample(gradientTexture, grsampler, in.uv).rgb, 1.0f) * in.color;
    //return vec4f(in.uv.xy, 0.0f, 1.0f);
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
webgpu_cxx_state* sample;
GLFWwindow* InitWindow(uint32_t width, uint32_t height){
    sample = new webgpu_cxx_state;
    #ifndef __EMSCRIPTEN__
    //dawnProcSetProcs(&dawn::native::GetProcs());
    #endif
    #ifndef __EMSCRIPTEN__
    wgpu::DawnTogglesDescriptor toggles = {};
    toggles.enabledToggles = 0;
    toggles.enabledToggleCount = 0;
    toggles.disabledToggles = 0;
    toggles.disabledToggleCount = 0;
    wgpu::InstanceDescriptor instanceDescriptor = {};
    instanceDescriptor.nextInChain = &toggles;
    instanceDescriptor.features.timedWaitAnyEnable = true;
    sample->instance = wgpu::CreateInstance(&instanceDescriptor);
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.compatibilityMode = true;
    adapterOptions.backendType = wgpu::BackendType::Undefined;
    adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, const char* message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    std::cerr << "Failed to get an adapter: " << message << "\n";
                    return;
                }
                sample->adapter = std::move(adapter);
            }),
        UINT64_MAX);
    if (sample->adapter == nullptr) {
        return nullptr;
    }
    wgpu::DeviceDescriptor deviceDesc = {};
    wgpu::RequiredLimits limits;
    limits.limits.maxTextureDimension2D = 1 << 13;
    deviceDesc.requiredLimits = &limits;

    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, const char* message) {
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
            std::cerr << "Device lost because of " << reasonName << ": " << message;
        });
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, const char* message) {
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
            std::cerr << errorTypeName << " error: " << message;
        });

    sample->instance.WaitAny(
        sample->adapter.RequestDevice(
            &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::RequestDeviceStatus status, wgpu::Device device, const char* message) {
                if (status != wgpu::RequestDeviceStatus::Success) {
                    std::cerr << "Failed to get an device:" << message;
                    return;
                }
                sample->device = std::move(device);
                sample->queue = sample->device.GetQueue();
            }),
        UINT64_MAX);
    
    if (sample->device == nullptr) {
        return nullptr;
    }
    #else
    sample->instance = wgpu::CreateInstance(nullptr);
    wgpu::RequestAdapterOptions adapterOptions = {};
    // Create the adapter, device, and set the emscripten loop via callbacks
    // TODO(crbug.com/42241221) Update to use the newer APIs once they are implemented in
    // Emscripten.
    sample->instance.RequestAdapter(
        &adapterOptions,
        [](WGPURequestAdapterStatus status, WGPUAdapter adapter, const char* message,
           void* userdata) {
            if (status != WGPURequestAdapterStatus_Success) {
                std::cerr << "Failed to get an adapter:" << message;
                return;
            }
            sample->adapter = wgpu::Adapter::Acquire(adapter);

            wgpu::DeviceDescriptor deviceDesc = {};
            sample->adapter.RequestDevice(
                &deviceDesc,
                [](WGPURequestDeviceStatus status, WGPUDevice device, const char* message,
                   void* userdata) {
                    if (status != WGPURequestDeviceStatus_Success) {
                        std::cerr << "Failed to get an device:" << message;
                        return;
                    }
                    sample->device = wgpu::Device::Acquire(device);
                    sample->queue = sample->device.GetQueue();

                    //if (sample->Setup()) {
                    //    emscripten_set_main_loop([]() { sample->FrameImpl(); }, 0, false);
                    //} else {
                    //    std::cerr << "Failed to setup sample";
                    //}
                },
                nullptr);
        },
        nullptr);
        while(sample->queue.Get() == 0){
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
            emscripten_sleep(100);
        }
        std::cout << "Success" << std::endl;
    #endif
    glfwInit();
        glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error: " << code << " - " << message;
    });

    if (!glfwInit()) {
        abort();
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Dawn window", nullptr, nullptr);
    if (!window) {
        abort();
    }
    #ifndef __EMSCRIPTEN__
    sample->surface = wgpu::glfw::CreateSurfaceForWindow(sample->instance, window);
    #else
    //WGPUTextureFormat swapChainFormat = WGPUTextureFormat_RGBA8Unorm;
    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
    canvasDesc.selector = "#canvas";
    wgpu::SurfaceDescriptor surfaceDesc = {};
    surfaceDesc.nextInChain = &canvasDesc;
    sample->surface = sample->instance.CreateSurface(&surfaceDesc);
    /*
    WGPUSwapChainDescriptor swapChainDesc{};
    swapChainDesc.width = width;
    swapChainDesc.height = height;
    swapChainDesc.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDesc.format = swapChainFormat;
    swapChainDesc.presentMode = WGPUPresentMode_Fifo;
    g_wgpustate.swapChain = wgpuDeviceCreateSwapChain(g_wgpustate.device, sample->surface.Get(), &swapChainDesc);*/
    #endif
    
    //#ifndef __EMSCRIPTEN__
    wgpu::SurfaceCapabilities capabilities;
    sample->surface.GetCapabilities(sample->adapter, &capabilities);
    wgpu::SurfaceConfiguration config = {};
    config.device = sample->device;
    config.format = capabilities.formats[0];
    config.presentMode = wgpu::PresentMode::Immediate;

    config.width = width;
    config.height = height;
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
    WGPUBufferDescriptor dummy{};
    dummy.mappedAtCreation = false;
    dummy.size = 64;
    dummy.usage = WGPUBufferUsage_Storage;

    float data[16] = {0};
    //std::fill(data, data + 16, 1.0f);

    g_wgpustate.dummyStorageBuffer = wgpuDeviceCreateBuffer(GetDevice(), &dummy);
    
    ShaderInputs shaderInputs{};
    auto arraySetter = [](uint32_t (&dat)[8], std::initializer_list<uint32_t> arg){
        for(auto it = arg.begin();it != arg.end();it++){
            dat[it - arg.begin()] = *it;
        }
    };
    auto uarraySetter = [](uniform_type (&dat)[8], std::initializer_list<uniform_type> arg){
        for(auto it = arg.begin();it != arg.end();it++){
            dat[it - arg.begin()] = *it;
        }
    };
    
    glfwSetKeyCallback(
        window, 
        [](GLFWwindow* window, int key, int scancode, int action, int mods){
            if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
                glfwSetWindowShouldClose(window, true);
            }
        }
    );
    shaderInputs.per_vertex_count = 3;
    shaderInputs.per_instance_count = 0;
    
    shaderInputs.uniform_count = 4;
    arraySetter(shaderInputs.per_vertex_sizes, {3,2,4});
    arraySetter(shaderInputs.uniform_minsizes, {64, 0, 0, 0});
    uarraySetter(shaderInputs.uniform_types, {uniform_buffer, texture2d, sampler, storage_buffer});
    
    depthTexture = rtex.depth;

    init_full_renderstate(g_wgpustate.rstate, tShader, shaderInputs, rtex.color.view, rtex.depth.view);
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
    
    #ifndef __EMSCRIPTEN__
    return window;
    #else
    return nullptr;
    #endif
}