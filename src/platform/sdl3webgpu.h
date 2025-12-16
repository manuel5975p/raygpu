// begin file src/sdl3webgpu.h
#ifndef _sdl3_webgpu_h_
#define _sdl3_webgpu_h_
#ifdef SUPPORT_VULKAN_BACKEND
#include <wgvk.h>
#else
#include <webgpu/webgpu.h>
#endif
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

#endif // _sdl3_webgpu_h_


// end file src/sdl3webgpu.h