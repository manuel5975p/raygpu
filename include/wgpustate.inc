#include <vector>
#include <mutex>
#include <unordered_map>

struct wgpustate{
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    
    WGPUSurface surface;
    uint32_t width, height;

    WGPUTextureFormat frameBufferFormat;
    draw_mode current_drawmode;
    full_renderstate* rstate;
    
    std::vector<DescribedBuffer> smallBufferPool;
    std::vector<DescribedBuffer> smallBufferRecyclingBin;

    std::unordered_map<uint64_t, WGPUBindGroup> bindGroupPool;
    std::unordered_map<uint64_t, WGPUBindGroup> bindGroupRecyclingBin;

    DescribedBuffer quadindicesCache{};

    Texture whitePixel;

    WGPUSurfaceTexture currentSurfaceTexture;
    WGPUTextureView currentSurfaceTextureView;
    std::vector<uint8_t> keydownPrevious = std::vector<uint8_t>(512, 0);
    std::vector<uint8_t> keydown = std::vector<uint8_t>(512, 0);
    
    int windowFlags = 0;
    // Frame timing / FPS
    int targetFPS;
    uint64_t total_frames = 0;
    uint64_t init_timestamp;

    int64_t last_timestamps[64] = {0};

    std::mutex drawmutex;
};