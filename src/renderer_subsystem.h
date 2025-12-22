#pragma once

#include <vector>

#include "helper.h"
#include "subsystem.h"
#include "window_subsystem.h"

static constexpr uint32_t FRAMES_IN_FLIGHT = 3;

struct RenderingInstanceInfo {
    VkImage image;
    VkImageView image_view;
    VkCommandBuffer cmd_buffer;
    VkFence fence;
    VkSemaphore image_acquire_semaphore;
    VkSemaphore render_completed_semaphore;
    VkSurfaceKHR surface;
    VkQueue queue;
    uint32_t queue_family;
    VkSwapchainKHR swapchain;
    uint32_t swapchain_image_index;
    VkExtent2D swapchain_extent;
};

class RenderingInstance {
public:
    RenderingInstance() = default;

    RenderingInstance(RenderingInstanceInfo const& info);

    void begin(float r, float g, float b, float a);

    void end();

    void submit_and_present();

    void bind_graphics_pipeline(VkPipeline graphics_pipeline);

    void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

    operator bool() const { return m_success; }

private:
    void begin_recording();

    void end_recording();

    void begin_rendering(float r, float g, float b, float a);

    void end_rendering();

    void transtition_image(
        VkPipelineStageFlags2 src_stage,
        VkAccessFlags2 src_access,
        VkPipelineStageFlags2 dst_stage,
        VkAccessFlags2 dst_access,
        VkImageLayout old_layout,
        VkImageLayout new_layout);

    void submit_cmd_buffer();

    void present_surface();

private:
    RenderingInstanceInfo m_info {};

    bool m_success { false };
};

struct Frame {
    VkFence fence;
    VkSemaphore image_acquired_semaphore;
    VkSemaphore render_completed_semaphore;
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buffer;
};

class FrameManager {
    MAKE_NON_COPYABLE(FrameManager);
    MAKE_NON_MOVABLE(FrameManager);

public:
    FrameManager() = default;

    void init(VkDevice device, uint32_t queue_family);

    void deinit();

    Frame get_frame();

private:
    void init_synchros_and_command_buffers();

private:
    VkFence m_fences[FRAMES_IN_FLIGHT];
    VkSemaphore m_image_acquired_semaphores[FRAMES_IN_FLIGHT];
    VkSemaphore m_render_completed_semaphores[FRAMES_IN_FLIGHT];
    VkCommandBuffer m_cmd_buffers[FRAMES_IN_FLIGHT];
    VkCommandPool m_cmd_pools[FRAMES_IN_FLIGHT];
    uint32_t m_current_frame { 0 };

    VkDevice m_device { nullptr };
    uint32_t m_queue_family {};
};

class RendererSubsystem {
    MAKE_NON_COPYABLE(RendererSubsystem);
    MAKE_NON_MOVABLE(RendererSubsystem);

public:
    static RendererSubsystem* instance()
    {
        static RendererSubsystem instance;
        return &instance;
    }

    Subsystem::InitResult<void> init(WindowSubsystem* window);

    void deinit();

    RenderingInstance try_get_frame();

    VkSurfaceFormatKHR get_surface_format() const { return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }; }

private:
    RendererSubsystem() = default;

    Subsystem::InitResult<vkb::Instance> init_instance(WindowSubsystem* window);

    Subsystem::InitResult<vkb::PhysicalDevice> init_physical_device(VkSurfaceKHR surface, vkb::Instance& instance);

    Subsystem::InitResult<vkb::Device> init_device(vkb::PhysicalDevice& physical_device);

    void init_swapchain(uint32_t frame_buffer_width, uint32_t frame_buffer_height);

private:
    WindowSubsystem* m_window { nullptr };
    FrameManager m_frame_manager {};

    VkInstance m_instance { nullptr };
    VkDebugUtilsMessengerEXT m_debug_messenger { nullptr };
    VkSurfaceKHR m_surface { nullptr };
    VkPhysicalDevice m_physical_device { nullptr };
    VkDevice m_device { nullptr };
    VkQueue m_queue { nullptr };
    uint32_t m_queue_family {};

    VkSwapchainKHR m_swapchain { nullptr };
    std::vector<VkImage> m_swapchain_images;
    std::vector<VkImageView> m_swapchain_image_views;
    VkExtent2D m_swapchain_extent {};

    bool m_initialized { false };
};
