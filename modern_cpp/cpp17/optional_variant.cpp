#include <iostream>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <functional>

// ==================== 入门级 ====================

std::optional<int> parse_int(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return std::nullopt;
    }
}

void basic_optional() {
    std::cout << "=== 入门：optional 基础 ===\n";

    auto r1 = parse_int("42");
    auto r2 = parse_int("abc");

    if (r1) std::cout << "parsed: " << *r1 << "\n";
    if (!r2) std::cout << "parse failed\n";

    // value_or：提供默认值
    std::cout << "r1.value_or(0)=" << r1.value_or(0) << "\n";
    std::cout << "r2.value_or(0)=" << r2.value_or(0) << "\n";
}

void basic_variant() {
    std::cout << "\n=== 入门：variant 基础 ===\n";

    std::variant<int, double, std::string> v;

    v = 42;
    std::cout << "int: " << std::get<int>(v) << "\n";

    v = 3.14;
    std::cout << "double: " << std::get<double>(v) << "\n";

    v = std::string("hello");
    std::cout << "string: " << std::get<std::string>(v) << "\n";

    // index() 查当前类型索引
    std::cout << "index=" << v.index() << "\n";  // 2 = string
}

// ==================== 中级 ====================

// optional 链式操作（模拟 monadic 风格）
std::optional<int> safe_divide(int a, int b) {
    if (b == 0) return std::nullopt;
    return a / b;
}

std::optional<double> safe_sqrt(int n) {
    if (n < 0) return std::nullopt;
    return std::sqrt(static_cast<double>(n));
}

void optional_chaining() {
    std::cout << "\n=== 中级：optional 链式计算 ===\n";

    auto compute = [&](int a, int b) -> std::optional<double> {
        auto div = safe_divide(a, b);
        if (!div) return std::nullopt;
        return safe_sqrt(*div);
    };

    auto r1 = compute(16, 4);
    auto r2 = compute(10, 0);  // 除以零
    auto r3 = compute(-4, 1);  // sqrt 负数

    std::cout << "16/4 sqrt: " << (r1 ? std::to_string(*r1) : "nullopt") << "\n";
    std::cout << "10/0 sqrt: " << (r2 ? std::to_string(*r2) : "nullopt") << "\n";
    std::cout << "-4/1 sqrt: " << (r3 ? std::to_string(*r3) : "nullopt") << "\n";
}

// variant + std::visit 实现多态
struct Circle    { double radius; };
struct Rectangle { double w, h; };
struct Triangle  { double base, height; };

using Shape = std::variant<Circle, Rectangle, Triangle>;

double area(const Shape& s) {
    return std::visit([](const auto& shape) -> double {
        using T = std::decay_t<decltype(shape)>;
        if constexpr (std::is_same_v<T, Circle>)
            return 3.14159 * shape.radius * shape.radius;
        else if constexpr (std::is_same_v<T, Rectangle>)
            return shape.w * shape.h;
        else
            return 0.5 * shape.base * shape.height;
    }, s);
}

void variant_visit() {
    std::cout << "\n=== 中级：variant + visit 实现多态 ===\n";

    std::vector<Shape> shapes{
        Circle{5.0},
        Rectangle{3.0, 4.0},
        Triangle{6.0, 8.0}
    };

    for (const auto& s : shapes)
        std::cout << "area=" << area(s) << "\n";
}

// ==================== 高级 ====================

// variant 实现表达式树（简单计算器）
struct Number { double val; };
struct Add {};
struct Mul {};

using Token = std::variant<Number, Add, Mul>;

// 用 variant 构建错误处理类型（类似 Rust 的 Result<T, E>）
template<typename T, typename E>
using Result = std::variant<T, E>;

struct ParseError { std::string msg; };
struct DivError   { std::string msg; };

Result<double, std::string> safe_compute(const std::string& expr) {
    // 简单解析 "a op b" 格式
    size_t pos = expr.find('+');
    char op = '+';
    if (pos == std::string::npos) {
        pos = expr.find('/');
        op = '/';
    }
    if (pos == std::string::npos)
        return std::string("invalid expression");

    try {
        double a = std::stod(expr.substr(0, pos));
        double b = std::stod(expr.substr(pos + 1));
        if (op == '/' && b == 0.0)
            return std::string("division by zero");
        return op == '+' ? a + b : a / b;
    } catch (...) {
        return std::string("parse error");
    }
}

void result_type_demo() {
    std::cout << "\n=== 高级：variant 实现 Result<T,E> ===\n";

    for (const auto& expr : {"3.0+4.0", "10.0/2.0", "10.0/0.0", "bad"}) {
        auto r = safe_compute(expr);
        if (std::holds_alternative<double>(r))
            std::cout << expr << " = " << std::get<double>(r) << "\n";
        else
            std::cout << expr << " error: " << std::get<std::string>(r) << "\n";
    }
}

// overloaded 模式：组合多个 lambda 成一个 visitor
template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void overloaded_visitor() {
    std::cout << "\n=== 高级：overloaded visitor 模式 ===\n";

    std::vector<std::variant<int, double, std::string>> values{
        42, 3.14, std::string("hello"), 100, 2.71
    };

    auto visitor = overloaded{
        [](int v)               { std::cout << "int:    " << v << "\n"; },
        [](double v)            { std::cout << "double: " << v << "\n"; },
        [](const std::string& v){ std::cout << "string: " << v << "\n"; }
    };

    for (const auto& v : values)
        std::visit(visitor, v);
}

int main() {
    basic_optional();
    basic_variant();
    optional_chaining();
    variant_visit();
    result_type_demo();
    overloaded_visitor();
    return 0;
}
