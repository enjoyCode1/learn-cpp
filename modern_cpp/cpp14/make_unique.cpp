#include <iostream>
#include <memory>
#include <vector>
#include <string>

// ==================== 入门级 ====================

struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    void print() const { std::cout << "Point(" << x << ", " << y << ")\n"; }
};

void basic_make_unique() {
    std::cout << "=== 入门：make_unique 基础 ===\n";

    // C++11 写法：new 裸露，异常不安全
    // std::unique_ptr<Point> p1(new Point(1, 2));

    // C++14：make_unique，更安全，更简洁
    auto p = std::make_unique<Point>(3, 4);
    p->print();

    // 数组形式
    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) arr[i] = i * i;
    for (int i = 0; i < 5; ++i) std::cout << arr[i] << " ";
    std::cout << "\n";
}

// ==================== 中级 ====================

class Resource {
public:
    explicit Resource(std::string name) : name_(std::move(name)) {
        std::cout << "Resource[" << name_ << "] acquired\n";
    }
    ~Resource() {
        std::cout << "Resource[" << name_ << "] released\n";
    }
    const std::string& name() const { return name_; }
private:
    std::string name_;
};

// make_unique 与所有权转移
void ownership_transfer() {
    std::cout << "\n=== 中级：所有权转移 ===\n";

    auto r = std::make_unique<Resource>("A");
    std::cout << "owner: main, resource=" << r->name() << "\n";

    // 转移所有权
    auto r2 = std::move(r);
    std::cout << "owner: r2, resource=" << r2->name() << "\n";
    std::cout << "r is null: " << (r == nullptr) << "\n";
}

// make_unique 存入容器
void unique_ptr_in_container() {
    std::cout << "\n=== 中级：unique_ptr 存入容器 ===\n";

    std::vector<std::unique_ptr<Resource>> resources;
    resources.push_back(std::make_unique<Resource>("R1"));
    resources.push_back(std::make_unique<Resource>("R2"));
    resources.push_back(std::make_unique<Resource>("R3"));

    std::cout << "resources:\n";
    for (const auto& r : resources)
        std::cout << "  " << r->name() << "\n";
}

// ==================== 高级 ====================

// 工厂函数模板，返回 unique_ptr
template<typename T, typename... Args>
std::unique_ptr<T> make_resource(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// 多态 + unique_ptr
struct Shape {
    virtual ~Shape() = default;
    virtual void draw() const = 0;
};

struct Circle : Shape {
    double radius;
    explicit Circle(double r) : radius(r) {}
    void draw() const override {
        std::cout << "Circle(r=" << radius << ")\n";
    }
};

struct Rectangle : Shape {
    double w, h;
    Rectangle(double w, double h) : w(w), h(h) {}
    void draw() const override {
        std::cout << "Rectangle(" << w << "x" << h << ")\n";
    }
};

void polymorphism_with_unique_ptr() {
    std::cout << "\n=== 高级：多态与 unique_ptr ===\n";

    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(make_resource<Circle>(5.0));
    shapes.push_back(make_resource<Rectangle>(3.0, 4.0));
    shapes.push_back(make_resource<Circle>(2.5));

    for (const auto& s : shapes) s->draw();
}

// unique_ptr 自定义删除器
void custom_deleter() {
    std::cout << "\n=== 高级：自定义删除器 ===\n";

    auto deleter = [](int* p) {
        std::cout << "custom deleter: freeing " << *p << "\n";
        delete p;
    };

    std::unique_ptr<int, decltype(deleter)> p(new int(99), deleter);
    std::cout << "value: " << *p << "\n";
}

int main() {
    basic_make_unique();
    ownership_transfer();
    unique_ptr_in_container();
    polymorphism_with_unique_ptr();
    custom_deleter();
    return 0;
}
