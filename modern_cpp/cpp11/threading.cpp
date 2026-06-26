#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <future>
#include <functional>
#include <atomic>

// ==================== 入门级 ====================

std::mutex g_mutex;
int g_counter = 0;

void incrementWithLock(int times) {
    for (int i = 0; i < times; ++i) {
        std::lock_guard<std::mutex> lock(g_mutex);
        ++g_counter;
    }
}

// unique_lock 更灵活，支持延迟加锁、手动解锁
void incrementWithUniqueLock(int times) {
    for (int i = 0; i < times; ++i) {
        std::unique_lock<std::mutex> lock(g_mutex);
        ++g_counter;
        lock.unlock();  // 提前解锁
        // 做其他不需要锁的事
    }
}

void worker(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock(g_mutex);
    std::cout << "worker " << id << " done\n";
}

void basic_threading() {
    std::cout << "\n=== 入门级：基本线程 ===\n";

    // 基本线程创建
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);

    t1.join();
    t2.join();
    t3.join();

    // 多线程竞争计数器
    g_counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(incrementWithLock, 1000);
    }
    for (auto& t : threads) t.join();
    std::cout << "counter (lock_guard)=" << g_counter << "\n";  // 应为 4000

    // unique_lock 版本
    g_counter = 0;
    threads.clear();
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(incrementWithUniqueLock, 1000);
    }
    for (auto& t : threads) t.join();
    std::cout << "counter (unique_lock)=" << g_counter << "\n";  // 应为 4000

    // 线程 id 和 sleep
    std::cout << "main thread id=" << std::this_thread::get_id() << "\n";
}

// ==================== 中级 ====================

// 条件变量：生产者-消费者模式
std::queue<int> g_queue;
std::mutex g_queue_mutex;
std::condition_variable g_cv;
const int MAX_QUEUE_SIZE = 5;

void producer(int id, int count) {
    for (int i = 0; i < count; ++i) {
        std::unique_lock<std::mutex> lock(g_queue_mutex);

        // 等待队列有空间
        g_cv.wait(lock, []{ return g_queue.size() < MAX_QUEUE_SIZE; });

        g_queue.push(i);
        std::cout << "Producer " << id << " produced: " << i << " (queue size: " << g_queue.size() << ")\n";

        lock.unlock();
        g_cv.notify_all();  // 通知消费者

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(int id, int count) {
    for (int i = 0; i < count; ++i) {
        std::unique_lock<std::mutex> lock(g_queue_mutex);

        // 等待队列有数据
        g_cv.wait(lock, []{ return !g_queue.empty(); });

        int value = g_queue.front();
        g_queue.pop();
        std::cout << "Consumer " << id << " consumed: " << value << " (queue size: " << g_queue.size() << ")\n";

        lock.unlock();
        g_cv.notify_all();  // 通知生产者

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

void producer_consumer_demo() {
    std::cout << "\n=== 中级：生产者-消费者 ===\n";

    std::thread p1(producer, 1, 10);
    std::thread c1(consumer, 1, 5);
    std::thread c2(consumer, 2, 5);

    p1.join();
    c1.join();
    c2.join();
}

// std::async：异步任务
int expensiveComputation(int x) {
    std::cout << "Computing " << x << "...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

void async_demo() {
    std::cout << "\n=== 中级：std::async异步任务 ===\n";

    // 异步执行
    std::future<int> future1 = std::async(std::launch::async, expensiveComputation, 5);
    std::future<int> future2 = std::async(std::launch::async, expensiveComputation, 10);

    std::cout << "Doing other work while computing...\n";

    // 获取结果（会阻塞直到完成）
    int result1 = future1.get();
    int result2 = future2.get();

    std::cout << "Results: " << result1 << ", " << result2 << "\n";
}

// std::promise和std::future：线程间通信
void promiseProducer(std::promise<int>&& promise) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Producer setting value...\n";
    promise.set_value(42);
}

void promise_future_demo() {
    std::cout << "\n=== 中级：promise和future ===\n";

    std::promise<int> promise;
    std::future<int> future = promise.get_future();

    std::thread t(promiseProducer, std::move(promise));

    std::cout << "Waiting for value...\n";
    int value = future.get();
    std::cout << "Received value: " << value << "\n";

    t.join();
}

// 多锁同时获取：避免死锁
std::mutex mutex1, mutex2;

void safeTransfer() {
    // 同时锁定多个互斥量，避免死锁
    std::lock(mutex1, mutex2);
    std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);

    std::cout << "Safely locked both mutexes\n";
}

void multiple_locks_demo() {
    std::cout << "\n=== 中级：多锁获取 ===\n";

    std::thread t1(safeTransfer);
    std::thread t2(safeTransfer);

    t1.join();
    t2.join();
}

// ==================== 高级 ====================

// 线程池（简化版）
class ThreadPool {
public:
    ThreadPool(size_t threads) : stop_(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        condition_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });

                        if (stop_ && tasks_.empty()) return;

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(f));
        }
        condition_.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (std::thread& worker : workers_) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

void threadpool_demo() {
    std::cout << "\n=== 高级：线程池 ===\n";

    ThreadPool pool(4);

    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " running on thread "
                      << std::this_thread::get_id() << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

// 读写锁模拟（C++11没有shared_mutex，手动实现）
class RWLock {
public:
    void lock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return writers_ == 0; });
        ++readers_;
    }

    void unlock_read() {
        std::unique_lock<std::mutex> lock(mutex_);
        --readers_;
        if (readers_ == 0) {
            cv_.notify_all();
        }
    }

    void lock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]{ return readers_ == 0 && writers_ == 0; });
        ++writers_;
    }

    void unlock_write() {
        std::unique_lock<std::mutex> lock(mutex_);
        --writers_;
        cv_.notify_all();
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    int readers_ = 0;
    int writers_ = 0;
};

