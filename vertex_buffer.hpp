#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP
#include <cstring>
#include <cassert>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>
#include <iostream>
#include <webgpu/webgpu.h>
#include <cstdlib>

struct Vector3{
    float x,y,z;
};
struct Vector2{
    float x,y;
};
struct vertex{
    Vector3 pos;
    Vector2 uv ;
    Vector3 col;
};
struct full_renderstate;
struct wgpustate{
    WGPUInstance instance;
    WGPUAdapter adapter;
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
    WGPUTextureFormat frameBufferFormat;
    std::vector<vertex> current_vertices;
    full_renderstate* rstate;
    Vector2 nextuv;
    Vector3 nextcol;
};

extern wgpustate g_wgpustate;


inline void rlColor3f(float r, float g, float b){
    g_wgpustate.nextcol.x = r;
    g_wgpustate.nextcol.y = g;
    g_wgpustate.nextcol.z = b;
}
inline void rlTexCoord2f(float u, float v){
    g_wgpustate.nextuv.x = u;
    g_wgpustate.nextuv.y = v;
}

inline void rlVertex2f(float x, float y){
    g_wgpustate.current_vertices.push_back(vertex{{x, y, 0}, g_wgpustate.nextuv, g_wgpustate.nextcol});
}
inline WGPUVertexFormat f32format(uint32_t s){
    switch(s){
        case 1:return WGPUVertexFormat_Float32  ;
        case 2:return WGPUVertexFormat_Float32x2;
        case 3:return WGPUVertexFormat_Float32x3;
        case 4:return WGPUVertexFormat_Float32x4;
        default: abort();
    }
    return WGPUVertexFormat(0);
} 
struct vertex_buffer{
    WGPUVertexBufferLayout layout;
    WGPUBuffer buffer;
    std::vector<WGPUVertexAttribute> attribs;
    size_t size;

    vertex_buffer(const std::vector<vertex>& data, const std::vector<uint32_t>& attribute_sizes) : layout{}, buffer{}{
        using dtype = typename std::remove_reference_t<decltype(data)>::value_type;
        attribs.resize(attribute_sizes.size());
        uint32_t offset = 0;
        for(uint32_t i = 0;i < attribs.size();i++){
            attribs[i].shaderLocation = i;
            attribs[i].format = f32format(attribute_sizes[i]);
            attribs[i].offset = offset * sizeof(float);
            offset += attribute_sizes[i];
        }
        layout.attributeCount = attribs.size();
        layout.attributes = attribs.data();
        layout.arrayStride = sizeof(dtype);
        layout.stepMode = WGPUVertexStepMode_Vertex;

        WGPUBufferDescriptor bd;
        bd.mappedAtCreation = false;
        bd.size = data.size() * sizeof(dtype);
        size = bd.size;
        bd.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
        buffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bd);
        wgpuQueueWriteBuffer(g_wgpustate.queue, buffer, 0, data.data(), data.size() * sizeof(dtype));
    }
};
struct uniform_buffer{
    WGPUBuffer buffer;
    size_t size;
    uniform_buffer(std::vector<float> data){
        WGPUBufferDescriptor bufferDesc;
        bufferDesc.size = sizeof(float) * data.size();
        size = sizeof(float) * data.size();
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        bufferDesc.mappedAtCreation = false;
        buffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
    }
};
typedef struct Color{
    uint8_t r, g, b, a;
} Color;
typedef struct BGRAColor{
    uint8_t b, g, r, a;
} BGRAColor;

struct Image{
    WGPUTextureFormat format;
    uint32_t width, height;
    void* data;
};

