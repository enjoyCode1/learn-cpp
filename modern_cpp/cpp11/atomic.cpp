#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

// ==================== 入门级 ====================

// 基本原子操作演示
void basic_atomic_ops() {
    std::cout << "=== 入门：基本原子操作 ===\n";

    std::atomic<int> val{10};
    std::cout << "initial=" << val.load() << "\n";

    val.store(20);
    std::cout << "after store(20)=" << val.load() << "\n";

    int old = val.fetch_add(5);
    std::cout << "fetch_add(5): old=" << old << " new=" << val.load() << "\n";

    old = val.fetch_sub(3);
    std::cout << "fetch_sub(3): old=" << old << " new=" << val.load() << "\n";

    // compare_exchange_strong：期望值匹配才交换
    int expected = 22;
    bool ok = val.compare_exchange_strong(expected, 100);
    std::cout << "CAS(22->100): success=" << ok << " val=" << val.load() << "\n";

    // 期望值不匹配：失败并将 expected 更新为当前值
    expected = 999;
    ok = val.compare_exchange_strong(expected, 200);
    std::cout << "CAS(999->200): success=" << ok << " expected updated to=" << expected << "\n";
}

// 多线程原子计数
std::atomic<int> g_counter{0};

void atomic_counter_thread(int times) {
    for (int i = 0; i < times; ++i)
        ++g_counter;
}

void basic_atomic_counter() {
    std::cout << "\n=== 入门：多线程原子计数 ===\n";
    g_counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(atomic_counter_thread, 1000);
    for (auto& t : threads) t.join();
    std::cout << "4 threads x 1000: counter=" << g_counter << " (expected 4000)\n";
}

// ==================== 中级 ====================

// 自旋锁
class SpinLock {
public:
    void lock() {
        bool expected = false;
        // compare_exchange_weak 允许伪失败，搭配循环效率更高
        while (!flag_.compare_exchange_weak(expected, true,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed))
            expected = false;
    }
    void unlock() {
        flag_.store(false, std::memory_order_release);
    }
private:
    std::atomic<bool> flag_{false};
};

SpinLock g_spinLock;
int g_spinCounter = 0;

void spinlock_thread(int times) {
    for (int i = 0; i < times; ++i) {
        g_spinLock.lock();
        ++g_spinCounter;
        g_spinLock.unlock();
    }
}

void spinlock_demo() {
    std::cout << "\n=== 中级：自旋锁 ===\n";
    g_spinCounter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(spinlock_thread, 1000);
    for (auto& t : threads) t.join();
    std::cout << "spinlock counter=" << g_spinCounter << " (expected 4000)\n";
}

// 基于 atomic_flag 的互斥量（最轻量的原子类型，保证无锁）
class AtomicFlagMutex {
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire))
            ;
    }
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

AtomicFlagMutex g_flagMutex;
int g_flagCounter = 0;

void flag_mutex_thread(int times) {
    for (int i = 0; i < times; ++i) {
        g_flagMutex.lock();
        ++g_flagCounter;
        g_flagMutex.unlock();
    }
}

void atomic_flag_demo() {
    std::cout << "\n=== 中级：atomic_flag 互斥量 ===\n";
    g_flagCounter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.emplace_back(flag_mutex_thread, 1000);
    for (auto& t : threads) t.join();
    std::cout << "atomic_flag counter=" << g_flagCounter << " (expected 4000)\n";
}

// ==================== 高级：内存序 ====================

// memory_order_relaxed：只保证原子性，不保证顺序
void memory_order_relaxed_demo() {
    std::cout << "\n=== 高级：memory_order_relaxed ===\n";

    std::atomic<int> x{0}, y{0};

    // relaxed 只适合不依赖其他变量可见性的场景，如纯计数
    auto writer = [&]() {
        x.store(1, std::memory_order_relaxed);
        y.store(1, std::memory_order_relaxed);
    };
    auto reader = [&]() {
        // relaxed 下 y==1 不保证 x==1 已可见
        while (y.load(std::memory_order_relaxed) == 0)
            ;
        std::cout << "relaxed: y=1 observed, x=" << x.load(std::memory_order_relaxed)
                  << " (may be 0 on weakly-ordered arch)\n";
    };

    std::thread tw(writer);
    std::thread tr(reader);
    tw.join(); tr.join();
}