RWLock g_rwlock;
int g_shared_data = 0;

void reader(int id) {
    g_rwlock.lock_read();
    std::cout << "Reader " << id << " reads: " << g_shared_data << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    g_rwlock.unlock_read();
}

void writer(int id, int value) {
    g_rwlock.lock_write();
    g_shared_data = value;
    std::cout << "Writer " << id << " writes: " << value << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    g_rwlock.unlock_write();
}

void rwlock_demo() {
    std::cout << "\n=== 高级：读写锁 ===\n";

    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(reader, i);
    }
    threads.emplace_back(writer, 1, 100);
    for (int i = 5; i < 10; ++i) {
        threads.emplace_back(reader, i);
    }

    for (auto& t : threads) {
        t.join();
    }
}

// 线程局部存储
thread_local int t_local_value = 0;

void threadLocalWorker(int id) {
    t_local_value = id * 100;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "Thread " << id << " local value: " << t_local_value << "\n";
}

void thread_local_demo() {
    std::cout << "\n=== 高级：线程局部存储 ===\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(threadLocalWorker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
}

// 原子操作与无锁编程
std::atomic<int> g_atomic_counter(0);

void atomicIncrement(int times) {
    for (int i = 0; i < times; ++i) {
        g_atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void atomic_demo() {
    std::cout << "\n=== 高级：原子操作 ===\n";

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(atomicIncrement, 10000);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Atomic counter: " << g_atomic_counter.load() << "\n";
}

// 使用std::packaged_task
void packaged_task_demo() {
    std::cout << "\n=== 高级：packaged_task ===\n";

    std::packaged_task<int(int)> task([](int x) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return x * x;
    });

    std::future<int> future = task.get_future();

    std::thread t(std::move(task), 7);

    std::cout << "Waiting for result...\n";
    int result = future.get();
    std::cout << "Result: " << result << "\n";

    t.join();
}

int main() {
    // 入门级
    basic_threading();

    // 中级
    producer_consumer_demo();
    async_demo();
    promise_future_demo();
    multiple_locks_demo();

    // 高级
    threadpool_demo();
    rwlock_demo();
    thread_local_demo();
    atomic_demo();
    packaged_task_demo();

    return 0;
}
