# 3rdparty

存放第三方开源库，可通过 git submodule 或手动下载添加。

## 使用方式

```bash
# 添加子模块示例
git submodule add https://github.com/fmtlib/fmt.git fmt
git submodule add https://github.com/gabime/spdlog.git spdlog
```

然后在 CMakeLists.txt 中 `add_subdirectory` 对应目录。
