#ifndef __RE_MAIN_HEADER_FILE
#define __RE_MAIN_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// *=================================================
// *
// * Engine Metadata
// *
// *=================================================

#define RE_ENGINE_NAME "Razor Engine"

#define RE_ENGINE_MAJOR_VER 0
#define RE_ENGINE_MINOR_VER 0
#define RE_ENGINE_PATCH_VER 1

// *=================================================
// *
// * Platform Values
// *
// *=================================================

// ? Numbering Convention is XAA
// ? X  = Platform Type
// ? AA = Platform Identifier

// ? Desktop Platforms

#define RE_PLATFORM_WINDOWS 000
#define RE_PLATFORM_LINUX   001
#define RE_PLATFORM_MACOS   002

// ? Mobile Platforms

#define RE_PLATFORM_IOS     100
#define RE_PLATFORM_ANDROID 101

// *=================================================
// *
// * Platform Detection
// *
// *=================================================

#if defined(_WIN32) || defined(_WIN64)
    #define RE_PLATFORM RE_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define RE_PLATFORM RE_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
    #if defined(TARGET_OS_IPHONE)
        #define RE_PLATFORM RE_PLATFORM_IOS
    #elif defined(TARGET_OS_MAC)
        #define RE_PLATFORM RE_PLATFORM_MACOS
    #else
        #error "Attempting to compile for unknown Apple platform!"
    #endif
#elif defined(__ANDROID__)
    #define RE_PLATFORM RE_PLATFORM_ANDROID
#else
    #error "Attempting to compile for unknown platform!"
#endif

// *=================================================
// *
// * 64-Bit Support Detection
// *
// *=================================================

#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__aarch64__) || defined(_M_ARM64) || defined(__ppc64__)
    #define RE_X64 1
#endif

// *=================================================
// *
// * DLL Import and Export
// *
// *=================================================

#if RE_PLATFORM == RE_PLATFORM_WINDOWS
    #define RE_DLL_IMPORT __declspec(dllimport)
    #define RE_DLL_EXPORT __declspec(dllexport)
#elif RE_PLATFORM == RE_PLATFORM_LINUX
    #define RE_DLL_IMPORT
    #define RE_DLL_EXPORT __attribute__((visibility("default")))
#endif

#ifdef RE_BUILD_DLL
    #define RE_API RE_DLL_EXPORT
#else
    #define RE_API RE_DLL_IMPORT
#endif

// *=================================================
// *
// * NULL Pointer Value
// *
// *=================================================

#ifdef __cplusplus
    #define RE_NULL_HANDLE nullptr
#else
    #define RE_NULL_HANDLE ((void*)0)
#endif

// *=================================================
// *
// * Debug Logging
// *
// *=================================================

#define __RE_LOG_TIME_BUFFER_SIZE__ 20u

/// @brief Internal time string creation method used for logging (DO NOT USE DIRECTLY).
/// @param buffer The buffer inside which the string will be stored (MUST BE OF SIZE __RE_LOG_TIME_BUFFER_SIZE__).
RE_API void __re_getTimeString(char* buffer);

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
    #define re_logInfo(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s"format"\n", __time_buffer, " [INFO]: ", ##__VA_ARGS__); \
    } while(0)

    #define re_logDebug(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s"format"\n", __time_buffer, " [DEBUG]: ", ##__VA_ARGS__); \
    } while(0)

    #define re_logSuccess(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s"format"\n", __time_buffer, " [SUCCESS]: ", ##__VA_ARGS__); \
    } while(0)

    #define re_logWarn(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s%d%s"format"\n", __time_buffer, " [WARN]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__); \
    } while(0)

    #define re_logError(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s%d%s"format"\n", __time_buffer, " [ERROR]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__); \
    } while(0)

    #define re_logFatal(format, ...) do { \
        char __time_buffer[__RE_LOG_TIME_BUFFER_SIZE__]; \
        __re_getTimeString(__time_buffer); \
        __re_internalLog("%s%s%d%s"format"\n", __time_buffer, " [FATAL]: (File: " __FILE__ "; Line: ", __LINE__, ") ", ##__VA_ARGS__); \
    } while(0)
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
// * Assertions
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
// *
// * Heap Allocation and Deallocation Methods
// *
// *=================================================

/// @brief Allocate a chunk of memory on the heap.
/// @param size The number of bytes to allocate.
/// @return A pointer to the new memory.
RE_API void* re_malloc(const size_t size);

/// @brief Allocate and zero a chunk of memory on the heap.
/// @param element_count The number of elements in the new memory.
/// @param element_size The size in bytes of each element.
/// @return A pointer to the new memory.
RE_API void* re_calloc(const size_t element_count, const size_t element_size);

/// @brief Allocate a chunk of memory on the heap and move an
/// existing chunk to the new memory.
/// @param src A pointer to the original heap memory.
/// @param new_size The number of byte for the new allocation.
/// @return A pointer to the new memory.
RE_API void* re_realloc(void* src, const size_t new_size);

/// @brief Free a chunk of memory on the heap.
/// @param src A pointer to the heap memory.
RE_API void re_free(void* src);

#ifdef __cplusplus
    }
#endif

#endif