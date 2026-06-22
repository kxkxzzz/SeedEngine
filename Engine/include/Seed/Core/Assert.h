// Assert.h —— 断言宏
#pragma once

#include "Seed/Core/Log.h"

// 调试中断：clang-cl / MSVC 用 __debugbreak，其它平台回退到 SIGTRAP
#if defined(_MSC_VER)
    #define SEED_DEBUGBREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #include <csignal>
    #define SEED_DEBUGBREAK() raise(SIGTRAP)
#else
    #define SEED_DEBUGBREAK()
#endif

#ifdef SEED_DEBUG
    // 条件为假时：打错误日志（含文件:行号）并中断
    // 用法：SEED_ASSERT(ptr != nullptr, "ptr 不能为空: {}", name);
    #define SEED_ASSERT(check, ...)                                       \
        do {                                                              \
            if (!(check)) {                                               \
                SEED_ERROR("断言失败 [{}:{}]: {}", __FILE__, __LINE__,    \
                           fmt::format(__VA_ARGS__));                     \
                SEED_DEBUGBREAK();                                        \
            }                                                             \
        } while (0)

    #define SEED_CORE_ASSERT(check, ...)                                  \
        do {                                                              \
            if (!(check)) {                                               \
                SEED_CORE_ERROR("断言失败 [{}:{}]: {}", __FILE__,         \
                                __LINE__, fmt::format(__VA_ARGS__));      \
                SEED_DEBUGBREAK();                                        \
            }                                                             \
        } while (0)
#else
    // Release：断言编译为空，零开销
    #define SEED_ASSERT(check, ...)
    #define SEED_CORE_ASSERT(check, ...)
#endif
