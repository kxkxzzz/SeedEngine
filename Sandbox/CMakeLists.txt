# Sandbox —— 客户端可执行文件
add_executable(Sandbox src/SandboxApp.cpp)
target_link_libraries(Sandbox PRIVATE Seed::Engine)
