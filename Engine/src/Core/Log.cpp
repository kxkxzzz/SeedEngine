// Log.cpp
#include "Seed/Core/Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace seed {

std::shared_ptr<spdlog::logger> Log::s_coreLogger;
std::shared_ptr<spdlog::logger> Log::s_clientLogger;

void Log::Init() {
    // 日志格式：[时间] 名称: 消息  —— %^...%$ 之间按等级着色
    // 例: [14:23:01] SEED: OpenGL 4.6 loaded
    spdlog::set_pattern("%^[%T] %n: %v%$");

    s_coreLogger = spdlog::stdout_color_mt("SEED");
    s_coreLogger->set_level(spdlog::level::trace);

    s_clientLogger = spdlog::stdout_color_mt("APP");
    s_clientLogger->set_level(spdlog::level::trace);
}

}  // namespace seed
