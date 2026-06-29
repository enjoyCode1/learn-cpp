#include <print>
#include <ranges>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>

// ==================== 入门级 ====================
// std::ranges::to<>：将 range/view 的结果收集到容器
// C++20 的 ranges 只提供惰性视图，没有直接收集的方法
// C++23 的 ranges::to 填补了这个空缺

void basic_ranges_to() {
    std::println("=== 入门：ranges::to 基础 ===");

    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // C++20 写法（繁琐）
    // std::vector<int> result;
    // std::ranges::copy(v | std::views::filter(...), std::back_inserter(result));

    // C++23 ranges::to（简洁）
    auto evens = v
        | std::views::filter([](int x) { return x % 2 == 0; })
        | std::ranges::to<std::vector>();

    std::print("evens: ");
    for (auto x : evens) std::print("{} ", x);
    std::println("");

    // 直接指定容器类型
    auto squares = std::views::iota(1, 6)
        | std::views::transform([](int x) { return x * x; })
        | std::ranges::to<std::vector<int>>();

    std::print("squares: ");
    for (auto x : squares) std::print("{} ", x);
    std::println("");
}

// ==================== 中级 ====================

void ranges_to_different_containers() {
    std::println("\n=== 中级：收集到不同容器类型 ===");

    std::vector<int> src{5, 3, 1, 4, 1, 5, 9, 2, 6, 5};

    // 收集到 set（自动去重排序）
    auto unique_sorted = src | std::ranges::to<std::set<int>>();
    std::print("set:    ");
    for (auto x : unique_sorted) std::print("{} ", x);
    std::println("");

    // 收集到 list
    auto as_list = src
        | std::views::take(5)
        | std::ranges::to<std::list<int>>();
    std::print("list:   ");
    for (auto x : as_list) std::print("{} ", x);
    std::println("");

    // 收集到 string
    std::vector<char> chars{'h','e','l','l','o'};
    auto str = chars | std::ranges::to<std::string>();
    std::println("string: {}", str);
}

void ranges_to_map() {
    std::println("\n=== 中级：ranges::to 构建 map ===");

    std::vector<std::string> words{"apple", "banana", "cherry", "date", "elderberry"};

    // 将单词映射为长度
    auto word_len = words
        | std::views::transform([](const std::string& w) {
              return std::pair{w, w.size()};
          })
        | std::ranges::to<std::map<std::string, size_t>>();

    for (const auto& [word, len] : word_len)
        std::println("  {}: {}", word, len);
}

// ==================== 高级 ====================

void pipeline_to_demo() {
    std::println("\n=== 高级：复杂管道 + ranges::to ===");

    std::vector<std::string> raw{
        "  hello  ", "WORLD", "  cpp  ", "23", "  ranges  "
    };

    // 去空格 -> 转小写 -> 过滤纯字母 -> 排序 -> 收集
    auto trim = [](std::string s) {
        auto start = s.find_first_not_of(' ');
        auto end   = s.find_last_not_of(' ');
        return start == std::string::npos ? std::string{} : s.substr(start, end - start + 1);
    };

    auto to_lower = [](std::string s) {
        std::ranges::transform(s, s.begin(), ::tolower);
        return s;
    };

    auto is_alpha_only = [](const std::string& s) {
        return !s.empty() && std::ranges::all_of(s, ::isalpha);
    };

    auto result = raw
        | std::views::transform(trim)
        | std::views::transform(to_lower)
        | std::views::filter(is_alpha_only)
        | std::ranges::to<std::vector<std::string>>();

    std::ranges::sort(result);

    std::println("processed words:");
    for (const auto& w : result) std::println("  {}", w);
}

// ranges::to 与自定义容器
template<typename T>
struct RingBuffer {
    std::vector<T> data;
    size_t capacity;

    using value_type = T;

    explicit RingBuffer(size_t cap) : capacity(cap) { data.reserve(cap); }

    void push_back(T val) {
        if (data.size() >= capacity)
            data.erase(data.begin());
        data.push_back(std::move(val));
    }
};

// 让 RingBuffer 支持 ranges::to（需要 from_range 构造或 push_back）
void custom_container_demo() {
    std::println("\n=== 高级：ranges::to 与 push_back 接口 ===");

    // ranges::to 对有 push_back 的容器通用
    auto nums = std::views::iota(1, 11)
        | std::views::transform([](int x) { return x * x; })
        | std::ranges::to<std::vector<int>>();

    std::print("first 5 squares: ");
    for (auto x : nums | std::views::take(5)) std::print("{} ", x);
    std::println("");

    // 嵌套容器
    std::vector<std::vector<int>> matrix{
        {1, 2, 3}, {4, 5, 6}, {7, 8, 9}
    };

    // 将矩阵展平后收集
    auto flat = matrix
        | std::views::join
        | std::ranges::to<std::vector<int>>();

    std::print("flattened: ");
    for (auto x : flat) std::print("{} ", x);
    std::println("");

    // 反转后收集
    auto rev = flat
        | std::views::reverse
        | std::ranges::to<std::vector<int>>();

    std::print("reversed:  ");
    for (auto x : rev) std::print("{} ", x);
    std::println("");
}

int main() {
    basic_ranges_to();
    ranges_to_different_containers();
    ranges_to_map();
    pipeline_to_demo();
    custom_container_demo();
    return 0;
}
