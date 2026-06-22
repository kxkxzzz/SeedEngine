// Log.cpp
#include "Seed/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace seed {

std::shared_ptr<spdlog::logger> Log::s_coreLogger;
std::shared_ptr<spdlog::logger> Log::s_clientLogger;

void Log::Init() {
#ifdef _WIN32
    // Windows 控制台默认 GBK(936)，强制切到 UTF-8 避免中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // 日志格式：[时间] 名称: 消息  —— %^...%$ 之间按等级着色
    // 例: [14:23:01] SEED: OpenGL 4.6 loaded
    spdlog::set_pattern("%^[%T] %n: %v%$");

    s_coreLogger = spdlog::stdout_color_mt("SEED");
    s_coreLogger->set_level(spdlog::level::trace);

    s_clientLogger = spdlog::stdout_color_mt("APP");
    s_clientLogger->set_level(spdlog::level::trace);
}

}  // namespace seed