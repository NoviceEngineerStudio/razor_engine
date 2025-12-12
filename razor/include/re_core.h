#ifndef __RAZOR_CORE_HEADER_FILE
#define __RAZOR_CORE_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#else
    #include <stdbool.h>
#endif

#include <stdint.h>

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
#else
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
// * NULL Handle
// *
// *=================================================

#ifdef __cplusplus
    #define RE_NULL_HANDLE nullptr
#else
    #ifdef NULL
        #define RE_NULL_HANDLE NULL
    #else
        #define RE_NULL_HANDLE ((void*)0)
    #endif
#endif

// *=================================================
// *
// * Module Initialization
// *
// *=================================================

/// @brief Initialize the core engine module.
RE_API void re_coreInit();

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

/// @brief Allocate a chunk of aligned memory on the heap.
/// @param size The number of bytes to allocate.
/// @param alignment The byte count to align by.
/// @return A pointer to the new memory.
RE_API void* re_mallocAlign(const size_t size, const size_t alignment);

/// @brief Allocate and zero a chunk of aligned memory on the heap.
/// @param element_count The number of elements in the new memory.
/// @param element_size The size in bytes of each element.
/// @param alignment The byte count to align by.
/// @return A pointer to the new memory.
RE_API void* re_callocAlign(const size_t element_count, const size_t element_size, const size_t alignment);

/// @brief Free a chunk of aligned memory on the heap.
/// @param src A pointer to the heap memory.
RE_API void re_freeAlign(void* src);

// *=================================================
// *
// * Application Window
// *
// *=================================================

typedef struct re_Window_T re_Window_T;
typedef re_Window_T* re_Window;

typedef enum re_WindowFlagBits {
    RE_WINDOW_FULLSCREEN   = 1 << 0,
    RE_WINDOW_RESIZEABLE   = 1 << 1,
    RE_WINDOW_BORDERLESS   = 1 << 2,
    RE_WINDOW_HIDDEN       = 1 << 3,
    RE_WINDOW_TOPMOST      = 1 << 4,
    RE_WINDOW_NO_MAXIMIZE  = 1 << 5,
    RE_WINDOW_NO_MINIMIZE  = 1 << 6
} re_WindowFlagBits;
typedef uint32_t re_WindowFlag;

typedef void(*re_WindowCloseCallback)();

typedef struct re_WindowCreateInfo {
    const char* title;
    const char* icon_path;

    uint32_t width;
    uint32_t height;
    
    re_WindowFlag flags;
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
RE_API void re_getWindowSize(const re_Window window, uint32_t* width, uint32_t* height);

/// @brief Returns a boolean indicating if the given window is in fullscreen mode.
/// @param window The window to get the fullscreen information from.
/// @return A boolean indicating if the given window is in fullscreen mode.
RE_API bool re_isWindowFullscreen(const re_Window window);

/// @brief Sets the fullscreen mode for a given window.
/// @param window The window to be set as fullscreen / not fullscreen.
/// @param fullscreen_enabled The new state of the window's fullscreen mode.
RE_API void re_setWindowFullscreen(re_Window window, const bool fullscreen_enabled);

/// @brief Set the on-close callback for some window.
/// @param window The window to assign the callback to.
/// @param callback The callback to assign.
RE_API void re_setWindowCloseCallback(re_Window window, const re_WindowCloseCallback callback);

/// @brief Poll operating system and window events.
/// @param window The window whose events are to be polled and processed.
RE_API void re_pollEvents(re_Window window);

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif