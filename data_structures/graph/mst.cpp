#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <numeric>
#include <functional>

// ============================================================
// 最小生成树（MST）
// Kruskal：按边权排序，并查集合并，O(E log E)，适合稀疏图
// Prim：从节点扩展，优先队列，O((V+E) log V)，适合稠密图
// ============================================================

// ==================== 并查集（Kruskal 用）====================
struct UnionFind {
    std::vector<int> parent, rank_;
    UnionFind(int n) : parent(n+1), rank_(n+1, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }
    int find(int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(int x, int y) {
        int rx = find(x), ry = find(y);
        if (rx == ry) return false;
        if (rank_[rx] < rank_[ry]) std::swap(rx, ry);
        parent[ry] = rx;
        if (rank_[rx] == rank_[ry]) ++rank_[rx];
        return true;
    }
};

// ==================== Kruskal ====================
struct KEdge { int u, v, w; };

std::vector<KEdge> kruskal(int n, std::vector<KEdge> edges) {
    std::sort(edges.begin(), edges.end(),
              [](const KEdge& a, const KEdge& b){ return a.w < b.w; });
    UnionFind uf(n);
    std::vector<KEdge> mst;

    for (auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            mst.push_back(e);
            if ((int)mst.size() == n - 1) break;
        }
    }
    return mst;
}

void kruskal_demo() {
    std::cout << "=== Kruskal 最小生成树 ===\n";

    // 图（无向带权）：
    // 1--4--2--8--3
    // |  \  |  /  |
    // 8   11 7   4  2
    // |    \ | /   |
    // 4--2--5--6--6--7
    int n = 7;
    std::vector<KEdge> edges{
        {1,2,4},{1,5,8},{2,3,8},{2,5,11},
        {3,4,7},{3,7,2},{4,5,7},{4,6,6},
        {5,6,1},{6,7,2}
    };

    auto mst = kruskal(n, edges);
    int total = 0;
    std::cout << "MST edges:\n";
    for (auto& e : mst) {
        std::cout << "  " << e.u << "--" << e.v << " (w=" << e.w << ")\n";
        total += e.w;
    }
    std::cout << "total weight=" << total << "\n";

    // 验证：MST 应有 n-1 条边
    std::cout << "edge count=" << mst.size()
              << " (expected " << n-1 << ")\n";
}

// ==================== Prim（优先队列）====================
std::pair<int, std::vector<std::pair<int,int>>>
prim(int n, const std::vector<std::vector<std::pair<int,int>>>& adj, int src = 1) {
    std::vector<int>  key(n+1, INT_MAX);   // 连接到 MST 的最小边权
    std::vector<int>  parent(n+1, -1);
    std::vector<bool> in_mst(n+1, false);

    // {weight, node}
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    key[src] = 0;
    pq.push({0, src});

    int total = 0;

    while (!pq.empty()) {
        auto [w, u] = pq.top(); pq.pop();
        if (in_mst[u]) continue;
        in_mst[u] = true;
        total += w;

        for (auto [v, wt] : adj[u]) {
            if (!in_mst[v] && wt < key[v]) {
                key[v]    = wt;
                parent[v] = u;
                pq.push({wt, v});
            }
        }
    }

    std::vector<std::pair<int,int>> mst_edges;
    for (int i = 1; i <= n; ++i)
        if (parent[i] != -1) mst_edges.push_back({parent[i], i});
    return {total, mst_edges};
}

void prim_demo() {
    std::cout << "\n=== Prim 最小生成树 ===\n";

    int n = 7;
    std::vector<std::vector<std::pair<int,int>>> adj(n+1);
    auto add = [&](int u, int v, int w){
        adj[u].push_back({v,w}); adj[v].push_back({u,w});
    };
    add(1,2,4); add(1,5,8); add(2,3,8); add(2,5,11);
    add(3,4,7); add(3,7,2); add(4,5,7); add(4,6,6);
    add(5,6,1); add(6,7,2);

    auto [total, mst] = prim(n, adj);
    std::cout << "MST edges:\n";
    for (auto [u, v] : mst)
        std::cout << "  " << u << "--" << v << "\n";
    std::cout << "total weight=" << total << "\n";
}

