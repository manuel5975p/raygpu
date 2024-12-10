#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include "GLFW/glfw3.h"
#include <raygpu.h>
#include <stb_image_write.h>
#include <tint/lang/wgsl/ast/extension.h>
#ifndef __EMSCRIPTEN__
//#include "dawn/dawn_proc.h"
//#include "dawn/native/DawnNative.h"
//#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__


#include "wgpustate.inc"

constexpr char shaderSource[] = R"(
@group(0) @binding(0) var texter: texture_2d<f32>;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) uv: vec2f,
    //@location(2) colar: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f,
};
//@builtin(instance_index) instanceID: u32;
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.position = vec4f(in.position.xyz, 1.0f);
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.uv.xy,1,1);
    return textureLoad(texter, vec2<i32>(i32(in.position.x) % 100, i32(in.position.y) % 100), 0);
}
)";

Texture checkers;
RenderTexture rtex;

uint64_t frames = 0;
uint64_t stmp = NanoTime();
int main(){
    
    constexpr uint32_t width = 1920;
    constexpr uint32_t height = 1080;
    auto window = InitWindow(width, height);
    WGPUSamplerDescriptor sdesc;
    
    AttributeAndResidence aar[2] = {{WGPUVertexAttribute{WGPUVertexFormat_Float32x3,0,0},0,WGPUVertexStepMode_Vertex}, {WGPUVertexAttribute{WGPUVertexFormat_Float32x2, 12, 1},0,WGPUVertexStepMode_Vertex}};
    UniformDescriptor udesc{};
    udesc.type = texture2d;
    //std::cout << "Loadpipeline" << std::endl;
    RenderSettings settings{};
    settings.depthTest = 1;
    settings.depthCompare = WGPUCompareFunction_LessEqual;
    auto pl = LoadPipelineEx(shaderSource, aar, 2, &udesc, 1, settings);
    //std::cout << "Loadpipelined" << std::endl;
    //Matrix udata = MatrixLookAt(Vector3{0,0,0.2}, Vector3{0,0,0}, Vector3{0,1,0});
    //Matrix udata = MatrixIdentity();

    //Matrix udata = (MatrixPerspective(1.2, 1, 0.01, 100.0));
    rtex = LoadRenderTexture(1000, 1000);
    Texture tex = LoadTextureFromImage(LoadImageChecker(Color{255,0,0,255}, Color{0,255,0,255}, 100, 100, 10));
    checkers = LoadTextureFromImage(LoadImageChecker(Color{230, 230, 230, 255}, Color{100, 100, 100, 255}, 100, 100, 10));
    WGPUBufferDescriptor mapBufferDesc{};
    float data[15] = {0,0,0,1,0,
                     1,0,0,0,1,
                     0,1,0,0.3,0.5
                    };
    DescribedBuffer vbo = GenBuffer(data, sizeof(data));
    VertexArray* va = LoadVertexArray();
    VertexAttribPointer(va, &vbo, 0, WGPUVertexFormat_Float32x3, 0, WGPUVertexStepMode_Vertex);
    EnableVertexAttribArray(va, 0);
    VertexAttribPointer(va, &vbo, 1, WGPUVertexFormat_Float32x2, 12, WGPUVertexStepMode_Vertex);
    EnableVertexAttribArray(va, 1);

    
    
    WGPUBindGroupDescriptor bgdesc{};
    bgdesc.layout = pl->bglayout.layout;
    WGPUBindGroupEntry the_texture_entry{};
    the_texture_entry.binding = 0;
    the_texture_entry.textureView = tex.view;
    bgdesc.entries = &the_texture_entry;
    bgdesc.entryCount = 1;
    
    //std::cout << "Waypoint" << std::endl;
    WGPUBindGroup bg = wgpuDeviceCreateBindGroup(g_wgpustate.device, &bgdesc);
    //g_wgpustate.rstate->executeRenderpassPlain([&vbo, &pl, &bg](WGPURenderPassEncoder encoder){
    //    UsePipeline(encoder, pl);
    //    wgpuRenderPassEncoderSetVertexBuffer(encoder, 0, vbo, 0, 60);
    //    wgpuRenderPassEncoderSetBindGroup(encoder,0, bg, 0, nullptr);
    //    wgpuRenderPassEncoderDraw(encoder, 3, 1, 0, 0);
    //});
    
    //constexpr int rtd = 2000;
    //rtex = LoadRenderTexture(rtd, rtd);
    //MatrixMultiply(MatrixPerspective(1.0, double(GetScreenWidth()) / GetScreenHeight(), 0.01, 100.0), MatrixLookAt(Vector3{0, 0, -2.5}, Vector3{0, 0, 0}, Vector3{0,1,0}));
    Matrix persp = MatrixMultiply(MatrixPerspective(1.0, double(GetScreenWidth()) / GetScreenHeight(), 0.01, 100.0), MatrixLookAt(Vector3{0, 0, -2.5}, Vector3{0, 0, 0}, Vector3{0,1,0}));
    //SetUniformBuffer(0, &udata, 64 * sizeof(float));    
    Matrix sc2 = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
    auto mainloop = [&](void* userdata){
        
        //wgpuQueueWriteBuffer(g_wgpustate.queue, vbo.buffer, 0, data, sizeof(data));
        float z = 0;
        //std::cout << "<<Before:" << std::endl;
        //BeginDrawing();
        ////std::cout << "<<After:" << std::endl;
        //g_wgpustate.rstate->executeRenderpassPlain([&](WGPURenderPassEncoder encoder){
        //    UsePipeline(encoder, pl);
        //    wgpuRenderPassEncoderSetVertexBuffer(encoder, 0, vbo.buffer, 0, 60);
        //    wgpuRenderPassEncoderSetBindGroup(encoder,0, bg, 0, nullptr);
        //    wgpuRenderPassEncoderDraw(encoder, 3, 1, 0, 0);
        //});
        //EndDrawing();
        //return;
        //Matrix udata = MatrixMultiply(MatrixPerspective(1.0, double(GetScreenWidth()) / GetScreenHeight(), 0.01, 100.0), MatrixLookAt(Vector3{0, 0, -2.5}, Vector3{0, 0, 0}, Vector3{0,1,0}));
        //SetUniformBuffer(0, &udata, 64 * sizeof(float));
        
        BeginDrawing();
        //g_wgpustate.rstate->executeRenderpassPlain([&](wgpu::RenderPassEncoder end){
        //    end.SetPipeline(pl.pipeline);
        //    end.SetVertexBuffer(0, vbo.buffer);
        //    end.SetBindGroup(0, bg);
        //    end.Draw(3);
        //});
        //UseNoTexture();
        //rlBegin(RL_TRIANGLES);
        //rlColor4f(1, 1, 1, 1);
        //rlVertex2f(0.1,0.1);
        //rlVertex2f(0, 0.1);
        //rlVertex2f(0.1, 0.0);
        //rlVertex2f(-0.5, -0.5);
        //rlVertex2f(-0.5, -0.4);
        //rlVertex2f(-0.4, -0.5);
        //rlEnd();
        UseTexture(checkers);
        BeginTextureMode(rtex);
        SetUniformBuffer(0, &persp, 16 * sizeof(float));
        rlBegin(RL_QUADS);

        rlTexCoord2f(0, 0);
        rlColor4f(1, 0, 0, 1);
        rlVertex3f(1, 0, z);

        rlTexCoord2f(0.0, 1.0);
        rlColor4f(0, 1, 0, 1);
        rlVertex3f(0, 1, z);

        rlTexCoord2f(1.0, 0.0);
        rlColor4f(1, 1, 1, 1);
        rlVertex3f(-1, 0, z);

        rlTexCoord2f(1.0, 1.0);

        rlVertex3f(0,-1, z);
        rlEnd();
        EndTextureMode();
        SetUniformBuffer(0, &sc2, 16 * sizeof(float));
        //SetUniformBuffer(0, &sc2, 16 * sizeof(float));
        if(frames == 3){
            Image frame = LoadImageFromTexture(rtex.color);
            SaveImage(frame, "output.png");
        }
        UseTexture(rtex.color);
        for(size_t i = 0;i < 1;i++){
            for(size_t j = 0;j < 3;j++){
                DrawTexturePro(
                    //g_wgpustate.whitePixel,
                    rtex.color,
                    Rectangle(0, 0, 1000, 1000), Rectangle(i * 110, j * 110, 90, 90),
                    Vector2(0,0), 
                    g_wgpustate.total_frames * (0.0f / 100.0f),
                    Color{255, 255, 255, 255}
                );
            }
        }
        for(size_t i = 0;i < 4;i++){
            for(size_t j = 0;j < 800;j++){
                DrawTexturePro(
                    g_wgpustate.whitePixel,
                    //rtex.color,
                    Rectangle(0, 0, 1000, 1000), Rectangle((i+2) * 110, j * 2, 90, 1),
                    Vector2(0,0), 
                    g_wgpustate.total_frames * (0.0f / 100.0f),
                    Color{255, 255, 255, 255}
                );
            }
        }

        
        //Image img = LoadImageFromTexture(rtex.color);
        //std::cout << img.format << "\n";
        //if(total_frames == 3)
        //    stbi_write_png("ass.png", img.width, img.height, 4, img.data, std::ceil(4.0 * img.width / 256.0) * 256);
        //std::free(img.data);
        EndDrawing();
        
        ++frames;
        //std::cout << g_wgpustate.total_frames << "\n";
        uint64_t nextStmp = NanoTime();
        if(nextStmp - stmp > 1000000000){
            std::cout << GetFPS() << "\n";
            stmp = NanoTime();
        }

    };
    auto mainloop2 = [&](void* userdata){

        BeginDrawing();
        //ClearBackground(Color{uint8_t(frames >> 3),uint8_t(frames >> 3),0,255});
        //WGPUBindGroupEntry entry{};
        //entry.binding = 0;
        //entry.textureView = checkers.view;
        //UpdateBindGroupEntry(&pl->bindGroup, 0, entry);
        
        //BeginPipelineMode(pl);
        //SetTexture(0, checkers);
        //BindVertexArray(pl, va);
        //DrawArrays(3);
        //EndPipelineMode();
        
        //EndRenderPass(&g_wgpustate.rstate->renderpass);
        //BeginRenderPass(&g_wgpustate.rstate->renderpass);
        for(double x = -1;x < 1; x += 0.1){
            for(double y = -1;y < 1; y += 0.1){
                rlBegin(RL_TRIANGLES);
                rlColor4f(1, 1, 1, 1);
                rlVertex2f(x, y);
                rlVertex2f(x + 0.005, y);
                rlVertex2f(x, y + 0.005);
                rlEnd();
                DrawTexturePro(checkers, Rectangle{0, 0, 100, 100}, Rectangle{(float)x,(float)y,0.05f,0.05f}, Vector2{0, 0}, 0.0f, Color{255,255,255,255});
            }
        }

        EndDrawing();
        ++frames;
        //std::cout << g_wgpustate.total_frames << "\n";
        uint64_t nextStmp = NanoTime();
        if(nextStmp - stmp > 1000000000){
            std::cout << GetFPS() << "\n";
            stmp = NanoTime();
        }
    };
    #ifndef __EMSCRIPTEN__
    while(!glfwWindowShouldClose(window)){
        mainloop2(nullptr);
    }
    #else
    emscripten_set_main_loop_arg(mainloop, nullptr, 240, true);
    #endif // __EMSCRIPTEN__
}