#ifndef MACROS_AND_CONSTANTS
#define MACROS_AND_CONSTANTS

#define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}

#ifdef __cplusplus
#define zeroinit {}
#define CLITERAL(X) X
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define cwoid
#include <cstdlib>
#include <cstring>
using std::malloc;
using std::calloc;
using std::realloc;
using std::memcpy;
using std::memset;
using std::free;
#else
#define zeroinit  = {0}
#define CLITERAL(X) (X)
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define cwoid void
#endif
#endif // MACROS_AND_CONSTANTS
