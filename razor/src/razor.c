#include <razor.h>

#include "./re_internals.h"

static re_EngineModuleFlag modules_init_flag = 0u;

// *=================================================
// *
// * __re_checkModuleInit
// *
// *=================================================

bool __re_checkModuleInit(const re_EngineModuleFlag modules_flag) {
    return (modules_init_flag & modules_flag) == modules_flag;
}

// *=================================================
// *
// * __re_setModuleInit
// *
// *=================================================

void __re_setModuleInit(const re_EngineModuleBits module) {
    modules_init_flag |= module;
}

// *=================================================
// *
// * re_init
// *
// *=================================================

void re_init(const re_InitParams* params) {
    RE_MODULE_INIT_GUARD(RE_ALL_MODULES, 0u);

    re_assert(params != RE_NULL_HANDLE, "Attempted to initialize engine modules with NULL parameters!");

    re_coreInit(&params->core);
    re_graphicsInit();
}