#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <map>

// ==================== 入门级 ====================

void basic_lambda() {
    std::cout << "\n=== 入门级：基本Lambda ===\n";

    // 基本 lambda
    auto greet = []() { std::cout << "Hello, Lambda!\n"; };
    greet();

    // 带参数和返回值
    auto add = [](int a, int b) -> int { return a + b; };
    std::cout << "add(3,4)=" << add(3, 4) << "\n";

    // 值捕获
    int x = 10;
    auto captureByValue = [x]() { std::cout << "capture by value: x=" << x << "\n"; };
    x = 99;
    captureByValue();  // 仍输出 10

    // 引用捕获
    int y = 10;
    auto captureByRef = [&y]() { std::cout << "capture by ref: y=" << y << "\n"; };
    y = 99;
    captureByRef();  // 输出 99

    // mutable：修改值捕获的副本
    int count = 0;
    auto inc = [count]() mutable { std::cout << "mutable count=" << ++count << "\n"; };
    inc();
    inc();
    std::cout << "original count=" << count << "\n";  // 仍为 0

    // lambda 配合 STL 算法
    std::vector<int> vec = {5, 3, 1, 4, 2};
    std::sort(vec.begin(), vec.end(), [](int a, int b) { return a < b; });
    std::cout << "sorted: ";
    for (auto v : vec) std::cout << v << " ";
    std::cout << "\n";

    // lambda 存入 std::function
    std::function<int(int)> square = [](int n) { return n * n; };
    std::cout << "square(5)=" << square(5) << "\n";
}

// ==================== 中级 ====================

// C++11方式：使用函数对象模拟泛型lambda
struct GenericPrint {
    template<typename T>
    void operator()(const T& value) const {
        std::cout << "generic: " << value << "\n";
    }
};

// 泛型 Lambda (C++14风格，但在C++11可以用模板函数模拟)
void generic_lambda() {
    std::cout << "\n=== 中级：泛型Lambda模拟 ===\n";

    // 使用std::function和模板函数模拟泛型lambda
    auto print = [](const auto& value) {
        std::cout << "value: " << value << "\n";
    };

    GenericPrint gp;
    gp(42);
    gp(3.14);
    gp(std::string("hello"));
}

// 捕获this指针
class Counter {
public:
    Counter() : count_(0) {}

    void startCounting() {
        // 捕获this指针，可以访问成员变量和成员函数
        auto increment = [this]() {
            count_++;
            std::cout << "Counter: " << count_ << "\n";
        };

        for (int i = 0; i < 3; ++i) {
            increment();
        }
    }

    // 返回lambda用于延迟执行
    std::function<void()> getIncrementer() {
        return [this]() { count_++; };
    }

    int getCount() const { return count_; }

private:
    int count_;
};

void capture_this() {
    std::cout << "\n=== 中级：捕获this ===\n";

    Counter c;
    c.startCounting();

    auto inc = c.getIncrementer();
    inc();
    inc();
    std::cout << "Final count: " << c.getCount() << "\n";
}

// IIFE (Immediately Invoked Function Expression)
void iife_pattern() {
    std::cout << "\n=== 中级：IIFE模式 ===\n";

    // 用于复杂的初始化
    const int value = [&]() {
        int temp = 10;
        temp *= 2;
        temp += 5;
        return temp;
    }();

    std::cout << "IIFE result: " << value << "\n";

    // 用于条件初始化const变量
    const std::string message = [](bool isError) {
        if (isError) return std::string("Error occurred");
        else return std::string("Success");
    }(false);

    std::cout << "Message: " << message << "\n";
}

// Lambda作为回调函数
void lambda_callbacks() {
    std::cout << "\n=== 中级：Lambda作为回调 ===\n";

    // 模拟事件系统
    using Callback = std::function<void(const std::string&)>;
    std::vector<Callback> callbacks;

    // 注册多个回调
    callbacks.push_back([](const std::string& msg) {
        std::cout << "Callback 1: " << msg << "\n";
    });

    int counter = 0;
    callbacks.push_back([&counter](const std::string& msg) {
        counter++;
        std::cout << "Callback 2 (" << counter << "): " << msg << "\n";
    });

    // 触发所有回调
    for (auto& cb : callbacks) {
        cb("Event triggered!");
    }
}

// ==================== 高级 ====================

// 递归Lambda (C++11需要使用std::function)
void recursive_lambda() {
    std::cout << "\n=== 高级：递归Lambda ===\n";

    // 阶乘计算
    std::function<int(int)> factorial = [&factorial](int n) -> int {
        return n <= 1 ? 1 : n * factorial(n - 1);
    };

    std::cout << "factorial(5) = " << factorial(5) << "\n";

    // 斐波那契数列（带记忆化）
    std::map<int, long long> memo;
    std::function<long long(int)> fibonacci = [&](int n) -> long long {
        if (n <= 1) return n;
        if (memo.count(n)) return memo[n];
        return memo[n] = fibonacci(n - 1) + fibonacci(n - 2);
    };

    std::cout << "fibonacci(10) = " << fibonacci(10) << "\n";
}

