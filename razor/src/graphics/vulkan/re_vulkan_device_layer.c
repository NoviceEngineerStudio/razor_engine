#ifdef RE_VULKAN_AVAILABLE

#include <re_debug.h>
#include <re_utils.h>
#include "./re_vulkan_device_layer.h"

#define __RE_VULKAN_DISCRETE_GPU_SCORE 5000u

#define __RE_DESCRIPTOR_SIZE_COUNT 7u

// *=================================================
// *
// * __re_createVulkanGPU
// *
// *=================================================

bool __re_createVulkanGPU(
    const VkPhysicalDevice physical_device,
    const VkSurfaceKHR surface,
    const char** enabled_extensions,
    const uint32_t enabled_extension_count,
    re_VkGPU* gpu
) {
    gpu->physical_device = physical_device;

    uint32_t __extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &__extension_count, VK_NULL_HANDLE);

    if (__extension_count < enabled_extension_count) {
        return false;
    }

    const uint32_t extension_count = __extension_count;
    VkExtensionProperties* extensions = (VkExtensionProperties*)re_malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &__extension_count, extensions);

    for (uint32_t idx = 0; idx < enabled_extension_count; ++idx) {
        const char* enabled_extension = enabled_extensions[idx];

        bool found_extension = false;
        for (uint32_t jdx = 0; jdx < extension_count; ++jdx) {
            const char* extension = extensions[jdx].extensionName;

            if (re_isStrEqual(extension, enabled_extension)) {
                found_extension = true;
                break;
            }
        }

        if (!found_extension) {
            re_free(extensions);
            return false;
        }
    }

    re_free(extensions);

    vkGetPhysicalDeviceFeatures(physical_device, &gpu->features);
    vkGetPhysicalDeviceProperties(physical_device, &gpu->properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &gpu->mem_properties);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &gpu->capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, VK_NULL_HANDLE);
    
    if (format_count == 0) {
        return false;
    }
    
    gpu->format_count = format_count;
    gpu->formats = (VkSurfaceFormatKHR*)re_malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, gpu->formats);

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, VK_NULL_HANDLE);
    
    if (present_mode_count == 0) {
        return false;
    }
    
    gpu->present_mode_count = present_mode_count;
    gpu->present_modes = (VkPresentModeKHR*)re_malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, gpu->present_modes);

    bool queues_assigned[RE_VK_QUEUE_ROLE_COUNT] = {0};
    uint32_t __family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, VK_NULL_HANDLE);

    const uint32_t family_count = __family_count;
    VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)re_malloc(sizeof(VkQueueFamilyProperties) * family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, families);

    for (uint32_t idx = 0; idx < family_count; ++idx) {
        const VkQueueFamilyProperties family = families[idx];

        if (family.queueCount == 0) {
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_GRAPHICS] && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_GRAPHICS] = idx;
            queues_assigned[RE_VK_QUEUE_GRAPHICS] = true;
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_PRESENT]) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, idx, surface, &present_support);

            if (present_support) {
                gpu->queue_indices[RE_VK_QUEUE_PRESENT] = idx;
                queues_assigned[RE_VK_QUEUE_PRESENT] = true;
                continue;
            }
        }

        if (!queues_assigned[RE_VK_QUEUE_TRANSFER] && family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_TRANSFER] = idx;
            queues_assigned[RE_VK_QUEUE_TRANSFER] = true;
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_COMPUTE] && family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_COMPUTE] = idx;
            queues_assigned[RE_VK_QUEUE_COMPUTE] = true;
            continue;
        }
    }

    re_free(families);

    if (!queues_assigned[RE_VK_QUEUE_GRAPHICS]) {
        return false;
    }

    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        if (!queues_assigned[idx]) {
            gpu->queue_indices[idx] = gpu->queue_indices[RE_VK_QUEUE_GRAPHICS];
            queues_assigned[idx] = true;
        }
    }

    return true;
}

// *=================================================
// *
// * __re_clearVulkanGPU
// *
// *=================================================

void __re_clearVulkanGPU(re_VkGPU* gpu) {
    if (gpu->formats != RE_NULL_HANDLE) {
        re_free(gpu->formats);
        gpu->formats = RE_NULL_HANDLE;
    }

    if (gpu->present_modes != RE_NULL_HANDLE) {
        re_free(gpu->present_modes);
        gpu->present_modes = RE_NULL_HANDLE;
    }
}

// *=================================================
// *
// * __re_scoreVulkanGPU
// *
// *=================================================

