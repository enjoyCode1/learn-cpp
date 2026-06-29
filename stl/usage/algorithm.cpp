#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <functional>

// ============================================================
// <algorithm> / <numeric> 从入门到高级用法
// 覆盖：排序、查找、变换、归约、排列、堆操作
// ============================================================

// ==================== 入门：排序与查找 ====================
void sort_search_demo() {
    std::cout << "=== 入门：排序与查找 ===\n";

    std::vector<int> v{5, 3, 1, 4, 2, 8, 6, 7};

    // sort：快排，O(n log n)
    std::sort(v.begin(), v.end());
    std::cout << "sort:  ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // stable_sort：保持相等元素的原始顺序
    std::vector<std::pair<int,char>> pv{{3,'a'},{1,'b'},{3,'c'},{1,'d'}};
    std::stable_sort(pv.begin(), pv.end(),
                     [](const auto& a, const auto& b){ return a.first < b.first; });
    std::cout << "stable_sort: ";
    for (auto& [k,c] : pv) std::cout << k << c << " ";
    std::cout << "\n";

    // binary_search / lower_bound / upper_bound（要求有序）
    std::cout << "binary_search(6)=" << std::boolalpha
              << std::binary_search(v.begin(), v.end(), 6) << "\n";
    auto lo = std::lower_bound(v.begin(), v.end(), 4);
    auto hi = std::upper_bound(v.begin(), v.end(), 6);
    std::cout << "lower_bound(4)=" << *lo << " upper_bound(6) points to " << *hi << "\n";

    // find / find_if
    auto it = std::find(v.begin(), v.end(), 5);
    std::cout << "find(5) at index=" << (it - v.begin()) << "\n";
    auto it2 = std::find_if(v.begin(), v.end(), [](int x){ return x > 6; });
    std::cout << "first >6: " << *it2 << "\n";
}

// ==================== 入门：统计与比较 ====================
void count_compare_demo() {
    std::cout << "\n=== 入门：统计与比较 ===\n";

    std::vector<int> v{1, 2, 3, 2, 4, 2, 5};

    std::cout << "count(2)=" << std::count(v.begin(), v.end(), 2) << "\n";
    std::cout << "count_if(>3)=" << std::count_if(v.begin(), v.end(),
                                                   [](int x){ return x > 3; }) << "\n";

    // min_element / max_element
    auto [mn, mx] = std::minmax_element(v.begin(), v.end());
    std::cout << "min=" << *mn << " max=" << *mx << "\n";

    // all_of / any_of / none_of
    std::cout << "all>0: "  << std::boolalpha << std::all_of(v.begin(), v.end(), [](int x){ return x > 0; }) << "\n";
    std::cout << "any>4: "  << std::any_of(v.begin(), v.end(), [](int x){ return x > 4; }) << "\n";
    std::cout << "none<0: " << std::none_of(v.begin(), v.end(), [](int x){ return x < 0; }) << "\n";
}

