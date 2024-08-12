#include <webgpu/webgpu_cpp.h>
#include <iostream>
#include "GLFW/glfw3.h"
#include <raygpu.h>
#include <stb_image_write.h>
#ifndef __EMSCRIPTEN__
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "webgpu/webgpu_glfw.h"
#else
#include <emscripten/html5.h>
#include <emscripten/emscripten.h>
#endif  // __EMSCRIPTEN__


#include "wgpustate.inc"
Texture checkers;
RenderTexture rtex;

uint64_t frames = 0;
uint64_t stmp = NanoTime();
int main(){
    constexpr uint32_t width = 1000;
    constexpr uint32_t height = 1000;
    auto window = InitWindow(width, height);
    
        
    //Matrix udata = MatrixLookAt(Vector3{0,0,0.2}, Vector3{0,0,0}, Vector3{0,1,0});
    //Matrix udata = MatrixIdentity();

    //Matrix udata = (MatrixPerspective(1.2, 1, 0.01, 100.0));
    checkers = LoadTextureFromImage(LoadImageChecker(Color{230, 230, 230, 255}, Color{100, 100, 100, 255}, 100, 100, 50));

    constexpr int rtd = 2000;
    rtex = LoadRenderTexture(rtd, rtd);

    auto mainloop = [](void* userdata){
        float z = 0;
        Matrix udata = MatrixMultiply(MatrixPerspective(1.0, double(GetScreenWidth()) / GetScreenHeight(), 0.01, 100.0), MatrixLookAt(Vector3{0, 0, -2.5}, Vector3{0, 0, 0}, Vector3{0,1,0}));
        SetUniformBuffer(0, &udata, 64 * sizeof(float));
        BeginDrawing();
        /*UseTexture(checkers);
        BeginTextureMode(rtex);
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
        EndTextureMode();*/
        //UseTexture(rtex.color);
        udata = ScreenMatrix(GetScreenWidth(), GetScreenHeight());
        SetUniformBuffer(0, &udata, 16 * sizeof(float));
        for(size_t i = 0;i < 1000;i++){
            DrawTexturePro(
                g_wgpustate.whitePixel,
                Rectangle(0, 0, 1, 1), Rectangle(i * 3, 10,20,20),
                Vector2(0,0), 
                g_wgpustate.total_frames * (0.0f / 1000.0f),
                Color{255,255,255,255}
            );
        }

        
        //Image img = LoadImageFromTexture(rtex.color);
        //std::cout << img.format << "\n";
        //if(total_frames == 3)
        //    stbi_write_png("ass.png", img.width, img.height, 4, img.data, std::ceil(4.0 * img.width / 256.0) * 256);
        //std::free(img.data);
        EndDrawing();
        
        ++frames;
        uint64_t nextStmp = NanoTime();
        if(nextStmp - stmp > 1000000000){
            std::cout << GetFPS() << "\n";
            stmp = NanoTime();
        }

    };
    #ifndef __EMSCRIPTEN__
    while(!glfwWindowShouldClose(window)){
        mainloop(nullptr);
    }
    #else
    emscripten_set_main_loop_arg(mainloop, nullptr, 240, true);
    #endif // __EMSCRIPTEN__
}