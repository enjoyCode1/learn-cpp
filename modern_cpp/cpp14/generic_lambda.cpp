#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>

// ==================== 入门级 ====================

void basic_generic_lambda() {
    std::cout << "=== 入门：泛型 Lambda 基础 ===\n";

    // C++14 允许 lambda 参数使用 auto，编译器自动推导类型
    auto print = [](auto val) {
        std::cout << val << "\n";
    };

    print(42);
    print(3.14);
    print(std::string("hello"));
    print('A');
}

void generic_lambda_arithmetic() {
    std::cout << "\n=== 入门：泛型 Lambda 运算 ===\n";

    auto add = [](auto a, auto b) { return a + b; };

    std::cout << "int:    " << add(1, 2) << "\n";
    std::cout << "double: " << add(1.5, 2.5) << "\n";
    std::cout << "string: " << add(std::string("hello "), std::string("world")) << "\n";
}

// ==================== 中级 ====================

void generic_lambda_with_stl() {
    std::cout << "\n=== 中级：泛型 Lambda 配合 STL ===\n";

    std::vector<int>    vi{3, 1, 4, 1, 5, 9, 2, 6};
    std::vector<double> vd{3.1, 1.4, 1.5, 9.2, 6.5};

    auto print_all = [](const auto& container) {
        for (const auto& v : container) std::cout << v << " ";
        std::cout << "\n";
    };

    auto sort_and_print = [&](auto& container) {
        std::sort(container.begin(), container.end());
        print_all(container);
    };

    std::cout << "sorted int:    ";
    sort_and_print(vi);
    std::cout << "sorted double: ";
    sort_and_print(vd);
}

// 泛型 lambda 作为比较器
void generic_comparator() {
    std::cout << "\n=== 中级：泛型 Lambda 比较器 ===\n";

    auto cmp_by_abs = [](auto a, auto b) {
        return (a < 0 ? -a : a) < (b < 0 ? -b : b);
    };

    std::vector<int> v{-5, 3, -1, 4, -9, 2};
    std::sort(v.begin(), v.end(), cmp_by_abs);
    std::cout << "sorted by abs: ";
    for (auto x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级 ====================

// 泛型 lambda 递归（通过 std::function 或 Y-combinator）
void recursive_generic_lambda() {
    std::cout << "\n=== 高级：泛型 Lambda 递归 ===\n";

    // 通过捕获 std::function 实现递归
    std::function<int(int)> fib = [&](auto n) -> int {
        return n <= 1 ? n : fib(n - 1) + fib(n - 2);
    };

    for (int i = 0; i <= 10; ++i)
        std::cout << "fib(" << i << ")=" << fib(i) << " ";
    std::cout << "\n";
}

// 泛型 lambda 实现 map/filter
void functional_combinators() {
    std::cout << "\n=== 高级：函数式组合子 ===\n";

    auto map = [](const auto& container, auto fn) {
        std::vector<decltype(fn(*container.begin()))> result;
        result.reserve(container.size());
        for (const auto& v : container) result.push_back(fn(v));
        return result;
    };

    auto filter = [](const auto& container, auto pred) {
        std::vector<typename std::decay<decltype(*container.begin())>::type> result;
        for (const auto& v : container)
            if (pred(v)) result.push_back(v);
        return result;
    };

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto doubled = map(v, [](auto x) { return x * 2; });
    auto evens   = filter(v, [](auto x) { return x % 2 == 0; });

    std::cout << "doubled: ";
    for (auto x : doubled) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "evens:   ";
    for (auto x : evens) std::cout << x << " ";
    std::cout << "\n";
}

// 泛型 lambda 捕获可变参数包（C++14 广义 lambda 捕获）
void generalized_capture() {
    std::cout << "\n=== 高级：广义 Lambda 捕获（移动捕获）===\n";

    // C++14 允许在捕获列表中初始化新变量，支持移动捕获
    auto ptr = std::make_unique<int>(42);

    // 移动捕获：C++11 无法做到，C++14 支持
    auto lambda = [p = std::move(ptr)]() {
        std::cout << "captured unique_ptr value: " << *p << "\n";
    };

    lambda();
    std::cout << "original ptr is null: " << (ptr == nullptr) << "\n";
}

int main() {
    basic_generic_lambda();
    generic_lambda_arithmetic();
    generic_lambda_with_stl();
    generic_comparator();
    recursive_generic_lambda();
    functional_combinators();
    generalized_capture();
    return 0;
}
