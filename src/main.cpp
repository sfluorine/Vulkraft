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

    while (!window->should_close()) {
        window->poll_events();

        auto frame = renderer->try_get_frame();
        if (!frame)
            continue;

        frame.begin(0.1f, 0.1f, 0.1f, 1.0f);
        frame.end();
        frame.submit_and_present();
    }

    renderer->deinit();
    window->deinit();
}