// ==================== 高级：最大生成树 ====================
void max_spanning_tree_demo() {
    std::cout << "\n=== 高级：最大生成树 ===\n";

    // 只需将 Kruskal 的排序改为降序
    int n = 4;
    std::vector<KEdge> edges{{1,2,1},{1,3,3},{2,3,2},{2,4,4},{3,4,5}};

    std::sort(edges.begin(), edges.end(),
              [](const KEdge& a, const KEdge& b){ return a.w > b.w; });
    UnionFind uf(n);
    std::vector<KEdge> max_mst;
    for (auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            max_mst.push_back(e);
            if ((int)max_mst.size() == n-1) break;
        }
    }

    int total = 0;
    std::cout << "max spanning tree:\n";
    for (auto& e : max_mst) {
        std::cout << "  " << e.u << "--" << e.v << " (w=" << e.w << ")\n";
        total += e.w;
    }
    std::cout << "total weight=" << total << "\n";
}

// ==================== 高级：次小生成树 ====================
void second_mst_demo() {
    std::cout << "\n=== 高级：次小生成树 ===\n";

    // 策略：枚举 MST 外的每条边 (u,v,w)，
    //        用 w 替换 MST 中 u->v 路径上权最大的边，取最小增量
    int n = 4;
    std::vector<KEdge> edges{{1,2,1},{1,3,3},{1,4,4},{2,3,2},{3,4,5}};

    // 先求 MST（Kruskal）
    auto mst = kruskal(n, edges);
    int mst_w = 0;
    for (auto& e : mst) mst_w += e.w;

    // 记录 MST 中的边（集合）
    std::vector<KEdge> non_mst;
    std::sort(edges.begin(), edges.end(),
              [](const KEdge& a, const KEdge& b){ return a.w < b.w; });
    std::sort(mst.begin(), mst.end(),
              [](const KEdge& a, const KEdge& b){ return a.w < b.w; });

    for (auto& e : edges) {
        bool in_mst = false;
        for (auto& m : mst)
            if ((e.u==m.u&&e.v==m.v)||(e.u==m.v&&e.v==m.u)) { in_mst=true; break; }
        if (!in_mst) non_mst.push_back(e);
    }

    // 对于每条非 MST 边，找路径上最大边（简化：暴力 DFS）
    // 构建 MST 邻接表
    std::vector<std::vector<std::pair<int,int>>> mst_adj(n+1);
    for (auto& e : mst) {
        mst_adj[e.u].push_back({e.v, e.w});
        mst_adj[e.v].push_back({e.u, e.w});
    }

    auto max_on_path = [&](int s, int t) -> int {
        std::vector<bool> vis(n+1, false);
        int max_w = 0;
        std::function<bool(int, int&)> dfs = [&](int u, int& mw) -> bool {
            if (u == t) return true;
            vis[u] = true;
            for (auto [v, w] : mst_adj[u]) {
                if (!vis[v]) {
                    int sub_max = 0;
                    if (dfs(v, sub_max)) {
                        mw = std::max(mw, std::max(w, sub_max));
                        return true;
                    }
                }
            }
            return false;
        };
        dfs(s, max_w);
        return max_w;
    };

    int best = INT_MAX;
    for (auto& e : non_mst) {
        int replace = max_on_path(e.u, e.v);
        best = std::min(best, mst_w + e.w - replace);
    }
    std::cout << "MST weight=" << mst_w
              << " second MST weight=" << best << "\n";
}

int main() {
    kruskal_demo();
    prim_demo();
    max_spanning_tree_demo();
    second_mst_demo();
    return 0;
}
