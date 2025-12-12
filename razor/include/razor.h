#ifndef __RAZOR_MAIN_HEADER_FILE
#define __RAZOR_MAIN_HEADER_FILE

#include "./re_core.h"
#include "./re_debug.h"

#ifdef __cplusplus
    extern "C" {
#endif

// *=================================================
// *
// * Full Engine Initialization (All Modules)
// *
// *=================================================

/// @brief Initialize all engine modules.
RE_API void re_init();

// *=================================================

#ifdef __cplusplus
    }
#endif

#endif