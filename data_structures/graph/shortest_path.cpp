#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>
#include <cmath>
#include <limits>

// ============================================================
// 最短路径算法
// Dijkstra：单源，非负权，O((V+E) log V)
// Bellman-Ford：单源，可负权，检测负环，O(VE)
// Floyd-Warshall：全源，O(V³)
// A*：启发式，单源单目标，适合有坐标的图
// ============================================================

const int INF = 1e9;

struct Edge { int to, w; };
using Graph = std::vector<std::vector<Edge>>;

Graph make_graph(int n) { return Graph(n + 1); }
void add_edge(Graph& g, int u, int v, int w) { g[u].push_back({v, w}); }
void add_undirected(Graph& g, int u, int v, int w) {
    add_edge(g, u, v, w); add_edge(g, v, u, w);
}

// ==================== Dijkstra（优先队列）====================
std::vector<int> dijkstra(const Graph& g, int src) {
    int n = (int)g.size() - 1;
    std::vector<int> dist(n + 1, INF);
    // {dist, node}，最小堆
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;  // 过期条目
        for (auto [v, w] : g[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }
    return dist;
}

void dijkstra_demo() {
    std::cout << "=== Dijkstra 单源最短路 ===\n";

    // 图：
    //   1 -4- 2
    //   |     |
    //   1     2
    //   |     |
    //   3 -5- 4
    //    \   /
    //     1-1
    int n = 5;
    Graph g = make_graph(n);
    add_edge(g, 1, 2, 4); add_edge(g, 1, 3, 1);
    add_edge(g, 3, 2, 2); add_edge(g, 2, 4, 1);
    add_edge(g, 3, 4, 5); add_edge(g, 4, 5, 3);

    auto dist = dijkstra(g, 1);
    std::cout << "shortest dist from 1:\n";
    for (int i = 1; i <= n; ++i)
        std::cout << "  to " << i << " = " << (dist[i] == INF ? -1 : dist[i]) << "\n";

    // 带路径重建
    std::vector<int> prev(n + 1, -1);
    std::vector<int> d2(n + 1, INF);
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    d2[1] = 0; pq.push({0, 1});
    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > d2[u]) continue;
        for (auto [v, w] : g[u]) {
            if (d2[u] + w < d2[v]) {
                d2[v] = d2[u] + w;
                prev[v] = u;
                pq.push({d2[v], v});
            }
        }
    }

    std::cout << "path 1->5: ";
    std::vector<int> path;
    for (int v = 5; v != -1; v = prev[v]) path.push_back(v);
    std::reverse(path.begin(), path.end());
    for (int v : path) std::cout << v << " ";
    std::cout << "(dist=" << d2[5] << ")\n";
}

// ==================== Bellman-Ford ====================
// 返回 dist，若存在负环返回空 vector
std::vector<int> bellman_ford(int n,
    const std::vector<std::tuple<int,int,int>>& edges, int src) {
    std::vector<int> dist(n + 1, INF);
    dist[src] = 0;

    // 松弛 V-1 次
    for (int i = 0; i < n - 1; ++i) {
        bool updated = false;
        for (auto [u, v, w] : edges) {
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                updated = true;
            }
        }
        if (!updated) break;  // 提前退出
    }

    // 第 V 次松弛：检测负环
    for (auto [u, v, w] : edges) {
        if (dist[u] != INF && dist[u] + w < dist[v])
            return {};  // 存在负环
    }
    return dist;
}

void bellman_ford_demo() {
    std::cout << "\n=== Bellman-Ford（支持负权边）===\n";

    int n = 5;
    std::vector<std::tuple<int,int,int>> edges{
        {1,2,4},{1,3,1},{3,2,-2},{2,4,1},{3,4,5},{4,5,3}
    };

    auto dist = bellman_ford(n, edges, 1);
    if (dist.empty()) {
        std::cout << "negative cycle detected!\n";
    } else {
        std::cout << "shortest dist from 1:\n";
        for (int i = 1; i <= n; ++i)
            std::cout << "  to " << i << " = " << (dist[i] == INF ? -1 : dist[i]) << "\n";
    }

    // 含负环的图
    std::vector<std::tuple<int,int,int>> neg_cycle{
        {1,2,1},{2,3,1},{3,1,-3}
    };
    auto d2 = bellman_ford(3, neg_cycle, 1);
    std::cout << "graph with negative cycle: "
              << (d2.empty() ? "detected!" : "no cycle") << "\n";
}

