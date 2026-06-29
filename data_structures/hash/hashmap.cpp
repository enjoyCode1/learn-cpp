#include <iostream>
#include <vector>
#include <list>
#include <functional>
#include <stdexcept>
#include <string>

// ============================================================
// HashMap 实现（链地址法解决冲突）
// 支持插入、查找、删除，O(1) 均摊
// 负载因子超过阈值时自动扩容 rehash
// ============================================================

template<typename K, typename V, typename Hash = std::hash<K>>
class HashMap {
    struct Entry {
        K key;
        V val;
    };

    using Bucket = std::list<Entry>;

public:
    explicit HashMap(size_t capacity = 16, float max_load = 0.75f)
        : buckets_(capacity), size_(0), max_load_(max_load) {}

    // 插入或更新，O(1) 均摊
    void put(const K& key, const V& val) {
        if (load_factor() >= max_load_) rehash(buckets_.size() * 2);
        auto& bucket = buckets_[bucket_index(key)];
        for (auto& e : bucket) {
            if (e.key == key) { e.val = val; return; }
        }
        bucket.push_back({key, val});
        ++size_;
    }

    // 查找，O(1) 均摊
    V* get(const K& key) {
        auto& bucket = buckets_[bucket_index(key)];
        for (auto& e : bucket)
            if (e.key == key) return &e.val;
        return nullptr;
    }

    const V* get(const K& key) const {
        const auto& bucket = buckets_[bucket_index(key)];
        for (const auto& e : bucket)
            if (e.key == key) return &e.val;
        return nullptr;
    }

    // operator[] 不存在时插入默认值
    V& operator[](const K& key) {
        if (load_factor() >= max_load_) rehash(buckets_.size() * 2);
        auto& bucket = buckets_[bucket_index(key)];
        for (auto& e : bucket)
            if (e.key == key) return e.val;
        bucket.push_back({key, V{}});
        ++size_;
        return bucket.back().val;
    }

    // 删除，O(1) 均摊
    bool remove(const K& key) {
        auto& bucket = buckets_[bucket_index(key)];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                bucket.erase(it);
                --size_;
                return true;
            }
        }
        return false;
    }

    bool contains(const K& key) const { return get(key) != nullptr; }

    size_t size()        const { return size_; }
    bool   empty()       const { return size_ == 0; }
    float  load_factor() const { return (float)size_ / buckets_.size(); }

    // 遍历所有键值对
    void foreach(std::function<void(const K&, const V&)> fn) const {
        for (const auto& bucket : buckets_)
            for (const auto& e : bucket)
                fn(e.key, e.val);
    }

    void print_stats() const {
        std::cout << "size=" << size_
                  << " buckets=" << buckets_.size()
                  << " load=" << load_factor() << "\n";
        // 打印桶的分布
        int used = 0, max_chain = 0;
        for (const auto& b : buckets_) {
            if (!b.empty()) ++used;
            max_chain = std::max(max_chain, (int)b.size());
        }
        std::cout << "used_buckets=" << used
                  << " max_chain_len=" << max_chain << "\n";
    }

private:
    std::vector<Bucket> buckets_;
    size_t              size_;
    float               max_load_;
    Hash                hasher_;

    size_t bucket_index(const K& key) const {
        return hasher_(key) % buckets_.size();
    }

    void rehash(size_t new_cap) {
        std::vector<Bucket> new_buckets(new_cap);
        for (auto& bucket : buckets_)
            for (auto& e : bucket) {
                size_t idx = hasher_(e.key) % new_cap;
                new_buckets[idx].push_back(std::move(e));
            }
        buckets_ = std::move(new_buckets);
    }
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：HashMap 基础操作 ===\n";

    HashMap<std::string, int> map;
    map.put("alice", 90);
    map.put("bob",   85);
    map.put("carol", 92);

    // 查找
    if (auto* v = map.get("alice")) std::cout << "alice=" << *v << "\n";
    if (auto* v = map.get("bob"))   std::cout << "bob="   << *v << "\n";
    std::cout << "contains dave: " << std::boolalpha << map.contains("dave") << "\n";

    // 更新
    map.put("alice", 95);
    if (auto* v = map.get("alice")) std::cout << "alice(updated)=" << *v << "\n";

    // operator[]
    map["dave"] = 78;
    std::cout << "dave=" << map["dave"] << "\n";

    // 删除
    map.remove("bob");
    std::cout << "after remove bob, contains bob: " << map.contains("bob") << "\n";
    std::cout << "size=" << map.size() << "\n";
}

