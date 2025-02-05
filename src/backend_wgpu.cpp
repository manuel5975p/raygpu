#include <raygpu.h>
#include <wgpustate.inc>
#include <unordered_set>
extern "C" void UnloadTexture(Texture tex){
    for(uint32_t i = 0;i < tex.mipmaps;i++){
        if(tex.mipViews[i]){
            wgpuTextureViewRelease((WGPUTextureView)tex.mipViews[i]);
            tex.mipViews[i] = nullptr;
        }
    }
    if(tex.view){
        wgpuTextureViewRelease((WGPUTextureView)tex.view);
        tex.view = nullptr;
    }
    if(tex.id){
        wgpuTextureRelease((WGPUTexture)tex.id);
        tex.id = nullptr;
    }
}
void PresentSurface(FullSurface* fsurface){
    wgpuSurfacePresent((WGPUSurface)fsurface->surface);
}
extern "C" void BindPipeline(DescribedPipeline* pipeline, WGPUPrimitiveTopology drawMode){
    switch(drawMode){
        case WGPUPrimitiveTopology_TriangleList:
        //std::cout << "Binding: " <<  pipeline->pipeline << "\n";
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_TriangleList);
        break;
        case WGPUPrimitiveTopology_TriangleStrip:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_TriangleStrip);
        break;
        case WGPUPrimitiveTopology_LineList:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_LineList);
        break;
        case WGPUPrimitiveTopology_PointList:
        wgpuRenderPassEncoderSetPipeline ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, (WGPURenderPipeline)pipeline->quartet.pipeline_PointList);
        break;
        default:
            assert(false && "Unsupported Drawmode");
            abort();
    }
    //pipeline->lastUsedAs = drawMode;
    wgpuRenderPassEncoderSetBindGroup ((WGPURenderPassEncoder)g_renderstate.renderpass.rpEncoder, 0, (WGPUBindGroup)UpdateAndGetNativeBindGroup(&pipeline->bindGroup), 0, 0);
}

extern "C" void UpdateBindGroup(DescribedBindGroup* bg){
    //std::cout << "Updating bindgroup with " << bg->desc.entryCount << " entries" << std::endl;
    //std::cout << "Updating bindgroup with " << bg->desc.entries[1].binding << " entries" << std::endl;
    if(bg->needsUpdate){
        WGPUBindGroupDescriptor desc zeroinit;
        std::vector<WGPUBindGroupEntry> aswgpu(bg->entryCount);
        for(uint32_t i = 0;i < bg->entryCount;i++){
            aswgpu[i].binding = bg->entries[i].binding;
            aswgpu[i].buffer = (WGPUBuffer)bg->entries[i].buffer;
            aswgpu[i].offset = bg->entries[i].offset;
            aswgpu[i].size = bg->entries[i].size;
            aswgpu[i].sampler = (WGPUSampler)bg->entries[i].sampler;
            aswgpu[i].textureView = (WGPUTextureView)bg->entries[i].textureView;
        }
        desc.entries = aswgpu.data();
        desc.entryCount = aswgpu.size();
        desc.layout = (WGPUBindGroupLayout)bg->layout->layout;
        bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &desc);
        bg->needsUpdate = false;
    }
}

inline uint64_t bgEntryHash(const ResourceDescriptor& bge){
    const uint32_t rotation = (bge.binding * 7) & 63;
    uint64_t value = ROT_BYTES((uint64_t)bge.buffer, rotation);
    value ^= ROT_BYTES((uint64_t)bge.textureView, rotation);
    value ^= ROT_BYTES((uint64_t)bge.sampler, rotation);
    value ^= ROT_BYTES((uint64_t)bge.offset, rotation);
    value ^= ROT_BYTES((uint64_t)bge.size, rotation);
    return value;
}