// ==================== 中级：变换与修改 ====================
void transform_demo() {
    std::cout << "\n=== 中级：变换与修改 ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // transform：一元变换
    std::vector<int> sq(v.size());
    std::transform(v.begin(), v.end(), sq.begin(), [](int x){ return x * x; });
    std::cout << "squares: ";
    for (int x : sq) std::cout << x << " ";
    std::cout << "\n";

    // transform：二元变换（两个序列）
    std::vector<int> a{1, 2, 3}, b{4, 5, 6}, c(3);
    std::transform(a.begin(), a.end(), b.begin(), c.begin(),
                   [](int x, int y){ return x + y; });
    std::cout << "a+b: ";
    for (int x : c) std::cout << x << " ";
    std::cout << "\n";

    // for_each
    std::vector<int> v2{1, 2, 3, 4, 5};
    std::for_each(v2.begin(), v2.end(), [](int& x){ x *= 2; });
    std::cout << "for_each *=2: ";
    for (int x : v2) std::cout << x << " ";
    std::cout << "\n";

    // fill / fill_n / generate
    std::vector<int> v3(5);
    std::fill(v3.begin(), v3.end(), 7);
    std::cout << "fill(7): ";
    for (int x : v3) std::cout << x << " ";
    std::cout << "\n";

    int n = 0;
    std::generate(v3.begin(), v3.end(), [&n]{ return n++ * n; });
    std::cout << "generate: ";
    for (int x : v3) std::cout << x << " ";
    std::cout << "\n";

    // replace / replace_if
    std::vector<int> v4{1, 2, 3, 2, 4, 2};
    std::replace(v4.begin(), v4.end(), 2, 99);
    std::cout << "replace(2->99): ";
    for (int x : v4) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：删除与去重 ====================
void remove_unique_demo() {
    std::cout << "\n=== 中级：删除与去重 ===\n";

    std::vector<int> v{1, 2, 2, 3, 3, 3, 4, 5};

    // unique：去除相邻重复（需先排序），返回新逻辑末尾
    auto new_end = std::unique(v.begin(), v.end());
    v.erase(new_end, v.end());
    std::cout << "unique: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // remove_if + erase（erase-remove idiom）
    std::vector<int> v2{1, 2, 3, 4, 5, 6, 7, 8};
    v2.erase(std::remove_if(v2.begin(), v2.end(),
                            [](int x){ return x % 2 == 0; }), v2.end());
    std::cout << "remove evens: ";
    for (int x : v2) std::cout << x << " ";
    std::cout << "\n";

    // partition：将满足条件的元素移到前面
    std::vector<int> v3{1, 2, 3, 4, 5, 6, 7, 8};
    auto mid = std::partition(v3.begin(), v3.end(),
                              [](int x){ return x % 2 == 0; });
    std::cout << "partition(even first): ";
    for (int x : v3) std::cout << x << " ";
    std::cout << "| pivot at index " << (mid - v3.begin()) << "\n";
}

// ==================== 中级：数值算法 ====================
void numeric_demo() {
    std::cout << "\n=== 中级：数值算法（<numeric>）===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // accumulate：折叠/归约
    int sum = std::accumulate(v.begin(), v.end(), 0);
    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>{});
    std::cout << "sum=" << sum << " product=" << product << "\n";

    // inner_product：点积
    std::vector<int> w{2, 3, 4, 5, 6};
    int dot = std::inner_product(v.begin(), v.end(), w.begin(), 0);
    std::cout << "dot product=" << dot << "\n";

    // partial_sum：前缀和
    std::vector<int> prefix(v.size());
    std::partial_sum(v.begin(), v.end(), prefix.begin());
    std::cout << "prefix_sum: ";
    for (int x : prefix) std::cout << x << " ";
    std::cout << "\n";

    // adjacent_difference：相邻差分
    std::vector<int> diff(v.size());
    std::adjacent_difference(v.begin(), v.end(), diff.begin());
    std::cout << "adjacent_diff: ";
    for (int x : diff) std::cout << x << " ";
    std::cout << "\n";

    // iota：填充递增序列
    std::vector<int> seq(5);
    std::iota(seq.begin(), seq.end(), 10);
    std::cout << "iota(10): ";
    for (int x : seq) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：堆操作 ====================
void heap_demo() {
    std::cout << "\n=== 高级：堆操作 ===\n";

    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};

    // make_heap：建立最大堆
    std::make_heap(v.begin(), v.end());
    std::cout << "max_heap top=" << v.front() << "\n";

    // push_heap：插入元素（先 push_back，再 push_heap）
    v.push_back(7);
    std::push_heap(v.begin(), v.end());
    std::cout << "after push 7, top=" << v.front() << "\n";

    // pop_heap：弹出最大值（移到末尾，再 pop_back）
    std::pop_heap(v.begin(), v.end());
    int top = v.back(); v.pop_back();
    std::cout << "popped=" << top << " new top=" << v.front() << "\n";

    // sort_heap：堆排序（原地，升序）
    std::make_heap(v.begin(), v.end());
    std::sort_heap(v.begin(), v.end());
    std::cout << "sort_heap: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // 最小堆：使用 greater
    std::vector<int> minv{5, 3, 8, 1, 9};
    std::make_heap(minv.begin(), minv.end(), std::greater<int>{});
    std::cout << "min_heap top=" << minv.front() << "\n";
}

// ==================== 高级：排列与旋转 ====================
void permutation_demo() {
    std::cout << "\n=== 高级：排列与旋转 ===\n";

    // next_permutation：枚举所有排列
    std::vector<int> v{1, 2, 3};
    std::cout << "all permutations of {1,2,3}:\n";
    do {
        std::cout << "  ";
        for (int x : v) std::cout << x << " ";
        std::cout << "\n";
    } while (std::next_permutation(v.begin(), v.end()));

    // rotate：旋转
    std::vector<int> r{1, 2, 3, 4, 5};
    std::rotate(r.begin(), r.begin() + 2, r.end());  // 左移 2 位
    std::cout << "rotate(left 2): ";
    for (int x : r) std::cout << x << " ";
    std::cout << "\n";

    // nth_element：第 k 小（O(n) 均摊）
    std::vector<int> a{5, 3, 1, 4, 2};
    std::nth_element(a.begin(), a.begin() + 2, a.end());
    std::cout << "nth_element(k=2): " << a[2]
              << " (3rd smallest)\n";
}

// ==================== 高级：merge / inplace_merge ====================
void merge_demo() {
    std::cout << "\n=== 高级：merge 合并有序序列 ===\n";

    std::vector<int> a{1, 3, 5, 7}, b{2, 4, 6, 8};
    std::vector<int> merged(a.size() + b.size());
    std::merge(a.begin(), a.end(), b.begin(), b.end(), merged.begin());
    std::cout << "merge: ";
    for (int x : merged) std::cout << x << " ";
    std::cout << "\n";

    // inplace_merge：将同一 vector 中两段有序序列合并
    std::vector<int> c{1, 3, 5, 2, 4, 6};
    std::inplace_merge(c.begin(), c.begin() + 3, c.end());
    std::cout << "inplace_merge: ";
    for (int x : c) std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    sort_search_demo();
    count_compare_demo();
    transform_demo();
    remove_unique_demo();
    numeric_demo();
    heap_demo();
    permutation_demo();
    merge_demo();
    return 0;
}
