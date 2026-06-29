#include <print>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <algorithm>

// 注意：std::flat_map / std::flat_set 是 C++23 特性
// MSVC 19.50 尚未实现，此文件用 std::map/set 演示相同接口语义
// 待编译器支持后将 using 别名替换为真实类型即可

// 用 map/set 作为占位，接口与 flat_map/flat_set 完全一致
template<typename K, typename V>
using flat_map = std::map<K, V>;

template<typename K>
using flat_set = std::set<K>;

// ==================== 入门级 ====================

void basic_flat_map() {
    std::println("=== 入门：flat_map 基础 ===");

    flat_map<std::string, int> scores;
    scores["Alice"]   = 95;
    scores["Bob"]     = 87;
    scores["Charlie"] = 92;

    for (const auto& [name, score] : scores)
        std::println("  {}: {}", name, score);

    std::println("contains Alice: {}", scores.contains("Alice"));
    std::println("size: {}", scores.size());
}

void basic_flat_set() {
    std::println("\n=== 入门：flat_set 基础 ===");

    flat_set<int> s{5, 3, 1, 4, 1, 5, 9, 2, 6};

    std::print("flat_set: ");
    for (auto v : s) std::print("{} ", v);
    std::println("");

    std::println("contains 4: {}", s.contains(4));
    std::println("contains 7: {}", s.contains(7));
}

// ==================== 中级 ====================

void flat_map_keys_values() {
    std::println("\n=== 中级：遍历键和值 ===");

    flat_map<int, std::string> fm{
        {3, "three"}, {1, "one"}, {4, "four"}, {2, "two"}
    };

    // flat_map 真正实现中 keys()/values() 返回连续内存视图
    // 这里用迭代器模拟相同效果
    std::print("keys:   ");
    for (const auto& [k, v] : fm) std::print("{} ", k);
    std::println("");

    std::print("values: ");
    for (const auto& [k, v] : fm) std::print("{} ", v);
    std::println("");
}

void flat_set_operations() {
    std::println("\n=== 中级：集合操作 ===");

    flat_set<int> a{1, 2, 3, 4, 5};
    flat_set<int> b{3, 4, 5, 6, 7};

    std::vector<int> inter, uni;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                          std::back_inserter(inter));
    std::set_union(a.begin(), a.end(), b.begin(), b.end(),
                   std::back_inserter(uni));

    std::print("intersection: ");
    for (auto v : inter) std::print("{} ", v);
    std::println("");

    std::print("union: ");
    for (auto v : uni) std::print("{} ", v);
    std::println("");
}

// ==================== 高级 ====================

void config_table_demo() {
    std::println("\n=== 高级：flat_map 配置表（读多写少）===");

    // 真正的 flat_map 底层是连续内存，缓存友好，查找比 map 快
    // 适合：构建一次、频繁查询的场景
    std::vector<std::pair<std::string, std::string>> raw_config{
        {"app.timeout", "30"},
        {"app.workers", "4"},
        {"db.host",     "localhost"},
        {"db.name",     "mydb"},
        {"db.port",     "5432"},
        {"log.level",   "info"},
    };

    // flat_map 真实用法：sorted_unique 构造跳过内部排序
    // std::flat_map<std::string,std::string> config(std::sorted_unique, raw_config);
    flat_map<std::string, std::string> config(raw_config.begin(), raw_config.end());

    auto get = [&](const std::string& key, const std::string& def = "") {
        auto it = config.find(key);
        return it != config.end() ? it->second : def;
    };

    std::println("db.host={} port={}", get("db.host"), get("db.port"));
    std::println("app.workers={}", get("app.workers"));
    std::println("missing.key={}", get("missing.key", "(default)"));
}

void flat_multimap_demo() {
    std::println("\n=== 高级：multimap 多值映射 ===");

    std::multimap<std::string, int> tags{
        {"cpp",    11}, {"cpp", 14}, {"cpp", 17},
        {"cpp",    20}, {"cpp", 23},
        {"python", 3},  {"rust", 2021},
    };

    auto [beg, end] = tags.equal_range("cpp");
    std::print("cpp versions: ");
    for (auto it = beg; it != end; ++it)
        std::print("{} ", it->second);
    std::println("");

    std::println("total entries: {}", tags.size());
}

// flat_map 与 map 性能特点对比说明
void performance_comparison() {
    std::println("\n=== 高级：flat_map vs map 性能特点 ===");
    std::println(R"(
  容器          底层结构      查找      插入/删除   内存    缓存
  ──────────────────────────────────────────────────────────────
  map           红黑树        O(log n)  O(log n)    分散    差
  flat_map      连续数组      O(log n)  O(n)        紧凑    好
  unordered_map 哈希表        O(1)均摊  O(1)均摊    分散    差
  ──────────────────────────────────────────────────────────────
  flat_map 适合：元素数量 < 1000、读多写少、需要缓存友好
  map      适合：频繁插入删除、元素数量大
)");
}

int main() {
    basic_flat_map();
    basic_flat_set();
    flat_map_keys_values();
    flat_set_operations();
    config_table_demo();
    flat_multimap_demo();
    performance_comparison();
    return 0;
}
