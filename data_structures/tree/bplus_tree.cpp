#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <sstream>

// ============================================================
// B+ 树实现（阶数 Order = 最大子节点数）
// 内部节点：只存 key，用于路由
// 叶子节点：存 key+value，双向链表串联，支持范围查询
// 插入：节点满时分裂，向上传递中间 key
// 删除：节点不足时合并或从兄弟借用
// 所有数据都在叶子层，内部节点只是索引
// ============================================================

template<typename K, typename V, int ORDER = 4>
class BPlusTree {
    static_assert(ORDER >= 3, "ORDER must be >= 3");

    // 叶子节点最多 ORDER-1 个键值对
    // 内部节点最多 ORDER-1 个键，ORDER 个子指针
    static constexpr int MAX_KEYS  = ORDER - 1;
    static constexpr int MIN_KEYS  = (ORDER - 1) / 2;  // 非根节点最少键数

    struct Node {
        bool            is_leaf;
        std::vector<K>  keys;
        Node*           parent;

        Node(bool leaf) : is_leaf(leaf), parent(nullptr) {}
        virtual ~Node() = default;
    };

    struct InternalNode : Node {
        std::vector<Node*> children;
        InternalNode() : Node(false) {}
        ~InternalNode() {
            for (auto* c : children) delete c;
        }
    };

    struct LeafNode : Node {
        std::vector<V>  vals;
        LeafNode*       prev;
        LeafNode*       next;
        LeafNode() : Node(true), prev(nullptr), next(nullptr) {}
    };

public:
    BPlusTree() : root_(new LeafNode()), size_(0) {}
    ~BPlusTree() { delete root_; }

    // 插入键值对，O(log n)
    void insert(const K& key, const V& val) {
        auto* leaf = find_leaf(key);
        insert_in_leaf(leaf, key, val);
        ++size_;

        if ((int)leaf->keys.size() > MAX_KEYS)
            split_leaf(leaf);
    }

    // 查找，O(log n)
    V* find(const K& key) {
        auto* leaf = find_leaf(key);
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        if (it == leaf->keys.end() || *it != key) return nullptr;
        return &leaf->vals[it - leaf->keys.begin()];
    }

    // 删除，O(log n)
    bool remove(const K& key) {
        auto* leaf = find_leaf(key);
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        if (it == leaf->keys.end() || *it != key) return false;

        int idx = it - leaf->keys.begin();
        leaf->keys.erase(leaf->keys.begin() + idx);
        leaf->vals.erase(leaf->vals.begin() + idx);
        --size_;

        if (leaf == root_) return true;  // 根叶子不检查下溢

        // 更新父节点中对应 key
        update_parent_key(leaf);

        if ((int)leaf->keys.size() < MIN_KEYS)
            fix_leaf_underflow(leaf);

        return true;
    }

    // 范围查询 [lo, hi]，O(log n + k)
    std::vector<std::pair<K,V>> range(const K& lo, const K& hi) const {
        std::vector<std::pair<K,V>> result;
        auto* leaf = find_leaf_const(lo);
        while (leaf) {
            for (int i = 0; i < (int)leaf->keys.size(); ++i) {
                if (leaf->keys[i] > hi) return result;
                if (leaf->keys[i] >= lo)
                    result.push_back({leaf->keys[i], leaf->vals[i]});
            }
            leaf = leaf->next;
        }
        return result;
    }

    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }

    // 打印所有叶子节点（链表顺序）
    void print_leaves() const {
        std::cout << "leaves: ";
        auto* leaf = first_leaf();
        while (leaf) {
            std::cout << "[";
            for (int i = 0; i < (int)leaf->keys.size(); ++i) {
                if (i) std::cout << ",";
                std::cout << leaf->keys[i] << ":" << leaf->vals[i];
            }
            std::cout << "] ";
            leaf = leaf->next;
        }
        std::cout << "\n";
    }

    // 打印树结构
    void print_tree() const {
        print_node(root_, 0);
    }

