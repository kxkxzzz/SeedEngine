# Engine —— 引擎静态库 (Seed::Engine)

add_library(SeedEngine STATIC
    src/Core/Application.cpp
    src/Core/Log.cpp
    src/Platform/GLFW/GLFWWindow.cpp
    src/Platform/GLFW/GLFWInput.cpp
    src/Core/LayerStack.cpp

    # 渲染器
    src/Renderer/Renderer.cpp
    src/Renderer/PerspectiveCamera.cpp
    src/Renderer/CameraController.cpp
    src/Renderer/Mesh.cpp
    src/Renderer/Model.cpp

    # RHI 抽象层工厂
    src/RHI/RenderAPI.cpp
    src/RHI/Buffer.cpp
    src/RHI/VertexArray.cpp
    src/RHI/Shader.cpp
    src/RHI/Texture.cpp

    # RHI OpenGL 后端
    src/RHI/OpenGL/OpenGLRenderAPI.cpp
    src/RHI/OpenGL/OpenGLBuffer.cpp
    src/RHI/OpenGL/OpenGLVertexArray.cpp
    src/RHI/OpenGL/OpenGLShader.cpp
    src/RHI/OpenGL/OpenGLTexture.cpp
)

# 现代 CMake 别名：客户端用 Seed::Engine 链接
add_library(Seed::Engine ALIAS SeedEngine)

# 按构建配置定义调试/发布宏（多配置生成器用 $<CONFIG>）
target_compile_definitions(SeedEngine PUBLIC
    $<$<CONFIG:Debug>:SEED_DEBUG>
    $<$<CONFIG:RelWithDebInfo>:SEED_DEBUG>
    $<$<CONFIG:Release>:SEED_RELEASE>
)

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
        stb_image
        assimp
)
