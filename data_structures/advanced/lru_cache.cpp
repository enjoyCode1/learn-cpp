#include <iostream>
#include <unordered_map>
#include <list>
#include <optional>
#include <stdexcept>
#include <string>
#include <mutex>
#include <thread>
#include <vector>

// ============================================================
// LRU 缓存（Least Recently Used Cache）
// 淘汰策略：最久未被访问的数据优先淘汰
// 实现：哈希表（O(1) 查找）+ 双向链表（O(1) 移动到头部）
// get/put 均为 O(1)
// ============================================================

template<typename K, typename V>
class LRUCache {
    struct Node {
        K key;
        V val;
    };
    using ListIt = typename std::list<Node>::iterator;

public:
    explicit LRUCache(int cap) : cap_(cap) {
        if (cap <= 0) throw std::invalid_argument("capacity must be > 0");
    }

    // 查找，O(1)
    std::optional<V> get(const K& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return std::nullopt;
        // 移到链表头部（最近访问）
        list_.splice(list_.begin(), list_, it->second);
        return it->second->val;
    }

    // 插入/更新，O(1)
    void put(const K& key, const V& val) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->val = val;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }
        if ((int)map_.size() == cap_) {
            // 淘汰链表尾部（最久未访问）
            auto& lru = list_.back();
            map_.erase(lru.key);
            list_.pop_back();
        }
        list_.push_front({key, val});
        map_[key] = list_.begin();
    }

    // 主动删除
    bool remove(const K& key) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        list_.erase(it->second);
        map_.erase(it);
        return true;
    }

    bool   contains(const K& key) const { return map_.count(key) > 0; }
    int    size()     const { return (int)map_.size(); }
    int    capacity() const { return cap_; }

    // 按访问顺序打印（头部最新，尾部最旧）
    void print() const {
        std::cout << "[MRU -> LRU]: ";
        for (const auto& node : list_)
            std::cout << node.key << ":" << node.val << " ";
        std::cout << "(size=" << map_.size() << "/" << cap_ << ")\n";
    }

private:
    int                                     cap_;
    std::list<Node>                         list_;
    std::unordered_map<K, ListIt>           map_;
};

// ============================================================
// 线程安全 LRU 缓存
// 在 LRU 基础上加互斥锁，支持多线程并发访问
// ============================================================

template<typename K, typename V>
class ThreadSafeLRU {
public:
    explicit ThreadSafeLRU(int cap) : cache_(cap) {}

    std::optional<V> get(const K& key) {
        std::lock_guard<std::mutex> lock(mu_);
        return cache_.get(key);
    }

    void put(const K& key, const V& val) {
        std::lock_guard<std::mutex> lock(mu_);
        cache_.put(key, val);
    }

    bool remove(const K& key) {
        std::lock_guard<std::mutex> lock(mu_);
        return cache_.remove(key);
    }

private:
    LRUCache<K, V>  cache_;
    std::mutex      mu_;
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：LRU 缓存基础操作 ===\n";

    LRUCache<int, std::string> cache(3);
    cache.put(1, "one");   cache.print();
    cache.put(2, "two");   cache.print();
    cache.put(3, "three"); cache.print();

    // 访问 key=1，使其变为最近访问
    auto v = cache.get(1);
    std::cout << "get(1)=" << (v ? *v : "null") << "\n";
    cache.print();

    // 插入 key=4，淘汰最久未访问（key=2）
    cache.put(4, "four");
    std::cout << "after put(4): "; cache.print();
    std::cout << "get(2)=" << (cache.get(2) ? *cache.get(2) : "null (evicted)") << "\n";
}

// ==================== 中级：LRU 作为 DNS 缓存 ====================
void dns_cache_demo() {
    std::cout << "\n=== 中级：DNS 缓存模拟 ===\n";

    LRUCache<std::string, std::string> dns(4);

    auto resolve = [&](const std::string& domain) -> std::string {
        auto cached = dns.get(domain);
        if (cached) {
            std::cout << "  [cache hit]  " << domain << " -> " << *cached << "\n";
            return *cached;
        }
        // 模拟 DNS 解析（实际应查询 DNS 服务器）
        std::string ip = "192.168.1." + std::to_string(dns.size() + 1);
        std::cout << "  [cache miss] " << domain << " -> " << ip << " (resolved)\n";
        dns.put(domain, ip);
        return ip;
    };

    std::vector<std::string> queries{
        "google.com", "github.com", "google.com",
        "stackoverflow.com", "rust-lang.org",
        "github.com",       // 命中缓存
        "cppreference.com", // 触发淘汰 google.com
        "google.com",       // 再次 miss
    };

    for (const auto& q : queries) resolve(q);
    std::cout << "final cache: "; dns.print();
}

// ==================== 高级：线程安全 LRU 并发测试 ====================
void concurrent_demo() {
    std::cout << "\n=== 高级：线程安全 LRU 并发测试 ===\n";

    ThreadSafeLRU<int, int> cache(50);
    const int THREADS = 4;
    const int OPS     = 1000;
    std::atomic<int> hits{0}, misses{0};

    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < OPS; ++i) {
                int key = (t * OPS + i) % 80;  // key 范围 0~79，缓存容量 50
                // 写入
                cache.put(key, key * key);
                // 读取
                auto v = cache.get(key);
                if (v) hits.fetch_add(1, std::memory_order_relaxed);
                else   misses.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto& th : threads) th.join();

    std::cout << "threads=" << THREADS << " ops_each=" << OPS << "\n";
    std::cout << "hits=" << hits << " misses=" << misses << "\n";
    std::cout << "cache size=" << cache.get(0).has_value() << " (sample check)\n";
}

int main() {
    basic_demo();
    dns_cache_demo();
    concurrent_demo();
    return 0;
}
