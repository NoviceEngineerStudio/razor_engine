#ifdef RE_VULKAN_AVAILABLE

#include <re_debug.h>
#include <re_utils.h>
#include "./re_vulkan.h"
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
    app_info.pApplicationName = "Vulkan-Check";
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

re_VulkanContext __re_createVulkanContext(const re_GraphicsContextCreateInfo* create_info) {
    re_VulkanContext context = (re_VulkanContext)re_calloc(1, sizeof(re_VulkanContext_T));

    // TODO:

    re_VkDeviceLayerCreateInfo device_layer_create_info = {0}; // TODO:

    __re_createVulkanDeviceLayer(
        &context->device_layer,
        &device_layer_create_info,
        context->allocator
    );

    return context;
}

// *=================================================
// *
// * __re_destroyVulkanContext
// *
// *=================================================

void __re_destroyVulkanContext(re_VulkanContext* context) {
    re_assert(context != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan backend context!");

    re_VulkanContext context_data = *context;
    re_assert(context_data != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan backend context!");

    // TODO:

    __re_destroyVulkanDeviceLayer(&context_data->device_layer, context_data->allocator);

    re_free(context_data);
    *context = RE_NULL_HANDLE;
}

#endif