#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

// ============================================================
// 二叉搜索树（BST）实现
// 左子树所有节点 < 根 < 右子树所有节点
// 平均 O(log n) 查找/插入/删除，最坏 O(n)（退化为链表）
// ============================================================

template<typename T>
struct BSTNode {
    T        val;
    BSTNode* left;
    BSTNode* right;
    BSTNode(T v) : val(v), left(nullptr), right(nullptr) {}
};

template<typename T>
class BST {
public:
    BST() : root_(nullptr) {}
    ~BST() { destroy(root_); }

    // 插入，O(h)
    void insert(const T& val) { root_ = insert(root_, val); }

    // 查找，O(h)
    bool contains(const T& val) const { return find(root_, val) != nullptr; }

    // 删除，O(h)
    void remove(const T& val) { root_ = remove(root_, val); }

    // 最小值 / 最大值
    T min_val() const {
        if (!root_) throw std::runtime_error("BST is empty");
        return min_node(root_)->val;
    }
    T max_val() const {
        if (!root_) throw std::runtime_error("BST is empty");
        return max_node(root_)->val;
    }

    // 中序遍历得到有序序列
    std::vector<T> inorder() const {
        std::vector<T> res;
        inorder(root_, res);
        return res;
    }

    // 前驱（小于 val 的最大值）
    bool predecessor(const T& val, T& out) const {
        BSTNode<T>* pred = nullptr;
        BSTNode<T>* cur  = root_;
        while (cur) {
            if (cur->val < val) { pred = cur; cur = cur->right; }
            else                 cur = cur->left;
        }
        if (pred) { out = pred->val; return true; }
        return false;
    }

    // 后继（大于 val 的最小值）
    bool successor(const T& val, T& out) const {
        BSTNode<T>* succ = nullptr;
        BSTNode<T>* cur  = root_;
        while (cur) {
            if (cur->val > val) { succ = cur; cur = cur->left; }
            else                 cur = cur->right;
        }
        if (succ) { out = succ->val; return true; }
        return false;
    }

    // 验证是否是合法 BST
    bool is_valid() const { return is_valid(root_, nullptr, nullptr); }

    void print_tree() const { print_tree(root_, "", false); }

private:
    BSTNode<T>* root_;

    BSTNode<T>* insert(BSTNode<T>* node, const T& val) {
        if (!node) return new BSTNode<T>(val);
        if      (val < node->val) node->left  = insert(node->left,  val);
        else if (val > node->val) node->right = insert(node->right, val);
        // 相等时忽略（不允许重复）
        return node;
    }

    BSTNode<T>* find(BSTNode<T>* node, const T& val) const {
        if (!node || node->val == val) return node;
        return (val < node->val) ? find(node->left, val) : find(node->right, val);
    }

    BSTNode<T>* remove(BSTNode<T>* node, const T& val) {
        if (!node) return nullptr;
        if (val < node->val) {
            node->left  = remove(node->left,  val);
        } else if (val > node->val) {
            node->right = remove(node->right, val);
        } else {
            // 找到目标节点
            if (!node->left) {
                auto* r = node->right; delete node; return r;
            }
            if (!node->right) {
                auto* l = node->left; delete node; return l;
            }
            // 有两个子节点：用右子树最小值替换，再删除右子树最小值
            auto* succ = min_node(node->right);
            node->val   = succ->val;
            node->right = remove(node->right, succ->val);
        }
        return node;
    }

    BSTNode<T>* min_node(BSTNode<T>* node) const {
        while (node->left) node = node->left;
        return node;
    }
    BSTNode<T>* max_node(BSTNode<T>* node) const {
        while (node->right) node = node->right;
        return node;
    }

    void inorder(BSTNode<T>* node, std::vector<T>& res) const {
        if (!node) return;
        inorder(node->left, res);
        res.push_back(node->val);
        inorder(node->right, res);
    }

    bool is_valid(BSTNode<T>* node, const T* lo, const T* hi) const {
        if (!node) return true;
        if (lo && node->val <= *lo) return false;
        if (hi && node->val >= *hi) return false;
        return is_valid(node->left,  lo,          &node->val) &&
               is_valid(node->right, &node->val,  hi);
    }

    void print_tree(BSTNode<T>* node, const std::string& prefix, bool is_left) const {
        if (!node) return;
        std::cout << prefix << (is_left ? "├──" : "└──") << node->val << "\n";
        print_tree(node->left,  prefix + (is_left ? "│   " : "    "), true);
        print_tree(node->right, prefix + (is_left ? "│   " : "    "), false);
    }

    void destroy(BSTNode<T>* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
};

// ==================== 入门：插入与查找 ====================
void basic_demo() {
    std::cout << "=== 入门：BST 插入与查找 ===\n";

    BST<int> bst;
    for (int v : {5, 3, 7, 1, 4, 6, 8, 2}) bst.insert(v);

    bst.print_tree();

    std::cout << "inorder: ";
    for (int v : bst.inorder()) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "contains(4)=" << std::boolalpha << bst.contains(4) << "\n";
    std::cout << "contains(9)=" << bst.contains(9) << "\n";
    std::cout << "min=" << bst.min_val() << " max=" << bst.max_val() << "\n";
}

// ==================== 中级：删除与前驱后继 ====================
void delete_demo() {
    std::cout << "\n=== 中级：删除节点 ===\n";

    BST<int> bst;
    for (int v : {5, 3, 7, 1, 4, 6, 8}) bst.insert(v);

    std::cout << "before: ";
    for (int v : bst.inorder()) std::cout << v << " ";
    std::cout << "\n";

    bst.remove(3);  // 删除有两个子节点的节点
    std::cout << "remove(3): ";
    for (int v : bst.inorder()) std::cout << v << " ";
    std::cout << "\n";

    bst.remove(8);  // 删除叶子节点
    std::cout << "remove(8): ";
    for (int v : bst.inorder()) std::cout << v << " ";
    std::cout << "\n";

    int pred, succ;
    if (bst.predecessor(5, pred)) std::cout << "pred(5)=" << pred << "\n";
    if (bst.successor(5, succ))   std::cout << "succ(5)=" << succ << "\n";
}

// ==================== 高级：BST 转有序双向链表 ====================
// 中序遍历，将节点的 left/right 指针改为 prev/next
BSTNode<int>* bst_to_dll(BSTNode<int>* root) {
    BSTNode<int>* head = nullptr;
    BSTNode<int>* prev = nullptr;

    std::function<void(BSTNode<int>*)> inorder = [&](BSTNode<int>* node) {
        if (!node) return;
        inorder(node->left);
        if (prev) { prev->right = node; node->left = prev; }
        else        head = node;
        prev = node;
        inorder(node->right);
    };

    inorder(root);
    return head;
}

void dll_demo() {
    std::cout << "\n=== 高级：BST 转有序双向链表 ===\n";

    BST<int> bst;
    for (int v : {4, 2, 6, 1, 3, 5, 7}) bst.insert(v);

    // 取出根节点（不走析构，手动管理）
    // 简化演示：直接构建同样的树
    auto* r = new BSTNode<int>(4);
    r->left  = new BSTNode<int>(2); r->right = new BSTNode<int>(6);
    r->left->left  = new BSTNode<int>(1); r->left->right  = new BSTNode<int>(3);
    r->right->left = new BSTNode<int>(5); r->right->right = new BSTNode<int>(7);

    BSTNode<int>* head = bst_to_dll(r);
    std::cout << "doubly linked list: ";
    for (auto* cur = head; cur; cur = cur->right)
        std::cout << cur->val << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    delete_demo();
    dll_demo();
    return 0;
}
