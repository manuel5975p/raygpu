#include <raygpu.h>
void UnloadTexture(Texture tex){
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