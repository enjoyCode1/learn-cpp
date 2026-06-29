#include <iostream>
#include <vector>
#include <functional>

// ============================================================
// 红黑树实现
// 自平衡 BST，满足以下性质：
// 1. 每个节点是红色或黑色
// 2. 根节点是黑色
// 3. 叶节点（NIL）是黑色
// 4. 红色节点的两个子节点都是黑色（不能有连续红节点）
// 5. 从任意节点到叶节点路径上的黑色节点数相同（黑高相等）
// 插入/删除/查找均为 O(log n)
// ============================================================

enum class Color { RED, BLACK };

template<typename T>
struct RBNode {
    T        val;
    Color    color;
    RBNode*  left;
    RBNode*  right;
    RBNode*  parent;

    RBNode(T v, Color c, RBNode* nil)
        : val(v), color(c), left(nil), right(nil), parent(nil) {}
};

template<typename T>
class RBTree {
public:
    RBTree() {
        // NIL 哨兵节点：黑色，所有叶节点指向它
        nil_ = new RBNode<T>(T{}, Color::BLACK, nullptr);
        nil_->left = nil_->right = nil_->parent = nil_;
        root_ = nil_;
    }

    ~RBTree() {
        destroy(root_);
        delete nil_;
    }

    void insert(const T& val) {
        auto* z = new RBNode<T>(val, Color::RED, nil_);
        bst_insert(z);
        fix_insert(z);
    }

    void remove(const T& val) {
        auto* z = find(root_, val);
        if (z != nil_) rb_delete(z);
    }

    bool contains(const T& val) const { return find(root_, val) != nil_; }

    std::vector<T> inorder() const {
        std::vector<T> res;
        inorder(root_, res);
        return res;
    }

    void print_tree() const { print_tree(root_, "", false); }

private:
    RBNode<T>* root_;
    RBNode<T>* nil_;   // 哨兵

    void bst_insert(RBNode<T>* z) {
        RBNode<T>* y = nil_;
        RBNode<T>* x = root_;
        while (x != nil_) {
            y = x;
            if      (z->val < x->val) x = x->left;
            else if (z->val > x->val) x = x->right;
            else { delete z; return; }  // 忽略重复
        }
        z->parent = y;
        if (y == nil_)       root_    = z;
        else if (z->val < y->val) y->left  = z;
        else                      y->right = z;
    }

