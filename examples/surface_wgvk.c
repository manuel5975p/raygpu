#include <GLFW/glfw3.h>
#include <wgvk.h>
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
#  ifdef _GLFW_WIN32
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


void adapterCallbackFunction(
        WGPURequestAdapterStatus status,
        WGPUAdapter adapter,
        WGPUStringView label,
        void* userdata1,
        void* userdata2
    ){
    *((WGPUAdapter*)userdata1) = adapter;
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
    
    WGPUDevice device = wgpuAdapterCreateDevice(requestedAdapter, &ddesc);
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(500, 500, "Binbow", NULL, NULL);
    Display* x11_display = glfwGetX11Display();
    Window x11_window = glfwGetX11Window(window);
    glfwSetKeyCallback(window, keyfunc);


    WGPUSurfaceSourceXlibWindow surfaceChain;
    surfaceChain.chain.sType = WGPUSType_SurfaceSourceXlibWindow;
    surfaceChain.chain.next = NULL;
    surfaceChain.display = x11_display;
    surfaceChain.window = x11_window;

    struct wl_display* native_display = glfwGetWaylandDisplay();
    struct wl_surface* native_surface = glfwGetWaylandWindow(window);

    //WGPUSurfaceSourceWaylandSurface surfaceChain;
    //surfaceChain.chain.sType = WGPUSType_SurfaceSourceWaylandSurface;
    //surfaceChain.chain.next = NULL;
    //surfaceChain.display = native_display;
    //surfaceChain.surface = native_surface;

    WGPUSurfaceDescriptor surfaceDescriptor;
    surfaceDescriptor.nextInChain = &surfaceChain.chain;
    surfaceDescriptor.label = (WGPUStringView){ NULL, WGPU_STRLEN };
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    wgpuSurfaceConfigure(surface, &(const WGPUSurfaceConfiguration){
        .alphaMode = WGPUCompositeAlphaMode_Opaque,
        .presentMode = WGPUPresentMode_Fifo,
        .device = device,
        .format = WGPUTextureFormat_BGRA8Unorm,
        .width = width,
        .height = height
    });
    WGPUSurfaceTexture surfaceTexture;
    int i = 0;
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        wgpuSurfaceGetCurrentTexture(surface, &surfaceTexture);
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
            .clearValue = (WGPUColor){(i % 255) / 255.0,0,0,1},
            .loadOp = WGPULoadOp_Clear,
            .storeOp = WGPUStoreOp_Store,
            .view = surfaceView
        };

        WGPURenderPassEncoder rpenc = wgpuCommandEncoderBeginRenderPass(cenc, &(const WGPURenderPassDescriptor){
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment,
        });
        wgpuRenderPassEncoderEnd(rpenc);
        
        WGPUCommandBuffer cBuffer = wgpuCommandEncoderFinish(cenc);
        
        wgpuQueueSubmit(queue, 1, &cBuffer);
        wgpuCommandEncoderRelease(cenc);
        wgpuCommandBufferRelease(cBuffer);
        wgpuRenderPassEncoderRelease(rpenc);
        wgpuTextureViewRelease(surfaceView);
        wgpuSurfacePresent(surface);
        //glfwSwapBuffers(window);
    }
    wgpuSurfaceRelease(surface);
}