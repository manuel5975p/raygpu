#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include <chrono>
#include "vertex_buffer.hpp"
#include "GLFW/glfw3.h"
#include "raygpu.h"
#ifndef __EMSCRIPTEN__
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__
uint64_t nanoTime(){
    using namespace std;
    using namespace chrono;
    return duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
}
Texture depthTexture;


int main(){
    uint32_t width = 400;
    uint32_t height = 400;
    auto window = InitWindow(width, height);
    if(
        #ifndef __EMSCRIPTEN__
        window == nullptr
         ||
         #endif
        g_wgpustate.surface == nullptr){
        std::terminate();
    }
    
    
    
    Texture tex(LoadImageChecker(Color{255,0,0,255}, Color{0,255,0,255}, 100, 100, 10));
    Texture single(LoadImageChecker(Color{255,255,255,255}, Color{255,255,255,255}, 1, 1, 0));

    rlTexCoord2f(0.1, 0.1);
    rlColor3f(1,0.5,0);
    rlVertex2f(0, 0);
    rlColor3f(0,0,1);
    rlTexCoord2f(0.1, 0.9);
    rlVertex2f(1,0);
    rlColor3f(1,1,1);
    rlTexCoord2f(0.9, 0.1);
    rlVertex2f(0,1);
    
    depthTexture = LoadDepthTexture(width, height);
    float udata[16] = {0};

    
    g_wgpustate.rstate->updateVertexBuffer(g_wgpustate.current_vertices.data(), g_wgpustate.current_vertices.size() * sizeof(vertex));
    g_wgpustate.rstate->setTexture(1, single);
    g_wgpustate.rstate->setUniformBuffer(0, udata, 16 * sizeof(float));
    g_wgpustate.rstate->updateBindGroup();
    auto mainloop = [](void* userdata){
        glfwPollEvents();
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture(g_wgpustate.surface, &surfaceTexture);

        //WGPUTextureView nextTexture = surfaceTexture.texture.CreateView().MoveToCHandle();
        //wgpu::Texture as;
        //as.CreateView();
        WGPUTextureView nextTexture = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
        g_wgpustate.rstate->setTargetTextures(nextTexture, depthTexture.view);
        g_wgpustate.rstate->executeRenderpass([](WGPURenderPassEncoder renderPass){
            wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);
        });
        #ifndef __EMSCRIPTEN__
        wgpuSurfacePresent(g_wgpustate.surface);
        #endif // __EMSCRIPTEN__

        //sample->surface.Present();
    };
    #ifndef __EMSCRIPTEN__

    uint64_t frames = 0;
    uint64_t stmp = nanoTime();
    while(!glfwWindowShouldClose(window)){
        mainloop(nullptr);
        
        ++frames;
        uint64_t nextStmp = nanoTime();
        if(nextStmp - stmp > 1000000000){
            std::cout << frames << " fps" << std::endl;
            frames = 0;
            stmp = nanoTime();
        }
        
    }
    #else
    emscripten_set_main_loop_arg(mainloop, nullptr, 0, true);
    #endif // __EMSCRIPTEN__
}