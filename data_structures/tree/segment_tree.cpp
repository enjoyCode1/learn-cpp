#include <iostream>
#include <vector>
#include <functional>

// ============================================================
// 线段树（Segment Tree）实现
// 支持区间查询和单点/区间更新，O(log n)
// 常用于：区间求和、区间最值、区间最大公约数等
// ============================================================

class SegmentTree {
public:
    explicit SegmentTree(const std::vector<int>& arr)
        : n_(arr.size()), tree_(4 * arr.size(), 0), lazy_(4 * arr.size(), 0) {
        if (!arr.empty()) build(arr, 1, 0, n_ - 1);
    }

    // 区间求和查询 [l, r]，O(log n)
    long long query_sum(int l, int r) {
        return query_sum(1, 0, n_ - 1, l, r);
    }

    // 区间最小值查询 [l, r]，O(log n)
    int query_min(int l, int r) {
        return query_min(1, 0, n_ - 1, l, r);
    }

    // 单点更新：arr[pos] = val，O(log n)
    void update_point(int pos, int val) {
        update_point(1, 0, n_ - 1, pos, val);
    }

    // 区间更新：arr[l..r] += val（懒标记），O(log n)
    void update_range(int l, int r, int val) {
        update_range(1, 0, n_ - 1, l, r, val);
    }

    void print() const {
        std::cout << "segment tree (sum): ";
        // 只打印叶子层（下标 n_~2n_-1 不适用于这里的 1-indexed 实现）
        // 简化：打印每个位置的查询结果
        for (int i = 0; i < (int)n_; ++i)
            std::cout << const_cast<SegmentTree*>(this)->query_sum(i, i) << " ";
        std::cout << "\n";
    }

private:
    size_t            n_;
    std::vector<long long> tree_;   // 区间和
    std::vector<long long> lazy_;   // 懒标记（待下推的增量）

    void build(const std::vector<int>& arr, int node, int start, int end) {
        if (start == end) {
            tree_[node] = arr[start];
            return;
        }
        int mid = (start + end) / 2;
        build(arr, 2*node,   start,   mid);
        build(arr, 2*node+1, mid+1, end);
        tree_[node] = tree_[2*node] + tree_[2*node+1];
    }

    void push_down(int node, int start, int end) {
        if (lazy_[node] != 0) {
            int mid = (start + end) / 2;
            // 左子树
            tree_[2*node]   += lazy_[node] * (mid - start + 1);
            lazy_[2*node]   += lazy_[node];
            // 右子树
            tree_[2*node+1] += lazy_[node] * (end - mid);
            lazy_[2*node+1] += lazy_[node];
            lazy_[node] = 0;
        }
    }

    long long query_sum(int node, int start, int end, int l, int r) {
        if (r < start || end < l) return 0;       // 区间不相交
        if (l <= start && end <= r) return tree_[node];  // 完全包含
        push_down(node, start, end);
        int mid = (start + end) / 2;
        return query_sum(2*node,   start,   mid, l, r)
             + query_sum(2*node+1, mid+1, end, l, r);
    }

    int query_min(int node, int start, int end, int l, int r) {
        if (r < start || end < l) return INT_MAX;
        if (l <= start && end <= r) return (int)tree_[node];
        int mid = (start + end) / 2;
        return std::min(query_min(2*node,   start,   mid, l, r),
                        query_min(2*node+1, mid+1, end, l, r));
    }

    void update_point(int node, int start, int end, int pos, int val) {
        if (start == end) { tree_[node] = val; return; }
        push_down(node, start, end);
        int mid = (start + end) / 2;
        if (pos <= mid) update_point(2*node,   start,   mid, pos, val);
        else            update_point(2*node+1, mid+1, end, pos, val);
        tree_[node] = tree_[2*node] + tree_[2*node+1];
    }

    void update_range(int node, int start, int end, int l, int r, int val) {
        if (r < start || end < l) return;
        if (l <= start && end <= r) {
            tree_[node] += (long long)val * (end - start + 1);
            lazy_[node] += val;
            return;
        }
        push_down(node, start, end);
        int mid = (start + end) / 2;
        update_range(2*node,   start,   mid, l, r, val);
        update_range(2*node+1, mid+1, end, l, r, val);
        tree_[node] = tree_[2*node] + tree_[2*node+1];
    }
};

// ==================== 入门：区间求和 ====================
void sum_demo() {
    std::cout << "=== 入门：线段树区间求和 ===\n";

    std::vector<int> arr{1, 3, 5, 7, 9, 11};
    SegmentTree st(arr);

    std::cout << "arr: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "sum[0,5] = " << st.query_sum(0, 5) << "\n";
    std::cout << "sum[1,3] = " << st.query_sum(1, 3) << "\n";
    std::cout << "sum[2,4] = " << st.query_sum(2, 4) << "\n";
}

// ==================== 中级：单点更新 ====================
void update_demo() {
    std::cout << "\n=== 中级：单点更新 ===\n";

    std::vector<int> arr{1, 3, 5, 7, 9, 11};
    SegmentTree st(arr);

    std::cout << "before: sum[0,5]=" << st.query_sum(0, 5) << "\n";

    st.update_point(2, 100);  // arr[2] = 100
    std::cout << "after update_point(2, 100):\n";
    std::cout << "  sum[0,5]=" << st.query_sum(0, 5) << "\n";
    std::cout << "  sum[1,3]=" << st.query_sum(1, 3) << "\n";
}

// ==================== 高级：区间更新（懒标记）====================
void lazy_demo() {
    std::cout << "\n=== 高级：区间更新（懒标记）===\n";

    std::vector<int> arr{1, 2, 3, 4, 5, 6, 7, 8};
    SegmentTree st(arr);

    std::cout << "before:              sum[0,7]=" << st.query_sum(0, 7) << "\n";

    // 区间 [2,5] 所有元素 +10
    st.update_range(2, 5, 10);
    std::cout << "after range+10[2,5]: sum[0,7]=" << st.query_sum(0, 7) << "\n";
    std::cout << "                     sum[2,5]=" << st.query_sum(2, 5) << "\n";
    std::cout << "                     sum[0,1]=" << st.query_sum(0, 1) << "\n";

    // 再对 [0,3] +5
    st.update_range(0, 3, 5);
    std::cout << "after range+5 [0,3]: sum[0,7]=" << st.query_sum(0, 7) << "\n";
    std::cout << "                     sum[0,3]=" << st.query_sum(0, 3) << "\n";

    std::cout << "current values: "; st.print();
}

int main() {
    sum_demo();
    update_demo();
    lazy_demo();
    return 0;
}