size_t __re_scoreVulkanGPU(const re_VkGPU* gpu) {
    size_t score = 0;

    if (gpu->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += __RE_VULKAN_DISCRETE_GPU_SCORE;
    }

    for (uint32_t idx = 0; idx < gpu->mem_properties.memoryHeapCount; ++idx) {
        const VkMemoryHeap* heap = &gpu->mem_properties.memoryHeaps[idx];

        if (heap->flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            score += heap->size; // ? Measured in bytes
        }
    }

    score += gpu->properties.limits.maxImageDimension2D;

    return score;
}

// *=================================================
// *
// * __re_createVulkanLogicalDevice
// *
// *=================================================

VkDevice __re_createVulkanLogicalDevice(
    const re_VkGPU* gpu,
    const char** enabled_extensions,
    const uint32_t enabled_extension_count,
    const VkAllocationCallbacks* allocator
) {
    // ? This may be altered if we ever need multiple queues from the same family
    // ? (typically for multithreading purposes)
    const float queue_priorities = 1.0f;

    uint32_t family_count = 0;
    VkDeviceQueueCreateInfo queue_create_infos[RE_VK_QUEUE_ROLE_COUNT] = {0};
    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const uint32_t family_index = gpu->queue_indices[idx];

        bool is_duplicate_queue = false;
        for (uint32_t jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (family_index == gpu->queue_indices[jdx]) {
                is_duplicate_queue = true;
                break;
            }
        }

        if (is_duplicate_queue) {
            continue;
        }

        VkDeviceQueueCreateInfo* queue_create_info = &queue_create_infos[family_count++];
        queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info->queueFamilyIndex = family_index;
        queue_create_info->queueCount = 1;
        queue_create_info->pQueuePriorities = &queue_priorities;
    }

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = family_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.enabledExtensionCount = enabled_extension_count;
    device_create_info.ppEnabledExtensionNames = enabled_extensions;
    device_create_info.pEnabledFeatures = &gpu->features;

    VkDevice logical_device = VK_NULL_HANDLE;
    const VkResult device_create_result = vkCreateDevice(
        gpu->physical_device,
        &device_create_info,
        allocator,
        &logical_device
    );

    re_assert(device_create_result == VK_SUCCESS, "Failed to create Vulkan logical device!");
    return logical_device;
}

// *=================================================
// *
// * __re_createVulkanCommandPool
// *
// *=================================================

VkCommandPool __re_createVulkanCommandPool(
    const VkDevice logical_device,
    const uint32_t family_index,
    const VkCommandPoolCreateFlags flags,
    const VkAllocationCallbacks* allocator
) {
    VkCommandPoolCreateInfo cmd_pool_create_info = {0};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = flags;
    cmd_pool_create_info.queueFamilyIndex = family_index;

    VkCommandPool cmd_pool = VK_NULL_HANDLE;
    const VkResult cmd_pool_create_result = vkCreateCommandPool(
        logical_device,
        &cmd_pool_create_info,
        allocator,
        &cmd_pool
    );

    re_assert(cmd_pool_create_result == VK_SUCCESS, "Failed to create Vulkan command pool!");
    return cmd_pool;
}

// *=================================================
// *
// * __re_createVulkanDescriptorPool
// *
// *=================================================

VkDescriptorPool __re_createVulkanDescriptorPool(
    const VkDevice logical_device,
    const VkDescriptorPoolSize* descriptor_sizes,
    const uint32_t descriptor_size_count,
    const VkAllocationCallbacks* allocator
) {
    uint32_t min_size = UINT32_MAX;
    for (uint32_t idx = 0; idx < descriptor_size_count; ++idx) {
        const uint32_t cur_size = descriptor_sizes[idx].descriptorCount;

        if (cur_size < min_size) {
            min_size = cur_size;
        } 
    }

    if (min_size == UINT32_MAX) {
        min_size = 1;
    }

    VkDescriptorPoolCreateInfo desc_pool_create_info = {0};
    desc_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_create_info.maxSets = min_size;
    desc_pool_create_info.pPoolSizes = descriptor_sizes;
    desc_pool_create_info.poolSizeCount = descriptor_size_count;

    VkDescriptorPool desc_pool = VK_NULL_HANDLE;
    const VkResult desc_pool_create_result = vkCreateDescriptorPool(
        logical_device,
        &desc_pool_create_info,
        allocator,
        &desc_pool
    );

    re_assert(desc_pool_create_result == VK_SUCCESS, "Failed to create Vulkan descriptor pool!");
    return desc_pool;
}

