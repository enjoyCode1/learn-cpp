#include <iostream>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

// ==================== 入门级 ====================

void basic_ranges() {
    std::cout << "=== 入门：ranges 基础 ===\n";

    std::vector<int> v{5, 3, 1, 4, 2, 8, 6};

    // 范围版排序，不需要 begin/end
    std::ranges::sort(v);
    std::cout << "sorted: ";
    for (auto x : v) std::cout << x << " ";
    std::cout << "\n";

    // 范围版查找
    auto it = std::ranges::find(v, 4);
    if (it != v.end())
        std::cout << "found 4 at index " << std::distance(v.begin(), it) << "\n";

    // 范围版 min/max
    std::cout << "min=" << std::ranges::min(v)
              << " max=" << std::ranges::max(v) << "\n";
}

// ==================== 中级 ====================

void range_views() {
    std::cout << "\n=== 中级：range views 惰性求值 ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // filter + transform 管道，惰性求值
    auto result = v
        | std::views::filter([](int x) { return x % 2 == 0; })  // 偶数
        | std::views::transform([](int x) { return x * x; });    // 平方

    std::cout << "even squares: ";
    for (auto x : result) std::cout << x << " ";
    std::cout << "\n";
}

void range_adaptors() {
    std::cout << "\n=== 中级：常用 range adaptors ===\n";

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // take：前 N 个
    std::cout << "take(5): ";
    for (auto x : v | std::views::take(5)) std::cout << x << " ";
    std::cout << "\n";

    // drop：跳过前 N 个
    std::cout << "drop(7): ";
    for (auto x : v | std::views::drop(7)) std::cout << x << " ";
    std::cout << "\n";

    // reverse
    std::cout << "reverse: ";
    for (auto x : v | std::views::reverse) std::cout << x << " ";
    std::cout << "\n";

    // iota：生成序列
    std::cout << "iota(1,6): ";
    for (auto x : std::views::iota(1, 6)) std::cout << x << " ";
    std::cout << "\n";
}

void range_pipeline() {
    std::cout << "\n=== 中级：管道组合 ===\n";

    // 从 1 到 20，取能被 3 整除的数，取前 4 个，乘以 10
    auto result = std::views::iota(1, 21)
        | std::views::filter([](int x) { return x % 3 == 0; })
        | std::views::take(4)
        | std::views::transform([](int x) { return x * 10; });

    std::cout << "pipeline result: ";
    for (auto x : result) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级 ====================

void string_range_demo() {
    std::cout << "\n=== 高级：字符串 range 处理 ===\n";

    std::vector<std::string> words{"hello", "world", "cpp", "ranges", "are", "great"};

    // 过滤长度大于 4 的单词，转大写首字母
    auto long_words = words
        | std::views::filter([](const std::string& s) { return s.size() > 4; })
        | std::views::transform([](std::string s) {
              s[0] = static_cast<char>(std::toupper(s[0]));
              return s;
          });

    std::cout << "long words (capitalized): ";
    for (const auto& w : long_words) std::cout << w << " ";
    std::cout << "\n";
}

void range_to_vector() {
    std::cout << "\n=== 高级：range 结果收集到容器 ===\n";

    // C++23 有 std::ranges::to<>，C++20 用 copy 收集
    auto view = std::views::iota(1, 11)
        | std::views::filter([](int x) { return x % 2 != 0; })
        | std::views::transform([](int x) { return x * x; });

    std::vector<int> result;
    std::ranges::copy(view, std::back_inserter(result));

    std::cout << "odd squares collected: ";
    for (auto x : result) std::cout << x << " ";
    std::cout << "\n";
    std::cout << "sum=" << std::accumulate(result.begin(), result.end(), 0) << "\n";
}

// 自定义 range view：生成 fibonacci 序列
struct FibView : std::ranges::view_interface<FibView> {
    struct Iterator {
        using difference_type = std::ptrdiff_t;
        using value_type = long long;
        using iterator_category = std::input_iterator_tag;

        long long a = 0, b = 1;
        int count = 0, limit = 0;

        Iterator() = default;
        Iterator(int n) : limit(n) {}

        long long operator*() const { return a; }
        Iterator& operator++() {
            auto next = a + b;
            a = b; b = next;
            ++count;
            return *this;
        }
        Iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
        bool operator==(std::default_sentinel_t) const { return count >= limit; }
    };

    int n_;
    explicit FibView(int n) : n_(n) {}
    Iterator begin() const { return Iterator{n_}; }
    std::default_sentinel_t end() const { return {}; }
};

void custom_range_view() {
    std::cout << "\n=== 高级：自定义 range view（Fibonacci）===\n";

    std::cout << "fib(10): ";
    for (auto x : FibView{10}) std::cout << x << " ";
    std::cout << "\n";

    // 配合标准 adaptors
    std::cout << "fib evens: ";
    for (auto x : FibView{15} | std::views::filter([](auto v){ return v % 2 == 0; }))
        std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    basic_ranges();
    range_views();
    range_adaptors();
    range_pipeline();
    string_range_demo();
    range_to_vector();
    custom_range_view();
    return 0;
}
