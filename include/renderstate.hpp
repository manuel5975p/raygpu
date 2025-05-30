#include <set>
#include <map>
#include <array>
#include <deque>
#include <mutex>
#include <vector>
#include <raygpu.h>
#if SUPPORT_WGPU_BACKEND == 1
    #include <webgpu/webgpu.h>
#else
    #include <wgvk.h>
#endif
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
template <typename T, size_t N>
struct array_stack{
    std::array<T, N> data;
    using iterator = std::array<T, N>::iterator;
    using const_iterator = std::array<T, N>::const_iterator;
    //iterator current_pos;
    uint32_t current_pos;
    array_stack() noexcept : current_pos(0){}

    void push(T d)noexcept{
        rassert(current_pos < N, "Stack is full");
        data[current_pos++] = d;
    }
    T pop()noexcept{
        rassert(current_pos > 0, "Stack is empty");
        T ret = peek();
        --current_pos;
        return ret;
    }
    T& peek()noexcept{
        rassert(!empty(), "Stack is empty");
        return data[current_pos - 1];
    }
    const T& peek()const noexcept{
        rassert(!empty(), "Stack is empty");
        return data[current_pos - 1];
    }
    size_t size() const noexcept{
        return current_pos;
    }
    bool empty()const noexcept{
        return current_pos == 0;
    }
};

struct renderstate{


    WGPUPresentMode unthrottled_PresentMode;
    WGPUPresentMode throttled_PresentMode;
 
    GLFWwindow* window;
    uint32_t width, height;

    PixelFormat frameBufferFormat;

    DescribedPipeline* defaultPipeline;
    Shader defaultShader;
    RenderSettings currentSettings;
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

    
    array_stack<std::pair<Matrix, WGPUBuffer>, 16> matrixStack;

    array_stack<RenderTexture, 8> renderTargetStack;

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