#include <iostream>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <list>
#include <string>

// ============================================================
// 时间轮（Time Wheel）实现
// 用于高效管理大量定时任务，O(1) 添加/取消定时器
// 应用：TCP 超时重传、心跳检测、延迟任务调度
//
// 原理：将时间划分为若干槽（slot），每个槽对应一个时间间隔
// 指针每 tick 移动一格，执行当前槽的所有任务
// 多层时间轮可表示更大范围的时间
// ============================================================

using TimerCallback = std::function<void()>;
using TimePoint     = std::chrono::steady_clock::time_point;

struct TimerTask {
    int             id;
    int             rounds;    // 还需绕几圈才执行（多圈时间轮）
    TimerCallback   cb;
    bool            cancelled;

    TimerTask(int i, int r, TimerCallback fn)
        : id(i), rounds(r), cb(std::move(fn)), cancelled(false) {}
};

// ============================================================
// 单层时间轮
// slots：槽数量（精度 = tick_ms，范围 = slots * tick_ms）
// ============================================================
class TimeWheel {
public:
    TimeWheel(int slots, int tick_ms)
        : slots_(slots), tick_ms_(tick_ms),
          wheel_(slots), current_(0), next_id_(1), running_(false) {}

    ~TimeWheel() { stop(); }

    // 添加定时任务，delay_ms 后触发，O(1)
    int add_timer(int delay_ms, TimerCallback cb) {
        std::lock_guard<std::mutex> lock(mu_);
        int ticks  = delay_ms / tick_ms_;
        int rounds = ticks / slots_;
        int slot   = (current_ + ticks % slots_) % slots_;

        int id = next_id_++;
        wheel_[slot].emplace_back(id, rounds, std::move(cb));
        std::cout << "  [add] timer-" << id
                  << " delay=" << delay_ms << "ms"
                  << " slot=" << slot << " rounds=" << rounds << "\n";
        return id;
    }

    // 取消定时任务，O(1)（标记取消，实际在 tick 时跳过）
    void cancel_timer(int id) {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto& slot : wheel_)
            for (auto& task : slot)
                if (task.id == id) { task.cancelled = true; return; }
    }

    // 手动推进一个 tick（测试用）
    void tick() {
        std::lock_guard<std::mutex> lock(mu_);
        do_tick();
    }

    // 启动后台线程自动推进
    void start() {
        running_ = true;
        worker_  = std::thread([this]() {
            while (running_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(tick_ms_));
                {
                    std::lock_guard<std::mutex> lock(mu_);
                    do_tick();
                }
            }
        });
    }

    void stop() {
        running_ = false;
        if (worker_.joinable()) worker_.join();
    }

    int current_slot() const { return current_; }

private:
    int                                      slots_, tick_ms_;
    std::vector<std::list<TimerTask>>        wheel_;
    int                                      current_;
    int                                      next_id_;
    std::mutex                               mu_;
    std::atomic<bool>                        running_;
    std::thread                              worker_;

    void do_tick() {
        auto& slot = wheel_[current_];
        auto  it   = slot.begin();
        while (it != slot.end()) {
            if (it->cancelled) {
                it = slot.erase(it);
                continue;
            }
            if (it->rounds > 0) {
                --it->rounds;  // 还需绕圈，本次跳过
                ++it;
            } else {
                it->cb();      // 执行回调
                it = slot.erase(it);
            }
        }
        current_ = (current_ + 1) % slots_;
    }
};

// ============================================================
// 分层时间轮（Hierarchical Time Wheel）
// 低精度轮 + 高精度轮，类似时钟的秒针/分针
// 这里用两层：ms 轮（256 槽，1ms/slot）+ s 轮（64 槽，256ms/slot）
// ============================================================
class HierarchicalTimeWheel {
public:
    HierarchicalTimeWheel()
        : ms_wheel_(256, 1), s_wheel_(64, 256) {}

