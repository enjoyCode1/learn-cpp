#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <string>

// ============================================================
// 二叉树实现
// 支持：前/中/后序遍历（递归+迭代），层序遍历，树高，节点数
// ============================================================

template<typename T>
struct BNode {
    T      val;
    BNode* left;
    BNode* right;
    BNode(T v) : val(v), left(nullptr), right(nullptr) {}
};

template<typename T>
class BinaryTree {
public:
    BinaryTree() : root_(nullptr) {}
    ~BinaryTree() { destroy(root_); }

    // 手动设置根节点（方便测试）
    BNode<T>*& root() { return root_; }

    // ---- 递归遍历 ----
    void preorder(BNode<T>* node, std::vector<T>& out) const {
        if (!node) return;
        out.push_back(node->val);       // 根
        preorder(node->left,  out);     // 左
        preorder(node->right, out);     // 右
    }

    void inorder(BNode<T>* node, std::vector<T>& out) const {
        if (!node) return;
        inorder(node->left,  out);
        out.push_back(node->val);
        inorder(node->right, out);
    }

    void postorder(BNode<T>* node, std::vector<T>& out) const {
        if (!node) return;
        postorder(node->left,  out);
        postorder(node->right, out);
        out.push_back(node->val);
    }

    // ---- 迭代前序（用栈模拟递归）----
    std::vector<T> preorder_iter() const {
        std::vector<T> out;
        if (!root_) return out;
        std::stack<BNode<T>*> st;
        st.push(root_);
        while (!st.empty()) {
            auto* node = st.top(); st.pop();
            out.push_back(node->val);
            // 先压右子树，再压左子树（保证左先出）
            if (node->right) st.push(node->right);
            if (node->left)  st.push(node->left);
        }
        return out;
    }

    // ---- 迭代中序 ----
    std::vector<T> inorder_iter() const {
        std::vector<T> out;
        std::stack<BNode<T>*> st;
        auto* cur = root_;
        while (cur || !st.empty()) {
            // 一路向左压栈
            while (cur) { st.push(cur); cur = cur->left; }
            cur = st.top(); st.pop();
            out.push_back(cur->val);
            cur = cur->right;  // 转向右子树
        }
        return out;
    }

    // ---- 层序遍历（BFS）----
    std::vector<std::vector<T>> level_order() const {
        std::vector<std::vector<T>> res;
        if (!root_) return res;
        std::queue<BNode<T>*> q;
        q.push(root_);
        while (!q.empty()) {
            int cnt = (int)q.size();
            std::vector<T> level;
            for (int i = 0; i < cnt; ++i) {
                auto* node = q.front(); q.pop();
                level.push_back(node->val);
                if (node->left)  q.push(node->left);
                if (node->right) q.push(node->right);
            }
            res.push_back(level);
        }
        return res;
    }

    // 树高（递归）
    int height(BNode<T>* node) const {
        if (!node) return 0;
        return 1 + std::max(height(node->left), height(node->right));
    }

    // 节点数
    int count(BNode<T>* node) const {
        if (!node) return 0;
        return 1 + count(node->left) + count(node->right);
    }

    // 判断是否为完全二叉树
    bool is_complete() const {
        if (!root_) return true;
        std::queue<BNode<T>*> q;
        q.push(root_);
        bool found_null = false;
        while (!q.empty()) {
            auto* node = q.front(); q.pop();
            if (!node) {
                found_null = true;
            } else {
                if (found_null) return false;  // null 之后出现非 null
                q.push(node->left);
                q.push(node->right);
            }
        }
        return true;
    }

private:
    BNode<T>* root_;

    void destroy(BNode<T>* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
};

// 构建示例树:
//         1
//       /   \
//      2     3
//     / \   / \
//    4   5 6   7
BNode<int>* build_full_tree() {
    auto* r  = new BNode<int>(1);
    r->left  = new BNode<int>(2);
    r->right = new BNode<int>(3);
    r->left->left   = new BNode<int>(4);
    r->left->right  = new BNode<int>(5);
    r->right->left  = new BNode<int>(6);
    r->right->right = new BNode<int>(7);
    return r;
}

void print_vec(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ==================== 入门：遍历 ====================
void traversal_demo() {
    std::cout << "=== 入门：二叉树遍历 ===\n";

    BinaryTree<int> tree;
    tree.root() = build_full_tree();

    std::vector<int> pre, in, post;
    tree.preorder(tree.root(),  pre);
    tree.inorder(tree.root(),   in);
    tree.postorder(tree.root(), post);
    print_vec("前序(递归)", pre);
    print_vec("中序(递归)", in);
    print_vec("后序(递归)", post);

    print_vec("前序(迭代)", tree.preorder_iter());
    print_vec("中序(迭代)", tree.inorder_iter());
}

// ==================== 中级：层序 + 树高 ====================
void level_demo() {
    std::cout << "\n=== 中级：层序遍历 + 树高 ===\n";

    BinaryTree<int> tree;
    tree.root() = build_full_tree();

    auto levels = tree.level_order();
    for (int i = 0; i < (int)levels.size(); ++i) {
        std::cout << "level " << i << ": ";
        for (int v : levels[i]) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "height = " << tree.height(tree.root()) << "\n";
    std::cout << "count  = " << tree.count(tree.root())  << "\n";
    std::cout << "complete = " << std::boolalpha << tree.is_complete() << "\n";
}

// ==================== 高级：序列化/反序列化 ====================
// 用层序将树序列化为字符串，-1 表示 null 节点
std::string serialize(BNode<int>* root) {
    if (!root) return "";
    std::string res;
    std::queue<BNode<int>*> q;
    q.push(root);
    while (!q.empty()) {
        auto* node = q.front(); q.pop();
        if (node) {
            res += std::to_string(node->val) + ",";
            q.push(node->left);
            q.push(node->right);
        } else {
            res += "#,";
        }
    }
    return res;
}

BNode<int>* deserialize(const std::string& data) {
    if (data.empty()) return nullptr;
    // 解析逗号分隔的 token
    std::vector<std::string> tokens;
    std::string tok;
    for (char c : data) {
        if (c == ',') { tokens.push_back(tok); tok.clear(); }
        else tok += c;
    }

    if (tokens.empty() || tokens[0] == "#") return nullptr;
    auto* root = new BNode<int>(std::stoi(tokens[0]));
    std::queue<BNode<int>*> q;
    q.push(root);
    size_t i = 1;
    while (!q.empty() && i < tokens.size()) {
        auto* node = q.front(); q.pop();
        if (i < tokens.size() && tokens[i] != "#") {
            node->left = new BNode<int>(std::stoi(tokens[i]));
            q.push(node->left);
        }
        ++i;
        if (i < tokens.size() && tokens[i] != "#") {
            node->right = new BNode<int>(std::stoi(tokens[i]));
            q.push(node->right);
        }
        ++i;
    }
    return root;
}

void serialize_demo() {
    std::cout << "\n=== 高级：序列化/反序列化 ===\n";

    BNode<int>* root = build_full_tree();
    std::string s = serialize(root);
    std::cout << "serialized: " << s << "\n";

    BNode<int>* root2 = deserialize(s);
    std::string s2 = serialize(root2);
    std::cout << "after deserialize + re-serialize: " << s2 << "\n";
    std::cout << "match: " << std::boolalpha << (s == s2) << "\n";
}

int main() {
    traversal_demo();
    level_demo();
    serialize_demo();
    return 0;
}
