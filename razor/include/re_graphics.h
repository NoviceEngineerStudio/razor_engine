#ifndef __RAZOR_GRAPHICS_HEADER_FILE
#define __RAZOR_GRAPHICS_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#endif

#include "./re_core.h"

// *=================================================
// *
// * Module Initialization
// *
// *=================================================

/// @brief Initialize the graphics engine module.
RE_API void re_graphicsInit();

// *=================================================
// *
// * Backend Graphics Context
// *
// *=================================================

typedef struct re_GraphicsContext_T re_GraphicsContext_T;
typedef re_GraphicsContext_T* re_GraphicsContext;

typedef struct re_GraphicsContextCreateInfo {
    re_Window window;

    const char* app_name;
    uint8_t app_major_version;
    uint8_t app_minor_version;
    uint8_t app_patch_version;
} re_GraphicsContextCreateInfo;

/// @brief Creates a new backend graphics context.
/// @param create_info The backend graphics context's creation parameters.
/// @return A new backend graphics context.
RE_API re_GraphicsContext re_createGraphicsContext(const re_GraphicsContextCreateInfo create_info);

/// @brief Destroy a backend graphics context.
/// @param context The graphics context to destroy. 
RE_API void re_destroyGraphicsContext(re_GraphicsContext* context);

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif