#include <re_core.h>

#include <re_debug.h>
#include "../re_internals.h"

#if RE_PLATFORM == RE_PLATFORM_WINDOWS
#include "./win32/re_win32.h"
#endif

const char* RE_APP_NAME = "Untitled Application";
uint8_t RE_APP_MAJOR_VERSION = 0u;
uint8_t RE_APP_MINOR_VERSION = 0u;
uint8_t RE_APP_PATCH_VERSION = 0u;

// *=================================================
// *
// * re_coreInit
// *
// *=================================================

void re_coreInit(const re_CoreInitParams* params) {
    RE_MODULE_INIT_GUARD(RE_CORE_MODULE, 0u);

    re_assert(params != RE_NULL_HANDLE, "Attempted to initialize core module with NULL parameters!");

    RE_APP_NAME = params->app_name;
    RE_APP_MAJOR_VERSION = params->app_major_version;
    RE_APP_MINOR_VERSION = params->app_minor_version;
    RE_APP_PATCH_VERSION = params->app_patch_version;

    #if RE_PLATFORM == RE_PLATFORM_WINDOWS
    __re_initCoreWin32();
    #endif
}