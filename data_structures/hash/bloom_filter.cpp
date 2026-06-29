#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <bitset>
#include <cmath>

// ============================================================
// 布隆过滤器（Bloom Filter）实现
// 空间高效的概率型数据结构，用于判断元素是否在集合中
// 特性：无假阴性（不在集合中一定返回 false）
//        有假阳性（可能误报存在）
// 不支持删除（标准布隆过滤器）
// ============================================================

class BloomFilter {
public:
    // capacity：预期元素数量，fp_rate：期望假阳性率
    BloomFilter(size_t capacity, double fp_rate = 0.01)
        : capacity_(capacity), fp_rate_(fp_rate) {
        // 最优位数组大小：m = -n*ln(p) / (ln2)^2
        size_t m = optimal_bits(capacity, fp_rate);
        bits_.assign(m, false);
        // 最优哈希函数数量：k = m/n * ln2
        k_ = optimal_k(m, capacity);

        std::cout << "BloomFilter: bits=" << m
                  << " hash_funcs=" << k_
                  << " fp_rate=" << fp_rate << "\n";
    }

    // 插入元素
    void insert(const std::string& key) {
        for (size_t i = 0; i < k_; ++i)
            bits_[hash(key, i) % bits_.size()] = true;
    }

    // 查询：可能存在（有假阳性）/ 一定不存在
    bool possibly_contains(const std::string& key) const {
        for (size_t i = 0; i < k_; ++i)
            if (!bits_[hash(key, i) % bits_.size()]) return false;
        return true;
    }

    // 当前填充率
    double fill_ratio() const {
        size_t set_bits = 0;
        for (bool b : bits_) if (b) ++set_bits;
        return (double)set_bits / bits_.size();
    }

    size_t bit_size()   const { return bits_.size(); }
    size_t hash_count() const { return k_; }

private:
    size_t              capacity_;
    double              fp_rate_;
    std::vector<bool>   bits_;
    size_t              k_;

    static size_t optimal_bits(size_t n, double p) {
        return (size_t)std::ceil(-(double)n * std::log(p) / (std::log(2) * std::log(2)));
    }

    static size_t optimal_k(size_t m, size_t n) {
        return std::max((size_t)1, (size_t)std::round((double)m / n * std::log(2)));
    }

    // 使用双重哈希模拟 k 个独立哈希函数
    // h_i(x) = h1(x) + i * h2(x)
    size_t hash(const std::string& key, size_t seed) const {
        size_t h1 = std::hash<std::string>{}(key);
        size_t h2 = std::hash<std::string>{}(key + "_salt");
        return h1 + seed * h2;
    }
};

// ============================================================
// 计数布隆过滤器（Counting Bloom Filter）
// 用计数器替代位，支持删除操作
// ============================================================
class CountingBloomFilter {
public:
    CountingBloomFilter(size_t bits, size_t k)
        : counters_(bits, 0), k_(k) {}

    void insert(const std::string& key) {
        for (size_t i = 0; i < k_; ++i)
            ++counters_[hash(key, i) % counters_.size()];
    }

    // 删除：只有之前插入过才能删除
    bool remove(const std::string& key) {
        if (!possibly_contains(key)) return false;
        for (size_t i = 0; i < k_; ++i) {
            size_t idx = hash(key, i) % counters_.size();
            if (counters_[idx] > 0) --counters_[idx];
        }
        return true;
    }

    bool possibly_contains(const std::string& key) const {
        for (size_t i = 0; i < k_; ++i)
            if (counters_[hash(key, i) % counters_.size()] == 0) return false;
        return true;
    }

private:
    std::vector<int> counters_;
    size_t           k_;

    size_t hash(const std::string& key, size_t seed) const {
        size_t h1 = std::hash<std::string>{}(key);
        size_t h2 = std::hash<std::string>{}(key + "_salt");
        return h1 + seed * h2;
    }
};

// ==================== 入门：基础用法 ====================
void basic_demo() {
    std::cout << "=== 入门：布隆过滤器基础 ===\n";

    BloomFilter bf(100, 0.01);

    std::vector<std::string> inserted{"apple", "banana", "cherry", "date", "elderberry"};
    for (const auto& s : inserted) bf.insert(s);

    std::cout << "inserted: ";
    for (const auto& s : inserted) std::cout << s << " ";
    std::cout << "\n";

    // 已插入的元素：必须返回 true（无假阴性）
    std::cout << "checking inserted elements (all must be true):\n";
    for (const auto& s : inserted)
        std::cout << "  " << s << ": " << std::boolalpha << bf.possibly_contains(s) << "\n";

    // 未插入的元素：大概率返回 false，偶尔假阳性
    std::vector<std::string> not_inserted{"fig", "grape", "honeydew", "kiwi"};
    std::cout << "checking NOT inserted (may have false positives):\n";
    for (const auto& s : not_inserted)
        std::cout << "  " << s << ": " << bf.possibly_contains(s) << "\n";

    std::cout << "fill_ratio=" << bf.fill_ratio() << "\n";
}

// ==================== 中级：假阳性率测试 ====================
void false_positive_demo() {
    std::cout << "\n=== 中级：假阳性率测试 ===\n";

    size_t n = 1000;
    BloomFilter bf(n, 0.01);

    // 插入 0~999
    for (size_t i = 0; i < n; ++i)
        bf.insert("item_" + std::to_string(i));

    // 测试 1000~1999（未插入），统计假阳性
    int fp = 0, total = 1000;
    for (int i = 1000; i < 1000 + total; ++i) {
        if (bf.possibly_contains("item_" + std::to_string(i))) ++fp;
    }

    std::cout << "expected fp_rate=0.01\n";
    std::cout << "actual   fp_rate=" << (double)fp / total
              << " (" << fp << "/" << total << ")\n";
}

// ==================== 高级：计数布隆过滤器（支持删除）====================
void counting_demo() {
    std::cout << "\n=== 高级：计数布隆过滤器（支持删除）===\n";

    CountingBloomFilter cbf(1024, 4);

    cbf.insert("alpha");
    cbf.insert("beta");
    cbf.insert("gamma");

    std::cout << "after insert alpha/beta/gamma:\n";
    std::cout << "  alpha: " << std::boolalpha << cbf.possibly_contains("alpha") << "\n";
    std::cout << "  beta:  " << cbf.possibly_contains("beta")  << "\n";
    std::cout << "  delta: " << cbf.possibly_contains("delta") << "\n";

    cbf.remove("beta");
    std::cout << "after remove beta:\n";
    std::cout << "  beta:  " << cbf.possibly_contains("beta")  << "\n";
    std::cout << "  alpha: " << cbf.possibly_contains("alpha") << "\n";
}

int main() {
    basic_demo();
    false_positive_demo();
    counting_demo();
    return 0;
}
