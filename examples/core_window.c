#include <raygpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>

typedef void (*FrameCallback)();
#ifdef __EMSCRIPTEN__
// Workaround for JSPI not working in emscripten_set_main_loop. Loosely based on this code:
// https://github.com/emscripten-core/emscripten/issues/22493#issuecomment-2330275282
// This code only works with JSPI is enabled.
// I believe -sEXPORTED_RUNTIME_METHODS=getWasmTableEntry is technically necessary to link this.
EM_JS(void, requestAnimationFrameLoopWithJSPI, (FrameCallback callback), {
    var wrappedCallback = WebAssembly.promising(getWasmTableEntry(callback));
    async function tick() {
        // Start the frame callback. 'await' means we won't call
        // requestAnimationFrame again until it completes.
        //var keepLooping = await wrappedCallback();
        //if (keepLooping) requestAnimationFrame(tick);
        await wrappedCallback();
        requestAnimationFrame(tick);
    }
    requestAnimationFrame(tick);
})
#endif
#endif
void mainloop(void){
    BeginDrawing();
    ClearBackground((Color){230, 230, 230,255});
    DrawText("Hello WebGPU enjoyer", 100, 300, 50, (Color){190, 190, 190,255});
    DrawFPS(5, 5);
    if(IsKeyPressed(KEY_U)){
        ToggleFullscreen();
    }
    EndDrawing();
}
int main(void){
    //SetConfigFlags(FLAG_STDOUT_TO_FFMPEG);
    InitWindow(1920, 1080, "WebGPU window");
    SetTargetFPS(0);
    #ifndef __EMSCRIPTEN__
    while(!WindowShouldClose()){
        mainloop();
    }
    
    //CopyTextureToTexture
    #else
    requestAnimationFrameLoopWithJSPI(mainloop);
    //emscripten_set_main_loop(mainloop, 0, 0);
    #endif
    
}
