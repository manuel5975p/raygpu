/*******************************************************************************************
*
*   raylib [core] example - Doing skinning on the gpu using a vertex shader
* 
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Daniel Holden (@orangeduck) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024 Daniel Holden (@orangeduck)
* 
*   Note: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
********************************************************************************************/

#include <raygpu.h>

constexpr char shaderSource[] = R"(

@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f;
@group(0) @binding(1) var colDiffuse: texture_2d<f32>;
@group(0) @binding(2) var texSampler: sampler;
@group(0) @binding(3) var<storage> modelMatrix: array<mat4x4f>;
@group(0) @binding(4) var<storage> boneMatrices: array<mat4x4f>;

// Vertex Input
struct VSInput {
    @location(0) position: vec3<f32>,
    @location(1) texCoord: vec2<f32>,
    @location(2) normal: vec3<f32>,
    @location(3) color: vec4<f32>,
    @location(4) boneWeights: vec4<f32>,
    @location(5) boneIds: vec4<u32>,
};

// Vertex Output
struct VSOutput {
    @builtin(position) Position: vec4<f32>,
    @location(0) fragTexCoord: vec2<f32>,
    @location(1) fragColor: vec4<f32>,
    @location(2) fragNormal: vec3<f32>,
};

@vertex
fn vs_main(input: VSInput) -> VSOutput {
    let pos =  boneMatrices[i32(input.boneIds.x)] * vec4<f32>(input.position, 1.0f) * input.boneWeights.x +
               boneMatrices[i32(input.boneIds.y)] * vec4<f32>(input.position, 1.0f) * input.boneWeights.y +
               boneMatrices[i32(input.boneIds.z)] * vec4<f32>(input.position, 1.0f) * input.boneWeights.z +
               boneMatrices[i32(input.boneIds.w)] * vec4<f32>(input.position, 1.0f) * input.boneWeights.w;
    let norm = boneMatrices[i32(input.boneIds.x)] * vec4<f32>(input.normal  , 0.0f) * input.boneWeights.x +
               boneMatrices[i32(input.boneIds.y)] * vec4<f32>(input.normal  , 0.0f) * input.boneWeights.y +
               boneMatrices[i32(input.boneIds.z)] * vec4<f32>(input.normal  , 0.0f) * input.boneWeights.z +
               boneMatrices[i32(input.boneIds.w)] * vec4<f32>(input.normal  , 0.0f) * input.boneWeights.w;
    //let pos =  vec4f(input.position, 1.0f);
    //let norm = vec4f(input.normal, 0.0f);
    var out: VSOutput;
    out.Position = Perspective_View * pos;
    out.fragTexCoord = input.texCoord;
    out.fragColor = input.color;
    out.fragNormal = normalize((Perspective_View * vec4<f32>(norm)).xyz);
    return out;
}

// Fragment Output
struct FSOutput {
    @location(0) finalColor: vec4<f32>,
};

@fragment
fn fs_main(@location(0) fragTexCoord: vec2<f32>, 
           @location(1) fragColor: vec4<f32>,
           @location(2) fragNormal: vec3<f32>) -> FSOutput {
    let texel = textureSample(colDiffuse, texSampler, fragTexCoord);
    var out: FSOutput;
    out.finalColor = texel * fragColor;
    out.finalColor = vec4f(abs(fragNormal), 1.0f);
    return out;
}
)";

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void){
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;
    //SetConfigFlags(FLAG_HEADLESS);
    InitWindow(screenWidth, screenHeight, "raylib [models] example - GPU skinning");

    // Define the camera to look into our 3d world
    Camera camera zeroinit;
    camera.position = CLITERAL(Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
    camera.target = CLITERAL(Vector3){ 0.0f, 0.0f, 0.0f };   // Camera looking at point
    camera.up = CLITERAL(Vector3){ 0.0f, 1.0f, 0.0f };       // Camera up vector (rotation towards target)
    camera.fovy = 1.0f;                                      // Camera field-of-view Y
    //camera.projection = CAMERA_PERSPECTIVE;                // Camera projection type

    // Load gltf model
    const char* dp = FindDirectory("resources", 3);
    Model characterModel = LoadModel(TextFormat("%s/greenman.glb", dp)); // Load character model
    for(int i = 0;i < characterModel.meshCount;i++){
        UploadMesh(characterModel.meshes + i, true);
    }
    // Load skinning shader
    DescribedPipeline* skinningShader = LoadPipelineForVAO(shaderSource, characterModel.meshes[0].vao);
    Mesh& mr = characterModel.meshes[0];
    SetPipelineSampler(skinningShader, 2, LoadSampler(repeat, linear));
    characterModel.materials[1].pipeline = skinningShader;
    
    // Load gltf model animations
    int animsCount = 0;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations(TextFormat("%s/greenman.glb", dp), &animsCount);

    SetPipelineTexture(skinningShader, GetUniformLocation(skinningShader, "colDiffuse"), GetDefaultTexture());
    Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position
    

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        //UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        
        // Select current animation
        if (IsKeyPressed(KEY_T)) {animIndex = (animIndex + 1)%animsCount;TRACELOG(LOG_INFO, "advanced");}
        else if (IsKeyPressed(KEY_G)) animIndex = (animIndex + animsCount - 1)%animsCount;
        
        // Update model animation
        ModelAnimation anim = modelAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        characterModel.transform = MatrixTranslate(position.x, position.y, position.z);
        UpdateModelAnimationBones(characterModel, anim, animCurrentFrame);
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(DARKBROWN);
            BeginPipelineMode(skinningShader);
            BeginMode3D(camera);
                
                // Draw character mesh, pose calculation is done in shader (GPU skinning)
                BufferData(&characterModel.meshes[0].boneMatrixBuffer, characterModel.meshes[0].boneMatrices, sizeof(Matrix) * characterModel.meshes[0].boneCount);
                SetStorageBuffer(4, &characterModel.meshes[0].boneMatrixBuffer);
                SetTexture(1, GetDefaultTexture());
                //DrawMesh(cb, Material{}, MatrixIdentity());
                DrawMesh(characterModel.meshes[0], characterModel.materials[1], MatrixIdentity());
                //std::cout << characterModel.transform << "\n";

                //DrawGrid(10, 1.0f);
                
            EndMode3D();
            EndPipelineMode();

            DrawText("Use the T/G to switch animation", 10, 10, 30, LIGHTGRAY);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    //UnloadModelAnimations(modelAnimations, animsCount); // Unload model animation
    //UnloadModel(characterModel);    // Unload model and meshes/material
    //UnloadShader(skinningShader);   // Unload GPU skinning shader
    //CloseWindow();                  // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}