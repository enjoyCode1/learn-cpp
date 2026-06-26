#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

// ============================================================
// C++ 内存序完整指南
//
// CPU 和编译器为了性能会对指令重排，内存序用来约束这种重排。
// 六种内存序从弱到强：
//   relaxed < consume < acquire/release < acq_rel < seq_cst
//
// 核心概念：
//   happens-before  A happens-before B，则 A 的副作用对 B 可见
//   synchronizes-with  release-store 与 acquire-load 配对后建立此关系
//   modification order  单个原子变量上所有写操作的全局顺序
// ============================================================


// ============================================================
// 1. memory_order_relaxed
//    - 只保证操作本身的原子性（不撕裂）
//    - 不建立任何 happens-before，不阻止编译器/CPU 重排
//    - 适合：纯计数、统计，不依赖其他变量的可见性
// ============================================================

void relaxed_counter_demo() {
    std::cout << "=== 1. relaxed：纯计数（正确用法）===\n";

    std::atomic<int> counter{0};
    std::vector<std::thread> threads;

    // 多线程累加：只需要原子性，不关心顺序 => relaxed 足够且最快
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 10000; ++j)
                counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    for (auto& t : threads) t.join();
    std::cout << "counter=" << counter << " (expected 40000)\n";
}

void relaxed_wrong_usage_demo() {
    std::cout << "\n=== 1. relaxed：错误用法示范（用 release/acquire 修复）===\n";

    std::atomic<int> x{0}, y{0};

    // writer：先写 x，再写 y，希望 reader 看到 y=1 时 x 也是 1
    auto writer = [&]() {
        x.store(1, std::memory_order_relaxed);  // (A)
        y.store(1, std::memory_order_relaxed);  // (B)
        // 问题：relaxed 允许 CPU/编译器将 B 重排到 A 前面
        // ARM/Power 架构上 reader 可能看到 y=1 但 x=0
    };

    auto reader = [&]() {
        while (y.load(std::memory_order_relaxed) == 0) ;  // (C)
        // relaxed：C 看到 y=1 不能推导出 A 已对本线程可见
        int xval = x.load(std::memory_order_relaxed);     // (D)
        std::cout << "  relaxed: y=1, x=" << xval
                  << " (x86 always 1, ARM may be 0)\n";
    };

    std::thread tw(writer);
    std::thread tr(reader);
    tw.join(); tr.join();

    // 修复：改用 release/acquire（见下一节）
}


// ============================================================
// 2. memory_order_release / memory_order_acquire
//    - release 用于 store：本操作之前的所有读写不能被重排到 store 之后
//    - acquire 用于 load：本操作之后的所有读写不能被重排到 load 之前
//    - 配对规则：acquire-load 读到 release-store 写的值时，
//      建立 synchronizes-with 关系，进而建立 happens-before
//    - 适合：生产者-消费者、标志位发布、指针发布
// ============================================================

void release_acquire_basic_demo() {
    std::cout << "\n=== 2. release/acquire：基础发布-订阅 ===\n";

    std::atomic<bool> ready{false};
    int data = 0;

    std::thread producer([&]() {
        data = 42;                                    // (1) 写数据
        ready.store(true, std::memory_order_release); // (2) 发布标志
        // release 保证：(1) 不会被重排到 (2) 之后
        // 即所有在 (2) 之前的写操作，对订阅者均可见
    });

    std::thread consumer([&]() {
        while (!ready.load(std::memory_order_acquire)) ;  // (3) 订阅标志
        // acquire 保证：(3) 之后的读操作不会被重排到 (3) 之前
        // (2) synchronizes-with (3) => (1) happens-before (4)
        int val = data;                               // (4) 读数据，保证读到 42
        std::cout << "  data=" << val << " (expected 42)\n";
    });

    producer.join();
    consumer.join();
}

