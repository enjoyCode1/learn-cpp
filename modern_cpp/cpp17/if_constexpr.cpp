#include <iostream>
#include <type_traits>
#include <string>
#include <vector>

// ==================== 入门级 ====================

template<typename T>
std::string type_name() {
    if constexpr (std::is_integral_v<T>)
        return "integral";
    else if constexpr (std::is_floating_point_v<T>)
        return "floating_point";
    else if constexpr (std::is_pointer_v<T>)
        return "pointer";
    else
        return "other";
}

void basic_if_constexpr() {
    std::cout << "=== 入门：if constexpr 基础 ===\n";
    std::cout << "int:    " << type_name<int>()    << "\n";
    std::cout << "double: " << type_name<double>() << "\n";
    std::cout << "int*:   " << type_name<int*>()   << "\n";
    std::cout << "string: " << type_name<std::string>() << "\n";
}

// ==================== 中级 ====================

// if constexpr 替代模板特化，统一处理不同类型
template<typename T>
void print_value(const T& val) {
    if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "string: \"" << val << "\" (len=" << val.size() << ")\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "float:  " << val << " (rounded=" << (int)val << ")\n";
    } else if constexpr (std::is_integral_v<T>) {
        std::cout << "int:    " << val << " (hex=0x" << std::hex << val << std::dec << ")\n";
    } else {
        std::cout << "other:  [unknown type]\n";
    }
}

void unified_print() {
    std::cout << "\n=== 中级：统一处理不同类型 ===\n";
    print_value(42);
    print_value(3.14);
    print_value(std::string("hello"));
    print_value('A');
}

// 编译期递归：variadic template + if constexpr
template<typename T, typename... Ts>
void print_all(T first, Ts... rest) {
    std::cout << first;
    if constexpr (sizeof...(rest) > 0) {
        std::cout << ", ";
        print_all(rest...);
    } else {
        std::cout << "\n";
    }
}

void variadic_if_constexpr() {
    std::cout << "\n=== 中级：variadic + if constexpr 替代递归终止特化 ===\n";
    print_all(1, 2.5, std::string("three"), 'D');
}

// ==================== 高级 ====================

// 编译期类型分发：序列化示例
template<typename T>
std::string serialize(const T& val) {
    if constexpr (std::is_same_v<T, bool>) {
        return val ? "true" : "false";
    } else if constexpr (std::is_integral_v<T>) {
        return std::to_string(val);
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(val);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + val + "\"";
    } else {
        // 若此分支不被选中，编译器不会实例化其中代码
        static_assert(std::is_same_v<T, void>, "unsupported type for serialize");
        return "";
    }
}

void compile_time_dispatch() {
    std::cout << "\n=== 高级：编译期类型分发（序列化）===\n";
    std::cout << serialize(true)              << "\n";
    std::cout << serialize(42)                << "\n";
    std::cout << serialize(3.14)              << "\n";
    std::cout << serialize(std::string("hi")) << "\n";
}

// SFINAE vs if constexpr：if constexpr 更清晰
template<typename Container>
auto sum(const Container& c) {
    using T = typename Container::value_type;
    T result{};
    if constexpr (std::is_floating_point_v<T>) {
        // 浮点数用 Kahan 求和减少误差
        T comp{};
        for (const auto& v : c) {
            T y = v - comp;
            T t = result + y;
            comp = (t - result) - y;
            result = t;
        }
    } else {
        for (const auto& v : c) result += v;
    }
    return result;
}

void kahan_sum_demo() {
    std::cout << "\n=== 高级：if constexpr 选择算法（Kahan 求和）===\n";
    std::vector<int>    vi{1, 2, 3, 4, 5};
    std::vector<double> vd{0.1, 0.2, 0.3, 0.4, 0.5};
    std::cout << "int sum:    " << sum(vi) << "\n";
    std::cout << "double sum: " << sum(vd) << "\n";
}

int main() {
    basic_if_constexpr();
    unified_print();
    variadic_if_constexpr();
    compile_time_dispatch();
    kahan_sum_demo();
    return 0;
}
