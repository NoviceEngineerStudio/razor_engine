#include <razor.h>
#include "./vulkan/re_vulkan_backend.h"

// *=================================================
// *
// * re_createGraphicsBackend
// *
// *=================================================

re_GraphicsBackend re_createGraphicsBackend(const re_GraphicsBackendCreateInfo* create_info) {
    re_GraphicsBackend backend = NULL;

    if (re_isVulkanSupported()) {
        backend = re_createVulkanBackend(create_info);
    }

    re_assert(backend != NULL, "No supported graphics backend found!");
    re_assert(backend->destroy != NULL, "Graphics backend does not implement 'destroy' method!");

    return backend;
}