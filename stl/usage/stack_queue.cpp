#include <iostream>
#include <stack>
#include <queue>
#include <string>
#include <vector>
#include <functional>

// ============================================================
// std::stack / std::queue / std::priority_queue
// stack：LIFO，默认底层 deque
// queue：FIFO，默认底层 deque
// priority_queue：堆，默认最大堆，底层 vector
// ============================================================

// ==================== 入门：stack 基础 ====================
void stack_demo() {
    std::cout << "=== 入门：stack ===\n";

    std::stack<int> st;
    st.push(1); st.push(2); st.push(3);

    std::cout << "top=" << st.top() << " size=" << st.size() << "\n";

    while (!st.empty()) {
        std::cout << st.top() << " ";
        st.pop();
    }
    std::cout << "\n";

    // 括号匹配
    auto is_valid = [](const std::string& s) {
        std::stack<char> st2;
        for (char c : s) {
            if (c == '(' || c == '[' || c == '{') {
                st2.push(c);
            } else {
                if (st2.empty()) return false;
                char top = st2.top(); st2.pop();
                if ((c == ')' && top != '(') ||
                    (c == ']' && top != '[') ||
                    (c == '}' && top != '{')) return false;
            }
        }
        return st2.empty();
    };

    std::cout << "\"()[]{}\" valid: " << std::boolalpha << is_valid("()[]{}") << "\n";
    std::cout << "\"([)]\" valid:   " << is_valid("([)]") << "\n";
    std::cout << "\"{[]}\" valid:   " << is_valid("{[]}") << "\n";

    // 使用 vector 作为底层容器
    std::stack<int, std::vector<int>> vec_stack;
    vec_stack.push(10); vec_stack.push(20);
    std::cout << "vector-backed stack top=" << vec_stack.top() << "\n";
}

// ==================== 入门：queue 基础 ====================
void queue_demo() {
    std::cout << "\n=== 入门：queue ===\n";

    std::queue<std::string> q;
    q.push("task1"); q.push("task2"); q.push("task3");

    std::cout << "front=" << q.front() << " back=" << q.back()
              << " size=" << q.size() << "\n";

    while (!q.empty()) {
        std::cout << "process: " << q.front() << "\n";
        q.pop();
    }

    // BFS 示例：树的层序遍历
    std::cout << "\nBFS 层序遍历（邻接表）:\n";
    // 图：0->1,2  1->3,4  2->5
    std::vector<std::vector<int>> adj{{1,2},{3,4},{5},{},{},{}};
    std::vector<bool> visited(6, false);

    std::queue<int> bfs;
    bfs.push(0); visited[0] = true;
    while (!bfs.empty()) {
        int node = bfs.front(); bfs.pop();
        std::cout << node << " ";
        for (int nb : adj[node]) {
            if (!visited[nb]) { visited[nb] = true; bfs.push(nb); }
        }
    }
    std::cout << "\n";
}

// ==================== 中级：priority_queue 基础 ====================
void pq_demo() {
    std::cout << "\n=== 中级：priority_queue ===\n";

    // 默认最大堆
    std::priority_queue<int> max_pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6})
        max_pq.push(x);

    std::cout << "max_heap pop order: ";
    while (!max_pq.empty()) {
        std::cout << max_pq.top() << " ";
        max_pq.pop();
    }
    std::cout << "\n";

    // 最小堆
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_pq;
    for (int x : {3, 1, 4, 1, 5, 9, 2, 6})
        min_pq.push(x);

    std::cout << "min_heap pop order: ";
    while (!min_pq.empty()) {
        std::cout << min_pq.top() << " ";
        min_pq.pop();
    }
    std::cout << "\n";
}

// ==================== 中级：priority_queue 自定义比较 ====================
struct Task {
    int priority;
    std::string name;
    bool operator<(const Task& o) const { return priority < o.priority; }  // 小的优先级低
};

void pq_custom_demo() {
    std::cout << "\n=== 中级：priority_queue 自定义类型 ===\n";

    std::priority_queue<Task> pq;
    pq.push({3, "low priority task"});
    pq.push({9, "critical task"});
    pq.push({6, "normal task"});
    pq.push({1, "idle task"});

    std::cout << "task execution order:\n";
    while (!pq.empty()) {
        auto t = pq.top(); pq.pop();
        std::cout << "  [" << t.priority << "] " << t.name << "\n";
    }

    // lambda 比较器
    auto cmp = [](const std::pair<int,std::string>& a,
                  const std::pair<int,std::string>& b){
        return a.first > b.first;  // 小值优先（最小堆）
    };
    std::priority_queue<std::pair<int,std::string>,
                        std::vector<std::pair<int,std::string>>,
                        decltype(cmp)> min_task(cmp);
    min_task.push({5, "medium"});
    min_task.push({1, "urgent"});
    min_task.push({9, "later"});

    std::cout << "min-priority order:\n";
    while (!min_task.empty()) {
        auto [p, n] = min_task.top(); min_task.pop();
        std::cout << "  [" << p << "] " << n << "\n";
    }
}

// ==================== 高级：Top-K 问题 ====================
void topk_demo() {
    std::cout << "\n=== 高级：Top-K 最大值（最小堆维护）===\n";

    std::vector<int> nums{3,1,4,1,5,9,2,6,5,3,5,8,9,7,9};
    int k = 4;

    // 用大小为 k 的最小堆，保留最大的 k 个
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
    for (int x : nums) {
        min_heap.push(x);
        if ((int)min_heap.size() > k) min_heap.pop();
    }

    std::vector<int> topk;
    while (!min_heap.empty()) { topk.push_back(min_heap.top()); min_heap.pop(); }
    std::sort(topk.rbegin(), topk.rend());

    std::cout << "top-" << k << ": ";
    for (int x : topk) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：Dijkstra 最短路 ====================
void dijkstra_demo() {
    std::cout << "\n=== 高级：Dijkstra 最短路（priority_queue）===\n";

    // 图：{邻居, 权重}
    int n = 5;
    std::vector<std::vector<std::pair<int,int>>> adj(n);
    auto add_edge = [&](int u, int v, int w){
        adj[u].push_back({v, w});
        adj[v].push_back({u, w});
    };
    add_edge(0,1,4); add_edge(0,2,1); add_edge(2,1,2);
    add_edge(1,3,1); add_edge(2,3,5); add_edge(3,4,3);

    std::vector<int> dist(n, INT_MAX);
    dist[0] = 0;

    // {dist, node}，最小堆
    std::priority_queue<std::pair<int,int>,
                        std::vector<std::pair<int,int>>,
                        std::greater<>> pq;
    pq.push({0, 0});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d > dist[u]) continue;  // 过期条目
        for (auto [v, w] : adj[u]) {
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                pq.push({dist[v], v});
            }
        }
    }

    std::cout << "shortest dist from 0: ";
    for (int i = 0; i < n; ++i)
        std::cout << "to " << i << "=" << dist[i] << " ";
    std::cout << "\n";
}

int main() {
    stack_demo();
    queue_demo();
    pq_demo();
    pq_custom_demo();
    topk_demo();
    dijkstra_demo();
    return 0;
}
