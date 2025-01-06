#include <stdbool.h>
#include <stdio.h>


#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>      // For _isatty and _fileno
    #define PORTABLE_ISATTY _isatty
    #define PORTABLE_FILENO _fileno
#else
    #include <unistd.h>  // For isatty and fileno
    #define PORTABLE_ISATTY isatty
    #define PORTABLE_FILENO fileno
#endif

/**
 * @brief Portable function to check if a file descriptor refers to a terminal.
 *
 * @param fd The file descriptor to check.
 * @return int Returns 1 if `fd` is a terminal, 0 otherwise.
 */
bool IsATerminal(FILE *stream) {
    return PORTABLE_ISATTY(PORTABLE_FILENO(stream));
}
