#ifndef __RAZOR_CORE_VULKAN_WINDOW_HEADER_FILE
#define __RAZOR_CORE_VULKAN_WINDOW_HEADER_FILE

#include <re_core.h>
#include <vulkan/vulkan.h>

/// @brief Returns the list of required Vulkan window extensions.
/// @param extension_count A pointer to where the extension count will be stored.
/// @return The list of required Vulkan window extensions.
const char** __re_getVulkanWindowExtensions(uint32_t* extension_count);

/// @brief Creates a Vulkan window surface.
/// @param window The window to derive the surface for.
/// @param instance The current Vulkan instance.
/// @param allocator The Vulkan allocation callbacks.
/// @return A Vulkan window surface.
VkSurfaceKHR __re_createVulkanSurface(
    const re_Window window,
    const VkInstance instance,
    const VkAllocationCallbacks* allocator
);

/// @brief Returns the window's dimensions as a Vulkan 2D extent.
/// @param window The window to get the size of.
/// @return The window's dimensions as a Vulkan 2D extent.
VkExtent2D __re_getVulkanExtent(const re_Window window);

#endif