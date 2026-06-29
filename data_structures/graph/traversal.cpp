#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include <functional>

// ============================================================
// 图的遍历：DFS 与 BFS
// 有向图 / 无向图均适用
// DFS：递归或显式栈，适合拓扑排序、SCC、环检测
// BFS：队列，适合最短路（无权）、层序遍历、连通性
// ============================================================

struct Graph {
    int n;
    std::vector<std::vector<std::pair<int,int>>> adj;  // {邻居, 权重}

    explicit Graph(int n) : n(n), adj(n + 1) {}

    void add_edge(int u, int v, int w = 1) { adj[u].push_back({v, w}); }
    void add_undirected(int u, int v, int w = 1) {
        add_edge(u, v, w); add_edge(v, u, w);
    }

    void print() const {
        for (int u = 1; u <= n; ++u) {
            std::cout << u << " -> ";
            for (auto [v, w] : adj[u]) std::cout << v << "(w=" << w << ") ";
            std::cout << "\n";
        }
    }
};

// ==================== 入门：递归 DFS ====================
void dfs_recursive(const Graph& g, int u,
                   std::vector<bool>& visited,
                   std::vector<int>& order) {
    visited[u] = true;
    order.push_back(u);
    for (auto [v, w] : g.adj[u])
        if (!visited[v]) dfs_recursive(g, v, visited, order);
}

void dfs_demo() {
    std::cout << "=== 入门：DFS 递归遍历 ===\n";

    Graph g(6);
    g.add_undirected(1, 2); g.add_undirected(1, 3);
    g.add_undirected(2, 4); g.add_undirected(2, 5);
    g.add_undirected(3, 6);

    std::vector<bool> visited(g.n + 1, false);
    std::vector<int>  order;
    dfs_recursive(g, 1, visited, order);

    std::cout << "DFS order from 1: ";
    for (int v : order) std::cout << v << " ";
    std::cout << "\n";

    // 非连通图：遍历所有连通分量
    Graph g2(7);
    g2.add_undirected(1, 2); g2.add_undirected(2, 3);
    g2.add_undirected(4, 5); g2.add_undirected(6, 7);

    std::vector<bool> vis2(g2.n + 1, false);
    int components = 0;
    for (int i = 1; i <= g2.n; ++i) {
        if (!vis2[i]) {
            std::vector<int> comp;
            dfs_recursive(g2, i, vis2, comp);
            std::cout << "component " << ++components << ": ";
            for (int v : comp) std::cout << v << " ";
            std::cout << "\n";
        }
    }
}