    // 插入修复：维护红黑树性质
    void fix_insert(RBNode<T>* z) {
        while (z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                auto* y = z->parent->parent->right;  // 叔父节点
                if (y->color == Color::RED) {
                    // Case 1：叔父红，重新染色
                    z->parent->color          = Color::BLACK;
                    y->color                  = Color::BLACK;
                    z->parent->parent->color  = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Case 2：叔父黑，z 是右孩子，先左旋
                        z = z->parent;
                        rotate_left(z);
                    }
                    // Case 3：叔父黑，z 是左孩子，右旋
                    z->parent->color         = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rotate_right(z->parent->parent);
                }
            } else {
                // 镜像对称
                auto* y = z->parent->parent->left;
                if (y->color == Color::RED) {
                    z->parent->color         = Color::BLACK;
                    y->color                 = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotate_right(z);
                    }
                    z->parent->color         = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rotate_left(z->parent->parent);
                }
            }
        }
        root_->color = Color::BLACK;
    }

    void rotate_left(RBNode<T>* x) {
        auto* y  = x->right;
        x->right = y->left;
        if (y->left != nil_) y->left->parent = x;
        y->parent = x->parent;
        if      (x->parent == nil_)        root_         = y;
        else if (x == x->parent->left)     x->parent->left  = y;
        else                               x->parent->right = y;
        y->left   = x;
        x->parent = y;
    }

    void rotate_right(RBNode<T>* y) {
        auto* x  = y->left;
        y->left  = x->right;
        if (x->right != nil_) x->right->parent = y;
        x->parent = y->parent;
        if      (y->parent == nil_)       root_         = x;
        else if (y == y->parent->right)   y->parent->right = x;
        else                              y->parent->left  = x;
        x->right  = y;
        y->parent = x;
    }

    void transplant(RBNode<T>* u, RBNode<T>* v) {
        if      (u->parent == nil_)        root_         = v;
        else if (u == u->parent->left)     u->parent->left  = v;
        else                               u->parent->right = v;
        v->parent = u->parent;
    }

    RBNode<T>* minimum(RBNode<T>* x) const {
        while (x->left != nil_) x = x->left;
        return x;
    }

    void rb_delete(RBNode<T>* z) {
        auto*  y = z;
        Color  y_orig = y->color;
        RBNode<T>* x;

        if (z->left == nil_) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil_) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_orig = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        delete z;
        if (y_orig == Color::BLACK)
            fix_delete(x);
    }

    void fix_delete(RBNode<T>* x) {
        while (x != root_ && x->color == Color::BLACK) {
            if (x == x->parent->left) {
                auto* w = x->parent->right;
                if (w->color == Color::RED) {
                    w->color         = Color::BLACK;
                    x->parent->color = Color::RED;
                    rotate_left(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Color::BLACK && w->right->color == Color::BLACK) {
                    w->color = Color::RED;
                    x = x->parent;
                } else {
                    if (w->right->color == Color::BLACK) {
                        w->left->color = Color::BLACK;
                        w->color       = Color::RED;
                        rotate_right(w);
                        w = x->parent->right;
                    }
                    w->color         = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->right->color  = Color::BLACK;
                    rotate_left(x->parent);
                    x = root_;
                }
            } else {
                auto* w = x->parent->left;
                if (w->color == Color::RED) {
                    w->color         = Color::BLACK;
                    x->parent->color = Color::RED;
                    rotate_right(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == Color::BLACK && w->left->color == Color::BLACK) {
                    w->color = Color::RED;
                    x = x->parent;
                } else {
                    if (w->left->color == Color::BLACK) {
                        w->right->color = Color::BLACK;
                        w->color        = Color::RED;
                        rotate_left(w);
                        w = x->parent->left;
                    }
                    w->color         = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->left->color   = Color::BLACK;
                    rotate_right(x->parent);
                    x = root_;
                }
            }
        }
        x->color = Color::BLACK;
    }

    RBNode<T>* find(RBNode<T>* node, const T& val) const {
        while (node != nil_) {
            if      (val < node->val) node = node->left;
            else if (val > node->val) node = node->right;
            else return node;
        }
        return nil_;
    }

    void inorder(RBNode<T>* node, std::vector<T>& res) const {
        if (node == nil_) return;
        inorder(node->left, res);
        res.push_back(node->val);
        inorder(node->right, res);
    }

    void print_tree(RBNode<T>* node, const std::string& prefix, bool is_left) const {
        if (node == nil_) return;
        std::cout << prefix << (is_left ? "├──" : "└──")
                  << node->val
                  << (node->color == Color::RED ? "(R)" : "(B)") << "\n";
        print_tree(node->left,  prefix + (is_left ? "│   " : "    "), true);
        print_tree(node->right, prefix + (is_left ? "│   " : "    "), false);
    }

    void destroy(RBNode<T>* node) {
        if (node == nil_) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
};

// ==================== 入门：插入与颜色 ====================
void basic_demo() {
    std::cout << "=== 入门：红黑树插入 ===\n";
    RBTree<int> rbt;
    for (int v : {10, 5, 15, 3, 7, 12, 20}) rbt.insert(v);
    rbt.print_tree();
    std::cout << "inorder: ";
    for (int v : rbt.inorder()) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 中级：顺序插入（最坏情况）====================
void sequential_demo() {
    std::cout << "\n=== 中级：顺序插入（红黑树自动平衡）===\n";
    RBTree<int> rbt;
    for (int i = 1; i <= 10; ++i) rbt.insert(i);
    rbt.print_tree();
    std::cout << "inorder: ";
    for (int v : rbt.inorder()) std::cout << v << " ";
    std::cout << "\n";
}

// ==================== 高级：删除后修复 ====================
void delete_demo() {
    std::cout << "\n=== 高级：删除后修复 ===\n";
    RBTree<int> rbt;
    for (int v : {10, 5, 15, 3, 7, 12, 20, 1, 4, 6, 8}) rbt.insert(v);
    std::cout << "before:\n"; rbt.print_tree();

    rbt.remove(5);
    rbt.remove(15);
    std::cout << "after remove(5) remove(15):\n"; rbt.print_tree();
    std::cout << "inorder: ";
    for (int v : rbt.inorder()) std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    sequential_demo();
    delete_demo();
    return 0;
}
