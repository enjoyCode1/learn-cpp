#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <string>

// ============================================================
// HashSet 实现（基于链地址法）
// 支持插入、删除、查找，O(1) 均摊
// ============================================================

template<typename T, typename Hash = std::hash<T>>
class HashSet {
    using Bucket = std::list<T>;

public:
    explicit HashSet(size_t cap = 16, float max_load = 0.75f)
        : buckets_(cap), size_(0), max_load_(max_load) {}

    // 插入，O(1) 均摊
    bool insert(const T& val) {
        if (load_factor() >= max_load_) rehash(buckets_.size() * 2);
        auto& bucket = buckets_[bucket_index(val)];
        for (const auto& v : bucket)
            if (v == val) return false;  // 已存在
        bucket.push_back(val);
        ++size_;
        return true;
    }

    // 删除，O(1) 均摊
    bool erase(const T& val) {
        auto& bucket = buckets_[bucket_index(val)];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (*it == val) { bucket.erase(it); --size_; return true; }
        }
        return false;
    }

    // 查找，O(1) 均摊
    bool contains(const T& val) const {
        const auto& bucket = buckets_[bucket_index(val)];
        for (const auto& v : bucket)
            if (v == val) return true;
        return false;
    }

    size_t size()        const { return size_; }
    bool   empty()       const { return size_ == 0; }
    float  load_factor() const { return (float)size_ / buckets_.size(); }

    // 遍历
    void foreach(std::function<void(const T&)> fn) const {
        for (const auto& bucket : buckets_)
            for (const auto& v : bucket)
                fn(v);
    }

    // 集合运算：交集
    HashSet intersection(const HashSet& other) const {
        HashSet res;
        foreach([&](const T& v) { if (other.contains(v)) res.insert(v); });
        return res;
    }

    // 集合运算：并集
    HashSet union_with(const HashSet& other) const {
        HashSet res;
        foreach([&](const T& v)       { res.insert(v); });
        other.foreach([&](const T& v) { res.insert(v); });
        return res;
    }

    // 集合运算：差集（this - other）
    HashSet difference(const HashSet& other) const {
        HashSet res;
        foreach([&](const T& v) { if (!other.contains(v)) res.insert(v); });
        return res;
    }

    void print() const {
        std::cout << "{";
        bool first = true;
        foreach([&](const T& v) {
            if (!first) std::cout << ", ";
            std::cout << v;
            first = false;
        });
        std::cout << "}  size=" << size_ << "\n";
    }

private:
    std::vector<Bucket> buckets_;
    size_t              size_;
    float               max_load_;
    Hash                hasher_;

    size_t bucket_index(const T& val) const {
        return hasher_(val) % buckets_.size();
    }

    void rehash(size_t new_cap) {
        std::vector<Bucket> new_buckets(new_cap);
        for (auto& bucket : buckets_)
            for (auto& v : bucket) {
                size_t idx = hasher_(v) % new_cap;
                new_buckets[idx].push_back(std::move(v));
            }
        buckets_ = std::move(new_buckets);
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：HashSet 基础操作 ===\n";

    HashSet<int> s;
    for (int v : {3, 1, 4, 1, 5, 9, 2, 6, 5, 3}) s.insert(v);  // 重复自动去重

    s.print();
    std::cout << "contains(4)=" << std::boolalpha << s.contains(4) << "\n";
    std::cout << "contains(7)=" << s.contains(7) << "\n";

    s.erase(4);
    std::cout << "after erase(4): "; s.print();
}

// ==================== 中级：集合运算 ====================
void set_ops_demo() {
    std::cout << "\n=== 中级：集合运算 ===\n";

    HashSet<int> a, b;
    for (int v : {1, 2, 3, 4, 5}) a.insert(v);
    for (int v : {3, 4, 5, 6, 7}) b.insert(v);

    std::cout << "A: "; a.print();
    std::cout << "B: "; b.print();

    auto inter = a.intersection(b);
    std::cout << "A ∩ B: "; inter.print();

    auto uni = a.union_with(b);
    std::cout << "A ∪ B: "; uni.print();

    auto diff = a.difference(b);
    std::cout << "A - B: "; diff.print();
}

// ==================== 中级：字符串去重 ====================
void string_dedup_demo() {
    std::cout << "\n=== 中级：字符串去重 ===\n";

    std::vector<std::string> words{
        "apple", "banana", "apple", "cherry", "banana", "date", "cherry"
    };

    HashSet<std::string> seen;
    std::vector<std::string> unique;
    for (const auto& w : words) {
        if (seen.insert(w)) unique.push_back(w);  // 首次插入成功才保留
    }

    std::cout << "original: ";
    for (const auto& w : words) std::cout << w << " ";
    std::cout << "\nunique:   ";
    for (const auto& w : unique) std::cout << w << " ";
    std::cout << "\n";
}

// ==================== 高级：两数之和（HashSet 加速查找）====================
std::vector<std::pair<int,int>> two_sum_pairs(const std::vector<int>& nums, int target) {
    HashSet<int> seen;
    HashSet<int> used;  // 避免重复对
    std::vector<std::pair<int,int>> res;

    for (int v : nums) {
        int complement = target - v;
        if (seen.contains(complement) && !used.contains(v) && !used.contains(complement)) {
            res.push_back({std::min(v, complement), std::max(v, complement)});
            used.insert(v);
            used.insert(complement);
        }
        seen.insert(v);
    }
    return res;
}

void two_sum_demo() {
    std::cout << "\n=== 高级：两数之和 ===\n";

    std::vector<int> nums{2, 7, 11, 15, 3, 4, 6, 9, 1};
    int target = 10;

    auto pairs = two_sum_pairs(nums, target);
    std::cout << "nums: ";
    for (int v : nums) std::cout << v << " ";
    std::cout << "\ntarget=" << target << ", pairs:\n";
    for (auto [a, b] : pairs)
        std::cout << "  " << a << " + " << b << " = " << target << "\n";
}

int main() {
    basic_demo();
    set_ops_demo();
    string_dedup_demo();
    two_sum_demo();
    return 0;
}
