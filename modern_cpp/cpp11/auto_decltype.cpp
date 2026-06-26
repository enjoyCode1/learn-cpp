#include <iostream>
#include <vector>
#include <map>
#include <typeinfo>
#include <string>
#include <memory>
#include <functional>

// ==================== 入门级 ====================

void basic_auto() {
    std::cout << "\n=== 入门级：基本auto ===\n";

    // auto 基本用法
    auto i = 42;
    auto d = 3.14;
    auto s = std::string("hello");
    std::cout << "auto i=" << i << " d=" << d << " s=" << s << "\n";

    // auto 推导容器迭代器
    std::vector<int> vec = {1, 2, 3, 4, 5};
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // auto 推导 map pair
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    for (auto& kv : m) {
        std::cout << kv.first << "=" << kv.second << " ";
    }
    std::cout << "\n";
}

void basic_decltype() {
    std::cout << "\n=== 入门级：基本decltype ===\n";

    // decltype 推导变量类型
    int x = 10;
    decltype(x) y = 20;  // y 是 int
    std::cout << "decltype(x) y=" << y << "\n";

    double d = 3.14;
    // decltype 推导表达式类型
    decltype(x + d) z = x + d;  // z 是 double
    std::cout << "decltype(x+d) z=" << z << "\n";
}

// decltype 推导表达式类型
template <typename T, typename U>
auto add(T a, U b) -> decltype(a + b) {
    return a + b;
}

void trailing_return_type() {
    std::cout << "\n=== 入门级：尾置返回类型 ===\n";

    // 尾置返回类型
    auto result = add(1, 2.5);
    std::cout << "add(1, 2.5)=" << result << "\n";

    auto result2 = add(std::string("Hello"), std::string(" World"));
    std::cout << "add strings=" << result2 << "\n";
}

// ==================== 中级 ====================

void auto_with_qualifiers() {
    std::cout << "\n=== 中级：auto与限定符 ===\n";

    int x = 10;

    // auto会丢弃顶层const和引用
    auto a1 = x;           // int
    const auto a2 = x;     // const int
    auto& a3 = x;          // int&
    const auto& a4 = x;    // const int&

    a1 = 20;  // OK
    // a2 = 20;  // 错误：const
    a3 = 30;  // OK，修改x
    // a4 = 40;  // 错误：const

    std::cout << "x=" << x << " (modified via a3)\n";

    // auto与指针
    int* p = &x;
    auto p1 = p;           // int*
    auto* p2 = p;          // int*
    const auto* p3 = p;    // const int*

    *p1 = 100;
    std::cout << "x=" << x << " (modified via p1)\n";
}

void decltype_details() {
    std::cout << "\n=== 中级：decltype细节 ===\n";

    int x = 10;

    // decltype(变量) 得到变量类型
    decltype(x) a = 20;    // int

    // decltype((变量)) 得到引用类型
    decltype((x)) b = x;   // int&
    b = 30;
    std::cout << "x=" << x << " (modified via decltype((x)))\n";

    // decltype(表达式) 推导表达式类型
    decltype(x + 1) c = 40;  // int

    // decltype与数组
    int arr[5] = {1, 2, 3, 4, 5};
    decltype(arr) arr2 = {6, 7, 8, 9, 10};  // int[5]

    std::cout << "arr2[0]=" << arr2[0] << "\n";
}

// auto与函数返回类型
auto getVector() -> std::vector<int> {
    return {1, 2, 3, 4, 5};
}

auto getSum(int a, int b) -> decltype(a + b) {
    return a + b;
}

void auto_return_type() {
    std::cout << "\n=== 中级：auto返回类型 ===\n";

    auto vec = getVector();
    std::cout << "vec.size()=" << vec.size() << "\n";

    auto sum = getSum(10, 20);
    std::cout << "sum=" << sum << "\n";
}

// auto与lambda
void auto_with_lambda() {
    std::cout << "\n=== 中级：auto与lambda ===\n";

    // auto存储lambda
    auto lambda = [](int x, int y) { return x + y; };
    std::cout << "lambda(3, 4)=" << lambda(3, 4) << "\n";

    // auto推导lambda返回类型
    auto lambda2 = [](int x) -> auto { return x * 2; };
    std::cout << "lambda2(5)=" << lambda2(5) << "\n";
}

// ==================== 高级 ====================

// decltype(auto)：精确推导类型（C++14特性，C++11可用decltype模拟）
template<typename T>
T&& forward_value(T&& value) {
    return std::forward<T>(value);
}

