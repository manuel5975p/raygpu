#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED


#ifndef SUPPORT_WGSL_PARSER
    #define SUPPORT_WGSL_PARSER 1
#endif

#ifndef SUPPORT_GLSL_PARSER
    #define SUPPORT_GLSL_PARSER 0
#endif
#ifndef SUPPORT_SDL2
    #define SUPPORT_SDL2 0
#endif
#ifndef SUPPORT_GLFW
    #define SUPPORT_GLFW 0
#endif
#if SUPPORT_SDL3 == 0 && SUPPORT_SDL2 == 0 && SUPPORT_GLFW == 0
    #define FORCE_HEADLESS 1
#endif
#if SUPPORT_SDL3 == 1// && SUPPORT_SDL2 == 0 && SUPPORT_GLFW == 0
    #define MAIN_WINDOW_SDL3
#elif SUPPORT_GLFW == 0 && SUPPORT_SDL2 == 1
    #define MAIN_WINDOW_SDL2
#elif SUPPORT_GLFW == 1 && SUPPORT_SDL2 == 0
    #define MAIN_WINDOW_GLFW
#elif SUPPORT_GLFW == 1 && SUPPORT_SDL2 == 1
    #define MAIN_WINDOW_SDL2
#else

#endif
//#if !defined(MAIN_WINDOW_SDL2) && !defined(MAIN_WINDOW_GLFW)
//    #define MAIN_WINDOW_GLFW
//#endif
#if defined(MAIN_WINDOW_SDL2) && defined(MAIN_WINDOW_GLFW)
#error only_one_main_window_type_is_supported
#endif


// Detect and define DEFAULT_BACKEND based on the target platform
#if defined(_WIN32) || defined(_WIN64)
    // Windows platform detected
    #define DEFAULT_BACKEND WGPUBackendType_D3D12

#elif defined(__APPLE__) && defined(__MACH__)
    // Apple platform detected (macOS, iOS, etc.)
    #define DEFAULT_BACKEND WGPUBackendType_Metal

#elif defined(__linux__) || defined(__unix__) || defined(__FreeBSD__)
    // Linux or Unix-like platform detected
    #define DEFAULT_BACKEND WGPUBackendType_Vulkan

#else
    // Fallback to Vulkan for any other platforms
    #define DEFAULT_BACKEND WGPUBackendType_Vulkan
    #pragma message("Unknown platform. Defaulting to Vulkan as the backend.")
#endif





// The RENDERBATCH_SIZE is how many vertices can be batched at most
// It must be a multiple of 12 to guarantee that RL_LINES, RL_TRIANGLES and RL_QUADS 
// trigger an overflow on a completed shape. 
// Because of that, it needs to be a multiple of both 3 and 4

#ifndef RENDERBATCH_SIZE_MULTIPLIER
    #define RENDERBATCH_SIZE_MULTIPLIER 200
#endif

#define RENDERBATCH_SIZE (RENDERBATCH_SIZE_MULTIPLIER * 12)


#endif // CONFIG_H_INCLUDED