// ==================== 中级：词频统计 ====================
void word_count_demo() {
    std::cout << "\n=== 中级：词频统计 ===\n";

    std::string text = "the quick brown fox jumps over the lazy dog the fox";
    HashMap<std::string, int> freq;

    std::string word;
    for (char c : text + " ") {
        if (c == ' ') {
            if (!word.empty()) { freq[word]++; word.clear(); }
        } else word += c;
    }

    freq.foreach([](const std::string& k, int v) {
        std::cout << "  " << k << ": " << v << "\n";
    });
}

// ==================== 高级：LRU 缓存（HashMap + 双向链表）====================
class LRUCache {
    struct Node {
        int key, val;
        Node *prev, *next;
        Node(int k, int v) : key(k), val(v), prev(nullptr), next(nullptr) {}
    };

public:
    explicit LRUCache(int cap) : cap_(cap) {
        head_ = new Node(0, 0); tail_ = new Node(0, 0);
        head_->next = tail_; tail_->prev = head_;
    }
    ~LRUCache() {
        // 通过链表遍历删除所有节点，不依赖 map 迭代器
        Node* cur = head_->next;
        while (cur != tail_) { Node* next = cur->next; delete cur; cur = next; }
        delete head_; delete tail_;
    }

    int get(int key) {
        auto* v = map_.get(key);
        if (!v) return -1;
        move_to_front(*v);
        return (*v)->val;
    }

    void put(int key, int val) {
        auto* v = map_.get(key);
        if (v) {
            (*v)->val = val;
            move_to_front(*v);
            return;
        }
        if ((int)map_.size() == cap_) {
            // 淘汰最久未用（链表尾部）
            auto* lru = tail_->prev;
            remove_node(lru);
            map_.remove(lru->key);
            delete lru;
        }
        auto* node = new Node(key, val);
        insert_front(node);
        map_.put(key, node);
    }

    void print() const {
        std::cout << "LRU [head->tail]: ";
        for (Node* cur = head_->next; cur != tail_; cur = cur->next)
            std::cout << cur->key << ":" << cur->val << " ";
        std::cout << "\n";
    }

private:
    int    cap_;
    HashMap<int, Node*> map_;
    Node*  head_, *tail_;

    void remove_node(Node* n) {
        n->prev->next = n->next;
        n->next->prev = n->prev;
    }
    void insert_front(Node* n) {
        n->next = head_->next; n->prev = head_;
        head_->next->prev = n; head_->next = n;
    }
    void move_to_front(Node* n) { remove_node(n); insert_front(n); }
};

void lru_demo() {
    std::cout << "\n=== 高级：LRU 缓存 ===\n";

    LRUCache cache(3);
    cache.put(1, 1); cache.print();
    cache.put(2, 2); cache.print();
    cache.put(3, 3); cache.print();
    std::cout << "get(1)=" << cache.get(1) << "\n"; cache.print();
    cache.put(4, 4); // 淘汰 key=2
    cache.print();
    std::cout << "get(2)=" << cache.get(2) << " (should be -1)\n";
    std::cout << "get(3)=" << cache.get(3) << "\n"; cache.print();
}

int main() {
    basic_demo();
    word_count_demo();
    lru_demo();
    return 0;
}
