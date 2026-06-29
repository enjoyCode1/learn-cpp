#include <iostream>
#include <vector>
#include <random>
#include <limits>
#include <functional>
#include <string>

// ============================================================
// 跳表（Skip List）实现
// 有序链表 + 多级索引，O(log n) 平均查找/插入/删除
// 比平衡树实现简单，Redis ZSet 底层使用跳表
// ============================================================

template<typename K, typename V>
class SkipList {
    static const int MAX_LEVEL = 16;
    static constexpr float P = 0.5f;  // 晋升概率

    struct Node {
        K    key;
        V    val;
        std::vector<Node*> forward;  // forward[i] 指向第 i 层的下一个节点

        Node(K k, V v, int level)
            : key(k), val(v), forward(level + 1, nullptr) {}
    };

public:
    SkipList()
        : level_(0), size_(0),
          head_(new Node(std::numeric_limits<K>::min(), V{}, MAX_LEVEL)),
          rng_(std::random_device{}()),
          dist_(0.0f, 1.0f) {}

    ~SkipList() {
        Node* cur = head_->forward[0];
        while (cur) { Node* next = cur->forward[0]; delete cur; cur = next; }
        delete head_;
    }

    // 插入，O(log n)
    void insert(const K& key, const V& val) {
        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* cur = head_;

        // 从最高层向下找每层的插入位置
        for (int i = level_; i >= 0; --i) {
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
            update[i] = cur;
        }

        cur = cur->forward[0];
        if (cur && cur->key == key) { cur->val = val; return; }  // 更新

        int new_level = random_level();
        if (new_level > level_) {
            for (int i = level_ + 1; i <= new_level; ++i) update[i] = head_;
            level_ = new_level;
        }

        Node* node = new Node(key, val, new_level);
        for (int i = 0; i <= new_level; ++i) {
            node->forward[i]    = update[i]->forward[i];
            update[i]->forward[i] = node;
        }
        ++size_;
    }

    // 查找，O(log n)
    V* find(const K& key) {
        Node* cur = head_;
        for (int i = level_; i >= 0; --i)
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
        cur = cur->forward[0];
        if (cur && cur->key == key) return &cur->val;
        return nullptr;
    }

    // 删除，O(log n)
    bool remove(const K& key) {
        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* cur = head_;

        for (int i = level_; i >= 0; --i) {
            while (cur->forward[i] && cur->forward[i]->key < key)
                cur = cur->forward[i];
            update[i] = cur;
        }

        cur = cur->forward[0];
        if (!cur || cur->key != key) return false;

        for (int i = 0; i <= level_; ++i) {
            if (update[i]->forward[i] != cur) break;
            update[i]->forward[i] = cur->forward[i];
        }
        delete cur;

        while (level_ > 0 && !head_->forward[level_]) --level_;
        --size_;
        return true;
    }

    // 范围查询 [lo, hi]
    std::vector<std::pair<K,V>> range(const K& lo, const K& hi) const {
        std::vector<std::pair<K,V>> res;
        Node* cur = head_;
        for (int i = level_; i >= 0; --i)
            while (cur->forward[i] && cur->forward[i]->key < lo)
                cur = cur->forward[i];
        cur = cur->forward[0];
        while (cur && cur->key <= hi) {
            res.push_back({cur->key, cur->val});
            cur = cur->forward[0];
        }
        return res;
    }

    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }
    int    level() const { return level_; }

    void print() const {
        std::cout << "SkipList (level=" << level_ << " size=" << size_ << "):\n";
        for (int i = level_; i >= 0; --i) {
            std::cout << "L" << i << ": ";
            Node* cur = head_->forward[i];
            while (cur) { std::cout << cur->key << " "; cur = cur->forward[i]; }
            std::cout << "\n";
        }
    }

private:
    int    level_, size_;
    Node*  head_;
    std::mt19937                  rng_;
    std::uniform_real_distribution<float> dist_;

    int random_level() {
        int lvl = 0;
        while (dist_(rng_) < P && lvl < MAX_LEVEL) ++lvl;
        return lvl;
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：SkipList 基础操作 ===\n";

    SkipList<int, std::string> sl;
    sl.insert(3, "three");
    sl.insert(1, "one");
    sl.insert(5, "five");
    sl.insert(2, "two");
    sl.insert(4, "four");

    sl.print();

    if (auto* v = sl.find(3)) std::cout << "find(3)=" << *v << "\n";
    if (!sl.find(9))           std::cout << "find(9)=null\n";
}

// ==================== 中级：范围查询 ====================
void range_demo() {
    std::cout << "\n=== 中级：范围查询 ===\n";

    SkipList<int, int> sl;
    for (int i = 1; i <= 10; ++i) sl.insert(i, i * i);

    auto res = sl.range(3, 7);
    std::cout << "range [3,7]: ";
    for (auto [k, v] : res) std::cout << k << ":" << v << " ";
    std::cout << "\n";
}

// ==================== 高级：模拟 Redis ZSet 排行榜 ====================
void leaderboard_demo() {
    std::cout << "\n=== 高级：排行榜（模拟 Redis ZSet）===\n";

    // score -> name（分数为 key，支持范围查分数段内的玩家）
    SkipList<int, std::string> zset;
    zset.insert(9500, "Alice");
    zset.insert(8800, "Bob");
    zset.insert(9200, "Carol");
    zset.insert(7600, "Dave");
    zset.insert(9800, "Eve");

    zset.print();

    // 查询分数在 9000~9999 的玩家
    std::cout << "players with score 9000-9999:\n";
    for (auto [score, name] : zset.range(9000, 9999))
        std::cout << "  " << name << ": " << score << "\n";

    // 更新分数
    zset.remove(8800);
    zset.insert(9100, "Bob");
    std::cout << "after Bob score update:\n";
    for (auto [score, name] : zset.range(0, 99999))
        std::cout << "  " << name << ": " << score << "\n";
}

int main() {
    basic_demo();
    range_demo();
    leaderboard_demo();
    return 0;
}
