#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_VULKAN_HEADER_FILE
#define __RAZOR_VULKAN_HEADER_FILE

#include <re_core.h>
#include <re_graphics.h>
#include <vulkan/vulkan.h>
#include "./re_vulkan_device_layer.h"
#include "./re_vulkan_swap_chain_layer.h"

#define __RE_VULKAN_API_VER VK_API_VERSION_1_3

typedef struct re_VulkanContext_T {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkRenderPass render_pass;

    re_VkDeviceLayer device_layer;
    re_VkSwapChainLayer swap_chain_layer;

    VkAllocationCallbacks* allocator;
} re_VulkanContext_T;
typedef re_VulkanContext_T* re_VulkanContext;

/// @brief Determine if Vulkan is available on this device.
/// @return A flag indicating if Vulkan is available on this device.
bool __re_vulkanAvailable();

/// @brief Create a new Vulkan backend context object.
/// @param create_info The Vulkan backend's creation parameters.
/// @return A new Vulkan backend context object.
re_VulkanContext __re_createVulkanContext(const re_GraphicsContextCreateInfo* create_info);

/// @brief Destroy the internal Vulkan backend context.
/// @param context The context to be destroyed.
void __re_destroyVulkanContext(re_VulkanContext* context);

#endif

#endif