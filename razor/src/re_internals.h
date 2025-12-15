#ifndef __RAZOR_INTERNAL_HEADER_FILE
#define __RAZOR_INTERNAL_HEADER_FILE

#include <re_core.h>

// *=================================================
// *
// * Engine Module-Level Metadata
// *
// *=================================================

typedef enum re_EngineModuleBits {
    RE_ALL_MODULES     = 1 << 0,
    
    RE_CORE_MODULE     = 1 << 1,
    RE_DEBUG_MODULE    = 1 << 2,
    RE_GRAPHICS_MODULE = 1 << 3
} re_EngineModuleBits;
typedef uint32_t re_EngineModuleFlag;

/// @brief Check if some engine modules have been initialized.
/// @param modules_flag A flag indicating which modules you'd like to check the init status of.
/// @return A flag determining if some engine modules have been initialized.
bool __re_checkModuleInit(const re_EngineModuleFlag modules_flag);

/// @brief Set an engine module as initialized.
/// @param module The module to set as initialized.
void __re_setModuleInit(const re_EngineModuleBits module);

#define RE_MODULE_INIT_GUARD(module, required_modules) do { \
    re_assert(__re_checkModuleInit( required_modules ), "Module missing required sibling module!"); \
    if (__re_checkModuleInit( module )) { return; } \
    __re_setModuleInit( module ); \
} while(0)

// *=================================================
// *
// * Global Variables
// *
// *=================================================

extern const char* RE_APP_NAME;
extern uint8_t RE_APP_MAJOR_VER;
extern uint8_t RE_APP_MINOR_VER;
extern uint8_t RE_APP_PATCH_VER;

#endif