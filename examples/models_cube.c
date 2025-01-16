#include <raygpu.h>
#include <stdio.h>
#include <assert.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Camera3D cam;
Mesh cube;
DescribedPipeline* pl;
Texture checkers;
Texture checkersHDR;
float angle;

// Ensure no padding is added by the compiler
#pragma pack(push, 1)
typedef struct {
    uint16_t fraction :10; // 10 bits for fraction
    uint16_t exponent :5;  // 5 bits for exponent
    uint16_t sign     :1;  // 1 bit for sign
} IEEE_FP16;
#pragma pack(pop)
IEEE_FP16 float_to_fp16(float value) {
    IEEE_FP16 fp16;
    uint32_t f_bits;

    // Copy the bits of the float into an integer
    memcpy(&f_bits, &value, sizeof(f_bits));

    // Extract sign (1 bit)
    uint16_t sign = (f_bits >> 31) & 0x1;

    // Extract exponent (8 bits) and adjust bias from 127 to 15
    int32_t exponent = ((f_bits >> 23) & 0xFF) - 127;

    // Extract fraction (23 bits)
    uint32_t fraction = f_bits & 0x7FFFFF;

    if (exponent == 128) { // Handle Inf and NaN
        fp16.sign = sign;
        fp16.exponent = 0x1F; // All 5 exponent bits set
        fp16.fraction = (fraction != 0) ? 0x200 : 0; // Set MSB of fraction for NaN
    }
    else if (exponent > 15) { // Overflow, set to Inf
        fp16.sign = sign;
        fp16.exponent = 0x1F;
        fp16.fraction = 0;
    }
    else if (exponent >= -14) { // Normalized number
        fp16.sign = sign;
        fp16.exponent = exponent + 15;
        fp16.fraction = fraction >> 13; // Truncate to 10 bits
    }
    else if (exponent >= -24) { // Subnormal number
        fp16.sign = sign;
        fp16.exponent = 0;
        // Add implicit leading 1 to fraction and shift
        fp16.fraction = (fraction | 0x800000) >> (14 - exponent);
    }
    else { // Underflow, set to zero
        fp16.sign = sign;
        fp16.exponent = 0;
        fp16.fraction = 0;
    }

    return fp16;
}
void mainloop(void){
    BeginDrawing();
    angle += GetFrameTime();
    cam.position = (Vector3){sinf(angle) * 10.f, 5.0f, cosf(angle) * 10.f};
    ClearBackground(BLACK);
    //TODO: Swapping the next two causes a problem since the BindGroup is lazily updated only at BindPipeline
    //EDIT: It's not due to lazy update; DrawArrays and DrawArraysIndexed did not check for a pending Bindgroup Update
    BeginPipelineMode(pl);
    UseTexture(checkersHDR);
    BeginMode3D(cam);
    BindVertexArray(cube.vao);
    DrawArraysIndexed(WGPUPrimitiveTopology_TriangleList, *cube.ibo, 36);
    EndMode3D();
    EndPipelineMode();
    DrawFPS(0, 0);
    if(IsKeyPressed(KEY_U)){
        ToggleFullscreen();
    }
    EndDrawing();
}
int main(cwoid){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1200, 800, "VAO");
    SetTargetFPS(300);
    cam = CLITERAL(Camera3D){
        .position = CLITERAL(Vector3){0,0,10},
        .target = CLITERAL(Vector3){0,0,0},
        .up = CLITERAL(Vector3){0,1,0},
        .fovy = 45.0f
    };
    cube = GenMeshCube(3.f,3.f,3.f);
    //assert(cube.ibo.buffer == 0);
    pl = Relayout(DefaultPipeline(), cube.vao);
    checkersHDR = LoadTextureEx(9, 9, WGPUTextureFormat_RGBA16Float, false);

    checkers = LoadTextureFromImage(GenImageChecker(RED, DARKBLUE, 100, 100, 4));
    IEEE_FP16 checkersHDRData[81 * 4];
    for(size_t i = 0;i < 81;i++){
        checkersHDRData[4 * i + 0] = (i & 1) ? float_to_fp16(.0f) : float_to_fp16(0.5f);
        checkersHDRData[4 * i + 1] = (i & 1) ? float_to_fp16(0.0f) : float_to_fp16(0.0f);
        checkersHDRData[4 * i + 2] = (i & 1) ? float_to_fp16(0.0f) : float_to_fp16(0.0f);
        checkersHDRData[4 * i + 3] = (i & 1) ? float_to_fp16(1.0f) : float_to_fp16(1.0f);
    }
    UpdateTexture(checkersHDR, checkersHDRData);
    angle = 0.0f;

    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    #else
    emscripten_set_main_loop(mainloop, 0, 0);
    #endif
}
