#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

// ============================================================
// 动态数组（Vector）实现
// 支持动态扩容，摊销 O(1) 尾部插入
// ============================================================

template<typename T>
class Vector {
public:
    Vector() : data_(nullptr), size_(0), capacity_(0) {}

    explicit Vector(size_t n, const T& val = T{})
        : data_(new T[n]), size_(n), capacity_(n) {
        std::fill(data_, data_ + n, val);
    }

    Vector(std::initializer_list<T> il)
        : data_(new T[il.size()]), size_(il.size()), capacity_(il.size()) {
        std::copy(il.begin(), il.end(), data_);
    }

    // 拷贝构造
    Vector(const Vector& other)
        : data_(new T[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // 移动构造：直接接管资源，O(1)
    Vector(Vector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = other.capacity_ = 0;
    }

    ~Vector() { delete[] data_; }

    Vector& operator=(const Vector& other) {
        if (this != &other) {
            Vector tmp(other);
            swap(tmp);
        }
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        swap(other);
        return *this;
    }

    // 尾部插入：摊销 O(1)
    void push_back(const T& val) {
        if (size_ == capacity_) grow();
        data_[size_++] = val;
    }

    void push_back(T&& val) {
        if (size_ == capacity_) grow();
        data_[size_++] = std::move(val);
    }

    // 尾部删除：O(1)
    void pop_back() {
        if (size_ == 0) throw std::underflow_error("Vector::pop_back on empty");
        --size_;
    }

    // 指定位置插入：O(n)
    void insert(size_t pos, const T& val) {
        if (pos > size_) throw std::out_of_range("Vector::insert out of range");
        if (size_ == capacity_) grow();
        // 向后移动腾出空间
        for (size_t i = size_; i > pos; --i)
            data_[i] = std::move(data_[i - 1]);
        data_[pos] = val;
        ++size_;
    }

    // 指定位置删除：O(n)
    void erase(size_t pos) {
        if (pos >= size_) throw std::out_of_range("Vector::erase out of range");
        for (size_t i = pos; i + 1 < size_; ++i)
            data_[i] = std::move(data_[i + 1]);
        --size_;
    }

    T& at(size_t i) {
        if (i >= size_) throw std::out_of_range("Vector::at out of range");
        return data_[i];
    }
    const T& at(size_t i) const {
        if (i >= size_) throw std::out_of_range("Vector::at out of range");
        return data_[i];
    }

    T& operator[](size_t i)             { return data_[i]; }
    const T& operator[](size_t i) const { return data_[i]; }

    T& front() { return data_[0]; }
    T& back()  { return data_[size_ - 1]; }

    size_t size()     const { return size_; }
    size_t capacity() const { return capacity_; }
    bool   empty()    const { return size_ == 0; }

    void clear() { size_ = 0; }

    // 预留空间，避免多次扩容
    void reserve(size_t n) {
        if (n > capacity_) reallocate(n);
    }

    // 收缩到实际大小
    void shrink_to_fit() {
        if (size_ < capacity_) reallocate(size_);
    }

    T* begin() { return data_; }
    T* end()   { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end()   const { return data_ + size_; }

    void swap(Vector& other) noexcept {
        std::swap(data_,     other.data_);
        std::swap(size_,     other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void print() const {
        std::cout << "[";
        for (size_t i = 0; i < size_; ++i) {
            std::cout << data_[i];
            if (i + 1 < size_) std::cout << ", ";
        }
        std::cout << "] size=" << size_ << " cap=" << capacity_ << "\n";
    }

private:
    T*     data_;
    size_t size_;
    size_t capacity_;

    // 2 倍扩容策略，均摊 O(1) push_back
    void grow() {
        size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
        reallocate(new_cap);
    }

    void reallocate(size_t new_cap) {
        T* new_data = new T[new_cap];
        for (size_t i = 0; i < size_; ++i)
            new_data[i] = std::move(data_[i]);
        delete[] data_;
        data_     = new_data;
        capacity_ = new_cap;
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：Vector 基础操作 ===\n";

    Vector<int> v;
    for (int i = 1; i <= 5; ++i) v.push_back(i);
    v.print();

    v.pop_back();
    std::cout << "after pop_back: "; v.print();

    v.insert(2, 99);
    std::cout << "insert 99 at 2: "; v.print();

    v.erase(2);
    std::cout << "erase at 2:     "; v.print();
}

// ==================== 中级：扩容过程演示 ====================
void growth_demo() {
    std::cout << "\n=== 中级：2 倍扩容策略 ===\n";

    Vector<int> v;
    size_t last_cap = 0;
    for (int i = 0; i < 20; ++i) {
        v.push_back(i);
        if (v.capacity() != last_cap) {
            std::cout << "size=" << v.size()
                      << " capacity grew to " << v.capacity() << "\n";
            last_cap = v.capacity();
        }
    }

    v.shrink_to_fit();
    std::cout << "after shrink_to_fit: cap=" << v.capacity() << "\n";
}

// ==================== 高级：移动语义性能对比 ====================
struct HeavyObject {
    std::string name;
    Vector<int> data;

    HeavyObject() = default;  // 默认构造（reallocate 需要）

    HeavyObject(std::string n, int size)
        : name(std::move(n)), data(size, 0) {}

    // 拷贝构造：深拷贝
    HeavyObject(const HeavyObject& o) : name(o.name), data(o.data) {
        std::cout << "  [copy] " << name << "\n";
    }

    // 移动构造：零拷贝
    HeavyObject(HeavyObject&& o) noexcept
        : name(std::move(o.name)), data(std::move(o.data)) {
        std::cout << "  [move] " << name << "\n";
    }

    // 拷贝赋值
    HeavyObject& operator=(const HeavyObject& o) {
        if (this != &o) { name = o.name; data = o.data; }
        return *this;
    }

    // 移动赋值
    HeavyObject& operator=(HeavyObject&& o) noexcept {
        if (this != &o) { name = std::move(o.name); data = std::move(o.data); }
        return *this;
    }
};

void move_semantics_demo() {
    std::cout << "\n=== 高级：移动语义 ===\n";

    Vector<HeavyObject> vec;
    vec.reserve(4);  // 预留空间，避免扩容时触发额外拷贝/移动

    std::cout << "push_back lvalue (copy):\n";
    HeavyObject obj("obj1", 1000);
    vec.push_back(obj);  // 拷贝

    std::cout << "push_back rvalue (move):\n";
    vec.push_back(std::move(obj));  // 移动，obj 之后为空

    std::cout << "push_back temporary (move):\n";
    vec.push_back(HeavyObject("obj3", 1000));  // 临时对象，直接移动

    std::cout << "total elements: " << vec.size() << "\n";
}

int main() {
    basic_demo();
    growth_demo();
    move_semantics_demo();
    return 0;
}
