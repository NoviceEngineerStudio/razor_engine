#include "./re_swap_chain_layer.h"

#pragma region Private Methods

void re_generateVulkanSwapChainLayer(
    re_VkSwapChainLayer swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer device_layer,
    const VkRenderPass render_pass,
    const VkAllocationCallbacks* allocator
) {
    re_assert(create_info != NULL, "Attempting to generate Vulkan swap chain layer with NULL creation parameters!");
    re_assert(device_layer != NULL, "Attempting to generate Vulkan swap chain layer with NULL device layer!");

    VkDevice logical_device = device_layer->logical_device;
    re_VkGPU* gpu = &device_layer->gpu;

    swap_chain_layer->image_size = gpu->capabilities.currentExtent;

    if (swap_chain_layer->image_size.width == RE_MAX_U32 || swap_chain_layer->image_size.height == RE_MAX_U32) {
        swap_chain_layer->image_size.width = re_clampU32(
            swap_chain_layer->image_size.width,
            gpu->capabilities.minImageExtent.width,
            gpu->capabilities.maxImageExtent.width
        );

        swap_chain_layer->image_size.height = re_clampU32(
            swap_chain_layer->image_size.height,
            gpu->capabilities.minImageExtent.height,
            gpu->capabilities.maxImageExtent.height
        );
    }

    const re_u32 max_image_count = gpu->capabilities.maxImageCount;
    re_u32 min_image_count = re_maxU32(create_info->target_frame_count + 1, gpu->capabilities.minImageCount);

    if (max_image_count > 0) {
        min_image_count = re_minU32(min_image_count, max_image_count);
    }

    re_u32 queue_index_count = 0;
    re_u32 unique_queue_indices[RE_VK_QUEUE_ROLE_COUNT];

    for (re_u32 idx = 0; idx < RE_VK_QUEUE_ROLE_COUNT; ++idx) {
        const re_u32 queue_index = gpu->queue_indices[idx];

        re_bool is_clone = re_false;
        for (re_u32 jdx = idx + 1; jdx < RE_VK_QUEUE_ROLE_COUNT; ++jdx) {
            if (queue_index == gpu->queue_indices[jdx]) {
                is_clone = re_true;
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
    swap_chain_create_info.presentMode = create_info->present_mode;
    swap_chain_create_info.imageExtent = swap_chain_layer->image_size;
    swap_chain_create_info.preTransform = gpu->capabilities.currentTransform;
    swap_chain_create_info.queueFamilyIndexCount = queue_index_count;
    swap_chain_create_info.pQueueFamilyIndices = unique_queue_indices;
    swap_chain_create_info.oldSwapchain = swap_chain_layer->swap_chain;
    swap_chain_create_info.imageUsage = create_info->image_usage_flags;
    swap_chain_create_info.imageArrayLayers = create_info->is_vr_application ? 2 : 1;
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

    if (
        create_info->is_window_transparent &&
        gpu->capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
    ) {
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
    }

    if (gpu->queue_indices[RE_VK_QUEUE_GRAPHICS] == gpu->queue_indices[RE_VK_QUEUE_PRESENT]) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR new_swap_chain = VK_NULL_HANDLE;
    const VkResult swap_chain_create_result = vkCreateSwapchainKHR(
        logical_device,
        &swap_chain_create_info,
        allocator,
        &new_swap_chain
    );

    re_assert(swap_chain_create_result == VK_SUCCESS, "Failed to create Vulkan swap chain!");

    if (swap_chain_layer->swap_chain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(logical_device, swap_chain_layer->swap_chain, allocator);
    }

    swap_chain_layer->swap_chain = new_swap_chain;

    re_u32 image_count = 0;
    vkGetSwapchainImagesKHR(logical_device, swap_chain_layer->swap_chain, &image_count, NULL);

    re_assert(image_count > 0, "Failed to gather images from Vulkan swap chain!");

    swap_chain_layer->swap_image_count = image_count;
    swap_chain_layer->swap_images = (re_VkSwapImage*)re_malloc(sizeof(re_VkSwapImage) * image_count);
    VkImage* raw_images = (VkImage*)re_malloc(sizeof(VkImage) * image_count);
    vkGetSwapchainImagesKHR(logical_device, swap_chain_layer->swap_chain, &image_count, raw_images);

    re_assert(create_info->view_count > 0, "Attempted to allocate Vulkan image with no views!");
    swap_chain_layer->image_view_count = create_info->view_count;

    for (re_u32 idx = 0; idx < swap_chain_layer->swap_image_count; ++idx) {
        re_VkSwapImage* swap_image = &swap_chain_layer->swap_images[idx];

        swap_image->image = raw_images[idx];
        swap_image->last_used_fence = VK_NULL_HANDLE;

        swap_image->views = (VkImageView*)re_malloc(sizeof(VkImageView) * create_info->view_count);

        VkImageViewCreateInfo view_create_info = {0};
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image = swap_image->image;

        re_u32 framebuffer_layers = RE_MAX_U32;

        for (re_u32 jdx = 0; jdx < create_info->view_count; ++jdx) {
            re_VkImageViewCreateInfo* view_params = &create_info->view_create_infos[jdx];

            view_create_info.pNext = NULL;
            view_create_info.flags = view_params->flags;
            view_create_info.viewType = view_params->type;
            view_create_info.format = view_params->format;
            view_create_info.components = view_params->mapping;
            view_create_info.subresourceRange = view_params->subresource_range;

            framebuffer_layers = re_minU32(framebuffer_layers, view_create_info.subresourceRange.layerCount);

            const VkResult view_create_result = vkCreateImageView(
                logical_device,
                &view_create_info,
                allocator,
                &swap_image->views[jdx]
            );
        }

        if (framebuffer_layers == RE_MAX_U32) {
            framebuffer_layers = 1;
        }

        VkFramebufferCreateInfo framebuffer_create_info = {0};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.attachmentCount = create_info->view_count;
        framebuffer_create_info.pAttachments = swap_image->views;
        framebuffer_create_info.width = swap_chain_layer->image_size.width;
        framebuffer_create_info.height = swap_chain_layer->image_size.height;
        framebuffer_create_info.layers = framebuffer_layers;
        framebuffer_create_info.renderPass = render_pass;

        const VkResult framebuffer_create_result = vkCreateFramebuffer(
            logical_device,
            &framebuffer_create_info,
            allocator,
            &swap_image->framebuffer
        );

        re_assert(framebuffer_create_result == VK_SUCCESS, "Failed to create Vulkan image framebuffer!");
    }

    re_free(raw_images);

    const re_u32 frame_count = re_maxU32(create_info->target_frame_count, 1u);
    swap_chain_layer->frame_count = frame_count;
    swap_chain_layer->frames = (re_VkFrame*)re_malloc(sizeof(re_VkFrame) * frame_count);

    VkCommandBuffer* graphics_command_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);
    VkCommandBuffer* compute_command_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);
    VkCommandBuffer* transfer_command_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);

    VkCommandBufferAllocateInfo cmd_buffer_allocate_info;
    cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buffer_allocate_info.commandBufferCount = frame_count;

    cmd_buffer_allocate_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_GRAPHICS];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_allocate_info, graphics_command_buffers);

    cmd_buffer_allocate_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_COMPUTE];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_allocate_info, compute_command_buffers);
    
    cmd_buffer_allocate_info.commandPool = device_layer->cmd_pools[RE_VK_QUEUE_TRANSFER];
    vkAllocateCommandBuffers(logical_device, &cmd_buffer_allocate_info, transfer_command_buffers);

    for (re_u32 idx = 0; idx < frame_count; ++idx) {
        re_VkFrame* frame = &swap_chain_layer->frames[idx];

        frame->graphics_cmd_buffer = graphics_command_buffers[idx];
        frame->compute_cmd_buffer = compute_command_buffers[idx];
        frame->transfer_cmd_buffer = transfer_command_buffers[idx];

        VkSemaphoreCreateInfo sem_create_info = {0};
        sem_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        const VkResult available_create_result = vkCreateSemaphore(
            logical_device,
            &sem_create_info,
            allocator,
            &frame->available_semaphore
        );

        const VkResult finished_create_result = vkCreateSemaphore(
            logical_device,
            &sem_create_info,
            allocator,
            &frame->finished_semaphore
        );

        re_assert(
            available_create_result == VK_SUCCESS && finished_create_result == VK_SUCCESS,
            "Failed to create Vulkan swap chain frame semaphores!"
        );

        VkFenceCreateInfo sync_fence_create_info = {0};
        sync_fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        sync_fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        const VkResult sync_fence_create_result = vkCreateFence(
            logical_device,
            &sync_fence_create_info,
            allocator,
            &frame->sync_fence
        );

        re_assert(sync_fence_create_result == VK_SUCCESS, "Failed to create Vulkan swap chain frame sync fence!");
    }

    re_free(graphics_command_buffers);
    re_free(compute_command_buffers);
    re_free(transfer_command_buffers);

    swap_chain_layer->present_image_index = 0;
    swap_chain_layer->draw_image_index = 0;
}

