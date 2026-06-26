cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp20_concepts    concepts.cpp)
add_executable(cpp20_ranges      ranges.cpp)
add_executable(cpp20_coroutines  coroutines.cpp)
add_executable(cpp20_format      format.cpp)
