#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_VULKAN_DEVICE_LAYER_HEADER_FILE
#define __RAZOR_VULKAN_DEVICE_LAYER_HEADER_FILE

#include <re_core.h>
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
    uint32_t format_count;

    VkPresentModeKHR* present_modes;
    uint32_t present_mode_count;

    uint32_t queue_indices[RE_VK_QUEUE_ROLE_COUNT];
} re_VkGPU;

typedef struct re_VkDeviceLayerCreateInfo {
    VkInstance instance;
    VkSurfaceKHR surface;

    const char** enabled_extensions;
    uint32_t enabled_extension_count;

    uint32_t max_texture_count;
    uint32_t max_sampler_count;
    uint32_t max_storage_image_count;
    uint32_t max_uniform_buffer_count;
    uint32_t max_storage_buffer_count;
    uint32_t max_input_attachment_count;
    uint32_t max_dynamic_uniform_buffer_count;
} re_VkDeviceLayerCreateInfo;

typedef struct re_VkDeviceLayer {
    re_VkGPU gpu;
    VkDevice logical_device;
    VkDescriptorPool desc_pool;

    VkQueue queues[RE_VK_QUEUE_ROLE_COUNT];
    VkCommandPool cmd_pools[RE_VK_QUEUE_ROLE_COUNT];
} re_VkDeviceLayer;

/// @brief Creates a new Vulkan device layer object.
/// @param create_info The device layer's creation parameters.
/// @param allocator The Vulkan allocation callbacks.
/// @return A new Vulkan device layer object.
void __re_createVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const re_VkDeviceLayerCreateInfo* create_info,
    const VkAllocationCallbacks* allocator
);

/// @brief Destroy a Vulkan device layer object.
/// @param device_layer The device layer to destroy.
/// @param allocator The Vulkan allocation callbacks.
void __re_destroyVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
);

#endif

#endif