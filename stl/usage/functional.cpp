#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>
#include <map>

// ============================================================
// <functional> 从入门到高级用法
// 覆盖：函数对象、lambda、std::function、bind、
//        比较/算术仿函数、std::invoke、组合
// ============================================================

// ==================== 入门：内置函数对象 ====================
void builtin_functor_demo() {
    std::cout << "=== 入门：内置函数对象 ===\n";

    std::vector<int> v{5, 3, 1, 4, 2};

    // 算术仿函数
    std::cout << "plus(3,4)="       << std::plus<int>{}(3, 4)       << "\n";
    std::cout << "minus(10,3)="     << std::minus<int>{}(10, 3)     << "\n";
    std::cout << "multiplies(3,4)=" << std::multiplies<int>{}(3, 4) << "\n";
    std::cout << "divides(10,3)="   << std::divides<int>{}(10, 3)   << "\n";
    std::cout << "modulus(10,3)="   << std::modulus<int>{}(10, 3)   << "\n";
    std::cout << "negate(5)="       << std::negate<int>{}(5)        << "\n";

    // 比较仿函数
    std::sort(v.begin(), v.end(), std::greater<int>{});  // 降序
    std::cout << "sort(greater): ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // 逻辑仿函数
    std::cout << "logical_and(1,0)="  << std::boolalpha
              << std::logical_and<bool>{}(true, false) << "\n";
    std::cout << "logical_or(1,0)="   << std::logical_or<bool>{}(true, false) << "\n";
    std::cout << "logical_not(true)=" << std::logical_not<bool>{}(true) << "\n";

    // 配合 accumulate 求积
    std::vector<int> v2{1, 2, 3, 4, 5};
    int prod = std::accumulate(v2.begin(), v2.end(), 1, std::multiplies<int>{});
    std::cout << "product=" << prod << "\n";
}

// ==================== 入门：lambda 表达式 ====================
void lambda_demo() {
    std::cout << "\n=== 入门：lambda 表达式 ===\n";

    // 基础 lambda
    auto add = [](int a, int b) { return a + b; };
    std::cout << "add(3,4)=" << add(3, 4) << "\n";

    // 捕获变量
    int threshold = 3;
    auto gt = [threshold](int x) { return x > threshold; };  // 值捕获
    auto inc = [&threshold](int x) { threshold++; return x + threshold; };  // 引用捕获

    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "count >3: " << std::count_if(v.begin(), v.end(), gt) << "\n";
    std::cout << "inc(10)=" << inc(10) << " threshold=" << threshold << "\n";

    // mutable lambda：修改值捕获的副本
    int counter = 0;
    auto count_calls = [counter]() mutable { return ++counter; };
    std::cout << "mutable lambda: " << count_calls()
              << " " << count_calls() << " (outer counter=" << counter << ")\n";

    // 泛型 lambda（C++14）
    auto print = [](const auto& x) { std::cout << x << " "; };
    print(42); print(3.14); print("hello");
    std::cout << "\n";

    // 立即调用 lambda（IIFE）
    int result = [](int n) {
        int s = 0;
        for (int i = 1; i <= n; ++i) s += i;
        return s;
    }(10);
    std::cout << "IIFE sum(10)=" << result << "\n";
}

// ==================== 中级：std::function ====================
void std_function_demo() {
    std::cout << "\n=== 中级：std::function ===\n";

    // std::function 可以存储任何可调用对象
    std::function<int(int, int)> f;

    // 存储普通函数
    f = [](int a, int b) { return a + b; };
    std::cout << "lambda: f(3,4)=" << f(3, 4) << "\n";

    // 存储函数指针
    auto fp = [](int a, int b) -> int { return a * b; };
    f = fp;
    std::cout << "fp: f(3,4)=" << f(3, 4) << "\n";

    // 存储成员函数（需配合 bind 或 lambda）
    struct Adder {
        int base;
        int add(int x) const { return base + x; }
    };
    Adder adder{100};
    std::function<int(int)> g = [&adder](int x){ return adder.add(x); };
    std::cout << "member func: g(5)=" << g(5) << "\n";

    // 回调函数表（dispatch table）
    std::map<std::string, std::function<int(int,int)>> ops{
        {"add", [](int a, int b){ return a + b; }},
        {"sub", [](int a, int b){ return a - b; }},
        {"mul", [](int a, int b){ return a * b; }},
    };
    std::cout << "dispatch: add(3,4)=" << ops["add"](3,4)
              << " mul(3,4)=" << ops["mul"](3,4) << "\n";
}