// *=================================================
// *
// * re_clearVulkanSwapChainLayer
// *
// *=================================================

void re_clearVulkanSwapChainLayer(
    re_VkSwapChainLayer swap_chain_layer,
    const re_VkDeviceLayer device_layer,
    const VkAllocationCallbacks* allocator
) {
    re_assert(device_layer != NULL, "Attempting to clear Vulkan swap chain layer with NULL device layer!");

    const VkDevice logical_device = device_layer->logical_device;
    vkDeviceWaitIdle(logical_device);

    const re_u32 frame_count = swap_chain_layer->frame_count;
    VkCommandBuffer* graphics_cmd_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);
    VkCommandBuffer* compute_cmd_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);
    VkCommandBuffer* transfer_cmd_buffers = (VkCommandBuffer*)re_malloc(sizeof(VkCommandBuffer) * frame_count);

    for (re_u32 idx = 0; idx < frame_count; ++idx) {
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

    vkFreeCommandBuffers(logical_device, graphics_cmd_pool, 1, graphics_cmd_buffers);
    vkFreeCommandBuffers(logical_device, compute_cmd_pool, 1, compute_cmd_buffers);
    vkFreeCommandBuffers(logical_device, transfer_cmd_pool, 1, transfer_cmd_buffers);

    re_free(graphics_cmd_buffers);
    re_free(compute_cmd_buffers);
    re_free(transfer_cmd_buffers);

    re_free(swap_chain_layer->frames);
    swap_chain_layer->frame_count = 0;
    swap_chain_layer->frames = NULL;

    for (re_u32 idx = 0; idx < swap_chain_layer->swap_image_count; ++idx) {
        re_VkSwapImage* swap_image = &swap_chain_layer->swap_images[idx];

        for (re_u32 jdx = 0; jdx < swap_chain_layer->image_view_count; ++jdx) {
            vkDestroyImageView(logical_device, swap_image->views[jdx], allocator);
        }

        re_free(swap_image->views);

        vkDestroyFramebuffer(logical_device, swap_image->framebuffer, allocator);
    }

    re_free(swap_chain_layer->swap_images);
    swap_chain_layer->image_view_count = 0;
    swap_chain_layer->swap_image_count = 0;
    swap_chain_layer->swap_images = NULL;

    swap_chain_layer->image_size.width = 0;
    swap_chain_layer->image_size.height = 0;

    swap_chain_layer->present_image_index = 0;
    swap_chain_layer->draw_image_index = 0;
}

