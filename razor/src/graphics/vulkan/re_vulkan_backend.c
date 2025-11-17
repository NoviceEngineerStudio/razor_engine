#include "./re_vulkan_backend.h"

#include <vulkan/vulkan.h>
#include "./re_device_layer.h"
#include "../../windows/re_vulkan_window.h"

#define __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT__ 1
static const char* __RE_VULKAN_ENABLED_INSTANCE_LAYERS__[__RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT__] = {
    "VK_LAYER_KHRONOS_validation"
};

#define __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT__ 1
static const char* __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS__[__RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT__] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

typedef struct re_VulkanBackend_T {
    VkInstance instance;
    VkSurfaceKHR surface;

    re_VkDeviceLayer device_layer;

    VkAllocationCallbacks* allocator;
} re_VulkanBackend_T;

typedef re_VulkanBackend_T* re_VulkanBackend;

#pragma region Backend API Methods

// *=================================================
// *
// * re_destroyVulkanBackend
// *
// *=================================================

void re_destroyVulkanBackend(re_GraphicsBackend* backend) {
    re_assert(backend != NULL, "Attempting to destroy NULL Vulkan backend!");
    re_assert(*backend != NULL, "Attempting to destroy NULL Vulkan backend!");

    re_VulkanBackend vulkan_backend = (*backend)->__user_data__;
    re_assert(vulkan_backend != NULL, "Attempting to destroy uninitialized Vulkan backend! Likely tampered with __user_data__...");

    re_destroyVulkanDeviceLayer(&vulkan_backend->device_layer, vulkan_backend->allocator);

    vkDestroySurfaceKHR(vulkan_backend->instance, vulkan_backend->surface, vulkan_backend->allocator);
    vkDestroyInstance(vulkan_backend->instance, vulkan_backend->allocator);

    if (vulkan_backend->allocator != NULL) {
        re_free(vulkan_backend->allocator);
    }

    re_free(*backend);
    *backend = NULL;
}

#pragma endregion
#pragma region Public Methods

// *=================================================
// *
// * re_isVulkanSupported
// *
// *=================================================

re_bool re_isVulkanSupported() {
    re_u32 required_extension_count = 0;
    const char** required_extensions = re_getVulkanWindowExtensions(&required_extension_count);

    re_u32 __instance_extension_count = 0;
    const VkResult extension_result = vkEnumerateInstanceExtensionProperties(NULL, &__instance_extension_count, NULL);

    if (extension_result != VK_SUCCESS || __instance_extension_count < required_extension_count) {
        return re_false;
    }

    const re_u32 instance_extension_count = __instance_extension_count;
    VkExtensionProperties* instance_extensions = (VkExtensionProperties*)re_malloc(sizeof(VkExtensionProperties) * instance_extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &__instance_extension_count, instance_extensions);

    if (instance_extension_count < required_extension_count) {
        re_free(instance_extensions);
        return re_false;
    }

    for (re_u32 idx = 0; idx < required_extension_count; ++idx) {
        const char* required_extension = required_extensions[idx];

        re_bool is_present = re_false;
        for (re_u32 jdx = 0; jdx < instance_extension_count; ++jdx) {
            const char* instance_extension = instance_extensions[jdx].extensionName;

            if (re_isStrEqual(required_extension, instance_extension)) {
                is_present = re_true;
                break;
            }
        }

        if (!is_present) {
            re_free(instance_extensions);
            return re_false;
        }
    }

    re_free(instance_extensions);

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
    app_info.apiVersion = __RE_VULKAN_API_VER__;

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;

    VkInstance instance = VK_NULL_HANDLE;
    const VkResult instance_create_result = vkCreateInstance(&instance_create_info, NULL, &instance);

    if (instance_create_result != VK_SUCCESS) {
        return re_false;
    }

    re_u32 __gpu_count = 0;
    const VkResult gpu_result = vkEnumeratePhysicalDevices(instance, &__gpu_count, NULL);
    vkDestroyInstance(instance, NULL);

    return gpu_result == VK_SUCCESS && __gpu_count > 0;
}

// *=================================================
// *
// * re_createVulkanBackend
// *
// *=================================================

re_GraphicsBackend re_createVulkanBackend(const re_GraphicsBackendCreateInfo* create_info) {
    re_GraphicsBackend backend = (re_GraphicsBackend)re_calloc(1, sizeof(re_GraphicsBackend_T));

    re_VulkanBackend vulkan_backend = (re_VulkanBackend)re_malloc(sizeof(re_VulkanBackend_T));
    backend->__user_data__ = vulkan_backend;

    // ? Assign API methods

    backend->destroy = re_destroyVulkanBackend;

    // ? Set to NULL for now, but may change for efficiency or debug purposes later
    vulkan_backend->allocator = NULL;

    // ? Create Vulkan backend user data

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = create_info->app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(
        create_info->app_major_version,
        create_info->app_minor_version,
        create_info->app_patch_version
    );
    app_info.pEngineName = RE_ENGINE_NAME;
    app_info.engineVersion = VK_MAKE_VERSION(
        RE_ENGINE_MAJOR_VER,
        RE_ENGINE_MINOR_VER,
        RE_ENGINE_PATCH_VER
    );
    app_info.apiVersion = __RE_VULKAN_API_VER__;

    VkInstanceCreateInfo instance_create_info = {0};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.ppEnabledExtensionNames = re_getVulkanWindowExtensions(
        &instance_create_info.enabledExtensionCount
    );

    if (__RE_LOGGER_ENABLED__) {
        instance_create_info.enabledLayerCount = __RE_VULKAN_ENABLED_INSTANCE_LAYER_COUNT__;
        instance_create_info.ppEnabledLayerNames = __RE_VULKAN_ENABLED_INSTANCE_LAYERS__;
    }

    const VkResult instance_create_result = vkCreateInstance(
        &instance_create_info,
        vulkan_backend->allocator,
        &vulkan_backend->instance
    );
    re_assert(instance_create_result == VK_SUCCESS, "Failed to create Vulkan instance!");

    vulkan_backend->surface = re_createVulkanSurface(
        create_info->window,
        vulkan_backend->instance,
        vulkan_backend->allocator
    );

    re_VkDeviceLayerCreateInfo device_layer_create_info = {0};
    device_layer_create_info.instance = vulkan_backend->instance;
    device_layer_create_info.surface = vulkan_backend->surface;
    device_layer_create_info.enabled_extensions = __RE_VULKAN_ENABLED_DEVICE_EXTENSIONS__;
    device_layer_create_info.enabled_extension_count = __RE_VULKAN_ENABLED_DEVICE_EXTENSION_COUNT__;

    device_layer_create_info.max_texture_count = 50; // TODO: These are all arbitrary and should be set according to the needs of the frontend renderer (or some better defaults)
    device_layer_create_info.max_sampler_count = 50;
    device_layer_create_info.max_storage_image_count = 50;
    device_layer_create_info.max_uniform_buffer_count = 50;
    device_layer_create_info.max_storage_buffer_count = 50;
    device_layer_create_info.max_input_attachment_count = 50;
    device_layer_create_info.max_dynamic_uniform_buffer_count = 50;

    vulkan_backend->device_layer = re_createVulkanDeviceLayer(
        &device_layer_create_info,
        vulkan_backend->allocator
    );

    return backend;
}

#pragma endregion