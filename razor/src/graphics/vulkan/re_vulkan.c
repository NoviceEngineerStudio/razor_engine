#ifdef RE_VULKAN_AVAILABLE

#include <re_debug.h>
#include <re_utils.h>
#include "./re_vulkan.h"
#include "../../re_internals.h"
#include "./re_vulkan_device_layer.h"
#include "../../core/re_vulkan_window.h"
#include "./re_vulkan_swap_chain_layer.h"

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

#define __RE_VULKAN_COLOR_PRIORITY_B8G8R8A8_SRGB 0u
#define __RE_VULKAN_COLOR_PRIORITY_R8G8B8_SRGB 1u
#define __RE_VULKAN_COLOR_PRIORITY_B8G8R8_SRGB 2u

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
// * __re_selectVulkanColorFormat
// *
// *=================================================

VkSurfaceFormatKHR __re_selectVulkanColorFormat(
    const VkSurfaceFormatKHR* formats,
    const uint32_t format_count
) {
    uint32_t fallback_priority = UINT32_MAX;
    VkSurfaceFormatKHR fallback_format = formats[0];
    for (uint32_t idx = 0; idx < format_count; ++idx) {
        const VkSurfaceFormatKHR format = formats[idx];

        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            switch (format.format) {
                case VK_FORMAT_R8G8B8A8_SRGB: {
                    return format;
                }

                case VK_FORMAT_B8G8R8A8_SRGB: {
                    if (fallback_priority < __RE_VULKAN_COLOR_PRIORITY_B8G8R8A8_SRGB) {
                        continue;
                    }

                    fallback_priority = __RE_VULKAN_COLOR_PRIORITY_B8G8R8A8_SRGB;
                    fallback_format = format;
                }

                case VK_FORMAT_R8G8B8_SRGB: {
                    if (fallback_priority < __RE_VULKAN_COLOR_PRIORITY_R8G8B8_SRGB) {
                        continue;
                    }

                    fallback_priority = __RE_VULKAN_COLOR_PRIORITY_R8G8B8_SRGB;
                    fallback_format = format;
                }

                case VK_FORMAT_B8G8R8_SRGB: {
                    if (fallback_priority < __RE_VULKAN_COLOR_PRIORITY_B8G8R8_SRGB) {
                        continue;
                    }

                    fallback_priority = __RE_VULKAN_COLOR_PRIORITY_B8G8R8_SRGB;
                    fallback_format = format;
                }
            }
        }
    }

    return fallback_format;
}

// *=================================================
// *
// * __re_selectVulkanDepthFormat
// *
// *=================================================

VkSurfaceFormatKHR __re_selectVulkanDepthFormat(
    const VkSurfaceFormatKHR* formats,
    const uint32_t format_count
) {
    uint32_t fallback_priority = UINT32_MAX;
    VkSurfaceFormatKHR fallback_format = formats[0];
    for (uint32_t idx = 0; idx < format_count; ++idx) {
        const VkSurfaceFormatKHR format = formats[idx];

        // TODO: Select format using priority selection
    }

    return fallback_format;
}

// *=================================================
// *
// * __re_selectVulkanPresentMode
// *
// *=================================================

