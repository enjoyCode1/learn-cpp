#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <iterator>
#include <algorithm>
#include <numeric>
#include <sstream>

// ============================================================
// 迭代器从入门到高级用法
// 5 类迭代器：输入、输出、前向、双向、随机访问
// 迭代器适配器：reverse、insert、stream
// ============================================================

// ==================== 入门：基础迭代器操作 ====================
void basic_demo() {
    std::cout << "=== 入门：基础迭代器操作 ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    // begin/end
    for (auto it = v.begin(); it != v.end(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // cbegin/cend：const 迭代器，禁止修改
    for (auto it = v.cbegin(); it != v.cend(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // 随机访问：+/-/[]
    auto it = v.begin();
    std::cout << "it[2]=" << it[2] << "\n";
    std::advance(it, 3);
    std::cout << "after advance(3): " << *it << "\n";
    std::cout << "distance(begin, it)=" << std::distance(v.begin(), it) << "\n";

    // next / prev
    auto it2 = std::next(v.begin(), 2);
    auto it3 = std::prev(v.end(), 1);
    std::cout << "next(begin,2)=" << *it2
              << " prev(end,1)=" << *it3 << "\n";
}

// ==================== 入门：反向迭代器 ====================
void reverse_iter_demo() {
    std::cout << "\n=== 入门：反向迭代器 ===\n";

    std::vector<int> v{1, 2, 3, 4, 5};

    std::cout << "reverse: ";
    for (auto it = v.rbegin(); it != v.rend(); ++it)
        std::cout << *it << " ";
    std::cout << "\n";

    // rbegin/rend 配合 algorithm
    auto it = std::find(v.rbegin(), v.rend(), 3);
    std::cout << "rfind(3) offset from rbegin="
              << std::distance(v.rbegin(), it) << "\n";

    // 反向排序
    std::sort(v.rbegin(), v.rend());
    std::cout << "sort descending: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：插入迭代器适配器 ====================
void insert_iter_demo() {
    std::cout << "\n=== 中级：插入迭代器适配器 ===\n";

    std::vector<int> src{1, 2, 3, 4, 5};

    // back_inserter：向末尾插入
    std::vector<int> dst1;
    std::copy(src.begin(), src.end(), std::back_inserter(dst1));
    std::cout << "back_inserter: ";
    for (int x : dst1) std::cout << x << " ";
    std::cout << "\n";

    // front_inserter：向头部插入（适合 list/deque）
    std::list<int> dst2;
    std::copy(src.begin(), src.end(), std::front_inserter(dst2));
    std::cout << "front_inserter: ";
    for (int x : dst2) std::cout << x << " ";
    std::cout << "\n";

    // inserter：在指定位置插入
    std::vector<int> dst3{10, 20, 30};
    std::copy(src.begin(), src.end(), std::inserter(dst3, dst3.begin() + 1));
    std::cout << "inserter(pos=1): ";
    for (int x : dst3) std::cout << x << " ";
    std::cout << "\n";

    // transform + back_inserter
    std::vector<int> sq;
    std::transform(src.begin(), src.end(), std::back_inserter(sq),
                   [](int x){ return x * x; });
    std::cout << "transform squares: ";
    for (int x : sq) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：流迭代器 ====================
void stream_iter_demo() {
    std::cout << "\n=== 中级：流迭代器 ===\n";

    // ostream_iterator：输出到流
    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "ostream_iterator: ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";

    // istream_iterator：从流读取
    std::istringstream iss("10 20 30 40 50");
    std::vector<int> parsed{std::istream_iterator<int>(iss),
                            std::istream_iterator<int>()};
    std::cout << "istream_iterator: ";
    for (int x : parsed) std::cout << x << " ";
    std::cout << "\n";

    // 用 istream_iterator + accumulate 求和
    std::istringstream iss2("1 2 3 4 5");
    int sum = std::accumulate(std::istream_iterator<int>(iss2),
                              std::istream_iterator<int>(), 0);
    std::cout << "stream sum=" << sum << "\n";
}

// ==================== 高级：迭代器类别与 advance 效率 ====================
void iter_category_demo() {
    std::cout << "\n=== 高级：迭代器类别 ===\n";

    // vector：随机访问迭代器（random_access_iterator）
    // list：双向迭代器（bidirectional_iterator）
    // advance 对 list 是 O(n)，对 vector 是 O(1)

    std::vector<int> vec{1, 2, 3, 4, 5};
    std::list<int>   lst{1, 2, 3, 4, 5};

    auto vit = vec.begin();
    auto lit = lst.begin();
    std::advance(vit, 3);  // O(1) for vector
    std::advance(lit, 3);  // O(n) for list
    std::cout << "vec advance(3)=" << *vit << "\n";
    std::cout << "lst advance(3)=" << *lit << "\n";

    // 用 type traits 检测迭代器类型
    using VecIter = std::vector<int>::iterator;
    using LstIter = std::list<int>::iterator;
    bool vec_random = std::is_same_v<
        std::iterator_traits<VecIter>::iterator_category,
        std::random_access_iterator_tag>;
    bool lst_bidir = std::is_same_v<
        std::iterator_traits<LstIter>::iterator_category,
        std::bidirectional_iterator_tag>;
    std::cout << "vector is random_access: " << std::boolalpha << vec_random << "\n";
    std::cout << "list is bidirectional:   " << lst_bidir << "\n";
}

// ==================== 高级：自定义迭代器 ====================
// 实现一个范围迭代器，遍历 [start, end) 步长为 step
class RangeIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = int;
    using difference_type   = int;
    using pointer           = const int*;
    using reference         = const int&;

    RangeIterator(int val, int step) : val_(val), step_(step) {}

    int  operator*()  const { return val_; }
    RangeIterator& operator++() { val_ += step_; return *this; }
    bool operator!=(const RangeIterator& o) const { return val_ < o.val_; }

private:
    int val_, step_;
};

class Range {
public:
    Range(int start, int end, int step = 1)
        : start_(start), end_(end), step_(step) {}
    RangeIterator begin() const { return {start_, step_}; }
    RangeIterator end()   const { return {end_,   step_}; }
private:
    int start_, end_, step_;
};

void custom_iter_demo() {
    std::cout << "\n=== 高级：自定义迭代器 ===\n";

    std::cout << "Range(0,10,2): ";
    for (int x : Range(0, 10, 2)) std::cout << x << " ";
    std::cout << "\n";

    // 与 algorithm 配合
    std::vector<int> v;
    Range r(1, 11, 1);
    std::copy(r.begin(), r.end(), std::back_inserter(v));
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "sum of Range(1,11)=" << sum << "\n";
}

// ==================== 高级：move_iterator ====================
void move_iter_demo() {
    std::cout << "\n=== 高级：move_iterator ===\n";

    std::vector<std::string> src{"hello", "world", "foo", "bar"};
    std::vector<std::string> dst;

    // move_iterator 让 copy 变成 move，避免字符串拷贝
    std::copy(std::make_move_iterator(src.begin()),
              std::make_move_iterator(src.end()),
              std::back_inserter(dst));

    std::cout << "dst after move_iterator copy:\n";
    for (auto& s : dst) std::cout << "  '" << s << "'\n";

    std::cout << "src after move (strings emptied):\n";
    for (auto& s : src) std::cout << "  '" << s << "'\n";
}

int main() {
    basic_demo();
    reverse_iter_demo();
    insert_iter_demo();
    stream_iter_demo();
    iter_category_demo();
    custom_iter_demo();
    move_iter_demo();
    return 0;
}
