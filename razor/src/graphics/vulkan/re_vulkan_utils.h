#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_GRAPHICS_VULKAN_UTILS_HEADER_FILE
#define __RAZOR_GRAPHICS_VULKAN_UTILS_HEADER_FILE

#include <vulkan/vulkan.h>
#include "./re_vulkan_types.h"

/// @brief Create a Vulkan instance.
/// @param allocator Vulkan allocation callbacks.
/// @return A handle to the new Vulkan instance.
VkInstance __re_createVulkanInstance(const VkAllocationCallbacks* allocator);

/// @brief Select the most appropriate Vulkan GPU.
/// @param surface The window surface to check the capabilities against.
/// @return The most appropriate Vulkan GPU.
re_VkGPU __re_selectVulkanGPU(
    const VkInstance instance,
    const VkSurfaceKHR surface
);

/// @brief Clear the internals of a Vulkan GPU.
/// @param gpu The GPU to be cleared.
void __re_clearVulkanGPU(re_VkGPU* gpu);

/// @brief Create a Vulkan logical device.
/// @param gpu The selected Vulkan GPU.
/// @param allocator Vulkan allocation callbacks.
/// @return A handle to the new Vulkan logical device.
VkDevice __re_createVulkanLogicalDevice(
    const re_VkGPU* gpu,
    const VkAllocationCallbacks* allocator
);

/// @brief Get Vulkan device queues for some queue family.
/// @param queue_arr The array to store the gathered queues.
/// @param queue_count The number of queues to be gathered.
/// @param queue_family_index The queue family's index.
/// @param logical_device The Vulkan logical device handle.
void __re_getVulkanQueues(
    VkQueue* queue_arr,
    const uint32_t queue_count,
    const uint32_t queue_family_index,
    const VkDevice logical_device
);

/// @brief Creates all of the Vulkan command pools needed according to the maximum number of queues and threads.
/// @param cmd_pool_arr The array to store the created command pools.
/// @param logical_device The Vulkan logical device handle.
/// @param gpu The selected Vulkan GPU.
/// @param allocator Vulkan allocation callbacks.
void __re_createVulkanCommandPools(
    VkCommandPool* cmd_pool_arr,
    const VkDevice logical_device,
    const re_VkGPU* gpu,
    const VkAllocationCallbacks* allocator
);

/// @brief Create a Vulkan descriptor pool.
/// @param logical_device The Vulkan logical device handle.
/// @param profile The rendering profile selected by the frontend.
/// @param allocator Vulkan allocation callbacks.
/// @return A handle to the new Vulkan descriptor pool.
VkDescriptorPool __re_createVulkanDescriptorPool(
    const VkDevice logical_device,
    const re_RenderProfile profile,
    const VkAllocationCallbacks* allocator
);

#endif

#endif