// begin file include/macros_and_constants.h
#ifndef MACROS_AND_CONSTANTS
#define MACROS_AND_CONSTANTS

#ifdef __cplusplus
    #define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}
#else
    #define STRVIEW(X) (WGPUStringView){X, sizeof(X) - 1}
#endif

#define callocnew(X) ((X*)calloc(1, (sizeof(X))))
#define callocnewpp(X) new (std::calloc(1, sizeof(X))) X
#ifdef __cplusplus
    #define CLITERAL(X) X
    #define EXTERN_C_BEGIN extern "C" {
    #define EXTERN_C_END }
    #define cwoid

    #ifndef M_PI
        #define M_PI 3.14159265358979323
    #endif
    constexpr float DEG2RAD = 3.14159265358979323 / 180.0;
    constexpr float RAD2DEG = 180.0 / 3.14159265358979323;
#else
    #define CLITERAL(X) (X)
    #define EXTERN_C_BEGIN
    #define EXTERN_C_END
    #define cwoid void
    #define DEG2RAD (3.14159265358979323 / 180.0)
    #define RAD2DEG (180.0 / 3.14159265358979323)
#endif
#if defined(_MSC_VER) || defined(_WIN32) // || defined (__EMSCRIPTEN__)
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

#define rg_countof(X) (sizeof(X) / sizeof((X)[0]))

#if defined(RG_STATIC) && RG_STATIC != 0
    #ifdef __cplusplus
        #define RGAPI extern "C"
    #else
        #define RGAPI
    #endif
#elif defined(_WIN32)
    #if defined(RG_EXPORTS) && RG_EXPORTS != 0
        #ifdef __cplusplus
            #define RGAPI extern "C" __declspec(dllexport)
        #else
            #define RGAPI __declspec(dllexport)
        #endif
    #else
        #ifdef __cplusplus
            #define RGAPI extern "C" __declspec(dllimport)
        #else
            #define RGAPI __declspec(dllimport)
        #endif
    #endif
#else
    #ifdef __cplusplus
        #define RGAPI extern "C" __attribute__((visibility("default")))
    #else
        #define RGAPI __attribute__((visibility("default")))
    #endif
#endif


#if RAYGPU_DISABLE_ASSERT == 1

#define rassert(Condition, Message, ...) //__builtin_assume(Condition)

