// test_shapes.cpp
#include "raygpu.h"
#include "visual_test.h"

// Define a specific test
TEST_F(VisualTest, DrawBasicCircle) {
    int width = 128;
    int height = 128;
    
    RenderTexture target = LoadRenderTexture(width, height);
    
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);

    BeginTextureMode(target);
        ClearBackground(BLUE);
        DrawCircle(width/2, height/2, 32.0f, RED);
    EndTextureMode();
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);

    // This handles Export or Compare automatically
    VerifyBuffer(target, "draw_basic_circle");
    
    UnloadRenderTexture(target);
}

TEST_F(VisualTest, DrawGradientRec) {
    RenderTexture target = LoadRenderTexture(200, 100);
    
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);

    BeginTextureMode(target);
        ClearBackground(BLACK);
        DrawRectangleGradientH(0, 0, 200, 100, BLANK, GREEN);
    EndTextureMode();

    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);

    VerifyBuffer(target, "draw_gradient_rec");
    
    UnloadRenderTexture(target);
}
