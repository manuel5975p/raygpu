#include <raygpu.h>
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