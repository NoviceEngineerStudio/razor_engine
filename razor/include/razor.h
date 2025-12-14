#ifndef __RAZOR_MAIN_HEADER_FILE
#define __RAZOR_MAIN_HEADER_FILE

#include "./re_core.h"
#include "./re_debug.h"
#include "./re_graphics.h"

#ifdef __cplusplus
    extern "C" {
#endif

// *=================================================
// *
// * Full Engine Initialization (All Modules)
// *
// *=================================================

typedef struct re_InitParams {
    re_CoreInitParams core;
} re_InitParams;

/// @brief Initialize all engine modules.
RE_API void re_init(const re_InitParams* params);

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif