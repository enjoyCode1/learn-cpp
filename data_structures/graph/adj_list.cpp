#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>
#include <functional>

// ============================================================
// 邻接表图实现
// 适合稀疏图，空间 O(V+E)，遍历邻居 O(degree)
// ============================================================

struct Edge {
    int to, weight;
};

class AdjList {
public:
    explicit AdjList(int v, bool directed = false)
        : V_(v), directed_(directed), adj_(v) {}

    void add_edge(int u, int v, int w = 1) {
        adj_[u].push_back({v, w});
        if (!directed_) adj_[v].push_back({u, w});
    }

    // BFS，O(V+E)
    std::vector<int> bfs(int start) const {
        std::vector<bool> visited(V_, false);
        std::vector<int>  order;
        std::queue<int>   q;
        visited[start] = true;
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (auto& e : adj_[u])
                if (!visited[e.to]) { visited[e.to] = true; q.push(e.to); }
        }
        return order;
    }

    // DFS 递归，O(V+E)
    std::vector<int> dfs(int start) const {
        std::vector<bool> visited(V_, false);
        std::vector<int>  order;
        std::function<void(int)> dfs_impl = [&](int u) {
            visited[u] = true;
            order.push_back(u);
            for (auto& e : adj_[u])
                if (!visited[e.to]) dfs_impl(e.to);
        };
        dfs_impl(start);
        return order;
    }

    // Dijkstra 最短路（优先队列优化），O((V+E) log V)
    std::vector<int> dijkstra(int src) const {
        std::vector<int> dist(V_, INT_MAX);
        // {distance, vertex}
        std::priority_queue<std::pair<int,int>,
                            std::vector<std::pair<int,int>>,
                            std::greater<>> pq;
        dist[src] = 0;
        pq.push({0, src});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;  // 过时条目
            for (auto& e : adj_[u]) {
                if (dist[u] + e.weight < dist[e.to]) {
                    dist[e.to] = dist[u] + e.weight;
                    pq.push({dist[e.to], e.to});
                }
            }
        }
        return dist;
    }

    // Bellman-Ford，O(VE)，支持负权边
    std::vector<int> bellman_ford(int src) const {
        std::vector<int> dist(V_, INT_MAX);
        dist[src] = 0;
        // 松弛 V-1 次
        for (int iter = 0; iter < V_ - 1; ++iter)
            for (int u = 0; u < V_; ++u)
                if (dist[u] != INT_MAX)
                    for (auto& e : adj_[u])
                        if (dist[u] + e.weight < dist[e.to])
                            dist[e.to] = dist[u] + e.weight;
        return dist;
    }

    // Prim 最小生成树，O((V+E) log V)
    int prim_mst() const {
        std::vector<bool> in_mst(V_, false);
        std::vector<int>  key(V_, INT_MAX);
        std::priority_queue<std::pair<int,int>,
                            std::vector<std::pair<int,int>>,
                            std::greater<>> pq;
        key[0] = 0;
        pq.push({0, 0});
        int total = 0;

        while (!pq.empty()) {
            auto [w, u] = pq.top(); pq.pop();
            if (in_mst[u]) continue;
            in_mst[u] = true;
            total += w;
            for (auto& e : adj_[u])
                if (!in_mst[e.to] && e.weight < key[e.to]) {
                    key[e.to] = e.weight;
                    pq.push({e.weight, e.to});
                }
        }
        return total;
    }

    // 检测无向图是否有环（DFS）
    bool has_cycle() const {
        std::vector<bool> visited(V_, false);
        std::function<bool(int, int)> dfs_cycle = [&](int u, int parent) -> bool {
            visited[u] = true;
            for (auto& e : adj_[u]) {
                if (!visited[e.to]) {
                    if (dfs_cycle(e.to, u)) return true;
                } else if (e.to != parent) return true;
            }
            return false;
        };
        for (int i = 0; i < V_; ++i)
            if (!visited[i] && dfs_cycle(i, -1)) return true;
        return false;
    }

    void print() const {
        for (int u = 0; u < V_; ++u) {
            std::cout << u << " -> ";
            for (auto& e : adj_[u])
                std::cout << e.to << "(w=" << e.weight << ") ";
            std::cout << "\n";
        }
    }

    int vertices() const { return V_; }
    const std::vector<Edge>& neighbors(int u) const { return adj_[u]; }

private:
    int                          V_;
    bool                         directed_;
    std::vector<std::vector<Edge>> adj_;
};

// ==================== 入门：构建与遍历 ====================
void traversal_demo() {
    std::cout << "=== 入门：邻接表 BFS/DFS ===\n";

    AdjList g(6, false);
    g.add_edge(0, 1); g.add_edge(0, 2);
    g.add_edge(1, 3); g.add_edge(1, 4);
    g.add_edge(2, 5);

    std::cout << "adjacency list:\n"; g.print();

    std::cout << "BFS from 0: ";
    for (int v : g.bfs(0)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "DFS from 0: ";
    for (int v : g.dfs(0)) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 中级：最短路 + MST ====================
void shortest_path_demo() {
    std::cout << "\n=== 中级：Dijkstra + Prim MST ===\n";

    AdjList g(5, false);
    g.add_edge(0, 1, 2); g.add_edge(0, 3, 6);
    g.add_edge(1, 2, 3); g.add_edge(1, 3, 8); g.add_edge(1, 4, 5);
    g.add_edge(2, 4, 7); g.add_edge(3, 4, 9);

    auto dist = g.dijkstra(0);
    std::cout << "Dijkstra from 0:\n";
    for (int i = 0; i < (int)dist.size(); ++i)
        std::cout << "  0->" << i << " = " << dist[i] << "\n";

    std::cout << "Prim MST total weight = " << g.prim_mst() << "\n";
}

// ==================== 高级：Bellman-Ford 负权边 ====================
void bellman_ford_demo() {
    std::cout << "\n=== 高级：Bellman-Ford 负权边 ===\n";

    AdjList g(5, true);
    g.add_edge(0, 1,  6);
    g.add_edge(0, 3,  7);
    g.add_edge(1, 2,  5); g.add_edge(1, 3,  8); g.add_edge(1, 4, -4);
    g.add_edge(2, 1, -2);
    g.add_edge(3, 2, -3); g.add_edge(3, 4,  9);
    g.add_edge(4, 0,  2); g.add_edge(4, 2,  7);

    auto dist = g.bellman_ford(0);
    std::cout << "Bellman-Ford from 0 (with negative edges):\n";
    for (int i = 0; i < (int)dist.size(); ++i) {
        std::cout << "  0->" << i << " = ";
        if (dist[i] == INT_MAX) std::cout << "INF";
        else std::cout << dist[i];
        std::cout << "\n";
    }
}

int main() {
    traversal_demo();
    shortest_path_demo();
    bellman_ford_demo();
    return 0;
}
