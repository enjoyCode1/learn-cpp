#include <iostream>
#include <set>
#include <unordered_set>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================
// std::set / std::unordered_set 从入门到高级用法
// set：红黑树，有序，唯一，O(log n)
// unordered_set：哈希表，无序，唯一，均摊 O(1)
// ============================================================

// ==================== 入门：set 基础操作 ====================
void set_basic_demo() {
    std::cout << "=== 入门：set 基础操作 ===\n";

    std::set<int> s{5, 3, 1, 4, 2, 3, 1};  // 自动去重 + 排序

    std::cout << "set: ";
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";

    // 插入
    auto [it, ok] = s.insert(6);
    std::cout << "insert(6) ok=" << std::boolalpha << ok << " val=" << *it << "\n";
    auto [it2, ok2] = s.insert(3);  // 重复，失败
    std::cout << "insert(3) ok=" << ok2 << "\n";

    // 查找
    std::cout << "contains(3)=" << (s.find(3) != s.end()) << "\n";
    std::cout << "contains(7)=" << s.contains(7) << "\n";  // C++20

    // 删除
    s.erase(3);
    std::cout << "after erase(3): ";
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "size=" << s.size() << "\n";
}

// ==================== 中级：set 有序范围操作 ====================
void set_range_demo() {
    std::cout << "\n=== 中级：set 有序范围操作 ===\n";

    std::set<int> s{1, 3, 5, 7, 9, 11, 13};

    // lower_bound / upper_bound
    auto lo = s.lower_bound(5);   // 第一个 >= 5
    auto hi = s.upper_bound(9);   // 第一个 >  9
    std::cout << "range [5,9]: ";
    for (auto it = lo; it != hi; ++it) std::cout << *it << " ";
    std::cout << "\n";

    // 前驱/后继
    auto it = s.find(7);
    std::cout << "pred(7)=" << *std::prev(it)
              << " succ(7)=" << *std::next(it) << "\n";

    // 反向遍历
    std::cout << "reverse: ";
    for (auto it2 = s.rbegin(); it2 != s.rend(); ++it2)
        std::cout << *it2 << " ";
    std::cout << "\n";

    // 自定义比较器（降序 set）
    std::set<int, std::greater<int>> desc{1, 3, 5, 7, 9};
    std::cout << "descending set: ";
    for (int x : desc) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：unordered_set ====================
void uset_demo() {
    std::cout << "\n=== 中级：unordered_set ===\n";

    std::unordered_set<std::string> us{"apple", "banana", "cherry"};

    us.insert("date");
    us.erase("banana");

    std::cout << "contains apple: " << std::boolalpha << us.contains("apple") << "\n";
    std::cout << "contains banana: " << us.contains("banana") << "\n";
    std::cout << "size=" << us.size()
              << " bucket_count=" << us.bucket_count()
              << " load_factor=" << us.load_factor() << "\n";

    std::cout << "elements (unordered): ";
    for (const auto& s : us) std::cout << s << " ";
    std::cout << "\n";
}

// ==================== 高级：集合运算 ====================
void set_operations_demo() {
    std::cout << "\n=== 高级：集合运算 ===\n";

    std::set<int> A{1, 2, 3, 4, 5};
    std::set<int> B{3, 4, 5, 6, 7};

    // 交集
    std::vector<int> inter;
    std::set_intersection(A.begin(), A.end(), B.begin(), B.end(),
                          std::back_inserter(inter));
    std::cout << "A ∩ B: ";
    for (int x : inter) std::cout << x << " ";
    std::cout << "\n";

    // 并集
    std::vector<int> uni;
    std::set_union(A.begin(), A.end(), B.begin(), B.end(),
                   std::back_inserter(uni));
    std::cout << "A ∪ B: ";
    for (int x : uni) std::cout << x << " ";
    std::cout << "\n";

    // 差集 A - B
    std::vector<int> diff;
    std::set_difference(A.begin(), A.end(), B.begin(), B.end(),
                        std::back_inserter(diff));
    std::cout << "A - B: ";
    for (int x : diff) std::cout << x << " ";
    std::cout << "\n";

    // 对称差
    std::vector<int> sym_diff;
    std::set_symmetric_difference(A.begin(), A.end(), B.begin(), B.end(),
                                  std::back_inserter(sym_diff));
    std::cout << "A △ B: ";
    for (int x : sym_diff) std::cout << x << " ";
    std::cout << "\n";

    // 子集判断
    std::set<int> C{3, 4};
    bool is_sub = std::includes(A.begin(), A.end(), C.begin(), C.end());
    std::cout << "C⊆A: " << std::boolalpha << is_sub << "\n";
}

// ==================== 高级：multiset ====================
void multiset_demo() {
    std::cout << "\n=== 高级：multiset（允许重复）===\n";

    std::multiset<int> ms{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};

    std::cout << "multiset: ";
    for (int x : ms) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "count(1)=" << ms.count(1)
              << " count(5)=" << ms.count(5) << "\n";

    // equal_range 找所有相同元素
    auto [lo, hi] = ms.equal_range(5);
    std::cout << "all 5s at positions: ";
    for (auto it = lo; it != hi; ++it) std::cout << *it << " ";
    std::cout << "\n";

    // 只删除一个 1
    ms.erase(ms.find(1));
    std::cout << "after erase one 1, count(1)=" << ms.count(1) << "\n";
}

// ==================== 高级：用 set 实现滑动窗口中位数 ====================
void sliding_window_median_demo() {
    std::cout << "\n=== 高级：滑动窗口中位数（双 multiset）===\n";

    // 维护两个 multiset：lo（较小一半，最大堆语义）+ hi（较大一半，最小堆语义）
    std::multiset<int> lo, hi;

    auto get_median = [&]() -> double {
        if (lo.size() == hi.size())
            return (*lo.rbegin() + *hi.begin()) / 2.0;
        return *lo.rbegin();
    };

    auto add = [&](int x) {
        lo.insert(x);
        hi.insert(*lo.rbegin()); lo.erase(lo.find(*lo.rbegin()));
        if (hi.size() > lo.size() + 1) {
            lo.insert(*hi.begin()); hi.erase(hi.begin());
        }
    };

    auto remove = [&](int x) {
        if (hi.count(x)) hi.erase(hi.find(x));
        else              lo.erase(lo.find(x));
        // 再平衡
        if (lo.size() < hi.size()) {
            lo.insert(*hi.begin()); hi.erase(hi.begin());
        } else if (lo.size() > hi.size() + 1) {
            hi.insert(*lo.rbegin()); lo.erase(lo.find(*lo.rbegin()));
        }
    };

    std::vector<int> nums{1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;

    std::cout << "nums: ";
    for (int x : nums) std::cout << x << " ";
    std::cout << "\nk=" << k << " medians: ";

    for (int i = 0; i < (int)nums.size(); ++i) {
        add(nums[i]);
        if (i >= k) remove(nums[i - k]);
        if (i >= k - 1) std::cout << get_median() << " ";
    }
    std::cout << "\n";
}

int main() {
    set_basic_demo();
    set_range_demo();
    uset_demo();
    set_operations_demo();
    multiset_demo();
    sliding_window_median_demo();
    return 0;
}
