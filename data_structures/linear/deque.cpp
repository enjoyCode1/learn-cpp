#include <iostream>
#include <stdexcept>
#include <vector>

// ============================================================
// 双端队列（Deque）实现
// 支持 O(1) 首尾插入/删除，基于分段数组（块状存储）
// ============================================================

template<typename T>
class Deque {
    static const size_t BLOCK = 8;  // 每块容量

    struct Block {
        T data[BLOCK];
    };

public:
    Deque() : front_block_(0), front_idx_(0),
              back_block_(0),  back_idx_(0), size_(0) {
        blocks_.push_back(new Block());
    }

    ~Deque() {
        for (auto* b : blocks_) delete b;
    }

    // 头部插入 O(1) 均摊
    void push_front(const T& val) {
        if (front_idx_ == 0) {
            blocks_.insert(blocks_.begin(), new Block());
            ++back_block_;
            front_block_ = 0;
            front_idx_ = BLOCK;
        }
        --front_idx_;
        blocks_[front_block_]->data[front_idx_] = val;
        ++size_;
    }

    // 尾部插入 O(1) 均摊
    void push_back(const T& val) {
        if (back_idx_ == BLOCK) {
            blocks_.push_back(new Block());
            ++back_block_;
            back_idx_ = 0;
        }
        blocks_[back_block_]->data[back_idx_] = val;
        ++back_idx_;
        ++size_;
    }

    // 头部删除 O(1)
    void pop_front() {
        if (empty()) throw std::underflow_error("Deque::pop_front on empty");
        ++front_idx_;
        if (front_idx_ == BLOCK) {
            delete blocks_[front_block_];
            blocks_.erase(blocks_.begin());
            --back_block_;
            front_idx_ = 0;
        }
        --size_;
    }

    // 尾部删除 O(1)
    void pop_back() {
        if (empty()) throw std::underflow_error("Deque::pop_back on empty");
        if (back_idx_ == 0) {
            delete blocks_[back_block_];
            blocks_.pop_back();
            --back_block_;
            back_idx_ = BLOCK;
        }
        --back_idx_;
        --size_;
    }

    T& front() {
        if (empty()) throw std::runtime_error("Deque::front on empty");
        return blocks_[front_block_]->data[front_idx_];
    }

    T& back() {
        if (empty()) throw std::runtime_error("Deque::back on empty");
        size_t idx = (back_idx_ == 0) ? BLOCK - 1 : back_idx_ - 1;
        size_t blk = (back_idx_ == 0) ? back_block_ - 1 : back_block_;
        return blocks_[blk]->data[idx];
    }

    // 随机访问 O(1)
    T& operator[](size_t i) {
        size_t abs_idx = front_idx_ + i;
        size_t blk = front_block_ + abs_idx / BLOCK;
        size_t idx = abs_idx % BLOCK;
        return blocks_[blk]->data[idx];
    }

    size_t size()  const { return size_; }
    bool   empty() const { return size_ == 0; }

    void print() const {
        std::cout << "front <-> [";
        for (size_t i = 0; i < size_; ++i) {
            size_t abs_idx = front_idx_ + i;
            size_t blk = front_block_ + abs_idx / BLOCK;
            size_t idx = abs_idx % BLOCK;
            std::cout << blocks_[blk]->data[idx];
            if (i + 1 < size_) std::cout << ", ";
        }
        std::cout << "] <-> back  (size=" << size_ << ")\n";
    }

private:
    std::vector<Block*> blocks_;
    size_t front_block_, front_idx_;
    size_t back_block_,  back_idx_;
    size_t size_;
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：Deque 基础操作 ===\n";

    Deque<int> dq;
    dq.push_back(3);
    dq.push_back(4);
    dq.push_back(5);
    dq.push_front(2);
    dq.push_front(1);
    dq.print();

    std::cout << "front=" << dq.front() << " back=" << dq.back() << "\n";

    dq.pop_front();
    dq.pop_back();
    std::cout << "after pop_front + pop_back: "; dq.print();
}

// ==================== 中级：随机访问 ====================
void random_access_demo() {
    std::cout << "\n=== 中级：随机访问 ===\n";

    Deque<int> dq;
    for (int i = 0; i < 20; ++i) dq.push_back(i * i);

    std::cout << "dq[0]=" << dq[0] << " dq[5]=" << dq[5]
              << " dq[19]=" << dq[19] << "\n";

    // 修改元素
    dq[10] = 999;
    std::cout << "after dq[10]=999: dq[10]=" << dq[10] << "\n";
    dq.print();
}

// ==================== 高级：滑动窗口最大值（单调双端队列）====================
// 经典应用：O(n) 求长度为 k 的每个窗口的最大值
std::vector<int> max_sliding_window(const std::vector<int>& nums, int k) {
    std::vector<int> result;
    Deque<int> dq;  // 存下标，队首始终是窗口最大值下标

    for (int i = 0; i < (int)nums.size(); ++i) {
        // 队首超出窗口范围则弹出
        while (!dq.empty() && dq.front() < i - k + 1)
            dq.pop_front();

        // 维护单调递减：移除队尾所有小于当前值的下标
        while (!dq.empty() && nums[dq.back()] < nums[i])
            dq.pop_back();

        dq.push_back(i);

        if (i >= k - 1)
            result.push_back(nums[dq.front()]);
    }
    return result;
}

void monotonic_deque_demo() {
    std::cout << "\n=== 高级：单调双端队列 - 滑动窗口最大值 ===\n";

    std::vector<int> nums{1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;
    auto res = max_sliding_window(nums, k);

    std::cout << "nums(k=" << k << "): ";
    for (int v : nums) std::cout << v << " ";
    std::cout << "\nwindow max:        ";
    for (int v : res)  std::cout << v << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    random_access_demo();
    monotonic_deque_demo();
    return 0;
}
