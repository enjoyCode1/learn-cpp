#include <iostream>

// constexpr 函数：编译期计算
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

constexpr int square(int x) {
    return x * x;
}

// constexpr 对象
constexpr double PI = 3.14159265358979;

// nullptr 演示
void func(int* p) {
    std::cout << "func(int*)\n";
}

void func(int n) {
    std::cout << "func(int) n=" << n << "\n";
}

int main() {
    // nullptr 避免 NULL/0 的二义性
    int* p = nullptr;
    std::cout << "p is null: " << (p == nullptr) << "\n";

    func(nullptr);  // 明确调用 func(int*)
    func(0);        // 调用 func(int)
    // func(NULL);  // 可能有二义性警告

    // constexpr 编译期计算
    constexpr int f5 = factorial(5);
    constexpr int s7 = square(7);
    std::cout << "factorial(5)=" << f5 << "\n";  // 120
    std::cout << "square(7)="    << s7 << "\n";  // 49

    // 编译期数组大小
    constexpr int SIZE = square(4);
    int arr[SIZE];  // 合法，SIZE 是编译期常量
    std::cout << "arr size=" << SIZE << "\n";

    std::cout << "PI=" << PI << "\n";

    return 0;
}
