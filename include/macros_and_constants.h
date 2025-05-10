#ifndef MACROS_AND_CONSTANTS
#define MACROS_AND_CONSTANTS

#define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}
#define callocnew(X) ((X*)calloc(1, (sizeof(X))))
#define callocnewpp(X) new (std::calloc(1, sizeof(X))) X
#ifdef __cplusplus
#define zeroinit {}
#define CLITERAL(X) X
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define cwoid
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <cstdint>
using std::malloc;
using std::fprintf;
using std::calloc;
using std::realloc;
using std::memcpy;
using std::memset;
using std::free;
#ifndef M_PI
#define M_PI 3.14159265358979323
#endif
constexpr float DEG2RAD = M_PI / 180.0;
constexpr float RAD2DEG = 180.0 / M_PI;
#else
#define zeroinit  = {0}
#define CLITERAL(X) (X)
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define cwoid void
#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)
#endif
#if defined(_MSC_VER) // || defined (__EMSCRIPTEN__)
#define TERMCTL_RESET   ""
#define TERMCTL_RED     ""
#define TERMCTL_GREEN   ""
#define TERMCTL_YELLOW  ""
#define TERMCTL_BLUE    ""
#define TERMCTL_CYAN    ""
#define TERMCTL_MAGENTA ""
#define TERMCTL_WHITE   ""
#else
#define TERMCTL_RESET   "\033[0m"
#define TERMCTL_RED     "\033[31m"
#define TERMCTL_GREEN   "\033[32m"
#define TERMCTL_YELLOW  "\033[33m"
#define TERMCTL_BLUE    "\033[34m"
#define TERMCTL_CYAN    "\033[36m"
#define TERMCTL_MAGENTA "\033[35m"
#define TERMCTL_WHITE   "\033[37m"
#endif
#ifndef TRACELOG
    #define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)
#endif

// Default shader vertex attribute names to set location points
#define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION             "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD             "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL               "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR                "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT              "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2            "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS              "vertexBoneIds"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS
#define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS          "vertexBoneWeights" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS

#define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP                 "mvp"               // model-view-projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW                "matView"           // view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION          "matProjection"     // projection matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION_VIEW     "Perspective_View"  // projection*view matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL               "matModel"          // model matrix
#define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL              "matNormal"         // normal matrix (transpose(inverse(matModelView))
#define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR               "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#define RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES       "boneMatrices"      // bone matrices
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0          "texture0"          // texture0 (texture slot active 0)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1          "texture1"          // texture1 (texture slot active 1)
#define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2          "texture2"          // texture2 (texture slot active 2)
#define RL_DEFAULT_SHADER_UNIFORM_NAME_INSTANCE_TX         "modelMatrix"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_NAME_INSTANCE_TX

#define RL_MAX_SHADER_LOCATIONS 32
#define VULKAN_BACKEND_ONLY

#if defined(_MSC_VER) || defined(_MSC_FULL_VER) 
    #define rg_unreachable(...) __assume(false)
    #define rg_assume(Condition) __assume(Condition)
    #define rg_trap(...) __debugbreak();
#else 
    #define rg_unreachable(...) __builtin_unreachable()
    #define rg_assume(Condition) __builtin_assume(Condition)
    #define rg_trap(...) __builtin_trap();
#endif

#if defined(RG_STATIC) && RG_STATIC != 0
    #ifdef __cplusplus
    #define RGAPI extern "C"
    #else
    #define RGAPI
    #endif
    #define RGAPICXX extern "C"
#elif defined(_WIN32)
    #if defined(RG_EXPORTS) && RG_EXPORTS != 0
        #define RGAPI __declspec(dllexport)
        #define RGAPICXX extern "C" __declspec(dllexport)
    #else
        #define RGAPI __declspec(dllimport)
        #define RGAPICXX extern "C" __declspec(dllimport)
    #endif
#else
    #define RGAPI __attribute__((visibility("default")))
    #define RGAPICXX extern "C" __attribute__((visibility("default")))
#endif


#if RAYGPU_DISABLE_ASSERT == 1

#define rassert(Condition, Message) __builtin_assume(Condition)

#else
#define rassert(Condition, Message, ...)                                                          \
do {                                                                                              \
    char buffer_for_snprintf_sdfsd[2048] = {0};                                                   \
    snprintf(buffer_for_snprintf_sdfsd, 2048, "Assertion failed: %s", Message);                   \
    if (!(Condition)) {                                                                           \
        TRACELOG(LOG_ERROR, buffer_for_snprintf_sdfsd, ##__VA_ARGS__);                          \
        TRACELOG(LOG_ERROR, "Condition: %s", #Condition);                                       \
        TRACELOG(LOG_ERROR, "Location: %s:%d", __FILE__, __LINE__);                             \
        rg_trap();                                                                                \
        abort();                                                                                  \
    }                                                                                             \
} while (0)

#endif

typedef uint32_t Bool32;

#endif // MACROS_AND_CONSTANTS
