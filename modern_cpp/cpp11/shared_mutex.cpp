#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <map>
#include <string>

// 读写锁：多个读者可以并发，写者独占
class ThreadSafeCache {
public:
    // 读操作：shared_lock 允许并发读
    std::string get(const std::string& key) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        auto it = data_.find(key);
        return it != data_.end() ? it->second : "";
    }

    // 写操作：unique_lock 独占
    void set(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_timed_mutex> lock(mutex_);
        data_[key] = value;
    }

    size_t size() const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        return data_.size();
    }

private:
    mutable std::shared_timed_mutex mutex_;
    std::map<std::string, std::string> data_;
};

ThreadSafeCache g_cache;

void reader(int id) {
    for (int i = 0; i < 3; ++i) {
        auto val = g_cache.get("key");
        std::cout << "reader " << id << " got: " << val << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void writer(int id) {
    for (int i = 0; i < 2; ++i) {
        std::string val = "value_from_writer" + std::to_string(id);
        g_cache.set("key", val);
        std::cout << "writer " << id << " set: " << val << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

int main() {
    g_cache.set("key", "initial");

    std::vector<std::thread> threads;

    // 3 个读线程，1 个写线程并发
    for (int i = 0; i < 3; ++i) threads.emplace_back(reader, i);
    threads.emplace_back(writer, 0);

    for (auto& t : threads) t.join();

    std::cout << "cache size=" << g_cache.size() << "\n";
    return 0;
}
