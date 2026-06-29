cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp23_print           print.cpp)
add_executable(cpp23_expected        expected.cpp)
add_executable(cpp23_flat_containers flat_containers.cpp)
add_executable(cpp23_mdspan          mdspan.cpp)
add_executable(cpp23_ranges_to       ranges_to.cpp)
add_executable(cpp23_misc            misc.cpp)
