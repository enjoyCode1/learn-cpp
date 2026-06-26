#include <iostream>
#include <tuple>
#include <map>
#include <vector>
#include <string>
#include <utility>

// ==================== 入门级 ====================

void basic_structured_bindings() {
    std::cout << "=== 入门：结构化绑定基础 ===\n";

    // 绑定 pair
    std::pair<int, std::string> p{42, "hello"};
    auto [id, name] = p;
    std::cout << "id=" << id << " name=" << name << "\n";

    // 绑定 tuple
    std::tuple<int, double, std::string> t{1, 3.14, "world"};
    auto [a, b, c] = t;
    std::cout << "a=" << a << " b=" << b << " c=" << c << "\n";

    // 绑定数组
    int arr[3] = {10, 20, 30};
    auto [x, y, z] = arr;
    std::cout << "x=" << x << " y=" << y << " z=" << z << "\n";
}

// ==================== 中级 ====================

struct Point3D {
    double x, y, z;
};

void struct_binding() {
    std::cout << "\n=== 中级：绑定结构体 ===\n";

    Point3D pt{1.0, 2.0, 3.0};
    auto [x, y, z] = pt;
    std::cout << "Point3D: x=" << x << " y=" << y << " z=" << z << "\n";

    // 引用绑定，修改原值
    auto& [rx, ry, rz] = pt;
    rx = 10.0;
    std::cout << "after modify: pt.x=" << pt.x << "\n";
}

void map_iteration() {
    std::cout << "\n=== 中级：map 迭代 ===\n";

    std::map<std::string, int> scores{
        {"Alice", 95}, {"Bob", 87}, {"Charlie", 92}
    };

    // C++17 前写法
    // for (const auto& kv : scores)
    //     std::cout << kv.first << ": " << kv.second << "\n";

    // C++17 结构化绑定，更清晰
    for (const auto& [name, score] : scores)
        std::cout << name << ": " << score << "\n";
}

void map_insert_check() {
    std::cout << "\n=== 中级：map insert 返回值绑定 ===\n";

    std::map<std::string, int> m;

    // insert 返回 pair<iterator, bool>
    auto [it1, ok1] = m.insert({"key1", 100});
    std::cout << "insert key1: ok=" << ok1 << " val=" << it1->second << "\n";

    auto [it2, ok2] = m.insert({"key1", 200});  // 重复插入
    std::cout << "insert key1 again: ok=" << ok2 << " val=" << it2->second << "\n";
}

// ==================== 高级 ====================

// 结构化绑定 + 自定义 get<> 支持（让自定义类型支持结构化绑定）
class RGB {
public:
    RGB(uint8_t r, uint8_t g, uint8_t b) : r_(r), g_(g), b_(b) {}

    template<size_t I>
    auto get() const {
        if constexpr (I == 0) return r_;
        else if constexpr (I == 1) return g_;
        else return b_;
    }

private:
    uint8_t r_, g_, b_;
};

// 特化 tuple_size 和 tuple_element 使 RGB 支持结构化绑定
namespace std {
    template<> struct tuple_size<RGB> : integral_constant<size_t, 3> {};
    template<size_t I> struct tuple_element<I, RGB> { using type = uint8_t; };
}

void custom_structured_binding() {
    std::cout << "\n=== 高级：自定义类型支持结构化绑定 ===\n";

    RGB color{255, 128, 0};
    auto [r, g, b] = color;
    std::cout << "RGB: r=" << (int)r << " g=" << (int)g << " b=" << (int)b << "\n";
}

// 结构化绑定 + 移动语义
void structured_binding_move() {
    std::cout << "\n=== 高级：结构化绑定与移动 ===\n";

    std::map<std::string, std::vector<int>> data{
        {"evens", {2, 4, 6, 8}},
        {"odds",  {1, 3, 5, 7}}
    };

    for (auto& [key, values] : data) {
        std::cout << key << ": ";
        for (auto v : values) std::cout << v << " ";
        std::cout << "\n";
    }

    // 移动出 map
    auto node = data.extract("evens");
    auto k = std::move(node.key());
    auto v = std::move(node.mapped());
    std::cout << "extracted key=" << k << " size=" << v.size() << "\n";
}

int main() {
    basic_structured_bindings();
    struct_binding();
    map_iteration();
    map_insert_check();
    custom_structured_binding();
    structured_binding_move();
    return 0;
}
