#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>

// ============================================================
// std::vector 从入门到高级用法
// 动态数组，随机访问 O(1)，尾部插入均摊 O(1)
// ============================================================

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：vector 基础操作 ===\n";

    // 构造
    std::vector<int> v1;                        // 空 vector
    std::vector<int> v2(5, 0);                  // 5 个 0
    std::vector<int> v3{1, 2, 3, 4, 5};         // 初始化列表
    std::vector<int> v4(v3.begin(), v3.end());  // 迭代器构造

    // 增：push_back / emplace_back
    v1.push_back(10);
    v1.push_back(20);
    v1.emplace_back(30);  // 原地构造，避免拷贝

    // 访问
    std::cout << "v1[0]=" << v1[0] << "  v1.at(1)=" << v1.at(1) << "\n";
    std::cout << "front=" << v1.front() << "  back=" << v1.back() << "\n";

    // 大小
    std::cout << "size=" << v1.size() << "  empty=" << std::boolalpha << v1.empty() << "\n";

    // 遍历
    std::cout << "v3: ";
    for (int x : v3) std::cout << x << " ";
    std::cout << "\n";

    // 删除
    v3.pop_back();                          // 删除尾部
    v3.erase(v3.begin() + 1);              // 删除索引 1
    std::cout << "v3 after erase: ";
    for (int x : v3) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：容量管理 ====================
void capacity_demo() {
    std::cout << "\n=== 中级：容量管理 ===\n";

    std::vector<int> v;
    std::cout << "初始 size=" << v.size() << " capacity=" << v.capacity() << "\n";

    // reserve 预分配，避免多次扩容
    v.reserve(100);
    std::cout << "reserve(100) capacity=" << v.capacity() << "\n";

    for (int i = 0; i < 10; ++i) v.push_back(i);
    std::cout << "push 10 个元素后 size=" << v.size() << " capacity=" << v.capacity() << "\n";

    // shrink_to_fit 释放多余内存
    v.shrink_to_fit();
    std::cout << "shrink_to_fit 后 capacity=" << v.capacity() << "\n";

    // resize vs reserve
    std::vector<int> v2;
    v2.resize(5, -1);  // 改变 size，新元素填 -1
    std::cout << "resize(5,-1): ";
    for (int x : v2) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：插入与修改 ====================
void insert_demo() {
    std::cout << "\n=== 中级：插入与修改 ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // insert 在指定位置插入
    v.insert(v.begin() + 2, 99);           // 在索引 2 插入 99
    std::cout << "insert(2, 99): ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // insert 批量插入
    std::vector<int> src{10, 20, 30};
    v.insert(v.end(), src.begin(), src.end());
    std::cout << "insert range: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // emplace 原地构造（对复杂对象更高效）
    v.emplace(v.begin(), 0);
    std::cout << "emplace(begin, 0): ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // assign 重新赋值
    v.assign(3, 7);
    std::cout << "assign(3, 7): ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：与算法配合 ====================
void algorithm_demo() {
    std::cout << "\n=== 高级：与算法配合 ===\n";

    std::vector<int> v{5, 3, 1, 4, 2, 8, 6, 7};

    // 排序
    std::sort(v.begin(), v.end());
    std::cout << "sorted: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";

    // 二分查找（要求有序）
    bool found = std::binary_search(v.begin(), v.end(), 6);
    std::cout << "binary_search(6)=" << std::boolalpha << found << "\n";

    auto it = std::lower_bound(v.begin(), v.end(), 6);
    std::cout << "lower_bound(6)=" << *it << " at index " << (it - v.begin()) << "\n";

    // accumulate 求和
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "sum=" << sum << "\n";

    // transform 变换
    std::vector<int> doubled(v.size());
    std::transform(v.begin(), v.end(), doubled.begin(), [](int x) { return x * 2; });
    std::cout << "doubled: ";
    for (int x : doubled) std::cout << x << " ";
    std::cout << "\n";

    // remove_if + erase（erase-remove 惯用法）
    v.erase(std::remove_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; }), v.end());
    std::cout << "remove evens: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：二维 vector 与自定义对象 ====================
struct Student {
    std::string name;
    int         score;
    bool operator<(const Student& o) const { return score > o.score; }  // 降序
};

void advanced_demo() {
    std::cout << "\n=== 高级：二维 vector 与自定义对象 ===\n";

    // 二维 vector
    std::vector<std::vector<int>> matrix(3, std::vector<int>(3, 0));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            matrix[i][j] = i * 3 + j + 1;

    std::cout << "3x3 matrix:\n";
    for (auto& row : matrix) {
        for (int v : row) std::cout << v << " ";
        std::cout << "\n";
    }

    // 自定义对象排序
    std::vector<Student> students{{"Alice", 88}, {"Bob", 95}, {"Carol", 72}, {"Dave", 95}};
    std::sort(students.begin(), students.end());
    std::cout << "students by score(desc):\n";
    for (auto& s : students)
        std::cout << "  " << s.name << " " << s.score << "\n";

    // stable_sort 保持相同 score 的原始顺序
    std::stable_sort(students.begin(), students.end());
    std::cout << "stable_sort (Bob before Dave preserved):\n";
    for (auto& s : students)
        std::cout << "  " << s.name << " " << s.score << "\n";
}

// ==================== 高级：移动语义与 emplace_back ====================
void move_demo() {
    std::cout << "\n=== 高级：移动语义 ===\n";

    std::vector<std::string> v;
    v.reserve(3);

    std::string s = "hello world this is a long string";
    v.push_back(s);               // 拷贝
    v.push_back(std::move(s));    // 移动，s 变为空
    v.emplace_back("direct construct");  // 直接构造，无临时对象

    std::cout << "s after move: '" << s << "'\n";
    std::cout << "v:\n";
    for (auto& str : v) std::cout << "  '" << str << "'\n";
}

int main() {
    basic_demo();
    capacity_demo();
    insert_demo();
    algorithm_demo();
    advanced_demo();
    move_demo();
    return 0;
}
