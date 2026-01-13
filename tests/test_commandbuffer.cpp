#include <gtest/gtest.h>
#include <raygpu.h>

void Sync(RenderTexture target) {
    Image img = LoadImageFromTexture(target.texture);
    UnloadImage(img);
}

TEST(CommandBufferDeathTest, Lifecycle) {
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);
    
    // Assuming we can access internals or just rely on not crashing
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);
}

TEST(CommandBufferDeathTest, PreventInterleaving) {
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);
    
    RenderTexture target = LoadRenderTexture(128, 128);
    BeginTextureMode(target);
    
    // This should technically fail internally and log error, preventing interleaving.
    BeginComputepass(); 
    
    EndTextureMode();
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);
    
    Sync(target);
    UnloadRenderTexture(target);
}



TEST(CommandBufferDeathTest, CopyBufferGuard) {
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);
    
    RenderTexture target = LoadRenderTexture(128, 128);
    BeginTextureMode(target);
    
    // CopyBufferToBuffer should fail (pass active)
    DescribedBuffer b1 = {0}; // Dummy
    DescribedBuffer b2 = {0};
    CopyBufferToBuffer(&b1, &b2, 10);
    
    EndTextureMode();
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);
    
    Sync(target);
    UnloadRenderTexture(target);
}

TEST(CommandBufferDeathTest, ImplicitHandling) {
    RenderTexture target = LoadRenderTexture(128, 128);
    
    // No BeginCommandBuffer here - this should trigger implicit command buffer creation
    BeginTextureMode(target); 
    EndTextureMode(); // Should trigger implicit Submit
    
    Sync(target);
    UnloadRenderTexture(target); 
}
