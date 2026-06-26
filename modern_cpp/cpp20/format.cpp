#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <chrono>

// ==================== 入门级 ====================

void basic_format() {
    std::cout << "=== 入门：std::format 基础 ===\n";

    // 替代 printf 和 stringstream
    std::string s = std::format("Hello, {}!", "world");
    std::cout << s << "\n";

    std::cout << std::format("int={}, double={}, bool={}\n", 42, 3.14, true);

    // 位置参数
    std::cout << std::format("{0} + {1} = {2}\n", 1, 2, 3);
    std::cout << std::format("{1} comes before {0}\n", "world", "hello");
}

// ==================== 中级 ====================

void format_specifiers() {
    std::cout << "\n=== 中级：格式说明符 ===\n";

    // 宽度和对齐
    std::cout << std::format("|{:10}|{:<10}|{:>10}|{:^10}|\n",
                             "left", "left", "right", "center");

    // 数字格式
    std::cout << std::format("dec={:d} hex={:x} HEX={:X} oct={:o} bin={:b}\n",
                             255, 255, 255, 255, 255);

    // 浮点数
    std::cout << std::format("fixed={:.2f} sci={:.2e} general={:.4g}\n",
                             3.14159, 3.14159, 3.14159);

    // 填充字符
    std::cout << std::format("{:*>10}\n", "hi");
    std::cout << std::format("{:0>8b}\n", 42);  // 二进制，前补零
}

void format_string_building() {
    std::cout << "\n=== 中级：构建格式化输出 ===\n";

    struct Student {
        std::string name;
        int age;
        double gpa;
    };

    std::vector<Student> students{
        {"Alice",   20, 3.95},
        {"Bob",     22, 3.60},
        {"Charlie", 21, 3.75},
    };

    std::cout << std::format("{:<10} {:>4} {:>6}\n", "Name", "Age", "GPA");
    std::cout << std::string(22, '-') << "\n";
    for (const auto& s : students)
        std::cout << std::format("{:<10} {:>4} {:>6.2f}\n", s.name, s.age, s.gpa);
}

// ==================== 高级 ====================

// 自定义类型支持 std::format
struct Point {
    double x, y;
};

template<>
struct std::formatter<Point> {
    char presentation = 'f';  // 默认浮点格式

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && (*it == 'f' || *it == 'e'))
            presentation = *it++;
        return it;
    }

    auto format(const Point& p, std::format_context& ctx) const {
        if (presentation == 'f')
            return std::format_to(ctx.out(), "({:.2f}, {:.2f})", p.x, p.y);
        return std::format_to(ctx.out(), "({:.2e}, {:.2e})", p.x, p.y);
    }
};

void custom_formatter() {
    std::cout << "\n=== 高级：自定义类型 formatter ===\n";

    Point p{3.14159, 2.71828};
    std::cout << std::format("point (default): {}\n", p);
    std::cout << std::format("point (float):   {:f}\n", p);
    std::cout << std::format("point (sci):     {:e}\n", p);
}

void format_to_demo() {
    std::cout << "\n=== 高级：format_to 写入缓冲区 ===\n";

    std::string buf;
    buf.reserve(256);

    // 直接写入 string，避免临时对象
    std::format_to(std::back_inserter(buf), "line1: {}\n", 42);
    std::format_to(std::back_inserter(buf), "line2: {:.2f}\n", 3.14);
    std::format_to(std::back_inserter(buf), "line3: {}\n", "hello");

    std::cout << buf;

    // format_to_n：限制写入长度
    char arr[20];
    auto res = std::format_to_n(arr, sizeof(arr) - 1, "truncated: {}", 123456789);
    arr[res.out - arr] = '\0';
    std::cout << "format_to_n: " << arr << "\n";
}

int main() {
    basic_format();
    format_specifiers();
    format_string_building();
    custom_formatter();
    format_to_demo();
    return 0;
}
