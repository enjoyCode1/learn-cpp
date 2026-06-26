#include <iostream>
#include <vector>
#include <map>
#include <initializer_list>
#include <string>

// 接收 initializer_list 的函数
void printAll(std::initializer_list<int> list) {
    for (auto v : list) std::cout << v << " ";
    std::cout << "\n";
}

// 支持 initializer_list 的自定义类
class NumberSet {
public:
    NumberSet(std::initializer_list<int> list) : data_(list) {}

    void print() const {
        for (auto v : data_) std::cout << v << " ";
        std::cout << "\n";
    }
private:
    std::vector<int> data_;
};

int main() {
    // 范围 for：vector
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "range for vector: ";
    for (auto v : vec) std::cout << v << " ";
    std::cout << "\n";

    // 范围 for：引用修改元素
    for (auto& v : vec) v *= 2;
    std::cout << "after *=2: ";
    for (auto v : vec) std::cout << v << " ";
    std::cout << "\n";

    // 范围 for：map
    std::map<std::string, int> scores = {{"Alice", 90}, {"Bob", 85}, {"Carol", 92}};
    std::cout << "range for map:\n";
    for (const auto& pair : scores) {
        std::cout << "  " << pair.first << ": " << pair.second << "\n";
    }

    // 范围 for：原生数组
    int arr[] = {10, 20, 30};
    std::cout << "range for array: ";
    for (auto v : arr) std::cout << v << " ";
    std::cout << "\n";

    // initializer_list
    printAll({1, 2, 3, 4, 5});

    // 自定义类使用 initializer_list 构造
    NumberSet ns = {10, 20, 30, 40};
    ns.print();

    return 0;
}