private:
    Node*  root_;
    size_t size_;

    LeafNode* find_leaf(const K& key) {
        Node* cur = root_;
        while (!cur->is_leaf) {
            auto* node = static_cast<InternalNode*>(cur);
            auto it = std::upper_bound(node->keys.begin(), node->keys.end(), key);
            int i = (int)std::distance(node->keys.begin(), it);
            cur = node->children[i];
        }
        return static_cast<LeafNode*>(cur);
    }

    LeafNode* find_leaf_const(const K& key) const {
        Node* cur = root_;
        while (!cur->is_leaf) {
            auto* node = static_cast<InternalNode*>(cur);
            auto it = std::upper_bound(node->keys.begin(), node->keys.end(), key);
            int i = (int)std::distance(node->keys.begin(), it);
            cur = node->children[i];
        }
        return static_cast<LeafNode*>(cur);
    }

    LeafNode* first_leaf() const {
        Node* cur = root_;
        while (!cur->is_leaf)
            cur = static_cast<InternalNode*>(cur)->children[0];
        return static_cast<LeafNode*>(cur);
    }

    void insert_in_leaf(LeafNode* leaf, const K& key, const V& val) {
        auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
        int idx = it - leaf->keys.begin();
        leaf->keys.insert(it, key);
        leaf->vals.insert(leaf->vals.begin() + idx, val);
    }

    // 叶子节点分裂
    void split_leaf(LeafNode* leaf) {
        int mid = (int)leaf->keys.size() / 2;

        auto* new_leaf = new LeafNode();
        new_leaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
        new_leaf->vals.assign(leaf->vals.begin() + mid, leaf->vals.end());
        leaf->keys.erase(leaf->keys.begin() + mid, leaf->keys.end());
        leaf->vals.erase(leaf->vals.begin() + mid, leaf->vals.end());

        // 维护叶子链表
        new_leaf->next = leaf->next;
        new_leaf->prev = leaf;
        if (leaf->next) leaf->next->prev = new_leaf;
        leaf->next = new_leaf;

        K push_up = new_leaf->keys[0];  // 叶子分裂：上推新叶子第一个 key
        insert_in_parent(leaf, push_up, new_leaf);
    }

    // 内部节点分裂
    void split_internal(InternalNode* node) {
        int mid = (int)node->keys.size() / 2;
        K push_up = node->keys[mid];

        auto* new_node = new InternalNode();
        new_node->keys.assign(node->keys.begin() + mid + 1, node->keys.end());
        new_node->children.assign(node->children.begin() + mid + 1, node->children.end());
        for (auto* c : new_node->children) c->parent = new_node;

        node->keys.erase(node->keys.begin() + mid, node->keys.end());
        node->children.erase(node->children.begin() + mid + 1, node->children.end());

        insert_in_parent(node, push_up, new_node);
    }

    void insert_in_parent(Node* left, const K& key, Node* right) {
        if (left == root_) {
            auto* new_root = new InternalNode();
            new_root->keys.push_back(key);
            new_root->children.push_back(left);
            new_root->children.push_back(right);
            left->parent  = new_root;
            right->parent = new_root;
            root_ = new_root;
            return;
        }

        auto* parent = static_cast<InternalNode*>(left->parent);
        // 找到 left 在父节点中的位置
        auto it = std::find(parent->children.begin(), parent->children.end(), left);
        int idx = it - parent->children.begin();

        parent->keys.insert(parent->keys.begin() + idx, key);
        parent->children.insert(parent->children.begin() + idx + 1, right);
        right->parent = parent;

        if ((int)parent->keys.size() > MAX_KEYS)
            split_internal(parent);
    }

    // 删除后更新父节点中对应的分隔 key
    void update_parent_key(LeafNode* leaf) {
        if (!leaf->parent || leaf->keys.empty()) return;
        auto* parent = static_cast<InternalNode*>(leaf->parent);
        auto it = std::find(parent->children.begin(), parent->children.end(), leaf);
        int idx = it - parent->children.begin();
        if (idx > 0)
            parent->keys[idx - 1] = leaf->keys[0];
    }

    // 修复叶子节点下溢
    void fix_leaf_underflow(LeafNode* leaf) {
        auto* parent = static_cast<InternalNode*>(leaf->parent);
        auto it = std::find(parent->children.begin(), parent->children.end(), leaf);
        int idx = it - parent->children.begin();

        // 尝试从右兄弟借
        if (idx < (int)parent->children.size() - 1) {
            auto* right = static_cast<LeafNode*>(parent->children[idx + 1]);
            if ((int)right->keys.size() > MIN_KEYS) {
                leaf->keys.push_back(right->keys.front());
                leaf->vals.push_back(right->vals.front());
                right->keys.erase(right->keys.begin());
                right->vals.erase(right->vals.begin());
                parent->keys[idx] = right->keys.front();
                return;
            }
        }

        // 尝试从左兄弟借
        if (idx > 0) {
            auto* left = static_cast<LeafNode*>(parent->children[idx - 1]);
            if ((int)left->keys.size() > MIN_KEYS) {
                leaf->keys.insert(leaf->keys.begin(), left->keys.back());
                leaf->vals.insert(leaf->vals.begin(), left->vals.back());
                left->keys.pop_back();
                left->vals.pop_back();
                parent->keys[idx - 1] = leaf->keys.front();
                return;
            }
        }

        // 合并：优先与右兄弟合并
        if (idx < (int)parent->children.size() - 1) {
            auto* right = static_cast<LeafNode*>(parent->children[idx + 1]);
            merge_leaves(leaf, right, idx);
        } else {
            auto* left = static_cast<LeafNode*>(parent->children[idx - 1]);
            merge_leaves(left, leaf, idx - 1);
        }
    }

    void merge_leaves(LeafNode* left, LeafNode* right, int sep_idx) {
        // 将 right 合并入 left
        left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
        left->vals.insert(left->vals.end(), right->vals.begin(), right->vals.end());
        left->next = right->next;
        if (right->next) right->next->prev = left;

        auto* parent = static_cast<InternalNode*>(left->parent);
        parent->keys.erase(parent->keys.begin() + sep_idx);
        parent->children.erase(parent->children.begin() + sep_idx + 1);
        right->next = right->prev = nullptr;  // 防止析构时误删
        // 手动清空 children 避免 delete 时递归
        delete right;

        if (parent == root_ && parent->keys.empty()) {
            root_ = left;
            left->parent = nullptr;
            static_cast<InternalNode*>(parent)->children.clear();
            delete parent;
        } else if (parent != root_ && (int)parent->keys.size() < MIN_KEYS) {
            fix_internal_underflow(parent);
        }
    }

    void fix_internal_underflow(InternalNode* node) {
        auto* par = static_cast<InternalNode*>(node->parent);
        auto it = std::find(par->children.begin(), par->children.end(), node);
        int idx = it - par->children.begin();

        // 从右兄弟借
        if (idx < (int)par->children.size() - 1) {
            auto* right = static_cast<InternalNode*>(par->children[idx + 1]);
            if ((int)right->keys.size() > MIN_KEYS) {
                node->keys.push_back(par->keys[idx]);
                par->keys[idx] = right->keys.front();
                node->children.push_back(right->children.front());
                node->children.back()->parent = node;
                right->keys.erase(right->keys.begin());
                right->children.erase(right->children.begin());
                return;
            }
        }

        // 从左兄弟借
        if (idx > 0) {
            auto* left = static_cast<InternalNode*>(par->children[idx - 1]);
            if ((int)left->keys.size() > MIN_KEYS) {
                node->keys.insert(node->keys.begin(), par->keys[idx - 1]);
                par->keys[idx - 1] = left->keys.back();
                node->children.insert(node->children.begin(), left->children.back());
                node->children.front()->parent = node;
                left->keys.pop_back();
                left->children.pop_back();
                return;
            }
        }

        // 合并
        if (idx < (int)par->children.size() - 1) {
            auto* right = static_cast<InternalNode*>(par->children[idx + 1]);
            merge_internal(node, right, idx);
        } else {
            auto* left = static_cast<InternalNode*>(par->children[idx - 1]);
            merge_internal(left, node, idx - 1);
        }
    }

    void merge_internal(InternalNode* left, InternalNode* right, int sep_idx) {
        auto* par = static_cast<InternalNode*>(left->parent);
        left->keys.push_back(par->keys[sep_idx]);
        left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
        for (auto* c : right->children) { c->parent = left; left->children.push_back(c); }
        right->children.clear();  // 防止析构时递归 delete

        par->keys.erase(par->keys.begin() + sep_idx);
        par->children.erase(par->children.begin() + sep_idx + 1);
        delete right;

        if (par == root_ && par->keys.empty()) {
            root_ = left;
            left->parent = nullptr;
            par->children.clear();
            delete par;
        } else if (par != root_ && (int)par->keys.size() < MIN_KEYS) {
            fix_internal_underflow(par);
        }
    }

    void print_node(Node* node, int depth) const {
        std::string indent(depth * 4, ' ');
        if (node->is_leaf) {
            auto* leaf = static_cast<LeafNode*>(node);
            std::cout << indent << "Leaf[";
            for (int i = 0; i < (int)leaf->keys.size(); ++i) {
                if (i) std::cout << ",";
                std::cout << leaf->keys[i];
            }
            std::cout << "]\n";
        } else {
            auto* inode = static_cast<InternalNode*>(node);
            std::cout << indent << "Internal[";
            for (int i = 0; i < (int)inode->keys.size(); ++i) {
                if (i) std::cout << ",";
                std::cout << inode->keys[i];
            }
            std::cout << "]\n";
            for (auto* c : inode->children)
                print_node(c, depth + 1);
        }
    }
};

