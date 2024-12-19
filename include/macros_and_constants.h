#ifndef MACROS_AND_CONSTANTS
#define MACROS_AND_CONSTANTS

#define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}
#define callocnew(X) ((X*)calloc(1, (sizeof(X))))
#ifdef __cplusplus
#define zeroinit {}
#define CLITERAL(X) X
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define cwoid
#include <cstdlib>
#include <cstring>
#include <cmath>
using std::malloc;
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
#include <math.h>
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define cwoid void
#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)
#endif
#if defined(_MSC_VER)
#define TERMCTL_RESET   ""
#define TERMCTL_RED     ""
#define TERMCTL_GREEN   ""
#define TERMCTL_YELLOW  ""
#define TERMCTL_BLUE    ""
#define TERMCTL_CYAN    ""
#define TERMCTL_MAGENTA ""
#define TERMCTL_WHITE   ""
#pragma message("Colors not supported in MSVC terminal")
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

#define TRACELOG(level, ...) TraceLog(level, __VA_ARGS__)

#endif // MACROS_AND_CONSTANTS