inline Image LoadImageChecker(Color a, Color b, uint32_t width, uint32_t height, uint32_t checkerCount){
    Image ret{WGPUTextureFormat_RGBA8Unorm, width, height, std::calloc(width * height, sizeof(Color))};
    for(uint32_t i = 0;i < height;i++){
        for(uint32_t j = 0;j < width;j++){
            const size_t index = size_t(i) * width + j;
            const size_t ic = i * checkerCount / height;
            const size_t jc = j * checkerCount / width;
            static_cast<Color*>(ret.data)[index] = ((ic ^ jc) & 1) ? a : b;
        }
    }
    return ret;
}
struct Texture{
    WGPUTextureDescriptor desc;
    WGPUTexture tex;
    WGPUTextureView view;
    Texture() : desc{}, tex{}, view{}{}
    Texture(Image img) : desc{
        nullptr,
        nullptr,
        WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc,
        WGPUTextureDimension_2D,
        WGPUExtent3D{img.width, img.height, 1},
        img.format,
        1,1,1,nullptr}, tex{}, view{}{
        
        desc.viewFormats = &desc.format;
        tex = wgpuDeviceCreateTexture(g_wgpustate.device, &desc);
        WGPUTextureViewDescriptor vdesc{};
        vdesc.arrayLayerCount = 0;
        vdesc.aspect = WGPUTextureAspect_All;
        vdesc.format = desc.format;
        vdesc.dimension = WGPUTextureViewDimension_2D;
        vdesc.baseArrayLayer = 0;
        vdesc.arrayLayerCount = 1;
        vdesc.baseMipLevel = 0;
        vdesc.mipLevelCount = 1;
        
        WGPUImageCopyTexture destination;
        destination.texture = tex;
        destination.mipLevel = 0;
        destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
        destination.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures
        WGPUTextureDataLayout source;
        source.offset = 0;
        source.bytesPerRow = 4 * img.width;
        source.rowsPerImage = img.height;
        //wgpuQueueWriteTexture()
        wgpuQueueWriteTexture(g_wgpustate.queue, &destination, img.data,  4 * img.width * img.height, &source, &desc.size);
        view = wgpuTextureCreateView(tex, &vdesc);
    }
    Texture(const WGPUTextureDescriptor& d, const WGPUTextureViewDescriptor& vd) : desc(d), tex(wgpuDeviceCreateTexture(g_wgpustate.device, &d)), 
    view(wgpuTextureCreateView(tex, &vd)) {

    }
    uint32_t width()const noexcept{
        return desc.size.width;
    }

    uint32_t height()const noexcept{
        return desc.size.height;
    }

    Image load()const noexcept{
        auto& device = g_wgpustate.device;
        Image ret{desc.format, desc.size.width,desc.size.height, nullptr};
        WGPUBufferDescriptor b{};
        b.mappedAtCreation = false;
        b.size = 4 * desc.size.width * desc.size.height;
        b.usage = WGPUBufferUsage_MapRead | WGPUBufferUsage_CopyDst;
        
        WGPUBuffer readtex = wgpuDeviceCreateBuffer(device, &b);
        
        WGPUCommandEncoderDescriptor commandEncoderDesc{};
        commandEncoderDesc.label = "Command Encoder";
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);

        WGPUImageCopyTexture tbsource;
        tbsource.texture = tex;
        tbsource.mipLevel = 0;
        tbsource.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
        tbsource.aspect = WGPUTextureAspect_All; // only relevant for depth/Stencil textures

        WGPUImageCopyBuffer tbdest;
        tbdest.buffer = readtex;
        tbdest.layout.offset = 0;
        tbdest.layout.bytesPerRow = 4 * desc.size.width;
        tbdest.layout.rowsPerImage = desc.size.height;

        wgpuCommandEncoderCopyTextureToBuffer(encoder, &tbsource, &tbdest, &desc.size);
        
        WGPUCommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
        wgpuCommandEncoderRelease(encoder);
        wgpuQueueSubmit(g_wgpustate.queue, 1, &command);
        wgpuCommandBufferRelease(command);
        #ifdef WEBGPU_BACKEND_DAWN
        wgpuDeviceTick(device);
        #endif
        
