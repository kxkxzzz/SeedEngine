cmake_minimum_required(VERSION 3.27)

project(SeedEngine LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 导出 compile_commands.json，供 clangd 使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 统一可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(ThirdParty)   # 第三方依赖 glfw/glad/glm/spdlog
add_subdirectory(Engine)       # 引擎静态库 Seed::Engine
add_subdirectory(Sandbox)      # 客户端测试程序
