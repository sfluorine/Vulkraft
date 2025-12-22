#pragma once

#include <fmt/format.h>

#include <string>

#define MAKE_SUBSYSTEM_INIT_ERROR(__fmt__, ...) \
    { false, fmt::format("[SUBSYTEM INIT ERROR]: " __fmt__ __VA_OPT__(, ) __VA_ARGS__) }

#define MAKE_SUBSYSTEM_INIT_ERROR_TYPED(__fmt__, ...)                                         \
    {                                                                                         \
        false, fmt::format("[SUBSYTEM INIT ERROR]: " __fmt__ __VA_OPT__(, ) __VA_ARGS__), { } \
    }

#define MAKE_SUBSYSTEM_INIT_SUCCESS() \
    { true, "" }

#define MAKE_SUBSYSTEM_INIT_SUCCESS_TYPED(__value__) \
    { true, "", __value__ }

namespace Subsystem {

template<typename>
struct InitResult;

template<>
struct InitResult<void> {
    bool success { false };
    std::string message;

    operator bool() const { return success; }
};

template<typename T>
struct InitResult : public InitResult<void> {
    T value;
};

}