        auto onBuffer2Mapped = [](WGPUBufferMapAsyncStatus status, void* userdata){
            //std::cout << "Backcalled: " << status << std::endl;
            assert(status == 0);
            std::pair<Image*, WGPUBuffer*>* rei = (std::pair<Image*, WGPUBuffer*>*)userdata;

            const void* map = wgpuBufferGetConstMappedRange(*rei->second, 0, wgpuBufferGetSize(*rei->second));
            rei->first->data = std::realloc(rei->first->data, wgpuBufferGetSize(*rei->second));
            std::memcpy(rei->first->data, map, wgpuBufferGetSize(*rei->second));
            wgpuBufferUnmap(*rei->second);
            wgpuBufferRelease(*rei->second);
            wgpuBufferDestroy(*rei->second);
        };

        std::pair<Image*, WGPUBuffer*> ibpair{&ret, &readtex};
        wgpuBufferMapAsync(readtex, WGPUMapMode_Read, 0, 4 * desc.size.width * desc.size.height, onBuffer2Mapped, &ibpair);
        while(ibpair.first->data == nullptr){
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            WGPUCommandBufferDescriptor cmdBufferDescriptor2{};
            cmdBufferDescriptor.label = "Command buffer";
            WGPUCommandEncoderDescriptor commandEncoderDesc2{};
            commandEncoderDesc.label = "Command Encode2";
            WGPUCommandEncoder encoder2 = wgpuDeviceCreateCommandEncoder(device, &commandEncoderDesc);
            WGPUCommandBuffer command2 = wgpuCommandEncoderFinish(encoder2, &cmdBufferDescriptor);
            wgpuQueueSubmit(g_wgpustate.queue, 1, &command2);
            wgpuCommandBufferRelease(command);
            #ifdef WEBGPU_BACKEND_DAWN
            wgpuDeviceTick(device);
            #endif
        }
        #ifdef WEBGPU_BACKEND_DAWN
        wgpuInstanceProcessEvents(g_wgpustate.instance);
        wgpuDeviceTick(device);
        #endif
        //readtex.destroy();
        //std::cout << "Function exited" << std::endl;
        return ret;
    }
};
inline Texture LoadTextureEx(uint32_t width, uint32_t height, WGPUTextureFormat format, bool to_be_used_as_rendertarget){
    WGPUTextureDescriptor tDesc{};
    tDesc.dimension = WGPUTextureDimension_2D;
    tDesc.size = {width, height, 1u};
    tDesc.mipLevelCount = 1;
    tDesc.sampleCount = 1;
    tDesc.format = format;
    tDesc.usage  = (WGPUTextureUsage_RenderAttachment * to_be_used_as_rendertarget) | WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst | WGPUTextureUsage_CopySrc;
    tDesc.viewFormatCount = 0;
    tDesc.viewFormats = nullptr;

    WGPUTextureViewDescriptor textureViewDesc{};
    textureViewDesc.aspect = ((format == WGPUTextureFormat_Depth24Plus) ? WGPUTextureAspect_DepthOnly : WGPUTextureAspect_All);
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = 1;
    textureViewDesc.dimension = WGPUTextureViewDimension_2D;
    textureViewDesc.format = tDesc.format;
    return Texture(tDesc, textureViewDesc);
}

inline Texture LoadTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_RGBA8Unorm, true);
}

inline Texture LoadDepthTexture(uint32_t width, uint32_t height){
    return LoadTextureEx(width, height, WGPUTextureFormat_Depth24Plus, true);
}
struct RenderTexture{
    Texture color;
    Texture depth;
};

inline RenderTexture LoadRenderTexture(uint32_t width, uint32_t height){
    return RenderTexture{.color = LoadTexture(width, height), .depth = LoadDepthTexture(width, height)};
}

struct render_pipeline{
    WGPURenderPipeline pl;
    WGPURenderPipelineDescriptor desc;
};

