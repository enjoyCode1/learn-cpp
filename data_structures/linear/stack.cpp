#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

// ============================================================
// 栈（Stack）实现
// LIFO：后进先出，基于动态数组实现
// ============================================================

template<typename T>
class Stack {
public:
    // 压栈 O(1) 均摊
    void push(const T& val) { data_.push_back(val); }
    void push(T&& val)      { data_.push_back(std::move(val)); }

    // 弹栈 O(1)
    void pop() {
        if (empty()) throw std::underflow_error("Stack::pop on empty");
        data_.pop_back();
    }

    // 查看栈顶，不弹出
    T& top() {
        if (empty()) throw std::underflow_error("Stack::top on empty");
        return data_.back();
    }
    const T& top() const {
        if (empty()) throw std::underflow_error("Stack::top on empty");
        return data_.back();
    }

    size_t size()  const { return data_.size(); }
    bool   empty() const { return data_.empty(); }

    void print() const {
        std::cout << "top -> [";
        for (int i = (int)data_.size() - 1; i >= 0; --i) {
            std::cout << data_[i];
            if (i > 0) std::cout << ", ";
        }
        std::cout << "] <- bottom\n";
    }

private:
    std::vector<T> data_;
};

// ==================== 入门：基础操作 ====================
void basic_demo() {
    std::cout << "=== 入门：Stack 基础操作 ===\n";

    Stack<int> s;
    for (int i = 1; i <= 5; ++i) s.push(i);
    s.print();

    std::cout << "top = " << s.top() << "\n";
    s.pop();
    std::cout << "after pop: "; s.print();
}

// ==================== 中级：括号匹配 ====================
bool is_balanced(const std::string& expr) {
    Stack<char> s;
    for (char c : expr) {
        if (c == '(' || c == '[' || c == '{') {
            s.push(c);
        } else if (c == ')' || c == ']' || c == '}') {
            if (s.empty()) return false;
            char top = s.top(); s.pop();
            if ((c == ')' && top != '(') ||
                (c == ']' && top != '[') ||
                (c == '}' && top != '{'))
                return false;
        }
    }
    return s.empty();
}

void bracket_demo() {
    std::cout << "\n=== 中级：括号匹配 ===\n";

    std::vector<std::string> exprs{
        "({[]})",
        "((()))",
        "([)]",
        "{[}",
        "",
        "()[]{}",
    };

    for (const auto& e : exprs)
        std::cout << "\"" << e << "\" -> "
                  << (is_balanced(e) ? "balanced" : "NOT balanced") << "\n";
}

// ==================== 中级：逆波兰表达式求值 ====================
int eval_rpn(const std::vector<std::string>& tokens) {
    Stack<int> s;
    for (const auto& tok : tokens) {
        if (tok == "+" || tok == "-" || tok == "*" || tok == "/") {
            int b = s.top(); s.pop();
            int a = s.top(); s.pop();
            if      (tok == "+") s.push(a + b);
            else if (tok == "-") s.push(a - b);
            else if (tok == "*") s.push(a * b);
            else                 s.push(a / b);
        } else {
            s.push(std::stoi(tok));
        }
    }
    return s.top();
}

void rpn_demo() {
    std::cout << "\n=== 中级：逆波兰表达式 ===\n";

    // (2 + 3) * 4 = 20
    std::vector<std::string> expr1{"2", "3", "+", "4", "*"};
    std::cout << "(2+3)*4 = " << eval_rpn(expr1) << "\n";

    // 10 - (3 + 2) = 5
    std::vector<std::string> expr2{"10", "3", "2", "+", "-"};
    std::cout << "10-(3+2) = " << eval_rpn(expr2) << "\n";
}

// ==================== 高级：单调栈求下一个更大元素 ====================
std::vector<int> next_greater(const std::vector<int>& nums) {
    int n = (int)nums.size();
    std::vector<int> result(n, -1);
    Stack<int> s;  // 存下标

    for (int i = 0; i < n; ++i) {
        // 当前元素比栈顶大，则栈顶找到了下一个更大元素
        while (!s.empty() && nums[i] > nums[s.top()]) {
            result[s.top()] = nums[i];
            s.pop();
        }
        s.push(i);
    }
    return result;
}

void monotonic_stack_demo() {
    std::cout << "\n=== 高级：单调栈 - 下一个更大元素 ===\n";

    std::vector<int> nums{2, 1, 5, 3, 6, 4, 8, 5};
    auto res = next_greater(nums);

    std::cout << "nums:  ";
    for (int v : nums) std::cout << v << " ";
    std::cout << "\nnext>: ";
    for (int v : res)  std::cout << (v == -1 ? "-" : std::to_string(v)) << " ";
    std::cout << "\n";
}

int main() {
    basic_demo();
    bracket_demo();
    rpn_demo();
    monotonic_stack_demo();
    return 0;
}
