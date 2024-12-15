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
    
    constexpr uint32_t width = 2560;
    constexpr uint32_t height = 1440;
    //SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    auto window = InitWindow(width, height, "Example");
    SetTargetFPS(144);
    HideCursor();
    ShowCursor();
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
    Texture tex = LoadTextureFromImage(GenImageChecker(Color{255,0,0,255}, Color{0,255,0,255}, 100, 100, 10));
    checkers = LoadTextureFromImage(GenImageChecker(Color{0, 255, 0, 255}, Color{0, 0, 0, 255}, 100, 100, 10));
    WGPUBufferDescriptor mapBufferDesc{};
    float data[15] = {0,0,0,1,0,
                     1,0,0,0,1,
                     0,1,0,0.3,0.5
                    };
    float data2[45] = {-1,-1,0,1,0,
                       0,-1,0,0,1,
                       -1,0,0,0.3,0.5,
                       -1,-1,0,1,0,
                       0,-1,0,0,1,
                       -1,0,0,0.3,0.5,
                       -1,-1,0,1,0,
                       0,-1,0,0,1,
                       -1,0,0,0.3,0.5
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
        SetUniformBufferData(0, &persp, 16 * sizeof(float));
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
        SetUniformBufferData(0, &sc2, 16 * sizeof(float));
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
    StagingBuffer mbuf = GenStagingBuffer(15 * sizeof(float), WGPUBufferUsage_CopyDst | WGPUBufferUsage_Vertex);
    memcpy(mbuf.map, data, sizeof(float) * 15);
    UpdateStagingBuffer(&mbuf);
    constexpr size_t ds = sizeof(float) * 15;
    SetUniformBufferData(0, &sc2, 64);
    DescribedBuffer buf2 = GenBuffer(data2, ds);
    auto deffont = LoadImage("deffont.png");
    Texture latlas = LoadTextureFromImage(deffont);
    std::string togst = "Helo ";
    auto mainloop2 = [&](void* userdata){

        BeginDrawing();
        ClearBackground(Color{uint8_t(frames >> 4),uint8_t(frames >> 4),0, 255});
        //WGPUBindGroupEntry entry{};
        //entry.binding = 0;
        //entry.textureView = checkers.view;
        //UpdateBindGroupEntry(&pl->bindGroup, 0, entry);
        //SaveImage(deffont, "again.png");
        //DrawTexturePro(latlas, Rectangle(0,0,100,100), Rectangle(0,0,500,500), Vector2{0,0},0, WHITE);
        //for(int j = 0;j < 10;j++)
        //    for(int i = 0;i < 100;i++){
        //        DrawTexturePro(checkers, Rectangle(0,0,100,100), Rectangle(i * 10,j*10,5,5), Vector2{0,0},0, WHITE);
        //        //DrawTexturePro(GetFontDefault().texture, Rectangle(0,0,100,100), Rectangle(i * 10+5,j*10,5,5), Vector2{0,0},0, WHITE);
        //    }

        //DrawTexturePro(latlas, Rectangle(0,0,100,100), Rectangle(0,0,500,500), Vector2{0,0},0, WHITE);
        //SetUniformBufferData(0, &sc2, 64);
        DrawCircle(GetMouseX(), GetMouseY(), 500, WHITE);
        DrawCircleV(GetMousePosition(), 20, Color{255,0,0,255});
        DrawCircle(880, 300, 50, Color{255,0,0,100});
        DrawCircleLines(900, 300, 50, Color{0,255,0,255});
        if(IsKeyPressed(GLFW_KEY_A))
            DrawText(togst.c_str(), 200, 400, 30, WHITE);
        if(IsKeyPressed(GLFW_KEY_U)){
            ToggleFullscreen();
        }
        while(int cp = GetCharPressed()){
            togst += cp;
        }
        DrawFPS(0, 0);
        //DescribedBuffer buf = GenBuffer(data, ds);
        //BeginPipelineMode(pl);
        //RecreateStagingBuffer(&mbuf);
        //memcpy(mbuf.map, data, sizeof(float) * 15);
        //UpdateStagingBuffer(&mbuf);
        //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, mbuf.gpuUsable.buffer, 0, sizeof(float)*15);
        //wgpuRenderPassEncoderDraw(g_wgpustate.rstate->activeRenderPass->rpEncoder, 3, 1, 0, 0);
        /*for(size_t i = 0;i < 0;i++){
            data2[5] += 0.001f;
            data2[5] = std::max(data2[5], -0.99f);
            data2[5] = std::fmod(data2[5]+1, 5.0f)-1;
            data2[11] += 0.002f;
            data2[11] = std::max(data2[11], -0.99f);
            data2[11] = std::fmod(data2[11]+1, 5.0f)-1;
            DescribedBuffer buf = GenBuffer(data2, ds * 3);
            
            //RecreateStagingBuffer(&mbuf);
            //memcpy(mbuf.map, data2, sizeof(float) * 15);
            //UpdateStagingBuffer(&mbuf);
            
            wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, buf.buffer, 0, sizeof(float)*15);
            wgpuRenderPassEncoderDraw(g_wgpustate.rstate->activeRenderPass->rpEncoder, 3, 1, 0, 0);
            wgpuBufferRelease(buf.buffer);
        }*/
        
        //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, buf.buffer, 0, sizeof(float)*15);
        //wgpuRenderPassEncoderDraw(g_wgpustate.rstate->activeRenderPass->rpEncoder, 3, 1, 0, 0);
        //wgpuBufferRelease(buf.buffer);
        //buf = GenBuffer(data2, ds);
        //wgpuRenderPassEncoderSetVertexBuffer(g_wgpustate.rstate->activeRenderPass->rpEncoder, 0, buf2.buffer, 0, sizeof(float)*15);
        //wgpuRenderPassEncoderDraw(g_wgpustate.rstate->activeRenderPass->rpEncoder, 3, 1, 0, 0);
        //SetTexture(0, checkers);
        //BindVertexArray(pl, va);
        //DrawArrays(3);
        //EndPipelineMode();
        //EndRenderPass(&g_wgpustate.rstate->renderpass);
        //BeginRenderPass(&g_wgpustate.rstate->renderpass);
        
        Matrix iden = MatrixIdentity();
        SetUniformBufferData(0, &iden, 64);
        for(double x = -1;x <= 1; x += 0.5){
            for(double y = -1;y <= 1; y += 0.5){
                UseTexture(g_wgpustate.whitePixel);
                rlBegin(RL_LINES);
                rlColor4f(0, 0, 1, 1);
                rlVertex2f(x, y);
                rlVertex2f(x + 0.4, y);
                //rlVertex2f(x, y + 0.4);
                rlEnd();

                //goto end;
                //assert(g_wgpustate.rstate->activeRenderPass == &g_wgpustate.rstate->renderpass);
                //DrawTexturePro(checkers, Rectangle{0, 0, 100, 100}, Rectangle{(float)x + 0.01f,(float)y,0.02f,0.02f}, Vector2{0, 0}, 0.0f, Color{255,255,255,255});
                //goto end;
                //EndRenderpassEx(&g_wgpustate.rstate->renderpass);
                //BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
                
                //DrawTexturePro(g_wgpustate.whitePixel, Rectangle{0, 0, 100, 100}, Rectangle{(float)x + 0.4f,(float)y,0.05f,0.05f}, Vector2{0, 0}, 0.0f, Color{255,255,255,255});
                //DrawTexturePro(g_wgpustate.whitePixel, Rectangle{0, 0, 100, 100}, Rectangle{(float)x + 0.4f,(float)y,0.05f,0.05f}, Vector2{0, 0}, 0.0f, Color{255,255,255,255});
                //EndRenderpassEx(&g_wgpustate.rstate->renderpass);
                //BeginRenderpassEx(&g_wgpustate.rstate->renderpass);
                //vboptr = vboptr_base;
                //std::cout << g_wgpustate.rstate->activeRenderPass->dsa->depthStoreOp << "\n";
                //std::cout << g_wgpustate.rstate->activeRenderPass->rca->storeOp << "\n";
                //break;
            }
        }
        end:
        ++frames;
        //std::cout << g_wgpustate.total_frames << "\n";
        uint64_t nextStmp = NanoTime();
        if(nextStmp - stmp > 100000000){
            std::cout << GetFPS() << "\n";
            stmp = NanoTime();
        }

        EndDrawing();
        //Image img = LoadImageFromTexture(g_wgpustate);
    };
    LoadFontDefault();
    #ifndef __EMSCRIPTEN__
    while(!glfwWindowShouldClose(window)){
        mainloop2(nullptr);
    }
    #else
    emscripten_set_main_loop_arg(mainloop, nullptr, 240, true);
    #endif // __EMSCRIPTEN__
}