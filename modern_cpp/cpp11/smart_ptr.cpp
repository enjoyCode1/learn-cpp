#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cassert>

// ==================== 入门级 ====================

struct Node {
    std::string name;
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> parent;  // 用 weak_ptr 避免循环引用

    explicit Node(const std::string& n) : name(n) {
        std::cout << "Node(" << name << ") created\n";
    }
    ~Node() {
        std::cout << "Node(" << name << ") destroyed\n";
    }
};

// unique_ptr：独占所有权
void unique_ptr_demo() {
    std::cout << "\n=== 入门级：unique_ptr ===\n";

    auto p = std::unique_ptr<int>(new int(42));
    std::cout << "p=" << *p << "\n";

    // 转移所有权
    auto p2 = std::move(p);
    std::cout << "p is null: " << (p == nullptr) << "\n";
    std::cout << "p2=" << *p2 << "\n";

    // 自定义删除器
    auto deleter = [](int* ptr) {
        std::cout << "custom deleter called\n";
        delete ptr;
    };
    std::unique_ptr<int, decltype(deleter)> p3(new int(99), deleter);
}

// shared_ptr：共享所有权
void shared_ptr_demo() {
    std::cout << "\n=== 入门级：shared_ptr ===\n";

    auto sp1 = std::make_shared<std::string>("shared");
    std::cout << "use_count=" << sp1.use_count() << "\n";  // 1

    {
        auto sp2 = sp1;
        std::cout << "use_count=" << sp1.use_count() << "\n";  // 2
    }
    std::cout << "use_count=" << sp1.use_count() << "\n";  // 1
}

// weak_ptr：打破循环引用
void weak_ptr_demo() {
    std::cout << "\n=== 入门级：weak_ptr ===\n";

    auto parent = std::make_shared<Node>("parent");
    auto child  = std::make_shared<Node>("child");

    parent->next = child;
    child->parent = parent;  // weak_ptr，不增加引用计数

    std::cout << "parent use_count=" << parent.use_count() << "\n";  // 1
    std::cout << "child use_count="  << child.use_count()  << "\n";  // 2 (parent->next)

    // 通过 weak_ptr 访问
    if (auto p = child->parent.lock()) {
        std::cout << "child's parent: " << p->name << "\n";
    }
}  // parent 和 child 正常析构，无内存泄漏

// ==================== 中级 ====================

// unique_ptr数组管理
void unique_ptr_array() {
    std::cout << "\n=== 中级：unique_ptr管理数组 ===\n";

    // 管理动态数组
    std::unique_ptr<int[]> arr(new int[5]);
    for (int i = 0; i < 5; ++i) {
        arr[i] = i * 10;
    }

    std::cout << "Array: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << "\n";
}

// shared_ptr自定义删除器
void shared_ptr_custom_deleter() {
    std::cout << "\n=== 中级：shared_ptr自定义删除器 ===\n";

    // 文件句柄管理
    auto fileDeleter = [](FILE* fp) {
        if (fp) {
            std::cout << "Closing file\n";
            fclose(fp);
        }
    };

    std::shared_ptr<FILE> file(fopen("test.txt", "w"), fileDeleter);
    if (file) {
        fprintf(file.get(), "Hello from shared_ptr\n");
    }

    // 自定义对象删除器
    auto customDeleter = [](int* p) {
        std::cout << "Custom deleter for value: " << *p << "\n";
        delete p;
    };

    std::shared_ptr<int> sp(new int(100), customDeleter);
}

// enable_shared_from_this：安全地从this创建shared_ptr
class Widget : public std::enable_shared_from_this<Widget> {
public:
    Widget(const std::string& name) : name_(name) {
        std::cout << "Widget(" << name_ << ") created\n";
    }

    ~Widget() {
        std::cout << "Widget(" << name_ << ") destroyed\n";
    }

    // 返回指向自己的shared_ptr
    std::shared_ptr<Widget> getShared() {
        return shared_from_this();
    }

