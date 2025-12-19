#ifdef RE_VULKAN_AVAILABLE

#include "./re_vulkan_utils.h"

#include <re_core.h>
#include <re_debug.h>
#include <re_utils.h>
#include "./re_vulkan.h"
#include "../../re_internals.h"
#include "../../core/re_vulkan_window.h"

#ifdef RE_LOGGER_ENABLED

#define __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT 1u
static const char* __RE_VULKAN_ENABLED_INSTANCE_LAYERS[__RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT] = {
    "VK_LAYER_KHRONOS_validation"
};

#else

#define __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT 0u
static const char* __RE_VULKAN_ENABLED_INSTANCE_LAYERS = RE_NULL_HANDLE;

#endif

#define __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT 1u
static const char* __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS[__RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#define __RE_VULKAN_DISCRETE_GPU_SCORE 5000u

#define __RE_DESCRIPTOR_SIZE_COUNT 7u

#define __RE_VULKAN_TEXTURE_COUNT_BASE 512u
#define __RE_VULKAN_SAMPLER_COUNT_BASE 64u
#define __RE_VULKAN_STORAGE_IMAGE_COUNT_BASE 128u
#define __RE_VULKAN_UNIFORM_BUFFER_COUNT_BASE 256u
#define __RE_VULKAN_STORAGE_BUFFER_COUNT_BASE 256u
#define __RE_VULKAN_INPUT_ATTACHMENT_COUNT_BASE 16u
#define __RE_VULKAN_DYNAMIC_UNIFORM_BUFFER_COUNT_BASE 128u

#define __RE_VULKAN_SIMPLE_DESCRIPTOR_SIZE_MULTIPLIER 1u
#define __RE_VULKAN_STANDARD_DESCRIPTOR_SIZE_MULTIPLIER 2u
#define __RE_VULKAN_HEAVY_DESCRIPTOR_SIZE_MULTIPLIER 4u

#define __RE_VULKAN_CMD_POOL_FRAME_FLAGS VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
#define __RE_VULKAN_CMD_POOL_STATIC_FLAGS VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
#define __RE_VULKAN_CMD_POOL_ONE_SHOT_FLAGS VK_COMMAND_POOL_CREATE_TRANSIENT_BIT

// *=================================================
// *
// * __re_fillVulkanGPU
// *
// *=================================================

bool __re_fillVulkanGPU(
    const VkPhysicalDevice physical_device,
    const VkSurfaceKHR surface,
    re_VkGPU* gpu
) {
    gpu->physical_device = physical_device;

    if (__RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT > 0) {
        uint32_t __extension_count = 0;
        vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &__extension_count, VK_NULL_HANDLE);

        if (__extension_count < __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT) {
            return false;
        }

        const uint32_t extension_count = __extension_count;
        VkExtensionProperties* extensions = (VkExtensionProperties*)re_malloc(extension_count * sizeof(VkExtensionProperties));
        vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &__extension_count, extensions);

        for (uint32_t idx = 0; idx < __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT; ++idx) {
            const char* enabled_extension = __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS[idx];

            bool found_extension = false;
            for (uint32_t jdx = 0; jdx < extension_count; ++jdx) {
                if (re_isStrEqual(enabled_extension, extensions[jdx].extensionName)) {
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
    }

    vkGetPhysicalDeviceFeatures(physical_device, &gpu->features);
    vkGetPhysicalDeviceProperties(physical_device, &gpu->properties);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &gpu->mem_properties);
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &gpu->capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, VK_NULL_HANDLE);

    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, VK_NULL_HANDLE);
    
    if (format_count == 0 || present_mode_count == 0) {
        return false;
    }

    gpu->format_count = format_count;
    gpu->formats = (VkSurfaceFormatKHR*)re_malloc(sizeof(VkSurfaceFormatKHR) * format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, gpu->formats);
    
    gpu->present_mode_count = present_mode_count;
    gpu->present_modes = (VkPresentModeKHR*)re_malloc(sizeof(VkPresentModeKHR) * present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, gpu->present_modes);

    uint32_t __family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, VK_NULL_HANDLE);

    if (__family_count == 0) {
        return false;
    }
    
    const uint32_t family_count = __family_count;
    VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)re_malloc(sizeof(VkQueueFamilyProperties) * family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &__family_count, families);
    
    bool queues_assigned[RE_VK_QUEUE_ROLE_COUNT] = {0};
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
            score += heap->size;
        }
    }

    score += gpu->properties.limits.maxImageDimension2D;

    return score;
}

// *=================================================
// *
// * __re_createVulkanInstance
// *
// *=================================================