extern "C" void UpdateBindGroupEntry(DescribedBindGroup* bg, size_t index, ResourceDescriptor entry){
    if(index >= bg->entryCount){
        TRACELOG(LOG_WARNING, "Trying to set entry %d on a BindGroup with only %d entries", (int)index, (int)bg->entryCount);
        //return;
    }
    auto& newpuffer = entry.buffer;
    auto& newtexture = entry.textureView;
    if(newtexture && bg->entries[index].textureView == newtexture){
        //return;
    }
    uint64_t oldHash = bg->descriptorHash;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);
    //bool donotcache = false;
    if(bg->releaseOnClear & (1 << index)){
        //donotcache = true;
        if(bg->entries[index].buffer){
            wgpuBufferRelease((WGPUBuffer)bg->entries[index].buffer);
        }
        else if(bg->entries[index].textureView){
            //Todo: currently not the case anyway, but this is nadinöf
            wgpuTextureViewRelease((WGPUTextureView)bg->entries[index].textureView);
        }
        else if(bg->entries[index].sampler){
            wgpuSamplerRelease((WGPUSampler)bg->entries[index].sampler);
        }
        bg->releaseOnClear &= ~(1 << index);
    }
    bg->entries[index] = entry;
    bg->descriptorHash ^= bgEntryHash(bg->entries[index]);

    //TODO don't release and recreate here or find something better
    if(true /*|| donotcache*/){
        if(bg->bindGroup)
            wgpuBindGroupRelease((WGPUBindGroup)bg->bindGroup);
        bg->bindGroup = nullptr;
    }
    //else if(!bg->needsUpdate && bg->bindGroup){
    //    g_wgpustate.bindGroupPool[oldHash] = bg->bindGroup;
    //    bg->bindGroup = nullptr;
    //}
    bg->needsUpdate = true;
    
    //bg->bindGroup = wgpuDeviceCreateBindGroup((WGPUDevice)GetDevice(), &(bg->desc));
}


