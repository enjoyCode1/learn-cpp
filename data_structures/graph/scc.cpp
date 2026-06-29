#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <functional>

// ============================================================
// 强连通分量（Strongly Connected Components, SCC）
// Kosaraju：两次 DFS，O(V+E)
// Tarjan：一次 DFS，栈 + low 值，O(V+E)
// 应用：有向图缩点、2-SAT、DAG 分析
// ============================================================

struct DiGraph {
    int n;
    std::vector<std::vector<int>> adj;   // 正向图
    std::vector<std::vector<int>> radj;  // 反向图

    explicit DiGraph(int n) : n(n), adj(n+1), radj(n+1) {}

    void add_edge(int u, int v) {
        adj[u].push_back(v);
        radj[v].push_back(u);
    }

    void print() const {
        for (int u = 1; u <= n; ++u) {
            if (adj[u].empty()) continue;
            std::cout << u << " -> ";
            for (int v : adj[u]) std::cout << v << " ";
            std::cout << "\n";
        }
    }
};

// ============================================================
// Kosaraju 算法
// 1. 正向图 DFS，记录完成时间（后序）压栈
// 2. 按栈顶顺序在反向图上 DFS，每次 DFS 的节点集合就是一个 SCC
// ============================================================
std::vector<std::vector<int>> kosaraju(const DiGraph& g) {
    int n = g.n;
    std::vector<bool> visited(n+1, false);
    std::stack<int>   finish_order;

    // 第一次 DFS：正向图，记录完成顺序
    std::function<void(int)> dfs1 = [&](int u) {
        visited[u] = true;
        for (int v : g.adj[u])
            if (!visited[v]) dfs1(v);
        finish_order.push(u);
    };
    for (int i = 1; i <= n; ++i)
        if (!visited[i]) dfs1(i);

    // 第二次 DFS：反向图，按完成时间逆序
    std::fill(visited.begin(), visited.end(), false);
    std::vector<std::vector<int>> sccs;

    std::function<void(int, std::vector<int>&)> dfs2 = [&](int u, std::vector<int>& scc) {
        visited[u] = true;
        scc.push_back(u);
        for (int v : g.radj[u])
            if (!visited[v]) dfs2(v, scc);
    };

    while (!finish_order.empty()) {
        int u = finish_order.top(); finish_order.pop();
        if (!visited[u]) {
            sccs.emplace_back();
            dfs2(u, sccs.back());
        }
    }
    return sccs;
}

void kosaraju_demo() {
    std::cout << "=== Kosaraju 强连通分量 ===\n";

    // 图：1->2->3->1（强连通）4->3, 4->5->6->4（强连通）
    DiGraph g(6);
    g.add_edge(1,2); g.add_edge(2,3); g.add_edge(3,1);  // SCC: {1,2,3}
    g.add_edge(4,3); g.add_edge(4,5);
    g.add_edge(5,6); g.add_edge(6,4);                   // SCC: {4,5,6}

    auto sccs = kosaraju(g);
    std::cout << "SCCs:\n";
    for (auto& scc : sccs) {
        std::cout << "  {";
        for (int i = 0; i < (int)scc.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << scc[i];
        }
        std::cout << "}\n";
    }
    std::cout << "count=" << sccs.size() << "\n";
}

// ============================================================
// Tarjan 算法
// 一次 DFS，维护：
//   disc[u]：发现时间戳
//   low[u]：u 所在 SCC 中最小的发现时间戳
//   on_stack[u]：u 是否在当前栈中
// 当 disc[u] == low[u] 时，弹出栈直到 u 为止，构成一个 SCC
// ============================================================
std::vector<std::vector<int>> tarjan(const DiGraph& g) {
    int n = g.n;
    std::vector<int>  disc(n+1, -1), low(n+1, 0);
    std::vector<bool> on_stack(n+1, false);
    std::stack<int>   stk;
    std::vector<std::vector<int>> sccs;
    int timer = 0;

    std::function<void(int)> dfs = [&](int u) {
        disc[u] = low[u] = timer++;
        stk.push(u);
        on_stack[u] = true;

        for (int v : g.adj[u]) {
            if (disc[v] == -1) {
                dfs(v);
                low[u] = std::min(low[u], low[v]);
            } else if (on_stack[v]) {
                // v 在栈中：是回边，更新 low
                low[u] = std::min(low[u], disc[v]);
            }
        }

        // u 是 SCC 的根
        if (low[u] == disc[u]) {
            sccs.emplace_back();
            while (true) {
                int v = stk.top(); stk.pop();
                on_stack[v] = false;
                sccs.back().push_back(v);
                if (v == u) break;
            }
        }
    };

    for (int i = 1; i <= n; ++i)
        if (disc[i] == -1) dfs(i);

    return sccs;
}