VkInstance __re_createVulkanInstance(const VkAllocationCallbacks* allocator) {
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = __RE_VULKAN_API_VER;
    app_info.pApplicationName = RE_APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(
        RE_APP_MAJOR_VER,
        RE_APP_MINOR_VER,
        RE_APP_PATCH_VER
    );
    app_info.pEngineName = RE_ENGINE_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(
        RE_ENGINE_MAJOR_VER,
        RE_ENGINE_MINOR_VER,
        RE_ENGINE_PATCH_VER
    );

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.ppEnabledExtensionNames = __re_getVulkanWindowExtensions(
        &instance_create_info.enabledExtensionCount
    );
    instance_create_info.enabledLayerCount = __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT;
    instance_create_info.ppEnabledLayerNames = __RE_VULKAN_ENABLED_INSTANCE_LAYERS;

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult instance_create_result = vkCreateInstance(&instance_create_info, allocator, &instance);

    re_assert(instance_create_result == VK_SUCCESS, "Failed to create Vulkan instance!");

    return instance;
}

// *=================================================
// *
// * __re_selectVulkanGPU
// *
// *=================================================

re_VkGPU __re_selectVulkanGPU(
    const VkInstance instance,
    const VkSurfaceKHR surface
) {
    uint32_t __gpu_count = 0;
    vkEnumeratePhysicalDevices(instance, &__gpu_count, VK_NULL_HANDLE);

    re_assert(__gpu_count > 0, "No available GPU devices for Vulkan!");

    const uint32_t gpu_count = __gpu_count;
    VkPhysicalDevice* gpus = (VkPhysicalDevice*)re_malloc(gpu_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &__gpu_count, gpus);

    uint32_t gpu_idx = 0;
    re_VkGPU best_gpu = {0};
    size_t best_gpu_score = 0;

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        const bool is_suitable = __re_fillVulkanGPU(gpus[gpu_idx], surface, &best_gpu);

        if (is_suitable) {
            best_gpu_score = __re_scoreVulkanGPU(&best_gpu);
            break;
        }

        __re_clearVulkanGPU(&best_gpu);
    }

    re_assert(best_gpu_score > 0, "No suitable GPU devices found for Vulkan!");

    for (; gpu_idx < gpu_count; ++gpu_idx) {
        re_VkGPU cur_gpu = {0};
        const bool is_suitable = __re_fillVulkanGPU(gpus[gpu_idx], surface, &cur_gpu);

        if (!is_suitable) {
            __re_clearVulkanGPU(&cur_gpu);
            continue;
        }

        const size_t cur_gpu_score = __re_scoreVulkanGPU(&cur_gpu);

        if (cur_gpu_score <= best_gpu_score) {
            __re_clearVulkanGPU(&cur_gpu);
            continue;
        }

        __re_clearVulkanGPU(&best_gpu);

        best_gpu = cur_gpu;
        best_gpu_score = cur_gpu_score;
    }

    re_free(gpus);

    return best_gpu;
}

// *=================================================
// *
// * __re_clearVulkanGPU
// *
// *=================================================

void __re_clearVulkanGPU(re_VkGPU* gpu) {
    gpu->physical_device = VK_NULL_HANDLE;

    if (gpu->formats != RE_NULL_HANDLE) {
        re_free(gpu->formats);
        gpu->formats = RE_NULL_HANDLE;
    }
    
    if (gpu->present_modes != RE_NULL_HANDLE) {
        re_free(gpu->present_modes);
        gpu->present_modes = RE_NULL_HANDLE;
    }

    gpu->format_count = 0;
    gpu->present_mode_count = 0;
}

// *=================================================
// *
// * __re_createVulkanLogicalDevice
// *
// *=================================================

VkDevice __re_createVulkanLogicalDevice(
    const re_VkGPU* gpu,
    const VkAllocationCallbacks* allocator
) {
    // TODO: This may be altered if we ever need multiple queues from the same family
    // ? If this occurs, the algorithm below will need to take the maximum number of queues
    // ? per family group (shared indices).
    const float queue_priorities[1] = { 1.0f };

    uint32_t unique_queue_count = 0;
    VkDeviceQueueCreateInfo queue_create_infos[RE_VK_QUEUE_ROLE_COUNT] = {0};

    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const uint32_t family_index = gpu->queue_indices[idx];

        bool is_duplicate_index = false;
        for (uint32_t jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (family_index == gpu->queue_indices[jdx]) {
                is_duplicate_index = true;
                break;
            }
        }
        
        if (is_duplicate_index) {
            continue;
        }

        VkDeviceQueueCreateInfo* queue_create_info = &queue_create_infos[unique_queue_count++];
        queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info->queueFamilyIndex = family_index;
        queue_create_info->queueCount = 1; // TODO: Should match queue_priorities size (static 1 for now)
        queue_create_info->pQueuePriorities = queue_priorities;
    }

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = unique_queue_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.enabledExtensionCount = __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT;
    device_create_info.ppEnabledExtensionNames = __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS;
    device_create_info.pEnabledFeatures = &gpu->features; // TODO: This should be more selective!

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
// * __re_getVulkanQueues
// *
// *=================================================

void __re_getVulkanQueues(
    VkQueue* queue_arr,
    const uint32_t queue_count,
    const uint32_t queue_family_index,
    const VkDevice logical_device
) {
    for (uint32_t idx = 0; idx < queue_count; ++idx) {
        vkGetDeviceQueue(
            logical_device,
            queue_family_index,
            idx,
            &queue_arr[idx]
        );
    }
}