// ==================== 中级：std::bind ====================
void bind_demo() {
    std::cout << "\n=== 中级：std::bind ===\n";

    using namespace std::placeholders;  // _1, _2, ...

    auto multiply = [](int a, int b) { return a * b; };

    // 绑定第一个参数
    auto double_it = std::bind(multiply, 2, _1);
    std::cout << "double(5)=" << double_it(5) << "\n";
    std::cout << "double(7)=" << double_it(7) << "\n";

    // 绑定第二个参数
    auto triple_it = std::bind(multiply, _1, 3);
    std::cout << "triple(4)=" << triple_it(4) << "\n";

    // 参数重排
    auto swap_args = std::bind(multiply, _2, _1);
    std::cout << "swap_args(3,10)=" << swap_args(3, 10) << "\n";

    // bind 成员函数
    struct Counter {
        int count = 0;
        void increment(int n) { count += n; }
        int get() const { return count; }
    };
    Counter c;
    auto inc = std::bind(&Counter::increment, &c, _1);
    inc(5); inc(3);
    std::cout << "counter=" << c.get() << "\n";

    // 配合 algorithm
    std::vector<int> v{1, 2, 3, 4, 5};
    auto greater3 = std::bind(std::greater<int>{}, _1, 3);
    std::cout << "count >3: "
              << std::count_if(v.begin(), v.end(), greater3) << "\n";
}

// ==================== 高级：std::invoke ====================
void invoke_demo() {
    std::cout << "\n=== 高级：std::invoke（C++17）===\n";

    // std::invoke 统一调用任何可调用对象
    auto f = [](int a, int b) { return a + b; };
    std::cout << "invoke lambda: " << std::invoke(f, 3, 4) << "\n";

    struct Foo {
        int val = 42;
        int get() const { return val; }
        int add(int x) const { return val + x; }
    };
    Foo foo;

    // invoke 成员函数
    std::cout << "invoke member func: " << std::invoke(&Foo::add, foo, 10) << "\n";

    // invoke 成员变量
    std::cout << "invoke member var: " << std::invoke(&Foo::val, foo) << "\n";

    // 配合 transform 调用成员函数
    std::vector<Foo> foos{{1}, {2}, {3}};
    std::vector<int> vals;
    std::transform(foos.begin(), foos.end(), std::back_inserter(vals),
                   [](const Foo& f2){ return std::invoke(&Foo::get, f2); });
    std::cout << "transform invoke: ";
    for (int v : vals) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 高级：函数组合（compose）====================
// C++ 标准库没有直接的 compose，手动实现
template<typename F, typename G>
auto compose(F f, G g) {
    return [f, g](auto x) { return f(g(x)); };
}

void compose_demo() {
    std::cout << "\n=== 高级：函数组合 ===\n";

    auto add1   = [](int x) { return x + 1; };
    auto double_ = [](int x) { return x * 2; };
    auto square = [](int x) { return x * x; };

    auto f1 = compose(add1, double_);    // add1(double_(x)) = 2x+1
    auto f2 = compose(double_, square);  // double_(square(x)) = 2x²

    std::cout << "add1(double(3))=" << f1(3) << "\n";   // 7
    std::cout << "double(square(3))=" << f2(3) << "\n"; // 18

    // 多级组合
    auto f3 = compose(square, compose(add1, double_));  // square(2x+1)
    std::cout << "square(add1(double(3)))=" << f3(3) << "\n"; // 49

    // 链式变换 vector
    std::vector<int> v{1, 2, 3, 4, 5};
    std::vector<int> out(v.size());
    std::transform(v.begin(), v.end(), out.begin(), f3);
    std::cout << "transform compose: ";
    for (int x : out) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：递归 lambda（Y 组合子）====================
void recursive_lambda_demo() {
    std::cout << "\n=== 高级：递归 lambda ===\n";

    // 方法1：std::function（有额外开销）
    std::function<int(int)> fib = [&](int n) -> int {
        return n <= 1 ? n : fib(n-1) + fib(n-2);
    };
    std::cout << "fib(10)=" << fib(10) << "\n";

    // 方法2：自引用 lambda（C++14，传 self）
    auto factorial = [](auto& self, int n) -> int {
        return n <= 1 ? 1 : n * self(self, n-1);
    };
    std::cout << "factorial(5)=" << factorial(factorial, 5) << "\n";
    std::cout << "factorial(10)=" << factorial(factorial, 10) << "\n";
}

int main() {
    builtin_functor_demo();
    lambda_demo();
    std_function_demo();
    bind_demo();
    invoke_demo();
    compose_demo();
    recursive_lambda_demo();
    return 0;
}
