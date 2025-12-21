#include <fmt/format.h>

#include "vulkan_helper.h"
#include "window_subsystem.h"

Subsystem::InitResult WindowSubsystem::init(char const* title, uint32_t width, uint32_t height, bool resizable)
{
    if (m_initialized)
        return MAKE_SUBSYSTEM_INIT_SUCCESS();

    if (!glfwInit()) {
        char const* description { nullptr };
        auto error = glfwGetError(&description);
        return MAKE_SUBSYSTEM_INIT_ERROR("{}: {}", description, error);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        char const* description { nullptr };
        auto error = glfwGetError(&description);
        return MAKE_SUBSYSTEM_INIT_ERROR("{}: {}", description, error);
    }

    m_initialized = true;
    return MAKE_SUBSYSTEM_INIT_SUCCESS();
}

void WindowSubsystem::deinit()
{
    if (!m_initialized)
        return;

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

VkSurfaceKHR WindowSubsystem::create_window_surface(VkInstance instance) const
{
    VkSurfaceKHR surface;

#if defined(VULKRAFT_WINDOWS)
    VkWin32SurfaceCreateInfoKHR create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance = GetModuleHandle(nullptr);
    create_info.hwnd = glfwGetWin32Window(m_window);

    VK_CHECK(vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));
#elif defined(VULKRAFT_LINUX)
    if (glfwGetPlatform() == GLFW_PLATFORM_X11) {
        VkXlibSurfaceCreateInfoKHR create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        create_info.dpy = glfwGetX11Display();
        create_info.window = glfwGetX11Window(window);

        VK_CHECK(vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &surface));
    } else {
        VkWaylandSurfaceCreateInfoKHR create_info {};
        create_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
        create_info.display = glfwGetWaylandDisplay();
        create_info.surface = glfwGetWaylandWindow(window);

        VK_CHECK(vkCreateWaylandSurfaceKHR(instance, &create_info, nullptr, &surface));
    }
#else
#    error "UNSUPPORTED PLATFORM!"
#endif

    return surface;
}

FrameBufferSize WindowSubsystem::get_framebuffer_size() const
{
    FrameBufferSize size {};
    glfwGetFramebufferSize(m_window, &size.width, &size.height);
    return size;
}

GLFWwindow* WindowSubsystem::get_window_handle() const
{
    return m_window;
}
