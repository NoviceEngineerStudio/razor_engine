#include <re_graphics.h>

#include <re_debug.h>
#include "../re_internals.h"
#include "./re_graphics_internal_backend.h"

#ifdef RE_VULKAN_AVAILABLE
#include "./vulkan/re_vulkan.h"
#endif

re_GraphicsInternalBackend RE_GRAPHICS_INTERNAL_BACKEND = {0};

// *=================================================
// *
// * re_graphicsInit
// *
// *=================================================

void re_graphicsInit() {
    RE_MODULE_INIT_GUARD(RE_GRAPHICS_MODULE, RE_CORE_MODULE);

    #ifdef RE_VULKAN_AVAILABLE

    if (__re_vulkanAvailable()) {
        RE_GRAPHICS_INTERNAL_BACKEND.createContext = (re_CreateInternalGraphicsContext)__re_createVulkanContext;
        RE_GRAPHICS_INTERNAL_BACKEND.destroyContext = (re_DestroyInternalGraphicsContext)__re_destroyVulkanContext;
    }

    #endif
}