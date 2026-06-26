cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp17_structured_bindings structured_bindings.cpp)
add_executable(cpp17_if_constexpr        if_constexpr.cpp)
add_executable(cpp17_string_view         string_view.cpp)
add_executable(cpp17_optional_variant    optional_variant.cpp)
add_executable(cpp17_filesystem          filesystem.cpp)
