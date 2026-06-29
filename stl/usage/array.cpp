#include <iostream>
#include <array>
#include <algorithm>
#include <numeric>
#include <string>

// ============================================================
// std::array 从入门到高级用法
// 固定大小数组，栈分配，零开销抽象
// 比 C 数组安全（知道自身大小），比 vector 快（无堆分配）
// ============================================================

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：array 基础操作 ===\n";

    std::array<int, 5> a{1, 2, 3, 4, 5};

    // 访问
    std::cout << "a[0]=" << a[0] << " a.at(4)=" << a.at(4) << "\n";
    std::cout << "front=" << a.front() << " back=" << a.back() << "\n";

    // 大小（编译期常量）
    std::cout << "size=" << a.size() << " empty=" << std::boolalpha << a.empty() << "\n";

    // 遍历
    std::cout << "range-for: ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";

    // 修改
    a[2] = 99;
    a.at(3) = 88;
    std::cout << "after modify: ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";

    // fill：全部填充同一值
    a.fill(0);
    std::cout << "fill(0): ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：与 C 数组互操作 ====================
void c_array_compat_demo() {
    std::cout << "\n=== 中级：与 C 数组互操作 ===\n";

    std::array<int, 4> a{10, 20, 30, 40};

    // data() 返回底层指针，可传给 C API
    int* ptr = a.data();
    std::cout << "data()[1]=" << ptr[1] << "\n";

    // C 数组转 array（通过初始化）
    int c_arr[] = {5, 4, 3, 2, 1};
    std::array<int, 5> from_c;
    std::copy(std::begin(c_arr), std::end(c_arr), from_c.begin());
    std::cout << "from c_arr: ";
    for (int x : from_c) std::cout << x << " ";
    std::cout << "\n";

    // swap：交换两个同类型 array
    std::array<int, 4> b{1, 2, 3, 4};
    a.swap(b);
    std::cout << "after swap, a: ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：与算法配合 ====================
void algorithm_demo() {
    std::cout << "\n=== 中级：array 与算法 ===\n";

    std::array<int, 8> a{5, 3, 1, 4, 2, 8, 6, 7};

    // sort
    std::sort(a.begin(), a.end());
    std::cout << "sorted: ";
    for (int x : a) std::cout << x << " ";
    std::cout << "\n";

    // accumulate
    int sum = std::accumulate(a.begin(), a.end(), 0);
    std::cout << "sum=" << sum << "\n";

    // find
    auto it = std::find(a.begin(), a.end(), 5);
    std::cout << "find(5) at index=" << (it - a.begin()) << "\n";

    // 反向遍历
    std::cout << "reverse: ";
    for (auto it2 = a.rbegin(); it2 != a.rend(); ++it2)
        std::cout << *it2 << " ";
    std::cout << "\n";
}

// ==================== 高级：多维 array ====================
void multidim_demo() {
    std::cout << "\n=== 高级：多维 array ===\n";

    // 二维 array（编译期固定大小）
    std::array<std::array<int, 3>, 3> matrix{{
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    }};

    std::cout << "3x3 matrix:\n";
    for (auto& row : matrix) {
        for (int v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    // 转置
    std::array<std::array<int, 3>, 3> transposed{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            transposed[j][i] = matrix[i][j];

    std::cout << "transposed:\n";
    for (auto& row : transposed) {
        for (int v : row) std::cout << v << " ";
        std::cout << "\n";
    }
}

// ==================== 高级：编译期操作（constexpr）====================
constexpr std::array<int, 5> make_squares() {
    std::array<int, 5> a{};
    for (int i = 0; i < 5; ++i) a[i] = (i+1) * (i+1);
    return a;
}

constexpr int array_sum(const std::array<int, 5>& a) {
    int s = 0;
    for (int x : a) s += x;
    return s;
}

void constexpr_demo() {
    std::cout << "\n=== 高级：constexpr array（编译期）===\n";

    constexpr auto squares = make_squares();
    constexpr int  sum     = array_sum(squares);

    std::cout << "squares: ";
    for (int x : squares) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "sum (compile-time)=" << sum << "\n";

    // 编译期查找
    constexpr bool has25 = [&]() constexpr {
        for (int x : squares) if (x == 25) return true;
        return false;
    }();
    std::cout << "has 25: " << std::boolalpha << has25 << "\n";
}

// ==================== 高级：std::array 用于状态机 ====================
void state_machine_demo() {
    std::cout << "\n=== 高级：array 状态机查找表 ===\n";

    // 简单状态机：识别字符串是否只含 a/b，且不以 aa 结尾
    // 状态：0=初始, 1=上一字符是a, 2=接受(上一不是a), 3=拒绝
    // 转移表 [state][input: 0='a', 1='b', 2=其他]
    constexpr std::array<std::array<int, 3>, 4> transition{{
        {1, 2, 3},  // state 0: a->1, b->2, other->3
        {3, 2, 3},  // state 1: a->3(aa), b->2, other->3
        {1, 2, 3},  // state 2: a->1, b->2, other->3
        {3, 3, 3}   // state 3: reject
    }};

    auto classify = [](char c) -> int {
        if (c == 'a') return 0;
        if (c == 'b') return 1;
        return 2;
    };

    auto check = [&](const std::string& s) {
        int state = 0;
        for (char c : s) state = transition[state][classify(c)];
        bool ok = (state == 1 || state == 2);
        std::cout << "\"" << s << "\" -> " << (ok ? "accept" : "reject") << "\n";
    };

    check("ab");    // accept
    check("abba");  // accept
    check("aa");    // reject (ends with aa)
    check("abc");   // reject (contains c)
    check("bab");   // accept
}

int main() {
    basic_demo();
    c_array_compat_demo();
    algorithm_demo();
    multidim_demo();
    constexpr_demo();
    state_machine_demo();
    return 0;
}
