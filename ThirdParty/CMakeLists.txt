# ThirdParty —— 第三方依赖统一接入
# glfw/glm/spdlog 是 git submodule；glad 是本地生成的源码。

# --- GLFW ---
# 关闭 GLFW 自带的文档/测试/示例，只编库本身
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)

# --- glad (本地生成) ---
add_subdirectory(glad)

# --- glm (header-only) ---
add_subdirectory(glm)

# --- spdlog ---
# header-only 模式即可，关闭其示例/测试/bench
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
add_subdirectory(spdlog)
