#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

// ==================== 入门级 ====================

// 普通函数
int add(int a, int b) { return a + b; }
void greet(const std::string& name, int times) {
    for (int i = 0; i < times; ++i)
        std::cout << "Hello, " << name << "!\n";
}

// 成员函数
class Calculator {
public:
    int multiply(int a, int b) const { return a * b; }
    int value = 100;
};

void basic_function() {
    std::cout << "\n=== 入门级：std::function基础 ===\n";

    // std::function 存储普通函数
    std::function<int(int, int)> f = add;
    std::cout << "add(3,4)=" << f(3, 4) << "\n";

    // std::function 存储 lambda
    std::function<std::string(int)> toStr = [](int n) {
        return std::to_string(n);
    };
    std::cout << "toStr(42)=" << toStr(42) << "\n";

    // std::function 存储成员函数（需绑定对象）
    Calculator calc;
    std::function<int(int, int)> mul = std::bind(&Calculator::multiply, &calc,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2);
    std::cout << "multiply(3,5)=" << mul(3, 5) << "\n";
}

void basic_bind() {
    std::cout << "\n=== 入门级：std::bind基础 ===\n";

    // std::bind 绑定部分参数（偏函数）
    auto add5 = std::bind(add, 5, std::placeholders::_1);
    std::cout << "add5(10)=" << add5(10) << "\n";

    // std::bind 绑定成员函数
    auto greetBob = std::bind(greet, "Bob", std::placeholders::_1);
    greetBob(2);

    // 参数重排
    auto subReversed = std::bind([](int a, int b) { return a - b; },
                                  std::placeholders::_2, std::placeholders::_1);
    std::cout << "subReversed(3,10)=" << subReversed(3, 10) << "\n";  // 10-3=7
}

void function_container() {
    std::cout << "\n=== 入门级：函数容器 ===\n";

    // 函数回调容器
    std::vector<std::function<int(int)>> transforms;
    transforms.push_back([](int x) { return x * 2; });
    transforms.push_back([](int x) { return x + 10; });
    transforms.push_back(std::bind(add, std::placeholders::_1, 100));

    int val = 5;
    for (auto& fn : transforms) {
        std::cout << "transform(" << val << ")=" << fn(val) << "\n";
    }
}

// ==================== 中级 ====================

// 命令模式：使用std::function实现
class Command {
public:
    using Action = std::function<void()>;

    void execute() {
        if (action_) action_();
    }

    void setAction(Action action) {
        action_ = action;
    }

private:
    Action action_;
};

void command_pattern() {
    std::cout << "\n=== 中级：命令模式 ===\n";

    Command cmd;

    cmd.setAction([]{ std::cout << "Command 1 executed\n"; });
    cmd.execute();

    cmd.setAction([]{ std::cout << "Command 2 executed\n"; });
    cmd.execute();
}

// 回调注册系统
class EventSystem {
public:
    using Callback = std::function<void(const std::string&)>;

    void subscribe(const std::string& event, Callback callback) {
        callbacks_[event].push_back(callback);
    }

    void trigger(const std::string& event, const std::string& data) {
        if (callbacks_.count(event)) {
            for (auto& cb : callbacks_[event]) {
                cb(data);
            }
        }
    }

private:
    std::map<std::string, std::vector<Callback>> callbacks_;
};

void event_system_demo() {
    std::cout << "\n=== 中级：事件系统 ===\n";

    EventSystem events;

    events.subscribe("login", [](const std::string& user) {
        std::cout << "User logged in: " << user << "\n";
    });

    events.subscribe("login", [](const std::string& user) {
        std::cout << "Logging activity for: " << user << "\n";
    });

    events.subscribe("logout", [](const std::string& user) {
        std::cout << "User logged out: " << user << "\n";
    });

    events.trigger("login", "Alice");
    events.trigger("logout", "Bob");
}

// std::bind与成员变量
class Counter {
public:
    Counter() : count_(0) {}

    void increment() { ++count_; }
    void add(int n) { count_ += n; }
    int get() const { return count_; }

private:
    int count_;
};

void bind_member_functions() {
    std::cout << "\n=== 中级：绑定成员函数 ===\n";

    Counter counter;

    auto inc = std::bind(&Counter::increment, &counter);
    auto add10 = std::bind(&Counter::add, &counter, 10);

    inc();
    inc();
    add10();

    std::cout << "Counter value: " << counter.get() << "\n";
}

// std::ref和std::cref：引用传递
void modifyValue(int& x) {
    x *= 2;
}

void ref_wrapper_demo() {
    std::cout << "\n=== 中级：std::ref引用包装 ===\n";

    int value = 10;

    // std::bind默认按值传递，使用std::ref传递引用
    auto modify = std::bind(modifyValue, std::ref(value));
    modify();

    std::cout << "Modified value: " << value << "\n";
}

// 函数组合：组合多个函数
template<typename F, typename G>
auto compose(F f, G g) -> std::function<decltype(f(g(std::declval<int>())))(int)> {
    return [=](int x) { return f(g(x)); };
}

void function_composition() {
    std::cout << "\n=== 中级：函数组合 ===\n";

    auto double_it = [](int x) { return x * 2; };
    auto add_one = [](int x) { return x + 1; };

    auto composed = compose(double_it, add_one);

    std::cout << "compose(double, add1)(5) = " << composed(5) << "\n";  // (5+1)*2 = 12
}

