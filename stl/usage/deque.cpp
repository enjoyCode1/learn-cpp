#include <iostream>
#include <deque>
#include <algorithm>
#include <string>
#include <vector>

std::vector<int> sliding_window_max(const std::vector<int>& nums, int k);

// ============================================================
// std::deque 从入门到高级用法
// 双端队列：两端插入/删除 O(1)，随机访问 O(1)
// 底层：分块数组，非连续内存
// ============================================================

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：deque 基础操作 ===\n";

    std::deque<int> dq{3, 4, 5};

    // 两端插入
    dq.push_front(2);
    dq.push_front(1);
    dq.push_back(6);
    dq.push_back(7);
    std::cout << "after push front/back: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";

    // 随机访问
    std::cout << "dq[0]=" << dq[0] << " dq[3]=" << dq[3] << "\n";
    std::cout << "front=" << dq.front() << " back=" << dq.back() << "\n";

    // 两端删除
    dq.pop_front();
    dq.pop_back();
    std::cout << "after pop front/back: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";

    // insert / erase（同 vector）
    dq.insert(dq.begin() + 2, 99);
    std::cout << "insert(99) at 2: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";

    dq.erase(dq.begin() + 2);
    std::cout << "erase pos 2: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：与算法配合 ====================
void algorithm_demo() {
    std::cout << "\n=== 中级：deque 与算法 ===\n";

    std::deque<int> dq{5, 3, 1, 4, 2};

    std::sort(dq.begin(), dq.end());
    std::cout << "sorted: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";

    // 二分查找
    std::cout << "binary_search(3)="
              << std::boolalpha << std::binary_search(dq.begin(), dq.end(), 3) << "\n";

    // deque 作为滑动窗口缓冲
    std::deque<int> window;
    std::vector<int> data{1, 3, 5, 2, 4};
    int k = 3;
    std::cout << "sliding window(k=" << k << ") sums: ";
    int wsum = 0;
    for (int i = 0; i < (int)data.size(); ++i) {
        window.push_back(data[i]);
        wsum += data[i];
        if ((int)window.size() > k) { wsum -= window.front(); window.pop_front(); }
        if ((int)window.size() == k) std::cout << wsum << " ";
    }
    std::cout << "\n";
}

// ==================== 高级：单调队列（滑动窗口最大值）====================
std::vector<int> sliding_window_max(const std::vector<int>& nums, int k) {
    std::deque<int> dq;  // 存储索引，单调递减
    std::vector<int> result;

    for (int i = 0; i < (int)nums.size(); ++i) {
        // 移除超出窗口的索引
        while (!dq.empty() && dq.front() < i - k + 1)
            dq.pop_front();
        // 维护单调递减：移除比当前元素小的尾部
        while (!dq.empty() && nums[dq.back()] < nums[i])
            dq.pop_back();
        dq.push_back(i);
        if (i >= k - 1)
            result.push_back(nums[dq.front()]);
    }
    return result;
}

void monotonic_queue_demo() {
    std::cout << "\n=== 高级：单调队列（滑动窗口最大值）===\n";

    std::vector<int> nums{1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;

    std::cout << "nums: ";
    for (int x : nums) std::cout << x << " ";
    std::cout << "\nk=" << k << " max values: ";

    auto result = sliding_window_max(nums, k);
    for (int x : result) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：deque 与 vector 性能对比 ====================
void performance_demo() {
    std::cout << "\n=== 高级：deque vs vector 头部插入 ===\n";

    // deque 头部插入 O(1)，vector 头部插入 O(n)
    std::deque<int> dq;
    for (int i = 0; i < 10; ++i) dq.push_front(i);
    std::cout << "deque push_front 0-9: ";
    for (int x : dq) std::cout << x << " ";
    std::cout << "\n";

    // deque 不保证内存连续，不能对 &dq[0] 取指针做指针算术
    // vector 保证连续，可以传给 C API
    std::cout << "注意：deque 内存不连续，不能用于需要连续内存的 C API\n";
}

int main() {
    basic_demo();
    algorithm_demo();
    monotonic_queue_demo();
    performance_demo();
    return 0;
}
