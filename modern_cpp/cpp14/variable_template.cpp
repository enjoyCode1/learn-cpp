#include <iostream>
#include <cmath>
#include <type_traits>
#include <string>

// ==================== 入门级 ====================

// C++14 变量模板：用模板参数化一个变量
template<typename T>
constexpr T pi = T(3.14159265358979323846);

void basic_variable_template() {
    std::cout << "=== 入门：变量模板 pi ===\n";
    std::cout << "pi<float>:       " << pi<float>  << "\n";
    std::cout << "pi<double>:      " << pi<double> << "\n";
    std::cout << "pi<long double>: " << pi<long double> << "\n";
}

// ==================== 中级 ====================

// 数学常量变量模板
template<typename T> constexpr T e      = T(2.71828182845904523536);
template<typename T> constexpr T sqrt2  = T(1.41421356237309504880);
template<typename T> constexpr T log2e  = T(1.44269504088896340736);

void math_constants() {
    std::cout << "\n=== 中级：数学常量变量模板 ===\n";
    std::cout << "e<double>:     " << e<double>     << "\n";
    std::cout << "sqrt2<double>: " << sqrt2<double> << "\n";
    std::cout << "log2e<double>: " << log2e<double> << "\n";

    // 用于计算
    double radius = 5.0;
    std::cout << "circle area(r=5): " << pi<double> * radius * radius << "\n";
}

// 类型特征变量模板（C++17 标准库大量使用此模式）
template<typename T>
constexpr bool is_integral_v = std::is_integral<T>::value;

template<typename T>
constexpr bool is_floating_point_v = std::is_floating_point<T>::value;

template<typename T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;

void type_traits_variable_template() {
    std::cout << "\n=== 中级：类型特征变量模板 ===\n";
    std::cout << "is_integral<int>:    " << is_integral_v<int>    << "\n";
    std::cout << "is_integral<double>: " << is_integral_v<double> << "\n";
    std::cout << "is_floating_point<double>: " << is_floating_point_v<double> << "\n";
    std::cout << "is_pointer<int*>:    " << is_pointer_v<int*>    << "\n";
    std::cout << "is_pointer<int>:     " << is_pointer_v<int>     << "\n";
}

// ==================== 高级 ====================

// 变量模板 + 偏特化
template<typename T>
constexpr size_t type_size = sizeof(T);

template<typename T>
constexpr size_t type_size<T*> = sizeof(void*);  // 指针偏特化，统一为指针大小

void partial_specialization_demo() {
    std::cout << "\n=== 高级：变量模板偏特化 ===\n";
    std::cout << "type_size<char>:   " << type_size<char>   << "\n";
    std::cout << "type_size<int>:    " << type_size<int>    << "\n";
    std::cout << "type_size<double>: " << type_size<double> << "\n";
    std::cout << "type_size<int*>:   " << type_size<int*>   << " (pointer specialization)\n";
    std::cout << "type_size<char*>:  " << type_size<char*>  << " (pointer specialization)\n";
}

// 变量模板用于编译期查表
template<int N>
constexpr int factorial = N * factorial<N - 1>;

template<>
constexpr int factorial<0> = 1;

void compile_time_table() {
    std::cout << "\n=== 高级：编译期阶乘查表 ===\n";
    std::cout << "0! = " << factorial<0> << "\n";
    std::cout << "1! = " << factorial<1> << "\n";
    std::cout << "5! = " << factorial<5> << "\n";
    std::cout << "10!= " << factorial<10> << "\n";
}

int main() {
    basic_variable_template();
    math_constants();
    type_traits_variable_template();
    partial_specialization_demo();
    compile_time_table();
    return 0;
}
