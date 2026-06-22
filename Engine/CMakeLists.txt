# Engine —— 引擎静态库 (Seed::Engine)

add_library(SeedEngine STATIC
    src/Core/Application.cpp
    src/Platform/GLFW/GLFWWindow.cpp
)

# 现代 CMake 别名：客户端用 Seed::Engine 链接
add_library(Seed::Engine ALIAS SeedEngine)

target_include_directories(SeedEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include   # 公开头：<Seed/...>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src       # 私有头：平台实现
)

target_link_libraries(SeedEngine
    PUBLIC
        glm::glm
        spdlog::spdlog
    PRIVATE
        glfw
        glad
)