// ==================== Floyd-Warshall（全源最短路）====================
void floyd_demo() {
    std::cout << "\n=== Floyd-Warshall 全源最短路 ===\n";

    int n = 4;
    // dist[i][j] 初始化
    std::vector<std::vector<int>> dist(n + 1, std::vector<int>(n + 1, INF));
    for (int i = 1; i <= n; ++i) dist[i][i] = 0;

    auto add = [&](int u, int v, int w){ dist[u][v] = w; };
    add(1,2,3); add(1,3,8); add(1,4,INF);
    add(2,1,INF); add(2,3,INF); add(2,4,1);
    add(3,1,INF); add(3,2,4); add(3,4,INF);
    add(4,1,2); add(4,2,INF); add(4,3,5);

    // Floyd 三重循环
    for (int k = 1; k <= n; ++k)
        for (int i = 1; i <= n; ++i)
            for (int j = 1; j <= n; ++j)
                if (dist[i][k] != INF && dist[k][j] != INF)
                    dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);

    std::cout << "all-pairs shortest path matrix:\n";
    std::cout << "     ";
    for (int j = 1; j <= n; ++j) std::cout << "  " << j;
    std::cout << "\n";
    for (int i = 1; i <= n; ++i) {
        std::cout << "  " << i << ": ";
        for (int j = 1; j <= n; ++j) {
            if (dist[i][j] == INF) std::cout << "  ∞";
            else std::cout << "  " << dist[i][j];
        }
        std::cout << "\n";
    }

    // 检测负环：对角线出现负值
    bool neg = false;
    for (int i = 1; i <= n; ++i) if (dist[i][i] < 0) neg = true;
    std::cout << "negative cycle: " << std::boolalpha << neg << "\n";
}

// ==================== A*（启发式最短路）====================
struct Point { int x, y; };

// 曼哈顿距离作为启发函数（h）
int heuristic(Point a, Point b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

void astar_demo() {
    std::cout << "\n=== A* 启发式最短路（网格图）===\n";

    // 5x5 网格，0=可通，1=障碍
    const int ROWS = 5, COLS = 5;
    int grid[ROWS][COLS] = {
        {0,0,0,0,0},
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,1,0,0,0},
        {0,0,0,0,0}
    };

    Point start{0,0}, goal{4,4};
    int dx[] = {0,0,1,-1};
    int dy[] = {1,-1,0,0};

    std::vector<std::vector<int>>  g_cost(ROWS, std::vector<int>(COLS, INF));
    std::vector<std::vector<Point>> parent(ROWS, std::vector<Point>(COLS, {-1,-1}));
    g_cost[start.x][start.y] = 0;

    // {f=g+h, x, y}
    using State = std::tuple<int,int,int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> open;
    open.push({heuristic(start, goal), start.x, start.y});

    while (!open.empty()) {
        auto [f, x, y] = open.top(); open.pop();
        if (x == goal.x && y == goal.y) break;

        int g = g_cost[x][y];
        if (f > g + heuristic({x,y}, goal)) continue;  // 过期条目

        for (int d = 0; d < 4; ++d) {
            int nx = x + dx[d], ny = y + dy[d];
            if (nx < 0 || nx >= ROWS || ny < 0 || ny >= COLS) continue;
            if (grid[nx][ny]) continue;
            int ng = g + 1;
            if (ng < g_cost[nx][ny]) {
                g_cost[nx][ny] = ng;
                parent[nx][ny] = {x, y};
                open.push({ng + heuristic({nx,ny}, goal), nx, ny});
            }
        }
    }

    if (g_cost[goal.x][goal.y] == INF) {
        std::cout << "no path found\n";
        return;
    }

    // 重建路径
    std::vector<Point> path;
    for (Point p = goal; p.x != -1; p = parent[p.x][p.y])
        path.push_back(p);
    std::reverse(path.begin(), path.end());

    // 打印网格路径
    std::vector<std::string> display(ROWS, std::string(COLS, '.'));
    for (int i = 0; i < ROWS; ++i)
        for (int j = 0; j < COLS; ++j)
            if (grid[i][j]) display[i][j] = '#';
    for (auto [x,y] : path) display[x][y] = '*';
    display[start.x][start.y] = 'S';
    display[goal.x][goal.y]   = 'G';

    std::cout << "grid (S=start G=goal #=wall *=path):\n";
    for (auto& row : display) std::cout << "  " << row << "\n";
    std::cout << "path length=" << g_cost[goal.x][goal.y] << "\n";
}

int main() {
    dijkstra_demo();
    bellman_ford_demo();
    floyd_demo();
    astar_demo();
    return 0;
}
