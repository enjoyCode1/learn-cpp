#include <iostream>
#include <stdexcept>
#include <vector>

// ============================================================
// 队列（Queue）实现
// FIFO：先进先出，基于循环数组实现，避免假溢出
// ============================================================

template<typename T>
class Queue {
public:
    explicit Queue(size_t cap = 8)
        : data_(cap + 1), head_(0), tail_(0), cap_(cap + 1) {}

    // 入队 O(1)
    void push(const T& val) {
        if (full()) grow();
        data_[tail_] = val;
        tail_ = (tail_ + 1) % cap_;
    }

    void push(T&& val) {
        if (full()) grow();
        data_[tail_] = std::move(val);
        tail_ = (tail_ + 1) % cap_;
    }

    // 出队 O(1)
    void pop() {
        if (empty()) throw std::underflow_error("Queue::pop on empty");
        head_ = (head_ + 1) % cap_;
    }

    // 查看队首，不出队
    T& front() {
        if (empty()) throw std::underflow_error("Queue::front on empty");
        return data_[head_];
    }
    const T& front() const {
        if (empty()) throw std::underflow_error("Queue::front on empty");
        return data_[head_];
    }

    T& back() {
        if (empty()) throw std::underflow_error("Queue::back on empty");
        return data_[(tail_ + cap_ - 1) % cap_];
    }

    size_t size()  const { return (tail_ + cap_ - head_) % cap_; }
    bool   empty() const { return head_ == tail_; }
    bool   full()  const { return (tail_ + 1) % cap_ == head_; }

    void print() const {
        std::cout << "front -> [";
        size_t i = head_, cnt = size();
        for (size_t k = 0; k < cnt; ++k) {
            std::cout << data_[(i + k) % cap_];
            if (k + 1 < cnt) std::cout << ", ";
        }
        std::cout << "] <- back  (size=" << cnt << ")\n";
    }

private:
    std::vector<T> data_;
    size_t head_, tail_, cap_;

    // 扩容：复制到新数组
    void grow() {
        size_t old_size = size();
        size_t new_cap  = cap_ * 2;
        std::vector<T> new_data(new_cap);
        for (size_t k = 0; k < old_size; ++k)
            new_data[k] = std::move(data_[(head_ + k) % cap_]);
        data_ = std::move(new_data);
        head_ = 0;
        tail_ = old_size;
        cap_  = new_cap;
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：Queue 基础操作 ===\n";

    Queue<int> q;
    for (int i = 1; i <= 5; ++i) q.push(i);
    q.print();

    std::cout << "front = " << q.front() << "\n";
    q.pop();
    std::cout << "after pop: "; q.print();

    q.push(6);
    std::cout << "after push(6): "; q.print();
}

// ==================== 中级：BFS 层序遍历（用队列） ====================
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

// 构建示例树:    1
//              / \
//             2   3
//            / \   \
//           4   5   6
TreeNode* build_tree() {
    auto* root = new TreeNode(1);
    root->left  = new TreeNode(2);
    root->right = new TreeNode(3);
    root->left->left  = new TreeNode(4);
    root->left->right = new TreeNode(5);
    root->right->right = new TreeNode(6);
    return root;
}

void bfs_demo() {
    std::cout << "\n=== 中级：BFS 层序遍历 ===\n";

    TreeNode* root = build_tree();
    Queue<TreeNode*> q;
    q.push(root);

    int level = 0;
    while (!q.empty()) {
        int cnt = (int)q.size();
        std::cout << "level " << level++ << ": ";
        for (int i = 0; i < cnt; ++i) {
            TreeNode* node = q.front(); q.pop();
            std::cout << node->val << " ";
            if (node->left)  q.push(node->left);
            if (node->right) q.push(node->right);
        }
        std::cout << "\n";
    }
}

// ==================== 高级：滑动窗口最大值（单调队列） ====================
// 用双端队列（deque）维护单调递减序列，O(n) 求每个窗口最大值
std::vector<int> sliding_window_max(const std::vector<int>& nums, int k) {
    std::vector<int> result;
    // 存下标的双端队列，队首始终是当前窗口最大值的下标
    std::vector<int> dq;  // 模拟 deque

    for (int i = 0; i < (int)nums.size(); ++i) {
        // 移除超出窗口范围的队首
        while (!dq.empty() && dq.front() < i - k + 1)
            dq.erase(dq.begin());

        // 维护单调递减：移除比当前小的元素
        while (!dq.empty() && nums[dq.back()] < nums[i])
            dq.pop_back();

        dq.push_back(i);

        // 窗口已满，记录最大值
        if (i >= k - 1)
            result.push_back(nums[dq.front()]);
    }
    return result;
}

void monotonic_queue_demo() {
    std::cout << "\n=== 高级：滑动窗口最大值（单调队列）===\n";

    std::vector<int> nums{1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;
    auto res = sliding_window_max(nums, k);

    std::cout << "nums: ";
    for (int v : nums) std::cout << v << " ";
    std::cout << "\nmax (k=" << k << "): ";
    for (int v : res) std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    bfs_demo();
    monotonic_queue_demo();
    return 0;
}