void tarjan_demo() {
    std::cout << "\n=== Tarjan 强连通分量 ===\n";

    DiGraph g(8);
    g.add_edge(1,2); g.add_edge(2,3); g.add_edge(3,1);  // SCC: {1,2,3}
    g.add_edge(3,4); g.add_edge(4,5); g.add_edge(5,4);  // SCC: {4,5}
    g.add_edge(5,6);                                     // SCC: {6}
    g.add_edge(7,8); g.add_edge(8,7);                   // SCC: {7,8}

    auto sccs = tarjan(g);
    std::cout << "SCCs:\n";
    for (auto& scc : sccs) {
        std::cout << "  {";
        for (int i = 0; i < (int)scc.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << scc[i];
        }
        std::cout << "}\n";
    }
    std::cout << "count=" << sccs.size() << "\n";
}

// ==================== 高级：SCC 缩点（condensation DAG）====================
void condensation_demo() {
    std::cout << "\n=== 高级：SCC 缩点构建 DAG ===\n";

    // 图：有向图，求缩点后的 DAG
    DiGraph g(6);
    g.add_edge(1,2); g.add_edge(2,1);  // SCC: {1,2}
    g.add_edge(2,3); g.add_edge(3,4);
    g.add_edge(4,5); g.add_edge(5,4);  // SCC: {4,5}
    g.add_edge(3,6); g.add_edge(6,3);  // SCC: {3,6}

    auto sccs = tarjan(g);
    int scc_cnt = (int)sccs.size();

    // 每个节点属于哪个 SCC（编号）
    std::vector<int> comp(g.n+1, 0);
    for (int i = 0; i < scc_cnt; ++i)
        for (int v : sccs[i]) comp[v] = i;

    // 构建缩点 DAG
    std::vector<std::vector<int>> dag(scc_cnt);
    for (int u = 1; u <= g.n; ++u)
        for (int v : g.adj[u])
            if (comp[u] != comp[v])
                dag[comp[u]].push_back(comp[v]);

    // 去重
    for (auto& adj : dag) {
        std::sort(adj.begin(), adj.end());
        adj.erase(std::unique(adj.begin(), adj.end()), adj.end());
    }

    std::cout << "SCCs:\n";
    for (int i = 0; i < scc_cnt; ++i) {
        std::cout << "  SCC-" << i << ": {";
        for (int j = 0; j < (int)sccs[i].size(); ++j) {
            if (j) std::cout << ",";
            std::cout << sccs[i][j];
        }
        std::cout << "}\n";
    }

    std::cout << "condensation DAG:\n";
    for (int i = 0; i < scc_cnt; ++i) {
        if (dag[i].empty()) continue;
        std::cout << "  SCC-" << i << " -> ";
        for (int j : dag[i]) std::cout << "SCC-" << j << " ";
        std::cout << "\n";
    }
}

// ==================== 高级：Tarjan 求桥（无向图）====================
void tarjan_bridge_demo() {
    std::cout << "\n=== 高级：Tarjan 求桥（无向图）===\n";

    int n = 5;
    std::vector<std::vector<std::pair<int,int>>> adj(n+1);  // {邻居, 边id}
    int edge_id = 0;
    auto add = [&](int u, int v){
        adj[u].push_back({v, edge_id});
        adj[v].push_back({u, edge_id});
        ++edge_id;
    };

    add(1,2); add(2,3); add(3,4); add(4,5); add(3,5);
    // 桥：1-2, 2-3

    std::vector<int>  disc(n+1,-1), low(n+1,0);
    std::vector<std::pair<int,int>> bridges;
    int timer = 0;

    std::function<void(int,int)> dfs = [&](int u, int par_edge) {
        disc[u] = low[u] = timer++;
        for (auto [v, eid] : adj[u]) {
            if (eid == par_edge) continue;  // 跳过父边（处理重边）
            if (disc[v] == -1) {
                dfs(v, eid);
                low[u] = std::min(low[u], low[v]);
                if (low[v] > disc[u]) bridges.push_back({u, v});
            } else {
                low[u] = std::min(low[u], disc[v]);
            }
        }
    };

    for (int i = 1; i <= n; ++i)
        if (disc[i] == -1) dfs(i, -1);

    std::cout << "bridges: ";
    for (auto [u,v] : bridges) std::cout << u << "-" << v << " ";
    std::cout << "\n";
}

int main() {
    kosaraju_demo();
    tarjan_demo();
    condensation_demo();
    tarjan_bridge_demo();
    return 0;
}