    int add_timer(int delay_ms, TimerCallback cb) {
        if (delay_ms < 256)
            return ms_wheel_.add_timer(delay_ms, std::move(cb));
        else
            return s_wheel_.add_timer(delay_ms, std::move(cb));
    }

    void cancel_timer(int id) {
        ms_wheel_.cancel_timer(id);
        s_wheel_.cancel_timer(id);
    }

    void tick_ms() { ms_wheel_.tick(); }
    void tick_s()  { s_wheel_.tick(); }

private:
    TimeWheel ms_wheel_, s_wheel_;
};

// ==================== 入门：手动 tick 推进 ====================
void basic_demo() {
    std::cout << "=== 入门：时间轮手动推进 ===\n";

    TimeWheel tw(10, 100);  // 10 槽，每槽 100ms

    int t1 = tw.add_timer(200, []() { std::cout << "  timer-1 fired at 200ms!\n"; });
    int t2 = tw.add_timer(500, []() { std::cout << "  timer-2 fired at 500ms!\n"; });
    int t3 = tw.add_timer(300, []() { std::cout << "  timer-3 fired at 300ms!\n"; });
    (void)t1; (void)t2; (void)t3;

    std::cout << "simulating ticks (each = 100ms):\n";
    for (int i = 1; i <= 6; ++i) {
        std::cout << "tick " << i << " (" << i * 100 << "ms):\n";
        tw.tick();
    }
}

// ==================== 中级：取消定时器 ====================
void cancel_demo() {
    std::cout << "\n=== 中级：取消定时器 ===\n";

    TimeWheel tw(10, 100);

    tw.add_timer(200, []() { std::cout << "  timer-1 fired\n"; });
    int id2 = tw.add_timer(400, []() { std::cout << "  timer-2 fired (should NOT appear)\n"; });
    tw.add_timer(600, []() { std::cout << "  timer-3 fired\n"; });

    std::cout << "cancelling timer-" << id2 << "\n";
    tw.cancel_timer(id2);

    for (int i = 1; i <= 7; ++i) {
        std::cout << "tick " << i << ":\n";
        tw.tick();
    }
}

// ==================== 高级：后台线程自动触发 ====================
void auto_tick_demo() {
    std::cout << "\n=== 高级：后台线程自动触发 ===\n";

    TimeWheel tw(20, 50);  // 20 槽，每槽 50ms
    std::atomic<int> fired{0};

    tw.add_timer(100, [&]() { std::cout << "  heartbeat fired at ~100ms\n"; ++fired; });
    tw.add_timer(200, [&]() { std::cout << "  retry fired at ~200ms\n";    ++fired; });
    tw.add_timer(350, [&]() { std::cout << "  timeout fired at ~350ms\n";  ++fired; });

    tw.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    tw.stop();

    std::cout << "total fired=" << fired << " (expected 3)\n";
}

// ==================== 高级：模拟 TCP 超时重传 ====================
void tcp_retransmit_demo() {
    std::cout << "\n=== 高级：TCP 超时重传模拟 ===\n";

    TimeWheel tw(20, 50);
    std::atomic<int> retransmit_count{0};

    struct TcpSegment {
        int seq;
        int retries;
    };

    std::function<void(TcpSegment)> send_segment = [&](TcpSegment seg) {
        std::cout << "  send seq=" << seg.seq << " (retry=" << seg.retries << ")\n";
        if (seg.retries >= 3) {
            std::cout << "  seq=" << seg.seq << " max retries reached, drop\n";
            return;
        }
        // 设置超时重传定时器（RTO = 150ms）
        tw.add_timer(150, [&, seg]() mutable {
            ++retransmit_count;
            ++seg.retries;
            send_segment(seg);  // 递归重传
        });
    };

    tw.start();
    send_segment({1001, 0});
    send_segment({1002, 0});

    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    tw.stop();

    std::cout << "total retransmits=" << retransmit_count << "\n";
}

int main() {
    basic_demo();
    cancel_demo();
    auto_tick_demo();
    tcp_retransmit_demo();
    return 0;
}
