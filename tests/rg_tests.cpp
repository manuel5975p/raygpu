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

TEST_F(VisualTest, DrawTextureProFlipY) {
    // Create a checkerboard texture to test flipping
    Image img = GenImageChecker(RED, BLUE, 64, 64, 8);
    Texture tex = LoadTextureFromImage(img);
    UnloadImage(img);
    
    RenderTexture target = LoadRenderTexture(128, 128);
    
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);

    BeginTextureMode(target);
        ClearBackground(BLACK);
        // Draw normal texture on left
        DrawTexturePro(tex, 
            CLITERAL(Rectangle){0, 0, 64, 64},      // source
            CLITERAL(Rectangle){0, 0, 64, 64},       // dest
            CLITERAL(Vector2){0, 0},                 // origin
            0.0f, WHITE);                            // rotation, tint
        // Draw vertically flipped texture on right
        DrawTexturePro(tex, 
            CLITERAL(Rectangle){0, 0, 64, -64},      // negative height for vertical flip
            CLITERAL(Rectangle){64, 0, 64, 64},      // dest
            CLITERAL(Vector2){0, 0},                 // origin
            0.0f, WHITE);                            // rotation, tint
    EndTextureMode();
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);

    VerifyBuffer(target, "draw_texture_pro_flip_y");
    
    UnloadTexture(tex);
    UnloadRenderTexture(target);
}

TEST_F(VisualTest, DrawTextureProFlipX) {
    // Create a checkerboard texture to test flipping
    Image img = GenImageChecker(RED, BLUE, 64, 64, 8);
    Texture tex = LoadTextureFromImage(img);
    UnloadImage(img);
    
    RenderTexture target = LoadRenderTexture(128, 128);
    
    CommandBuffer cb = {0};
    BeginCommandBuffer(&cb);

    BeginTextureMode(target);
        ClearBackground(BLACK);
        // Draw normal texture on top
        DrawTexturePro(tex, 
            CLITERAL(Rectangle){0, 0, 64, 64},      // source
            CLITERAL(Rectangle){0, 0, 64, 64},       // dest
            CLITERAL(Vector2){0, 0},                 // origin
            0.0f, WHITE);                            // rotation, tint
        // Draw horizontally flipped texture on bottom
        DrawTexturePro(tex, 
            CLITERAL(Rectangle){0, 0, -64, 64},      // negative width for horizontal flip
            CLITERAL(Rectangle){0, 64, 64, 64},      // dest
            CLITERAL(Vector2){0, 0},                 // origin
            0.0f, WHITE);                            // rotation, tint
    EndTextureMode();
    
    EndCommandBuffer(&cb);
    SubmitCommandBuffer(&cb);

    VerifyBuffer(target, "draw_texture_pro_flip_x");
    
    UnloadTexture(tex);
    UnloadRenderTexture(target);
}