extern "C" void GetNewTexture(FullSurface* fsurface){
    if(fsurface->surface == 0){
        return;
    }
    else{
        WGPUSurfaceTexture surfaceTexture;
        wgpuSurfaceGetCurrentTexture((WGPUSurface)fsurface->surface, &surfaceTexture);
        fsurface->renderTarget.texture.id = surfaceTexture.texture;
        fsurface->renderTarget.texture.width = wgpuTextureGetWidth(surfaceTexture.texture);
        fsurface->renderTarget.texture.height = wgpuTextureGetHeight(surfaceTexture.texture);
        fsurface->renderTarget.texture.view = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
    }
}
std::optional<wgpu::Limits> limitsToBeRequested = std::nullopt;
WGPUBackendType requestedBackend = DEFAULT_BACKEND;
WGPUAdapterType requestedAdapterType = WGPUAdapterType_Unknown;
void setlimit(wgpu::Limits& limits, LimitType limit, uint64_t value){
    switch(limit){
        case maxTextureDimension1D:limits.maxTextureDimension1D = value;break;
        case maxTextureDimension2D:limits.maxTextureDimension2D = value;break;
        case maxTextureDimension3D:limits.maxTextureDimension3D = value;break;
        case maxTextureArrayLayers:limits.maxTextureArrayLayers = value;break;
        case maxBindGroups:limits.maxBindGroups = value;break;
        case maxBindGroupsPlusVertexBuffers:limits.maxBindGroupsPlusVertexBuffers = value;break;
        case maxBindingsPerBindGroup:limits.maxBindingsPerBindGroup = value;break;
        case maxDynamicUniformBuffersPerPipelineLayout:limits.maxDynamicUniformBuffersPerPipelineLayout = value;break;
        case maxDynamicStorageBuffersPerPipelineLayout:limits.maxDynamicStorageBuffersPerPipelineLayout = value;break;
        case maxSampledTexturesPerShaderStage:limits.maxSampledTexturesPerShaderStage = value;break;
        case maxSamplersPerShaderStage:limits.maxSamplersPerShaderStage = value;break;
        case maxStorageBuffersPerShaderStage:limits.maxStorageBuffersPerShaderStage = value;break;
        case maxStorageTexturesPerShaderStage:limits.maxStorageTexturesPerShaderStage = value;break;
        case maxUniformBuffersPerShaderStage:limits.maxUniformBuffersPerShaderStage = value;break;
        case maxUniformBufferBindingSize:limits.maxUniformBufferBindingSize = value;break;
        case maxStorageBufferBindingSize:limits.maxStorageBufferBindingSize = value;break;
        case minUniformBufferOffsetAlignment:limits.minUniformBufferOffsetAlignment = value;break;
        case minStorageBufferOffsetAlignment:limits.minStorageBufferOffsetAlignment = value;break;
        case maxVertexBuffers:limits.maxVertexBuffers = value;break;
        case maxBufferSize:limits.maxBufferSize = value;break;
        case maxVertexAttributes:limits.maxVertexAttributes = value;break;
        case maxVertexBufferArrayStride:limits.maxVertexBufferArrayStride = value;break;
        case maxInterStageShaderComponents:limits.maxInterStageShaderComponents = value;break;
        case maxInterStageShaderVariables:limits.maxInterStageShaderVariables = value;break;
        case maxColorAttachments:limits.maxColorAttachments = value;break;
        case maxColorAttachmentBytesPerSample:limits.maxColorAttachmentBytesPerSample = value;break;
        case maxComputeWorkgroupStorageSize:limits.maxComputeWorkgroupStorageSize = value;break;
        case maxComputeInvocationsPerWorkgroup:limits.maxComputeInvocationsPerWorkgroup = value;break;
        case maxComputeWorkgroupSizeX:limits.maxComputeWorkgroupSizeX = value;break;
        case maxComputeWorkgroupSizeY:limits.maxComputeWorkgroupSizeY = value;break;
        case maxComputeWorkgroupSizeZ:limits.maxComputeWorkgroupSizeZ = value;break;
        case maxComputeWorkgroupsPerDimension:limits.maxComputeWorkgroupsPerDimension = value;break;
        case maxStorageBuffersInVertexStage:limits.maxStorageBuffersInVertexStage = value;break;
        case maxStorageTexturesInVertexStage:limits.maxStorageTexturesInVertexStage = value;break;
        case maxStorageBuffersInFragmentStage:limits.maxStorageBuffersInFragmentStage = value;break;
        case maxStorageTexturesInFragmentStage:limits.maxStorageTexturesInFragmentStage = value;break;
    }
}
extern "C" void RequestLimit(LimitType limit, uint64_t value){
    if(!limitsToBeRequested.has_value()){
        limitsToBeRequested = wgpu::Limits();
    }
    setlimit(limitsToBeRequested.value(), limit, value);
}
extern "C" void RequestAdapterType(WGPUAdapterType type){
    requestedAdapterType = type;
}
extern "C" void RequestBackend(WGPUBackendType backend){
    requestedBackend = backend;
}
void InitBackend(){
    wgpustate* sample = &g_wgpustate;
    // Create the toggles descriptor if not using emscripten.
    wgpu::ChainedStruct* togglesChain = nullptr;
    wgpu::SType type;
#ifndef __EMSCRIPTEN__
    std::vector<const char*> enableToggleNames{};
    std::vector<const char*> disabledToggleNames{};

    wgpu::DawnTogglesDescriptor toggles = {};
    toggles.enabledToggles = enableToggleNames.data();
    toggles.enabledToggleCount = enableToggleNames.size();
    toggles.disabledToggles = disabledToggleNames.data();
    toggles.disabledToggleCount = disabledToggleNames.size();

    togglesChain = &toggles;
#endif  // __EMSCRIPTEN__

    // Setup base adapter options with toggles.
    wgpu::RequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = togglesChain;
    auto backendType = (wgpu::BackendType)requestedBackend;
    auto adapterType = (wgpu::AdapterType)requestedAdapterType;
    adapterOptions.backendType = backendType;
    if (backendType != wgpu::BackendType::Undefined) {
        auto bcompat = [](wgpu::BackendType backend) {
            switch (backend) {
                case wgpu::BackendType::D3D12:
                case wgpu::BackendType::Metal:
                case wgpu::BackendType::Vulkan:
                case wgpu::BackendType::WebGPU:
                case wgpu::BackendType::Null:
                    return false;
                case wgpu::BackendType::D3D11:
                case wgpu::BackendType::OpenGL:
                case wgpu::BackendType::OpenGLES:
                    return true;
                case wgpu::BackendType::Undefined:
                default:
                    __builtin_unreachable();
                    //return false;
            }
        };
        adapterOptions.featureLevel = bcompat(backendType) ?  wgpu::FeatureLevel::Compatibility : wgpu::FeatureLevel::Core;

    }

    switch (adapterType) {
        case wgpu::AdapterType::CPU:
            adapterOptions.forceFallbackAdapter = true;
            break;
        case wgpu::AdapterType::DiscreteGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::IntegratedGPU:
            adapterOptions.powerPreference = wgpu::PowerPreference::HighPerformance;
            break;
        case wgpu::AdapterType::Unknown:
            break;
    }

#ifndef __EMSCRIPTEN__
    //dawnProcSetProcs(&dawn::native::GetProcs());

    // Create the instance with the toggles
    wgpu::InstanceDescriptor instanceDescriptor = {};
    instanceDescriptor.nextInChain = togglesChain;
    instanceDescriptor.features.timedWaitAnyEnable = true;
    
    sample->instance = wgpu::CreateInstance(&instanceDescriptor);
#else
    // Create the instance
    sample->instance = wgpu::CreateInstance(nullptr);
#endif  // __EMSCRIPTEN__

    //wgpu::WGSLFeatureName wgslfeatures[8];
    //sample->instance.EnumerateWGSLLanguageFeatures(wgslfeatures);
    //std::cout << sample->instance.HasWGSLLanguageFeature(wgpu::WGSLFeatureName::ReadonlyAndReadwriteStorageTextures);
    //exit(0);
    // Synchronously create the adapter
    sample->instance.WaitAny(
        sample->instance.RequestAdapter(
            &adapterOptions, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter, wgpu::StringView message) {
                if (status != wgpu::RequestAdapterStatus::Success) {
                    char tmp[2048] = {};
                    std::memcpy(tmp, message.data, std::min(message.length, (size_t)2047));
                    TRACELOG(LOG_FATAL, "Failed to get an adapter: %s\n", tmp);
                    return;
                }
                sample->adapter = std::move(adapter);
            }),
        UINT64_MAX);
    if (sample->adapter == nullptr) {
        std::cerr << "Adapter is null\n";
        abort();
    }
    wgpu::AdapterInfo info;
    sample->adapter.GetInfo(&info);
    
    std::vector<char> deviceNameCstr(info.device.length + 1, '\0');
    std::vector<char> architectureCstr(info.architecture.length + 1, '\0');
    std::vector<char> deviceDescCstr(info.description.length + 1, '\0');
    std::vector<char> vendorC2str(info.vendor.length + 1, '\0');

    memcpy(deviceNameCstr.data(), info.device.data, info.device.length);
    memcpy(deviceDescCstr.data(), info.description.data, info.description.length);
    memcpy(architectureCstr.data(), info.architecture.data, info.architecture.length);
    memcpy(vendorC2str.data(), info.vendor.data, info.vendor.length);
    
    const char* adapterTypeString = info.adapterType == wgpu::AdapterType::CPU ? "CPU" : (info.adapterType == wgpu::AdapterType::IntegratedGPU ? "Integrated GPU" : "Dedicated GPU");
    const char* backendString = backendTypeSpellingTable.at((WGPUBackendType)info.backendType).c_str();

    TRACELOG(LOG_INFO, "Using adapter %s %s", vendorC2str.data(), deviceNameCstr.data());
    TRACELOG(LOG_INFO, "Adapter description: %s", deviceDescCstr.data());
    TRACELOG(LOG_INFO, "Adapter architecture: %s", architectureCstr.data());
    TRACELOG(LOG_INFO, "%s renderer running on %s", backendString, adapterTypeString);
    // Create device descriptor with callbacks and toggles
    {
        wgpu::SupportedFeatures features;
        sample->adapter.GetFeatures(&features);
        std::string featuresString;
        for(size_t i = 0; i < features.featureCount;i++){
            featuresString += featureSpellingTable.contains((WGPUFeatureName)features.features[i]) ? featureSpellingTable.at((WGPUFeatureName)features.features[i]) : "<unknown feature>";
            if(i < features.featureCount - 1)featuresString += ", ";
        }
        TRACELOG(LOG_INFO, "Features supported: %s ", featuresString.c_str());
    }
    wgpu::SupportedLimits adapterLimits;
    sample->adapter.GetLimits(&adapterLimits);
    
    {
        TraceLog(LOG_INFO, "Platform could support %u bindings per bindgroup",    (unsigned)adapterLimits.limits.maxBindingsPerBindGroup);
        TraceLog(LOG_INFO, "Platform could support %u bindgroups",                (unsigned)adapterLimits.limits.maxBindGroups);
        TraceLog(LOG_INFO, "Platform could support buffers up to %llu megabytes", (unsigned long long)adapterLimits.limits.maxBufferSize / (1000000ull));
        TraceLog(LOG_INFO, "Platform could support textures up to %u x %u",       (unsigned)adapterLimits.limits.maxTextureDimension2D, (unsigned)adapterLimits.limits.maxTextureDimension2D);
        TraceLog(LOG_INFO, "Platform could support %u VBO slots",                 (unsigned)adapterLimits.limits.maxVertexBuffers);
    }
    
    wgpu::DeviceDescriptor deviceDesc = {};
    #ifndef __EMSCRIPTEN__ //y tho
    wgpu::FeatureName fnames[2] = {
        wgpu::FeatureName::ClipDistances,
        wgpu::FeatureName::Float32Filterable,
    };
    deviceDesc.requiredFeatures = fnames;
    deviceDesc.requiredFeatureCount = 2;
    #endif
    deviceDesc.nextInChain = togglesChain;
    deviceDesc.SetDeviceLostCallback(
        wgpu::CallbackMode::AllowSpontaneous,
        [](const wgpu::Device&, wgpu::DeviceLostReason reason, wgpu::StringView message) {
            const char* reasonName = "";
            switch (reason) {
                case wgpu::DeviceLostReason::Unknown:
                    reasonName = "Unknown";
                    break;
                case wgpu::DeviceLostReason::Destroyed:
                    reasonName = "Destroyed";
                    break;
                case wgpu::DeviceLostReason::InstanceDropped:
                    reasonName = "InstanceDropped";
                    break;
                case wgpu::DeviceLostReason::FailedCreation:
                    reasonName = "FailedCreation";
                    break;
                default:
                    __builtin_unreachable();
            }
            std::cerr << "Device lost because of " << reasonName << ": " << std::string(message.data, message.length) << std::endl;
        });
    deviceDesc.SetUncapturedErrorCallback(
        [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
            const char* errorTypeName = "";
            switch (type) {
                case wgpu::ErrorType::Validation:
                    errorTypeName = "Validation";
                    break;
                case wgpu::ErrorType::OutOfMemory:
                    errorTypeName = "Out of memory";
                    break;
                case wgpu::ErrorType::Unknown:
                    errorTypeName = "Unknown";
                    break;
                case wgpu::ErrorType::DeviceLost:
                    errorTypeName = "Device lost";
                    break;
                default:
                    __builtin_unreachable();
            }
            std::cerr << errorTypeName << " error: " << std::string(message.data, message.length);
            __builtin_trap();
        });

    
    if(!limitsToBeRequested.has_value()){
        limitsToBeRequested = wgpu::Limits();
    }
    
    
    // Synchronously create the device
    wgpu::RequiredLimits reqLimits;



    if(limitsToBeRequested.has_value()){
        limitsToBeRequested->maxStorageBuffersInVertexStage = adapterLimits.limits.maxStorageBuffersInVertexStage;
        limitsToBeRequested->maxStorageBuffersInFragmentStage = adapterLimits.limits.maxStorageBuffersInFragmentStage;
        limitsToBeRequested->maxStorageBuffersPerShaderStage = adapterLimits.limits.maxStorageBuffersPerShaderStage;

        reqLimits.limits = limitsToBeRequested.value();
        deviceDesc.requiredLimits = &reqLimits;
    }
    else{
        deviceDesc.requiredLimits = nullptr;
    }
    sample->instance.WaitAny(
        sample->adapter.RequestDevice(
            &deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
            [sample](wgpu::RequestDeviceStatus status, wgpu::Device device, wgpu::StringView message) {
                if (status != wgpu::RequestDeviceStatus::Success) {
                    std::cerr << "Failed to get an device: " << std::string(message.data, message.length);
                    return;
                }
                sample->device = std::move(device);
                sample->queue = sample->device.GetQueue();
            }),

        UINT64_MAX);
    if (sample->device == nullptr) {
        TRACELOG(LOG_FATAL, "Device is null");
    }
    wgpu::SupportedLimits slimits;
    
    sample->device.GetLimits(&slimits);
    TraceLog(LOG_INFO, "Device supports %u bindings per bindgroup", (unsigned)slimits.limits.maxBindingsPerBindGroup);
    TraceLog(LOG_INFO, "Device supports %u bindgroups", (unsigned)slimits.limits.maxBindGroups);
    TraceLog(LOG_INFO, "Device supports buffers up to %llu megabytes", (unsigned long long)slimits.limits.maxBufferSize / (1000000ull));
    TraceLog(LOG_INFO, "Device supports textures up to %u x %u", (unsigned)slimits.limits.maxTextureDimension2D, (unsigned)slimits.limits.maxTextureDimension2D);
    TraceLog(LOG_INFO, "Device supports %u VBO slots", (unsigned)slimits.limits.maxVertexBuffers);

}


