#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <map>

// ============================================================
// 一致性哈希（Consistent Hash）实现
// 解决传统哈希在节点增删时大量数据迁移的问题
// 核心：将节点和数据映射到同一个哈希环，数据归属顺时针最近节点
// 虚拟节点（Virtual Nodes）：解决节点分布不均问题
// ============================================================

class ConsistentHash {
public:
    // replicas：每个实际节点的虚拟节点数量，越多分布越均匀
    explicit ConsistentHash(int replicas = 150) : replicas_(replicas) {}

    // 添加节点
    void add_node(const std::string& node) {
        for (int i = 0; i < replicas_; ++i) {
            size_t h = hash(virtual_key(node, i));
            ring_[h] = node;
        }
        nodes_.push_back(node);
    }

    // 移除节点
    void remove_node(const std::string& node) {
        for (int i = 0; i < replicas_; ++i) {
            size_t h = hash(virtual_key(node, i));
            ring_.erase(h);
        }
        nodes_.erase(std::find(nodes_.begin(), nodes_.end(), node));
    }

    // 查找 key 所属的节点（顺时针找最近节点）
    std::string get_node(const std::string& key) const {
        if (ring_.empty()) return "";
        size_t h = hash(key);
        // lower_bound 找到第一个 >= h 的位置
        auto it = ring_.lower_bound(h);
        if (it == ring_.end()) it = ring_.begin();  // 环形回绕
        return it->second;
    }

    // 统计每个节点负责的虚拟节点数量（均匀性指标）
    void print_distribution() const {
        std::map<std::string, int> cnt;
        for (const auto& [h, node] : ring_) ++cnt[node];
        std::cout << "virtual node distribution:\n";
        for (const auto& [node, c] : cnt)
            std::cout << "  " << node << ": " << c << " virtual nodes\n";
    }

    int node_count()    const { return (int)nodes_.size(); }
    int virtual_count() const { return (int)ring_.size(); }

private:
    int                         replicas_;
    std::map<size_t, std::string> ring_;   // 哈希环（有序map）
    std::vector<std::string>    nodes_;

    size_t hash(const std::string& key) const {
        return std::hash<std::string>{}(key);
    }

    std::string virtual_key(const std::string& node, int replica) const {
        return node + "#" + std::to_string(replica);
    }
};

// ==================== 入门：基础路由 ====================
void basic_demo() {
    std::cout << "=== 入门：一致性哈希基础路由 ===\n";

    ConsistentHash ch(10);
    ch.add_node("server-A");
    ch.add_node("server-B");
    ch.add_node("server-C");

    std::vector<std::string> keys{"user:1001", "user:1002", "order:2001",
                                   "product:3001", "session:4001"};
    for (const auto& k : keys)
        std::cout << "  " << k << " -> " << ch.get_node(k) << "\n";
}

// ==================== 中级：节点增删对数据迁移的影响 ====================
void migration_demo() {
    std::cout << "\n=== 中级：节点增删数据迁移量对比 ===\n";

    // 生成测试 key
    std::vector<std::string> keys;
    for (int i = 0; i < 1000; ++i) keys.push_back("key:" + std::to_string(i));

    ConsistentHash ch(150);
    ch.add_node("node-1");
    ch.add_node("node-2");
    ch.add_node("node-3");

    // 记录初始映射
    std::map<std::string, std::string> before;
    for (const auto& k : keys) before[k] = ch.get_node(k);

    // 添加新节点
    ch.add_node("node-4");

    // 统计受影响的 key 数量
    int affected = 0;
    for (const auto& k : keys)
        if (ch.get_node(k) != before[k]) ++affected;

    std::cout << "total keys: " << keys.size() << "\n";
    std::cout << "affected after adding node-4: " << affected
              << " (" << (100.0 * affected / keys.size()) << "%)\n";
    std::cout << "expected ~25% (1/4 of keys should migrate)\n";

    // 传统哈希对比（取模）：增加节点后几乎所有 key 都要迁移
    int trad_affected = 0;
    for (const auto& k : keys) {
        size_t h = std::hash<std::string>{}(k);
        bool before3 = (h % 3) != (h % 4);  // 模3 vs 模4 的桶不同
        if (before3) ++trad_affected;
    }
    std::cout << "traditional hash affected: " << trad_affected
              << " (" << (100.0 * trad_affected / keys.size()) << "%)\n";
}

// ==================== 高级：虚拟节点均匀性 ====================
void balance_demo() {
    std::cout << "\n=== 高级：虚拟节点数量对均匀性的影响 ===\n";

    std::vector<std::string> nodes{"node-A", "node-B", "node-C", "node-D"};
    std::vector<std::string> test_keys;
    for (int i = 0; i < 10000; ++i) test_keys.push_back("key:" + std::to_string(i));

    for (int replicas : {1, 10, 50, 150}) {
        ConsistentHash ch(replicas);
        for (const auto& n : nodes) ch.add_node(n);

        std::map<std::string, int> dist;
        for (const auto& k : test_keys) ++dist[ch.get_node(k)];

        // 计算标准差
        double mean = (double)test_keys.size() / nodes.size();
        double variance = 0;
        for (const auto& [n, c] : dist)
            variance += (c - mean) * (c - mean);
        variance /= nodes.size();
        double stddev = std::sqrt(variance);

        std::cout << "replicas=" << replicas << ": ";
        for (const auto& [n, c] : dist) std::cout << n << "=" << c << " ";
        std::cout << " stddev=" << (int)stddev << "\n";
    }
}

// ==================== 高级：分布式缓存场景模拟 ====================
void cache_demo() {
    std::cout << "\n=== 高级：分布式缓存路由模拟 ===\n";

    ConsistentHash ch(150);
    ch.add_node("cache-1:6379");
    ch.add_node("cache-2:6379");
    ch.add_node("cache-3:6379");

    ch.print_distribution();

    // 模拟缓存路由
    std::vector<std::string> cache_keys{
        "user:profile:1001", "user:profile:1002", "user:session:abc",
        "product:detail:999", "order:list:2001", "recommend:home:1001"
    };

    std::cout << "\nrouting table:\n";
    for (const auto& k : cache_keys)
        std::cout << "  " << k << " -> " << ch.get_node(k) << "\n";

    // 节点故障模拟
    std::cout << "\ncache-2 goes down:\n";
    ch.remove_node("cache-2:6379");
    for (const auto& k : cache_keys)
        std::cout << "  " << k << " -> " << ch.get_node(k) << "\n";
}

int main() {
    basic_demo();
    migration_demo();
    balance_demo();
    cache_demo();
    return 0;
}
