#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <climits>
#include <algorithm>

// ============================================================
// 邻接矩阵图实现
// 适合稠密图，空间 O(V^2)，边查询 O(1)
// 支持有向/无向、带权/不带权图
// ============================================================

class AdjMatrix {
public:
    explicit AdjMatrix(int v, bool directed = false)
        : V_(v), directed_(directed),
          matrix_(v, std::vector<int>(v, 0)) {}

    // 添加边（不带权）
    void add_edge(int u, int v) {
        matrix_[u][v] = 1;
        if (!directed_) matrix_[v][u] = 1;
    }

    // 添加带权边
    void add_weighted_edge(int u, int v, int w) {
        matrix_[u][v] = w;
        if (!directed_) matrix_[v][u] = w;
    }

    // 删除边
    void remove_edge(int u, int v) {
        matrix_[u][v] = 0;
        if (!directed_) matrix_[v][u] = 0;
    }

    bool has_edge(int u, int v) const { return matrix_[u][v] != 0; }
    int  weight(int u, int v)   const { return matrix_[u][v]; }

    // BFS 遍历，O(V^2)
    std::vector<int> bfs(int start) const {
        std::vector<int>  visited(V_, false);
        std::vector<int>  order;
        std::queue<int>   q;

        visited[start] = true;
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (int v = 0; v < V_; ++v) {
                if (matrix_[u][v] && !visited[v]) {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }
        return order;
    }

    // DFS 遍历（迭代），O(V^2)
    std::vector<int> dfs(int start) const {
        std::vector<bool> visited(V_, false);
        std::vector<int>  order;
        std::stack<int>   st;

        st.push(start);
        while (!st.empty()) {
            int u = st.top(); st.pop();
            if (visited[u]) continue;
            visited[u] = true;
            order.push_back(u);
            // 逆序压栈保证访问顺序一致
            for (int v = V_ - 1; v >= 0; --v)
                if (matrix_[u][v] && !visited[v])
                    st.push(v);
        }
        return order;
    }

    // Dijkstra 最短路，O(V^2)（邻接矩阵版）
    std::vector<int> dijkstra(int src) const {
        std::vector<int>  dist(V_, INT_MAX);
        std::vector<bool> done(V_, false);
        dist[src] = 0;

        for (int iter = 0; iter < V_; ++iter) {
            // 选取未处理中距离最小的节点
            int u = -1;
            for (int v = 0; v < V_; ++v)
                if (!done[v] && (u == -1 || dist[v] < dist[u]))
                    u = v;
            if (u == -1 || dist[u] == INT_MAX) break;
            done[u] = true;

            for (int v = 0; v < V_; ++v) {
                if (matrix_[u][v] && dist[u] != INT_MAX &&
                    dist[u] + matrix_[u][v] < dist[v])
                    dist[v] = dist[u] + matrix_[u][v];
            }
        }
        return dist;
    }

    // Floyd-Warshall 全源最短路，O(V^3)
    std::vector<std::vector<int>> floyd_warshall() const {
        auto dist = matrix_;
        // 对角线设为 0，0 权边设为 INF
        for (int i = 0; i < V_; ++i) {
            dist[i][i] = 0;
            for (int j = 0; j < V_; ++j)
                if (i != j && dist[i][j] == 0) dist[i][j] = INT_MAX / 2;
        }
        for (int k = 0; k < V_; ++k)
            for (int i = 0; i < V_; ++i)
                for (int j = 0; j < V_; ++j)
                    if (dist[i][k] + dist[k][j] < dist[i][j])
                        dist[i][j] = dist[i][k] + dist[k][j];
        return dist;
    }

    void print() const {
        std::cout << "  ";
        for (int i = 0; i < V_; ++i) std::cout << i << " ";
        std::cout << "\n";
        for (int i = 0; i < V_; ++i) {
            std::cout << i << " ";
            for (int j = 0; j < V_; ++j)
                std::cout << matrix_[i][j] << " ";
            std::cout << "\n";
        }
    }

    int vertices() const { return V_; }

private:
    int                          V_;
    bool                         directed_;
    std::vector<std::vector<int>> matrix_;
};

// ==================== 入门：无向图遍历 ====================
void traversal_demo() {
    std::cout << "=== 入门：无向图 BFS/DFS ===\n";

    //  0 - 1 - 2
    //  |   |
    //  3 - 4
    AdjMatrix g(5, false);
    g.add_edge(0, 1); g.add_edge(1, 2);
    g.add_edge(0, 3); g.add_edge(1, 4); g.add_edge(3, 4);

    std::cout << "adjacency matrix:\n"; g.print();

    std::cout << "BFS from 0: ";
    for (int v : g.bfs(0)) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "DFS from 0: ";
    for (int v : g.dfs(0)) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 中级：Dijkstra 最短路 ====================
void dijkstra_demo() {
    std::cout << "\n=== 中级：Dijkstra 最短路 ===\n";

    //  0 --1-- 1 --2-- 2
    //  |               |
    //  4               1
    //  |               |
    //  3 ------3------ 4
    AdjMatrix g(5, false);
    g.add_weighted_edge(0, 1, 1);
    g.add_weighted_edge(1, 2, 2);
    g.add_weighted_edge(0, 3, 4);
    g.add_weighted_edge(2, 4, 1);
    g.add_weighted_edge(3, 4, 3);

    auto dist = g.dijkstra(0);
    std::cout << "shortest distances from vertex 0:\n";
    for (int i = 0; i < (int)dist.size(); ++i)
        std::cout << "  0 -> " << i << " = " << dist[i] << "\n";
}

// ==================== 高级：Floyd-Warshall 全源最短路 ====================
void floyd_demo() {
    std::cout << "\n=== 高级：Floyd-Warshall 全源最短路 ===\n";

    AdjMatrix g(4, true);
    g.add_weighted_edge(0, 1, 3);
    g.add_weighted_edge(0, 3, 7);
    g.add_weighted_edge(1, 2, 2);
    g.add_weighted_edge(2, 3, 1);
    g.add_weighted_edge(1, 3, 5);

    auto dist = g.floyd_warshall();
    std::cout << "all-pairs shortest path:\n";
    std::cout << "    ";
    for (int j = 0; j < 4; ++j) std::cout << j << "   ";
    std::cout << "\n";
    for (int i = 0; i < 4; ++i) {
        std::cout << i << " [ ";
        for (int j = 0; j < 4; ++j) {
            if (dist[i][j] >= INT_MAX / 2) std::cout << "INF ";
            else std::cout << dist[i][j] << "   ";
        }
        std::cout << "]\n";
    }
}

int main() {
    traversal_demo();
    dijkstra_demo();
    floyd_demo();
    return 0;
}
