#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_GRAPHICS_VULKAN_HEADER_FILE
#define __RAZOR_GRAPHICS_VULKAN_HEADER_FILE

#define __RE_VULKAN_API_VER VK_API_VERSION_1_3

#include <re_core.h>
#include <re_graphics.h>
#include <vulkan/vulkan.h>

typedef struct re_VkContext_T re_VkContext_T;
typedef re_VkContext_T* re_VkContext;

/// @brief Determine if Vulkan is available on this device.
/// @return A flag indicating if Vulkan is available on this device.
bool __re_vulkanAvailable();

/// @brief Creates a new Vulkan backend context.
/// @param create_info The Vulkan context's creation parameters.
/// @return A new Vulkan backend context.
re_VkContext __re_createVulkanContext(const re_GraphicsInstanceCreateInfo* create_info);

/// @brief Destroy a Vulkan backend context.
/// @param context The Vulkan context to be destroyed.
void __re_destroyVulkanContext(re_VkContext* context);

#endif

#endif