bool negotiateSurfaceFormatAndPresentMode_called = false;
void negotiateSurfaceFormatAndPresentMode(const void* SurfaceHandle){
    const wgpu::Surface& surf = *reinterpret_cast<const wgpu::Surface*>(SurfaceHandle);
    if(negotiateSurfaceFormatAndPresentMode_called)return;
    negotiateSurfaceFormatAndPresentMode_called = true;
    wgpu::SurfaceCapabilities capabilities;
    surf.GetCapabilities(GetCXXAdapter(), &capabilities);
    {
        std::string presentModeString;
        for(uint32_t i = 0;i < capabilities.presentModeCount;i++){
            presentModeString += presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[i]).c_str();
            if(i < capabilities.presentModeCount - 1){
                presentModeString += ", ";
            }
        }
        TRACELOG(LOG_INFO, "Supported present modes: %s", presentModeString.c_str());
    }
    if(capabilities.presentModeCount == 0){
        TRACELOG(LOG_ERROR, "No presentation modes supported! This surface is most likely invalid");
    }
    else if(capabilities.presentModeCount == 1){
        TRACELOG(LOG_INFO, "Only %s supported", presentModeSpellingTable.at((WGPUPresentMode)capabilities.presentModes[0]).c_str());
        g_renderstate.unthrottled_PresentMode = (SurfacePresentMode)capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode = (SurfacePresentMode)capabilities.presentModes[0];
    }
    else if(capabilities.presentModeCount > 1){
        g_renderstate.unthrottled_PresentMode = (SurfacePresentMode)capabilities.presentModes[0];
        g_renderstate.throttled_PresentMode = (SurfacePresentMode)capabilities.presentModes[0];
        std::unordered_set<wgpu::PresentMode> pmset(capabilities.presentModes, capabilities.presentModes + capabilities.presentModeCount);
        if(pmset.find(wgpu::PresentMode::Fifo) != pmset.end()){
            g_renderstate.throttled_PresentMode = PresentMode_Fifo;
        }
        else if(pmset.find(wgpu::PresentMode::FifoRelaxed) != pmset.end()){
            g_renderstate.throttled_PresentMode = PresentMode_FifoRelaxed;
        }
        if(pmset.find(wgpu::PresentMode::Mailbox) != pmset.end()){
            g_renderstate.unthrottled_PresentMode = PresentMode_Mailbox;
        }
        else if(pmset.find(wgpu::PresentMode::Immediate) != pmset.end()){
            g_renderstate.unthrottled_PresentMode = PresentMode_Immediate;
        }
    }
    
    {
        std::string formatsString;
        for(uint32_t i = 0;i < capabilities.formatCount;i++){
            formatsString += textureFormatSpellingTable.at((WGPUTextureFormat)capabilities.formats[i]).c_str();
            if(i < capabilities.formatCount - 1){
                formatsString += ", ";
            }
        }
        TRACELOG(LOG_INFO, "Supported surface formats: %s", formatsString.c_str());
    }

    wgpu::TextureFormat selectedFormat = wgpu::TextureFormat::Undefined;
    int format_index = 0;
    for(format_index = 0;format_index < capabilities.formatCount;format_index++){
        if(capabilities.formats[format_index] == wgpu::TextureFormat::RGBA16Float){
            selectedFormat = (capabilities.formats[format_index]);
            break;
        }
        if(capabilities.formats[format_index] == wgpu::TextureFormat::BGRA8Unorm ||
           capabilities.formats[format_index] == wgpu::TextureFormat::RGBA8Unorm){
            selectedFormat = (capabilities.formats[format_index]);
            break;
        }
    }
    g_renderstate.frameBufferFormat = (PixelFormat)selectedFormat;
    if(format_index == capabilities.formatCount){
        TRACELOG(LOG_WARNING, "No RGBA8 / BGRA8 Unorm framebuffer format found, colors might be off"); 
        g_renderstate.frameBufferFormat = (PixelFormat)selectedFormat;
    }
    
    TRACELOG(LOG_INFO, "Selected surface format %s", textureFormatSpellingTable.at((WGPUTextureFormat)g_renderstate.frameBufferFormat).c_str());
    
    //TRACELOG(LOG_INFO, "Selected present mode %s", presentModeSpellingTable.at((WGPUPresentMode)g_wgpustate.presentMode).c_str());
}

