#include <iostream>
#include <string>

// 传统 enum：污染命名空间，隐式转换为 int
enum OldColor { RED, GREEN, BLUE };

// enum class：作用域限定，类型安全，无隐式转换
enum class Color { Red, Green, Blue };

// 指定底层类型
enum class Status : uint8_t {
    Ok      = 0,
    Error   = 1,
    Timeout = 2,
    Unknown = 255
};

// 在 switch 中使用
std::string colorName(Color c) {
    switch (c) {
        case Color::Red:   return "Red";
        case Color::Green: return "Green";
        case Color::Blue:  return "Blue";
    }
    return "Unknown";
}

std::string statusMsg(Status s) {
    switch (s) {
        case Status::Ok:      return "OK";
        case Status::Error:   return "Error";
        case Status::Timeout: return "Timeout";
        case Status::Unknown: return "Unknown";
    }
    return "";
}

int main() {
    // 传统 enum 隐式转换（不推荐）
    OldColor oc = RED;
    int ocVal = oc;  // 隐式转换为 int
    std::cout << "OldColor RED=" << ocVal << "\n";

    // enum class 类型安全
    Color c = Color::Green;
    std::cout << "Color::Green=" << colorName(c) << "\n";

    // 必须显式转换才能得到底层值
    int cVal = static_cast<int>(c);
    std::cout << "static_cast<int>(Color::Green)=" << cVal << "\n";

    // 指定底层类型
    Status s = Status::Timeout;
    std::cout << "Status::Timeout=" << statusMsg(s)
              << " underlying=" << static_cast<int>(s) << "\n";

    // enum class 不会与其他枚举的成员冲突
    Color r = Color::Red;
    // OldColor r2 = RED;  // RED 在全局作用域，可能冲突
    std::cout << "Color::Red=" << colorName(r) << "\n";

    return 0;
}
