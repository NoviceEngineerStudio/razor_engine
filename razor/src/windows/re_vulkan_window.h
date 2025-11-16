#ifndef __RE_SRC_WINDOWS_VULKAN_WINDOW__
#define __RE_SRC_WINDOWS_VULKAN_WINDOW__

#include <razor.h>
#include <vulkan/vulkan.h>

/// @brief Returns the list of required Vulkan window extensions.
/// @param extension_count A pointer to where the extension count will be stored.
/// @return The list of required Vulkan window extensions.
const char** re_getVulkanWindowExtensions(re_u32* extension_count);

/// @brief Creates a Vulkan window surface.
/// @param window The window to derive the surface for.
/// @param instance The current Vulkan instance.
/// @param allocator The Vulkan allocation callbacks.
/// @return A Vulkan window surface.
VkSurfaceKHR re_createVulkanSurface(
    const re_Window window,
    const VkInstance instance,
    const VkAllocationCallbacks* allocator
);

/// @brief Returns the window's dimensions as a Vulkan 2D extent.
/// @param window The window to get the size of.
/// @return The window's dimensions as a Vulkan 2D extent.
VkExtent2D re_getVulkanExtent(const re_Window window);

#endif