// ==================== 入门：基础插入与查找 ====================
void basic_demo() {
    std::cout << "=== 入门：B+ 树基础操作（ORDER=4）===\n";

    BPlusTree<int, std::string, 4> tree;

    // 插入
    for (auto [k, v] : std::vector<std::pair<int,std::string>>{
        {10,"ten"},{20,"twenty"},{5,"five"},{15,"fifteen"},
        {25,"twenty-five"},{30,"thirty"},{35,"thirty-five"}
    }) {
        tree.insert(k, v);
    }

    std::cout << "tree structure:\n";
    tree.print_tree();
    tree.print_leaves();

    // 查找
    if (auto* v = tree.find(15)) std::cout << "find(15)=" << *v << "\n";
    if (auto* v = tree.find(20)) std::cout << "find(20)=" << *v << "\n";
    std::cout << "find(99)=" << (tree.find(99) ? "found" : "not found") << "\n";
    std::cout << "size=" << tree.size() << "\n";
}

// ==================== 中级：范围查询 ====================
void range_demo() {
    std::cout << "\n=== 中级：范围查询 ===\n";

    BPlusTree<int, int, 4> tree;
    for (int i = 1; i <= 20; ++i) tree.insert(i, i * 10);

    std::cout << "range [5, 10]: ";
    for (auto [k, v] : tree.range(5, 10))
        std::cout << k << ":" << v << " ";
    std::cout << "\n";

    std::cout << "range [15, 20]: ";
    for (auto [k, v] : tree.range(15, 20))
        std::cout << k << ":" << v << " ";
    std::cout << "\n";
}

