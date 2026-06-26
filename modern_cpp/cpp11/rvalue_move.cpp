#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <memory>

// ==================== 入门级 ====================

// 演示移动语义的简单资源类
class Buffer {
public:
    explicit Buffer(size_t size) : size_(size), data_(new int[size]) {
        std::cout << "Buffer constructed, size=" << size_ << "\n";
    }

    // 拷贝构造
    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        std::copy(other.data_, other.data_ + size_, data_);
        std::cout << "Buffer copy constructed\n";
    }

    // 移动构造：窃取资源，不做深拷贝
    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "Buffer move constructed\n";
    }

    // 拷贝赋值
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = new int[other.size_];
            std::copy(other.data_, other.data_ + size_, data_);
            std::cout << "Buffer copy assigned\n";
        }
        return *this;
    }

    // 移动赋值
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
            std::cout << "Buffer move assigned\n";
        }
        return *this;
    }

    ~Buffer() {
        delete[] data_;
        std::cout << "Buffer destructed, size=" << size_ << "\n";
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    int* data_;
};

void basic_move_semantics() {
    std::cout << "\n=== 入门级：基本移动语义 ===\n";

    // 右值引用
    std::string s = "hello";
    std::string&& rref = std::move(s);
    std::cout << "rref=" << rref << "\n";

    // std::move 触发移动构造
    Buffer b1(10);
    Buffer b2 = std::move(b1);
    std::cout << "b1.size=" << b1.size() << " b2.size=" << b2.size() << "\n";

    // vector 存储时的移动
    std::vector<Buffer> vec;
    vec.push_back(Buffer(5));   // 临时对象，触发移动构造
    std::cout << "vec[0].size=" << vec[0].size() << "\n";
}

// ==================== 中级 ====================

// 左值 vs 右值
void lvalue_rvalue_basics() {
    std::cout << "\n=== 中级：左值与右值 ===\n";

    int x = 10;        // x是左值
    int y = x + 5;     // x+5是右值

    // int& ref1 = x + 5;     // 错误：不能将右值绑定到左值引用
    const int& ref2 = x + 5;  // OK：const左值引用可以绑定到右值
    int&& rref = x + 5;       // OK：右值引用绑定到右值

    std::cout << "rref=" << rref << "\n";

    // std::move将左值转换为右值
    int&& rref2 = std::move(x);
    std::cout << "rref2=" << rref2 << "\n";
    // 注意：x仍然可以访问，但处于"有效但未指定"状态
}

// 移动语义优化性能
class HeavyObject {
public:
    HeavyObject(size_t size) : size_(size), data_(new int[size]) {
        std::cout << "HeavyObject(" << size_ << ") constructed\n";
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = i;
        }
    }

    // 拷贝构造（昂贵）
    HeavyObject(const HeavyObject& other)
        : size_(other.size_), data_(new int[other.size_]) {
        std::cout << "HeavyObject copy constructed (expensive!)\n";
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // 移动构造（廉价）
    HeavyObject(HeavyObject&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        std::cout << "HeavyObject move constructed (cheap!)\n";
        other.size_ = 0;
        other.data_ = nullptr;
    }

    ~HeavyObject() {
        delete[] data_;
    }

    size_t size() const { return size_; }

private:
    size_t size_;
    int* data_;
};

void move_optimization() {
    std::cout << "\n=== 中级：移动优化性能 ===\n";

    std::vector<HeavyObject> vec;

    // 使用拷贝（昂贵）
    HeavyObject obj1(1000);
    vec.push_back(obj1);  // 拷贝

    // 使用移动（廉价）
    HeavyObject obj2(1000);
    vec.push_back(std::move(obj2));  // 移动

    // 临时对象自动移动
    vec.push_back(HeavyObject(1000));  // 自动移动
}

// 返回值优化 (RVO/NRVO)
HeavyObject createObject(size_t size) {
    HeavyObject obj(size);
    return obj;  // NRVO: Named Return Value Optimization
}

HeavyObject createObjectTemp(size_t size) {
    return HeavyObject(size);  // RVO: Return Value Optimization
}

void return_value_optimization() {
    std::cout << "\n=== 中级：返回值优化 ===\n";

    // 编译器优化：通常不会调用拷贝或移动构造
    auto obj1 = createObject(100);
    auto obj2 = createObjectTemp(100);

    std::cout << "obj1.size=" << obj1.size() << "\n";
    std::cout << "obj2.size=" << obj2.size() << "\n";
}

// ==================== 高级 ====================

// 万能引用（Universal Reference）与引用折叠
template<typename T>
void universalRef(T&& param) {
    std::cout << "Universal reference called\n";
}

void universal_reference_demo() {
    std::cout << "\n=== 高级：万能引用 ===\n";

    int x = 10;
    universalRef(x);           // T推导为int&, T&&折叠为int&
    universalRef(std::move(x)); // T推导为int, T&&为int&&
    universalRef(42);          // T推导为int, T&&为int&&

    // 引用折叠规则：
    // T& &   -> T&
    // T& &&  -> T&
    // T&& &  -> T&
    // T&& && -> T&&
}

// 完美转发：保持参数的值类别
template<typename T>
void process(T&& param) {
    std::cout << "Processing value\n";
}

template<typename T>
void forwardWrapper(T&& arg) {
    // std::forward保持参数的值类别（左值或右值）
    process(std::forward<T>(arg));
}

void perfect_forwarding() {
    std::cout << "\n=== 高级：完美转发 ===\n";

    int x = 10;
    forwardWrapper(x);           // 转发左值
    forwardWrapper(std::move(x)); // 转发右值
    forwardWrapper(42);          // 转发右值
}

