#ifdef RE_VULKAN_AVAILABLE

#include "./re_vulkan_swap_chain_layer.h"

#include <re_debug.h>

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
    re_assert(swap_chain_layer != RE_NULL_HANDLE, "Attempting to create Vulkan swap chain layer with NULL memory!");

    const VkDevice logical_device = device_layer->logical_device;
    const re_VkGPU* gpu = &device_layer->gpu;

    if (
        create_info->window_size.width == UINT32_MAX ||
        create_info->window_size.height == UINT32_MAX
    ) {
        swap_chain_layer->image_size.width = max(
            gpu->capabilities.minImageExtent.width,
            min(
                gpu->capabilities.maxImageExtent.width,
                create_info->window_size.width
            )
        );

        swap_chain_layer->image_size.height = max(
            gpu->capabilities.minImageExtent.height,
            min(
                gpu->capabilities.maxImageExtent.height,
                create_info->window_size.height
            )
        );
    }
    else {
        swap_chain_layer->image_size = create_info->window_size;
    }

    const uint32_t max_image_count = gpu->capabilities.maxImageCount;
    uint32_t min_image_count = max(create_info->target_frame_count, gpu->capabilities.minImageCount);

    if (max_image_count > 0) {
        min_image_count = min(min_image_count, max_image_count);
    }

    uint32_t queue_index_count = 0;
    uint32_t unique_queue_indices[RE_VK_QUEUE_ROLE_COUNT];

    for (uint32_t idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const uint32_t queue_index = gpu->queue_indices[idx];

        bool is_clone = false;
        for (uint32_t jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (queue_index == gpu->queue_indices[jdx]) {
                is_clone = true;
                break;
            }
        }

        if (is_clone) {
            continue;
        }

        unique_queue_indices[queue_index_count++] = queue_index;
    }

    VkSwapchainCreateInfoKHR swap_chain_create_info = {0};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = create_info->surface;
    swap_chain_create_info.minImageCount = min_image_count;
    swap_chain_create_info.imageFormat = create_info->color_format.format;
    swap_chain_create_info.imageColorSpace = create_info->color_format.colorSpace;
    swap_chain_create_info.imageExtent = swap_chain_layer->image_size;
    swap_chain_create_info.imageArrayLayers = create_info->is_vr_application ? 2 : 1;
    swap_chain_create_info.imageUsage = create_info->image_usage_flags;
    swap_chain_create_info.queueFamilyIndexCount = queue_index_count;
    swap_chain_create_info.pQueueFamilyIndices = unique_queue_indices;
    swap_chain_create_info.preTransform = gpu->capabilities.currentTransform;
    swap_chain_create_info.presentMode = create_info->present_mode;
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = swap_chain_layer->swap_chain;

    if (
        create_info->is_window_transparent &&
        gpu->capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    ) {
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }
    else {
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    if (gpu->queue_indices[RE_VK_QUEUE_GRAPHICS] == gpu->queue_indices[RE_VK_QUEUE_PRESENT]) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }

    VkSwapchainKHR swap_chain = VK_NULL_HANDLE;
    const VkResult swap_chain_create_result = vkCreateSwapchainKHR(
        logical_device,
        &swap_chain_create_info,
        allocator,
        &swap_chain
    );

    re_assert(swap_chain_create_result == VK_SUCCESS, "Failed to create Vulkan swap chain!");

    if (swap_chain_layer->swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(logical_device, swap_chain_layer->swap_chain, allocator);
    }

    swap_chain_layer->swap_chain = swap_chain;

    const uint32_t image_view_count = create_info->view_count;
    swap_chain_layer->image_view_count = image_view_count;
    re_assert(image_view_count > 0, "Attempting to create Vulkan swap images with no views!");

    uint32_t framebuffer_layers = UINT32_MAX;
    for (uint32_t idx = 0; idx < image_view_count; ++idx) {
        framebuffer_layers = min(
            framebuffer_layers,
            create_info->view_create_infos[idx].subresource_range.layerCount
        );
    }

    if (framebuffer_layers == UINT32_MAX) {
        framebuffer_layers = 1;
    }

    uint32_t __image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &__image_count, VK_NULL_HANDLE);

    re_assert(__image_count > 0, "Failed to gather images from Vulkan swap chain!");

    const uint32_t image_count = __image_count;
    swap_chain_layer->swap_image_count = image_count;

    re_VkSwapImage* swap_images = (re_VkSwapImage*)re_malloc(sizeof(re_VkSwapImage) * image_count);
    swap_chain_layer->swap_images = swap_images;

    VkImage* raw_images = (VkImage*)re_malloc(sizeof(VkImage) * image_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain, &__image_count, raw_images);

    for (uint32_t idx = 0; idx < image_count; ++idx) {
        re_VkSwapImage* swap_image = &swap_images[idx];

        swap_image->image = raw_images[idx];
        swap_image->last_used_fence = VK_NULL_HANDLE;

        swap_image->views = (VkImageView*)re_malloc(sizeof(VkImageView) * image_view_count);

        VkImageViewCreateInfo view_create_info = {0};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = swap_image->image;

        for (uint32_t jdx = 0; jdx < image_view_count; ++jdx) {
            re_VkImageViewCreateInfo* view_params = &create_info->view_create_infos[jdx];

            view_create_info.pNext = VK_NULL_HANDLE;
            view_create_info.flags = view_params->flags;
            view_create_info.viewType = view_params->type;
            view_create_info.format = view_params->format;
            view_create_info.components = view_params->mapping;
            view_create_info.subresourceRange = view_params->subresource_range;

            const VkResult view_create_result = vkCreateImageView(
                logical_device,
                &view_create_info,
                allocator,
                &swap_image->views[jdx]
            );

            re_assert(view_create_result == VK_SUCCESS, "Failed to create Vulkan swap image view!");
        }

        VkFramebufferCreateInfo framebuffer_create_info = {0};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = create_info->render_pass;
        framebuffer_create_info.attachmentCount = image_view_count;
        framebuffer_create_info.pAttachments = swap_image->views;
        framebuffer_create_info.width = swap_chain_layer->image_size.width;
        framebuffer_create_info.height = swap_chain_layer->image_size.height;
        framebuffer_create_info.layers = framebuffer_layers;

        const VkResult framebuffer_create_result = vkCreateFramebuffer(
            logical_device,
            &framebuffer_create_info,
            allocator,
            &swap_image->framebuffer
        );

        re_assert(framebuffer_create_result == VK_SUCCESS, "Failed to create Vulkan swap image framebuffer!");
    }

    re_free(raw_images);

    const uint32_t frame_count = max(1u, min(create_info->target_frame_count, image_count));
    swap_chain_layer->frame_count = frame_count;

    re_VkFrame* frames = (re_VkFrame*)re_malloc(sizeof(re_VkFrame) * frame_count);
    swap_chain_layer->frames = frames;

    const size_t cmd_buffer_list_size = sizeof(VkCommandBuffer) * frame_count;
    VkCommandBuffer* graphics_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);
    VkCommandBuffer* compute_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);
    VkCommandBuffer* transfer_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);

    VkCommandBufferAllocateInfo cmd_buffer_alloc_info = {0};
    cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_alloc_info.commandBufferCount = frame_count;

    cmd_buffer_alloc_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_GRAPHICS];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_alloc_info, graphics_cmd_buffers);

    cmd_buffer_alloc_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_COMPUTE];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_alloc_info, compute_cmd_buffers);

    cmd_buffer_alloc_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_TRANSFER];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_alloc_info, transfer_cmd_buffers);

    for (uint32_t idx = 0; idx < frame_count; ++idx) {
        re_VkFrame* frame = &frames[idx];

        frame->graphics_cmd_buffer = graphics_cmd_buffers[idx];
        frame->compute_cmd_buffer = compute_cmd_buffers[idx];
        frame->graphics_cmd_buffer = transfer_cmd_buffers[idx];

        VkSemaphoreCreateInfo sem_create_info = {0};
        sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        const VkResult avail_sem_create_result = vkCreateSemaphore(
            logical_device,
            &sem_create_info,
            allocator,
            &frame->available_semaphore
        );

        const VkResult fin_sem_create_result = vkCreateSemaphore(
            logical_device,
            &sem_create_info,
            allocator,
            &frame->finished_semaphore
        );

        re_assert(
            avail_sem_create_result == VK_SUCCESS && fin_sem_create_result == VK_SUCCESS,
            "Failed to create Vulkan frame semaphores!"
        );

        VkFenceCreateInfo fence_create_info = {0};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        const VkResult fence_create_result = vkCreateFence(
            logical_device,
            &fence_create_info,
            allocator,
            &frame->sync_fence
        );

        re_assert(fence_create_result == VK_SUCCESS, "Failed to create Vulkan frame fence!");
    }

    re_free(graphics_cmd_buffers);
    re_free(compute_cmd_buffers);
    re_free(transfer_cmd_buffers);

    swap_chain_layer->present_image_index = 0;
    swap_chain_layer->draw_image_index = 0;
}

