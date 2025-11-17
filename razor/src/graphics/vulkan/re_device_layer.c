#include "./re_device_layer.h"

#define __RE_DESCRIPTOR_SIZE_COUNT__ 7

#pragma region Private Methods

// *=================================================
// *
// * re_scoreVulkanGPU
// *
// *=================================================

re_size re_scoreVulkanGPU(const re_VkGPU* gpu) {
    re_size score = 0;

    if (gpu->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    for (re_u32 idx = 0; idx <  gpu->mem_properties.memoryHeapCount; ++idx) {
        const VkMemoryHeap* heap = &gpu->mem_properties.memoryHeaps[idx];

        if (heap->flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            score += heap->size;
        }
    }

    score += gpu->properties.limits.maxImageDimension2D;

    return score;
}

// *=================================================
// *
// * re_createVulkanGPU
// *
// *=================================================

re_bool re_createVulkanGPU(
    const VkPhysicalDevice physical_device,
    const VkSurfaceKHR surface,
    const char** enabled_extensions,
    const re_u32 enabled_extension_count,
    re_VkGPU* gpu
) {
    gpu->physical_device = physical_device;

    re_u32 __extension_count = 0;
    vkEnumerateDeviceExtensionProperties(gpu->physical_device, NULL, &__extension_count, NULL);

    if (__extension_count < enabled_extension_count) {
        return re_false;
    }

    const re_u32 extension_count = __extension_count;
    VkExtensionProperties* extensions = (VkExtensionProperties*)re_malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateDeviceExtensionProperties(gpu->physical_device, NULL, &__extension_count, extensions);

    for (re_u32 idx = 0; idx < enabled_extension_count; ++idx) {
        const char* enabled_extension = enabled_extensions[idx];

        re_bool found_extension = re_false;
        for (re_u32 jdx = 0; jdx < extension_count; ++jdx) {
            const char* extension = extensions[jdx].extensionName;

            if (re_isStrEqual(enabled_extension, extension)) {
                found_extension = re_true;
                break;
            }
        }

        if (!found_extension) {
            re_free(extensions);
            return re_false;
        }
    }

    re_free(extensions);

    vkGetPhysicalDeviceFeatures(physical_device, &gpu->features);
    vkGetPhysicalDeviceProperties(physical_device, &gpu->properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &gpu->mem_properties);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &gpu->capabilities);

    re_u32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
    gpu->format_count = format_count;

    if (format_count == 0) {
        return re_false;
    }

    gpu->formats = (VkSurfaceFormatKHR*)re_malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, gpu->formats);

    re_u32 present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
    gpu->present_mode_count = present_mode_count;

    if (present_mode_count == 0) {
        return re_false;
    }

    gpu->present_modes = (VkPresentModeKHR*)re_malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, gpu->present_modes);

    re_bool queues_assigned[RE_VK_QUEUE_ROLE_COUNT] = {0};

    re_u32 __family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, NULL);

    const re_u32 family_count = __family_count;
    VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)re_malloc(sizeof(VkQueueFamilyProperties) * family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, families);

    for (re_u32 idx = 0; idx < family_count; ++idx) {
        const VkQueueFamilyProperties family = families[idx];

        if (family.queueCount == 0) {
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_GRAPHICS] && family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_GRAPHICS] = idx;
            queues_assigned[RE_VK_QUEUE_GRAPHICS] = re_true;
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_PRESENT]) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, idx, surface, &present_support);

            if (present_support) {
                gpu->queue_indices[RE_VK_QUEUE_PRESENT] = idx;
                queues_assigned[RE_VK_QUEUE_PRESENT] = re_true;
                continue;
            }
        }

        if (!queues_assigned[RE_VK_QUEUE_TRANSFER] && family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_TRANSFER] = idx;
            queues_assigned[RE_VK_QUEUE_TRANSFER] = re_true;
            continue;
        }

        if (!queues_assigned[RE_VK_QUEUE_COMPUTE] && family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            gpu->queue_indices[RE_VK_QUEUE_COMPUTE] = idx;
            queues_assigned[RE_VK_QUEUE_COMPUTE] = re_true;
            continue;
        }
    }

    re_free(families);

    if (!queues_assigned[RE_VK_QUEUE_GRAPHICS]) {
        return re_false;
    }

    for (re_u32 idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        if (!queues_assigned[idx]) {
            gpu->queue_indices[idx] = gpu->queue_indices[RE_VK_QUEUE_GRAPHICS];
            queues_assigned[idx] = re_true;
        }
    }

    return re_true;
}

// *=================================================
// *
// * re_clearVulkanGPU
// *
// *=================================================

void re_clearVulkanGPU(re_VkGPU* gpu) {
    if (gpu->formats != NULL) {
        re_free(gpu->formats);
        gpu->formats = NULL;
    }

    if (gpu->present_modes != NULL) {
        re_free(gpu->present_modes);
        gpu->present_modes = NULL;
    }
}

// *=================================================
// *
// * re_createVulkanLogicalDevice
// *
// *=================================================

