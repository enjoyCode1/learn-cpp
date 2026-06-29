cmake_minimum_required(VERSION 3.14)

add_executable(ds_hashmap        hashmap.cpp)
add_executable(ds_hashset        hashset.cpp)
add_executable(ds_bloom_filter   bloom_filter.cpp)
add_executable(ds_consistent_hash consistent_hash.cpp)

foreach(target ds_hashmap ds_hashset ds_bloom_filter ds_consistent_hash)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W4)
    endif()
endforeach()
