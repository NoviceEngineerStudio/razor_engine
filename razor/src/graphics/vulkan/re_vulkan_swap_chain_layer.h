#ifdef RE_VULKAN_AVAILABLE

#ifndef __RAZOR_VULKAN_SWAP_CHAIN_LAYER_HEADER_FILE
#define __RAZOR_VULKAN_SWAP_CHAIN_LAYER_HEADER_FILE

#include <vulkan/vulkan.h>

typedef struct re_VkSwapChainLayer {
    VkSwapchainKHR swap_chain;
} re_VkSwapChainLayer;

#endif

#endif