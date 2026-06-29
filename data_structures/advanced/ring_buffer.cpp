#include <iostream>
#include <vector>
#include <atomic>
#include <thread>
#include <stdexcept>
#include <string>

// ============================================================
// 环形缓冲区（Ring Buffer / Circular Buffer）
// 固定大小的 FIFO 队列，无需动态分配内存
// 适合生产者-消费者场景，避免内存碎片
// ============================================================

// 单线程版本
template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t cap)
        : buf_(cap + 1), cap_(cap + 1), head_(0), tail_(0) {}

    // 入队，O(1)
    bool push(const T& val) {
        if (full()) return false;
        buf_[tail_] = val;
        tail_ = (tail_ + 1) % cap_;
        return true;
    }

    // 出队，O(1)
    bool pop(T& val) {
        if (empty()) return false;
        val  = buf_[head_];
        head_ = (head_ + 1) % cap_;
        return true;
    }

    T& front() {
        if (empty()) throw std::underflow_error("RingBuffer empty");
        return buf_[head_];
    }

    size_t size()  const { return (tail_ + cap_ - head_) % cap_; }
    bool   empty() const { return head_ == tail_; }
    bool   full()  const { return (tail_ + 1) % cap_ == head_; }
    size_t capacity() const { return cap_ - 1; }

    void print() const {
        std::cout << "RingBuffer [";
        size_t i = head_, cnt = size();
        for (size_t k = 0; k < cnt; ++k) {
            std::cout << buf_[(i + k) % cap_];
            if (k + 1 < cnt) std::cout << ", ";
        }
        std::cout << "] size=" << cnt << "/" << (cap_-1) << "\n";
    }

private:
    std::vector<T> buf_;
    size_t         cap_, head_, tail_;
};

// ============================================================
// 无锁环形缓冲区（Lock-Free Ring Buffer）
// SPSC（单生产者单消费者）线程安全版本
// 使用 atomic 保证可见性，无需互斥锁
// ============================================================

template<typename T>
class SPSCRingBuffer {
public:
    explicit SPSCRingBuffer(size_t cap)
        : buf_(cap + 1), cap_(cap + 1),
          head_(0), tail_(0) {}

    // 生产者调用，O(1) 无锁
    bool push(const T& val) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t next = (tail + 1) % cap_;
        // 检查是否已满：需要读 head，用 acquire 保证看到消费者的更新
        if (next == head_.load(std::memory_order_acquire)) return false;
        buf_[tail] = val;
        // release：确保写入 buf_ 对消费者可见
        tail_.store(next, std::memory_order_release);
        return true;
    }

    // 消费者调用，O(1) 无锁
    bool pop(T& val) {
        size_t head = head_.load(std::memory_order_relaxed);
        // acquire：确保看到生产者写入的 buf_
        if (head == tail_.load(std::memory_order_acquire)) return false;
        val = buf_[head];
        // release：告知生产者该槽位已释放
        head_.store((head + 1) % cap_, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) ==
               tail_.load(std::memory_order_acquire);
    }

    size_t capacity() const { return cap_ - 1; }

private:
    std::vector<T>      buf_;
    size_t              cap_;
    std::atomic<size_t> head_, tail_;
};

// ==================== 入门：单线程基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：RingBuffer 基础操作 ===\n";

    RingBuffer<int> rb(5);
    std::cout << "capacity=" << rb.capacity() << "\n";

    for (int i = 1; i <= 5; ++i) {
        bool ok = rb.push(i);
        std::cout << "push(" << i << ")=" << std::boolalpha << ok << " ";
        rb.print();
    }

    // 已满，再入队失败
    std::cout << "push(6) when full: " << rb.push(6) << "\n";

    // 出队
    int val;
    rb.pop(val); std::cout << "pop=" << val << " "; rb.print();
    rb.pop(val); std::cout << "pop=" << val << " "; rb.print();

    // 出队后再入队（循环利用空间）
    rb.push(10); rb.push(11);
    std::cout << "after push 10,11: "; rb.print();
}

// ==================== 中级：循环覆盖（日志缓冲）====================
// 固定大小的日志缓冲区，旧日志被新日志覆盖
template<typename T>
class OverwriteRingBuffer {
public:
    explicit OverwriteRingBuffer(size_t cap)
        : buf_(cap), cap_(cap), head_(0), tail_(0), full_(false) {}

    void push(const T& val) {
        buf_[tail_] = val;
        tail_ = (tail_ + 1) % cap_;
        if (full_) head_ = (head_ + 1) % cap_;  // 覆盖最旧的
        full_ = (tail_ == head_);
    }

    std::vector<T> snapshot() const {
        std::vector<T> res;
        if (!full_ && head_ == tail_) return res;
        size_t i = head_;
        do {
            res.push_back(buf_[i]);
            i = (i + 1) % cap_;
        } while (i != tail_ || (full_ && res.size() < cap_));
        // 修正：full_ 时需特殊处理
        return res;
    }

    size_t size() const {
        if (full_) return cap_;
        return (tail_ + cap_ - head_) % cap_;
    }

private:
    std::vector<T> buf_;
    size_t cap_, head_, tail_;
    bool   full_;
};

void log_buffer_demo() {
    std::cout << "\n=== 中级：覆盖式日志缓冲区 ===\n";

    RingBuffer<std::string> log_buf(3);
    for (int i = 1; i <= 3; ++i)
        log_buf.push("log-" + std::to_string(i));

    log_buf.print();

    // 缓冲区满后，先出队再入新日志（模拟循环）
    std::string tmp;
    for (int i = 4; i <= 6; ++i) {
        if (log_buf.full()) log_buf.pop(tmp);  // 丢弃最旧
        log_buf.push("log-" + std::to_string(i));
    }
    log_buf.print();
}

// ==================== 高级：SPSC 无锁生产者-消费者 ====================
void spsc_demo() {
    std::cout << "\n=== 高级：SPSC 无锁环形缓冲区 ===\n";

    SPSCRingBuffer<int> rb(64);
    std::atomic<int> total_produced{0}, total_consumed{0};
    const int N = 100;

    std::thread producer([&]() {
        for (int i = 0; i < N; ++i) {
            while (!rb.push(i)) {}  // 自旋等待空间
            total_produced.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::thread consumer([&]() {
        int val, cnt = 0;
        while (cnt < N) {
            if (rb.pop(val)) {
                ++cnt;
                total_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        }
    });

    producer.join();
    consumer.join();

    std::cout << "produced=" << total_produced
              << " consumed=" << total_consumed << "\n";
    std::cout << "buffer empty after test: " << std::boolalpha << rb.empty() << "\n";
}

int main() {
    basic_demo();
    log_buffer_demo();
    spsc_demo();
    return 0;
}