// *=================================================
// *
// * __re_destroyVulkanSwapChainLayer
// *
// *=================================================

void __re_destroyVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkDeviceLayer* device_layer,
    const VkAllocationCallbacks* allocator,
    const bool clear_swap_chain
) {
    re_assert(swap_chain_layer != RE_NULL_HANDLE, "Attempting to destroy NULL Vulkan swap chain layer!");

    const VkDevice logical_device = device_layer->logical_device;
    vkDeviceWaitIdle(logical_device);

    const uint32_t frame_count = swap_chain_layer->frame_count;
    const size_t cmd_buffer_list_size = sizeof(VkCommandBuffer) * frame_count;
    
    VkCommandBuffer* graphics_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);
    VkCommandBuffer* compute_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);
    VkCommandBuffer* transfer_cmd_buffers = (VkCommandBuffer*)re_malloc(cmd_buffer_list_size);

    for (uint32_t idx = 0; idx < frame_count; ++idx) {
        re_VkFrame* frame = &swap_chain_layer->frames[idx];

        graphics_cmd_buffers[idx] = frame->graphics_cmd_buffer;
        compute_cmd_buffers[idx] = frame->compute_cmd_buffer;
        transfer_cmd_buffers[idx] = frame->transfer_cmd_buffer;

        vkDestroySemaphore(logical_device, frame->available_semaphore, allocator);
        vkDestroySemaphore(logical_device, frame->finished_semaphore, allocator);
        vkDestroyFence(logical_device, frame->sync_fence, allocator);
    }

    VkCommandPool graphics_cmd_pool = device_layer->cmd_pools[RE_VK_QUEUE_GRAPHICS];
    VkCommandPool compute_cmd_pool = device_layer->cmd_pools[RE_VK_QUEUE_COMPUTE];
    VkCommandPool transfer_cmd_pool = device_layer->cmd_pools[RE_VK_QUEUE_TRANSFER];

    vkFreeCommandBuffers(logical_device, graphics_cmd_pool, frame_count, graphics_cmd_buffers);
    vkFreeCommandBuffers(logical_device, compute_cmd_pool, frame_count, compute_cmd_buffers);
    vkFreeCommandBuffers(logical_device, transfer_cmd_pool, frame_count, transfer_cmd_buffers);

    re_free(graphics_cmd_buffers);
    re_free(compute_cmd_buffers);
    re_free(transfer_cmd_buffers);

    re_free(swap_chain_layer->frames);
    swap_chain_layer->frame_count = 0;
    swap_chain_layer->frames = RE_NULL_HANDLE;

    const uint32_t image_count = swap_chain_layer->swap_image_count;
    swap_chain_layer->swap_image_count = 0;

    const uint32_t image_view_count = swap_chain_layer->image_view_count;
    swap_chain_layer->image_view_count = 0;

    for (uint32_t idx = 0; idx < image_count; ++idx) {
        re_VkSwapImage* swap_image = &swap_chain_layer->swap_images[idx];

        for (uint32_t jdx = 0; jdx < image_view_count; ++jdx) {
            vkDestroyImageView(logical_device, swap_image->views[jdx], allocator);
        }

        re_free(swap_image->views);

        vkDestroyFramebuffer(logical_device, swap_image->framebuffer, allocator);
    }

    re_free(swap_chain_layer->swap_images);
    swap_chain_layer->swap_images = RE_NULL_HANDLE;

    swap_chain_layer->image_size.width = 0;
    swap_chain_layer->image_size.height = 0;

    swap_chain_layer->present_image_index = 0;
    swap_chain_layer->draw_image_index = 0;

    if (clear_swap_chain) {
        vkDestroySwapchainKHR(logical_device, swap_chain_layer->swap_chain, allocator);
        swap_chain_layer->swap_chain = VK_NULL_HANDLE;
    }
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
    __re_destroyVulkanSwapChainLayer(swap_chain_layer, device_layer, allocator, false);
    __re_createVulkanSwapChainLayer(swap_chain_layer, create_info, device_layer, allocator);
}

#endif