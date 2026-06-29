cmake_minimum_required(VERSION 3.14)

add_executable(ds_skip_list      skip_list.cpp)
add_executable(ds_ring_buffer    ring_buffer.cpp)
add_executable(ds_lockfree_queue lockfree_queue.cpp)
add_executable(ds_lru_cache      lru_cache.cpp)
add_executable(ds_time_wheel     time_wheel.cpp)

foreach(target ds_skip_list ds_ring_buffer ds_lockfree_queue ds_lru_cache ds_time_wheel)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W4)
    endif()
endforeach()
