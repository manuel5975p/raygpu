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

#endif // MACROS_AND_CONSTANTS
