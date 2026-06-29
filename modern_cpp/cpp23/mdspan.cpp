#include <print>
#include <mdspan>
#include <vector>
#include <array>
#include <numeric>
#include <algorithm>

// ==================== 入门级 ====================
// std::mdspan：多维数组视图，不拥有数据，零开销抽象
// 类比：std::span 是一维视图，mdspan 是多维视图

void basic_mdspan_2d() {
    std::println("=== 入门：mdspan 2D 矩阵视图 ===");

    // 底层数据（行优先存储）
    std::vector<int> data{
        1, 2, 3,
        4, 5, 6,
        7, 8, 9
    };

    // 用 mdspan 将一维数据视为 3x3 矩阵
    auto mat = std::mdspan(data.data(), 3, 3);

    std::println("3x3 matrix:");
    for (size_t i = 0; i < mat.extent(0); ++i) {
        std::print("  ");
        for (size_t j = 0; j < mat.extent(1); ++j)
            std::print("{:3}", mat[i, j]);  // C++23 多维下标
        std::println("");
    }

    // 修改元素（视图，修改影响原数据）
    mat[1, 1] = 99;
    std::println("after mat[1,1]=99: data[4]={}", data[4]);
}

// ==================== 中级 ====================

// 矩阵乘法：用 mdspan 清晰表达多维操作
void matrix_multiply(
    std::mdspan<const double, std::dextents<size_t, 2>> A,
    std::mdspan<const double, std::dextents<size_t, 2>> B,
    std::mdspan<double,       std::dextents<size_t, 2>> C)
{
    size_t M = A.extent(0), K = A.extent(1), N = B.extent(1);
    for (size_t i = 0; i < M; ++i)
        for (size_t j = 0; j < N; ++j) {
            C[i, j] = 0;
            for (size_t k = 0; k < K; ++k)
                C[i, j] += A[i, k] * B[k, j];
        }
}

void matmul_demo() {
    std::println("\n=== 中级：矩阵乘法 ===");

    // 2x3 矩阵 A
    std::vector<double> a_data{1, 2, 3, 4, 5, 6};
    auto A = std::mdspan(a_data.data(), 2, 3);

    // 3x2 矩阵 B
    std::vector<double> b_data{7, 8, 9, 10, 11, 12};
    auto B = std::mdspan(b_data.data(), 3, 2);

    // 结果 2x2
    std::vector<double> c_data(4);
    auto C = std::mdspan(c_data.data(), 2, 2);

    matrix_multiply(A, B, C);

    std::println("A(2x3) * B(3x2) = C(2x2):");
    for (size_t i = 0; i < C.extent(0); ++i) {
        std::print("  ");
        for (size_t j = 0; j < C.extent(1); ++j)
            std::print("{:8.1f}", C[i, j]);
        std::println("");
    }
}

// 3D 张量
void tensor_3d_demo() {
    std::println("\n=== 中级：3D 张量 ===");

    // 2x3x4 张量
    std::vector<int> data(2 * 3 * 4);
    std::iota(data.begin(), data.end(), 0);

    auto tensor = std::mdspan(data.data(), 2, 3, 4);

    std::println("tensor shape: {}x{}x{}",
                 tensor.extent(0), tensor.extent(1), tensor.extent(2));
    std::println("tensor[1,2,3] = {}", tensor[1, 2, 3]);

    // 计算预期值：1*12 + 2*4 + 3 = 23
    std::println("expected: 1*12 + 2*4 + 3 = {}", 1*12 + 2*4 + 3);
}

// ==================== 高级 ====================

// 自定义 layout：列优先（Fortran 风格）
void column_major_demo() {
    std::println("\n=== 高级：列优先布局 ===");

    std::vector<double> data{1, 4, 2, 5, 3, 6};  // 列优先存储

    // layout_left = 列优先（Fortran/BLAS 风格）
    // layout_right = 行优先（C 风格，默认）
    using col_major = std::mdspan<double,
                                  std::dextents<size_t, 2>,
                                  std::layout_left>;

    col_major mat(data.data(), 3, 2);

    std::println("column-major 3x2:");
    for (size_t i = 0; i < mat.extent(0); ++i) {
        std::print("  ");
        for (size_t j = 0; j < mat.extent(1); ++j)
            std::print("{:4.0f}", mat[i, j]);
        std::println("");
    }
}

// submdspan：取子视图（C++26 标准化，部分编译器已支持）
void mdspan_subview_demo() {
    std::println("\n=== 高级：mdspan 函数参数（零拷贝传递多维数组）===");

    // 演示 mdspan 作为函数参数，完全零拷贝
    auto fill_diagonal = [](auto mat, double val) {
        size_t n = std::min(mat.extent(0), mat.extent(1));
        for (size_t i = 0; i < n; ++i)
            mat[i, i] = val;
    };

    auto print_mat = [](auto mat) {
        for (size_t i = 0; i < mat.extent(0); ++i) {
            std::print("  ");
            for (size_t j = 0; j < mat.extent(1); ++j)
                std::print("{:6.1f}", mat[i, j]);
            std::println("");
        }
    };

    std::vector<double> data(16, 0.0);
    auto mat = std::mdspan(data.data(), 4, 4);

    fill_diagonal(mat, 1.0);  // 单位矩阵

    std::println("identity matrix 4x4:");
    print_mat(mat);
}

int main() {
    basic_mdspan_2d();
    matmul_demo();
    tensor_3d_demo();
    column_major_demo();
    mdspan_subview_demo();
    return 0;
}
