#include <iostream>
#include <vector>
#include <algorithm>

// ============================================================
// 树状数组（Binary Indexed Tree / Fenwick Tree）
// 支持单点更新和前缀求和，O(log n)
// 比线段树实现简单，常量因子更小
// 核心：利用二进制低位 lowbit(i) = i & (-i) 分段存储
// ============================================================

class BIT {
public:
    explicit BIT(int n) : n_(n), tree_(n + 1, 0) {}

    // 初始化：从数组构建，O(n log n)
    explicit BIT(const std::vector<int>& arr) : n_((int)arr.size()), tree_(arr.size() + 1, 0) {
        for (int i = 0; i < n_; ++i) update(i + 1, arr[i]);
    }

    // 单点更新：位置 i（1-indexed）加上 delta，O(log n)
    void update(int i, long long delta) {
        for (; i <= n_; i += i & (-i))
            tree_[i] += delta;
    }

    // 前缀求和：[1, i]，O(log n)
    long long prefix_sum(int i) const {
        long long s = 0;
        for (; i > 0; i -= i & (-i))
            s += tree_[i];
        return s;
    }

    // 区间求和：[l, r]（1-indexed），O(log n)
    long long range_sum(int l, int r) const {
        return prefix_sum(r) - prefix_sum(l - 1);
    }

    // 单点查询：位置 i 的值，O(log n)
    long long point_query(int i) const {
        return range_sum(i, i);
    }

    int size() const { return n_; }

private:
    int              n_;
    std::vector<long long> tree_;
};

// ============================================================
// 二维树状数组
// 支持二维单点更新和二维前缀求和，O(log m * log n)
// ============================================================

class BIT2D {
public:
    BIT2D(int rows, int cols) : rows_(rows), cols_(cols), tree_(rows+1, std::vector<long long>(cols+1, 0)) {}

    void update(int r, int c, long long delta) {
        for (int i = r; i <= rows_; i += i & (-i))
            for (int j = c; j <= cols_; j += j & (-j))
                tree_[i][j] += delta;
    }

    long long prefix_sum(int r, int c) const {
        long long s = 0;
        for (int i = r; i > 0; i -= i & (-i))
            for (int j = c; j > 0; j -= j & (-j))
                s += tree_[i][j];
        return s;
    }

    // 矩形区域求和 [r1,c1] 到 [r2,c2]（1-indexed）
    long long range_sum(int r1, int c1, int r2, int c2) const {
        return prefix_sum(r2, c2)
             - prefix_sum(r1-1, c2)
             - prefix_sum(r2, c1-1)
             + prefix_sum(r1-1, c1-1);
    }

private:
    int rows_, cols_;
    std::vector<std::vector<long long>> tree_;
};

// ==================== 入门：前缀求和 ====================
void basic_demo() {
    std::cout << "=== 入门：树状数组前缀求和 ===\n";

    std::vector<int> arr{1, 3, 5, 7, 9, 11, 13, 15};
    BIT bit(arr);

    std::cout << "arr: ";
    for (int v : arr) std::cout << v << " ";
    std::cout << "\n";

    // 前缀和（1-indexed）
    for (int i = 1; i <= (int)arr.size(); ++i)
        std::cout << "prefix_sum[1.." << i << "] = " << bit.prefix_sum(i) << "\n";
}

// ==================== 中级：单点更新 + 区间查询 ====================
void update_demo() {
    std::cout << "\n=== 中级：单点更新 + 区间查询 ===\n";

    std::vector<int> arr{2, 4, 6, 8, 10};
    BIT bit(arr);

    std::cout << "sum[1,5]=" << bit.range_sum(1, 5) << "\n";
    std::cout << "sum[2,4]=" << bit.range_sum(2, 4) << "\n";

    // 修改 arr[3]（1-indexed）：原值 6，改为 20，delta = +14
    bit.update(3, 14);
    std::cout << "after update(3, +14):\n";
    std::cout << "  point[3]=" << bit.point_query(3) << "\n";
    std::cout << "  sum[1,5]=" << bit.range_sum(1, 5) << "\n";
    std::cout << "  sum[2,4]=" << bit.range_sum(2, 4) << "\n";
}

// ==================== 中级：逆序对计数 ====================
// 统计数组中逆序对数量：i < j 且 arr[i] > arr[j]
long long count_inversions(std::vector<int> arr) {
    // 离散化
    std::vector<int> sorted = arr;
    std::sort(sorted.begin(), sorted.end());
    sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
    for (auto& v : arr)
        v = (int)(std::lower_bound(sorted.begin(), sorted.end(), v) - sorted.begin()) + 1;

    int n = (int)arr.size();
    BIT bit(n);
    long long inv = 0;

    // 从右往左扫描，统计比当前元素小的已插入元素数量
    for (int i = n - 1; i >= 0; --i) {
        inv += bit.prefix_sum(arr[i] - 1);  // 已插入中比 arr[i] 小的个数
        bit.update(arr[i], 1);
    }
    return inv;
}

void inversion_demo() {
    std::cout << "\n=== 中级：逆序对计数 ===\n";

    std::vector<int> a1{2, 4, 1, 3, 5};
    std::vector<int> a2{5, 4, 3, 2, 1};
    std::vector<int> a3{1, 2, 3, 4, 5};

    auto print = [](const std::vector<int>& v) {
        for (int x : v) std::cout << x << " ";
    };

    std::cout << "arr: "; print(a1); std::cout << " -> inversions=" << count_inversions(a1) << "\n";
    std::cout << "arr: "; print(a2); std::cout << " -> inversions=" << count_inversions(a2) << "\n";
    std::cout << "arr: "; print(a3); std::cout << " -> inversions=" << count_inversions(a3) << "\n";
}

// ==================== 高级：二维树状数组 ====================
void bit2d_demo() {
    std::cout << "\n=== 高级：二维树状数组 ===\n";

    // 3x3 矩阵，初始全 0
    BIT2D bit2d(3, 3);

    // 设置初始值（通过 update 建立）
    // 矩阵:  1  2  3
    //        4  5  6
    //        7  8  9
    int v = 1;
    for (int i = 1; i <= 3; ++i)
        for (int j = 1; j <= 3; ++j)
            bit2d.update(i, j, v++);

    std::cout << "全矩阵求和 [1,1]-[3,3] = " << bit2d.range_sum(1,1,3,3) << "\n";
    std::cout << "左上角 [1,1]-[2,2] = "     << bit2d.range_sum(1,1,2,2) << "\n";
    std::cout << "右下角 [2,2]-[3,3] = "     << bit2d.range_sum(2,2,3,3) << "\n";

    // 更新 (2,2) 位置 +10
    bit2d.update(2, 2, 10);
    std::cout << "update(2,2,+10) 后全矩阵 = " << bit2d.range_sum(1,1,3,3) << "\n";
}

int main() {
    basic_demo();
    update_demo();
    inversion_demo();
    bit2d_demo();
    return 0;
}
