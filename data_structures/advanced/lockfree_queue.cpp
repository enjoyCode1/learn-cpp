#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>

// ============================================================
// 无锁队列（Lock-Free Queue）实现
// 1. SPSC（单生产者单消费者）：基于环形缓冲区，最简单高效
// 2. MPMC（多生产者多消费者）：基于 Michael-Scott 算法（链表 + CAS）
// ============================================================

// ==================== SPSC 无锁队列 ====================
template<typename T>
class SPSCQueue {
public:
    explicit SPSCQueue(size_t cap)
        : buf_(cap + 1), cap_(cap + 1), head_(0), tail_(0) {}

    bool push(T val) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next = (tail + 1) % cap_;
        if (next == head_.load(std::memory_order_acquire)) return false;  // 满
        buf_[tail] = std::move(val);
        tail_.store(next, std::memory_order_release);
        return true;
    }

    std::optional<T> pop() {
        size_t head = head_.load(std::memory_order_relaxed);
        if (head == tail_.load(std::memory_order_acquire)) return std::nullopt;  // 空
        T val = std::move(buf_[head]);
        head_.store((head + 1) % cap_, std::memory_order_release);
        return val;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

private:
    std::vector<T>      buf_;
    size_t              cap_;
    std::atomic<size_t> head_, tail_;
};

// ==================== MPMC 无锁队列（Michael-Scott 算法）====================
// 基于链表 + CAS，支持多生产者多消费者
template<typename T>
class MPMCQueue {
    struct Node {
        std::atomic<Node*> next;
        T                  val;
        Node() : next(nullptr), val{} {}
        explicit Node(T v) : next(nullptr), val(std::move(v)) {}
    };

public:
    MPMCQueue() {
        // 哨兵节点：head 和 tail 都指向它
        Node* dummy = new Node();
        head_.store(dummy, std::memory_order_relaxed);
        tail_.store(dummy, std::memory_order_relaxed);
    }

    ~MPMCQueue() {
        // 清理所有节点
        Node* cur = head_.load(std::memory_order_relaxed);
        while (cur) {
            Node* next = cur->next.load(std::memory_order_relaxed);
            delete cur;
            cur = next;
        }
    }

    // 入队，O(1) 均摊，无锁
    void push(T val) {
        Node* node = new Node(std::move(val));
        Node* prev_tail;
        while (true) {
            prev_tail = tail_.load(std::memory_order_acquire);
            Node* next = prev_tail->next.load(std::memory_order_acquire);
            // 确认 tail 仍然是尾节点
            if (prev_tail != tail_.load(std::memory_order_acquire)) continue;
            if (next == nullptr) {
                // CAS 将 node 接到链表尾部
                if (prev_tail->next.compare_exchange_weak(
                        next, node,
                        std::memory_order_release,
                        std::memory_order_relaxed))
                    break;
            } else {
                // tail 落后了，帮助推进
                tail_.compare_exchange_weak(prev_tail, next,
                    std::memory_order_release, std::memory_order_relaxed);
            }
        }
        // 推进 tail（可能失败，下一个线程会帮忙）
        tail_.compare_exchange_weak(prev_tail, node,
            std::memory_order_release, std::memory_order_relaxed);
    }

    // 出队，O(1) 均摊，无锁
    std::optional<T> pop() {
        while (true) {
            Node* head = head_.load(std::memory_order_acquire);
            Node* tail = tail_.load(std::memory_order_acquire);
            Node* next = head->next.load(std::memory_order_acquire);

            if (head != head_.load(std::memory_order_acquire)) continue;
            if (head == tail) {
                if (next == nullptr) return std::nullopt;  // 队列为空
                // tail 落后了，帮助推进
                tail_.compare_exchange_weak(tail, next,
                    std::memory_order_release, std::memory_order_relaxed);
            } else {
                T val = next->val;
                if (head_.compare_exchange_weak(head, next,
                        std::memory_order_release, std::memory_order_relaxed)) {
                    delete head;  // 删除旧哨兵
                    return val;
                }
            }
        }
    }

