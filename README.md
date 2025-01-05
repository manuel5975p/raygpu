# RayGPU
A fast and simple [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) based graphics library for C and C++, inspired by and based on [raylib](https://github.com/raysan5/raylib/)
### Why WebGPU
[WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) is a new graphics API meant to give portable access to most GPU features, able to run on DirectX 11/12, Vulkan, OpenGL, Metal platforms and most importantly in [browsers](https://caniuse.com/webgpu). Web support is done by [Emscripten](https://emscripten.org/).
- **Pros**
  - Direct but portable support for OpenGL(ES), Vulkan, DirectX 12 and Metal
  - Web support for compute shaders and storage buffers
  - No global context Multi-windowing and multithreading support, 

- **Cons**
  - Not as lightweight and ubiquitous as OpenGL
  - Still in development

# Roadmap
- [x] Basic Windowing
- [x] Basic Shapes
- [x] Textures and Render Targets (RenderTexture)
- [x] Screenshotting and Recording
- [x] Text Rendering
- [x] 2D and 3D Camera and Model Support 
- [x] Shaders / Pipelines
- [x] Compute Shaders / Pipelines
- [x] Multisampling
- [x] **Multiple Windows and Headless mode**
- [ ] Proper animation support
- [ ] IQM / VOX support

# Getting Started
For building instructions, see [Building](#building) <br>
For shaders and buffers, see [Shaders and Buffers](#shaders-and-buffers)
### The first window
Opening a window and drawing on it is equivalent to raylib. However
- `BeginDrawing()` **must** be called before drawing the frame.

```c
#include <raygpu.h>
int main(){
    InitWindow(800, 600, "Title");
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(GREEN);
        DrawText("Hello there!",200, 200, 30, BLACK);
        EndDrawing();
    }
}
```
More examples can be found in the [Advanced Examples Section](#more-advanced-examples)
___
### Rewriting the main loop for emscripten
It is highly advisable to use `emscripten_set_main_loop` for Web-Targeting programs. This gives control back to the browser in a deterministic and efficient way to control framerate and its own event loop. 
```c
#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
void mainloop(){
    BeginDrawing();
    ClearBackground(GREEN);
    DrawText("Hello there!",200, 200, 30, BLACK);
    EndDrawing();
}

int main(){
    InitWindow(800, 600, "Title");
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(mainloop, 0, 0);
    #else 
    while(!WindowShouldClose()){
        mainloop();
    }
    #endif
}
```
### Building
CMake is required for all platforms.
___
#### Building for Linux
```bash
git clone <repo>
cd raygpu
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release # optionally: -GNinja

make -j8 # or ninja i.a.
./examples/core_window
```
___
#### Building for Windows
```bash
git clone <repo>
cd raygpu
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
```
See the complete [list of generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for older Visual Studio versions.
# OpenGL Compatibility and Similarity
## Shaders and Buffers
Shaders work a little bit different in WebGPU compared to OpenGL.

Instead of an active shaders, WebGPU has an active pipeline. This Pipeline is an immutable object composed of:
- Vertex Buffer Layouts (Where which attribute is)
- Bindgroup Layouts (Where which uniforms are)
- Blend Mode
- Cull Mode
- Depth testing mode
- Shader Module containing both vs and fs code

The Vertex Buffer Layouts are especially tricky since in OpenGL "Vertex Array Objects" are responsible for attribute offsets, strides and what buffer they reside in.

Nevertheless, it's still possible to load a Pipeline with a single line
```c
const char shaderSource[] = "...";
DescribedPipeline* pl = LoadPipeline(shaderSource);
```
## Setting Uniforms
The shader sourse might contain a uniform or storage binding entry:
```wgsl
@group(0) @binding(0) var<uniform> Perspective_View: mat4x4f
```
Currently, RayGPU supports only one bindgroup, which is bindgroup 0.
`@group(0) @binding(0)` can be viewed as `layout(location = 0)` from GLSL. Setting this uniform can be achieved as follows:
```c
Matrix pv = MatrixIdentity();
SetPipelineUniformBufferData(pl, &pv, sizeof(Matrix));
```
## Drawing Vertex Arrays
```c
float vertices[6] = {
    0,0,
    1,0,
    0,1
};
DescribedBuffer vbo = GenBuffer(vertices, sizeof(vertices)); 
VertexArray* vao = LoadVertexArray();
VertexAttribPointer(
    vao, 
    &vbo, 
    0,//<-- Attribute Location 
    WGPUVertexFormat_Float32x2, 
    0,//<-- Offset
    WGPUVertexStepMode_Vertex //or WGPUVertexStepMode_Instance
);
EnableVertexAttribArray(vao, 0);

//Afterwards:
BindVertexArray(pipeline, vao);
DrawArrays(WGPUPrimitiveTopology_TriangleList, 3);
```

See the example [pipeline_basic.c](https://github.com/manuel5975p/raygpu/tree/oofbranch/examples/pipeline_basic.c) for a complete example.
# More Advanced Examples
## Headless Window
```c
#include <raygpu.h>

int main(void){

    SetConfigFlags(FLAG_HEADLESS);
    InitWindow(800, 600, "This title has no effect");
    
    Texture tex = LoadTextureFromImage(GenImageChecker(WHITE, BLACK, 100, 100, 10));
    SetTargetFPS(0);
    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color) {20,50,50,255});
        
        DrawRectangle(100,100,100,100,WHITE);
        DrawTexturePro(
            tex, 
            (Rectangle){0,0,100,100}, 
            (Rectangle){200,100,100,100}, 
            (Vector2){0,0}, 
            0.0f, 
            WHITE
        );
        DrawCircle(GetMouseX(), GetMouseY(), 40, WHITE);
        DrawCircleV(GetMousePosition(), 20, (Color){255,0,0,255});
        DrawCircle(880, 300, 50, (Color){255,0,0,100});
        
        DrawFPS(0, 0);
        EndDrawing();
        if(GetFrameCount() % 128 == 0){ //Export every 128th frame
            TakeScreenshot(TextFormat("frame%llu.png", GetFrameCount()));
        }
    }
}
```
___
More examples can be found in [here](https://github.com/manuel5975p/raygpu/tree/oofbranch/examples).