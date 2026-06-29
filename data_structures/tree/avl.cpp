#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

// ============================================================
// AVL 树实现
// 自平衡 BST：任意节点左右子树高度差 ≤ 1
// 插入/删除/查找均为 O(log n)，通过旋转维护平衡
// ============================================================

template<typename T>
struct AVLNode {
    T        val;
    AVLNode* left;
    AVLNode* right;
    int      height;  // 节点高度，叶子为 1

    AVLNode(T v) : val(v), left(nullptr), right(nullptr), height(1) {}
};

template<typename T>
class AVLTree {
public:
    AVLTree() : root_(nullptr) {}
    ~AVLTree() { destroy(root_); }

    void insert(const T& val) { root_ = insert(root_, val); }
    void remove(const T& val) { root_ = remove(root_, val); }
    bool contains(const T& val) const { return find(root_, val); }

    std::vector<T> inorder() const {
        std::vector<T> res;
        inorder(root_, res);
        return res;
    }

    int height() const { return height(root_); }

    void print_tree() const { print_tree(root_, "", false); }

private:
    AVLNode<T>* root_;

    int height(AVLNode<T>* n) const { return n ? n->height : 0; }

    // 平衡因子：左高 - 右高
    int balance_factor(AVLNode<T>* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }

    void update_height(AVLNode<T>* n) {
        n->height = 1 + std::max(height(n->left), height(n->right));
    }

    // LL 右旋：左子树过高
    //      y                x
    //     / \              / \
    //    x   T4   =>     T1   y
    //   / \                  / \
    //  T1  T2               T2  T4
    AVLNode<T>* rotate_right(AVLNode<T>* y) {
        auto* x  = y->left;
        auto* T2 = x->right;
        x->right = y;
        y->left  = T2;
        update_height(y);
        update_height(x);
        return x;
    }

    // RR 左旋：右子树过高
    AVLNode<T>* rotate_left(AVLNode<T>* x) {
        auto* y  = x->right;
        auto* T2 = y->left;
        y->left  = x;
        x->right = T2;
        update_height(x);
        update_height(y);
        return y;
    }

    // 重新平衡
    AVLNode<T>* rebalance(AVLNode<T>* node) {
        update_height(node);
        int bf = balance_factor(node);

        // LL：右旋
        if (bf > 1 && balance_factor(node->left) >= 0)
            return rotate_right(node);

        // LR：先左旋左子树，再右旋
        if (bf > 1 && balance_factor(node->left) < 0) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }

        // RR：左旋
        if (bf < -1 && balance_factor(node->right) <= 0)
            return rotate_left(node);

        // RL：先右旋右子树，再左旋
        if (bf < -1 && balance_factor(node->right) > 0) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }

        return node;
    }

    AVLNode<T>* insert(AVLNode<T>* node, const T& val) {
        if (!node) return new AVLNode<T>(val);
        if      (val < node->val) node->left  = insert(node->left,  val);
        else if (val > node->val) node->right = insert(node->right, val);
        else return node;  // 忽略重复
        return rebalance(node);
    }

    AVLNode<T>* min_node(AVLNode<T>* n) const {
        while (n->left) n = n->left;
        return n;
    }

    AVLNode<T>* remove(AVLNode<T>* node, const T& val) {
        if (!node) return nullptr;
        if      (val < node->val) node->left  = remove(node->left,  val);
        else if (val > node->val) node->right = remove(node->right, val);
        else {
            if (!node->left || !node->right) {
                auto* child = node->left ? node->left : node->right;
                delete node;
                return child;
            }
            auto* succ  = min_node(node->right);
            node->val   = succ->val;
            node->right = remove(node->right, succ->val);
        }
        return rebalance(node);
    }

    bool find(AVLNode<T>* node, const T& val) const {
        if (!node) return false;
        if (val == node->val) return true;
        return (val < node->val) ? find(node->left, val) : find(node->right, val);
    }

    void inorder(AVLNode<T>* node, std::vector<T>& res) const {
        if (!node) return;
        inorder(node->left, res);
        res.push_back(node->val);
        inorder(node->right, res);
    }

    void print_tree(AVLNode<T>* node, const std::string& prefix, bool is_left) const {
        if (!node) return;
        std::cout << prefix << (is_left ? "├──" : "└──")
                  << node->val << "(h=" << node->height << ")\n";
        print_tree(node->left,  prefix + (is_left ? "│   " : "    "), true);
        print_tree(node->right, prefix + (is_left ? "│   " : "    "), false);
    }

    void destroy(AVLNode<T>* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
};

// ==================== 入门：插入与自动平衡 ====================
void basic_demo() {
    std::cout << "=== 入门：AVL 插入自动平衡 ===\n";

    AVLTree<int> avl;
    // 顺序插入会使普通 BST 退化为链表，AVL 会旋转保持平衡
    for (int v : {1, 2, 3, 4, 5, 6, 7}) avl.insert(v);

    std::cout << "tree structure:\n";
    avl.print_tree();

    std::cout << "inorder: ";
    for (int v : avl.inorder()) std::cout << v << " ";
    std::cout << "\nheight = " << avl.height() << " (should be 3)\n";
}

// ==================== 中级：四种旋转情况演示 ====================
void rotation_demo() {
    std::cout << "\n=== 中级：四种旋转情况 ===\n";

    // LL：连续插入左侧，触发右旋
    {
        AVLTree<int> t;
        t.insert(3); t.insert(2); t.insert(1);
        std::cout << "LL(insert 3,2,1): "; t.print_tree();
    }
    // RR：连续插入右侧，触发左旋
    {
        AVLTree<int> t;
        t.insert(1); t.insert(2); t.insert(3);
        std::cout << "RR(insert 1,2,3): "; t.print_tree();
    }
    // LR：左子树右侧插入，先左旋再右旋
    {
        AVLTree<int> t;
        t.insert(3); t.insert(1); t.insert(2);
        std::cout << "LR(insert 3,1,2): "; t.print_tree();
    }
    // RL：右子树左侧插入，先右旋再左旋
    {
        AVLTree<int> t;
        t.insert(1); t.insert(3); t.insert(2);
        std::cout << "RL(insert 1,3,2): "; t.print_tree();
    }
}

// ==================== 高级：删除后重新平衡 ====================
void delete_demo() {
    std::cout << "\n=== 高级：删除后重新平衡 ===\n";

    AVLTree<int> avl;
    for (int v : {10, 5, 15, 3, 7, 12, 20, 1, 4}) avl.insert(v);

    std::cout << "before delete:\n";
    avl.print_tree();
    std::cout << "height=" << avl.height() << "\n";

    avl.remove(15);
    avl.remove(20);
    std::cout << "after remove(15) remove(20):\n";
    avl.print_tree();
    std::cout << "height=" << avl.height() << "\n";

    std::cout << "inorder: ";
    for (int v : avl.inorder()) std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    rotation_demo();
    delete_demo();
    return 0;
}
