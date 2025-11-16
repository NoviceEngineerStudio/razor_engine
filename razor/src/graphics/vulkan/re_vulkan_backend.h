#ifndef __RE_SRC_GRAPHICS_VULKAN_BACKEND__
#define __RE_SRC_GRAPHICS_VULKAN_BACKEND__

#include <razor.h>
#include <vulkan/vulkan.h>

#ifndef __RE_VULKAN_API_VER__
    #define __RE_VULKAN_API_VER__ VK_API_VERSION_1_3
#endif

/// @brief Determine if Vulkan is supported by this system.
/// @return A boolean flag indicating if Vulkan is supported.
re_bool re_isVulkanSupported();

/// @brief Create a Vulkan-specific graphics backend.
/// @param create_info The backend's creation parameters.
/// @return A new Vulkan-specific graphics backend.
re_GraphicsBackend re_createVulkanBackend(const re_GraphicsBackendCreateInfo* create_info);

#endif