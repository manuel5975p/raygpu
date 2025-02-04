struct renderstate{


    SurfacePresentMode unthrottled_PresentMode;
    SurfacePresentMode throttled_PresentMode;

    GLFWwindow* window;
    uint32_t width, height;

    WGPUTextureFormat frameBufferFormat;

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
    std::array<std::pair<Matrix, WGPUBuffer>, 16> matrixStack;

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