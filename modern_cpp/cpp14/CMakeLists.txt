cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cpp14_generic_lambda      generic_lambda.cpp)
add_executable(cpp14_make_unique         make_unique.cpp)
add_executable(cpp14_return_type_deduction return_type_deduction.cpp)
add_executable(cpp14_variable_template   variable_template.cpp)
