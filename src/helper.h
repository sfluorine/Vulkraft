#pragma once

#define MAKE_NON_COPYABLE(__TYPE__)     \
    __TYPE__(const __TYPE__&) = delete; \
    __TYPE__& operator=(const __TYPE__&) = delete;

#define MAKE_NON_MOVABLE(__TYPE__) \
    __TYPE__(__TYPE__&&) = delete; \
    __TYPE__& operator=(__TYPE__&&) = delete;