    // 注册回调（需要shared_ptr保持对象存活）
    void registerCallback(std::vector<std::shared_ptr<Widget>>& callbacks) {
        callbacks.push_back(shared_from_this());
    }

    void print() const {
        std::cout << "Widget: " << name_ << "\n";
    }

private:
    std::string name_;
};

void enable_shared_from_this_demo() {
    std::cout << "\n=== 中级：enable_shared_from_this ===\n";

    auto widget = std::make_shared<Widget>("W1");
    std::cout << "use_count=" << widget.use_count() << "\n";  // 1

    auto widget2 = widget->getShared();
    std::cout << "use_count=" << widget.use_count() << "\n";  // 2

    std::vector<std::shared_ptr<Widget>> callbacks;
    widget->registerCallback(callbacks);
    std::cout << "use_count=" << widget.use_count() << "\n";  // 3
}

// weak_ptr检测对象是否存活
void weak_ptr_lifetime_check() {
    std::cout << "\n=== 中级：weak_ptr检测对象存活 ===\n";

    std::weak_ptr<int> weak;

    {
        auto shared = std::make_shared<int>(42);
        weak = shared;

        std::cout << "expired: " << weak.expired() << "\n";  // false
        std::cout << "use_count: " << weak.use_count() << "\n";  // 1

        if (auto locked = weak.lock()) {
            std::cout << "value: " << *locked << "\n";
        }
    }

    // shared_ptr已销毁
    std::cout << "expired: " << weak.expired() << "\n";  // true
    std::cout << "use_count: " << weak.use_count() << "\n";  // 0

    if (auto locked = weak.lock()) {
        std::cout << "value: " << *locked << "\n";
    } else {
        std::cout << "Object has been destroyed\n";
    }
}

// ==================== 高级 ====================

// Aliasing构造函数：shared_ptr指向另一个对象的成员
struct Person {
    std::string name;
    int age;

    Person(const std::string& n, int a) : name(n), age(a) {}
};

void aliasing_constructor() {
    std::cout << "\n=== 高级：Aliasing构造函数 ===\n";

    auto person = std::make_shared<Person>("Alice", 30);

    // 创建一个shared_ptr指向person的name成员
    // 但引用计数跟踪的是person对象
    std::shared_ptr<std::string> namePtr(person, &person->name);

    std::cout << "name: " << *namePtr << "\n";
    std::cout << "person use_count: " << person.use_count() << "\n";  // 2

    person.reset();  // 释放person
    std::cout << "After person.reset(), name still valid: " << *namePtr << "\n";
    // name仍然有效，因为namePtr保持了person对象存活
}

// 智能指针与多态
class Base {
public:
    virtual ~Base() { std::cout << "Base destroyed\n"; }
    virtual void print() const { std::cout << "Base\n"; }
};

class Derived : public Base {
public:
    ~Derived() override { std::cout << "Derived destroyed\n"; }
    void print() const override { std::cout << "Derived\n"; }
};

void polymorphic_smart_ptr() {
    std::cout << "\n=== 高级：智能指针与多态 ===\n";

    // unique_ptr支持多态
    std::unique_ptr<Base> base = std::unique_ptr<Derived>(new Derived());
    base->print();

    // shared_ptr支持多态
    std::shared_ptr<Base> shared = std::make_shared<Derived>();
    shared->print();

    // 向下转型（使用dynamic_pointer_cast）
    std::shared_ptr<Derived> derived = std::dynamic_pointer_cast<Derived>(shared);
    if (derived) {
        std::cout << "Downcast successful\n";
        derived->print();
    }
}

// 智能指针循环引用检测和解决
class Parent;
class Child;

class Parent {
public:
    std::string name;
    std::vector<std::shared_ptr<Child>> children;

    Parent(const std::string& n) : name(n) {
        std::cout << "Parent(" << name << ") created\n";
    }
    ~Parent() {
        std::cout << "Parent(" << name << ") destroyed\n";
    }
};

