#include <iostream>
#include <vector>
#include <functional>
#include <stdexcept>

// ============================================================
// 堆（Heap）实现
// 最大堆：父节点 >= 子节点
// 基于数组存储，父节点 i 的左子 2i+1，右子 2i+2
// 插入/删除 O(log n)，建堆 O(n)，堆顶查看 O(1)
// ============================================================

template<typename T, typename Compare = std::less<T>>
class Heap {
public:
    Heap() = default;

    // 从数组建堆，O(n)
    explicit Heap(std::vector<T> arr) : data_(std::move(arr)) {
        // 从最后一个非叶节点向上 heapify
        for (int i = (int)data_.size() / 2 - 1; i >= 0; --i)
            sift_down(i);
    }

    // 插入，O(log n)
    void push(const T& val) {
        data_.push_back(val);
        sift_up((int)data_.size() - 1);
    }

    // 删除堆顶，O(log n)
    void pop() {
        if (empty()) throw std::underflow_error("Heap::pop on empty");
        std::swap(data_.front(), data_.back());
        data_.pop_back();
        if (!empty()) sift_down(0);
    }

    // 查看堆顶，O(1)
    const T& top() const {
        if (empty()) throw std::underflow_error("Heap::top on empty");
        return data_.front();
    }

    size_t size()  const { return data_.size(); }
    bool   empty() const { return data_.empty(); }

    // 堆排序（原地，升序），O(n log n)
    static std::vector<T> heap_sort(std::vector<T> arr) {
        // 建最大堆
        Heap<T, std::less<T>> h(arr);
        std::vector<T> res;
        res.reserve(arr.size());
        while (!h.empty()) {
            res.push_back(h.top());
            h.pop();
        }
        // 最大堆弹出是降序，反转得升序
        std::reverse(res.begin(), res.end());
        return res;
    }

    void print() const {
        std::cout << "heap: ";
        for (const auto& v : data_) std::cout << v << " ";
        std::cout << "(size=" << data_.size() << ")\n";
    }

private:
    std::vector<T> data_;
    Compare        cmp_;

    // 上浮：新插入节点与父节点比较，若"更优"则交换
    void sift_up(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (cmp_(data_[parent], data_[i])) {
                std::swap(data_[parent], data_[i]);
                i = parent;
            } else break;
        }
    }

    // 下沉：堆顶被替换后向下恢复堆性质
    void sift_down(int i) {
        int n = (int)data_.size();
        while (true) {
            int best = i;
            int l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && cmp_(data_[best], data_[l])) best = l;
            if (r < n && cmp_(data_[best], data_[r])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};

// 最小堆别名
template<typename T>
using MinHeap = Heap<T, std::greater<T>>;

// 最大堆别名
template<typename T>
using MaxHeap = Heap<T, std::less<T>>;

// ==================== 入门：最大堆基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：最大堆基础操作 ===\n";

    MaxHeap<int> h;
    for (int v : {3, 1, 4, 1, 5, 9, 2, 6}) h.push(v);
    h.print();

    std::cout << "top = " << h.top() << "\n";

    std::cout << "pop sequence: ";
    while (!h.empty()) {
        std::cout << h.top() << " ";
        h.pop();
    }
    std::cout << "\n";
}

// ==================== 中级：O(n) 建堆 + 堆排序 ====================
void build_sort_demo() {
    std::cout << "\n=== 中级：O(n) 建堆 + 堆排序 ===\n";

    std::vector<int> arr{5, 3, 8, 1, 9, 2, 7, 4, 6};
    MaxHeap<int> h(arr);
    std::cout << "build from array: "; h.print();

    auto sorted = MaxHeap<int>::heap_sort(arr);
    std::cout << "heap_sort result: ";
    for (int v : sorted) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 中级：最小堆求 Top-K 最大元素 ====================
std::vector<int> top_k(const std::vector<int>& nums, int k) {
    // 用大小为 k 的最小堆维护最大的 k 个元素
    MinHeap<int> h;
    for (int v : nums) {
        h.push(v);
        if ((int)h.size() > k) h.pop();  // 弹出最小的，保留最大的 k 个
    }
    std::vector<int> res;
    while (!h.empty()) { res.push_back(h.top()); h.pop(); }
    std::reverse(res.begin(), res.end());
    return res;
}

void topk_demo() {
    std::cout << "\n=== 中级：Top-K 最大元素（最小堆）===\n";

    std::vector<int> nums{3, 1, 5, 12, 2, 11, 7, 4, 9, 6, 8, 10};
    int k = 4;
    auto res = top_k(nums, k);

    std::cout << "nums: ";
    for (int v : nums) std::cout << v << " ";
    std::cout << "\ntop-" << k << ": ";
    for (int v : res) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 高级：合并 K 个有序数组 ====================
std::vector<int> merge_k_sorted(const std::vector<std::vector<int>>& arrays) {
    // 最小堆：{值, 数组下标, 元素下标}
    using T = std::tuple<int, int, int>;
    MinHeap<T> h;

    for (int i = 0; i < (int)arrays.size(); ++i)
        if (!arrays[i].empty())
            h.push({arrays[i][0], i, 0});

    std::vector<int> res;
    while (!h.empty()) {
        auto [val, arr_i, elem_i] = h.top(); h.pop();
        res.push_back(val);
        if (elem_i + 1 < (int)arrays[arr_i].size())
            h.push({arrays[arr_i][elem_i + 1], arr_i, elem_i + 1});
    }
    return res;
}

void merge_demo() {
    std::cout << "\n=== 高级：合并 K 个有序数组 ===\n";

    std::vector<std::vector<int>> arrays{
        {1, 4, 7},
        {2, 5, 8},
        {3, 6, 9},
        {0, 10, 11},
    };

    auto res = merge_k_sorted(arrays);
    std::cout << "merged: ";
    for (int v : res) std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    build_sort_demo();
    topk_demo();
    merge_demo();
    return 0;
}
