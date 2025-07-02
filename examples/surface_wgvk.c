#include <GLFW/glfw3.h>
#include <wgvk.h>
#include <stdio.h>
#include <external/incbin.h>
//INCBIN(simple_shader, "../resources/simple_shader.wgsl");
INCBIN(simple_shaderSpirv, "../resources/simple_shader.spv");
#ifndef STRVIEW
    #define STRVIEW(X) (WGPUStringView){X, sizeof(X) - 1}
#endif
#ifdef __EMSCRIPTEN__
#  define GLFW_EXPOSE_NATIVE_EMSCRIPTEN
#  ifndef GLFW_PLATFORM_EMSCRIPTEN // not defined in older versions of emscripten
#    define GLFW_PLATFORM_EMSCRIPTEN 0
#  endif
#else // __EMSCRIPTEN__
#  ifdef SUPPORT_XLIB_SURFACE
#    define GLFW_EXPOSE_NATIVE_X11
#  endif
#  ifdef SUPPORT_WAYLAND_SURFACE
#    define GLFW_EXPOSE_NATIVE_WAYLAND
#  endif
#  ifdef _GLFW_COCOA
#    define GLFW_EXPOSE_NATIVE_COCOA
#  endif
#  ifdef _WIN32
#    define GLFW_EXPOSE_NATIVE_WIN32
#  endif
#endif // __EMSCRIPTEN__

#ifdef GLFW_EXPOSE_NATIVE_COCOA
#  include <Foundation/Foundation.h>
#  include <QuartzCore/CAMetalLayer.h>
#endif

#ifndef __EMSCRIPTEN__
#  include <GLFW/glfw3native.h>
#endif

#include <stdint.h>

/* ---------- POSIX / Unix-like ---------- */
#if defined(__unix__) || defined(__APPLE__)
  #include <time.h>

  static inline uint64_t nanoTime(void)
  {
      struct timespec ts;
  #if defined(CLOCK_MONOTONIC_RAW)        /* Linux, FreeBSD */
      clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  #else                                   /* macOS 10.12+, other POSIX */
      clock_gettime(CLOCK_MONOTONIC, &ts);
  #endif
      return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
  }

/* ---------- Windows ---------- */
#elif defined(_WIN32)
  #include <windows.h>

  static inline uint64_t nanoTime(void)
  {
      static LARGE_INTEGER freq = { 0 };
      if (freq.QuadPart == 0)               /* one-time init */
          QueryPerformanceFrequency(&freq);

      LARGE_INTEGER counter;
      QueryPerformanceCounter(&counter);
      /* scale ticks → ns: (ticks * 1e9) / freq */
      return (uint64_t)((counter.QuadPart * 1000000000ULL) / freq.QuadPart);
  }

#else
  #error "Platform not supported"
#endif