#pragma endregion
#pragma region Public Methods

// *=================================================
// *
// * re_createVulkanSwapChainLayer
// *
// *=================================================

re_VkSwapChainLayer re_createVulkanSwapChainLayer(
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer device_layer,
    const VkRenderPass render_pass,
    const VkAllocationCallbacks* allocator
) {
    re_VkSwapChainLayer swap_chain_layer = (re_VkSwapChainLayer)re_malloc(sizeof(re_VkSwapChainLayer_T));

    re_generateVulkanSwapChainLayer(
        swap_chain_layer,
        create_info,
        device_layer,
        render_pass,
        allocator
    );

    return swap_chain_layer;
}

// *=================================================
// *
// * re_destroyVulkanSwapChainLayer
// *
// *=================================================

void re_destroyVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkDeviceLayer device_layer,
    const VkAllocationCallbacks* allocator
) {
    re_assert(swap_chain_layer != NULL, "Attempting to destroy a NULL Vulkan swap chain layer!");

    re_clearVulkanSwapChainLayer(*swap_chain_layer, device_layer, allocator);

    vkDestroySwapchainKHR(device_layer->logical_device, (*swap_chain_layer)->swap_chain, allocator);

    re_free(*swap_chain_layer);
    *swap_chain_layer = NULL;
}

// *=================================================
// *
// * re_regenerateVulkanSwapChainLayer
// *
// *=================================================

void re_regenerateVulkanSwapChainLayer(
    re_VkSwapChainLayer* swap_chain_layer,
    const re_VkSwapChainLayerCreateInfo* create_info,
    const re_VkDeviceLayer device_layer,
    const VkRenderPass render_pass,
    const VkAllocationCallbacks* allocator
) {
    re_assert(swap_chain_layer != NULL, "Attempting to regenerate a NULL Vulkan swap chain layer!");

    re_clearVulkanSwapChainLayer(*swap_chain_layer, device_layer, allocator);

    re_generateVulkanSwapChainLayer(
        *swap_chain_layer,
        create_info,
        device_layer,
        render_pass,
        allocator
    );
}

#pragma endregion