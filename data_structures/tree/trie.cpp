#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

// ============================================================
// Trie（前缀树）实现
// 用于字符串的高效插入、查找、前缀匹配
// 插入/查找 O(m)，m 为字符串长度
// ============================================================

struct TrieNode {
    std::unordered_map<char, TrieNode*> children;
    bool is_end;    // 标记是否是某个单词的结尾
    int  count;     // 以该节点结尾的单词数量

    TrieNode() : is_end(false), count(0) {}
    ~TrieNode() {
        for (auto& [c, child] : children) delete child;
    }
};

class Trie {
public:
    Trie() : root_(new TrieNode()) {}
    ~Trie() { delete root_; }

    // 插入单词，O(m)
    void insert(const std::string& word) {
        auto* cur = root_;
        for (char c : word) {
            if (!cur->children.count(c))
                cur->children[c] = new TrieNode();
            cur = cur->children[c];
        }
        cur->is_end = true;
        ++cur->count;
    }

    // 精确查找，O(m)
    bool search(const std::string& word) const {
        auto* node = find_node(word);
        return node && node->is_end;
    }

    // 前缀查找：是否存在以 prefix 开头的单词，O(m)
    bool starts_with(const std::string& prefix) const {
        return find_node(prefix) != nullptr;
    }

    // 删除单词
    void remove(const std::string& word) { remove(root_, word, 0); }

    // 获取所有以 prefix 开头的单词
    std::vector<std::string> autocomplete(const std::string& prefix) const {
        std::vector<std::string> res;
        auto* node = find_node(prefix);
        if (node) collect(node, prefix, res);
        return res;
    }

    // 统计以 prefix 开头的单词数量
    int count_with_prefix(const std::string& prefix) const {
        auto* node = find_node(prefix);
        if (!node) return 0;
        return count_subtree(node);
    }

    // 最长公共前缀
    std::string longest_common_prefix() const {
        std::string prefix;
        auto* cur = root_;
        // 沿唯一路径走，直到分叉或到达单词结尾
        while (cur->children.size() == 1 && !cur->is_end) {
            auto it = cur->children.begin();
            prefix += it->first;
            cur = it->second;
        }
        return prefix;
    }

private:
    TrieNode* root_;

    TrieNode* find_node(const std::string& prefix) const {
        auto* cur = root_;
        for (char c : prefix) {
            auto it = cur->children.find(c);
            if (it == cur->children.end()) return nullptr;
            cur = it->second;
        }
        return cur;
    }

    bool remove(TrieNode* node, const std::string& word, int depth) {
        if (!node) return false;
        if (depth == (int)word.size()) {
            if (node->is_end) {
                node->is_end = false;
                --node->count;
            }
            return node->children.empty();
        }
        char c = word[depth];
        auto it = node->children.find(c);
        if (it == node->children.end()) return false;
        bool should_delete = remove(it->second, word, depth + 1);
        if (should_delete) {
            delete it->second;
            node->children.erase(it);
            return !node->is_end && node->children.empty();
        }
        return false;
    }

    void collect(TrieNode* node, const std::string& cur,
                 std::vector<std::string>& res) const {
        if (node->is_end) res.push_back(cur);
        for (auto& [c, child] : node->children)
            collect(child, cur + c, res);
    }

    int count_subtree(TrieNode* node) const {
        int cnt = node->count;
        for (auto& [c, child] : node->children)
            cnt += count_subtree(child);
        return cnt;
    }
};

// ==================== 入门：插入与查找 ====================
void basic_demo() {
    std::cout << "=== 入门：Trie 插入与查找 ===\n";

    Trie trie;
    for (const auto& w : {"apple", "app", "application", "apply", "apt", "banana"})
        trie.insert(w);

    for (const auto& w : {"app", "apple", "appl", "apt", "ban", "banana", "xyz"}) {
        std::cout << "search(\"" << w << "\")="
                  << std::boolalpha << trie.search(w)
                  << "  starts_with=\"" << w << "\"="
                  << trie.starts_with(w) << "\n";
    }
}

// ==================== 中级：自动补全 ====================
void autocomplete_demo() {
    std::cout << "\n=== 中级：自动补全 ===\n";

    Trie trie;
    std::vector<std::string> dict{
        "hello", "help", "helper", "helm", "world", "word", "work", "worker"
    };
    for (const auto& w : dict) trie.insert(w);

    auto show = [&](const std::string& prefix) {
        auto res = trie.autocomplete(prefix);
        std::cout << "prefix \"" << prefix << "\": ";
        for (const auto& w : res) std::cout << w << " ";
        std::cout << "(count=" << trie.count_with_prefix(prefix) << ")\n";
    };

    show("hel");
    show("wor");
    show("xyz");
}

// ==================== 高级：删除 + 最长公共前缀 ====================
void advanced_demo() {
    std::cout << "\n=== 高级：删除 + 最长公共前缀 ===\n";

    // 删除演示
    Trie trie;
    for (const auto& w : {"car", "card", "care", "careful", "cart"})
        trie.insert(w);

    std::cout << "before remove(\"care\"):\n";
    for (const auto& w : trie.autocomplete("car")) std::cout << "  " << w << "\n";

    trie.remove("care");
    std::cout << "after remove(\"care\"):\n";
    for (const auto& w : trie.autocomplete("car")) std::cout << "  " << w << "\n";

    // 最长公共前缀
    Trie trie2;
    for (const auto& w : {"flower", "flow", "flight"}) trie2.insert(w);
    std::cout << "longest common prefix of [flower,flow,flight]: \""
              << trie2.longest_common_prefix() << "\"\n";

    Trie trie3;
    for (const auto& w : {"dog", "racecar", "car"}) trie3.insert(w);
    std::cout << "longest common prefix of [dog,racecar,car]: \""
              << trie3.longest_common_prefix() << "\"\n";
}

int main() {
    basic_demo();
    autocomplete_demo();
    advanced_demo();
    return 0;
}