class Child {
public:
    std::string name;
    std::weak_ptr<Parent> parent;  // 使用weak_ptr打破循环引用

    Child(const std::string& n) : name(n) {
        std::cout << "Child(" << name << ") created\n";
    }
    ~Child() {
        std::cout << "Child(" << name << ") destroyed\n";
    }

    void showParent() const {
        if (auto p = parent.lock()) {
            std::cout << "Child " << name << " -> Parent " << p->name << "\n";
        } else {
            std::cout << "Parent is destroyed\n";
        }
    }
};

void circular_reference_solution() {
    std::cout << "\n=== 高级：循环引用解决方案 ===\n";

    auto parent = std::make_shared<Parent>("Dad");
    auto child1 = std::make_shared<Child>("Son");
    auto child2 = std::make_shared<Child>("Daughter");

    parent->children.push_back(child1);
    parent->children.push_back(child2);

    child1->parent = parent;
    child2->parent = parent;

    child1->showParent();

    std::cout << "parent use_count: " << parent.use_count() << "\n";  // 1
    std::cout << "child1 use_count: " << child1.use_count() << "\n";  // 2
}

// 智能指针工厂模式
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_cpp11(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void smart_ptr_factory() {
    std::cout << "\n=== 高级：智能指针工厂 ===\n";

    // C++11没有std::make_unique，手动实现
    auto ptr = make_unique_cpp11<std::string>("Factory pattern");
    std::cout << "Created: " << *ptr << "\n";
}

// 智能指针作为容器元素
void smart_ptr_in_container() {
    std::cout << "\n=== 高级：容器中的智能指针 ===\n";

    // vector存储unique_ptr（不可复制，只能移动）
    std::vector<std::unique_ptr<int>> uniqueVec;
    uniqueVec.push_back(std::unique_ptr<int>(new int(1)));
    uniqueVec.push_back(std::unique_ptr<int>(new int(2)));
    uniqueVec.push_back(std::unique_ptr<int>(new int(3)));

    std::cout << "uniqueVec: ";
    for (const auto& ptr : uniqueVec) {
        std::cout << *ptr << " ";
    }
    std::cout << "\n";

    // map存储shared_ptr
    std::map<std::string, std::shared_ptr<Widget>> widgetMap;
    widgetMap["w1"] = std::make_shared<Widget>("Widget1");
    widgetMap["w2"] = std::make_shared<Widget>("Widget2");

    for (const auto& pair : widgetMap) {
        std::cout << pair.first << ": ";
        pair.second->print();
    }
}

// 资源管理：RAII模式
class Resource {
public:
    Resource(const std::string& name) : name_(name) {
        std::cout << "Acquiring resource: " << name_ << "\n";
    }

    ~Resource() {
        std::cout << "Releasing resource: " << name_ << "\n";
    }

    void use() const {
        std::cout << "Using resource: " << name_ << "\n";
    }

private:
    std::string name_;
};

void raii_with_smart_ptr() {
    std::cout << "\n=== 高级：RAII模式 ===\n";

    {
        auto res = std::make_shared<Resource>("Database connection");
        res->use();

        // 异常安全：即使抛出异常，资源也会被正确释放
        try {
            auto res2 = std::make_shared<Resource>("File handle");
            res2->use();
            // throw std::runtime_error("Error");
        } catch (...) {
            std::cout << "Exception caught\n";
        }
    }

    std::cout << "Resources automatically released\n";
}

int main() {
    // 入门级
    unique_ptr_demo();
    shared_ptr_demo();
    weak_ptr_demo();

    // 中级
    unique_ptr_array();
    shared_ptr_custom_deleter();
    enable_shared_from_this_demo();
    weak_ptr_lifetime_check();

    // 高级
    aliasing_constructor();
    polymorphic_smart_ptr();
    circular_reference_solution();
    smart_ptr_factory();
    smart_ptr_in_container();
    raii_with_smart_ptr();

    return 0;
}
