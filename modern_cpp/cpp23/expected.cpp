#include <expected>
#include <print>
#include <string>
#include <vector>
#include <optional>
#include <charconv>
#include <cmath>
#include <format>

// ==================== 入门级 ====================
// std::expected<T, E>：表示"可能失败的值"
// 类似 Rust 的 Result<T, E>，比异常更轻量，比错误码更表达力强

std::expected<int, std::string> parse_int(const std::string& s) {
    int result{};
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), result);
    if (ec != std::errc{})
        return std::unexpected("parse error: '" + s + "' is not an integer");
    return result;
}

void basic_expected() {
    std::println("=== 入门：expected 基础 ===");

    auto r1 = parse_int("42");
    auto r2 = parse_int("abc");
    auto r3 = parse_int("999");

    // has_value() 检查是否成功
    std::println("parse('42'):  ok={} val={}", r1.has_value(), r1.value_or(0));
    std::println("parse('abc'): ok={} err={}", r2.has_value(), r2.error());
    std::println("parse('999'): ok={} val={}", r3.has_value(), r3.value_or(0));
}

// ==================== 中级 ====================

// 链式操作：and_then / or_else / transform
std::expected<double, std::string> safe_sqrt(double x) {
    if (x < 0)
        return std::unexpected(std::format("sqrt of negative: {}", x));
    return std::sqrt(x);
}

std::expected<double, std::string> safe_divide(double a, double b) {
    if (b == 0.0)
        return std::unexpected("division by zero");
    return a / b;
}

void expected_chaining() {
    std::println("\n=== 中级：and_then / transform / or_else 链式 ===");

    // and_then：成功时继续，失败时传递错误
    auto result = parse_int("16")
        .and_then([](int v) -> std::expected<double, std::string> {
            return safe_sqrt(static_cast<double>(v));
        })
        .and_then([](double v) -> std::expected<double, std::string> {
            return safe_divide(100.0, v);
        });

    if (result)
        std::println("16 -> sqrt -> 100/x = {:.4f}", *result);
    else
        std::println("error: {}", result.error());

    // transform：成功时变换值，失败时透传
    auto r2 = parse_int("7")
        .transform([](int v) { return v * v; })        // 7 -> 49
        .transform([](int v) { return std::format("squared={}", v); });

    std::println("transform: {}", r2.value_or("error"));

    // or_else：失败时提供备选
    auto r3 = parse_int("bad")
        .or_else([](const std::string&) -> std::expected<int, std::string> {
            return 0;  // 失败时返回默认值 0
        });
    std::println("or_else fallback: {}", r3.value_or(-1));

    // 错误传播
    auto r4 = parse_int("bad")
        .and_then([](int v) -> std::expected<double, std::string> {
            return safe_sqrt(v);
        });
    std::println("propagated error: {}", r4.error());
}

// expected 用于多步骤处理
struct Config {
    std::string host;
    int port;
    int timeout;
};

std::expected<Config, std::string> parse_config(
    const std::string& host,
    const std::string& port_str,
    const std::string& timeout_str)
{
    auto port = parse_int(port_str);
    if (!port) return std::unexpected("invalid port: " + port.error());
    if (*port < 1 || *port > 65535)
        return std::unexpected(std::format("port out of range: {}", *port));

    auto timeout = parse_int(timeout_str);
    if (!timeout) return std::unexpected("invalid timeout: " + timeout.error());
    if (*timeout < 1)
        return std::unexpected("timeout must be positive");

    return Config{host, *port, *timeout};
}

void expected_config_demo() {
    std::println("\n=== 中级：多步骤验证 ===");

    auto cases = std::vector<std::tuple<std::string, std::string, std::string>>{
        {"example.com", "8080",  "30"},
        {"example.com", "abc",   "30"},
        {"example.com", "99999", "30"},
        {"example.com", "8080",  "-1"},
    };

    for (auto& [host, port, timeout] : cases) {
        auto cfg = parse_config(host, port, timeout);
        if (cfg)
            std::println("ok: {}:{} timeout={}", cfg->host, cfg->port, cfg->timeout);
        else
            std::println("err: {}", cfg.error());
    }
}

// ==================== 高级 ====================

// 自定义错误类型
struct AppError {
    enum class Code { ParseError, RangeError, NetworkError, NotFound };
    Code code;
    std::string message;

    static AppError parse(std::string msg) { return {Code::ParseError, std::move(msg)}; }
    static AppError range(std::string msg) { return {Code::RangeError, std::move(msg)}; }
    static AppError network(std::string msg) { return {Code::NetworkError, std::move(msg)}; }

    std::string to_string() const {
        const char* names[] = {"ParseError", "RangeError", "NetworkError", "NotFound"};
        return std::format("[{}] {}", names[static_cast<int>(code)], message);
    }
};

template<typename T>
using Result = std::expected<T, AppError>;

Result<int> parse_port(const std::string& s) {
    auto r = parse_int(s);
    if (!r) return std::unexpected(AppError::parse(r.error()));
    if (*r < 1 || *r > 65535)
        return std::unexpected(AppError::range(std::format("port {} out of [1,65535]", *r)));
    return *r;
}

Result<std::string> simulate_connect(const std::string& host, int port) {
    if (host.empty())
        return std::unexpected(AppError::network("empty host"));
    if (port == 9999)
        return std::unexpected(AppError::network("connection refused"));
    return std::format("connected to {}:{}", host, port);
}

void custom_error_type_demo() {
    std::println("\n=== 高级：自定义错误类型 + Result<T> ===");

    auto run = [&](const std::string& host, const std::string& port_str) {
        auto result = parse_port(port_str)
            .and_then([&](int port) { return simulate_connect(host, port); });

        if (result)
            std::println("success: {}", *result);
        else
            std::println("failed:  {}", result.error().to_string());
    };

    run("example.com", "8080");
    run("example.com", "abc");
    run("example.com", "99999");
    run("example.com", "9999");  // connection refused
    run("",            "8080");  // empty host
}

// expected 与 optional 对比
void expected_vs_optional() {
    std::println("\n=== 高级：expected vs optional 对比 ===");

    // optional：只知道成功/失败，不知道失败原因
    // expected：成功时有值，失败时有具体错误信息

    std::println("optional: 适合'值可能不存在'，不需要知道为什么");
    std::println("expected: 适合'操作可能失败'，需要知道失败原因");
    std::println("exception: 适合'真正异常情况'，性能敏感代码避免");

    // 互转
    auto exp = parse_int("42");
    std::optional<int> opt = exp ? std::optional<int>(*exp) : std::nullopt;
    std::println("expected->optional: {}", opt.value_or(-1));
}

int main() {
    basic_expected();
    expected_chaining();
    expected_config_demo();
    custom_error_type_demo();
    expected_vs_optional();
    return 0;
}
