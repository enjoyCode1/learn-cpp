#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

// ============================================================
// 静态数组（固定大小）实现
// 封装原生数组，提供边界检查、迭代器等功能
// ============================================================

template<typename T, size_t N>
class Array {
public:
    // 默认构造：所有元素零初始化
    Array() { std::fill(data_, data_ + N, T{}); }

    // 初始化列表构造
    Array(std::initializer_list<T> il) {
        size_t i = 0;
        for (auto& v : il) {
            if (i >= N) break;
            data_[i++] = v;
        }
        // 剩余元素零初始化
        while (i < N) data_[i++] = T{};
    }

    // 带边界检查的访问
    T& at(size_t i) {
        if (i >= N) throw std::out_of_range("Array::at out of range");
        return data_[i];
    }
    const T& at(size_t i) const {
        if (i >= N) throw std::out_of_range("Array::at out of range");
        return data_[i];
    }

    // 不检查边界的访问（用于性能敏感路径）
    T& operator[](size_t i)       { return data_[i]; }
    const T& operator[](size_t i) const { return data_[i]; }

    T& front() { return data_[0]; }
    T& back()  { return data_[N - 1]; }

    size_t size() const { return N; }
    bool   empty() const { return N == 0; }

    // 迭代器支持（范围 for）
    T* begin() { return data_; }
    T* end()   { return data_ + N; }
    const T* begin() const { return data_; }
    const T* end()   const { return data_ + N; }

    // 填充全部元素
    void fill(const T& val) { std::fill(data_, data_ + N, val); }

    // 交换两个数组
    void swap(Array& other) { std::swap_ranges(data_, data_ + N, other.data_); }

    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < N; ++i) {
            std::cout << data_[i];
            if (i + 1 < N) std::cout << ", ";
        }
        std::cout << "]\n";
    }

private:
    T data_[N];
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：Array 基础操作 ===\n";

    Array<int, 5> arr{1, 2, 3, 4, 5};
    arr.print();

    // 随机访问 O(1)
    std::cout << "arr[2] = " << arr[2] << "\n";
    arr[2] = 99;
    arr.print();

    // 边界检查
    try {
        arr.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "caught: " << e.what() << "\n";
    }

    // 遍历
    int sum = 0;
    for (auto v : arr) sum += v;
    std::cout << "sum = " << sum << "\n";
}

// ==================== 中级：排序与查找 ====================
void sort_search_demo() {
    std::cout << "\n=== 中级：排序与查找 ===\n";

    Array<int, 8> arr{5, 3, 8, 1, 9, 2, 7, 4};
    std::cout << "before sort: "; arr.print();

    // std::sort 利用迭代器接口
    std::sort(arr.begin(), arr.end());
    std::cout << "after sort:  "; arr.print();

    // 二分查找 O(log n)
    int target = 7;
    auto it = std::lower_bound(arr.begin(), arr.end(), target);
    if (it != arr.end() && *it == target)
        std::cout << "found " << target << " at index " << (it - arr.begin()) << "\n";

    // 线性查找 O(n)
    auto it2 = std::find(arr.begin(), arr.end(), 3);
    std::cout << "linear find 3 at index " << (it2 - arr.begin()) << "\n";
}

// ==================== 高级：二维数组模拟矩阵 ====================
template<typename T, size_t R, size_t C>
class Matrix {
public:
    Matrix() { data_.fill(T{}); }

    T& operator()(size_t r, size_t c)       { return data_[r * C + c]; }
    const T& operator()(size_t r, size_t c) const { return data_[r * C + c]; }

    // 矩阵乘法（要求 C == R2）
    template<size_t C2>
    Matrix<T, R, C2> multiply(const Matrix<T, C, C2>& other) const {
        Matrix<T, R, C2> result;
        for (size_t i = 0; i < R; ++i)
            for (size_t k = 0; k < C; ++k)
                for (size_t j = 0; j < C2; ++j)
                    result(i, j) += (*this)(i, k) * other(k, j);
        return result;
    }

    void print() const {
        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j)
                std::cout << data_[i * C + j] << "\t";
            std::cout << "\n";
        }
    }

private:
    Array<T, R * C> data_;
};

void matrix_demo() {
    std::cout << "\n=== 高级：二维矩阵（基于 Array）===\n";

    Matrix<int, 2, 3> a;
    Matrix<int, 3, 2> b;

    // 填充 a: [[1,2,3],[4,5,6]]
    int v = 1;
    for (size_t i = 0; i < 2; ++i)
        for (size_t j = 0; j < 3; ++j)
            a(i, j) = v++;

    // 填充 b: [[7,8],[9,10],[11,12]]
    v = 7;
    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 2; ++j)
            b(i, j) = v++;

    std::cout << "Matrix A (2x3):\n"; a.print();
    std::cout << "Matrix B (3x2):\n"; b.print();

    auto c = a.multiply(b);
    std::cout << "A x B (2x2):\n"; c.print();
}

int main() {
    basic_demo();
    sort_search_demo();
    matrix_demo();
    return 0;
}