void release_acquire_pointer_publish_demo() {
    std::cout << "\n=== 2. release/acquire：指针发布（构造完成后发布）===\n";

    struct Config {
        int timeout;
        int retry;
        std::string host;
        Config(int t, int r, std::string h)
            : timeout(t), retry(r), host(std::move(h)) {}
    };

    std::atomic<Config*> g_config{nullptr};

    std::thread publisher([&]() {
        auto* cfg = new Config(30, 3, "example.com");
        // release：保证 Config 对象完整构造后再发布指针
        g_config.store(cfg, std::memory_order_release);
    });

    std::thread reader([&]() {
        Config* cfg = nullptr;
        // acquire：保证拿到指针后，Config 内部数据也已可见
        while (!(cfg = g_config.load(std::memory_order_acquire))) ;
        std::cout << "  host=" << cfg->host
                  << " timeout=" << cfg->timeout << "\n";
        delete cfg;
    });

    publisher.join();
    reader.join();
}

// release-sequence：多个 acquire 都能同步到同一个 release
void release_sequence_demo() {
    std::cout << "\n=== 2. release/acquire：release sequence（多消费者）===\n";

    std::atomic<int> flag{0};
    int data1 = 0, data2 = 0;

    std::thread producer([&]() {
        data1 = 100;
        data2 = 200;
        flag.store(1, std::memory_order_release);  // release
    });

    // 两个消费者都 acquire 同一个 flag，都能看到 data1/data2
    std::thread c1([&]() {
        while (flag.load(std::memory_order_acquire) == 0) ;
        std::cout << "  c1: data1=" << data1 << " data2=" << data2 << "\n";
    });
    std::thread c2([&]() {
        while (flag.load(std::memory_order_acquire) == 0) ;
        std::cout << "  c2: data1=" << data1 << " data2=" << data2 << "\n";
    });

    producer.join(); c1.join(); c2.join();
}


// ============================================================
// 3. memory_order_acq_rel
//    - 专用于 read-modify-write（RMW）操作：fetch_add/fetch_sub/
//      compare_exchange 等
//    - 兼具 acquire（读取部分）和 release（写入部分）语义
//    - 适合：无锁数据结构中的中间节点操作、引用计数
// ============================================================

void acq_rel_reference_count_demo() {
    std::cout << "\n=== 3. acq_rel：引用计数 ===\n";

    struct SharedData {
        std::atomic<int> ref_count{1};
        int value;
        explicit SharedData(int v) : value(v) {}
    };

    auto* obj = new SharedData(42);

    auto add_ref = [](SharedData* p) {
        // acquire：确保读到最新引用计数
        p->ref_count.fetch_add(1, std::memory_order_relaxed);
        // 增加引用不需要 release，不发布任何数据
    };

    auto release_ref = [](SharedData* p) {
        // acq_rel：
        //   release 部分：本次 fetch_sub 之前的所有操作对其他线程可见
        //   acquire 部分：若 fetch_sub 返回 1（即将变 0），
        //                 acquire 确保其他线程的 release 写入已可见
        if (p->ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // 最后一个引用，安全删除
            std::cout << "  deleting object, value=" << p->value << "\n";
            delete p;
        }
    };

    // 模拟多线程共享
    add_ref(obj);  // ref=2
    add_ref(obj);  // ref=3

    std::thread t1([&]() { release_ref(obj); });  // ref=2
    std::thread t2([&]() { release_ref(obj); });  // ref=1
    t1.join(); t2.join();
    release_ref(obj);  // ref=0，触发删除
}

void acq_rel_cas_demo() {
    std::cout << "\n=== 3. acq_rel：CAS 链表头插 ===\n";

    struct Node {
        int val;
        Node* next;
        explicit Node(int v) : val(v), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};

    // 多线程无锁头插
    auto push = [&](int val) {
        Node* node = new Node(val);
        Node* old_head = head.load(std::memory_order_relaxed);
        do {
            node->next = old_head;
        } while (!head.compare_exchange_weak(
            old_head, node,
            std::memory_order_release,   // 成功：release 发布新节点
            std::memory_order_relaxed)); // 失败：relaxed 重读 head
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i)
        threads.emplace_back(push, i * 10);
    for (auto& t : threads) t.join();

    // 遍历链表
    std::cout << "  list: ";
    Node* cur = head.load(std::memory_order_acquire);
    while (cur) {
        std::cout << cur->val << " ";
        Node* next = cur->next;
        delete cur;
        cur = next;
    }
    std::cout << "\n";
}


