#ifndef _sdl2_webgpu_h_
#define _sdl2_webgpu_h_

#include <webgpu/webgpu.h>
#include <SDL3/SDL.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get a WGPUSurface from a SDL3 window.
 */
WGPUSurface SDL3_GetWGPUSurface(WGPUInstance instance, SDL_Window* window);

#ifdef __cplusplus
}
#endif

#endif // _sdl2_webgpu_h_