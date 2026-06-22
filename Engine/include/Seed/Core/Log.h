// Log.h —— 日志系统（封装 spdlog）
#pragma once

#include <memory>

// 屏蔽 spdlog 头里第三方代码的告警，不污染引擎自身的 -Wall
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>  // 支持用 operator<< 的类型直接格式化

namespace seed {

// 双 logger：Core 给引擎内部用，Client 给客户端（Sandbox/Editor）用,
// 两者输出带不同名称前缀，便于区分日志来源
class Log {
public:
    static void Init();

    static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
        return s_coreLogger;
    }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() {
        return s_clientLogger;
    }

private:
    static std::shared_ptr<spdlog::logger> s_coreLogger;
    static std::shared_ptr<spdlog::logger> s_clientLogger;
};

}  // namespace seed

// --- 引擎内部日志宏（SEED_CORE_*）---
#define SEED_CORE_TRACE(...) ::seed::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define SEED_CORE_INFO(...) ::seed::Log::GetCoreLogger()->info(__VA_ARGS__)
#define SEED_CORE_WARN(...) ::seed::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define SEED_CORE_ERROR(...) ::seed::Log::GetCoreLogger()->error(__VA_ARGS__)
#define SEED_CORE_CRITICAL(...) \
    ::seed::Log::GetCoreLogger()->critical(__VA_ARGS__)

// --- 客户端日志宏（SEED_*）---
#define SEED_TRACE(...) ::seed::Log::GetClientLogger()->trace(__VA_ARGS__)
#define SEED_INFO(...) ::seed::Log::GetClientLogger()->info(__VA_ARGS__)
#define SEED_WARN(...) ::seed::Log::GetClientLogger()->warn(__VA_ARGS__)
#define SEED_ERROR(...) ::seed::Log::GetClientLogger()->error(__VA_ARGS__)
#define SEED_CRITICAL(...) ::seed::Log::GetClientLogger()->critical(__VA_ARGS__)