extern "C" DescribedBuffer* GenBufferEx(const void* data, size_t size, WGPUBufferUsage usage){
    DescribedBuffer* ret = callocnew(DescribedBuffer);
    WGPUBufferDescriptor descriptor{};
    descriptor.size = size;
    descriptor.mappedAtCreation = false;
    descriptor.usage = usage;
    ret->buffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &descriptor);
    ret->size = size;
    ret->usage = usage;
    if(data != nullptr){
        wgpuQueueWriteBuffer(GetQueue(), (WGPUBuffer)ret->buffer, 0, data, size);
    }
    return ret;
}
void* GetInstance(){
    return g_wgpustate.instance.Get();
}
void* GetDevice(){
    return g_wgpustate.device.Get();
}
void* GetAdapter(){
    return g_wgpustate.adapter.Get();
}

extern "C" Texture LoadTexturePro(uint32_t width, uint32_t height, PixelFormat format, TextureUsage usage, uint32_t sampleCount, uint32_t mipmaps){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = mipmaps;
    tDesc.sampleCount = sampleCount;
    tDesc.format = (WGPUTextureFormat)format;
    tDesc.usage  = usage;
    tDesc.viewFormatCount = 1;
    tDesc.viewFormats = &tDesc.format;

    WGPUTextureViewDescriptor textureViewDesc{};
    char potlabel[128]; 
    if(format == Depth24){
        int len = snprintf(potlabel, 128, "Depftex %d x %d", width, height);
        textureViewDesc.label.data = potlabel;
        textureViewDesc.label.length = len;
    }
    textureViewDesc.usage = usage;
    textureViewDesc.aspect = ((format == Depth24 || format == Depth32) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = mipmaps;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    Texture ret zeroinit;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &tDesc);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &textureViewDesc);
    ret.format = format;
    ret.width = width;
    ret.height = height;
    ret.sampleCount = sampleCount;
    ret.mipmaps = mipmaps;
    if(mipmaps > 1){
        for(uint32_t i = 0;i < mipmaps;i++){
            textureViewDesc.baseMipLevel = i;
            textureViewDesc.mipLevelCount = 1;
            ret.mipViews[i] = wgpuTextureCreateView((WGPUTexture)ret.id, &textureViewDesc);
        }
    }
    return ret;
}
DescribedSampler LoadSamplerEx(addressMode amode, filterMode fmode, filterMode mipmapFilter, float maxAnisotropy){
    DescribedSampler ret zeroinit;
    ret.magFilter    = fmode;
    ret.minFilter    = fmode;
    ret.mipmapFilter = fmode;
    ret.compare      = CompareFunction_Undefined;
    ret.lodMinClamp  = 0.0f;
    ret.lodMaxClamp  = 10.0f;
    ret.maxAnisotropy = maxAnisotropy;
    ret.addressModeU = amode;
    ret.addressModeV = amode;
    ret.addressModeW = amode;

    WGPUSamplerDescriptor sdesc{};
    sdesc.magFilter    = (WGPUFilterMode)fmode;
    sdesc.minFilter    = (WGPUFilterMode)fmode;
    sdesc.mipmapFilter = (WGPUMipmapFilterMode)fmode;
    sdesc.compare      = WGPUCompareFunction_Undefined;
    sdesc.lodMinClamp  = 0.0f;
    sdesc.lodMaxClamp  = 10.0f;
    sdesc.maxAnisotropy = maxAnisotropy;
    sdesc.addressModeU = (WGPUAddressMode)amode;
    sdesc.addressModeV = (WGPUAddressMode)amode;
    sdesc.addressModeW = (WGPUAddressMode)amode;

    ret.sampler = wgpuDeviceCreateSampler((WGPUDevice)GetDevice(), &sdesc);
    return ret;
}
void SetBindgroupUniformBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
    bufferDesc.mappedAtCreation = false;
    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer(GetQueue(), uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
}

