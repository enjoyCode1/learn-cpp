#include <iostream>
#include <concepts>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

// ==================== 入门级 ====================

// 使用标准库 concept 约束模板参数
template<std::integral T>
T square(T x) { return x * x; }

template<std::floating_point T>
T circle_area(T r) { return T(3.14159265358979) * r * r; }

void basic_concepts() {
    std::cout << "=== 入门：标准库 concept ===\n";

    std::cout << "square(5)=   " << square(5)   << "\n";
    std::cout << "square(5L)=  " << square(5L)  << "\n";
    std::cout << "circle_area= " << circle_area(3.0) << "\n";

    // square(3.14);  // 编译错误：double 不满足 std::integral
}

// ==================== 中级 ====================

// 自定义 concept
template<typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Container = requires(T c) {
    c.begin();
    c.end();
    c.size();
    typename T::value_type;
};

template<Printable T>
void print(const T& val) {
    std::cout << val << "\n";
}

template<Addable T>
T sum_two(T a, T b) { return a + b; }

template<Container C>
void print_container(const C& c) {
    std::cout << "[";
    bool first = true;
    for (const auto& v : c) {
        if (!first) std::cout << ", ";
        std::cout << v;
        first = false;
    }
    std::cout << "]\n";
}

void custom_concepts() {
    std::cout << "\n=== 中级：自定义 concept ===\n";

    print(42);
    print(std::string("hello"));
    print(3.14);

    std::cout << "sum_two(1,2)=" << sum_two(1, 2) << "\n";
    std::cout << "sum_two(1.5,2.5)=" << sum_two(1.5, 2.5) << "\n";

    std::vector<int> v{1, 2, 3, 4, 5};
    print_container(v);
}

// requires 子句约束
template<typename T>
    requires std::is_arithmetic_v<T> && (sizeof(T) >= 4)
T safe_multiply(T a, T b) { return a * b; }

// requires 表达式内嵌
template<typename T>
concept HasToString = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
};

struct MyType {
    int val;
    std::string to_string() const { return "MyType(" + std::to_string(val) + ")"; }
};

void requires_clause_demo() {
    std::cout << "\n=== 中级：requires 子句 ===\n";

    std::cout << "safe_multiply(3,4)=    " << safe_multiply(3, 4) << "\n";
    std::cout << "safe_multiply(1.5,2.0)=" << safe_multiply(1.5, 2.0) << "\n";
    // safe_multiply((short)1, (short)2);  // 编译错误：sizeof(short) < 4

    MyType obj{42};
    if constexpr (HasToString<MyType>)
        std::cout << "MyType.to_string(): " << obj.to_string() << "\n";
}

// ==================== 高级 ====================

// concept 组合
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept SortableContainer = Container<T> && requires(T c) {
    std::sort(c.begin(), c.end());
};

// 泛型算法：用 concept 约束
template<SortableContainer C>
C sorted_copy(C c) {
    std::sort(c.begin(), c.end());
    return c;
}

template<Container C>
    requires Numeric<typename C::value_type>
auto container_sum(const C& c) {
    return std::accumulate(c.begin(), c.end(), typename C::value_type{});
}

void advanced_concepts() {
    std::cout << "\n=== 高级：concept 组合与约束算法 ===\n";

    std::vector<int> v{5, 3, 1, 4, 2};
    auto sv = sorted_copy(v);
    std::cout << "sorted: ";
    print_container(sv);

    std::cout << "sum=" << container_sum(v) << "\n";

    std::vector<double> vd{1.1, 2.2, 3.3};
    std::cout << "double sum=" << container_sum(vd) << "\n";
}

// concept 用于函数重载（比 SFINAE 更清晰）
template<std::integral T>
void describe(T v) {
    std::cout << "integral: " << v << " (bits=" << sizeof(T)*8 << ")\n";
}

template<std::floating_point T>
void describe(T v) {
    std::cout << "floating: " << v << " (bits=" << sizeof(T)*8 << ")\n";
}

template<typename T>
    requires (!std::integral<T> && !std::floating_point<T>)
void describe(const T&) {
    std::cout << "other type\n";
}

void concept_overload() {
    std::cout << "\n=== 高级：concept 约束重载 ===\n";
    describe(42);
    describe(42L);
    describe(3.14f);
    describe(3.14);
    describe(std::string("hi"));
}

int main() {
    basic_concepts();
    custom_concepts();
    requires_clause_demo();
    advanced_concepts();
    concept_overload();
    return 0;
}