VkPresentModeKHR __re_selectVulkanPresentMode(
    const VkPresentModeKHR* present_modes,
    const uint32_t present_mode_count,
    const bool vsync_enabled
) {
    if (vsync_enabled) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkPresentModeKHR fallback_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t idx = 0; idx < present_mode_count; ++idx) {
        switch (present_modes[idx]) {
            case VK_PRESENT_MODE_MAILBOX_KHR: {
                return VK_PRESENT_MODE_MAILBOX_KHR;
            }

            case VK_PRESENT_MODE_IMMEDIATE_KHR: {
                fallback_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    return fallback_mode;
}

// *=================================================
// *
// * __re_createVulkanRenderPass
// *
// *=================================================

VkRenderPass __re_createVulkanRenderPass(
    const re_RenderPassDescription* render_pass_description,
    const VkDevice logical_device,
    const VkSurfaceFormatKHR color_format,
    const VkSurfaceFormatKHR depth_format,
    const VkAllocationCallbacks* allocator
) {
    const uint32_t attachment_count = render_pass_description->attachment_count;
    const uint32_t subpass_count = render_pass_description->subpass_count;
    const uint32_t dependency_count = render_pass_description->dependency_count;

    re_assert(attachment_count <= RE_MAX_RENDER_PASS_ATTACHMENTS, "Too many attachments provided to Vulkan render pass: %d", attachment_count);
    re_assert(subpass_count <= RE_MAX_RENDER_PASS_SUBPASSES, "Too many subpasses provided to Vulkan render pass: %d", subpass_count);
    re_assert(dependency_count <= RE_MAX_RENDER_PASS_SUBPASSES, "Too many dependencies provided to Vulkan render pass: %d", dependency_count);

    VkAttachmentDescription vk_attachments[RE_MAX_RENDER_PASS_ATTACHMENTS] = {0};
    VkSubpassDescription vk_subpasses[RE_MAX_RENDER_PASS_SUBPASSES] = {0};
    VkSubpassDependency vk_dependencies[RE_MAX_RENDER_PASS_DEPENDENCIES] = {0};

    for (uint32_t idx = 0; idx < attachment_count; ++idx) {
        const re_AttachmentDescription* attachment_desc = &render_pass_description->attachments[idx];
        VkAttachmentDescription* vk_attachment = &vk_attachments[idx];

        attachment_desc->type;
        attachment_desc->is_presentable;

        switch (attachment_desc->load_op) {
            case RE_LOAD_OP_CLEAR: { vk_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; break; }
            case RE_LOAD_OP_LOAD: { vk_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; break; }
            case RE_LOAD_OP_DONT_CARE: { vk_attachment->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; break; }
        }

        switch (attachment_desc->store_op) {
            case RE_STORE_OP_STORE: { vk_attachment->storeOp = VK_ATTACHMENT_STORE_OP_STORE; break; }
            case RE_STORE_OP_DONT_CARE: { vk_attachment->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; break; }
        }

        switch (attachment_desc->samples) {
            case RE_SAMPLE_COUNT_1_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_1_BIT; break; }
            case RE_SAMPLE_COUNT_2_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_2_BIT; break; }
            case RE_SAMPLE_COUNT_4_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_4_BIT; break; }
            case RE_SAMPLE_COUNT_8_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_8_BIT; break; }
            case RE_SAMPLE_COUNT_16_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_16_BIT; break; }
            case RE_SAMPLE_COUNT_32_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_32_BIT; break; }
            case RE_SAMPLE_COUNT_64_BIT: { vk_attachment->samples = VK_SAMPLE_COUNT_64_BIT; break; }
        }

        if (attachment_desc->uses_stencil) {
            switch (attachment_desc->stencil_load_op) {
                case RE_LOAD_OP_CLEAR: { vk_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; break; }
                case RE_LOAD_OP_LOAD: { vk_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD; break; }
                case RE_LOAD_OP_DONT_CARE: { vk_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; break; }
            }

            switch (attachment_desc->stencil_store_op) {
                case RE_STORE_OP_STORE: { vk_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE; break; }
                case RE_STORE_OP_DONT_CARE: { vk_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; break; }
            }
        }
        else {
            vk_attachment->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_NONE;
            vk_attachment->stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
        }

        vk_attachment->format = ;
        vk_attachment->initialLayout = ;
        vk_attachment->finalLayout = ;
    }

    for (uint32_t idx = 0; idx < subpass_count; ++idx) {
        const re_SubpassDescription* subpass_desc = &render_pass_description->subpasses[idx];
        VkSubpassDescription* vk_attachment = &vk_subpasses[idx];

        // TODO: Fill Out
    }

    for (uint32_t idx = 0; idx < dependency_count; ++idx) {
        const re_DependencyDescription* dependency_desc = &render_pass_description->dependencies[idx];
        VkSubpassDependency* vk_attachment = &vk_dependencies[idx];

        // TODO: Fill Out
    }

    VkRenderPassCreateInfo render_pass_create_info = {0};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = attachment_count;
    render_pass_create_info.pAttachments = vk_attachments;
    render_pass_create_info.subpassCount = subpass_count;
    render_pass_create_info.pSubpasses = vk_subpasses;
    render_pass_create_info.dependencyCount = dependency_count;
    render_pass_create_info.pDependencies = vk_dependencies;


    VkRenderPass render_pass = VK_NULL_HANDLE;
    const VkResult render_pass_create_result = vkCreateRenderPass(
        logical_device,
        &render_pass_create_info,
        allocator,
        &render_pass
    );

    re_assert(render_pass_create_result == VK_SUCCESS, "Failed to create Vulkan rendering pass!");

    return render_pass;
}

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
    VkAllocationCallbacks* allocator = VK_NULL_HANDLE;
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
            re_assert(false, "Vulkan does not recognize renderer profile with value: %d", create_info->profile);
            break;
        }
    }

    device_layer_create_info.max_texture_count = __RE_VULKAN_TEXTURE_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_sampler_count = __RE_VULKAN_SAMPLER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_storage_image_count = __RE_VULKAN_STORAGE_IMAGE_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_uniform_buffer_count = __RE_VULKAN_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_storage_buffer_count = __RE_VULKAN_STORAGE_BUFFER_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_input_attachment_count = __RE_VULKAN_INPUT_ATTACHMENT_COUNT_BASE * descriptor_multiplier;
    device_layer_create_info.max_dynamic_uniform_buffer_count = __RE_VULKAN_DYNAMIC_UNIFORM_BUFFER_COUNT_BASE * descriptor_multiplier;

    __re_createVulkanDeviceLayer(
        &context->device_layer,
        &device_layer_create_info,
        allocator
    );

    const VkSurfaceFormatKHR color_format = __re_selectVulkanColorFormat(
        context->device_layer.gpu.formats,
        context->device_layer.gpu.format_count
    );
    const VkSurfaceFormatKHR depth_format = __re_selectVulkanDepthFormat(
        context->device_layer.gpu.formats,
        context->device_layer.gpu.format_count
    );

    // TODO: Refactor All Code Beyond This Point (Render Graphs, Add Pipeline, Ensure Swap Chain Consistency)

    const VkRenderPass render_pass = __re_createVulkanRenderPass(
        &create_info->render_pass_description,
        context->device_layer.logical_device,
        color_format,
        depth_format,
        allocator
    );
    context->render_pass = render_pass;

    re_VkSwapChainLayerCreateInfo swap_chain_layer_create_info = {0};
    swap_chain_layer_create_info.window_size = __re_getVulkanExtent(create_info->window);
    swap_chain_layer_create_info.surface = surface;
    swap_chain_layer_create_info.render_pass = render_pass;
    swap_chain_layer_create_info.color_format = color_format;
    swap_chain_layer_create_info.present_mode = __re_selectVulkanPresentMode(
        context->device_layer.gpu.present_modes,
        context->device_layer.gpu.present_mode_count,
        create_info->vsync_enabled
    );
    // TODO: Assign values here!
    // swap_chain_layer_create_info.view_create_infos = ;
    // swap_chain_layer_create_info.view_count = ;
    // swap_chain_layer_create_info.image_usage_flags = ;
    // swap_chain_layer_create_info.target_frame_count = ;
    // swap_chain_layer_create_info.view_layer_count = ;
    // swap_chain_layer_create_info.is_window_transparent = ;

    __re_createVulkanSwapChainLayer(
        &context->swap_chain_layer,
        &swap_chain_layer_create_info,
        &context->device_layer,
        allocator
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

    VkAllocationCallbacks* allocator = context_data->allocator;

    __re_destroyVulkanSwapChainLayer(
        &context_data->swap_chain_layer,
        &context_data->device_layer,
        allocator,
        true
    );

    vkDestroyRenderPass(
        context_data->device_layer.logical_device,
        context_data->render_pass,
        allocator
    );

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