struct render_pass{
    WGPURenderPassDescriptor desc;
    WGPUTextureView color, depth;
    WGPURenderPassColorAttachment colorAttachment;
    WGPURenderPassDepthStencilAttachment depthAttachment;
    render_pass(WGPUTextureView c, WGPUTextureView d): color(c), depth(d){

    }
};
/*inline render_pipeline generatePipeline(const vertex_buffer& vb, const std::vector<uniform_buffer> uniform_buffers, wgpu::ShaderModule shader, wgpu::TextureFormat format){
    WGPURenderPipelineDescriptor rpd{};
    rpd.vertex.bufferCount = 1;
    rpd.vertex.buffers = &(vb.layout);
    rpd.vertex.constantCount = 0;
    rpd.vertex.module = shader;
    rpd.vertex.entryPoint = "vs_main";
    rpd.vertex.constants = nullptr;

    rpd.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    rpd.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    rpd.primitive.frontFace = wgpu::FrontFace::CCW;
    rpd.primitive.cullMode = wgpu::CullMode::None;
    
    wgpu::BlendState blendState;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;
    blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blendState.alpha.dstFactor = wgpu::BlendFactor::One;
    blendState.alpha.operation = wgpu::BlendOperation::Add;


    std::vector<wgpu::BindGroupLayoutEntry> blayouts(uniform_buffers.size());
    for(uint32_t i = 0; i < uniform_buffers.size();i++){
        blayouts[i] = wgpu::Default;
        blayouts[i].binding = i;
        blayouts[i].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        blayouts[0].buffer.type = wgpu::BufferBindingType::Uniform;
        blayouts[0].buffer.minBindingSize = uniform_buffers[i].size;
    }
    wgpu::BindGroupLayoutDescriptor bgld{};
    bgld.entryCount = uniform_buffers.size();
    bgld.entries = blayouts.data();
    
    
    wgpu::FragmentState fs;
    fs.constantCount = 0;
    fs.constants = nullptr;
    fs.module = shader;
    fs.entryPoint = "fs_main";

    wgpu::DepthStencilState depthStencilState = wgpu::Default;
    // We setup a depth buffer state for the render pipeline
    // Keep a fragment only if its depth is lower than the previously blended one
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    // Each time a fragment is blended into the target, we update the value of the Z-buffer
    depthStencilState.depthWriteEnabled = true;
    // Store the format in a variable as later parts of the code depend on it
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
    depthStencilState.format = depthTextureFormat;
    // Deactivate the stencil alltogether
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    
    wgpu::ColorTargetState colorTarget;
    colorTarget.format = format;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = wgpu::ColorWriteMask::All;

    fs.targetCount = 1;
    fs.targets = &colorTarget;
    rpd.fragment = &fs;
    rpd.depthStencil = &depthStencilState;

    wgpu::MultisampleState mss;
    mss.count = 1;
    mss.mask = ~0u;
    mss.alphaToCoverageEnabled = false;
    rpd.multisample = mss;
    //std::cout << "Creating the piplin" << std::endl;
    return render_pipeline{g_wgpustate.device.createRenderPipeline(rpd)};
}*/
/*inline void createRenderPass(render_pass& ret){
    using namespace wgpu;
    //render_pass ret(c, d);
    wgpu::RenderPassDescriptor renderPassDesc{};
    wgpu::RenderPassColorAttachment& renderPassColorAttachment = ret.colorAttachment;
    renderPassColorAttachment.view = ret.color;
    renderPassColorAttachment.resolveTarget = nullptr;
    renderPassColorAttachment.loadOp = LoadOp::Clear;
    renderPassColorAttachment.storeOp = StoreOp::Store;
    renderPassColorAttachment.clearValue = Color{ 0.01, 0.01, 0.2, 1.0 };
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &ret.colorAttachment;


    RenderPassDepthStencilAttachment& depthStencilAttachment = ret.depthAttachment;
    // The view of the depth texture
    depthStencilAttachment.view = ret.depth;

    // The initial value of the depth buffer, meaning "far"
    depthStencilAttachment.depthClearValue = 1.0f;
    // Operation settings comparable to the color attachment
    depthStencilAttachment.depthLoadOp = LoadOp::Clear;
    depthStencilAttachment.depthStoreOp = StoreOp::Store;
    // we could turn off writing to the depth buffer globally here
    depthStencilAttachment.depthReadOnly = false;

    // Stencil setup, mandatory but unused
    depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
    depthStencilAttachment.stencilLoadOp = LoadOp::Load;
    depthStencilAttachment.stencilStoreOp = StoreOp::Store;
#else
    depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
    depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
#endif
    depthStencilAttachment.stencilReadOnly = true;
    renderPassDesc.depthStencilAttachment = &ret.depthAttachment;
    renderPassDesc.timestampWriteCount = 0;
    renderPassDesc.timestampWrites = nullptr;
    ret.desc = renderPassDesc;
    //return ret;
}*/
enum uniform_type{
    uniform_buffer, texture2d, sampler
};
struct ass{
    union{
        float x;
        int y;
    };
};
inline WGPUShaderModule LoadShaderFromMemory(const std::string& shaderSource) {
    WGPUShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    WGPUShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    #ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    #endif

    return wgpuDeviceCreateShaderModule(g_wgpustate.device, &shaderDesc);
}
inline WGPUShaderModule LoadShader(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string shaderSource(size, ' ');
    file.seekg(0);
    file.read(shaderSource.data(), size);

    WGPUShaderModuleWGSLDescriptor shaderCodeDesc;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    shaderCodeDesc.code = shaderSource.c_str();
    WGPUShaderModuleDescriptor shaderDesc;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
#endif

    return wgpuDeviceCreateShaderModule(g_wgpustate.device, &shaderDesc);
}
struct full_renderstate{
    WGPUShaderModule shader;
    WGPUTextureView color;
    WGPUTextureView depth;

