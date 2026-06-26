# C++ Learning

C++ 学习目录，涵盖现代 C++ 特性、STL 使用与实现、算法练习及第三方库学习。

## 目录结构

```
.
├── CMakeLists.txt
├── 3rdparty/               # 第三方开源库
├── modern_cpp/
│   ├── cpp11/              # C++11 特性
│   ├── cpp14/              # C++14 特性
│   ├── cpp17/              # C++17 特性
│   └── cpp20/              # C++20 特性
├── stl/
│   ├── usage/              # STL 使用练习
│   └── implementation/     # STL 手写实现
└── algorithms/             # 算法练习
```

## 构建

```bash
mkdir build && cd build
cmake ..
make
```

## 新增练习项目

在对应目录下新建子目录，添加源文件和 CMakeLists.txt，然后在父级 CMakeLists.txt 中 `add_subdirectory` 即可。
