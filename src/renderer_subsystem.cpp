#include <fmt/format.h>

#include <span>
#include <string>
#include <vector>

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

#include "renderer_subsystem.h"

Subsystem::InitResult<void> RendererSubsystem::init(WindowSubsystem* window)
{
    if (m_initialized)
        return MAKE_SUBSYSTEM_INIT_SUCCESS();

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

    m_initialized = true;
    return MAKE_SUBSYSTEM_INIT_SUCCESS();
}

void RendererSubsystem::deinit()
{
    if (!m_initialized)
        return;

    vkDeviceWaitIdle(m_device);

    vkDestroyDevice(m_device, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    m_initialized = false;
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
