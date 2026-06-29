cmake_minimum_required(VERSION 3.14)

add_executable(ds_adj_matrix    adj_matrix.cpp)
add_executable(ds_adj_list      adj_list.cpp)
add_executable(ds_union_find    union_find.cpp)
add_executable(ds_topo_sort     topo_sort.cpp)

foreach(target ds_adj_matrix ds_adj_list ds_union_find ds_topo_sort)
    target_compile_features(${target} PRIVATE cxx_std_17)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8 /W4)
    endif()
endforeach()
