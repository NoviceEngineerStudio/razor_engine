#ifdef RE_VULKAN_AVAILABLE

#include "./re_vulkan_queues.h"

#include <re_debug.h>

static const float __RE_VK_QUEUE_PRESENT_PRIORITIES[RE_VK_QUEUE_PRESENT_COUNT] = { 1.0f };
static const float __RE_VK_QUEUE_COMPUTE_PRIORITIES[RE_VK_QUEUE_COMPUTE_COUNT] = { 1.0f };
static const float __RE_VK_QUEUE_GRAPHICS_PRIORITIES[RE_VK_QUEUE_GRAPHICS_COUNT] = { 1.0f };
static const float __RE_VK_QUEUE_TRANSFER_PRIORITIES[RE_VK_QUEUE_TRANSFER_COUNT] = { 1.0f };

// *=================================================
// *
// * __re_addRoleToVulkanQueue
// *
// *=================================================

void __re_addRoleToVulkanQueue(
    re_VkQueueFamily* queue_family,
    const re_VkQueueRole new_queue_role
) {
    uint32_t new_queue_count = 0;
    const float* new_queue_priorities = RE_NULL_HANDLE;

    switch (new_queue_role) {
        case RE_VK_QUEUE_PRESENT: {
            new_queue_count = RE_VK_QUEUE_PRESENT_COUNT;
            new_queue_priorities = __RE_VK_QUEUE_PRESENT_PRIORITIES;
            queue_family->roles_flag |= RE_VK_QUEUE_BIT_PRESENT;
            break;
        }

        case RE_VK_QUEUE_COMPUTE: {
            new_queue_count = RE_VK_QUEUE_COMPUTE_COUNT;
            new_queue_priorities = __RE_VK_QUEUE_COMPUTE_PRIORITIES;
            queue_family->roles_flag |= RE_VK_QUEUE_BIT_COMPUTE;
            break;
        }

        case RE_VK_QUEUE_GRAPHICS: {
            new_queue_count = RE_VK_QUEUE_GRAPHICS_COUNT;
            new_queue_priorities = __RE_VK_QUEUE_GRAPHICS_PRIORITIES;
            queue_family->roles_flag |= RE_VK_QUEUE_BIT_GRAPHICS;
            break;
        }

        case RE_VK_QUEUE_TRANSFER: {
            new_queue_count = RE_VK_QUEUE_TRANSFER_COUNT;
            new_queue_priorities = __RE_VK_QUEUE_TRANSFER_PRIORITIES;
            queue_family->roles_flag |= RE_VK_QUEUE_BIT_TRANSFER;
            break;
        }

        default: {
            re_assert(false, "Unknown Vulkan queue role added to queue family! Type: %d", new_queue_role);
            break;
        }
    }

    if (queue_family->queue_count < new_queue_count) {
        queue_family->queue_count = new_queue_count;

        float* old_queue_priorities = queue_family->queue_priorities;

        queue_family->queue_priorities = re_calloc(new_queue_count, sizeof(float));

        if (old_queue_priorities != RE_NULL_HANDLE) {
            re_memcpy(
                queue_family->queue_priorities,
                old_queue_priorities,
                new_queue_count * sizeof(float)
            );

            re_free(old_queue_priorities);
        }
    }

    float* queue_priorities = queue_family->queue_priorities;
    for (uint32_t idx = 0; idx < new_queue_count; ++idx) {
        const float new_priority = new_queue_priorities[idx];

        if (new_priority > queue_priorities[idx]) {
            queue_priorities[idx] = new_priority;
        }
    }
}

// *=================================================
// *
// * __re_clearVulkanQueueFamily
// *
// *=================================================

void __re_clearVulkanQueueFamily(re_VkQueueFamily* queue_family) {
    if (queue_family->queue_priorities != RE_NULL_HANDLE) {
        re_free(queue_family->queue_priorities);
        queue_family->queue_priorities = RE_NULL_HANDLE;
    }

    queue_family->family_index = 0;
    queue_family->queue_count = 0;
    queue_family->roles_flag = 0;
}

#endif