    WGPURenderPipeline pipeline;
    WGPURenderPassDescriptor renderPassDesc;
    WGPURenderPassColorAttachment rca;
    WGPURenderPassDepthStencilAttachment dsa;


    WGPUBindGroupLayoutDescriptor bglayoutdesc;
    WGPUBindGroupLayout bglayout;
    WGPURenderPipelineDescriptor pipelineDesc;
    
    WGPUVertexBufferLayout vlayout;
    WGPUBuffer vbo;

    WGPUVertexAttribute attribs[8]; // Size: vlayout.attributeCount

    WGPUBindGroup bg;
    WGPUBindGroupEntry bgEntries[8]; // Size: bglayoutdesc.entryCount
    full_renderstate(const WGPUShaderModule& sh, const std::vector<std::pair<uniform_type, size_t>>& uniform_types_and_size, const std::vector<uint32_t>& vattribute_sizes, WGPUTextureView c, WGPUTextureView d) : shader(sh), color(c), depth(d),
    bglayout([&](const std::vector<std::pair<uniform_type, size_t>>& uniform_types_and_size, WGPUBindGroupLayoutDescriptor& _bglayoutdesc){
        std::vector<WGPUBindGroupLayoutEntry> blayouts(uniform_types_and_size.size());
        _bglayoutdesc = WGPUBindGroupLayoutDescriptor{};
        std::memset(blayouts.data(), 0, blayouts.size() * sizeof(WGPUBindGroupLayoutEntry));
        for(size_t i = 0;i < uniform_types_and_size.size();i++){
            blayouts[i].binding = i;
            switch(uniform_types_and_size[i].first){
                case uniform_buffer:
                blayouts[i].visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
                blayouts[i].buffer.type = WGPUBufferBindingType_Uniform;
                blayouts[i].buffer.minBindingSize = sizeof(uniform_types_and_size[i].second);
                break;
                case texture2d:
                blayouts[i].visibility = WGPUShaderStage_Fragment;
                blayouts[i].texture.sampleType = WGPUTextureSampleType_Float;
                blayouts[i].texture.viewDimension = WGPUTextureViewDimension_2D;
                break;
                case sampler:
                blayouts[i].visibility = WGPUShaderStage_Fragment;
                blayouts[i].sampler.type = WGPUSamplerBindingType_Filtering;
                break;
                default:break;
            }
        }
        bglayoutdesc.entryCount = uniform_types_and_size.size();
        bglayoutdesc.entries = blayouts.data();
        return wgpuDeviceCreateBindGroupLayout(g_wgpustate.device, &bglayoutdesc);
    }(uniform_types_and_size, bglayoutdesc)), vbo(0), bg(0), bgEntries{}{
        uint32_t offset = 0;
        assert(vattribute_sizes.size() <= 8);
        for(uint32_t i = 0;i < vattribute_sizes.size();i++){
            attribs[i].shaderLocation = i;
            attribs[i].format = f32format(vattribute_sizes[i]);
            attribs[i].offset = offset * sizeof(float);
            offset += vattribute_sizes[i];
        }
        vlayout.attributeCount = vattribute_sizes.size();
        vlayout.attributes = attribs;
        vlayout.arrayStride = sizeof(vertex);
        vlayout.stepMode = WGPUVertexStepMode_Vertex;  
        updatePipeline();
        updateRenderPassDesc();  
    }
    void updateVertexBuffer(const void* data, size_t size){
        if(vbo){
            wgpuBufferRelease(vbo);
            wgpuBufferDestroy(vbo);
        }
        WGPUBufferDescriptor bufferDesc{};
        bufferDesc.size = size;
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex;
        
        bufferDesc.mappedAtCreation = false;

        vbo = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);
        wgpuQueueWriteBuffer(g_wgpustate.queue, vbo, 0, data, size);
    }
    void setTexture(uint32_t index, Texture tex){
        bgEntries[index] = WGPUBindGroupEntry{};
        bgEntries[index].binding = index;
        bgEntries[index].textureView = tex.view;
    }
    void setSampler(uint32_t index, WGPUSampler sampler){
        bgEntries[index] = WGPUBindGroupEntry{};
        bgEntries[index].binding = index;
        bgEntries[index].sampler = sampler;
    }
    void setUniformBuffer(uint32_t index, const void* data, size_t size){
        if(bgEntries[index].buffer){
            wgpuBufferRelease(bgEntries[index].buffer);
            wgpuBufferDestroy(bgEntries[index].buffer);
        }
        WGPUBufferDescriptor bufferDesc{};
        bufferDesc.size = size;
        bufferDesc.usage = WGPUBufferUsage_CopyDst | WGPUBufferUsage_Uniform;
        bufferDesc.mappedAtCreation = false;
        WGPUBuffer uniformBuffer = wgpuDeviceCreateBuffer(g_wgpustate.device, &bufferDesc);

        wgpuQueueWriteBuffer(g_wgpustate.queue, uniformBuffer, 0, data, size);
        
        bgEntries[index] = WGPUBindGroupEntry{};
        bgEntries[index].binding = index;
        bgEntries[index].buffer = uniformBuffer;
        bgEntries[index].offset = 0;
        bgEntries[index].size = size;
        
    }
    void updateBindGroup(){
        WGPUBindGroupDescriptor bgdesc{};
        bgdesc.entryCount = this->bglayoutdesc.entryCount;
        bgdesc.entries = bgEntries;
        bgdesc.layout = this->bglayout;
        if(bg)wgpuBindGroupRelease(bg);
        bg = wgpuDeviceCreateBindGroup(g_wgpustate.device, &bgdesc);
    }
    void updatePipeline(){
        pipelineDesc = WGPURenderPipelineDescriptor{};
        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vlayout;

        pipelineDesc.vertex.module = shader;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.constantCount = 0;
        pipelineDesc.vertex.constants = nullptr;

        pipelineDesc.primitive.topology =         WGPUPrimitiveTopology_TriangleList;
        pipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
        pipelineDesc.primitive.frontFace =        WGPUFrontFace_CCW;
        pipelineDesc.primitive.cullMode =         WGPUCullMode_None;

        WGPUFragmentState fragmentState{};
        pipelineDesc.fragment = &fragmentState;
        fragmentState.module = shader;
        fragmentState.entryPoint = "fs_main";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        WGPUBlendState blendState{};
        blendState.color.srcFactor = WGPUBlendFactor_SrcAlpha;
        blendState.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
        blendState.color.operation = WGPUBlendOperation_Add;
        blendState.alpha.srcFactor = WGPUBlendFactor_Zero;
        blendState.alpha.dstFactor = WGPUBlendFactor_One;
        blendState.alpha.operation = WGPUBlendOperation_Add;

        WGPUColorTargetState colorTarget{};
        colorTarget.format = g_wgpustate.frameBufferFormat;

        colorTarget.blend = &blendState;
        colorTarget.writeMask = WGPUColorWriteMask_All;

        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;

        // We setup a depth buffer state for the render pipeline
        WGPUDepthStencilState depthStencilState{};
        // Keep a fragment only if its depth is lower than the previously blended one
        depthStencilState.depthCompare = WGPUCompareFunction_Less;
        // Each time a fragment is blended into the target, we update the value of the Z-buffer
        depthStencilState.depthWriteEnabled = true;
        // Store the format in a variable as later parts of the code depend on it
        WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
        depthStencilState.format = depthTextureFormat;
        // Deactivate the stencil alltogether
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;
        depthStencilState.stencilFront.compare = WGPUCompareFunction_Always;
        depthStencilState.stencilBack.compare = WGPUCompareFunction_Always;
        pipelineDesc.depthStencil = &depthStencilState;

        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = ~0u;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;
        // Create a bind group layout

        // Create the pipeline layout
        WGPUPipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bglayout;
        WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(g_wgpustate.device, &layoutDesc);
        pipelineDesc.layout = layout;
        pipeline = wgpuDeviceCreateRenderPipeline(g_wgpustate.device, &pipelineDesc);
        wgpuPipelineLayoutRelease(layout);
    }
    void updateRenderPassDesc(){
        renderPassDesc = WGPURenderPassDescriptor{};
        rca = WGPURenderPassColorAttachment{};
        rca.view = color;
        rca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        #ifdef __EMSCRIPTEN__
        rca.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
        #endif
        //std::cout << rca.depthSlice << "\n";
        rca.resolveTarget = nullptr;
        rca.loadOp =  WGPULoadOp_Clear;
        rca.storeOp = WGPUStoreOp_Store;
        rca.clearValue = WGPUColor{ 0.01, 0.01, 0.2, 1.0 };
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &rca;

        // We now add a depth/stencil attachment:
        dsa = WGPURenderPassDepthStencilAttachment{};
        // The view of the depth texture
        dsa.view = depth;
        //dsa.depthSlice = 0;
        // The initial value of the depth buffer, meaning "far"
        dsa.depthClearValue = 1.0f;
        // Operation settings comparable to the color attachment
        dsa.depthLoadOp = WGPULoadOp_Clear;
        dsa.depthStoreOp = WGPUStoreOp_Store;
        // we could turn off writing to the depth buffer globally here
        dsa.depthReadOnly = false;

        // Stencil setup, mandatory but unused
        dsa.stencilClearValue = 0;
        #ifdef WEBGPU_BACKEND_WGPU
        dsa.stencilLoadOp =  WGPULoadOp_Load;
        dsa.stencilStoreOp = WGPUStoreOp_Store;
        #else
        dsa.stencilLoadOp = WGPULoadOp_Undefined;
        dsa.stencilStoreOp = WGPUStoreOp_Undefined;
        #endif
        dsa.stencilReadOnly = true;
        renderPassDesc.depthStencilAttachment = &dsa;
        renderPassDesc.timestampWrites = nullptr;
    }
    void setTargetTextures(WGPUTextureView c, WGPUTextureView d){
        color = c;
        depth = d;
        updateRenderPassDesc();
    }
    template<typename callable>
    void executeRenderpass(callable&& c){
        WGPUCommandEncoderDescriptor commandEncoderDesc = {};
        commandEncoderDesc.label = "Command Encoder";
        WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(g_wgpustate.device, &commandEncoderDesc);
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

        wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

        wgpuRenderPassEncoderSetBindGroup(renderPass, 0, bg, 0, 0);
        wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, this->vbo, 0, wgpuBufferGetSize(vbo));

        c(renderPass);
        wgpuRenderPassEncoderEnd(renderPass);
        WGPUCommandBufferDescriptor cmdBufferDescriptor{};
        cmdBufferDescriptor.label = "Command buffer";
        WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);

        wgpuQueueSubmit(g_wgpustate.queue, 1, &command);
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(command);
    }

};
#endif
