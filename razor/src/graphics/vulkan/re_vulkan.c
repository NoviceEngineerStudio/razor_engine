#ifdef RE_VULKAN_AVAILABLE

#include "./re_vulkan.h"

#include <re_debug.h>
#include <re_utils.h>
#include "./re_vulkan_types.h"
#include "./re_vulkan_utils.h"
#include "../../core/re_vulkan_window.h"

// *=================================================
// *
// * __re_vulkanAvailable
// *
// *=================================================

bool __re_vulkanAvailable() {
    uint32_t req_extension_count = 0;
    const char** req_extensions = __re_getVulkanWindowExtensions(&req_extension_count);

    uint32_t __instance_extension_count = 0;
    const VkResult get_extensions_result = vkEnumerateInstanceExtensionProperties(
        VK_NULL_HANDLE,
        &__instance_extension_count,
        VK_NULL_HANDLE
    );

    if (get_extensions_result != VK_SUCCESS || __instance_extension_count < req_extension_count) {
        return false;
    }

    if (__instance_extension_count > 0) {
        const uint32_t instance_extension_count = __instance_extension_count;
        VkExtensionProperties* instance_extensions = (VkExtensionProperties*)re_malloc(sizeof(VkExtensionProperties) * instance_extension_count);
        vkEnumerateInstanceExtensionProperties(
            VK_NULL_HANDLE,
            &__instance_extension_count,
            instance_extensions
        );

        for (uint32_t idx = 0; idx < req_extension_count; ++idx) {
            const char* req_extension = req_extensions[idx];

            bool is_present = false;
            for (uint32_t jdx = 0; jdx < instance_extension_count; ++jdx) {
                const char* instance_extension = instance_extensions[jdx].extensionName;

                if (re_isStrEqual(req_extension, instance_extension)) {
                    is_present = true;
                    break;
                }
            }

            if (!is_present) {
                re_free(instance_extensions);
                return false;
            }
        }

        re_free(instance_extensions);
    }

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Razor Engine Vulkan Check";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = RE_ENGINE_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(
        RE_ENGINE_MAJOR_VER,
        RE_ENGINE_MINOR_VER,
        RE_ENGINE_PATCH_VER
    );
    app_info.apiVersion = __RE_VULKAN_API_VER;

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult instance_create_result = vkCreateInstance(&instance_create_info, VK_NULL_HANDLE, &instance);

    if (instance_create_result != VK_SUCCESS) {
        return false;
    }

    uint32_t __gpu_count = 0;
    const VkResult get_gpus_result = vkEnumeratePhysicalDevices(instance, &__gpu_count, VK_NULL_HANDLE);
    vkDestroyInstance(instance, VK_NULL_HANDLE);

    return __gpu_count > 0 && get_gpus_result == VK_SUCCESS;
}

// *=================================================
// *
// * __re_createVulkanContext
// *
// *=================================================

re_VkContext __re_createVulkanContext(const re_GraphicsInstanceCreateInfo* create_info) {
    re_VkContext context = (re_VkContext)re_calloc(1, sizeof(re_VkContext_T));

    // ? Set to NULL for now, but may change later for efficiency or debug purposes.
    VkAllocationCallbacks* allocator = VK_NULL_HANDLE;
    context->allocator = allocator;

    const VkInstance instance = __re_createVulkanInstance(allocator);
    context->instance = instance;

    const VkSurfaceKHR surface = __re_createVulkanSurface(create_info->window, instance, allocator);
    context->surface = surface;

    context->gpu = __re_selectVulkanGPU(instance, surface);
    const re_VkGPU* gpu = &context->gpu;

    const VkDevice logical_device = __re_createVulkanLogicalDevice(gpu, allocator);
    context->logical_device = logical_device;

    // ? The following all assume one queue per family (may be changed later)

    __re_getVulkanQueues(
        &context->queues[RE_VK_QUEUE_GRAPHICS],
        1,
        gpu->queue_indices[RE_VK_QUEUE_GRAPHICS],
        logical_device
    );
    __re_getVulkanQueues(
        &context->queues[RE_VK_QUEUE_PRESENT],
        1,
        gpu->queue_indices[RE_VK_QUEUE_PRESENT],
        logical_device
    );
    __re_getVulkanQueues(
        &context->queues[RE_VK_QUEUE_COMPUTE],
        1,
        gpu->queue_indices[RE_VK_QUEUE_COMPUTE],
        logical_device
    );
    __re_getVulkanQueues(
        &context->queues[RE_VK_QUEUE_TRANSFER],
        1,
        gpu->queue_indices[RE_VK_QUEUE_TRANSFER],
        logical_device
    );

    __re_createVulkanCommandPools(
        context->cmd_pools,
        logical_device,
        gpu,
        allocator
    );

    const VkDescriptorPool desc_pool = __re_createVulkanDescriptorPool(logical_device, create_info->profile, allocator);
    context->desc_pool = desc_pool;

    return context;
}

// *=================================================
// *
// * __re_destroyVulkanContext
// *
// *=================================================

void __re_destroyVulkanContext(re_VkContext* context) {
    re_assert(context != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan context!");

    re_VkContext context_data = *context;
    re_assert(context_data != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan context!");

    VkAllocationCallbacks* allocator = context_data->allocator;

    __re_clearVulkanGPU(&context_data->gpu);

    VkDevice logical_device = context_data->logical_device;
    VkCommandPool* cmd_pools = context_data->cmd_pools;

    for (uint32_t idx = 0; idx < RE_VULKAN_MAX_CMD_POOLS; ++idx) {
        const VkCommandPool cmd_pool = cmd_pools[idx];

        bool is_clone = false;
        for (uint32_t jdx = idx + 1; jdx < RE_VULKAN_MAX_CMD_POOLS; ++jdx) {
            if (cmd_pools[jdx] == cmd_pool) {
                is_clone = true;
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        vkDestroyCommandPool(logical_device, cmd_pool, allocator);
    }

    vkDestroyDescriptorPool(logical_device, context_data->desc_pool, allocator);
    vkDestroyDevice(logical_device, allocator);

    vkDestroySurfaceKHR(context_data->instance, context_data->surface, allocator);
    vkDestroyInstance(context_data->instance, allocator);

    if (allocator != VK_NULL_HANDLE) {
        re_free(allocator);
    }

    re_free(context_data);
    *context = RE_NULL_HANDLE;
}

#endif