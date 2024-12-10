#include <vector>
#include <mutex>

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
    

    Texture whitePixel;

    WGPUSurfaceTexture currentSurfaceTexture;
    WGPUTextureView currentSurfaceTextureView;

    // Frame timing / FPS
    uint64_t total_frames = 0;
    uint64_t init_timestamp;

    int64_t last_timestamps[32] = {0};

    std::mutex drawmutex;
};