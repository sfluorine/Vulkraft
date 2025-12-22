cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(Vulkraft CXX)

if(WIN32)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX AND NOT APPLE)
  set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR VK_USE_PLATFORM_WAYLAND_KHR)
else()
  message(FATAL_ERROR "unsupported platform!")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(vendor/volk)
add_subdirectory(vendor/vk-bootstrap)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/fmt)

add_executable(
  Vulkraft
  src/main.cpp
  src/platform.h
  src/helper.h
  src/vulkan_helper.h
  src/vulkan_helper.cpp
  src/subsystem.h
  src/window_subsystem.h
  src/window_subsystem.cpp
  src/renderer_subsystem.h
  src/renderer_subsystem.cpp
)

target_compile_definitions(Vulkraft PRIVATE GLFW_INCLUDE_NONE)

if(WIN32)
  if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set_target_properties(Vulkraft PROPERTIES WIN32_EXECUTABLE TRUE)
  endif()

  target_compile_definitions(Vulkraft PRIVATE WIN32_LEAN_AND_MEAN)
  target_compile_definitions(Vulkraft PRIVATE NOMINMAX)
  target_compile_definitions(Vulkraft PRIVATE VULKRAFT_WINDOWS)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(Vulkraft PRIVATE VULKRAFT_LINUX)
else()
  message(FATAL_ERROR "unsupported platform!")
endif()

target_link_libraries(
  Vulkraft PRIVATE
  volk
  vk-bootstrap::vk-bootstrap
  glfw
  fmt::fmt
)
