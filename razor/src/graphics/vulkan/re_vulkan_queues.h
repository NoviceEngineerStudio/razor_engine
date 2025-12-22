#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_GRAPHICS_VULKAN_QUEUES_HEADER_FILE
#define __RAZOR_GRAPHICS_VULKAN_QUEUES_HEADER_FILE

#include <re_core.h>

#define RE_VK_QUEUE_PRESENT_COUNT 1
#define RE_VK_QUEUE_COMPUTE_COUNT 1
#define RE_VK_QUEUE_GRAPHICS_COUNT 1
#define RE_VK_QUEUE_TRANSFER_COUNT 1

typedef enum re_VkQueueRole {
    RE_VK_QUEUE_PRESENT,
    RE_VK_QUEUE_COMPUTE,
    RE_VK_QUEUE_GRAPHICS,
    RE_VK_QUEUE_TRANSFER,

    RE_VK_QUEUE_ROLE_COUNT
} re_VkQueueRole;

typedef struct re_VkQueueFamily {
    uint32_t family_index;
    float* queue_priorities;
    uint32_t queue_count;
} re_VkQueueFamily;

/// @brief Add a new role to a Vulkan queue family, altering the family's definition.
/// @param queue_family The queue family to add the new role to.
/// @param new_queue_role The queue role to add.
void __re_addRoleToVulkanQueue(
    re_VkQueueFamily* queue_family,
    const re_VkQueueRole new_queue_role
);

/// @brief Clear the internals of a Vulkan queue family object (DOES NOT CLEAR THE OBJECT POINTER).
/// @param queue_family A pointer to the queue family object.
void __re_clearVulkanQueueFamily(re_VkQueueFamily* queue_family);

#endif

#endif