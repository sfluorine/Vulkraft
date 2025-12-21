#pragma once

#include <fmt/format.h>

#include <string>

#define MAKE_SUBSYSTEM_INIT_ERROR(__fmt__, ...) \
    { false, fmt::format("[SUBSYTEM INIT ERROR]: " __fmt__ __VA_OPT__(, ) __VA_ARGS__) }

#define MAKE_SUBSYSTEM_INIT_SUCCESS() \
    { true, "" }

namespace Subsystem {

struct InitResult {
    bool success { false };
    std::string message;

    operator bool() const { return success; }
};

}
