#include <vector>
#include <mutex>
#include <unordered_map>
#include <deque>
#include <array>
struct window_input_state{
    Rectangle windowPosition; // Recovery after fullscreen
    std::vector<uint8_t> keydownPrevious = std::vector<uint8_t>(512, 0);
    std::vector<uint8_t> keydown = std::vector<uint8_t>(512, 0);
    Vector2 scrollThisFrame, scrollPreviousFrame;
    Vector2 mousePosPrevious;
    Vector2 mousePos;
    int cursorInWindow;
    std::array<uint8_t, 16> mouseButtonDownPrevious;
    std::array<uint8_t, 16> mouseButtonDown;

    std::deque<int> charQueue;
};
struct wgpustate{
    wgpu::Instance instance;
    wgpu::Adapter adapter;
    wgpu::Device device;
    wgpu::Queue queue;

    wgpu::PresentMode unthrottled_PresentMode;
    wgpu::PresentMode throttled_PresentMode;

    GLFWwindow* window;
    wgpu::Surface surface;
    uint32_t width, height;

    WGPUTextureFormat frameBufferFormat;
    full_renderstate* rstate;
    
    std::vector<DescribedBuffer*> smallBufferPool;
    std::vector<DescribedBuffer*> smallBufferRecyclingBin;

    std::unordered_map<uint64_t, WGPUBindGroup> bindGroupPool;
    std::unordered_map<uint64_t, WGPUBindGroup> bindGroupRecyclingBin;

    DescribedBuffer* identityMatrix;
    DescribedSampler defaultSampler;
    
    DescribedBuffer* quadindicesCache{};

    Texture whitePixel;

    int stackPosition = 0;
    std::array<std::pair<Matrix, WGPUBuffer>, 16> matrixStack;

    int renderTargetStackPosition = -1;
    std::array<RenderTexture> renderTargetStack;

    //RenderTexture mainWindowRenderTarget;
    //RenderTexture currentDefaultRenderTarget;
    
    std::unordered_map<GLFWwindow*, window_input_state> input_map;
    
    int windowFlags = 0;
    // Frame timing / FPS
    int targetFPS;
    uint64_t total_frames = 0;
    uint64_t init_timestamp;

    int64_t last_timestamps[64] = {0};

    std::mutex drawmutex;
    GIFRecordState* grst;

    std::unordered_map<void*, SubWindow> createdSubwindows;
    SubWindow activeSubWindow{};
};