// ==================== 中级：迭代 DFS（显式栈）====================
void dfs_iterative_demo() {
    std::cout << "\n=== 中级：DFS 迭代（显式栈）===\n";

    Graph g(6);
    g.add_undirected(1, 2); g.add_undirected(1, 3);
    g.add_undirected(2, 4); g.add_undirected(2, 5);
    g.add_undirected(3, 6);

    std::vector<bool> visited(g.n + 1, false);
    std::vector<int>  order;
    std::stack<int>   stk;

    stk.push(1);
    while (!stk.empty()) {
        int u = stk.top(); stk.pop();
        if (visited[u]) continue;
        visited[u] = true;
        order.push_back(u);
        // 逆序压栈保证访问顺序与递归一致
        for (int i = (int)g.adj[u].size() - 1; i >= 0; --i)
            if (!visited[g.adj[u][i].first])
                stk.push(g.adj[u][i].first);
    }

    std::cout << "iterative DFS order: ";
    for (int v : order) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 中级：BFS ====================
void bfs_demo() {
    std::cout << "\n=== 中级：BFS 遍历 ===\n";

    Graph g(6);
    g.add_undirected(1, 2); g.add_undirected(1, 3);
    g.add_undirected(2, 4); g.add_undirected(2, 5);
    g.add_undirected(3, 6);

    std::vector<bool> visited(g.n + 1, false);
    std::vector<int>  dist(g.n + 1, -1);
    std::vector<int>  parent(g.n + 1, -1);
    std::vector<int>  order;
    std::queue<int>   q;

    visited[1] = true; dist[1] = 0;
    q.push(1);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (auto [v, w] : g.adj[u]) {
            if (!visited[v]) {
                visited[v] = true;
                dist[v]    = dist[u] + 1;
                parent[v]  = u;
                q.push(v);
            }
        }
    }

    std::cout << "BFS order: ";
    for (int v : order) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "distances from 1:\n";
    for (int i = 1; i <= g.n; ++i)
        std::cout << "  dist[" << i << "]=" << dist[i] << "\n";

    // 打印最短路径 1->6
    std::cout << "shortest path 1->6: ";
    std::vector<int> path;
    for (int v = 6; v != -1; v = parent[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    for (int v : path) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 高级：DFS 应用——环检测（有向图）====================
enum class Color { WHITE, GRAY, BLACK };

bool has_cycle_directed(const Graph& g) {
    std::vector<Color> color(g.n + 1, Color::WHITE);
    bool cycle = false;

    std::function<void(int)> dfs = [&](int u) {
        if (cycle) return;
        color[u] = Color::GRAY;
        for (auto [v, w] : g.adj[u]) {
            if (color[v] == Color::GRAY) { cycle = true; return; }
            if (color[v] == Color::WHITE) dfs(v);
        }
        color[u] = Color::BLACK;
    };

    for (int i = 1; i <= g.n; ++i)
        if (color[i] == Color::WHITE) dfs(i);
    return cycle;
}

void cycle_detect_demo() {
    std::cout << "\n=== 高级：有向图环检测（DFS 三色标记）===\n";

    Graph g1(4);
    g1.add_edge(1, 2); g1.add_edge(2, 3); g1.add_edge(3, 4);
    std::cout << "DAG(1->2->3->4) has cycle: "
              << std::boolalpha << has_cycle_directed(g1) << "\n";

    Graph g2(4);
    g2.add_edge(1, 2); g2.add_edge(2, 3); g2.add_edge(3, 1);
    std::cout << "cycle(1->2->3->1) has cycle: " << has_cycle_directed(g2) << "\n";
}

// ==================== 高级：DFS 应用——割点与桥 ====================
struct ArticulationInfo {
    std::vector<int>  disc;   // 发现时间戳
    std::vector<int>  low;    // low 值（可回溯到的最早时间戳）
    std::vector<bool> is_ap;  // 是否为割点
    std::vector<std::pair<int,int>> bridges;
    int timer = 0;
};

void dfs_ap(const Graph& g, int u, int parent,
            ArticulationInfo& info) {
    info.disc[u] = info.low[u] = ++info.timer;
    int children = 0;

    for (auto [v, w] : g.adj[u]) {
        if (!info.disc[v]) {
            ++children;
            dfs_ap(g, v, u, info);
            info.low[u] = std::min(info.low[u], info.low[v]);

            // 割点条件
            if (parent == -1 && children > 1) info.is_ap[u] = true;
            if (parent != -1 && info.low[v] >= info.disc[u]) info.is_ap[u] = true;

            // 桥条件
            if (info.low[v] > info.disc[u])
                info.bridges.push_back({u, v});
        } else if (v != parent) {
            info.low[u] = std::min(info.low[u], info.disc[v]);
        }
    }
}

void articulation_demo() {
    std::cout << "\n=== 高级：割点与桥（DFS）===\n";

    // 图：1-2-3-4, 2-5, 3-5
    Graph g(5);
    g.add_undirected(1, 2); g.add_undirected(2, 3);
    g.add_undirected(3, 4); g.add_undirected(2, 5);
    g.add_undirected(3, 5);

    ArticulationInfo info;
    info.disc.assign(g.n + 1, 0);
    info.low.assign(g.n + 1, 0);
    info.is_ap.assign(g.n + 1, false);

    for (int i = 1; i <= g.n; ++i)
        if (!info.disc[i]) dfs_ap(g, i, -1, info);

    std::cout << "articulation points: ";
    for (int i = 1; i <= g.n; ++i)
        if (info.is_ap[i]) std::cout << i << " ";
    std::cout << "\n";

    std::cout << "bridges: ";
    for (auto [u, v] : info.bridges)
        std::cout << u << "-" << v << " ";
    std::cout << "\n";
}

// ==================== 高级：BFS 应用——01 BFS（权重 0/1 的最短路）====================
void bfs_01_demo() {
    std::cout << "\n=== 高级：01 BFS（双端队列）===\n";

    // 权重只有 0 或 1 时，用 deque 代替优先队列，O(V+E)
    int n = 6;
    std::vector<std::vector<std::pair<int,int>>> adj(n + 1);
    auto add = [&](int u, int v, int w){ adj[u].push_back({v, w}); };

    add(1,2,0); add(1,3,1); add(2,4,1); add(3,4,0);
    add(4,5,1); add(3,6,0); add(6,5,1);

    const int INF = 1e9;
    std::vector<int> dist(n + 1, INF);
    std::deque<int>  dq;
    dist[1] = 0; dq.push_back(1);

    while (!dq.empty()) {
        int u = dq.front(); dq.pop_front();
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                if (w == 0) dq.push_front(v);  // 0 权边插队首
                else        dq.push_back(v);
            }
        }
    }

    std::cout << "01-BFS distances from 1:\n";
    for (int i = 1; i <= n; ++i)
        std::cout << "  dist[" << i << "]=" << dist[i] << "\n";
}

int main() {
    dfs_demo();
    dfs_iterative_demo();
    bfs_demo();
    cycle_detect_demo();
    articulation_demo();
    bfs_01_demo();
    return 0;
}
