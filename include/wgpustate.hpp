#ifndef WGPUSTATE_HPP
#define WGPUSTATE_HPP

#include <vector>
#include <mutex>
#include <webgpu/webgpu.h>
#include <raygpu.h>
struct wgpustate{
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    
    WGPUSurface surface;
    uint32_t width, height;

    WGPUTextureFormat frameBufferFormat;
    std::vector<vertex> current_vertices;
    draw_mode current_drawmode;
    full_renderstate* rstate;
    Vector2 nextuv;
    Vector4 nextcol;

    Texture whitePixel;
    Texture activeTexture;

    WGPUSurfaceTexture currentSurfaceTexture;
    WGPUTextureView currentSurfaceTextureView;

    // Frame timing / FPS
    uint64_t total_frames = 0;
    uint64_t init_timestamp;

    uint64_t last_timestamps[32];

    std::mutex drawmutex;
};


#endif