static void adapterCallbackFunction(
        WGPURequestAdapterStatus status,
        WGPUAdapter adapter,
        WGPUStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGPUAdapter*)userdata1) = adapter;
}
static void deviceCallbackFunction(
        WGPURequestDeviceStatus status,
        WGPUDevice device,
        WGPUStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGPUDevice*)userdata1) = device;
}
void keyfunc(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
        return glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
int main(){
    
    WGPUInstanceLayerSelection lsel = {
        .chain = {
            .next = NULL,
            .sType = WGPUSType_InstanceValidationLayerSelection
        }
    };
    const char* layernames[] = {"VK_LAYER_KHRONOS_validation"};
    lsel.instanceLayers = layernames;
    lsel.instanceLayerCount = 1;
    
    WGPUInstanceDescriptor instanceDescriptor = {
        .nextInChain = 
        #ifdef NDEBUG
        NULL
        #else
        &lsel.chain
        #endif
        ,
        .capabilities = {0}
    };

    WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);

    WGPURequestAdapterOptions adapterOptions = {0};
    adapterOptions.featureLevel = WGPUFeatureLevel_Core;
    WGPURequestAdapterCallbackInfo adapterCallback = {0};
    adapterCallback.callback = adapterCallbackFunction;
    WGPUAdapter requestedAdapter;
    adapterCallback.userdata1 = (void*)&requestedAdapter;
    

    WGPUFuture aFuture = wgpuInstanceRequestAdapter(instance, &adapterOptions, adapterCallback);
    WGPUFutureWaitInfo winfo = {
        .future = aFuture,
        .completed = 0
    };

    wgpuInstanceWaitAny(instance, 1, &winfo, ~0ull);
    WGPUStringView deviceLabel = {"WGPU Device", sizeof("WGPU Device") - 1};

    WGPUDeviceDescriptor ddesc = {
        .nextInChain = 0,
        .label = deviceLabel,
        .requiredFeatureCount = 0,
        .requiredFeatures = NULL,
        .requiredLimits = NULL,
        .defaultQueue = {0},
        .deviceLostCallbackInfo = {0},
        .uncapturedErrorCallbackInfo = {0},
    };
    WGPUDevice device;
    
    WGPURequestDeviceCallbackInfo dcinfo = {
        .callback = deviceCallbackFunction,
        .mode = WGPUCallbackMode_WaitAnyOnly,
        .userdata1 = (void*)&device,
    };
    
    WGPUFuture future = wgpuAdapterRequestDevice(requestedAdapter, &ddesc, dcinfo);
    WGPUFutureWaitInfo waitInfo = {
        .future = future,
        .completed = 0
    };
    
    wgpuInstanceWaitAny(instance, 1, &waitInfo, UINT32_MAX);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    glfwInit();
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "WGVK Window", NULL, NULL);
    glfwSetKeyCallback(window, keyfunc);
    #ifdef _WIN32
    WGPUSurfaceSourceWindowsHWND surfaceChain = {
        .chain = {
            .sType = WGPUSType_SurfaceSourceWindowsHWND,
            .next = NULL
        },
        .hwnd = glfwGetWin32Window(window),
        .hinstance = GetModuleHandle(NULL)
    };
    #else    
    WGPUSurfaceSourceXlibWindow surfaceChain;
    Display* x11_display = glfwGetX11Display();
    Window x11_window = glfwGetX11Window(window);
    surfaceChain.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
    surfaceChain.chain.next = NULL;
    surfaceChain.display = x11_display;
    surfaceChain.window = x11_window;

    //struct wl_display* native_display = glfwGetWaylandDisplay();
    //struct wl_surface* native_surface = glfwGetWaylandWindow(window);
    //WGPUSurfaceSourceWaylandSurface surfaceChain;
    //surfaceChain.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    //surfaceChain.chain.next = NULL;
    //surfaceChain.display = native_display;
    //surfaceChain.surface = native_surface;
    #endif
    WGPUSurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &surfaceChain.chain;
    surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    WGPUSurfaceCapabilities caps = {0};
    WGPUPresentMode desiredPresentMode = WGPUPresentMode_Fifo;
    WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);

    wgpuSurfaceGetCapabilities(surface, requestedAdapter, &caps);
    wgpuSurfaceConfigure(surface, &(const WGPUSurfaceConfiguration){
        .alphaMode = WGPUCompositeAlphaMode_Opaque,
        .presentMode = desiredPresentMode,
        .device = device,
        .format = WGPUTextureFormat_BGRA8Unorm,
        .width = width,
        .height = height
    });
    //WGPUShaderSourceWGSL shaderSourceWgsl = {
    //    .chain = {
    //        .sType = WGPUSType_ShaderSourceWGSL
    //    },
    //    .code = {
    //        .data = (const char*)gsimple_shaderData,
    //        .length = gsimple_shaderSize
    //    }
    //};
    
    WGPUShaderSourceSPIRV shaderSourceSpirv = {
        .chain = {
            .sType = WGPUSType_ShaderSourceSPIRV
        },
        .code = (uint32_t*)gsimple_shaderSpirvData,
        .codeSize = gsimple_shaderSpirvSize,
    };

    WGPUShaderModuleDescriptor shaderModuleDesc = {
        .nextInChain = &shaderSourceSpirv.chain
    };
    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(device, &shaderModuleDesc);
    WGPUVertexAttribute vbAttribute = {
        .nextInChain = NULL,
        .shaderLocation = 0,
        .format = WGPUVertexFormat_Float32x2,
        .offset = 0
    };
    WGPUVertexBufferLayout vbLayout = {
        .nextInChain = NULL,
        .arrayStride = sizeof(float) * 2,
        .attributeCount = 1,
        .attributes = &vbAttribute,
        .stepMode = WGPUVertexStepMode_Vertex
    };
    WGPUBlendState blendState = {
        .alpha = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_One,
            .dstFactor = WGPUBlendFactor_One
        },
        .color = {
            .operation = WGPUBlendOperation_Add,
            .srcFactor = WGPUBlendFactor_One,
            .dstFactor = WGPUBlendFactor_One
        }
    };

    WGPUColorTargetState colorTargetState = {
        .format = WGPUTextureFormat_BGRA8Unorm,
        .writeMask = WGPUColorWriteMask_All,
        .blend = NULL
    };

    WGPUFragmentState fragmentState = {
        .entryPoint = STRVIEW("fs_main"),
        .module = shaderModule,
        .targetCount = 1,
        .targets = &colorTargetState,
    };
    WGPUPipelineLayoutDescriptor pldesc = {0};
    WGPUPipelineLayout pllayout = wgpuDeviceCreatePipelineLayout(device, &pldesc);

    WGPURenderPipelineDescriptor rpdesc = {
        .vertex = {
            .bufferCount = 1,
            .buffers = &vbLayout,
            .module = shaderModule,
            .entryPoint = STRVIEW("vs_main")
        },
        .fragment = &fragmentState,
        .primitive = {
            .cullMode = WGPUCullMode_None,
            .frontFace = WGPUFrontFace_CCW,
            .topology = WGPUPrimitiveTopology_TriangleList
        },
        .layout = pllayout,
        .multisample = {
            .count = 1,
            .mask = 0xffffffff
        },
    };
    WGPURenderPipeline rp = wgpuDeviceCreateRenderPipeline(device, &rpdesc);
    const float scale = 0.2f;
    const float vertices[6] = {-scale,-scale,-scale,scale,scale,scale};
    
    WGPUBufferDescriptor bufferDescriptor = {
        .size = sizeof(vertices),
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst
    };
    WGPUBuffer vertexBuffer = wgpuDeviceCreateBuffer(device, &bufferDescriptor);
    wgpuQueueWriteBuffer(queue, vertexBuffer, 0, vertices, sizeof(vertices));
    //wgpuDeviceTick(device);
    WGPUSurfaceTexture surfaceTexture;
    uint64_t stamp = nanoTime();
    uint64_t frameCount = 0;
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
        if(surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal){
            printf("Reconfiguring\n");
            glfwGetWindowSize(window, &width, &height);
            wgpuSurfaceConfigure(surface, &(const WGPUSurfaceConfiguration){
                .alphaMode = WGPUCompositeAlphaMode_Opaque,
                .presentMode = desiredPresentMode,
                .device = device,
                .format = WGPUTextureFormat_BGRA8Unorm,
                .width = width,
                .height = height
            });
            wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
        }
        WGPUTextureView surfaceView = wgpuTextureCreateView(surfaceTexture.texture, &(const WGPUTextureViewDescriptor){
            .baseArrayLayer = 0,
            .arrayLayerCount = 1,
            .baseMipLevel = 0,
            .mipLevelCount = 1,
            .format = WGPUTextureFormat_BGRA8Unorm,
            .dimension = WGPUTextureViewDimension_2D,
            .usage = WGPUTextureUsage_RenderAttachment,
            .aspect = WGPUTextureAspect_All,
        });
        WGPUCommandEncoder cenc = wgpuDeviceCreateCommandEncoder(device, NULL);
        WGPURenderPassColorAttachment colorAttachment = {
            .clearValue = (WGPUColor){0.5,0.2,0,1},
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .view = surfaceView
        };

        WGPURenderPassEncoder rpenc = wgpuCommandEncoderBeginRenderPass(cenc, &(const WGPURenderPassDescriptor){
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment,
        });
    
        wgpuRenderPassEncoderSetPipeline(rpenc, rp);
        wgpuRenderPassEncoderSetScissorRect(rpenc, 0,0, 800, 600);
        wgpuRenderPassEncoderSetVertexBuffer(rpenc, 0, vertexBuffer, 0, WGPU_WHOLE_SIZE);
        wgpuRenderPassEncoderDraw(rpenc, 6, 1, 0, 0);
        wgpuRenderPassEncoderEnd(rpenc);
        
        WGPUCommandBuffer cBuffer = wgpuCommandEncoderFinish(cenc, NULL);
        wgpuQueueSubmit(queue, 1, &cBuffer);
        wgpuCommandEncoderRelease(cenc);
        wgpuCommandBufferRelease(cBuffer);
        wgpuRenderPassEncoderRelease(rpenc);
        wgpuTextureViewRelease(surfaceView);
        wgpuSurfacePresent(surface);
        ++frameCount;
        uint64_t nextStamp = nanoTime();
        if(nextStamp - stamp > ((uint64_t)1000000000ULL)){
            stamp = nextStamp;
            printf("FPS: %llu\n", (unsigned long long)frameCount);
            frameCount = 0;
        }
    }
    wgpuSurfaceRelease(surface);
}