// ==================== 高级 ====================

// 高阶函数：返回函数的函数
std::function<int(int)> makeMultiplier(int factor) {
    return [factor](int x) { return x * factor; };
}

std::function<int(int)> makeAdder(int addend) {
    return [addend](int x) { return x + addend; };
}

void higher_order_functions() {
    std::cout << "\n=== 高级：高阶函数 ===\n";

    auto triple = makeMultiplier(3);
    auto add100 = makeAdder(100);

    std::cout << "triple(7) = " << triple(7) << "\n";
    std::cout << "add100(42) = " << add100(42) << "\n";

    // 组合高阶函数
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::transform(numbers.begin(), numbers.end(), numbers.begin(), triple);

    std::cout << "After tripling: ";
    for (int n : numbers) std::cout << n << " ";
    std::cout << "\n";
}

// 柯里化：将多参数函数转换为单参数函数链
template<typename R, typename T1, typename T2>
std::function<std::function<R(T2)>(T1)> curry(std::function<R(T1, T2)> f) {
    return [f](T1 x) {
        return [f, x](T2 y) {
            return f(x, y);
        };
    };
}

void currying_demo() {
    std::cout << "\n=== 高级：柯里化 ===\n";

    std::function<int(int, int)> add_func = [](int a, int b) { return a + b; };

    auto curried_add = curry(add_func);
    auto add10 = curried_add(10);

    std::cout << "curried_add(10)(5) = " << add10(5) << "\n";
    std::cout << "curried_add(20)(30) = " << curried_add(20)(30) << "\n";
}

// 记忆化：缓存函数结果
template<typename R, typename... Args>
std::function<R(Args...)> memoize(std::function<R(Args...)> func) {
    auto cache = std::make_shared<std::map<std::tuple<Args...>, R>>();

    return [=](Args... args) mutable -> R {
        auto key = std::make_tuple(args...);
        auto it = cache->find(key);

        if (it != cache->end()) {
            std::cout << "Cache hit!\n";
            return it->second;
        }

        std::cout << "Computing...\n";
        R result = func(args...);
        (*cache)[key] = result;
        return result;
    };
}

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void memoization_demo() {
    std::cout << "\n=== 高级：记忆化 ===\n";

    std::function<int(int)> fib = fibonacci;
    auto memoized_fib = memoize(fib);

    std::cout << "fib(5) = " << memoized_fib(5) << "\n";
    std::cout << "fib(5) again = " << memoized_fib(5) << "\n";
    std::cout << "fib(6) = " << memoized_fib(6) << "\n";
}

// 函数管道：链式调用
template<typename T>
class Pipeline {
public:
    Pipeline(T value) : value_(value) {}

    template<typename F>
    auto then(F func) -> Pipeline<decltype(func(value_))> {
        return Pipeline<decltype(func(value_))>(func(value_));
    }

    T get() const { return value_; }

private:
    T value_;
};

void pipeline_demo() {
    std::cout << "\n=== 高级：函数管道 ===\n";

    auto result = Pipeline<int>(5)
        .then([](int x) { return x * 2; })
        .then([](int x) { return x + 10; })
        .then([](int x) { return x * x; })
        .get();

    std::cout << "Pipeline result: " << result << "\n";  // ((5*2)+10)^2 = 400
}

// 策略模式：使用std::function
class Sorter {
public:
    using CompareFunc = std::function<bool(int, int)>;

    void setStrategy(CompareFunc comp) {
        compare_ = comp;
    }

    void sort(std::vector<int>& data) {
        std::sort(data.begin(), data.end(), compare_);
    }

private:
    CompareFunc compare_;
};

void strategy_pattern() {
    std::cout << "\n=== 高级：策略模式 ===\n";

    std::vector<int> data = {5, 2, 8, 1, 9};

    Sorter sorter;

    sorter.setStrategy([](int a, int b) { return a < b; });
    sorter.sort(data);

    std::cout << "Ascending: ";
    for (int n : data) std::cout << n << " ";
    std::cout << "\n";

    sorter.setStrategy([](int a, int b) { return a > b; });
    sorter.sort(data);

    std::cout << "Descending: ";
    for (int n : data) std::cout << n << " ";
    std::cout << "\n";
}

// 类型擦除：统一接口
class AnyCallable {
public:
    template<typename F>
    AnyCallable(F func) : func_(func) {}

    void operator()() const {
        func_();
    }

private:
    std::function<void()> func_;
};

void type_erasure_demo() {
    std::cout << "\n=== 高级：类型擦除 ===\n";

    std::vector<AnyCallable> tasks;

    tasks.push_back([]{ std::cout << "Task 1\n"; });
    tasks.push_back([]{ std::cout << "Task 2\n"; });
    tasks.push_back([]{ std::cout << "Task 3\n"; });

    for (auto& task : tasks) {
        task();
    }
}

int main() {
    // 入门级
    basic_function();
    basic_bind();
    function_container();

    // 中级
    command_pattern();
    event_system_demo();
    bind_member_functions();
    ref_wrapper_demo();
    function_composition();

    // 高级
    higher_order_functions();
    currying_demo();
    memoization_demo();
    pipeline_demo();
    strategy_pattern();
    type_erasure_demo();

    return 0;
}