VkDevice re_createVulkanLogicalDevice(
    const re_VkGPU* gpu,
    const char** enabled_extensions,
    const re_u32 enabled_extension_count,
    const VkAllocationCallbacks* allocator
) {
    // This may be altered if we ever need multiple queues from the same family
    // (typically for multithreading purposes)
    const re_f32 queue_priorities = 1.0f;

    re_u32 family_count = 0;
    VkDeviceQueueCreateInfo queue_create_infos[RE_VK_QUEUE_ROLE_COUNT] = {0};
    for (re_u32 idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const re_u32 family_index = gpu->queue_indices[idx];

        re_bool is_duplicate_queue = re_false;
        for (re_u32 jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (family_index == gpu->queue_indices[jdx]) {
                is_duplicate_queue = re_true;
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
// * re_createVulkanCommandPool
// *
// *=================================================

VkCommandPool re_createVulkanCommandPool(
    const VkDevice logical_device,
    const re_u32 family_index,
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
// * re_createVulkanDescriptorPool
// *
// *=================================================

VkDescriptorPool re_createVulkanDescriptorPool(
    const VkDevice logical_device,
    const VkDescriptorPoolSize* descriptor_sizes,
    const re_u32 descriptor_size_count,
    const VkAllocationCallbacks* allocator
) {
    re_u32 min_size = RE_MAX_U32;
    for (re_u32 idx = 0; idx < descriptor_size_count; ++idx) {
        min_size = re_minU32(min_size, descriptor_sizes[idx].descriptorCount);   
    }

    if (min_size == RE_MAX_U32) {
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

#pragma endregion
#pragma region Public Methods

// *=================================================
// *
// * re_createVulkanDeviceLayer
// *
// *=================================================

re_VkDeviceLayer re_createVulkanDeviceLayer(
    const re_VkDeviceLayerCreateInfo* create_info,
    const VkAllocationCallbacks* allocator
) {
    re_VkDeviceLayer device_layer = (re_VkDeviceLayer)re_malloc(sizeof(re_VkDeviceLayer_T));

    re_u32 __gpu_count = 0;
    vkEnumeratePhysicalDevices(create_info->instance, &__gpu_count, NULL);

    const re_u32 gpu_count = __gpu_count;
    VkPhysicalDevice* gpus = (VkPhysicalDevice*)re_malloc(sizeof(VkPhysicalDevice) * gpu_count);
    vkEnumeratePhysicalDevices(create_info->instance, &__gpu_count, gpus);

    re_u32 gpu_idx = 0;
    re_size best_gpu_score = 0;

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        const re_bool is_suitable = re_createVulkanGPU(
            gpus[gpu_idx],
            create_info->surface,
            create_info->enabled_extensions,
            create_info->enabled_extension_count,
            &device_layer->gpu
        );

        if (is_suitable) {
            best_gpu_score = re_scoreVulkanGPU(&device_layer->gpu);
            break;
        }

        re_clearVulkanGPU(&device_layer->gpu);
    }

    re_assert(best_gpu_score > 0, "No suitable GPU devices found for Vulkan!");

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        re_VkGPU cur_gpu = {0};
        const re_bool is_suitable = re_createVulkanGPU(
            gpus[gpu_idx],
            create_info->surface,
            create_info->enabled_extensions,
            create_info->enabled_extension_count,
            &cur_gpu
        );

        if (!is_suitable) {
            re_clearVulkanGPU(&cur_gpu);
            continue;
        }

        const re_size cur_score = re_scoreVulkanGPU(&cur_gpu);

        if (cur_score > best_gpu_score) {
            re_clearVulkanGPU(&device_layer->gpu);

            best_gpu_score = cur_score;
            device_layer->gpu = cur_gpu;

            continue;
        }

        re_clearVulkanGPU(&cur_gpu);
    }

    re_free(gpus);

    device_layer->logical_device = re_createVulkanLogicalDevice(
        &device_layer->gpu,
        create_info->enabled_extensions,
        create_info->enabled_extension_count,
        allocator
    );

    for (re_u32 idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const re_u32 family_index = device_layer->gpu.queue_indices[idx];
        
        vkGetDeviceQueue(
            device_layer->logical_device,
            family_index,
            0,
            &device_layer->queues[idx]
        );

        re_bool is_clone = re_false;
        for (re_i32 jdx = (re_i32)(idx) - 1; jdx >= 0; --jdx) {
            if (family_index == device_layer->gpu.queue_indices[jdx]) {
                is_clone = re_true;
                device_layer->cmd_pools[idx] = device_layer->cmd_pools[jdx];
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        device_layer->cmd_pools[idx] = re_createVulkanCommandPool(
            device_layer->logical_device,
            family_index,
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            allocator
        );
    }

    const VkDescriptorPoolSize descriptor_sizes[__RE_DESCRIPTOR_SIZE_COUNT__] = {
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

    device_layer->desc_pool = re_createVulkanDescriptorPool(
        device_layer->logical_device,
        descriptor_sizes,
        __RE_DESCRIPTOR_SIZE_COUNT__,
        allocator
    );

    return device_layer;
}

// *=================================================
// *
// * re_destroyVulkanDeviceLayer
// *
// *=================================================

void re_destroyVulkanDeviceLayer(
    re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
) {
    re_assert(device_layer != NULL, "Attempting to destroy NULL Vulkan Device Layer!");

    re_VkDeviceLayer device_layer_data = *device_layer;
    re_assert(device_layer_data != NULL, "Attempting to destroy NULL Vulkan Device Layer!");

    re_clearVulkanGPU(&device_layer_data->gpu);

    for (re_u32 idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const VkCommandPool cmd_pool = device_layer_data->cmd_pools[idx];

        re_bool is_clone = re_false;
        for (re_u32 jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (device_layer_data->cmd_pools[jdx] == cmd_pool) {
                is_clone = re_true;
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        vkDestroyCommandPool(device_layer_data->logical_device, cmd_pool, allocator);
    }

    vkDestroyDescriptorPool(device_layer_data->logical_device, device_layer_data->desc_pool, allocator);
    vkDestroyDevice(device_layer_data->logical_device, allocator);

    re_free(device_layer_data);
    *device_layer = NULL;
}

#pragma endregion