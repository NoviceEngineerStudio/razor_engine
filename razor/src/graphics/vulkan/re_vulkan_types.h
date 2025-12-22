#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_GRAPHICS_VULKAN_TYPES_HEADER_FILE
#define __RAZOR_GRAPHICS_VULKAN_TYPES_HEADER_FILE

#include <vulkan/vulkan.h>
#include "./re_vulkan.h"
#include "./re_vulkan_queues.h"

#define RE_MAX_VULKAN_THREADS 1

typedef enum re_VkCmdPoolRole {
    RE_VK_CMD_POOL_FRAME,
    RE_VK_CMD_POOL_STATIC,
    RE_VK_CMD_POOL_ONE_SHOT,

    RE_VK_CMD_POOL_ROLE_COUNT
} re_VkCmdPoolRole;

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

    uint32_t queue_family_count;
    re_VkQueueFamily queue_families[RE_VK_QUEUE_ROLE_COUNT];

    uint32_t queue_role_indices[RE_VK_QUEUE_ROLE_COUNT];
} re_VkGPU;

#define RE_VULKAN_MAX_CMD_POOLS RE_MAX_VULKAN_THREADS * RE_VK_CMD_POOL_ROLE_COUNT * (RE_VK_QUEUE_ROLE_COUNT - 1)
#define RE_VULKAN_MAX_DEVICE_QUEUES RE_VK_QUEUE_PRESENT_COUNT + RE_VK_QUEUE_COMPUTE_COUNT + RE_VK_QUEUE_GRAPHICS_COUNT + RE_VK_QUEUE_TRANSFER_COUNT

#define __re_getVulkanCmdPoolIndex(thread_index, queue_role, pool_role) ( \
    ((thread_index) * RE_VK_CMD_POOL_ROLE_COUNT * (RE_VK_QUEUE_ROLE_COUNT - 1)) +\
    ((queue_role - 1) * RE_VK_CMD_POOL_ROLE_COUNT) + \
    (pool_role) \
)

typedef struct re_VkContext_T {
    VkInstance instance;
    VkSurfaceKHR surface;

    re_VkGPU gpu;
    VkDevice logical_device;
    VkDescriptorPool desc_pool;

    VkQueue queues[RE_VULKAN_MAX_DEVICE_QUEUES];
    VkCommandPool cmd_pools[RE_VULKAN_MAX_CMD_POOLS];

    VkAllocationCallbacks* allocator;
} re_VulkanContext_T;

#endif

#endif