// memory_order_release / acquire：建立 happens-before 关系
// store(release) 之前的所有写操作，对 load(acquire) 之后的所有读操作可见
void memory_order_release_acquire_demo() {
    std::cout << "\n=== 高级：release/acquire 同步 ===\n";

    std::atomic<bool> ready{false};
    int data = 0;

    std::thread producer([&]() {
        data = 42;                                  // (1) 写数据
        ready.store(true, std::memory_order_release); // (2) 发布
        // (1) happens-before (2)
    });

    std::thread consumer([&]() {
        while (!ready.load(std::memory_order_acquire)) // (3) 等待发布
            ;
        // (2) synchronizes-with (3) => (1) happens-before 此处
        std::cout << "release/acquire: data=" << data << " (expected 42)\n";
    });

    producer.join();
    consumer.join();
}

// memory_order_seq_cst：全序，最强，也是默认值
// 所有线程观察到的 seq_cst 操作顺序一致
void memory_order_seq_cst_demo() {
    std::cout << "\n=== 高级：seq_cst 全序 ===\n";

    std::atomic<bool> x{false}, y{false};
    std::atomic<int> z{0};

    std::thread t1([&]() { x.store(true, std::memory_order_seq_cst); });
    std::thread t2([&]() { y.store(true, std::memory_order_seq_cst); });
    std::thread t3([&]() {
        while (!x.load(std::memory_order_seq_cst)) ;
        if (y.load(std::memory_order_seq_cst)) ++z;
    });
    std::thread t4([&]() {
        while (!y.load(std::memory_order_seq_cst)) ;
        if (x.load(std::memory_order_seq_cst)) ++z;
    });

    t1.join(); t2.join(); t3.join(); t4.join();
    // seq_cst 保证 t3/t4 至少有一个能看到两个 store，z >= 1
    std::cout << "seq_cst: z=" << z << " (expected >= 1)\n";
}

// memory_order_acq_rel：用于 read-modify-write，兼具 acquire 和 release 语义
void memory_order_acq_rel_demo() {
    std::cout << "\n=== 高级：acq_rel (fetch_add RMW) ===\n";

    std::atomic<int> counter{0};
    int snapshot = -1;

    std::thread t1([&]() {
        // fetch_add 带 acq_rel：读取旧值(acquire) + 写入新值(release)
        counter.fetch_add(1, std::memory_order_acq_rel);
    });
    std::thread t2([&]() {
        counter.fetch_add(1, std::memory_order_acq_rel);
    });
    std::thread t3([&]() {
        // 等两个线程都加完后读取
        while (counter.load(std::memory_order_acquire) < 2) ;
        snapshot = counter.load(std::memory_order_relaxed);
    });

    t1.join(); t2.join(); t3.join();
    std::cout << "acq_rel fetch_add: counter=" << snapshot << " (expected 2)\n";
}

// ==================== 高级：无锁队列 ====================

// 单生产者单消费者无锁队列（SPSC），基于循环缓冲区
// 利用 release/acquire 内存序保证可见性，无需任何锁
template<typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
public:
    bool push(const T& val) {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t next = (head + 1) & mask_;
        if (next == tail_.load(std::memory_order_acquire))
            return false; // 队列满
        buf_[head] = val;
        head_.store(next, std::memory_order_release); // 发布新元素
        return true;
    }

    bool pop(T& val) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        if (tail == head_.load(std::memory_order_acquire))
            return false; // 队列空
        val = buf_[tail];
        tail_.store((tail + 1) & mask_, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

private:
    static constexpr size_t mask_ = Capacity - 1;
    T buf_[Capacity];
    std::atomic<size_t> head_{0};
    std::atomic<size_t> tail_{0};
};

