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
// * Render Hardware Interface Methods
// *
// *=================================================

typedef enum re_RenderProfile {
    RE_RENDERER_SIMPLE,
    RE_RENDERER_STANDARD,
    RE_RENDERER_HEAVY
} re_RenderProfile;

typedef struct re_GraphicsInstance_T re_GraphicsInstance_T;
typedef re_GraphicsInstance_T* re_GraphicsInstance; 

typedef struct re_GraphicsInstanceCreateInfo {
    re_Window window;
    re_RenderProfile profile;
} re_GraphicsInstanceCreateInfo;

/// @brief Create a new graphics instance for interfacing with the underlying render backend.
/// @param create_info The graphics instance's creation parameters.
/// @return A new graphics instance for interfacing with the underlying render backend.
RE_API re_GraphicsInstance re_createGraphicsInstance(const re_GraphicsInstanceCreateInfo* create_info);

/// @brief Destroy a graphics instance.
/// @param instance The graphics instance to be destroyed.
RE_API void re_destroyGraphicsInstance(re_GraphicsInstance* instance);

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif