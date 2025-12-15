#ifdef RE_VULKAN_AVAILABLE

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

#define RE_VULKAN_TEXTURE_COUNT_BASE 512u
#define RE_VULKAN_SAMPLER_COUNT_BASE 64u
#define RE_VULKAN_STORAGE_IMAGE_COUNT_BASE 128u
#define RE_VULKAN_UNIFORM_BUFFER_COUNT_BASE 256u
#define RE_VULKAN_STORAGE_BUFFER_COUNT_BASE 256u
#define RE_VULKAN_INPUT_ATTACHMENT_COUNT_BASE 16u
#define RE_VULKAN_DYNAMIC_UNIFORM_BUFFER_COUNT_BASE 128u

#define RE_VULKAN_SIMPLE_DESCRIPTOR_SIZE_MULTIPLIER 1u
#define RE_VULKAN_STANDARD_DESCRIPTOR_SIZE_MULTIPLIER 2u
#define RE_VULKAN_HEAVY_DESCRIPTOR_SIZE_MULTIPLIER 4u

typedef struct re_VulkanContext_T {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkRenderPass render_pass;

    re_VkDeviceLayer device_layer;
    re_VkSwapChainLayer swap_chain_layer;

    VkAllocationCallbacks* allocator;
} re_VulkanContext_T;

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

    // ? Set to NULL for now, but may change for efficiency or debug purposes later
    const VkAllocationCallbacks* allocator = VK_NULL_HANDLE;
    context->allocator = allocator;

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
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
    app_info.apiVersion = __RE_VULKAN_API_VER;

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.ppEnabledExtensionNames = __re_getVulkanWindowExtensions(
        &instance_create_info.enabledExtensionCount
    );
    instance_create_info.enabledLayerCount = __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT;
    instance_create_info.ppEnabledLayerNames = __RE_VULKAN_ENABLED_INSTANCE_LAYERS;

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult instance_create_result = vkCreateInstance(
        &instance_create_info,
        allocator,
        &instance
    );

    re_assert(instance_create_result == VK_SUCCESS, "Failed to create Vulkan instance!");
    context->instance = instance;

    const VkSurfaceKHR surface = __re_createVulkanSurface(create_info->window, instance, allocator);
    context->surface = surface;

    re_VkDeviceLayerCreateInfo device_layer_create_info = {0};
    device_layer_create_info.instance = instance;
    device_layer_create_info.surface = surface;
    device_layer_create_info.enabled_extensions = __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS;
    device_layer_create_info.enabled_extension_count = __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT;

    uint32_t descriptor_multiplier = 0u;

    switch (create_info->profile) {
        case RE_RENDERER_SIMPLE: {
            descriptor_multiplier = RE_VULKAN_SIMPLE_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }

        case RE_RENDERER_STANDARD: {
            descriptor_multiplier = RE_VULKAN_STANDARD_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }

        case RE_RENDERER_HEAVY: {
            descriptor_multiplier = RE_VULKAN_HEAVY_DESCRIPTOR_SIZE_MULTIPLIER;
            break;
        }
        
        default: {
            re_assert(false, "Vulkan does not recognize renderer profile with value: %d", create_info->profile);
            break;
        }
    }

    device_layer_create_info.max_texture_count = RE_VULKAN_TEXTURE_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_sampler_count = RE_VULKAN_SAMPLER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_storage_image_count = RE_VULKAN_STORAGE_IMAGE_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_uniform_buffer_count = RE_VULKAN_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_storage_buffer_count = RE_VULKAN_STORAGE_BUFFER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_input_attachment_count = RE_VULKAN_INPUT_ATTACHMENT_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_dynamic_uniform_buffer_count = RE_VULKAN_DYNAMIC_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier;

    __re_createVulkanDeviceLayer(
        &context->device_layer,
        &device_layer_create_info,
        allocator
    );

    // TODO: Create Render Pass
    
    // TODO: Create Swap Chain Layer

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

    const VkAllocationCallbacks* allocator = context_data->allocator;

    // TODO: Destroy Swap Chain Layer

    vkDestroyRenderPass(context_data->device_layer.logical_device, context_data->render_pass, allocator);

    __re_destroyVulkanDeviceLayer(&context_data->device_layer, allocator);

    vkDestroySurfaceKHR(context_data->instance, context_data->surface, allocator);
    vkDestroyInstance(context_data->instance, allocator);

    if (allocator != VK_NULL_HANDLE) {
        re_free(allocator);
    }

    re_free(context_data);
    *context = RE_NULL_HANDLE;
}

#endif