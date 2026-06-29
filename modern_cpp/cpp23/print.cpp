#include <print>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <format>

// ==================== 入门级 ====================

void basic_print() {
    std::cout << "=== 入门：std::print / std::println ===\n";

    // C++23 std::print：类似 Python print，基于 std::format
    std::print("Hello, {}!\n", "world");
    std::println("int={}, double={:.2f}, bool={}", 42, 3.14, true);

    // std::println 自动追加换行
    std::println("name={}, age={}", "Alice", 30);

    // 位置参数
    std::println("{0} + {1} = {2}", 1, 2, 3);
}

void print_format_specifiers() {
    std::println("\n=== 入门：格式说明符 ===");

    // 宽度与对齐
    std::println("|{:10}|{:<10}|{:>10}|{:^10}|", "def", "left", "right", "center");

    // 数字
    std::println("dec={} hex={:x} HEX={:X} oct={:o} bin={:b}", 255, 255, 255, 255, 255);

    // 浮点
    std::println("fixed={:.3f} sci={:.2e} pct={:.1%}", 3.14159, 3.14159, 0.856);

    // 填充
    std::println("{:*>10}", "hi");
    std::println("{:0>8b}", 42);
}

// ==================== 中级 ====================

struct Student {
    std::string name;
    int age;
    double gpa;
};

void print_table() {
    std::println("\n=== 中级：表格输出 ===");

    std::vector<Student> students{
        {"Alice",   20, 3.95},
        {"Bob",     22, 3.60},
        {"Charlie", 21, 3.75},
        {"Dave",    23, 3.88},
    };

    std::println("{:<10} {:>4} {:>6}", "Name", "Age", "GPA");
    std::println("{}", std::string(22, '-'));
    for (const auto& s : students)
        std::println("{:<10} {:>4} {:>6.2f}", s.name, s.age, s.gpa);
}

void print_to_stderr() {
    std::println("\n=== 中级：输出到 stderr ===");

    // std::print/println 可以指定输出流
    std::println(stdout, "this goes to stdout");
    std::println(stderr, "this goes to stderr");
    std::println("default goes to stdout");
}

// ==================== 高级 ====================

// 自定义类型支持 std::print（通过 std::formatter）
struct Point {
    double x, y;
};

template<>
struct std::formatter<Point> {
    char fmt = 'f';
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && (*it == 'f' || *it == 'e'))
            fmt = *it++;
        return it;
    }
    auto format(const Point& p, format_context& ctx) const {
        if (fmt == 'e')
            return std::format_to(ctx.out(), "({:.2e}, {:.2e})", p.x, p.y);
        return std::format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
    }
};

void custom_type_print() {
    std::println("\n=== 高级：自定义类型 ===");

    Point p{3.14159, 2.71828};
    std::println("point: {}", p);
    std::println("point sci: {:e}", p);

    std::vector<Point> pts{{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    for (const auto& pt : pts)
        std::println("  {}", pt);
}

void print_performance_demo() {
    std::println("\n=== 高级：print vs cout 风格对比 ===");

    // std::print 线程安全：单次调用是原子的，不会与其他线程输出交错
    // std::cout << a << b 不是原子的，多线程下可能交错

    std::string name = "C++23";
    int year = 2023;
    double version = 23.0;

    // 旧风格
    // std::cout << "name=" << name << " year=" << year << " ver=" << version << "\n";

    // 新风格：更简洁，线程安全，性能更好
    std::println("name={} year={} ver={:.1f}", name, year, version);

    // 构建字符串
    std::string s = std::format("built: name={} year={}", name, year);
    std::println("formatted string: {}", s);
}

int main() {
    basic_print();
    print_format_specifiers();
    print_table();
    print_to_stderr();
    custom_type_print();
    print_performance_demo();
    return 0;
}
