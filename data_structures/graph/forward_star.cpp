#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>

// ============================================================
// 前向星（Forward Star）/ 链式前向星
// 用数组模拟邻接表，内存连续，缓存友好
// 竞赛算法最常用的图存储方式
// head[u]：u 的第一条边的索引
// next[i]：第 i 条边的下一条同起点边的索引
// to[i]：第 i 条边的终点
// w[i]：第 i 条边的权重
// ============================================================

struct ForwardStar {
    int n, m;               // 节点数，边数
    std::vector<int> head;  // head[u] = 从 u 出发的第一条边的索引，-1 表示无边
    std::vector<int> next;  // next[i] = 第 i 条边的下一条同起点边
    std::vector<int> to;    // to[i]   = 第 i 条边的终点
    std::vector<int> w;     // w[i]    = 第 i 条边的权重
    int cnt;                // 当前边计数

    ForwardStar(int n, int max_edges)
        : n(n), m(0), head(n + 1, -1),
          next(max_edges), to(max_edges), w(max_edges), cnt(0) {}

    // 添加有向边 u->v，权重 wt，O(1)
    void add_edge(int u, int v, int wt = 1) {
        to[cnt]   = v;
        w[cnt]    = wt;
        next[cnt] = head[u];  // 新边插入链表头部
        head[u]   = cnt++;
        ++m;
    }

    // 添加无向边（两条有向边）
    void add_undirected(int u, int v, int wt = 1) {
        add_edge(u, v, wt);
        add_edge(v, u, wt);
    }

    // 遍历 u 的所有出边
    void for_each_edge(int u, std::function<void(int to, int w)> fn) const {
        for (int i = head[u]; i != -1; i = next[i])
            fn(to[i], w[i]);
    }

    void print() const {
        for (int u = 1; u <= n; ++u) {
            std::cout << u << " -> ";
            for (int i = head[u]; i != -1; i = next[i])
                std::cout << to[i] << "(w=" << w[i] << ") ";
            std::cout << "\n";
        }
    }
};

// ==================== 入门：基础构建与遍历 ====================
void basic_demo() {
    std::cout << "=== 入门：前向星基础 ===\n";

    // 构建图：
    // 1->2(1), 1->3(4), 2->3(2), 2->4(5), 3->4(1)
    ForwardStar g(4, 10);
    g.add_edge(1, 2, 1);
    g.add_edge(1, 3, 4);
    g.add_edge(2, 3, 2);
    g.add_edge(2, 4, 5);
    g.add_edge(3, 4, 1);

    std::cout << "adjacency (forward star):\n";
    g.print();

    std::cout << "edges from node 2: ";
    g.for_each_edge(2, [](int v, int wt){
        std::cout << "->" << v << "(w=" << wt << ") ";
    });
    std::cout << "\n";
    std::cout << "total edges=" << g.m << "\n";
}

// ==================== 中级：BFS 用前向星 ====================
void bfs_demo() {
    std::cout << "\n=== 中级：BFS（前向星）===\n";

    ForwardStar g(6, 20);
    g.add_undirected(1, 2);
    g.add_undirected(1, 3);
    g.add_undirected(2, 4);
    g.add_undirected(2, 5);
    g.add_undirected(3, 6);

    std::vector<int>  dist(g.n + 1, -1);
    std::vector<int>  queue(g.n + 1);
    int front = 0, back = 0;

    dist[1] = 0;
    queue[back++] = 1;

    while (front < back) {
        int u = queue[front++];
        for (int i = g.head[u]; i != -1; i = g.next[i]) {
            int v = g.to[i];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                queue[back++] = v;
            }
        }
    }

    std::cout << "BFS distances from 1:\n";
    for (int i = 1; i <= g.n; ++i)
        std::cout << "  dist[" << i << "]=" << dist[i] << "\n";
}

// ==================== 中级：Dijkstra 用前向星 ====================
void dijkstra_demo() {
    std::cout << "\n=== 中级：Dijkstra（前向星）===\n";

    ForwardStar g(5, 20);
    g.add_edge(1, 2, 4); g.add_edge(1, 3, 1);
    g.add_edge(3, 2, 2); g.add_edge(2, 4, 1);
    g.add_edge(3, 4, 5); g.add_edge(4, 5, 3);

    const int INF = 1e9;
    std::vector<int>  dist(g.n + 1, INF);
    std::vector<bool> vis(g.n + 1, false);
    dist[1] = 0;

    // 简单 O(V²) Dijkstra（适合稠密图）
    for (int iter = 0; iter < g.n; ++iter) {
        int u = -1;
        for (int i = 1; i <= g.n; ++i)
            if (!vis[i] && (u == -1 || dist[i] < dist[u])) u = i;
        if (u == -1 || dist[u] == INF) break;
        vis[u] = true;

        for (int i = g.head[u]; i != -1; i = g.next[i]) {
            int v = g.to[i], wt = g.w[i];
            if (dist[u] + wt < dist[v]) dist[v] = dist[u] + wt;
        }
    }

    std::cout << "shortest dist from 1:\n";
    for (int i = 1; i <= g.n; ++i)
        std::cout << "  dist[" << i << "]=" << (dist[i] == INF ? -1 : dist[i]) << "\n";
}

// ==================== 高级：前向星 + 反向图（SCC 预处理）====================
void reverse_graph_demo() {
    std::cout << "\n=== 高级：构建反向图 ===\n";

    ForwardStar g(4, 10), rg(4, 10);
    g.add_edge(1, 2); g.add_edge(2, 3);
    g.add_edge(3, 4); g.add_edge(4, 2);

    // 构建反向图
    for (int u = 1; u <= g.n; ++u)
        for (int i = g.head[u]; i != -1; i = g.next[i])
            rg.add_edge(g.to[i], u, g.w[i]);

    std::cout << "original:\n"; g.print();
    std::cout << "reversed:\n"; rg.print();
}

// ==================== 高级：前向星存储多重边 ====================
void multigraph_demo() {
    std::cout << "\n=== 高级：多重边（平行边）===\n";

    // 前向星天然支持平行边（邻接矩阵不行）
    ForwardStar g(3, 10);
    g.add_edge(1, 2, 5);
    g.add_edge(1, 2, 3);  // 1->2 有两条边，权重不同
    g.add_edge(2, 3, 1);
    g.add_edge(1, 3, 10);

    std::cout << "multigraph:\n";
    g.print();

    // 找 1->2 的最小权边
    int min_w = INT_MAX;
    for (int i = g.head[1]; i != -1; i = g.next[i])
        if (g.to[i] == 2) min_w = std::min(min_w, g.w[i]);
    std::cout << "min edge weight 1->2: " << min_w << "\n";
}

int main() {
    basic_demo();
    bfs_demo();
    dijkstra_demo();
    reverse_graph_demo();
    multigraph_demo();
    return 0;
}
