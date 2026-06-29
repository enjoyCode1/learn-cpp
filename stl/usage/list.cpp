#include <iostream>
#include <list>
#include <algorithm>
#include <string>

// ============================================================
// std::list 从入门到高级用法
// 双向链表，任意位置插入/删除 O(1)，不支持随机访问
// ============================================================

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：list 基础操作 ===\n";

    std::list<int> lst{1, 2, 3, 4, 5};

    // 头尾插入
    lst.push_front(0);
    lst.push_back(6);
    std::cout << "after push_front(0) push_back(6): ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";

    // 头尾删除
    lst.pop_front();
    lst.pop_back();
    std::cout << "after pop_front pop_back: ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";

    // 访问头尾（无随机访问）
    std::cout << "front=" << lst.front() << " back=" << lst.back() << "\n";

    // insert：在迭代器位置前插入
    auto it = lst.begin();
    std::advance(it, 2);
    lst.insert(it, 99);
    std::cout << "insert(99) at pos 2: ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";

    // erase：删除迭代器位置元素
    it = lst.begin();
    std::advance(it, 2);
    lst.erase(it);
    std::cout << "erase pos 2: ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 中级：list 专属操作 ====================
void list_specific_demo() {
    std::cout << "\n=== 中级：list 专属操作 ===\n";

    // splice：O(1) 移动元素（不拷贝）
    std::list<int> a{1, 2, 3};
    std::list<int> b{4, 5, 6};
    auto it = a.end();
    a.splice(it, b);  // 将 b 的全部元素移到 a 末尾
    std::cout << "splice b into a: ";
    for (int x : a) std::cout << x << " ";
    std::cout << " | b.size=" << b.size() << "\n";

    // remove / remove_if
    std::list<int> lst{1, 2, 3, 2, 4, 2, 5};
    lst.remove(2);
    std::cout << "remove(2): ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";

    lst.remove_if([](int x){ return x > 3; });
    std::cout << "remove_if(>3): ";
    for (int x : lst) std::cout << x << " ";
    std::cout << "\n";

    // unique：去除连续重复（先排序再 unique）
    std::list<int> lst2{3, 1, 2, 2, 3, 3, 1};
    lst2.sort();
    lst2.unique();
    std::cout << "sort+unique: ";
    for (int x : lst2) std::cout << x << " ";
    std::cout << "\n";

    // reverse
    lst2.reverse();
    std::cout << "reverse: ";
    for (int x : lst2) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 高级：归并排序（list::sort）====================
void sort_merge_demo() {
    std::cout << "\n=== 高级：list sort 与 merge ===\n";

    std::list<int> a{5, 1, 3};
    std::list<int> b{4, 2, 6};

    a.sort();  // list 专属 sort（归并排序，稳定）
    b.sort();

    // merge：合并两个有序链表，b 变空
    a.merge(b);
    std::cout << "merge sorted lists: ";
    for (int x : a) std::cout << x << " ";
    std::cout << " | b.size=" << b.size() << "\n";
}

// ==================== 高级：用 list 实现 LRU ====================
void lru_demo() {
    std::cout << "\n=== 高级：list 实现访问记录（LRU 顺序）===\n";

    // 最近访问的移到头部，尾部是最久未访问
    std::list<std::string> lru;
    auto access = [&](const std::string& page) {
        lru.remove(page);       // O(n) 删除旧位置
        lru.push_front(page);   // O(1) 插入头部
        std::cout << "access(" << page << "): ";
        for (auto& s : lru) std::cout << s << " ";
        std::cout << "\n";
    };

    access("home"); access("about"); access("contact");
    access("home");   // 再次访问，移到头部
    access("shop");
}

int main() {
    basic_demo();
    list_specific_demo();
    sort_merge_demo();
    lru_demo();
    return 0;
}
