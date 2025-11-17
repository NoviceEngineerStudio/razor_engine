#ifndef __RE_SRC_GRAPHICS_VULKAN_DEVICE_LAYER__
#define __RE_SRC_GRAPHICS_VULKAN_DEVICE_LAYER__

#include <razor.h>
#include <vulkan/vulkan.h>

typedef enum re_VkQueueRole {
    RE_VK_QUEUE_PRESENT,
    RE_VK_QUEUE_COMPUTE,
    RE_VK_QUEUE_GRAPHICS,
    RE_VK_QUEUE_TRANSFER,

    RE_VK_QUEUE_ROLE_COUNT
} re_VkQueueRole;

typedef struct re_VkGPU {
    VkPhysicalDevice physical_device;

    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceProperties properties;
    VkSurfaceCapabilitiesKHR capabilities;
    VkPhysicalDeviceMemoryProperties mem_properties;

    VkSurfaceFormatKHR* formats;
    re_u32 format_count;

    VkPresentModeKHR* present_modes;
    re_u32 present_mode_count;

    re_u32 queue_indices[RE_VK_QUEUE_ROLE_COUNT];
} re_VkGPU;

typedef struct re_VkDeviceLayerCreateInfo {
    VkInstance instance;
    VkSurfaceKHR surface;

    const char** enabled_extensions;
    re_u32 enabled_extension_count;

    re_u32 max_texture_count;
    re_u32 max_sampler_count;
    re_u32 max_storage_image_count;
    re_u32 max_uniform_buffer_count;
    re_u32 max_storage_buffer_count;
    re_u32 max_input_attachment_count;
    re_u32 max_dynamic_uniform_buffer_count;
} re_VkDeviceLayerCreateInfo;

typedef struct re_VkDeviceLayer_T {
    re_VkGPU gpu;
    VkDevice logical_device;
    VkDescriptorPool desc_pool;

    VkQueue queues[RE_VK_QUEUE_ROLE_COUNT];
    VkCommandPool cmd_pools[RE_VK_QUEUE_ROLE_COUNT];
} re_VkDeviceLayer_T;

typedef re_VkDeviceLayer_T* re_VkDeviceLayer;

/// @brief Creates a new Vulkan device layer object.
/// @param create_info The device layer's creation parameters.
/// @param allocator The Vulkan allocation callbacks.
/// @return A new Vulkan device layer object.
re_VkDeviceLayer re_createVulkanDeviceLayer(
    const re_VkDeviceLayerCreateInfo* create_info,
    const VkAllocationCallbacks* allocator
);

/// @brief Destroy a Vulkan device layer object.
/// @param device_layer The device layer to destroy.
/// @param allocator The Vulkan allocation callbacks.
void re_destroyVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
);

#endif