void decltype_auto_simulation() {
    std::cout << "\n=== 高级：decltype(auto)模拟 ===\n";

    int x = 10;

    // C++11使用decltype模拟decltype(auto)的行为
    auto a = x;           // int（丢弃引用）
    decltype(x) b = x;    // int（保持类型）
    decltype((x)) c = x;  // int&（保持引用）

    c = 20;
    std::cout << "x=" << x << " (modified via decltype((x)))\n";
}

// 使用auto推导复杂类型
void auto_complex_types() {
    std::cout << "\n=== 高级：auto推导复杂类型 ===\n";

    // 智能指针
    auto ptr = std::make_shared<std::string>("smart pointer");
    std::cout << "*ptr=" << *ptr << "\n";

    // 函数指针
    auto func = [](int x) { return x * x; };
    std::cout << "func(5)=" << func(5) << "\n";

    // 嵌套容器
    auto nestedVec = std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5, 6}};
    std::cout << "nestedVec[0][1]=" << nestedVec[0][1] << "\n";

    // pair
    auto p = std::make_pair(42, std::string("answer"));
    std::cout << "pair: " << p.first << ", " << p.second << "\n";
}

// 类型推导辅助：使用decltype推导模板参数
template<typename Container>
auto front(Container& c) -> decltype(c.front()) {
    return c.front();
}

template<typename Container>
auto back(Container& c) -> decltype(c.back()) {
    return c.back();
}

void decltype_with_templates() {
    std::cout << "\n=== 高级：decltype与模板 ===\n";

    std::vector<int> vec = {10, 20, 30};
    std::cout << "front(vec)=" << front(vec) << "\n";
    std::cout << "back(vec)=" << back(vec) << "\n";

    std::vector<std::string> strVec = {"first", "second", "third"};
    std::cout << "front(strVec)=" << front(strVec) << "\n";
}

// SFINAE与decltype
template<typename T>
auto has_size_method(T t) -> decltype(t.size(), std::true_type()) {
    return std::true_type();
}

std::false_type has_size_method(...) {
    return std::false_type();
}

void sfinae_with_decltype() {
    std::cout << "\n=== 高级：SFINAE与decltype ===\n";

    std::vector<int> vec;
    int x = 10;

    std::cout << "vector has size(): " << decltype(has_size_method(vec))::value << "\n";
    std::cout << "int has size(): " << decltype(has_size_method(x))::value << "\n";
}

// 完美转发与decltype
template<typename Func, typename... Args>
auto invoke_and_return(Func func, Args&&... args)
    -> decltype(func(std::forward<Args>(args)...)) {
    std::cout << "Invoking function...\n";
    return func(std::forward<Args>(args)...);
}

int multiply(int a, int b) {
    return a * b;
}

void perfect_forwarding_with_decltype() {
    std::cout << "\n=== 高级：完美转发与decltype ===\n";

    auto result = invoke_and_return(multiply, 6, 7);
    std::cout << "result=" << result << "\n";

    auto lambda_result = invoke_and_return(
        [](const std::string& s) { return s + " world"; },
        std::string("hello")
    );
    std::cout << "lambda_result=" << lambda_result << "\n";
}

// 使用auto简化复杂迭代器类型
void auto_simplify_iterators() {
    std::cout << "\n=== 高级：auto简化迭代器 ===\n";

    std::map<std::string, std::vector<int>> data = {
        {"first", {1, 2, 3}},
        {"second", {4, 5, 6}}
    };

    // 不使用auto：类型非常复杂
    // std::map<std::string, std::vector<int>>::iterator it = data.begin();

    // 使用auto：简洁明了
    for (auto it = data.begin(); it != data.end(); ++it) {
        std::cout << it->first << ": ";
        for (auto val : it->second) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}

// decltype推导成员类型
template<typename T>
class Container {
public:
    using value_type = T;

    void add(const T& value) {
        data_.push_back(value);
    }

    // 使用decltype推导迭代器类型
    auto begin() -> typename std::vector<T>::iterator {
        return data_.begin();
    }

    auto end() -> typename std::vector<T>::iterator {
        return data_.end();
    }

private:
    std::vector<T> data_;
};

void decltype_member_types() {
    std::cout << "\n=== 高级：decltype推导成员类型 ===\n";

    Container<int> container;
    container.add(10);
    container.add(20);
    container.add(30);

    std::cout << "Container: ";
    for (auto it = container.begin(); it != container.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

int main() {
    // 入门级
    basic_auto();
    basic_decltype();
    trailing_return_type();

    // 中级
    auto_with_qualifiers();
    decltype_details();
    auto_return_type();
    auto_with_lambda();

    // 高级
    decltype_auto_simulation();
    auto_complex_types();
    decltype_with_templates();
    sfinae_with_decltype();
    perfect_forwarding_with_decltype();
    auto_simplify_iterators();
    decltype_member_types();

    return 0;
}