// *=================================================
// *
// * __re_createVulkanDeviceLayer
// *
// *=================================================

void __re_createVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const re_VkDeviceLayerCreateInfo* create_info,
    const VkAllocationCallbacks* allocator
) {
    re_assert(device_layer != RE_NULL_HANDLE, "Attempting to create Vulkan device layer with NULL memory!");

    uint32_t __gpu_count = 0;
    vkEnumeratePhysicalDevices(create_info->instance, &__gpu_count, VK_NULL_HANDLE);

    re_assert(__gpu_count > 0, "No available GPU devices for Vulkan!");

    const uint32_t gpu_count = __gpu_count;
    VkPhysicalDevice* gpus = (VkPhysicalDevice*)re_malloc(sizeof(VkPhysicalDevice) * gpu_count);
    vkEnumeratePhysicalDevices(create_info->instance, &__gpu_count, gpus);

    uint32_t gpu_idx = 0;
    size_t best_gpu_score = 0;

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        const bool is_suitable = __re_createVulkanGPU(
            gpus[gpu_idx],
            create_info->surface,
            create_info->enabled_extensions,
            create_info->enabled_extension_count,
            &device_layer->gpu
        );

        if (is_suitable) {
            best_gpu_score = __re_scoreVulkanGPU(&device_layer->gpu);
            break;
        }

        __re_clearVulkanGPU(&device_layer->gpu);
    }

    re_assert(best_gpu_score > 0, "No suitable GPU devices found for Vulkan!");

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        re_VkGPU cur_gpu = {0};
        const bool is_suitable = __re_createVulkanGPU(
            gpus[gpu_idx],
            create_info->surface,
            create_info->enabled_extensions,
            create_info->enabled_extension_count,
            &device_layer->gpu
        );

        if (!is_suitable) {
            __re_clearVulkanGPU(&cur_gpu);
            continue;
        }

        const size_t cur_score = __re_scoreVulkanGPU(&cur_gpu);

        if (cur_score <= best_gpu_score) {
            __re_clearVulkanGPU(&cur_gpu);
            continue;
        }

        __re_clearVulkanGPU(&device_layer->gpu);

        best_gpu_score = cur_score;
        device_layer->gpu = cur_gpu;
    }

    re_free(gpus);

    device_layer->logical_device = __re_createVulkanLogicalDevice(
        &device_layer->gpu,
        create_info->enabled_extensions,
        create_info->enabled_extension_count,
        allocator
    );

    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const uint32_t family_index = device_layer->gpu.queue_indices[idx];
        
        vkGetDeviceQueue(
            device_layer->logical_device,
            family_index,
            0,
            &device_layer->queues[idx]
        );

        bool is_clone = false;
        for (int32_t jdx = (int32_t)(idx) - 1; jdx >= 0; --jdx) {
            if (family_index == device_layer->gpu.queue_indices[jdx]) {
                is_clone = true;
                device_layer->cmd_pools[idx] = device_layer->cmd_pools[jdx];
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        device_layer->cmd_pools[idx] = __re_createVulkanCommandPool(
            device_layer->logical_device,
            family_index,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            allocator
        );
    }

    const VkDescriptorPoolSize descriptor_sizes[__RE_DESCRIPTOR_SIZE_COUNT] = {
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            create_info->max_texture_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_SAMPLER,
            create_info->max_sampler_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            create_info->max_storage_image_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            create_info->max_uniform_buffer_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            create_info->max_storage_buffer_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            create_info->max_input_attachment_count,
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            create_info->max_dynamic_uniform_buffer_count,
        },
    };

    device_layer->desc_pool = __re_createVulkanDescriptorPool(
        device_layer->logical_device,
        descriptor_sizes,
        __RE_DESCRIPTOR_SIZE_COUNT,
        allocator
    );

    return device_layer;
}

// *=================================================
// *
// * __re_destroyVulkanDeviceLayer
// *
// *=================================================

void __re_destroyVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
) {
    re_assert(device_layer != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan Device Layer!");

    __re_clearVulkanGPU(&device_layer->gpu);

    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const VkCommandPool cmd_pool = device_layer->cmd_pools[idx];

        bool is_clone = false;
        for (uint32_t jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (device_layer->cmd_pools[jdx] == cmd_pool) {
                is_clone = true;
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        vkDestroyCommandPool(device_layer->logical_device, cmd_pool, allocator);
    }

    vkDestroyDescriptorPool(device_layer->logical_device, device_layer->desc_pool, allocator);
    vkDestroyDevice(device_layer->logical_device, allocator);
}

#endif