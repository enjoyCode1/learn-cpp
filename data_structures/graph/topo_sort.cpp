#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <functional>
#include <stdexcept>

// ============================================================
// 拓扑排序（Topological Sort）
// 仅适用于有向无环图（DAG）
// 应用：任务调度、编译依赖、课程先修关系
// 两种算法：
//   1. Kahn 算法（BFS + 入度）
//   2. DFS 后序逆置
// ============================================================

class TopoGraph {
public:
    explicit TopoGraph(int v) : V_(v), adj_(v), indegree_(v, 0) {}

    void add_edge(int u, int v) {
        adj_[u].push_back(v);
        ++indegree_[v];
    }

    // ---- Kahn 算法（BFS）----
    // 每次选入度为 0 的节点加入结果，O(V+E)
    std::vector<int> kahn_topo() const {
        std::vector<int> indeg = indegree_;  // 工作副本
        std::queue<int>  q;
        for (int i = 0; i < V_; ++i)
            if (indeg[i] == 0) q.push(i);

        std::vector<int> order;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (int v : adj_[u]) {
                if (--indeg[v] == 0) q.push(v);
            }
        }

        if ((int)order.size() != V_)
            throw std::runtime_error("graph has a cycle, no topological order");
        return order;
    }

    // ---- DFS 后序逆置 ----
    // 递归 DFS，节点完成后压栈，最后逆置得到拓扑序，O(V+E)
    std::vector<int> dfs_topo() const {
        std::vector<int>  color(V_, 0);  // 0 白, 1 灰(递归中), 2 黑(完成)
        std::stack<int>   st;
        bool              has_cycle = false;

        std::function<void(int)> dfs = [&](int u) {
            if (has_cycle) return;
            color[u] = 1;
            for (int v : adj_[u]) {
                if (color[v] == 1) { has_cycle = true; return; }
                if (color[v] == 0) dfs(v);
            }
            color[u] = 2;
            st.push(u);
        };

        for (int i = 0; i < V_; ++i)
            if (color[i] == 0) dfs(i);

        if (has_cycle)
            throw std::runtime_error("graph has a cycle, no topological order");

        std::vector<int> order;
        while (!st.empty()) { order.push_back(st.top()); st.pop(); }
        return order;
    }

    // 检测是否有环（利用 Kahn：如果拓扑序长度 < V 则有环）
    bool has_cycle() const {
        try { kahn_topo(); return false; }
        catch (...) { return true; }
    }

    void print() const {
        for (int u = 0; u < V_; ++u) {
            std::cout << u << " -> ";
            for (int v : adj_[u]) std::cout << v << " ";
            std::cout << "\n";
        }
    }

private:
    int                          V_;
    std::vector<std::vector<int>> adj_;
    std::vector<int>             indegree_;
};

// ==================== 入门：课程先修 ====================
void course_demo() {
    std::cout << "=== 入门：课程先修关系（拓扑排序）===\n";

    // 节点含义：
    // 0=数学基础, 1=线代, 2=微积分, 3=概率论
    // 4=机器学习, 5=深度学习
    // 边 u->v 表示 u 是 v 的先修课
    TopoGraph g(6);
    g.add_edge(0, 1);  // 数学基础 -> 线代
    g.add_edge(0, 2);  // 数学基础 -> 微积分
    g.add_edge(1, 3);  // 线代 -> 概率论
    g.add_edge(2, 3);  // 微积分 -> 概率论
    g.add_edge(3, 4);  // 概率论 -> 机器学习
    g.add_edge(4, 5);  // 机器学习 -> 深度学习

    std::vector<std::string> names{
        "数学基础", "线代", "微积分", "概率论", "机器学习", "深度学习"
    };

    std::cout << "依赖图:\n"; g.print();

    std::cout << "Kahn 拓扑序: ";
    for (int v : g.kahn_topo()) std::cout << names[v] << " -> ";
    std::cout << "END\n";

    std::cout << "DFS  拓扑序: ";
    for (int v : g.dfs_topo()) std::cout << names[v] << " -> ";
    std::cout << "END\n";
}

// ==================== 中级：任务调度（并行层次）====================
// 分层拓扑排序：同一层的任务可以并行执行
std::vector<std::vector<int>> layered_topo(int n,
        const std::vector<std::pair<int,int>>& deps) {
    std::vector<std::vector<int>> adj(n);
    std::vector<int> indeg(n, 0);
    for (auto [u, v] : deps) { adj[u].push_back(v); ++indeg[v]; }

    std::queue<int> q;
    for (int i = 0; i < n; ++i) if (indeg[i] == 0) q.push(i);

    std::vector<std::vector<int>> layers;
    while (!q.empty()) {
        int cnt = (int)q.size();
        std::vector<int> layer;
        for (int i = 0; i < cnt; ++i) {
            int u = q.front(); q.pop();
            layer.push_back(u);
            for (int v : adj[u]) if (--indeg[v] == 0) q.push(v);
        }
        layers.push_back(layer);
    }
    return layers;
}

void parallel_tasks_demo() {
    std::cout << "\n=== 中级：并行任务调度（分层拓扑）===\n";

    // 任务 0-5，deps[i]=(u,v) 表示 u 完成后才能开始 v
    int n = 6;
    std::vector<std::pair<int,int>> deps{
        {0,2},{0,3},{1,3},{1,4},{2,5},{3,5},{4,5}
    };

    auto layers = layered_topo(n, deps);
    std::cout << "execution plan (tasks in same layer run in parallel):\n";
    for (int i = 0; i < (int)layers.size(); ++i) {
        std::cout << "  wave " << i << ": ";
        for (int v : layers[i]) std::cout << "task-" << v << " ";
        std::cout << "\n";
    }
}

// ==================== 高级：有向图环检测 ====================
void cycle_demo() {
    std::cout << "\n=== 高级：有向图环检测 ===\n";

    // 无环 DAG
    TopoGraph dag(4);
    dag.add_edge(0,1); dag.add_edge(1,2); dag.add_edge(2,3);
    std::cout << "DAG has cycle: " << std::boolalpha << dag.has_cycle() << "\n";

    // 有环图
    TopoGraph cyclic(4);
    cyclic.add_edge(0,1); cyclic.add_edge(1,2);
    cyclic.add_edge(2,3); cyclic.add_edge(3,1);  // 1->2->3->1 成环
    std::cout << "cyclic graph has cycle: " << cyclic.has_cycle() << "\n";

    // 尝试对有环图排序
    try {
        cyclic.kahn_topo();
    } catch (const std::exception& e) {
        std::cout << "kahn_topo exception: " << e.what() << "\n";
    }
}

int main() {
    course_demo();
    parallel_tasks_demo();
    cycle_demo();
    return 0;
}
