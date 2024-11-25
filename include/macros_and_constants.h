#ifndef MACROS_AND_CONSTANTS
#define MACROS_AND_CONSTANTS

#define STRVIEW(X) WGPUStringView{X, sizeof(X) - 1}

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#define cwoid
#include <cstdlib>
#else
#include <stdlib.h>
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define cwoid void
#endif
#endif // MACROS_AND_CONSTANTS