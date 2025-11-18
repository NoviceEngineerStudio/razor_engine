#ifndef __RE_SRC_GRAPHICS_VULKAN_SWAP_CHAIN_LAYER__
#define __RE_SRC_GRAPHICS_VULKAN_SWAP_CHAIN_LAYER__

#include <razor.h>
#include <vulkan/vulkan.h>
#include "./re_device_layer.h"

typedef struct re_VkFrame {
    VkCommandBuffer graphics_cmd_buffer;
    VkCommandBuffer compute_cmd_buffer;
    VkCommandBuffer transfer_cmd_buffer;

    VkSemaphore available_semaphore;
    VkSemaphore finished_semaphore;
    VkFence sync_fence;
} re_VkFrame;

typedef struct re_VkSwapImage {
    VkImage image;

    VkFramebuffer framebuffer;
    VkFence last_used_fence;

    VkImageView* views;
} re_VkSwapImage;

typedef struct re_VkImageViewCreateInfo {
    VkImageViewCreateFlags flags;
    VkImageViewType type;
    VkFormat format;
    VkComponentMapping mapping;
    VkImageSubresourceRange subresource_range;
} re_VkImageViewCreateInfo;

typedef struct re_VkSwapChainLayerCreateInfo {
    VkExtent2D window_size;

    VkSurfaceKHR surface;
    VkSurfaceFormatKHR color_format;
    VkPresentModeKHR present_mode;

    re_VkImageViewCreateInfo* view_create_infos;
    re_u32 view_count;

    VkImageUsageFlags image_usage_flags;
    re_u32 target_frame_count;
    re_bool is_vr_application;
    re_bool is_window_transparent;
} re_VkSwapChainLayerCreateInfo;

typedef struct re_VkSwapChainLayer_T {
    VkSwapchainKHR swap_chain;

    VkExtent2D image_size;
    re_u32 image_view_count;

    re_VkFrame* frames;
    re_u32 frame_count;

    re_VkSwapImage* swap_images;
    re_u32 swap_image_count;

    re_u32 present_image_index;
    re_u32 draw_image_index;
} re_VkSwapChainLayer_T;

typedef re_VkSwapChainLayer_T* re_VkSwapChainLayer;

/// @brief Creates a new Vulkan swap chain layer.
/// @param create_info The swap chain's creation parameters.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
/// @return A new Vulkan swap chain layer.
re_VkSwapChainLayer re_createVulkanSwapChainLayer(
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer device_layer,
    const VkRenderPass render_pass,
    const VkAllocationCallbacks* allocator
);

/// @brief Destroy a Vulkan swap chain layer.
/// @param swap_chain_layer The Vulkan swap chain layer to destroy.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
void re_destroyVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkDeviceLayer device_layer,
    const VkAllocationCallbacks* allocator
);

/// @brief Regenerates the internal values of a Vulkan swap chain layer.
/// @param swap_chain_layer The Vulkan swap chain layer to regenerate.
/// @param create_info The swap chain's updated creation parameters.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
void re_regenerateVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer device_layer,
    const VkRenderPass render_pass,
    const VkAllocationCallbacks* allocator
);

#endif