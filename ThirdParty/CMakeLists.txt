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

# --- stb_image（header-only，单独编译一次实现体） ---
add_library(stb_image STATIC stb_image_impl.cpp)
target_include_directories(stb_image PUBLIC stb)

# --- assimp（模型加载） ---
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)