// ==================== 中级：删除操作 ====================
void delete_demo() {
    std::cout << "\n=== 中级：删除操作 ===\n";

    BPlusTree<int, int, 4> tree;
    for (int i : {10, 20, 30, 40, 50, 60, 70, 80})
        tree.insert(i, i);

    std::cout << "before delete:\n";
    tree.print_leaves();

    tree.remove(20);
    std::cout << "after remove(20):\n";
    tree.print_leaves();

    tree.remove(50);
    std::cout << "after remove(50):\n";
    tree.print_leaves();

    tree.remove(10);
    std::cout << "after remove(10):\n";
    tree.print_leaves();

    std::cout << "size=" << tree.size() << "\n";
}

// ==================== 高级：大量数据插入与遍历 ====================
void large_demo() {
    std::cout << "\n=== 高级：大量数据（ORDER=6）===\n";

    BPlusTree<int, int, 6> tree;
    const int N = 100;

    // 乱序插入
    std::vector<int> keys(N);
    std::iota(keys.begin(), keys.end(), 1);
    // 简单打乱
    for (int i = N - 1; i > 0; --i) std::swap(keys[i], keys[i % 7]);

    for (int k : keys) tree.insert(k, k * k);

    std::cout << "inserted " << N << " elements, size=" << tree.size() << "\n";

    // 验证所有键都能找到
    bool all_found = true;
    for (int i = 1; i <= N; ++i)
        if (!tree.find(i)) { all_found = false; break; }
    std::cout << "all keys found: " << std::boolalpha << all_found << "\n";

    // 范围查询验证有序
    auto res = tree.range(1, N);
    bool sorted = true;
    for (int i = 1; i < (int)res.size(); ++i)
        if (res[i].first <= res[i-1].first) { sorted = false; break; }
    std::cout << "range query ordered: " << sorted << "\n";
    std::cout << "range(1," << N << ") count=" << res.size() << "\n";
}

// ==================== 高级：数据库索引模拟 ====================
void db_index_demo() {
    std::cout << "\n=== 高级：数据库索引模拟 ===\n";

    // 模拟用 B+ 树建立索引：age -> name
    BPlusTree<int, std::string, 4> idx;
    idx.insert(25, "Alice");
    idx.insert(30, "Bob");
    idx.insert(22, "Carol");
    idx.insert(28, "Dave");
    idx.insert(35, "Eve");
    idx.insert(27, "Frank");

    // 精确查找
    if (auto* v = idx.find(28)) std::cout << "age=28: " << *v << "\n";

    // 范围查询：查找 25-30 岁的人
    std::cout << "age 25~30: ";
    for (auto [age, name] : idx.range(25, 30))
        std::cout << name << "(" << age << ") ";
    std::cout << "\n";

    // 删除后范围查询
    idx.remove(28);
    std::cout << "after remove Dave(28), age 25~30: ";
    for (auto [age, name] : idx.range(25, 30))
        std::cout << name << "(" << age << ") ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    range_demo();
    delete_demo();
    large_demo();
    db_index_demo();
    return 0;
}
