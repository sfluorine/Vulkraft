#include <fmt/format.h>

#include <span>
#include <string>
#include <vector>

#include "renderer_subsystem.h"
#include "vulkan_helper.h"

namespace {

std::string flatten_vkb_detailed_error(std::vector<std::string> const& errors)
{
    std::string full_error;
    for (auto i { 0 }; i < errors.size(); i++) {
        auto const& error = errors[i];
        if (i + 1 == errors.size()) {
            full_error.append(fmt::format("{}", error));
        } else {
            full_error.append(fmt::format("{}\n", error));
        }
    }
    return full_error;
}

}

RenderingInstance::RenderingInstance(RenderingInstanceInfo const& info)
    : m_info(info)
    , m_success(true)
{
}

void RenderingInstance::begin(float r, float g, float b, float a)
{
    begin_recording();
    transtition_image(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    begin_rendering(r, g, b, a);
}

void RenderingInstance::end()
{
    end_rendering();
    transtition_image(
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    end_recording();
}

void RenderingInstance::submit_and_present()
{
    submit_cmd_buffer();
    present_surface();
}

void RenderingInstance::bind_graphics_pipeline(VkPipeline graphics_pipeline)
{
    vkCmdBindPipeline(m_info.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
}

void RenderingInstance::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
    vkCmdDraw(m_info.cmd_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void RenderingInstance::begin_recording()
{
    VkCommandBufferBeginInfo cmd_buffer_begin_info {};
    cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(m_info.cmd_buffer, &cmd_buffer_begin_info));
}

void RenderingInstance::end_recording()
{
    VK_CHECK(vkEndCommandBuffer(m_info.cmd_buffer));
}

void RenderingInstance::begin_rendering(float r, float g, float b, float a)
{
    VkRenderingAttachmentInfo color_attachment {};
    color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color_attachment.imageView = m_info.image_view;
    color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color_attachment.resolveMode = VK_RESOLVE_MODE_NONE;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.clearValue = { .color = { .float32 = { r, g, b, a } } };

    VkRenderingInfo rendering_info {};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    rendering_info.renderArea = { { 0, 0 }, m_info.swapchain_extent };
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &color_attachment;
    vkCmdBeginRendering(m_info.cmd_buffer, &rendering_info);
}

void RenderingInstance::end_rendering()
{
    vkCmdEndRendering(m_info.cmd_buffer);
}

void RenderingInstance::transtition_image(
    VkPipelineStageFlags2 src_stage,
    VkAccessFlags2 src_access,
    VkPipelineStageFlags2 dst_stage,
    VkAccessFlags2 dst_access,
    VkImageLayout old_layout,
    VkImageLayout new_layout)
{
    VkImageMemoryBarrier2 image_barrier {};
    image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    image_barrier.srcStageMask = src_stage;
    image_barrier.srcAccessMask = src_access;
    image_barrier.dstStageMask = dst_stage;
    image_barrier.dstAccessMask = dst_access;
    image_barrier.oldLayout = old_layout;
    image_barrier.newLayout = new_layout;
    image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_barrier.image = m_info.image;
    image_barrier.subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,
        0, 1, 0, 1
    };

    VkDependencyInfo dep_info {};
    dep_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep_info.imageMemoryBarrierCount = 1;
    dep_info.pImageMemoryBarriers = &image_barrier;

    vkCmdPipelineBarrier2(m_info.cmd_buffer, &dep_info);
}

void RenderingInstance::submit_cmd_buffer()
{

    VkSemaphoreSubmitInfo wait_semaphore_info {};
    wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    wait_semaphore_info.semaphore = m_info.image_acquire_semaphore;
    wait_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSemaphoreSubmitInfo render_semaphore_info {};
    render_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    render_semaphore_info.semaphore = m_info.render_completed_semaphore;
    render_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

    VkCommandBufferSubmitInfo command_buffer_info {};
    command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    command_buffer_info.commandBuffer = m_info.cmd_buffer;

    VkSubmitInfo2 submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submit_info.waitSemaphoreInfoCount = 1;
    submit_info.pWaitSemaphoreInfos = &wait_semaphore_info;
    submit_info.signalSemaphoreInfoCount = 1;
    submit_info.pSignalSemaphoreInfos = &render_semaphore_info;
    submit_info.commandBufferInfoCount = 1;
    submit_info.pCommandBufferInfos = &command_buffer_info;

    VK_CHECK(vkQueueSubmit2(m_info.queue, 1, &submit_info, m_info.fence));
}

void RenderingInstance::present_surface()
{
    VkPresentInfoKHR present_info {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &m_info.render_completed_semaphore;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &m_info.swapchain;
    present_info.pImageIndices = &m_info.swapchain_image_index;

    VK_CHECK(vkQueuePresentKHR(m_info.queue, &present_info));
}

void FrameManager::init(FrameManagerInfo const& info)
{
    m_device = info.device;
    m_queue_family = info.queue_family;

    VkSurfaceCapabilitiesKHR surface_capabilities {};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(info.physical_device, info.surface, &surface_capabilities));
    m_frames_in_flight = surface_capabilities.minImageCount;

    init_synchros_and_command_buffers();
}

void FrameManager::deinit()
{
    for (auto i { 0 }; i < m_frames_in_flight; i++) {
        vkFreeCommandBuffers(m_device, m_cmd_pools[i], 1, &m_cmd_buffers[i]);
        vkDestroyCommandPool(m_device, m_cmd_pools[i], nullptr);

        vkDestroySemaphore(m_device, m_render_completed_semaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_image_acquired_semaphores[i], nullptr);
        vkDestroyFence(m_device, m_fences[i], nullptr);
    }
}

Frame FrameManager::get_frame()
{
    auto current_frame = m_current_frame++ % m_frames_in_flight;
    auto fence = m_fences[current_frame];
    auto image_acquired_semaphore = m_image_acquired_semaphores[current_frame];
    auto render_completed_semaphore = m_render_completed_semaphores[current_frame];
    auto cmd_pool = m_cmd_pools[current_frame];
    auto cmd_buffer = m_cmd_buffers[current_frame];

    return { fence, image_acquired_semaphore, render_completed_semaphore, cmd_pool, cmd_buffer };
}

void FrameManager::init_synchros_and_command_buffers()
{
    m_fences.resize(m_frames_in_flight);
    m_image_acquired_semaphores.resize(m_frames_in_flight);
    m_render_completed_semaphores.resize(m_frames_in_flight);
    m_cmd_pools.resize(m_frames_in_flight);
    m_cmd_buffers.resize(m_frames_in_flight);

    VkFenceCreateInfo fence_create_info {};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphore_create_info {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkCommandPoolCreateInfo cmd_pool_create_info {};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.queueFamilyIndex = m_queue_family;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    for (auto i { 0 }; i < m_frames_in_flight; i++) {
        VK_CHECK(vkCreateFence(m_device, &fence_create_info, nullptr, &m_fences[i]));
        VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_image_acquired_semaphores[i]));
        VK_CHECK(vkCreateSemaphore(m_device, &semaphore_create_info, nullptr, &m_render_completed_semaphores[i]));

        VK_CHECK(vkCreateCommandPool(m_device, &cmd_pool_create_info, nullptr, &m_cmd_pools[i]));

        VkCommandBufferAllocateInfo cmd_buffer_allocate_info {};
        cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buffer_allocate_info.commandPool = m_cmd_pools[i];
        cmd_buffer_allocate_info.commandBufferCount = 1;
        cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VK_CHECK(vkAllocateCommandBuffers(m_device, &cmd_buffer_allocate_info, &m_cmd_buffers[i]));
    }
}

Subsystem::InitResult<void> RendererSubsystem::init(WindowSubsystem* window)
{
    if (m_initialized)
        return MAKE_SUBSYSTEM_INIT_SUCCESS();

    m_window = window;

    volkInitialize();

    vkb::Instance vkb_instance;
    if (auto result = init_instance(window); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR("{}", std::move(result.message));
    } else {
        vkb_instance = std::move(result.value);
    }

    volkLoadInstance(vkb_instance.instance);

    vkb::PhysicalDevice vkb_physical_device;
    if (auto result = init_physical_device(window->create_window_surface(vkb_instance.instance), vkb_instance); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR("{}", std::move(result.message));
    } else {
        vkb_physical_device = std::move(result.value);
    }

    vkb::Device vkb_device;
    if (auto result = init_device(vkb_physical_device); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR("{}", std::move(result.message));
    } else {
        vkb_device = std::move(result.value);
    }

    volkLoadDevice(vkb_device.device);

    VkQueue queue;
    if (auto result = vkb_device.get_queue(vkb::QueueType::graphics); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR("{}", flatten_vkb_detailed_error(result.detailed_failure_reasons()));
    } else {
        queue = result.value();
    }

    uint32_t queue_family;
    if (auto result = vkb_device.get_queue_index(vkb::QueueType::graphics); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR("{}", flatten_vkb_detailed_error(result.detailed_failure_reasons()));
    } else {
        queue_family = result.value();
    }

    m_instance = vkb_instance.instance;
    m_debug_messenger = vkb_instance.debug_messenger;
    m_surface = vkb_physical_device.surface;
    m_physical_device = vkb_physical_device.physical_device;
    m_device = vkb_device.device;
    m_queue = queue;
    m_queue_family = queue_family;

    auto [width, height] = window->get_framebuffer_size();
    init_swapchain(width, height);

    FrameManagerInfo frame_manager_info {};
    frame_manager_info.surface = m_surface;
    frame_manager_info.physical_device = m_physical_device;
    frame_manager_info.device = m_device;
    frame_manager_info.queue_family = m_queue_family;
    m_frame_manager.init(frame_manager_info);

    m_initialized = true;
    return MAKE_SUBSYSTEM_INIT_SUCCESS();
}

void RendererSubsystem::deinit()
{
    if (!m_initialized)
        return;

    vkDeviceWaitIdle(m_device);

    m_frame_manager.deinit();

    for (auto image_view : m_swapchain_image_views)
        vkDestroyImageView(m_device, image_view, nullptr);

    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    m_initialized = false;
}

RenderingInstance RendererSubsystem::try_get_frame()
{
    auto [width, height] = m_window->get_framebuffer_size();
    auto frame = m_frame_manager.get_frame();

    VK_CHECK(vkWaitForFences(m_device, 1, &frame.fence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(m_device, 1, &frame.fence));

    VkAcquireNextImageInfoKHR acquire_info {};
    acquire_info.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    acquire_info.deviceMask = 1;
    acquire_info.semaphore = frame.image_acquired_semaphore;
    acquire_info.swapchain = m_swapchain;
    acquire_info.timeout = UINT64_MAX;

    uint32_t swapchain_image_index {};
    auto result = vkAcquireNextImage2KHR(m_device, &acquire_info, &swapchain_image_index);
    switch (result) {
    case VK_SUBOPTIMAL_KHR:
    case VK_ERROR_OUT_OF_DATE_KHR:
        init_swapchain(width, height);
        return {};
    default:
        VK_CHECK(result);
    }

    auto image = m_swapchain_images[swapchain_image_index];
    auto image_view = m_swapchain_image_views[swapchain_image_index];

    RenderingInstanceInfo rendering_instance_info {};
    rendering_instance_info.image = image;
    rendering_instance_info.image_view = image_view;
    rendering_instance_info.cmd_buffer = frame.cmd_buffer;
    rendering_instance_info.fence = frame.fence;
    rendering_instance_info.image_acquire_semaphore = frame.image_acquired_semaphore;
    rendering_instance_info.render_completed_semaphore = frame.render_completed_semaphore;
    rendering_instance_info.surface = m_surface;
    rendering_instance_info.queue = m_queue;
    rendering_instance_info.queue_family = m_queue_family;
    rendering_instance_info.swapchain = m_swapchain;
    rendering_instance_info.swapchain_image_index = swapchain_image_index;
    rendering_instance_info.swapchain_extent = m_swapchain_extent;

    return RenderingInstance(rendering_instance_info);
}

Subsystem::InitResult<vkb::Instance> RendererSubsystem::init_instance(WindowSubsystem* window)
{
    uint32_t required_ext_count {};
    char const** required_exts = glfwGetRequiredInstanceExtensions(&required_ext_count);

    if (auto result = vkb::InstanceBuilder()
            .require_api_version(1, 3)
            .enable_extensions(std::span(required_exts, required_ext_count))
            .request_validation_layers()
            .use_default_debug_messenger()
            .build();
        !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR_TYPED("{}", flatten_vkb_detailed_error(result.detailed_failure_reasons()));
    } else {
        return MAKE_SUBSYSTEM_INIT_SUCCESS_TYPED(std::move(result.value()));
    }
}

Subsystem::InitResult<vkb::PhysicalDevice> RendererSubsystem::init_physical_device(VkSurfaceKHR surface, vkb::Instance& instance)
{
    VkPhysicalDeviceVulkan13Features vk13_features {};
    vk13_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    vk13_features.dynamicRendering = VK_TRUE;
    vk13_features.synchronization2 = VK_TRUE;

    if (auto result = vkb::PhysicalDeviceSelector(instance)
            .set_required_features_13(vk13_features)
            .set_surface(surface)
            .select();
        !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR_TYPED("{}", flatten_vkb_detailed_error(result.detailed_failure_reasons()));
    } else {
        return MAKE_SUBSYSTEM_INIT_SUCCESS_TYPED(std::move(result.value()));
    }
}

Subsystem::InitResult<vkb::Device> RendererSubsystem::init_device(vkb::PhysicalDevice& physical_device)
{
    if (auto result = vkb::DeviceBuilder(physical_device).build(); !result) {
        return MAKE_SUBSYSTEM_INIT_ERROR_TYPED("{}", flatten_vkb_detailed_error(result.detailed_failure_reasons()));
    } else {
        return MAKE_SUBSYSTEM_INIT_SUCCESS_TYPED(std::move(result.value()));
    }
}

void RendererSubsystem::init_swapchain(
    uint32_t frame_buffer_width,
    uint32_t frame_buffer_height)
{
    auto [format, color_space] = get_surface_format();

    VkSurfaceCapabilitiesKHR surface_capabilities {};
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &surface_capabilities));

    VkSwapchainCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_surface;
    create_info.minImageCount = surface_capabilities.minImageCount;
    create_info.imageFormat = format;
    create_info.imageColorSpace = color_space;
    create_info.imageExtent = { frame_buffer_width, frame_buffer_height };
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_TRUE;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.oldSwapchain = m_swapchain;

    VK_CHECK(vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain));

    m_swapchain_images.clear();
    m_swapchain_image_views.clear();

    uint32_t swapchain_image_count {};
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr));
    m_swapchain_images.resize(swapchain_image_count);
    m_swapchain_image_views.resize(swapchain_image_count);
    VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, m_swapchain_images.data()));

    for (auto i { 0 }; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo image_view_create_info {};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_swapchain_images[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = format;
        image_view_create_info.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        image_view_create_info.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        VK_CHECK(vkCreateImageView(m_device, &image_view_create_info, nullptr, &m_swapchain_image_views[i]));
    }

    m_swapchain_extent = create_info.imageExtent;
}
