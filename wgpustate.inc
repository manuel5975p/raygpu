struct wgpustate{
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
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
    WGPUBuffer dummyStorageBuffer;
};