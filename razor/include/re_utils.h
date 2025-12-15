#ifndef __RAZOR_UTILS_HEADER_FILE
#define __RAZOR_UTILS_HEADER_FILE

#ifdef __cplusplus
    extern "C" {
#endif

#include "./re_core.h"

// *=================================================
// *
// * String Utilities
// *
// *=================================================

/// @brief Determine if two NULL-terminated strings are equivalent.
/// @param str_a Some NULL-terminated string.
/// @param str_b Some other NULL-terminated string.
/// @return A flag indicating if the strings are equivalent.
bool re_isStrEqual(const char* str_a, const char* str_b);

#ifdef __cplusplus
    }
#endif

#endif