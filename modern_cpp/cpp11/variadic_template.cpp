#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <tuple>
#include <type_traits>

// ==================== 入门级 ====================

// 递归终止：零参数版本
void print() {
    std::cout << "\n";
}

// 变参模板：逐个展开
template <typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);  // 递归展开
}

// 计算参数个数
template <typename... Args>
constexpr size_t argCount(Args...) {
    return sizeof...(Args);
}

// 求和：任意数量、任意数值类型
template <typename T>
T sum(T val) { return val; }

template <typename T, typename... Args>
T sum(T first, Args... rest) {
    return first + sum(rest...);
}

void basic_variadic() {
    std::cout << "\n=== 入门级：基本可变参数模板 ===\n";

    // 变参 print
    print(1, 2.5, "hello", 'A');

    // 参数个数
    std::cout << "argCount(1,2,3)=" << argCount(1, 2, 3) << "\n";

    // 求和
    std::cout << "sum(1,2,3,4,5)=" << sum(1, 2, 3, 4, 5) << "\n";
    std::cout << "sum(1.1,2.2,3.3)=" << sum(1.1, 2.2, 3.3) << "\n";
}

// 类型安全的 printf 实现
void safePrintf(const char* fmt) {
    while (*fmt) {
        if (*fmt == '%' && *(fmt + 1) != '\0') {
            throw std::runtime_error("too few arguments");
        }
        std::cout << *fmt++;
    }
}

template <typename T, typename... Args>
void safePrintf(const char* fmt, T value, Args... args) {
    while (*fmt) {
        if (*fmt == '%') {
            std::cout << value;
            safePrintf(fmt + 1, args...);
            return;
        }
        std::cout << *fmt++;
    }
}

// 变参模板拼接字符串（C++11 递归展开）
void concatImpl(std::ostringstream&) {}

template <typename T, typename... Args>
void concatImpl(std::ostringstream& oss, T first, Args... rest) {
    oss << first;
    concatImpl(oss, rest...);
}

template <typename... Args>
std::string concat(Args... args) {
    std::ostringstream oss;
    concatImpl(oss, args...);
    return oss.str();
}

void basic_printf_and_concat() {
    std::cout << "\n=== 入门级：类型安全printf和拼接 ===\n";

    // 类型安全 printf
    safePrintf("name=%, age=%, score=%\n", "Alice", 30, 99.5);

    // concat
    std::cout << concat("Hello", ", ", "World", "!", " ", 2024) << "\n";
}

// ==================== 中级 ====================

// 参数包展开技巧：初始化列表展开
template<typename... Args>
void printWithExpansion(Args... args) {
    std::cout << "\n=== 中级：初始化列表展开 ===\n";

    // 使用初始化列表展开参数包
    int dummy[] = {(std::cout << args << " ", 0)...};
    (void)dummy;  // 避免未使用变量警告
    std::cout << "\n";
}

// 类型萃取：检查所有类型是否相同
template<typename T, typename... Args>
struct all_same : std::true_type {};

template<typename T, typename U, typename... Args>
struct all_same<T, U, Args...>
    : std::integral_constant<bool, std::is_same<T, U>::value && all_same<U, Args...>::value> {};

void type_traits_demo() {
    std::cout << "\n=== 中级：类型萃取 ===\n";

    std::cout << "all int: " << all_same<int, int, int>::value << "\n";
    std::cout << "mixed: " << all_same<int, double, int>::value << "\n";
}

// 变参模板类：Tuple的简化实现
template<typename... Types>
class SimpleTuple;

// 特化：空tuple
template<>
class SimpleTuple<> {
public:
    SimpleTuple() {
        std::cout << "Empty tuple\n";
    }
};

// 递归特化：非空tuple
template<typename Head, typename... Tail>
class SimpleTuple<Head, Tail...> : private SimpleTuple<Tail...> {
public:
    SimpleTuple(Head h, Tail... t)
        : SimpleTuple<Tail...>(t...), head_(h) {}

    Head head() const { return head_; }

    SimpleTuple<Tail...>& tail() {
        return *this;
    }

private:
    Head head_;
};

void simple_tuple_demo() {
    std::cout << "\n=== 中级：简化Tuple实现 ===\n";

    SimpleTuple<int, double, std::string> tuple(42, 3.14, "hello");
    std::cout << "head: " << tuple.head() << "\n";
}

// 完美转发与变参模板
template<typename Func, typename... Args>
void callWithLog(Func func, Args&&... args) {
    std::cout << "\n=== 中级：完美转发 ===\n";
    std::cout << "Calling function with " << sizeof...(args) << " arguments\n";
    func(std::forward<Args>(args)...);
}

void exampleFunc(int a, double b, const std::string& c) {
    std::cout << "a=" << a << ", b=" << b << ", c=" << c << "\n";
}

// ==================== 高级 ====================

// 折叠表达式模拟（C++17特性，C++11需手动递归）
// 逻辑与
template<typename... Args>
bool all_true() { return true; }

