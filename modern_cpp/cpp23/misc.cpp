#include <print>
#include <string>
#include <vector>
#include <algorithm>
#include <ranges>
#include <type_traits>
#include <utility>

// ============================================================
// C++23 其他特性杂项
// 1. if consteval
// 2. static operator()
// 3. auto(x) decay-copy
// 4. 多维下标 operator[]
// 5. std::unreachable
// 6. std::to_underlying
// ============================================================

// ==================== 1. if consteval ====================
// 区分编译期求值和运行期求值，比 std::is_constant_evaluated() 更清晰

constexpr int compute(int x) {
    if consteval {
        // 编译期路径：可以用更精确但较慢的算法
        return x * x + 1;
    } else {
        // 运行期路径：可以调用非 constexpr 函数
        std::println("  runtime compute({})", x);
        return x * x + 1;
    }
}

void if_consteval_demo() {
    std::println("=== 1. if consteval ===");

    constexpr int ct = compute(5);  // 编译期，不打印
    std::println("compile-time result: {}", ct);

    int n = 5;
    int rt = compute(n);            // 运行期，打印日志
    std::println("runtime result:      {}", rt);
}

// ==================== 2. static operator() ====================
// lambda 或仿函数的 operator() 声明为 static，无需 this 指针，更高效

void static_operator_demo() {
    std::println("\n=== 2. static operator() ===");

    // C++23：lambda 可以声明为 static（不捕获任何东西时）
    auto square = [](int x) static { return x * x; };
    auto add    = [](int a, int b) static { return a + b; };

    std::println("square(7)={}", square(7));
    std::println("add(3,4)={}", add(3, 4));

    // 配合 STL 算法：static lambda 可以转换为函数指针
    std::vector<int> v{5, 3, 1, 4, 2};
    std::ranges::sort(v, [](int a, int b) static { return a < b; });

    std::print("sorted: ");
    for (auto x : v) std::print("{} ", x);
    std::println("");

    // 仿函数的 static operator()
    struct Multiplier {
        int factor;
        // 注意：有成员变量时不能是 static，这里演示无状态情况
        auto operator()(int x) const { return x * factor; }
    };

    Multiplier m3{3};
    std::println("multiplier(5)={}", m3(5));
}

// ==================== 3. auto(x) decay-copy ====================
// 显式拷贝并 decay，等价于 std::decay_t<decltype(x)>(x)
// 在 lambda 捕获、泛型代码中很有用

void decay_copy_demo() {
    std::println("\n=== 3. auto(x) decay-copy ===");

    // 场景：在 lambda 中捕获迭代器的拷贝
    std::vector<std::string> words{"hello", "world", "cpp", "23"};

    // auto(it) 显式拷贝迭代器，捕获到 lambda 中
    auto it = words.begin();
    // MSVC workaround: 先用 auto(it) 拷贝，再捕获变量
    auto it_copy = auto(it);
    auto get_current = [it_copy]() { return *it_copy; };
    ++it;  // 移动原迭代器，it_copy 不受影响
    std::println("captured copy: {}", get_current());

    // decay 数组为指针
    const char arr[] = "hello";
    auto decayed = auto(arr);  // const char* 而不是 const char[6]
    std::println("decayed type is pointer: {}", std::is_pointer_v<decltype(decayed)>);

    // 泛型函数中强制拷贝
    auto process = [](auto&& val) {
        auto copy = auto(val);  // 无论 val 是引用还是值，都拷贝
        return copy;
    };

    std::string s = "original";
    auto copied = process(s);
    std::println("process copy: {}", copied);
}

// ==================== 4. 多维下标 operator[] ====================
// C++23 允许 operator[] 接受多个参数

struct Matrix2D {
    std::vector<double> data;
    size_t rows, cols;

    Matrix2D(size_t r, size_t c, double init = 0.0)
        : data(r * c, init), rows(r), cols(c) {}

    // C++23：多维下标
    double& operator[](size_t i, size_t j) {
        return data[i * cols + j];
    }
    const double& operator[](size_t i, size_t j) const {
        return data[i * cols + j];
    }

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            std::print("  ");
            for (size_t j = 0; j < cols; ++j)
                std::print("{:6.1f}", (*this)[i, j]);
            std::println("");
        }
    }
};

void multidim_subscript_demo() {
    std::println("\n=== 4. 多维下标 operator[] ===");

    Matrix2D m(3, 3);
    // 对角线赋值
    for (size_t i = 0; i < 3; ++i)
        m[i, i] = static_cast<double>(i + 1);

    std::println("diagonal matrix:");
    m.print();

    m[0, 2] = 9.0;
    m[2, 0] = 7.0;
    std::println("after modification:");
    m.print();
}

// ==================== 5. std::unreachable ====================
// 告知编译器某路径不可达，辅助优化，替代 __builtin_unreachable()

enum class Direction { North, South, East, West };

int direction_value(Direction d) {
    switch (d) {
        case Direction::North: return 0;
        case Direction::South: return 1;
        case Direction::East:  return 2;
        case Direction::West:  return 3;
    }
    std::unreachable();  // 告知编译器不会到达这里，消除警告
}

void unreachable_demo() {
    std::println("\n=== 5. std::unreachable ===");
    std::println("North={}", direction_value(Direction::North));
    std::println("West={}",  direction_value(Direction::West));
}

// ==================== 6. std::to_underlying ====================
// 获取枚举的底层整数值，替代 static_cast<std::underlying_type_t<E>>(e)

enum class Color : uint8_t { Red = 1, Green = 2, Blue = 4 };
enum class Status : int     { Ok = 0, Error = -1, Pending = 2 };

void to_underlying_demo() {
    std::println("\n=== 6. std::to_underlying ===");

    // C++20 前写法
    // auto v = static_cast<std::underlying_type_t<Color>>(Color::Red);

    // C++23
    std::println("Red={}   (type: uint8_t)", std::to_underlying(Color::Red));
    std::println("Green={} (type: uint8_t)", std::to_underlying(Color::Green));
    std::println("Blue={}  (type: uint8_t)", std::to_underlying(Color::Blue));

    std::println("Ok={}     Error={} Pending={}",
                 std::to_underlying(Status::Ok),
                 std::to_underlying(Status::Error),
                 std::to_underlying(Status::Pending));

    // 位运算组合
    auto flags = std::to_underlying(Color::Red) | std::to_underlying(Color::Blue);
    std::println("Red|Blue = {}", flags);
}

int main() {
    if_consteval_demo();
    static_operator_demo();
    decay_copy_demo();
    multidim_subscript_demo();
    unreachable_demo();
    to_underlying_demo();
    return 0;
}
