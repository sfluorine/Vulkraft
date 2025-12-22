#pragma once

#include "helper.h"
#include "subsystem.h"
#include "window_subsystem.h"

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

    VkInstance get_vulkan_instance() const { return m_instance; }

    VkSurfaceKHR get_surface() const { return m_surface; }

    VkPhysicalDevice get_physical_device() const { return m_physical_device; }

    VkDevice get_device() const { return m_device; }

    VkQueue get_queue() const { return m_queue; }

    uint32_t get_queue_family() const { return m_queue_family; }

private:
    RendererSubsystem() = default;

    Subsystem::InitResult<vkb::Instance> init_instance(WindowSubsystem* window);

    Subsystem::InitResult<vkb::PhysicalDevice> init_physical_device(VkSurfaceKHR surface, vkb::Instance& instance);

    Subsystem::InitResult<vkb::Device> init_device(vkb::PhysicalDevice& physical_device);

private:
    VkInstance m_instance { nullptr };
    VkDebugUtilsMessengerEXT m_debug_messenger { nullptr };
    VkSurfaceKHR m_surface { nullptr };
    VkPhysicalDevice m_physical_device { nullptr };
    VkDevice m_device { nullptr };
    VkQueue m_queue { nullptr };
    uint32_t m_queue_family {};

    bool m_initialized { false };
};
