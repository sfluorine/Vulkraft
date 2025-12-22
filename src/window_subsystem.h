#pragma once

#if defined(VULKRAFT_WINDOWS)
#    define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(VULKRAFT_LINUX)
#    define GLFW_EXPOSE_NATIVE_X11
#    define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#    error "UNSUPPORTED PLATFORM!"
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <cstdint>

#include "helper.h"
#include "subsystem.h"
#include "vulkan.h"

struct FrameBufferSize {
    int32_t width;
    int32_t height;
};

class WindowSubsystem {
    MAKE_NON_COPYABLE(WindowSubsystem);
    MAKE_NON_MOVABLE(WindowSubsystem);

public:
    static WindowSubsystem* instance()
    {
        static WindowSubsystem instance;
        return &instance;
    }

    Subsystem::InitResult<void> init(char const* title, uint32_t width, uint32_t height, bool resizable = false);

    void deinit();

    void poll_events() const { return glfwPollEvents(); }

    bool should_close() const { return glfwWindowShouldClose(m_window); }

    VkSurfaceKHR create_window_surface(VkInstance instance) const;

    FrameBufferSize get_framebuffer_size() const;

    GLFWwindow* get_window_handle() const { return m_window; }

private:
    WindowSubsystem() = default;

private:
    GLFWwindow* m_window { nullptr };

    bool m_initialized { false };
};
