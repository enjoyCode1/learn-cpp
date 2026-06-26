cmake_minimum_required(VERSION 3.16)
project(CppLearning)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3rdparty
add_subdirectory(3rdparty)

# C++ Modern Features
add_subdirectory(modern_cpp/cpp11)
add_subdirectory(modern_cpp/cpp14)
add_subdirectory(modern_cpp/cpp17)
add_subdirectory(modern_cpp/cpp20)

# STL
add_subdirectory(stl/usage)
add_subdirectory(stl/implementation)

# Algorithms
add_subdirectory(algorithms)
