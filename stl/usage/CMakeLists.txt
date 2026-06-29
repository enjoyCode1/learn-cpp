cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC UTF-8 编码支持
if(MSVC)
    add_compile_options(/utf-8 /W4)
endif()

foreach(target vector map set algorithm iterator functional list deque stack_queue array)
    add_executable(stl_${target} ${target}.cpp)
endforeach()
