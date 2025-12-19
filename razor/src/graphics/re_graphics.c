#include <re_graphics.h>

#include <re_debug.h>
#include "./re_rhi.h"
#include "../re_internals.h"
#include "./re_graphics_types.h"

#ifdef RE_VULKAN_AVAILABLE
#include "./vulkan/re_vulkan.h"
#endif

re_RHIVirtualTable RE_GRAPHICS_RHI = {0};

// *=================================================
// *
// * __re_selectRenderBackend
// *
// *=================================================

bool __re_selectRenderBackend() {
    #ifdef RE_VULKAN_AVAILABLE

    if (__re_vulkanAvailable()) {
        RE_GRAPHICS_RHI.createInternalGraphicsContext = (re_CreateGraphicsBackendContextFn)__re_createVulkanContext;
        RE_GRAPHICS_RHI.destroyInternalGraphicsContext = (re_DestroyGraphicsBackendContextFn)__re_destroyVulkanContext;

        return true;
    }

    #endif

    return false;
}

// *=================================================
// *
// * re_graphicsInit
// *
// *=================================================

void re_graphicsInit() {
    RE_MODULE_INIT_GUARD(RE_GRAPHICS_MODULE, RE_CORE_MODULE);

    const bool backend_selected = __re_selectRenderBackend();
    re_assert(backend_selected, "No rendering backend available!");
}

// *=================================================
// *
// * re_createGraphicsInstance
// *
// *=================================================

re_GraphicsInstance re_createGraphicsInstance(const re_GraphicsInstanceCreateInfo* create_info) {
    re_GraphicsInstance instance = (re_GraphicsInstance)re_calloc(1, sizeof(re_GraphicsInstance_T));

    instance->backend_context = RE_GRAPHICS_RHI.createInternalGraphicsContext(create_info);

    return instance;
}

// *=================================================
// *
// * re_destroyGraphicsInstance
// *
// *=================================================

void re_destroyGraphicsInstance(re_GraphicsInstance* instance) {
    re_assert(instance != RE_NULL_HANDLE, "Attempting to destroy NULL graphics instance!");

    re_GraphicsInstance instance_data = *instance;
    re_assert(instance_data != RE_NULL_HANDLE, "Attempting to destroy NULL graphics instance!");

    RE_GRAPHICS_RHI.destroyInternalGraphicsContext(&instance_data->backend_context);

    re_free(instance_data);
    *instance = RE_NULL_HANDLE;
}