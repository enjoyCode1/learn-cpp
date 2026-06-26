#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <sstream>

// ==================== 入门级 ====================

void basic_string_view() {
    std::cout << "=== 入门：string_view 基础 ===\n";

    // string_view 不拥有数据，零拷贝
    std::string_view sv1 = "hello world";
    std::cout << "sv1=" << sv1 << " size=" << sv1.size() << "\n";

    std::string s = "C++ string";
    std::string_view sv2 = s;  // 指向 std::string 的内容
    std::cout << "sv2=" << sv2 << "\n";

    // 字符串字面量的 view
    std::string_view sv3{"hello", 5};
    std::cout << "sv3=" << sv3 << "\n";

    // 常用操作
    std::cout << "starts_with 'hello': " << (sv1.substr(0, 5) == "hello") << "\n";
    std::cout << "find 'world': " << sv1.find("world") << "\n";
}

// ==================== 中级 ====================

// 函数参数用 string_view 避免拷贝，同时接受 const char* 和 std::string
size_t count_vowels(std::string_view sv) {
    size_t count = 0;
    for (char c : sv)
        if (c=='a'||c=='e'||c=='i'||c=='o'||c=='u'||
            c=='A'||c=='E'||c=='I'||c=='O'||c=='U')
            ++count;
    return count;
}

bool starts_with(std::string_view sv, std::string_view prefix) {
    return sv.size() >= prefix.size() &&
           sv.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view sv, std::string_view suffix) {
    return sv.size() >= suffix.size() &&
           sv.substr(sv.size() - suffix.size()) == suffix;
}

void string_view_as_param() {
    std::cout << "\n=== 中级：string_view 作为函数参数 ===\n";

    std::cout << "vowels in 'Hello World': " << count_vowels("Hello World") << "\n";

    std::string s = "programming";
    std::cout << "vowels in '" << s << "': " << count_vowels(s) << "\n";

    std::cout << "starts_with 'pro': " << starts_with(s, "pro") << "\n";
    std::cout << "ends_with 'ing':   " << ends_with(s, "ing")   << "\n";
}

// 用 string_view 做零拷贝分割
std::vector<std::string_view> split(std::string_view sv, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    while (start < sv.size()) {
        size_t pos = sv.find(delim, start);
        if (pos == std::string_view::npos) {
            result.push_back(sv.substr(start));
            break;
        }
        result.push_back(sv.substr(start, pos - start));
        start = pos + 1;
    }
    return result;
}

void zero_copy_split() {
    std::cout << "\n=== 中级：零拷贝字符串分割 ===\n";

    std::string csv = "Alice,Bob,Charlie,Dave";
    auto parts = split(csv, ',');
    for (auto part : parts)
        std::cout << "  [" << part << "]\n";
}

// ==================== 高级 ====================

// string_view 的注意事项：生命周期陷阱
void lifetime_pitfall_demo() {
    std::cout << "\n=== 高级：string_view 生命周期注意 ===\n";

    std::string_view sv;
    {
        std::string temp = "temporary string";
        sv = temp;
        std::cout << "inside scope: sv=" << sv << "\n";
        // temp 析构后 sv 悬空！下面不能再用 sv
    }
    // sv 此时悬空，不能访问——这里只演示原理，实际代码避免此用法
    std::cout << "string_view must not outlive its source string!\n";
}

// string_view 用于编译期字符串处理
constexpr size_t constexpr_strlen(std::string_view sv) {
    return sv.size();
}

constexpr bool constexpr_contains(std::string_view sv, std::string_view sub) {
    return sv.find(sub) != std::string_view::npos;
}

void constexpr_string_view() {
    std::cout << "\n=== 高级：constexpr string_view ===\n";

    constexpr std::string_view hello = "hello world";
    constexpr size_t len = constexpr_strlen(hello);
    constexpr bool has_world = constexpr_contains(hello, "world");

    std::cout << "len=" << len << " has_world=" << has_world << "\n";

    // 编译期字符串 switch（用 hash 模拟）
    auto parse_cmd = [](std::string_view cmd) {
        if (cmd == "quit")  return 0;
        if (cmd == "start") return 1;
        if (cmd == "stop")  return 2;
        return -1;
    };

    std::cout << "quit="  << parse_cmd("quit")  << "\n";
    std::cout << "start=" << parse_cmd("start") << "\n";
    std::cout << "unknown=" << parse_cmd("xyz") << "\n";
}

int main() {
    basic_string_view();
    string_view_as_param();
    zero_copy_split();
    lifetime_pitfall_demo();
    constexpr_string_view();
    return 0;
}