void spsc_queue_demo() {
    std::cout << "\n=== 高级：SPSC 无锁队列 ===\n";

    SPSCQueue<int, 1024> q;
    constexpr int N = 100000;
    std::atomic<long long> sum_produced{0}, sum_consumed{0};

    std::thread producer([&]() {
        for (int i = 1; i <= N; ++i) {
            while (!q.push(i)) ; // 队列满时自旋等待
            sum_produced.fetch_add(i, std::memory_order_relaxed);
        }
    });

    std::thread consumer([&]() {
        int val;
        int count = 0;
        while (count < N) {
            if (q.pop(val)) {
                sum_consumed.fetch_add(val, std::memory_order_relaxed);
                ++count;
            }
        }
    });

    producer.join();
    consumer.join();

    long long expected = (long long)N * (N + 1) / 2;
    std::cout << "produced sum=" << sum_produced << "\n";
    std::cout << "consumed sum=" << sum_consumed << "\n";
    std::cout << "match=" << (sum_produced == sum_consumed && sum_consumed == expected) << "\n";
}

// 多生产者多消费者无锁队列（MPMC），基于 compare_exchange
template<typename T, size_t Capacity>
class MPMCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");

    struct Slot {
        std::atomic<size_t> seq;
        T data;
    };

public:
    MPMCQueue() {
        for (size_t i = 0; i < Capacity; ++i)
            slots_[i].seq.store(i, std::memory_order_relaxed);
        head_.store(0, std::memory_order_relaxed);
        tail_.store(0, std::memory_order_relaxed);
    }

    bool push(const T& val) {
        size_t head = head_.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = slots_[head & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);
            intptr_t diff = (intptr_t)seq - (intptr_t)head;
            if (diff == 0) {
                if (head_.compare_exchange_weak(head, head + 1, std::memory_order_relaxed))
                    break;
            } else if (diff < 0) {
                return false; // 队列满
            } else {
                head = head_.load(std::memory_order_relaxed);
            }
        }
        Slot& slot = slots_[head & mask_];
        slot.data = val;
        slot.seq.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& val) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = slots_[tail & mask_];
            size_t seq = slot.seq.load(std::memory_order_acquire);
            intptr_t diff = (intptr_t)seq - (intptr_t)(tail + 1);
            if (diff == 0) {
                if (tail_.compare_exchange_weak(tail, tail + 1, std::memory_order_relaxed))
                    break;
            } else if (diff < 0) {
                return false; // 队列空
            } else {
                tail = tail_.load(std::memory_order_relaxed);
            }
        }
        Slot& slot = slots_[tail & mask_];
        val = slot.data;
        slot.seq.store(tail + Capacity, std::memory_order_release);
        return true;
    }

private:
    static constexpr size_t mask_ = Capacity - 1;
    Slot slots_[Capacity];
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

void mpmc_queue_demo() {
    std::cout << "\n=== 高级：MPMC 无锁队列 ===\n";

    MPMCQueue<int, 1024> q;
    constexpr int N = 10000;
    constexpr int PRODUCERS = 4;
    constexpr int CONSUMERS = 4;

    std::atomic<long long> sum_produced{0}, sum_consumed{0};
    std::atomic<int> produced_count{0};

    auto producer_fn = [&](int id) {
        for (int i = id; i <= N; i += PRODUCERS) {
            while (!q.push(i)) ;
            sum_produced.fetch_add(i, std::memory_order_relaxed);
            produced_count.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto consumer_fn = [&]() {
        int val;
        while (true) {
            if (q.pop(val)) {
                sum_consumed.fetch_add(val, std::memory_order_relaxed);
            } else if (produced_count.load(std::memory_order_acquire) == N &&
                       sum_consumed.load(std::memory_order_relaxed) ==
                       sum_produced.load(std::memory_order_relaxed)) {
                break;
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 1; i <= PRODUCERS; ++i)
        threads.emplace_back(producer_fn, i);
    for (int i = 0; i < CONSUMERS; ++i)
        threads.emplace_back(consumer_fn);
    for (auto& t : threads) t.join();

    std::cout << "4P4C produced sum=" << sum_produced << "\n";
    std::cout << "4P4C consumed sum=" << sum_consumed << "\n";
    std::cout << "match=" << (sum_produced == sum_consumed) << "\n";
}

int main() {
    // 入门
    basic_atomic_ops();
    basic_atomic_counter();

    // 中级
    spinlock_demo();
    atomic_flag_demo();

    // 高级：内存序
    memory_order_relaxed_demo();
    memory_order_release_acquire_demo();
    memory_order_seq_cst_demo();
    memory_order_acq_rel_demo();

    // 高级：无锁队列
    spsc_queue_demo();
    mpmc_queue_demo();

    return 0;
}
