#include <cstdint>

#if defined(VULKRAFT_WINDOWS)
#    define VULKRAFT_WINMAIN
#    include "platform.h"
#endif

#include "renderer_subsystem.h"

enum class Color {
    Red,
    Green,
    Blue,
};

int32_t main(int32_t argc, char** argv)
{
    auto window = WindowSubsystem::instance();
    if (auto result = window->init("Vulkraft", 800, 600, true); !result) {
        fmt::println(stderr, "{}", result.message);
        return -1;
    }
    auto renderer = RendererSubsystem::instance();
    if (auto result = renderer->init(window); !result) {
        fmt::println(stderr, "{}", result.message);
        return -1;
    }

    Color current_color = Color::Red;

    float red_value = 0.0f;
    float green_value = 0.0f;
    float blue_value = 0.0f;

    float red_factor = 0.1f;
    float green_factor = 0.1f;
    float blue_factor = 0.1f;

    while (!window->should_close()) {
        window->poll_events();

        auto frame = renderer->try_get_frame();
        if (!frame)
            continue;

        if (current_color == Color::Red) {
            red_value += red_factor;
            if (red_value >= 1.0f || red_value <= 0.0f) {
                current_color = Color::Green;
                red_factor *= -1.0f;
            }
        } else if (current_color == Color::Green) {
            green_value += green_factor;
            if (green_value >= 1.0f || green_value <= 0.0f) {
                current_color = Color::Blue;
                green_factor *= -1.0f;
            }
        } else {
            blue_value += blue_factor;
            if (blue_value >= 1.0f || blue_value <= 0.0f) {
                current_color = Color::Red;
                blue_factor *= -1.0f;
            }
        }

        frame.begin(red_value, green_value, blue_value, 1.0f);
        frame.end();
        frame.submit_and_present();
    }

    renderer->deinit();
    window->deinit();
}
