#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_VULKAN_SWAP_CHAIN_LAYER_HEADER_FILE
#define __RAZOR_VULKAN_SWAP_CHAIN_LAYER_HEADER_FILE

#include <re_core.h>
#include <vulkan/vulkan.h>
#include "./re_vulkan_device_layer.h"

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
    VkRenderPass render_pass;

    VkSurfaceFormatKHR color_format;
    VkPresentModeKHR present_mode;

    re_VkImageViewCreateInfo* view_create_infos;
    uint32_t view_count;

    VkImageUsageFlags image_usage_flags;
    uint32_t target_frame_count;
    uint32_t view_layer_count;
    
    bool is_window_transparent;
} re_VkSwapChainLayerCreateInfo;

typedef struct re_VkSwapChainLayer {
    VkSwapchainKHR swap_chain;

    VkExtent2D image_size;
    uint32_t image_view_count;

    re_VkFrame* frames;
    uint32_t frame_count;

    re_VkSwapImage* swap_images;
    uint32_t swap_image_count;

    uint32_t present_image_index;
    uint32_t draw_image_index;
} re_VkSwapChainLayer;

/// @brief Creates a new Vulkan swap chain layer.
/// @param swap_chain_layer The swap chain's memory pointer.
/// @param create_info The swap chain's creation parameters.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
void __re_createVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
);

/// @brief Destroy a Vulkan swap chain layer.
/// @param swap_chain_layer The Vulkan swap chain layer to destroy.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
void __re_destroyVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator,
    const bool clear_swap_chain
);

/// @brief Regenerates the internal values of a Vulkan swap chain layer.
/// @param swap_chain_layer The Vulkan swap chain layer to regenerate.
/// @param create_info The swap chain's updated creation parameters.
/// @param device_layer The associated Vulkan device layer.
/// @param allocator The Vulkan allocation callbacks.
void __re_regenerateVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
);

#endif

#endif