#else
#define rassert(Condition, Message, ...)                                                          \
do {                                                                                              \
    char buffer_for_snprintf_sdfsd[4096] = {0};                                                   \
    snprintf(buffer_for_snprintf_sdfsd, 4096, "Internal bug: assertion failed: %s", Message);     \
    if (!(Condition)) {                                                                           \
        TRACELOG(LOG_ERROR, buffer_for_snprintf_sdfsd, ##__VA_ARGS__);                            \
        TRACELOG(LOG_ERROR, "Condition: %s", #Condition);                                         \
        TRACELOG(LOG_ERROR, "Location: %s:%d", __FILE__, __LINE__);                               \
        rg_trap();                                                                                \
    }                                                                                             \
} while (0)

#endif

typedef unsigned Bool32;

#include <stdint.h>
#include <limits.h>

#define COUNTR_ZERO_NBITS(T) ((int)(sizeof(T) * CHAR_BIT))

#if defined(__GNUC__) || defined(__clang__)
    #define COUNTR_ZERO_BUILTIN_U32(x) ((x) == 0 ? 32 : __builtin_ctz((uint32_t)(x)))
    #define COUNTR_ZERO_BUILTIN_U64(x) ((x) == 0 ? 64 : __builtin_ctzll((uint64_t)(x)))
    #define COUNTR_ZERO_BUILTIN_U16(x) ((x) == 0 ? 16 : __builtin_ctz((uint16_t)(x)))
    #define COUNTR_ZERO_BUILTIN_U8(x)  ((x) == 0 ?  8 : __builtin_ctz((uint8_t)(x)))
#elif defined(_MSC_VER)
    #include <intrin.h>
    #pragma intrinsic(_BitScanForward)

    static inline int countr_zero_msvc_u32(uint32_t x) {
        unsigned long index;
        if (_BitScanForward(&index, x)) return (int)index;
        return 32;
    }

    #if defined(_M_X64) || defined(_WIN64)
        #pragma intrinsic(_BitScanForward64)
        static inline int countr_zero_msvc_u64(uint64_t x) {
            unsigned long index;
            if (_BitScanForward64(&index, x)) return (int)index;
            return 64;
        }
    #else
        static inline int countr_zero_msvc_u64(uint64_t x) {
            unsigned long index;
            uint32_t lo = (uint32_t)x;
            if (_BitScanForward(&index, lo)) return (int)index;
            uint32_t hi = (uint32_t)(x >> 32);
            if (_BitScanForward(&index, hi)) return (int)index + 32;
            return 64;
        }
    #endif

    #define COUNTR_ZERO_BUILTIN_U32(x) countr_zero_msvc_u32((uint32_t)(x))
    #define COUNTR_ZERO_BUILTIN_U64(x) countr_zero_msvc_u64((uint64_t)(x))
    #define COUNTR_ZERO_BUILTIN_U16(x) countr_zero_fallback_u16((uint16_t)(x))
    #define COUNTR_ZERO_BUILTIN_U8(x)  countr_zero_fallback_u8 ((uint8_t)(x))
#else
    #define COUNTR_ZERO_BUILTIN_U32(x) countr_zero_fallback_u32((uint32_t)(x))
    #define COUNTR_ZERO_BUILTIN_U64(x) countr_zero_fallback_u64((uint64_t)(x))
    #define COUNTR_ZERO_BUILTIN_U16(x) countr_zero_fallback_u16((uint16_t)(x))
    #define COUNTR_ZERO_BUILTIN_U8(x)  countr_zero_fallback_u8 ((uint8_t)(x))
#endif

// Fallback portable implementation with short names
#define DEFINE_COUNTR_ZERO_FALLBACK_IMPL(SHORT, TYPE, WIDTH) \
static inline int countr_zero_fallback_##SHORT(TYPE x) { \
    if (x == 0) return WIDTH; \
    int n = 0; \
    while ((x & 1) == 0) { \
        x >>= 1; \
        ++n; \
    } \
    return n; \
}

DEFINE_COUNTR_ZERO_FALLBACK_IMPL(u8,  uint8_t,  8)
DEFINE_COUNTR_ZERO_FALLBACK_IMPL(u16, uint16_t, 16)
DEFINE_COUNTR_ZERO_FALLBACK_IMPL(u32, uint32_t, 32)
DEFINE_COUNTR_ZERO_FALLBACK_IMPL(u64, uint64_t, 64)

static inline int countr_zero_u8 (uint8_t  x) { return COUNTR_ZERO_BUILTIN_U8 (x); }
static inline int countr_zero_u16(uint16_t x) { return COUNTR_ZERO_BUILTIN_U16(x); }
static inline int countr_zero_u32(uint32_t x) { return COUNTR_ZERO_BUILTIN_U32(x); }
static inline int countr_zero_u64(uint64_t x) { return COUNTR_ZERO_BUILTIN_U64(x); }

static inline int countr_zero_i8 (int8_t  x) { return countr_zero_u8 ((uint8_t)x); }
static inline int countr_zero_i16(int16_t x) { return countr_zero_u16((uint16_t)x); }
static inline int countr_zero_i32(int32_t x) { return countr_zero_u32((uint32_t)x); }
static inline int countr_zero_i64(int64_t x) { return countr_zero_u64((uint64_t)x); }

// Bit-scan intrinsics

static inline int8_t      std_max_i8 (int8_t      a, int8_t      b) {return a > b ? a : b;}
static inline int16_t     std_max_i16(int16_t     a, int16_t     b) {return a > b ? a : b;}
static inline int32_t     std_max_i32(int32_t     a, int32_t     b) {return a > b ? a : b;}
static inline int64_t     std_max_i64(int64_t     a, int64_t     b) {return a > b ? a : b;}
static inline uint8_t     std_max_u8 (uint8_t     a, uint8_t     b) {return a > b ? a : b;}
static inline uint16_t    std_max_u16(uint16_t    a, uint16_t    b) {return a > b ? a : b;}
static inline uint32_t    std_max_u32(uint32_t    a, uint32_t    b) {return a > b ? a : b;}
static inline uint64_t    std_max_u64(uint64_t    a, uint64_t    b) {return a > b ? a : b;}
static inline float       std_max_f32(float       a, float       b) {return a > b ? a : b;}
static inline double      std_max_f64(double      a, double      b) {return a > b ? a : b;}
static inline long double std_max_f80(long double a, long double b) {return a > b ? a : b;}

static inline int8_t      std_min_i8 (int8_t      a, int8_t      b) {return a < b ? a : b;}
static inline int16_t     std_min_i16(int16_t     a, int16_t     b) {return a < b ? a : b;}
static inline int32_t     std_min_i32(int32_t     a, int32_t     b) {return a < b ? a : b;}
static inline int64_t     std_min_i64(int64_t     a, int64_t     b) {return a < b ? a : b;}
static inline uint8_t     std_min_u8 (uint8_t     a, uint8_t     b) {return a < b ? a : b;}
static inline uint16_t    std_min_u16(uint16_t    a, uint16_t    b) {return a < b ? a : b;}
static inline uint32_t    std_min_u32(uint32_t    a, uint32_t    b) {return a < b ? a : b;}
static inline uint64_t    std_min_u64(uint64_t    a, uint64_t    b) {return a < b ? a : b;}
static inline float       std_min_f32(float       a, float       b) {return a < b ? a : b;}
static inline double      std_min_f64(double      a, double      b) {return a < b ? a : b;}
static inline long double std_min_f80(long double a, long double b) {return a < b ? a : b;}

static inline int8_t      std_clamp_i8 (int8_t      x, int8_t      lo, int8_t      hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline int16_t     std_clamp_i16(int16_t     x, int16_t     lo, int16_t     hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline int32_t     std_clamp_i32(int32_t     x, int32_t     lo, int32_t     hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline int64_t     std_clamp_i64(int64_t     x, int64_t     lo, int64_t     hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline uint8_t     std_clamp_u8 (uint8_t     x, uint8_t     lo, uint8_t     hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline uint16_t    std_clamp_u16(uint16_t    x, uint16_t    lo, uint16_t    hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline uint32_t    std_clamp_u32(uint32_t    x, uint32_t    lo, uint32_t    hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline uint64_t    std_clamp_u64(uint64_t    x, uint64_t    lo, uint64_t    hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline float       std_clamp_f32(float       x, float       lo, float       hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline double      std_clamp_f64(double      x, double      lo, double      hi) {return x < lo ? lo : (x > hi ? hi : x);}
static inline long double std_clamp_f80(long double x, long double lo, long double hi) {return x < lo ? lo : (x > hi ? hi : x);}


#endif // MACROS_AND_CONSTANTS

// end file include/macros_and_constants.h