#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ==================== 入门级 ====================

void basic_path_ops() {
    std::cout << "=== 入门：path 基础操作 ===\n";

    fs::path p = "D:/learn/C++/code";

    std::cout << "path:       " << p << "\n";
    std::cout << "filename:   " << p.filename() << "\n";
    std::cout << "parent:     " << p.parent_path() << "\n";
    std::cout << "root:       " << p.root_path() << "\n";
    std::cout << "exists:     " << fs::exists(p) << "\n";
    std::cout << "is_dir:     " << fs::is_directory(p) << "\n";

    // 路径拼接
    fs::path file = p / "CMakeLists.txt";
    std::cout << "joined:     " << file << "\n";
    std::cout << "extension:  " << file.extension() << "\n";
    std::cout << "stem:       " << file.stem() << "\n";
}

// ==================== 中级 ====================

void directory_iteration() {
    std::cout << "\n=== 中级：目录遍历 ===\n";

    fs::path dir = "D:/learn/C++/code/modern_cpp/cpp11";
    if (!fs::exists(dir)) {
        std::cout << "directory not found\n";
        return;
    }

    std::cout << "files in " << dir << ":\n";
    for (const auto& entry : fs::directory_iterator(dir)) {
        std::cout << "  " << entry.path().filename();
        if (entry.is_regular_file())
            std::cout << " [" << entry.file_size() << " bytes]";
        std::cout << "\n";
    }
}

void recursive_directory_iteration() {
    std::cout << "\n=== 中级：递归目录遍历 ===\n";

    fs::path dir = "D:/learn/C++/code/modern_cpp";
    size_t cpp_count = 0;
    uintmax_t total_size = 0;

    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
            ++cpp_count;
            total_size += entry.file_size();
            std::cout << "  " << entry.path().lexically_relative(dir) << "\n";
        }
    }
    std::cout << "total .cpp files: " << cpp_count
              << ", size: " << total_size << " bytes\n";
}

// ==================== 高级 ====================

void file_operations_demo() {
    std::cout << "\n=== 高级：文件操作 ===\n";

    fs::path tmp = fs::temp_directory_path() / "cpp17_fs_demo";

    // 创建目录
    fs::create_directories(tmp / "subdir");
    std::cout << "created: " << tmp << "\n";

    // 写文件
    {
        std::ofstream f(tmp / "test.txt");
        f << "Hello, filesystem!\n";
        f << "C++17 rocks.\n";
    }
    std::cout << "file size: " << fs::file_size(tmp / "test.txt") << " bytes\n";

    // 复制文件
    fs::copy_file(tmp / "test.txt", tmp / "subdir" / "test_copy.txt");
    std::cout << "copied to subdir\n";

    // 遍历临时目录
    for (const auto& e : fs::recursive_directory_iterator(tmp))
        std::cout << "  " << e.path().lexically_relative(tmp) << "\n";

    // 清理
    fs::remove_all(tmp);
    std::cout << "cleaned up: " << tmp << "\n";
}

void space_info_demo() {
    std::cout << "\n=== 高级：磁盘空间信息 ===\n";

    std::error_code ec;
    auto si = fs::space("D:/", ec);
    if (!ec) {
        auto gb = [](uintmax_t b) { return b / 1024.0 / 1024.0 / 1024.0; };
        std::cout << "D:/ capacity: " << gb(si.capacity) << " GB\n";
        std::cout << "D:/ free:     " << gb(si.free)     << " GB\n";
        std::cout << "D:/ available:" << gb(si.available)<< " GB\n";
    } else {
        std::cout << "space query failed: " << ec.message() << "\n";
    }
}

int main() {
    basic_path_ops();
    directory_iteration();
    recursive_directory_iteration();
    file_operations_demo();
    space_info_demo();
    return 0;
}