void SetBindgroupStorageBufferData (DescribedBindGroup* bg, uint32_t index, const void* data, size_t size){
    ResourceDescriptor entry{};
    WGPUBufferDescriptor bufferDesc{};

    bufferDesc.size = size;
    bufferDesc.usage = WGPUBufferUsage_CopySrc | WGPUBufferUsage_CopyDst | WGPUBufferUsage_Storage;
    bufferDesc.mappedAtCreation = false;

    WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer((WGPUDevice)GetDevice(), &bufferDesc);
    wgpuQueueWriteBuffer(GetQueue(), uniformBuffer, 0, data, size);
    entry.binding = index;
    entry.buffer = uniformBuffer;
    entry.size = size;
    UpdateBindGroupEntry(bg, index, entry);
    bg->releaseOnClear |= (1 << index);
}
void UnloadBuffer(DescribedBuffer* buffer){
    wgpuBufferRelease((WGPUBuffer)buffer->buffer);
    free(buffer);
}
Texture LoadTextureFromImage(Image img){
    Texture ret zeroinit;
    ret.sampleCount = 1;
    Color* altdata = nullptr;
    if(img.format == GRAYSCALE){
        altdata = (Color*)calloc(img.width * img.height, sizeof(Color));
        for(size_t i = 0;i < img.width * img.height;i++){
            uint16_t gscv = ((uint16_t*)img.data)[i];
            ((Color*)altdata)[i].r = gscv & 255;
            ((Color*)altdata)[i].g = gscv & 255;
            ((Color*)altdata)[i].b = gscv & 255;
            ((Color*)altdata)[i].a = gscv >> 8;
        }
    }
    WGPUTextureDescriptor desc = {
        nullptr,
        WGPUStringView{nullptr, 0},
        WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
        WGPUTextureDimension_2D,
        WGPUExtent3D{img.width, img.height, 1},
        img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format,
        1,1,1,nullptr
    };
    WGPUTextureFormat resulting_tf = img.format == GRAYSCALE ? WGPUTextureFormat_RGBA8Unorm : (WGPUTextureFormat)img.format;
    desc.viewFormats = (WGPUTextureFormat*)&resulting_tf;
    ret.id = wgpuDeviceCreateTexture((WGPUDevice)GetDevice(), &desc);
    WGPUTextureViewDescriptor vdesc{};
    vdesc.arrayLayerCount = 0;
    vdesc.aspect = WGPUTextureAspect_All;
    vdesc.format = desc.format;
    vdesc.dimension = WGPUTextureViewDimension_2D;
    vdesc.baseArrayLayer = 0;
    vdesc.arrayLayerCount = 1;
    vdesc.baseMipLevel = 0;
    vdesc.mipLevelCount = 1;
        
    WGPUImageCopyTexture destination{};
    destination.texture = (WGPUTexture)ret.id;
    destination.mipLevel = 0;
    destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
    destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
    WGPUTextureDataLayout source{};
    source.offset = 0;
    source.bytesPerRow = 4 * img.width;
    source.rowsPerImage = img.height;
    //wgpuQueueWriteTexture()
    wgpuQueueWriteTexture(GetQueue(), &destination, altdata ? altdata : img.data, 4 * img.width * img.height, &source, &desc.size);
    ret.view = wgpuTextureCreateView((WGPUTexture)ret.id, &vdesc);
    ret.width = img.width;
    ret.height = img.height;
    if(altdata)free(altdata);
    return ret;
}