// *=================================================
// *
// * __re_createVulkanCommandPools
// *
// *=================================================

void __re_createVulkanCommandPools(
    VkCommandPool* cmd_pool_arr,
    const VkDevice logical_device,
    const re_VkGPU* gpu,
    const VkAllocationCallbacks* allocator
) {
    for (uint32_t queue_role = 0; queue_role < RE_VK_QUEUE_ROLE_COUNT; ++queue_role) {
        if (queue_role == RE_VK_QUEUE_PRESENT) {
            continue;
        }

        const uint32_t queue_family_index = gpu->queue_indices[queue_role];


        bool is_duplicate = false;
        for (int32_t other_queue_role = ((int32_t)queue_role) - 1; other_queue_role >= 0; --other_queue_role) {
            if (other_queue_role == RE_VK_QUEUE_PRESENT) {
                continue;
            }
            
            const uint32_t other_family_index = gpu->queue_indices[other_queue_role];

            if (queue_family_index == other_family_index) {
                is_duplicate = true;
                
                for (uint32_t pool_role = 0; pool_role < RE_VK_CMD_POOL_ROLE_COUNT; ++pool_role) {
                    for (uint32_t thread_idx = 0; thread_idx < RE_MAX_VULKAN_THREADS; ++thread_idx) {
                        cmd_pool_arr[__re_getVulkanCmdPoolIndex(thread_idx, queue_role, pool_role)] = 
                        cmd_pool_arr[__re_getVulkanCmdPoolIndex(thread_idx, other_queue_role, pool_role)];
                    }
                }

                break;
            }
        }

        if (is_duplicate) {
            continue;
        }

        VkCommandPoolCreateInfo cmd_pool_create_info = {0};
        cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmd_pool_create_info.queueFamilyIndex = queue_family_index;

        for (uint32_t pool_role = 0; pool_role < RE_VK_CMD_POOL_ROLE_COUNT; ++pool_role) {
            switch (pool_role) {
                case RE_VK_CMD_POOL_FRAME: {
                    cmd_pool_create_info.flags = __RE_VULKAN_CMD_POOL_FRAME_FLAGS;
                    break;
                }

                case RE_VK_CMD_POOL_STATIC: {
                    cmd_pool_create_info.flags = __RE_VULKAN_CMD_POOL_STATIC_FLAGS;
                    break;
                }

                case RE_VK_CMD_POOL_ONE_SHOT: {
                    cmd_pool_create_info.flags = __RE_VULKAN_CMD_POOL_ONE_SHOT_FLAGS;
                    break;
                }

                default: {
                    re_assert(false, "Unknown Vulkan command pool type used in creation! Type: %d", pool_role);
                    break;
                }
            }

            for (uint32_t thread_idx = 0; thread_idx < RE_MAX_VULKAN_THREADS; ++thread_idx) {
                const VkResult cmd_pool_create_result = vkCreateCommandPool(
                    logical_device,
                    &cmd_pool_create_info,
                    allocator,
                    &cmd_pool_arr[__re_getVulkanCmdPoolIndex(thread_idx, queue_role, pool_role)]
                );

                re_assert(cmd_pool_create_result == VK_SUCCESS, "Failed to create Vulkan command pool!");
            }
        }
    }
}

// *=================================================
// *
// * __re_createVulkanDescriptorPool
// *
// *=================================================

VkDescriptorPool __re_createVulkanDescriptorPool(
    const VkDevice logical_device,
    const re_RenderProfile profile,
    const VkAllocationCallbacks* allocator
) {
    uint32_t descriptor_multiplier = 0u;

    switch (profile) {
        case RE_RENDERER_SIMPLE: {
            descriptor_multiplier = __RE_VULKAN_SIMPLE_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }

        case RE_RENDERER_STANDARD: {
            descriptor_multiplier = __RE_VULKAN_STANDARD_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }

        case RE_RENDERER_HEAVY: {
            descriptor_multiplier = __RE_VULKAN_HEAVY_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }
        
        default: {
            re_assert(false, "Vulkan does not recognize renderer profile with value: %d", profile);
            break;
        }
    }

    const VkDescriptorPoolSize descriptor_sizes[__RE_DESCRIPTOR_SIZE_COUNT] = {
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            __RE_VULKAN_TEXTURE_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_SAMPLER,
            __RE_VULKAN_SAMPLER_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            __RE_VULKAN_STORAGE_IMAGE_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            __RE_VULKAN_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            __RE_VULKAN_STORAGE_BUFFER_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
            __RE_VULKAN_INPUT_ATTACHMENT_COUNT_BASE * descriptor_multiplier
        },
        (VkDescriptorPoolSize){
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            __RE_VULKAN_DYNAMIC_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier
        },
    };

    uint32_t min_size = UINT32_MAX;
    for (uint32_t idx = 0; idx < __RE_DESCRIPTOR_SIZE_COUNT; ++idx) {
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
    desc_pool_create_info.poolSizeCount = __RE_DESCRIPTOR_SIZE_COUNT;
    desc_pool_create_info.pPoolSizes = descriptor_sizes;

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

#endif