#ifndef __RE_RAZOR_ENGINE_MAIN_HEADER_FILE__
#define __RE_RAZOR_ENGINE_MAIN_HEADER_FILE__

#ifdef __cplusplus
    extern "C" {
#else
    #define static_assert _Static_assert
#endif

#pragma region Engine Metadata

#define RE_ENGINE_NAME "Razor Engine"

#define RE_ENGINE_MAJOR_VER 0
#define RE_ENGINE_MINOR_VER 0
#define RE_ENGINE_PATCH_VER 1

#pragma endregion
#pragma region Platform Values

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

#pragma endregion
#pragma region Platform Detection

// *=================================================
// *
// * Platform Detection
// *
// *=================================================

#ifndef RE_PLATFORM
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
#endif

#pragma endregion
#pragma region 64-Bit Support Detection

// *=================================================
// *
// * 64-Bit Support Detection
// *
// *=================================================

#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__aarch64__) || defined(_M_ARM64) || defined(__ppc64__)
    #define RE_X64 1
#endif

#pragma endregion
#pragma region DLL Import and Export

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

#pragma endregion
#pragma region Primitive Types

// *=================================================
// *
// * Primitive Types
// *
// *=================================================

// ? Pointer Primitives

#ifndef NULL
    #define NULL ((void *)0)
#endif

// ? Boolean Primitives

#include <stdbool.h>

typedef bool re_bool;

static_assert(sizeof(re_bool) == 1, "Size of re_bool is not 1 byte!");

#define re_true ((re_bool)(1u))
#define re_false ((re_bool)(0u))

// ? Integer Primitives (Up to 32-bit)

typedef signed char   re_i8;
typedef signed short re_i16;
typedef signed int   re_i32;

typedef unsigned char   re_u8;
typedef unsigned short re_u16;
typedef unsigned int   re_u32;

static_assert( sizeof(re_i8) == 1, "Size of re_i8 is not 1 byte!");
static_assert(sizeof(re_i16) == 2, "Size of re_i16 is not 2 bytes!");
static_assert(sizeof(re_i32) == 4, "Size of re_i32 is not 4 bytes!");

static_assert( sizeof(re_u8) == 1, "Size of re_u8 is not 1 byte!");
static_assert(sizeof(re_u16) == 2, "Size of re_u16 is not 2 bytes!");
static_assert(sizeof(re_u32) == 4, "Size of re_u32 is not 4 bytes!");

#define RE_MIN_I8  ((re_i8)(0x80))
#define RE_MIN_I16 ((re_i16)(0x8000))
#define RE_MIN_I32 ((re_i32)(0x80000000))

#define RE_MAX_I8  ((re_i8)(0x7F))
#define RE_MAX_I16 ((re_i16)(0x7FFF))
#define RE_MAX_I32 ((re_i32)(0x7FFFFFFF))

#define RE_MIN_U8  ((re_u8)(0x00u))
#define RE_MIN_U16 ((re_u16)(0x0000u))
#define RE_MIN_U32 ((re_u32)(0x00000000u))

#define RE_MAX_U8  ((re_u8)(0xFFu))
#define RE_MAX_U16 ((re_u16)(0xFFFFu))
#define RE_MAX_U32 ((re_u32)(0xFFFFFFFFu))

// ? Floating Point Primitive (32-Bit)

typedef float re_f32;

static_assert(sizeof(re_f32) == 4, "Size of re_f32 is not 4 bytes!");

#define RE_MIN_POS_F32 ((re_f32)(1.175494351e-38f))

#define RE_MIN_F32 ((re_f32)(-3.402823466e+38f))
#define RE_MAX_F32 ((re_f32)( 3.402823466e+38f))

// ? 64-Bit Primitives

#ifdef RE_X64

    // ? 64-Bit Integers

    typedef signed long long   re_i64;
    typedef unsigned long long re_u64;

    static_assert(sizeof(re_i64) == 8, "Size of re_i64 is not 8 bytes!");
    static_assert(sizeof(re_u64) == 8, "Size of re_i64 is not 8 bytes!");

    #define RE_MIN_I64 ((re_i64)(0x8000000000000000))
    #define RE_MAX_I64 ((re_i64)(0x7FFFFFFFFFFFFFFF))

    #define RE_MIN_U64 ((re_u64)(0x0000000000000000u))
    #define RE_MAX_U64 ((re_u64)(0xFFFFFFFFFFFFFFFFu))

    // ? 64-Bit Floating Point

    typedef double re_f64;

    static_assert(sizeof(re_f64) == 8, "Size of re_f64 is not 8 bytes!");

    #define RE_MIN_POS_F64 ((re_f64)(2.2250738585072014e-308))

    #define RE_MIN_F64 ((re_f64)(-1.7976931348623158e+308))
    #define RE_MAX_F64 ((re_f64)( 1.7976931348623158e+308))

    // ? 64-Bit Size

    typedef re_u64 re_size;
    
    #define RE_MIN_SIZE RE_MIN_U64
    #define RE_MAX_SIZE RE_MAX_U64

#else

    // ? 32-Bit Size

    typedef re_u32 re_size;
    
    #define RE_MIN_SIZE RE_MIN_U32
    #define RE_MAX_SIZE RE_MAX_U32

#endif

#pragma endregion
#pragma region Utility Methods

#define __RE_DEFINE_NUMERIC_COMPARISONS__(Type, Suffix) \
RE_API Type re_min##Suffix(Type a, Type b); \
RE_API Type re_max##Suffix(Type a, Type b); \
RE_API Type re_clamp##Suffix(Type value, Type lower, Type upper);

__RE_DEFINE_NUMERIC_COMPARISONS__(re_i8, I8);
__RE_DEFINE_NUMERIC_COMPARISONS__(re_i16, I16);
__RE_DEFINE_NUMERIC_COMPARISONS__(re_i32, I32);

__RE_DEFINE_NUMERIC_COMPARISONS__(re_u8, U8);
__RE_DEFINE_NUMERIC_COMPARISONS__(re_u16, U16);
__RE_DEFINE_NUMERIC_COMPARISONS__(re_u32, U32);

__RE_DEFINE_NUMERIC_COMPARISONS__(re_f32, F32);

__RE_DEFINE_NUMERIC_COMPARISONS__(re_size, Size);

#ifdef RE_X64
    __RE_DEFINE_NUMERIC_COMPARISONS__(re_i64, I64);
    __RE_DEFINE_NUMERIC_COMPARISONS__(re_u64, U64);
    __RE_DEFINE_NUMERIC_COMPARISONS__(re_f64, F64);
#endif

/// @brief Return the length of a C-style string.
/// @param str A C-style string.
/// @return The length of the string, not including the null-terminator.
RE_API re_u32 re_getStrLen(const char* str);

/// @brief Determine if two C-style strings are equivalent.
/// @param str_a A C-style string.
/// @param str_b A C-style string.
/// @return A boolean indicating if the contents of the strings match.
RE_API re_bool re_isStrEqual(const char* str_a, const char* str_b);

#pragma endregion
#pragma region Debug Logging

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

    static const re_bool __RE_LOGGER_ENABLED__ = re_true;

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

    static const re_bool __RE_LOGGER_ENABLED__ = re_false;

    #define re_logInfo(format, ...) ((void)(0))
    #define re_logDebug(format, ...) ((void)(0))
    #define re_logSuccess(format, ...) ((void)(0))

    #define re_logWarn(format, ...) ((void)(0))
    #define re_logError(format, ...) ((void)(0))
    #define re_logFatal(format, ...) ((void)(0))

#endif

#pragma endregion
#pragma region Debug Assertions

// *=================================================
// *
// * Debug Assertions
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

#pragma endregion
#pragma region Heap Allocation and Deallocation Methods

// *=================================================
// *
// * Heap Allocation and Deallocation Methods
// *
// *=================================================

/// @brief Allocate a chunk of memory on the heap.
/// @param size The number of bytes to allocate.
/// @return A pointer to the new memory.
RE_API void* re_malloc(re_size size);

/// @brief Allocate and zero a chunk of memory on the heap.
/// @param element_count The number of elements in the new memory.
/// @param element_size The size in bytes of each element.
/// @return A pointer to the new memory.
RE_API void* re_calloc(re_size element_count, re_size element_size);

/// @brief Allocate a chunk of memory on the heap and move an
/// existing chunk to the new memory.
/// @param src A pointer to the original heap memory.
/// @param new_size The number of byte for the new allocation.
/// @return A pointer to the new memory.
RE_API void* re_realloc(void* src, re_size new_size);

/// @brief Free a chunk of memory on the heap.
/// @param src A pointer to the heap memory.
RE_API void re_free(void* src);

#pragma endregion
#pragma region Application Window

// *=================================================
// *
// * Application Window
// *
// *=================================================

typedef struct re_Window_T re_Window_T;
typedef struct re_Window_T* re_Window;

typedef struct re_WindowCreateInfo {
    const char* title;
    const char* icon_path;

    void(*closeCallback)();

    re_u32 width;
    re_u32 height;
    re_bool fullscreen_enabled;
} re_WindowCreateInfo;

/// @brief Create a new application window.
/// @param create_info The window's creation parameters.
/// @return A new application window.
RE_API re_Window re_createWindow(const re_WindowCreateInfo* create_info);

/// @brief Destroys an application window.
/// @param window A pointer to the window to destroy.
RE_API void re_destroyWindow(re_Window* window);

/// @brief Returns the current dimensions of the given window.
/// @param window The window to get the size of.
/// @param width A pointer to where the width will be stored.
/// @param height A pointer to where the height will be stored.
RE_API void re_getWindowSize(const re_Window window, re_u32* width, re_u32* height);

/// @brief Returns a boolean indicating if the given window is in fullscreen mode.
/// @param window The window to get the fullscreen information from.
/// @return A boolean indicating if the given window is in fullscreen mode.
RE_API re_bool re_isWindowFullscreen(const re_Window window);

/// @brief Sets the fullscreen mode for a given window.
/// @param window The window to be set as fullscreen / not fullscreen.
/// @param fullscreen_enabled The new state of the window's fullscreen mode.
RE_API void re_setWindowFullscreen(re_Window window, const re_bool fullscreen_enabled);

/// @brief Poll operating system and window events.
/// @param window The window whose events are to be polled and processed.
RE_API void re_pollEvents(re_Window window);

#pragma endregion
#pragma region Graphics Backend

typedef struct re_GraphicsBackend_T re_GraphicsBackend_T;
typedef re_GraphicsBackend_T* re_GraphicsBackend;

typedef void (*re_GraphicsBackendDestroy)(re_GraphicsBackend*);

typedef struct re_GraphicsBackend_T {
    // Render API specific backend. NOTE: Never modify this value!
    void* __user_data__;

    re_GraphicsBackendDestroy destroy;
} re_GraphicsBackend_T;

typedef struct re_GraphicsBackendCreateInfo {
    re_Window window;

    const char* app_name;
    re_u8 app_major_version;
    re_u8 app_minor_version;
    re_u8 app_patch_version;
} re_GraphicsBackendCreateInfo;

/// @brief Create a new graphics backend.
/// @param create_info Graphics backend creation parameters.
/// @return A new graphics backend.
RE_API re_GraphicsBackend re_createGraphicsBackend(const re_GraphicsBackendCreateInfo* create_info);

#pragma endregion

#ifdef __cplusplus
}
#endif

#endif