void BeginRenderpassEx(DescribedRenderpass* renderPass){
    WGPUCommandEncoderDescriptor desc{};
    desc.label = STRVIEW("another cmdencoder");
    
    renderPass->cmdEncoder = wgpuDeviceCreateCommandEncoder((WGPUDevice)GetDevice(), &desc);

    WGPURenderPassDescriptor renderPassDesc zeroinit;
    renderPassDesc.colorAttachmentCount = 1;
    
    WGPURenderPassColorAttachment colorAttachment zeroinit;
    WGPURenderPassDepthStencilAttachment depthAttachment zeroinit;
    if(g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].colorMultisample.view){
        colorAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].colorMultisample.view;
        colorAttachment.resolveTarget = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.view;
    }
    else{
        colorAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].texture.view;
        colorAttachment.resolveTarget = nullptr;
    }


    
    colorAttachment.loadOp  = (WGPULoadOp)renderPass->colorLoadOp;
    colorAttachment.storeOp = (WGPUStoreOp)renderPass->colorStoreOp;
    colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    colorAttachment.clearValue = WGPUColor{
        renderPass->colorClear.r,
        renderPass->colorClear.g,
        renderPass->colorClear.b,
        renderPass->colorClear.a,
    };

    depthAttachment.view = (WGPUTextureView)g_renderstate.renderTargetStack[g_renderstate.renderTargetStackPosition].depth.view;
    depthAttachment.depthLoadOp  = (WGPULoadOp)renderPass->depthLoadOp;
    depthAttachment.depthStoreOp = (WGPUStoreOp)renderPass->depthStoreOp;
    depthAttachment.depthClearValue = 1.0f;
    depthAttachment.depthReadOnly = false;
    depthAttachment.stencilLoadOp = WGPULoadOp_Undefined;
    depthAttachment.stencilStoreOp = WGPUStoreOp_Undefined;

    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = &depthAttachment;
    
    
    renderPass->rpEncoder = wgpuCommandEncoderBeginRenderPass((WGPUCommandEncoder)renderPass->cmdEncoder, &renderPassDesc);
    g_renderstate.activeRenderpass = renderPass;
}

void EndRenderpassEx(DescribedRenderpass* renderPass){
    drawCurrentBatch();
    wgpuRenderPassEncoderEnd((WGPURenderPassEncoder)renderPass->rpEncoder);
    g_renderstate.activeRenderpass = nullptr;
    auto re = renderPass->rpEncoder;
    renderPass->rpEncoder = 0;
    WGPUCommandBufferDescriptor cmdBufferDescriptor{};
    cmdBufferDescriptor.label = STRVIEW("CB");
    WGPUCommandBuffer command = wgpuCommandEncoderFinish((WGPUCommandEncoder)renderPass->cmdEncoder, &cmdBufferDescriptor);
    wgpuQueueSubmit((WGPUQueue)GetQueue(), 1, &command);
    wgpuRenderPassEncoderRelease((WGPURenderPassEncoder)re);
    wgpuCommandEncoderRelease((WGPUCommandEncoder)renderPass->cmdEncoder);
    wgpuCommandBufferRelease(command);
}