    bool empty() const {
        Node* head = head_.load(std::memory_order_acquire);
        Node* tail = tail_.load(std::memory_order_acquire);
        return head == tail &&
               head->next.load(std::memory_order_acquire) == nullptr;
    }

private:
    std::atomic<Node*> head_, tail_;
};

// ==================== 入门：SPSC 基础操作 ====================
void spsc_basic_demo() {
    std::cout << "=== 入门：SPSC 无锁队列 ===\n";

    SPSCQueue<int> q(8);
    for (int i = 1; i <= 5; ++i) q.push(i);

    while (auto v = q.pop())
        std::cout << *v << " ";
    std::cout << "\n";
    std::cout << "empty: " << std::boolalpha << q.empty() << "\n";
}

// ==================== 中级：SPSC 单生产者单消费者压测 ====================
void spsc_throughput_demo() {
    std::cout << "\n=== 中级：SPSC 吞吐量测试 ===\n";

    const int N = 1'000'000;
    SPSCQueue<int> q(1024);
    std::atomic<long long> sum{0};

    auto t0 = std::chrono::high_resolution_clock::now();

    std::thread producer([&]() {
        for (int i = 0; i < N; ++i)
            while (!q.push(i)) {}
    });

    std::thread consumer([&]() {
        long long s = 0;
        int cnt = 0;
        while (cnt < N) {
            if (auto v = q.pop()) { s += *v; ++cnt; }
        }
        sum.store(s, std::memory_order_relaxed);
    });

    producer.join();
    consumer.join();

    auto t1 = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "N=" << N << " sum=" << sum
              << " time=" << ms << "ms"
              << " throughput=" << (N * 1000LL / std::max(ms, 1LL)) << " ops/s\n";
}

// ==================== 高级：MPMC 多生产者多消费者 ====================
void mpmc_demo() {
    std::cout << "\n=== 高级：MPMC 无锁队列 ===\n";

    const int N         = 100'000;
    const int PRODUCERS = 4;
    const int CONSUMERS = 4;
    const int PER_PROD  = N / PRODUCERS;

    MPMCQueue<int> q;
    std::atomic<long long> total{0};
    std::atomic<int>       produced{0}, consumed{0};

    std::vector<std::thread> threads;

    // 启动生产者
    for (int p = 0; p < PRODUCERS; ++p) {
        threads.emplace_back([&, p]() {
            int start = p * PER_PROD;
            for (int i = start; i < start + PER_PROD; ++i) {
                q.push(i);
                produced.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // 启动消费者
    std::atomic<bool> done{false};
    for (int c = 0; c < CONSUMERS; ++c) {
        threads.emplace_back([&]() {
            long long local_sum = 0;
            while (true) {
                auto v = q.pop();
                if (v) {
                    local_sum += *v;
                    int cnt = consumed.fetch_add(1, std::memory_order_relaxed) + 1;
                    if (cnt >= N) {
                        total.fetch_add(local_sum, std::memory_order_relaxed);
                        return;
                    }
                } else if (done.load(std::memory_order_acquire) &&
                           consumed.load(std::memory_order_relaxed) >= N) {
                    total.fetch_add(local_sum, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }

    // 等待所有生产者结束，通知消费者
    for (int i = 0; i < PRODUCERS; ++i) threads[i].join();
    done.store(true, std::memory_order_release);
    for (int i = PRODUCERS; i < PRODUCERS + CONSUMERS; ++i) threads[i].join();

    long long expected = (long long)(N - 1) * N / 2;
    std::cout << "produced=" << produced << " consumed=" << consumed << "\n";
    std::cout << "sum=" << total << " expected=" << expected
              << " correct=" << std::boolalpha << (total == expected) << "\n";
}

int main() {
    spsc_basic_demo();
    spsc_throughput_demo();
    mpmc_demo();
    return 0;
}
