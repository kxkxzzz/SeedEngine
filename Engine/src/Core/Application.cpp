// Application.cpp
#include "Seed/Core/Application.h"

#include <glad/gl.h>

#include "Seed/Core/Log.h"

namespace seed {

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;

    // 日志须最先初始化，后续所有模块都依赖它
    Log::Init();
    SEED_CORE_INFO("SeedEngine 启动");

    WindowCreateInfo info;
    info.title = "SeedEngine";
    info.width = 1280;
    info.height = 720;
    m_window.reset(Window::Create(info));
}

Application::~Application() = default;

void Application::Run() {
    while (m_running && !m_window->ShouldClose()) {
        // 阶段 A：仅清屏，验证窗口与 OpenGL 上下文可用。
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_window->SwapBuffers();
        m_window->PollEvents();
    }
}

}  // namespace seed
