#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>

// ============================================================
// 并查集（Union-Find / Disjoint Set Union）实现
// 支持路径压缩 + 按秩合并，近似 O(α(n)) 每次操作
// α 为阿克曼函数的反函数，实际上可视为常数
// 应用：连通分量、最小生成树（Kruskal）、网络连通性
// ============================================================

class UnionFind {
public:
    explicit UnionFind(int n)
        : parent_(n), rank_(n, 0), size_(n, 1), components_(n) {
        std::iota(parent_.begin(), parent_.end(), 0);  // parent[i] = i
    }

    // 查找根节点（路径压缩：将沿途节点直接指向根）
    int find(int x) {
        if (parent_[x] != x)
            parent_[x] = find(parent_[x]);  // 递归压缩
        return parent_[x];
    }

    // 合并两个集合（按秩合并：矮树挂在高树下）
    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;  // 已在同一集合

        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent_[ry] = rx;
        size_[rx]  += size_[ry];
        if (rank_[rx] == rank_[ry]) ++rank_[rx];
        --components_;
        return true;
    }

    // 判断是否连通
    bool connected(int x, int y) { return find(x) == find(y); }

    // 集合大小
    int component_size(int x) { return size_[find(x)]; }

    // 连通分量数
    int components() const { return components_; }

    void print() const {
        std::cout << "parent: ";
        for (int i = 0; i < (int)parent_.size(); ++i)
            std::cout << i << "->" << parent_[i] << " ";
        std::cout << "\ncomponents=" << components_ << "\n";
    }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
    std::vector<int> size_;
    int              components_;
};

// ==================== 入门：基础连通操作 ====================
void basic_demo() {
    std::cout << "=== 入门：并查集基础操作 ===\n";

    UnionFind uf(7);
    std::cout << "initial components=" << uf.components() << "\n";

    uf.unite(0, 1);
    uf.unite(1, 2);
    uf.unite(3, 4);
    uf.unite(5, 6);

    std::cout << "after unite(0,1) unite(1,2) unite(3,4) unite(5,6):\n";
    std::cout << "components=" << uf.components() << "\n";
    std::cout << "connected(0,2)=" << std::boolalpha << uf.connected(0, 2) << "\n";
    std::cout << "connected(0,3)=" << uf.connected(0, 3) << "\n";
    std::cout << "size of component(0)=" << uf.component_size(0) << "\n";

    uf.unite(2, 3);
    std::cout << "after unite(2,3): connected(0,4)=" << uf.connected(0, 4) << "\n";
    std::cout << "components=" << uf.components() << "\n";
}

// ==================== 中级：Kruskal 最小生成树 ====================
struct KEdge {
    int u, v, w;
    bool operator<(const KEdge& o) const { return w < o.w; }
};

int kruskal_mst(int n, std::vector<KEdge> edges) {
    std::sort(edges.begin(), edges.end());
    UnionFind uf(n);
    int total = 0, cnt = 0;

    std::cout << "Kruskal MST edges:\n";
    for (auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            total += e.w;
            ++cnt;
            std::cout << "  " << e.u << " -- " << e.v << " (w=" << e.w << ")\n";
            if (cnt == n - 1) break;
        }
    }
    return total;
}

void kruskal_demo() {
    std::cout << "\n=== 中级：Kruskal 最小生成树 ===\n";

    //  0 --4-- 1
    //  |  \ /  |
    //  2   X   2
    //  |  / \  |
    //  2 --3-- 3
    int n = 4;
    std::vector<KEdge> edges{
        {0, 1, 4}, {0, 2, 2}, {1, 2, 1},
        {1, 3, 2}, {2, 3, 3}
    };

    int cost = kruskal_mst(n, edges);
    std::cout << "MST total weight = " << cost << "\n";
}

// ==================== 高级：动态连通性（在线加边）====================
void dynamic_connectivity_demo() {
    std::cout << "\n=== 高级：动态连通性 ===\n";

    // 模拟社交网络：用户逐步建立好友关系
    // 查询：两用户是否在同一个朋友圈？
    int users = 10;
    UnionFind uf(users);

    auto add_friend = [&](int a, int b) {
        bool new_conn = uf.unite(a, b);
        std::cout << "friend(" << a << "," << b << "): "
                  << (new_conn ? "new connection" : "already connected")
                  << " | circles=" << uf.components() << "\n";
    };

    auto query = [&](int a, int b) {
        std::cout << "same circle(" << a << "," << b << ")? "
                  << std::boolalpha << uf.connected(a, b) << "\n";
    };

    add_friend(0, 1); add_friend(2, 3); add_friend(4, 5);
    add_friend(0, 2);  // 合并两个圈子
    query(1, 3);       // 应该是同一圈子
    query(1, 4);       // 应该不是

    add_friend(1, 4);  // 再合并
    query(0, 5);       // 现在是同一圈子

    std::cout << "group size of user 0: " << uf.component_size(0) << "\n";
    std::cout << "group size of user 6: " << uf.component_size(6) << "\n";
}

// ==================== 高级：判断图中是否有环 ====================
bool has_cycle(int n, const std::vector<std::pair<int,int>>& edges) {
    UnionFind uf(n);
    for (auto [u, v] : edges)
        if (!uf.unite(u, v)) return true;  // 已连通，加边成环
    return false;
}

void cycle_detect_demo() {
    std::cout << "\n=== 高级：并查集检测无向图环 ===\n";

    // 无环图
    std::vector<std::pair<int,int>> edges1{{0,1},{1,2},{2,3}};
    std::cout << "edges 0-1-2-3 has cycle: "
              << std::boolalpha << has_cycle(4, edges1) << "\n";

    // 有环图
    std::vector<std::pair<int,int>> edges2{{0,1},{1,2},{2,0}};
    std::cout << "edges 0-1-2-0 has cycle: " << has_cycle(3, edges2) << "\n";
}

int main() {
    basic_demo();
    kruskal_demo();
    dynamic_connectivity_demo();
    cycle_detect_demo();
    return 0;
}
