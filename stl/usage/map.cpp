#include <iostream>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================
// std::map / std::unordered_map 从入门到高级用法
// map：红黑树，有序，O(log n)
// unordered_map：哈希表，无序，均摊 O(1)
// ============================================================

// ==================== 入门：map 基础操作 ====================
void map_basic_demo() {
    std::cout << "=== 入门：map 基础操作 ===\n";

    std::map<std::string, int> m;

    // 插入
    m["alice"] = 90;
    m["bob"]   = 85;
    m.insert({"carol", 92});
    m.emplace("dave", 78);

    // 查找
    auto it = m.find("alice");
    if (it != m.end())
        std::cout << "alice=" << it->second << "\n";

    // operator[] 不存在时插入默认值（危险：会创建键）
    std::cout << "eve(default)=" << m["eve"] << "\n";  // 插入 0

    // at() 不存在时抛异常，更安全
    try { m.at("nobody"); }
    catch (const std::out_of_range& e) { std::cout << "at(nobody): " << e.what() << "\n"; }

    // contains (C++20)
    std::cout << "contains bob: " << std::boolalpha << m.contains("bob") << "\n";

    // 遍历（有序）
    std::cout << "all entries (sorted by key):\n";
    for (auto& [k, v] : m)
        std::cout << "  " << k << "=" << v << "\n";

    // 删除
    m.erase("eve");
    std::cout << "size after erase(eve)=" << m.size() << "\n";
}

// ==================== 中级：map 的有序特性 ====================
void map_ordered_demo() {
    std::cout << "\n=== 中级：map 有序特性 ===\n";

    std::map<int, std::string> m{{3,"c"},{1,"a"},{4,"d"},{2,"b"}};

    // lower_bound / upper_bound 范围查找
    auto lo = m.lower_bound(2);
    auto hi = m.upper_bound(3);
    std::cout << "range [2,3]: ";
    for (auto it = lo; it != hi; ++it)
        std::cout << it->first << ":" << it->second << " ";
    std::cout << "\n";

    // 最小/最大键
    std::cout << "min key=" << m.begin()->first
              << " max key=" << m.rbegin()->first << "\n";

    // 自定义比较器（降序）
    std::map<int, std::string, std::greater<int>> desc_map{{1,"a"},{2,"b"},{3,"c"}};
    std::cout << "descending: ";
    for (auto& [k, v] : desc_map) std::cout << k << " ";
    std::cout << "\n";
}

// ==================== 中级：unordered_map 基础 ====================
void umap_basic_demo() {
    std::cout << "\n=== 中级：unordered_map 基础 ===\n";

    std::unordered_map<std::string, int> um;
    um.reserve(16);  // 预分配，减少 rehash

    um["x"] = 1; um["y"] = 2; um["z"] = 3;

    std::cout << "load_factor=" << um.load_factor()
              << " bucket_count=" << um.bucket_count() << "\n";

    // 查找
    if (auto it = um.find("x"); it != um.end())
        std::cout << "x=" << it->second << "\n";

    // 遍历（无序）
    std::cout << "all (unordered): ";
    for (auto& [k, v] : um) std::cout << k << "=" << v << " ";
    std::cout << "\n";
}

// ==================== 高级：词频统计 ====================
void word_freq_demo() {
    std::cout << "\n=== 高级：词频统计 ===\n";

    std::string text = "the quick brown fox jumps over the lazy dog the fox";
    std::unordered_map<std::string, int> freq;

    std::string word;
    for (char c : text + " ") {
        if (c == ' ') {
            if (!word.empty()) { ++freq[word]; word.clear(); }
        } else word += c;
    }

    // 按频率降序输出
    std::vector<std::pair<std::string,int>> sorted(freq.begin(), freq.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const auto& a, const auto& b){ return a.second > b.second; });

    for (auto& [w, c] : sorted)
        std::cout << "  " << w << ": " << c << "\n";
}

// ==================== 高级：multimap 一键多值 ====================
void multimap_demo() {
    std::cout << "\n=== 高级：multimap 一键多值 ===\n";

    std::multimap<int, std::string> mm;
    mm.insert({1, "apple"});
    mm.insert({2, "banana"});
    mm.insert({1, "avocado"});
    mm.insert({2, "blueberry"});
    mm.insert({1, "apricot"});

    // 查找所有 key=1 的值
    auto [lo, hi] = mm.equal_range(1);
    std::cout << "key=1: ";
    for (auto it = lo; it != hi; ++it)
        std::cout << it->second << " ";
    std::cout << "\n";

    std::cout << "count(1)=" << mm.count(1) << "\n";
}

// ==================== 高级：自定义哈希 ====================
struct Point { int x, y; };

struct PointHash {
    size_t operator()(const Point& p) const {
        // 常用哈希组合方式
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 32);
    }
};

struct PointEq {
    bool operator()(const Point& a, const Point& b) const {
        return a.x == b.x && a.y == b.y;
    }
};

void custom_hash_demo() {
    std::cout << "\n=== 高级：自定义哈希 ===\n";

    std::unordered_map<Point, std::string, PointHash, PointEq> grid;
    grid[{0, 0}] = "origin";
    grid[{1, 2}] = "point A";
    grid[{3, 4}] = "point B";

    if (auto it = grid.find({1, 2}); it != grid.end())
        std::cout << "(1,2)=" << it->second << "\n";

    std::cout << "grid size=" << grid.size() << "\n";
}

// ==================== 高级：insert_or_assign / try_emplace (C++17) ====================
void cpp17_demo() {
    std::cout << "\n=== 高级：C++17 insert_or_assign / try_emplace ===\n";

    std::map<std::string, std::vector<int>> m;

    // try_emplace：不存在才构造，避免不必要的拷贝
    m.try_emplace("a", std::initializer_list<int>{1, 2, 3});
    m.try_emplace("a", std::initializer_list<int>{9, 9});  // 已存在，不覆盖
    std::cout << "a (try_emplace twice): ";
    for (int v : m["a"]) std::cout << v << " ";
    std::cout << "\n";

    // insert_or_assign：无论是否存在都写入
    m.insert_or_assign("b", std::vector<int>{4, 5, 6});
    m.insert_or_assign("b", std::vector<int>{7, 8, 9});  // 覆盖
    std::cout << "b (insert_or_assign twice): ";
    for (int v : m["b"]) std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    map_basic_demo();
    map_ordered_demo();
    umap_basic_demo();
    word_freq_demo();
    multimap_demo();
    custom_hash_demo();
    cpp17_demo();
    return 0;
}
