#include <cstdint>

#if defined(VULKRAFT_WINDOWS)
#    define VULKRAFT_WINMAIN
#    include "platform.h"
#endif

#include "window_subsystem.h"

int32_t main(int32_t argc, char** argv)
{
    WindowSubsystem::instance()->init("Vulkraft", 800, 600);
}
