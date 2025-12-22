#include <cstdint>

#if defined(VULKRAFT_WINDOWS)
#    define VULKRAFT_WINMAIN
#    include "platform.h"
#endif

#include "renderer_subsystem.h"

int32_t main(int32_t argc, char** argv)
{
    auto window = WindowSubsystem::instance();
    if (auto result = window->init("Vulkraft", 800, 600); !result) {
        fmt::println(stderr, "{}", result.message);
        return -1;
    }
    auto renderer = RendererSubsystem::instance();
    if (auto result = renderer->init(window); !result) {
        fmt::println(stderr, "{}", result.message);
        return -1;
    }

    renderer->deinit();
    window->deinit();
}