// 完美转发工厂函数
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_advanced(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class Product {
public:
    Product(const std::string& name, int id)
        : name_(name), id_(id) {
        std::cout << "Product(" << name_ << ", " << id_ << ") created\n";
    }

    void show() const {
        std::cout << "Product: " << name_ << ", ID: " << id_ << "\n";
    }

private:
    std::string name_;
    int id_;
};

void perfect_forwarding_factory() {
    std::cout << "\n=== 高级：完美转发工厂 ===\n";

    std::string name = "Widget";
    auto product1 = make_unique_advanced<Product>(name, 100);  // 左值
    auto product2 = make_unique_advanced<Product>("Gadget", 200);  // 右值

    product1->show();
    product2->show();
}

// 移动语义与异常安全
class SafeBuffer {
public:
    explicit SafeBuffer(size_t size) : size_(size), data_(new int[size]) {
        std::cout << "SafeBuffer(" << size_ << ") constructed\n";
    }

    // 移动构造函数标记为noexcept，确保异常安全
    SafeBuffer(SafeBuffer&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
        std::cout << "SafeBuffer move constructed (noexcept)\n";
    }

    // 如果移动构造不是noexcept，std::vector等容器会退回使用拷贝
    SafeBuffer& operator=(SafeBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

    ~SafeBuffer() { delete[] data_; }

private:
    size_t size_;
    int* data_;
};

void move_and_exception_safety() {
    std::cout << "\n=== 高级：移动语义与异常安全 ===\n";

    std::vector<SafeBuffer> vec;
    vec.reserve(2);

    vec.push_back(SafeBuffer(10));
    vec.push_back(SafeBuffer(20));

    // 扩容时，如果移动构造是noexcept，使用移动；否则使用拷贝
    vec.push_back(SafeBuffer(30));
}

// 移动语义与Rule of Five
class RuleOfFive {
public:
    explicit RuleOfFive(size_t size = 0)
        : size_(size), data_(size > 0 ? new int[size] : nullptr) {
        std::cout << "Constructor\n";
    }

    // 1. 析构函数
    ~RuleOfFive() {
        std::cout << "Destructor\n";
        delete[] data_;
    }

    // 2. 拷贝构造函数
    RuleOfFive(const RuleOfFive& other)
        : size_(other.size_), data_(other.size_ > 0 ? new int[other.size_] : nullptr) {
        std::cout << "Copy constructor\n";
        if (data_) {
            std::copy(other.data_, other.data_ + size_, data_);
        }
    }

    // 3. 拷贝赋值运算符
    RuleOfFive& operator=(const RuleOfFive& other) {
        std::cout << "Copy assignment\n";
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.size_ > 0 ? new int[other.size_] : nullptr;
            if (data_) {
                std::copy(other.data_, other.data_ + size_, data_);
            }
        }
        return *this;
    }

    // 4. 移动构造函数
    RuleOfFive(RuleOfFive&& other) noexcept
        : size_(other.size_), data_(other.data_) {
        std::cout << "Move constructor\n";
        other.size_ = 0;
        other.data_ = nullptr;
    }

    // 5. 移动赋值运算符
    RuleOfFive& operator=(RuleOfFive&& other) noexcept {
        std::cout << "Move assignment\n";
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            data_ = other.data_;
            other.size_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

private:
    size_t size_;
    int* data_;
};

void rule_of_five_demo() {
    std::cout << "\n=== 高级：Rule of Five ===\n";

    RuleOfFive obj1(10);
    RuleOfFive obj2 = obj1;              // 拷贝构造
    RuleOfFive obj3 = std::move(obj1);   // 移动构造

    RuleOfFive obj4(5);
    obj4 = obj2;              // 拷贝赋值
    obj4 = std::move(obj3);   // 移动赋值
}

// 条件性移动：仅在有益时移动
template<typename T>
T conditionalMove(T value, bool shouldMove) {
    if (shouldMove) {
        return std::move(value);  // 移动
    }
    return value;  // 拷贝
}

void conditional_move_demo() {
    std::cout << "\n=== 高级：条件性移动 ===\n";

    std::string str1 = "original";
    std::string str2 = conditionalMove(str1, true);

    std::cout << "str1: " << str1 << " (moved from)\n";
    std::cout << "str2: " << str2 << "\n";

    std::string str3 = "another";
    std::string str4 = conditionalMove(str3, false);

    std::cout << "str3: " << str3 << " (copied)\n";
    std::cout << "str4: " << str4 << "\n";
}

// 移动捕获（C++14特性，但可以用bind模拟）
void move_capture_simulation() {
    std::cout << "\n=== 高级：移动捕获模拟 ===\n";

    // C++11没有移动捕获，但可以通过bind模拟
    auto ptr = std::unique_ptr<int>(new int(42));

    // 使用bind模拟移动捕获
    auto lambda = std::bind([](const std::unique_ptr<int>& p) {
        std::cout << "Value: " << *p << "\n";
    }, std::move(ptr));

    lambda();
    // ptr现在为空
    std::cout << "ptr is null: " << (ptr == nullptr) << "\n";
}

// std::move的实现原理
template<typename T>
typename std::remove_reference<T>::type&& my_move(T&& arg) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

void move_implementation() {
    std::cout << "\n=== 高级：std::move实现原理 ===\n";

    std::string str = "test";
    std::string moved = my_move(str);

    std::cout << "Original: " << str << " (moved from)\n";
    std::cout << "Moved: " << moved << "\n";
}

int main() {
    // 入门级
    basic_move_semantics();

    // 中级
    lvalue_rvalue_basics();
    move_optimization();
    return_value_optimization();

    // 高级
    universal_reference_demo();
    perfect_forwarding();
    perfect_forwarding_factory();
    move_and_exception_safety();
    rule_of_five_demo();
    conditional_move_demo();
    move_capture_simulation();
    move_implementation();

    return 0;
}