// ============================================================
// 4. memory_order_seq_cst（顺序一致，默认值）
//    - 最强的内存序，所有 seq_cst 操作在所有线程中观察到的顺序一致
//    - 相当于在 release/acquire 基础上额外加了一个全局单一顺序
//    - 代价：在 x86 上 store 需要 MFENCE，在 ARM 上开销更大
//    - 适合：需要全局顺序的场景；默认首选，性能敏感时再降级
// ============================================================

void seq_cst_store_buffer_demo() {
    std::cout << "\n=== 4. seq_cst：store buffer 问题（release/acquire 无法解决）===\n";

    // 经典的 Dekker 场景：两个线程各写一个标志，再读对方标志
    // release/acquire 无法保证全局顺序，seq_cst 可以
    std::atomic<bool> x{false}, y{false};
    std::atomic<int> z{0};

    auto t1_fn = [&]() {
        x.store(true, std::memory_order_seq_cst);   // (1)
        if (!y.load(std::memory_order_seq_cst))     // (2)
            ++z;
    };
    auto t2_fn = [&]() {
        y.store(true, std::memory_order_seq_cst);   // (3)
        if (!x.load(std::memory_order_seq_cst))     // (4)
            ++z;
    };

    // seq_cst 保证全局顺序：(1)(2)(3)(4) 有唯一的全局观察顺序
    // 不可能同时出现 (2) 看到 y=false 且 (4) 看到 x=false
    // 即 z 不可能为 0
    std::thread ta(t1_fn), tb(t2_fn);
    ta.join(); tb.join();
    std::cout << "  z=" << z << " (seq_cst guarantees z >= 1)\n";
}

void seq_cst_vs_release_acquire_demo() {
    std::cout << "\n=== 4. seq_cst vs release/acquire：差异对比 ===\n";

    // release/acquire 只保证"单对"同步，不保证全局顺序
    // 以下场景 release/acquire 可能出现 r1=0, r2=0，seq_cst 不会

    std::atomic<int> a{0}, b{0};
    int r1 = -1, r2 = -1;

    for (int trial = 0; trial < 10000; ++trial) {
        a = 0; b = 0; r1 = -1; r2 = -1;

        std::thread tw1([&]() { a.store(1, std::memory_order_seq_cst); });
        std::thread tw2([&]() { b.store(1, std::memory_order_seq_cst); });
        std::thread tr1([&]() {
            r1 = a.load(std::memory_order_seq_cst) +
                 b.load(std::memory_order_seq_cst);
        });
        std::thread tr2([&]() {
            r2 = b.load(std::memory_order_seq_cst) +
                 a.load(std::memory_order_seq_cst);
        });

        tw1.join(); tw2.join(); tr1.join(); tr2.join();

        // seq_cst 保证不会出现 r1==0 && r2==0
        if (r1 == 0 && r2 == 0) {
            std::cout << "  seq_cst violated! (should never happen)\n";
            return;
        }
    }
    std::cout << "  10000 trials: seq_cst holds, r1+r2 >= 1 always\n";
}


// ============================================================
// 5. memory_order_consume（了解即可，实践中用 acquire 代替）
//    - 比 acquire 弱：只保证依赖于 load 结果的操作不被重排
//    - 理论上比 acquire 性能好（DEC Alpha 外现代架构无需额外屏障）
//    - 实践问题：编译器实现困难，GCC/Clang 将其提升为 acquire
//    - C++17 起标准标注"discouraged"，C++26 可能移除
// ============================================================

