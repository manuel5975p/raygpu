# RayGPU
A fast and simple [WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) based graphics library for C and C++, inspired by and based on [raylib](https://github.com/raysan5/raylib/)
### Why WebGPU
[WebGPU](https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API) is a new (still in development) graphics API meant to give portable access to most GPU features, able to run on DirectX, Vulkan, OpenGL, Metal platforms and most importantly in [several modern browsers](https://caniuse.com/webgpu). Web support is done by [Emscripten](https://emscripten.org/).
# Getting Started
### The first window
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
### Building
CMake is required for all platforms.
___
```bash
git clone <repo>
cd raygpu
mkdir build && cd build
cmake .. # optionally: -GNinja

make -j8 # or ninja i.a.
./examples/core_window
```


# More Advanced Examples