// Lambda与智能指针：延迟释放资源
void lambda_with_smart_ptr() {
    std::cout << "\n=== 高级：Lambda与智能指针 ===\n";

    // 捕获智能指针，延长对象生命周期
    auto createDelayedPrinter = []() {
        auto ptr = std::make_shared<std::string>("Delayed message");

        // 返回的lambda捕获了shared_ptr，即使外部ptr销毁，对象仍存活
        return [ptr]() {
            std::cout << "Message: " << *ptr << "\n";
        };
    };

    auto printer = createDelayedPrinter();
    // 此时原始的shared_ptr已销毁，但对象仍存活
    printer();

    // 使用unique_ptr的移动捕获（C++14特性，C++11需要模拟）
    auto uniquePtr = std::unique_ptr<int>(new int(42));
    // C++11方式：使用shared_ptr或手动管理
    auto sharedPtr = std::shared_ptr<int>(new int(42));
    auto lambda = [sharedPtr]() {
        std::cout << "Captured value: " << *sharedPtr << "\n";
    };
    lambda();
}

// 高级捕获技巧：混合捕获
void advanced_captures() {
    std::cout << "\n=== 高级：混合捕获 ===\n";

    int a = 1, b = 2, c = 3;

    // 混合捕获：部分值捕获，部分引用捕获
    auto mixed = [a, &b, c]() mutable {
        a += 10;  // 修改副本
        b += 10;  // 修改原值
        std::cout << "Inside lambda: a=" << a << " b=" << b << " c=" << c << "\n";
    };

    mixed();
    std::cout << "After lambda: a=" << a << " b=" << b << " c=" << c << "\n";

    // 捕获所有（慎用）
    auto captureAll = [=]() {  // 按值捕获所有
        std::cout << "All by value: a=" << a << " b=" << b << " c=" << c << "\n";
    };
    captureAll();

    auto captureAllRef = [&]() {  // 按引用捕获所有
        std::cout << "All by ref: a=" << a << " b=" << b << " c=" << c << "\n";
    };
    captureAllRef();
}

// Lambda实现策略模式
class Sorter {
public:
    using CompareFunc = std::function<bool(int, int)>;

    Sorter(CompareFunc comp) : compare_(comp) {}

    void sort(std::vector<int>& vec) {
        std::sort(vec.begin(), vec.end(), compare_);
    }

private:
    CompareFunc compare_;
};

void lambda_strategy_pattern() {
    std::cout << "\n=== 高级：策略模式 ===\n";

    std::vector<int> vec1 = {5, 2, 8, 1, 9};
    std::vector<int> vec2 = vec1;

    // 升序策略
    Sorter ascending([](int a, int b) { return a < b; });
    ascending.sort(vec1);

    std::cout << "Ascending: ";
    for (int v : vec1) std::cout << v << " ";
    std::cout << "\n";

    // 降序策略
    Sorter descending([](int a, int b) { return a > b; });
    descending.sort(vec2);

    std::cout << "Descending: ";
    for (int v : vec2) std::cout << v << " ";
    std::cout << "\n";
}

// Lambda工厂模式
auto createMultiplier(int factor) {
    return [factor](int value) {
        return value * factor;
    };
}

void lambda_factory_pattern() {
    std::cout << "\n=== 高级：工厂模式 ===\n";

    auto double_it = createMultiplier(2);
    auto triple_it = createMultiplier(3);

    std::cout << "double_it(5) = " << double_it(5) << "\n";
    std::cout << "triple_it(5) = " << triple_it(5) << "\n";
}

// 完美转发与Lambda（C++11限制）
template<typename Func, typename... Args>
auto callWithLogging(Func&& func, Args&&... args)
    -> decltype(func(std::forward<Args>(args)...))
{
    std::cout << "Calling function...\n";
    auto result = func(std::forward<Args>(args)...);
    std::cout << "Function returned\n";
    return result;
}

void lambda_perfect_forwarding() {
    std::cout << "\n=== 高级：Lambda与完美转发 ===\n";

    auto add = [](int a, int b) { return a + b; };

    int result = callWithLogging(add, 3, 4);
    std::cout << "Result: " << result << "\n";
}

int main() {
    // 入门级
    basic_lambda();

    // 中级
    generic_lambda();
    capture_this();
    iife_pattern();
    lambda_callbacks();

    // 高级
    recursive_lambda();
    lambda_with_smart_ptr();
    advanced_captures();
    lambda_strategy_pattern();
    lambda_factory_pattern();
    lambda_perfect_forwarding();

    return 0;
}
