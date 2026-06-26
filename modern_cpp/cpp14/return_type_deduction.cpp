#include <iostream>
#include <vector>
#include <string>
#include <type_traits>

// ==================== 入门级 ====================

// C++14 允许普通函数省略 -> 返回类型，编译器自动推导
auto add(int a, int b) {
    return a + b;  // 推导为 int
}

auto concat(const std::string& a, const std::string& b) {
    return a + b;  // 推导为 std::string
}

void basic_return_deduction() {
    std::cout << "=== 入门：返回类型自动推导 ===\n";
    std::cout << "add(1,2)=" << add(1, 2) << "\n";
    std::cout << "concat=" << concat("hello ", "world") << "\n";
}

// ==================== 中级 ====================

// 模板函数返回类型推导
template<typename T, typename U>
auto multiply(T a, U b) {
    return a * b;  // 推导为 T*U 的公共类型
}

// decltype(auto)：保留引用和 cv 限定，auto 会剥离
int g_value = 42;

auto get_value_copy() {
    return g_value;  // 返回 int（值拷贝）
}

decltype(auto) get_value_ref() {
    return (g_value);  // 返回 int&（引用，括号触发引用推导）
}

void decltype_auto_demo() {
    std::cout << "\n=== 中级：decltype(auto) ===\n";

    auto v = get_value_copy();
    v = 100;
    std::cout << "g_value after copy modify: " << g_value << " (unchanged)\n";

    decltype(auto) r = get_value_ref();
    r = 200;
    std::cout << "g_value after ref modify:  " << g_value << " (changed)\n";
}

void template_return_deduction() {
    std::cout << "\n=== 中级：模板返回类型推导 ===\n";
    std::cout << "int*int:    " << multiply(3, 4) << "\n";
    std::cout << "int*double: " << multiply(3, 4.5) << "\n";
    std::cout << "double*double: " << multiply(1.5, 2.5) << "\n";
}

// ==================== 高级 ====================

// 递归函数的返回类型推导（C++14 支持，C++11 不支持）
auto fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// decltype(auto) 用于完美转发返回值
template<typename F, typename... Args>
decltype(auto) invoke_and_return(F&& f, Args&&... args) {
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

void perfect_forward_return() {
    std::cout << "\n=== 高级：decltype(auto) 完美转发返回值 ===\n";

    auto square = [](int x) { return x * x; };
    auto result = invoke_and_return(square, 7);
    std::cout << "square(7)=" << result << "\n";

    // 返回引用的情况
    int val = 10;
    auto get_ref = [&]() -> int& { return val; };
    decltype(auto) ref = invoke_and_return(get_ref);
    ref = 999;
    std::cout << "val after ref modify: " << val << "\n";
}

void recursive_deduction_demo() {
    std::cout << "\n=== 高级：递归函数返回类型推导 ===\n";
    for (int i = 0; i <= 10; ++i)
        std::cout << "fib(" << i << ")=" << fibonacci(i) << " ";
    std::cout << "\n";
}

int main() {
    basic_return_deduction();
    template_return_deduction();
    decltype_auto_demo();
    perfect_forward_return();
    recursive_deduction_demo();
    return 0;
}
