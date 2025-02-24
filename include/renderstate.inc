#include <set>
#include <map>
#include <array>
#include <deque>
#include <mutex>
#include <vector>
struct PenInputState{
    float axes[16];
    Vector2 position;
};

struct window_input_state{
    
    Rectangle windowPosition; // Recovery after fullscreen
    std::vector<uint8_t> keydownPrevious = std::vector<uint8_t>(512, 0);
    std::vector<uint8_t> keydown = std::vector<uint8_t>(512, 0);
    Vector2 scrollThisFrame, scrollPreviousFrame;
    float gestureZoomThisFrame = 1.0f;
    float gestureAngleThisFrame;
    Vector2 mousePosPrevious;
    Vector2 mousePos;
    int cursorInWindow;
    std::array<uint8_t, 16> mouseButtonDownPrevious;
    std::array<uint8_t, 16> mouseButtonDown;
    
    std::vector<std::pair<int64_t, Vector2>> touchPoints; //Map from SDL_FingerID to Vector2
    std::deque<int> charQueue;

    std::unordered_map<unsigned int, PenInputState> penStates;
};

struct renderstate{


    PresentMode unthrottled_PresentMode;
    PresentMode throttled_PresentMode;

    GLFWwindow* window;
    uint32_t width, height;

    PixelFormat frameBufferFormat;

    DescribedPipeline* defaultPipeline;
    DescribedPipeline* activePipeline;

    DescribedRenderpass clearPass;
    DescribedRenderpass renderpass;
    DescribedComputepass computepass;
    DescribedRenderpass* activeRenderpass;
    DescribedComputepass* activeComputepass;
    
    uint32_t renderExtentX; // Dimensions of the current viewport
    uint32_t renderExtentY; // Required for camera function

    std::vector<DescribedBuffer*> smallBufferPool;
    std::vector<DescribedBuffer*> smallBufferRecyclingBin;

    //std::unordered_map<uint64_t, WGPUBindGroup> bindGroupPool;
    //std::unordered_map<uint64_t, WGPUBindGroup> bindGroupRecyclingBin;

    DescribedBuffer* identityMatrix;
    DescribedSampler defaultSampler;
    
    DescribedBuffer* quadindicesCache{};

    Texture whitePixel;

    int stackPosition = 0;
    std::array<std::pair<Matrix, WGVKBuffer>, 16> matrixStack;

    int renderTargetStackPosition = -1;
    std::array<RenderTexture, 8> renderTargetStack;

    bool wantsToggleFullscreen;
    
    

    RenderTexture mainWindowRenderTarget;
    //RenderTexture currentDefaultRenderTarget;
    
    std::unordered_map<void*, window_input_state> input_map;
    
    int windowFlags = 0;
    // Frame timing / FPS
    int targetFPS;
    uint64_t total_frames = 0;
    uint64_t init_timestamp;

    int64_t last_timestamps[64] = {0};

    std::mutex drawmutex;
    GIFRecordState* grst;

    SubWindow* mainWindow{};
    std::map<void*, SubWindow> createdSubwindows;
    SubWindow activeSubWindow{};

    bool closeFlag = false;
};