template<typename T, typename... Args>
bool all_true(T first, Args... rest) {
    return first && all_true(rest...);
}

// 逻辑或
template<typename... Args>
bool any_true() { return false; }

template<typename T, typename... Args>
bool any_true(T first, Args... rest) {
    return first || any_true(rest...);
}

void fold_expression_simulation() {
    std::cout << "\n=== 高级：折叠表达式模拟 ===\n";

    std::cout << "all_true(true, true, true): " << all_true(true, true, true) << "\n";
    std::cout << "all_true(true, false, true): " << all_true(true, false, true) << "\n";
    std::cout << "any_true(false, false, true): " << any_true(false, false, true) << "\n";
    std::cout << "any_true(false, false, false): " << any_true(false, false, false) << "\n";
}

// 索引访问：获取参数包中第N个元素
template<size_t N, typename T, typename... Args>
struct get_nth {
    using type = typename get_nth<N - 1, Args...>::type;
};

template<typename T, typename... Args>
struct get_nth<0, T, Args...> {
    using type = T;
};

template<size_t N, typename T, typename... Args>
typename get_nth<N, T, Args...>::type get_nth_value(T first, Args... rest) {
    if (N == 0) return first;
    return get_nth_value<N - 1>(rest...);
}

// 特化版本以正确处理递归终止
template<size_t N, typename T>
T get_nth_value_impl(T value) {
    return value;
}

template<size_t N, typename T, typename... Args>
typename std::enable_if<N == 0, T>::type
get_nth_value_impl(T first, Args... rest) {
    return first;
}

template<size_t N, typename T, typename... Args>
typename std::enable_if<(N > 0), typename get_nth<N, T, Args...>::type>::type
get_nth_value_impl(T first, Args... rest) {
    return get_nth_value_impl<N - 1>(rest...);
}

void nth_element_access() {
    std::cout << "\n=== 高级：索引访问参数包 ===\n";

    auto value0 = get_nth_value_impl<0>(10, 20, 30, 40);
    auto value2 = get_nth_value_impl<2>(10, 20, 30, 40);

    std::cout << "Element 0: " << value0 << "\n";
    std::cout << "Element 2: " << value2 << "\n";
}

// 变参模板实现make_unique（C++11没有标准版本）
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_variadic(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Point {
    int x, y;
    Point(int x_, int y_) : x(x_), y(y_) {
        std::cout << "Point(" << x << ", " << y << ") created\n";
    }
};

void variadic_factory() {
    std::cout << "\n=== 高级：变参工厂函数 ===\n";

    auto p1 = make_unique_variadic<Point>(10, 20);
    auto p2 = make_unique_variadic<std::string>("Variadic factory");

    std::cout << "Created string: " << *p2 << "\n";
}

// 变参模板与SFINAE：类型检查
template<typename T>
struct is_printable {
    template<typename U>
    static auto test(int) -> decltype(std::cout << std::declval<U>(), std::true_type());

    template<typename>
    static std::false_type test(...);

    static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template<typename T, typename... Args>
typename std::enable_if<is_printable<T>::value>::type
print_if_printable(T first, Args... rest) {
    std::cout << first << " ";
    print_if_printable(rest...);
}

void print_if_printable() {
    std::cout << "\n";
}

void sfinae_with_variadic() {
    std::cout << "\n=== 高级：SFINAE与变参模板 ===\n";

    print_if_printable(1, 2.5, "hello", 'A');
}

// 参数包展开到容器
template<typename T, typename... Args>
std::vector<T> make_vector(Args... args) {
    std::vector<T> result;
    result.reserve(sizeof...(args));

    // 使用初始化列表展开
    int dummy[] = {(result.push_back(args), 0)...};
    (void)dummy;

    return result;
}

void pack_expansion_to_container() {
    std::cout << "\n=== 高级：参数包展开到容器 ===\n";

    auto vec = make_vector<int>(1, 2, 3, 4, 5);

    std::cout << "Vector: ";
    for (int v : vec) {
        std::cout << v << " ";
    }
    std::cout << "\n";
}

// 变参模板与递归Lambda（需要std::function）
void variadic_with_lambda() {
    std::cout << "\n=== 高级：变参模板与Lambda ===\n";

    // 使用lambda处理每个参数
    auto process_all = [](auto&&... args) {
        int dummy[] = {(std::cout << "Processing: " << args << "\n", 0)...};
        (void)dummy;
    };

    process_all(1, "test", 3.14, 'X');
}

int main() {
    // 入门级
    basic_variadic();
    basic_printf_and_concat();

    // 中级
    printWithExpansion(1, 2, 3, 4, 5);
    type_traits_demo();
    simple_tuple_demo();

    callWithLog(exampleFunc, 42, 3.14, std::string("test"));

    // 高级
    fold_expression_simulation();
    nth_element_access();
    variadic_factory();
    sfinae_with_variadic();
    pack_expansion_to_container();
    variadic_with_lambda();

    return 0;
}
