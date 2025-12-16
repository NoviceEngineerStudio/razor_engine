#ifdef RE_VULKAN_AVAILABLE

#include "./re_vulkan_swap_chain_layer.h"

// *=================================================
// *
// * __re_createVulkanSwapChainLayer
// *
// *=================================================

void __re_createVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
) {
    // TODO:
}

// *=================================================
// *
// * __re_destroyVulkanSwapChainLayer
// *
// *=================================================

void __re_destroyVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
) {
    // TODO:
}

// *=================================================
// *
// * __re_regenerateVulkanSwapChainLayer
// *
// *=================================================

void __re_regenerateVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator
) {
    __re_destroyVulkanSwapChainLayer(swap_chain_layer, device_layer, allocator);
    __re_createVulkanSwapChainLayer(swap_chain_layer, create_info, device_layer, allocator);
}

#endif