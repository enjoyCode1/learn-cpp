#include <iostream>
#include <coroutine>
#include <vector>
#include <string>
#include <optional>
#include <thread>
#include <chrono>

// ==================== 入门级 ====================

// 最简单的 Generator：用协程实现惰性序列
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T val) {
            current_value = val;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() { if (handle) handle.destroy(); }

    Generator(const Generator&) = delete;
    Generator(Generator&& o) noexcept : handle(o.handle) { o.handle = nullptr; }

    bool next() {
        handle.resume();
        return !handle.done();
    }

    T value() const { return handle.promise().current_value; }
};

Generator<int> range(int from, int to) {
    for (int i = from; i < to; ++i)
        co_yield i;
}

void basic_generator() {
    std::cout << "=== 入门：Generator 协程 ===\n";

    auto gen = range(1, 6);
    std::cout << "range(1,6): ";
    while (gen.next())
        std::cout << gen.value() << " ";
    std::cout << "\n";
}

// ==================== 中级 ====================

Generator<int> fibonacci_gen(int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; ++i) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
}

Generator<int> filter_gen(Generator<int> source, int divisor) {
    while (source.next()) {
        int v = source.value();
        if (v % divisor == 0)
            co_yield v;
    }
}

void generator_pipeline() {
    std::cout << "\n=== 中级：Generator 管道 ===\n";

    std::cout << "fib(12): ";
    auto fib = fibonacci_gen(12);
    while (fib.next())
        std::cout << fib.value() << " ";
    std::cout << "\n";

    // 过滤 fibonacci 中的偶数
    std::cout << "fib(15) even: ";
    auto even_fib = filter_gen(fibonacci_gen(15), 2);
    while (even_fib.next())
        std::cout << even_fib.value() << " ";
    std::cout << "\n";
}

// ==================== 高级 ====================

// Task：简单的异步任务协程
struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never  initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    explicit Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }

    void wait() { while (!handle.done()) handle.resume(); }
};

// Awaitable：可等待对象
struct SleepAwaitable {
    int ms;
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        // 实际项目中这里会提交到线程池；这里简化为直接 sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        h.resume();
    }
    void await_resume() {}
};

Task async_task(std::string name, int delay_ms) {
    std::cout << "[" << name << "] started\n";
    co_await SleepAwaitable{delay_ms};
    std::cout << "[" << name << "] completed after " << delay_ms << "ms\n";
}

void task_demo() {
    std::cout << "\n=== 高级：Task 协程 ===\n";
    auto t1 = async_task("TaskA", 10);
    auto t2 = async_task("TaskB", 5);
    t1.wait();
    t2.wait();
}

// 协程实现状态机
enum class State { Idle, Running, Paused, Done };

Generator<State> state_machine() {
    std::cout << "  [state machine] init\n";
    co_yield State::Idle;

    std::cout << "  [state machine] starting\n";
    co_yield State::Running;

    std::cout << "  [state machine] pausing\n";
    co_yield State::Paused;

    std::cout << "  [state machine] resuming\n";
    co_yield State::Running;

    std::cout << "  [state machine] finishing\n";
    co_yield State::Done;
}

void state_machine_demo() {
    std::cout << "\n=== 高级：协程状态机 ===\n";

    auto sm = state_machine();
    auto state_name = [](State s) -> const char* {
        switch (s) {
            case State::Idle:    return "Idle";
            case State::Running: return "Running";
            case State::Paused:  return "Paused";
            case State::Done:    return "Done";
        }
        return "Unknown";
    };

    while (sm.next())
        std::cout << "  -> state: " << state_name(sm.value()) << "\n";
}

int main() {
    basic_generator();
    generator_pipeline();
    task_demo();
    state_machine_demo();
    return 0;
}
