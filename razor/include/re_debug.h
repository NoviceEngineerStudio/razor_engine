#ifndef __RAZOR_DEBUG_HEADER_FILE
#define __RAZOR_DEBUG_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#endif

#include "./re_core.h"

// *=================================================
// *
// * Logging
// *
// *=================================================

/// @brief Internal logging method (DO NOT USE DIRECTLY).
/// @param format The logging format.
/// @param ... The variadic logging arguments.
RE_API void __re_internalLog(const char* format, ...);

/// @brief Close the logger's currently open file (does nothing if one is not open).
RE_API void re_closeLogFile();

/// @brief Set the logger to output to some file.
/// @param file_path The path of the file.
RE_API void re_setLogFile(const char* file_path);

#ifdef RE_LOGGER_ENABLED
    #define re_logInfo(format, ...) __re_internalLog("%s"format"\n", "[INFO]: ", ##__VA_ARGS__)

    #define re_logDebug(format, ...) __re_internalLog("%s"format"\n", "[DEBUG]: ", ##__VA_ARGS__)

    #define re_logSuccess(format, ...) __re_internalLog("%s"format"\n", "[SUCCESS]: ", ##__VA_ARGS__)

    #define re_logWarn(format, ...) __re_internalLog("%s%d%s"format"\n", "[WARN]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__)

    #define re_logError(format, ...) __re_internalLog("%s%d%s"format"\n", "[ERROR]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__)

    #define re_logFatal(format, ...) __re_internalLog("%s%d%s"format"\n", "[FATAL]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__)
#else
    #define re_logInfo(format, ...) ((void)(0))
    #define re_logDebug(format, ...) ((void)(0))
    #define re_logSuccess(format, ...) ((void)(0))

    #define re_logWarn(format, ...) ((void)(0))
    #define re_logError(format, ...) ((void)(0))
    #define re_logFatal(format, ...) ((void)(0))
#endif

// *=================================================
// *
// * Runtime Assert
// *
// *=================================================

#ifdef RE_ASSERT_ENABLED
    #include <stdlib.h>

    #define re_assert(condition, format, ...) do { \
        if (!(condition)) { \
            re_logFatal("%s" format, "Assertion failed (" #condition ") ", ##__VA_ARGS__); \
            abort(); \
        } \
    } while(0)
#else
    #define re_assert(condition, format, ...) ((void)(0))
#endif

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif