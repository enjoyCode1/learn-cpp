#include <iostream>
#include <stdexcept>
#include <initializer_list>

// ============================================================
// 双向链表实现
// 每个节点保存前驱和后继指针，支持 O(1) 首尾插入/删除
// ============================================================

template<typename T>
class List {
    struct Node {
        T     val;
        Node* prev;
        Node* next;
        Node(const T& v, Node* p = nullptr, Node* n = nullptr)
            : val(v), prev(p), next(n) {}
    };

public:
    List() : head_(nullptr), tail_(nullptr), size_(0) {}

    List(std::initializer_list<T> il) : List() {
        for (auto& v : il) push_back(v);
    }

    ~List() { clear(); }

    // 禁止拷贝（简化实现）
    List(const List&) = delete;
    List& operator=(const List&) = delete;

    // 头部插入 O(1)
    void push_front(const T& val) {
        Node* node = new Node(val, nullptr, head_);
        if (head_) head_->prev = node;
        else       tail_ = node;
        head_ = node;
        ++size_;
    }

    // 尾部插入 O(1)
    void push_back(const T& val) {
        Node* node = new Node(val, tail_, nullptr);
        if (tail_) tail_->next = node;
        else       head_ = node;
        tail_ = node;
        ++size_;
    }

    // 头部删除 O(1)
    void pop_front() {
        if (!head_) throw std::underflow_error("List::pop_front on empty");
        Node* old = head_;
        head_ = head_->next;
        if (head_) head_->prev = nullptr;
        else       tail_ = nullptr;
        delete old;
        --size_;
    }

    // 尾部删除 O(1)
    void pop_back() {
        if (!tail_) throw std::underflow_error("List::pop_back on empty");
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) tail_->next = nullptr;
        else       head_ = nullptr;
        delete old;
        --size_;
    }

    T& front() {
        if (!head_) throw std::runtime_error("List::front on empty");
        return head_->val;
    }
    T& back() {
        if (!tail_) throw std::runtime_error("List::back on empty");
        return tail_->val;
    }

    // 在第 pos 个节点之后插入，O(n)
    void insert_after(size_t pos, const T& val) {
        Node* cur = node_at(pos);
        Node* node = new Node(val, cur, cur->next);
        if (cur->next) cur->next->prev = node;
        else           tail_ = node;
        cur->next = node;
        ++size_;
    }

    // 删除第 pos 个节点，O(n)
    void erase(size_t pos) {
        Node* cur = node_at(pos);
        if (cur->prev) cur->prev->next = cur->next;
        else           head_ = cur->next;
        if (cur->next) cur->next->prev = cur->prev;
        else           tail_ = cur->prev;
        delete cur;
        --size_;
    }

    // 反转链表，O(n)
    void reverse() {
        Node* cur = head_;
        std::swap(head_, tail_);
        while (cur) {
            std::swap(cur->prev, cur->next);
            cur = cur->prev;   // 反转后 prev 是原来的 next
        }
    }

    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }

    void clear() {
        while (head_) {
            Node* next = head_->next;
            delete head_;
            head_ = next;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    void print() const {
        std::cout << "head <-> ";
        for (Node* cur = head_; cur; cur = cur->next) {
            std::cout << cur->val;
            if (cur->next) std::cout << " <-> ";
        }
        std::cout << " <-> tail  (size=" << size_ << ")\n";
    }

private:
    Node*  head_;
    Node*  tail_;
    size_t size_;

    Node* node_at(size_t pos) {
        if (pos >= size_) throw std::out_of_range("List::node_at out of range");
        Node* cur = head_;
        for (size_t i = 0; i < pos; ++i) cur = cur->next;
        return cur;
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：List 基础操作 ===\n";

    List<int> lst{1, 2, 3, 4, 5};
    lst.print();

    lst.push_front(0);
    lst.push_back(6);
    std::cout << "after push_front(0) push_back(6): "; lst.print();

    lst.pop_front();
    lst.pop_back();
    std::cout << "after pop_front pop_back:          "; lst.print();
}

// ==================== 中级：插入删除与反转 ====================
void insert_erase_demo() {
    std::cout << "\n=== 中级：插入、删除、反转 ===\n";

    List<int> lst{1, 2, 4, 5};
    lst.print();

    // 在下标 1（值为 2）之后插入 3
    lst.insert_after(1, 3);
    std::cout << "insert_after(1, 3): "; lst.print();

    // 删除下标 0（头节点）
    lst.erase(0);
    std::cout << "erase(0):           "; lst.print();

    // 反转
    lst.reverse();
    std::cout << "after reverse:      "; lst.print();
}

// ==================== 高级：用链表实现 LRU 淘汰顺序 ====================
// 最近访问的节点移到头部，尾部是最久未访问的
void lru_order_demo() {
    std::cout << "\n=== 高级：链表模拟 LRU 访问顺序 ===\n";

    // 用链表维护访问顺序，头部最新，尾部最旧
    List<int> order{3, 1, 4, 1, 5};
    std::cout << "initial order: "; order.print();

    // 模拟访问 4：找到并移到头部（先删除再头插）
    // 真实 LRU 用 hashmap + 双向链表，这里仅演示链表操作
    int access = 4;
    // 找到位置（线性扫描）
    size_t pos = 0;
    for (size_t i = 0; i < order.size(); ++i) {
        // 通过 erase + push_front 模拟移动到头部
        // 这里简化：先删除位置 2（值 4），再头插
        if (i == 2) { pos = i; break; }
    }
    order.erase(pos);
    order.push_front(access);
    std::cout << "after access " << access << ": "; order.print();
}

int main() {
    basic_demo();
    insert_erase_demo();
    lru_order_demo();
    return 0;
}
