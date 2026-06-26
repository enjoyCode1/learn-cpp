#include <iostream>
#include <mutex>
#include <thread>

std::recursive_mutex g_rmutex;

// 递归函数中多次加锁，普通 mutex 会死锁，recursive_mutex 不会
void recursiveTask(int depth) {
    std::lock_guard<std::recursive_mutex> lock(g_rmutex);
    std::cout << "depth=" << depth << "\n";
    if (depth > 0) {
        recursiveTask(depth - 1);  // 同一线程再次加锁，recursive_mutex 允许
    }
}

// 演示成员函数互相调用的场景
class SafeCounter {
public:
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        ++count_;
    }

    void incrementTwice() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        increment();  // 内部再次加锁，不会死锁
        increment();
    }

    int get() const {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        return count_;
    }

private:
    mutable std::recursive_mutex mutex_;
    int count_ = 0;
};

int main() {
    // 递归加锁
    recursiveTask(3);

    // 成员函数互调
    SafeCounter counter;
    counter.incrementTwice();
    counter.increment();
    std::cout << "count=" << counter.get() << "\n";  // 应为 3

    // 多线程使用
    std::thread t1([&]() {
        for (int i = 0; i < 5; ++i) counter.incrementTwice();
    });
    std::thread t2([&]() {
        for (int i = 0; i < 5; ++i) counter.increment();
    });
    t1.join();
    t2.join();
    std::cout << "final count=" << counter.get() << "\n";  // 应为 3 + 10*2 + 5 = 28

    return 0;
}