void consume_demo() {
    std::cout << "\n=== 5. consume：依赖链同步（实践中用 acquire 代替）===\n";

    std::atomic<int*> ptr{nullptr};
    int payload = 0;

    std::thread producer([&]() {
        payload = 42;
        int* p = new int(100);
        ptr.store(p, std::memory_order_release);
    });

    std::thread consumer([&]() {
        int* p = nullptr;
        // consume：只有"依赖于 p 的操作"（如 *p）不被重排
        // 不依赖 p 的操作（如读 payload）不受保证
        // 实际编译器将 consume 提升为 acquire，行为与 acquire 相同
        while (!(p = ptr.load(std::memory_order_consume))) ;
        std::cout << "  *p=" << *p << " (consume: dependent load safe)\n";
        // payload 不在依赖链上，consume 不保证，实际仍可读（编译器提升）
        delete p;
    });

    producer.join();
    consumer.join();
}


// ============================================================
// 6. 综合对比：六种内存序总结
// ============================================================

void summary_demo() {
    std::cout << "\n=== 6. 综合：各内存序适用场景对比 ===\n";

    std::cout << R"(
  内存序              操作        保证                    典型场景
  ─────────────────────────────────────────────────────────────────
  relaxed             任意        仅原子性                纯计数/统计
  consume             load        依赖链 happens-before   指针追踪（已废弃倾向）
  acquire             load        后续读写不重排到前      订阅标志/指针
  release             store       之前读写不重排到后      发布数据/指针
  acq_rel             RMW         acquire+release 兼备    引用计数/无锁结构
  seq_cst             任意        全局单一顺序            需要全局顺序的场景
  ─────────────────────────────────────────────────────────────────
  性能（快→慢）：relaxed > consume ≈ acquire/release > acq_rel > seq_cst
  正确性（弱→强）：relaxed < consume < acquire/release < acq_rel < seq_cst
)";

    // 内存屏障等价（x86）：
    // release store  =>  普通 store（x86 TSO 天然保证）
    // acquire load   =>  普通 load（x86 TSO 天然保证）
    // seq_cst store  =>  XCHG 或 MOV + MFENCE（有代价）
    // seq_cst load   =>  普通 load（x86 上与 acquire 相同）

    std::cout << "  x86 注意：release/acquire 在 x86 上几乎无额外开销\n";
    std::cout << "            seq_cst store 在 x86 上需要 MFENCE，有代价\n";
    std::cout << "  ARM 注意：release/acquire 需要 dmb 屏障指令\n";
    std::cout << "            relaxed 在 ARM 上才能真正体现性能优势\n";
}


// ============================================================
// 7. 实战：用最合适的内存序实现一个无锁标志位
// ============================================================

class OnceFlag {
public:
    // 只执行一次的操作（类似 std::call_once 简化版）
    template<typename F>
    void call_once(F&& fn) {
        // 先 relaxed 检测，避免 acquire 的开销
        if (done_.load(std::memory_order_relaxed)) return;
        // 再用 seq_cst CAS 确保只有一个线程执行
        bool expected = false;
        if (done_.compare_exchange_strong(expected, true,
                std::memory_order_seq_cst,
                std::memory_order_relaxed)) {
            fn();
        }
    }
private:
    std::atomic<bool> done_{false};
};

void once_flag_demo() {
    std::cout << "\n=== 7. 实战：OnceFlag（最优内存序选择）===\n";

    OnceFlag flag;
    std::atomic<int> exec_count{0};

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&]() {
            flag.call_once([&]() {
                exec_count.fetch_add(1, std::memory_order_relaxed);
                std::cout << "  fn executed exactly once\n";
            });
        });
    }
    for (auto& t : threads) t.join();
    std::cout << "  exec_count=" << exec_count << " (expected 1)\n";
}


int main() {
    // 1. relaxed
    relaxed_counter_demo();
    relaxed_wrong_usage_demo();

    // 2. release / acquire
    release_acquire_basic_demo();
    release_acquire_pointer_publish_demo();
    release_sequence_demo();

    // 3. acq_rel
    acq_rel_reference_count_demo();
    acq_rel_cas_demo();

    // 4. seq_cst
    seq_cst_store_buffer_demo();
    seq_cst_vs_release_acquire_demo();

    // 5. consume
    consume_demo();

    // 6. 总结对比
    summary_demo();

    // 7. 实战
    once_flag_demo();

    return 0;
}
