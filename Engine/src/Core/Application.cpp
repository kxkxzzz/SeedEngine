// Application.cpp
#include "Seed/Core/Application.h"

#include <glad/gl.h>

#include "Seed/Core/Log.h"
#include "Seed/Events/Event.h"

namespace seed {

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;

    // 日志须最先初始化，后续所有模块都依赖它
    Log::Init();
    SEED_CORE_INFO("SeedEngine 启动");

    WindowCreateInfo info;
    info.title  = "SeedEngine";
    info.width  = 1280;
    info.height = 720;
    m_window.reset(Window::Create(info));

    // 把 OnEvent 绑定到 Window 的事件回调，GLFW 触发后会流入此处
    m_window->SetEventCallback(SEED_BIND_EVENT_FN(Application::OnEvent));
}

Application::~Application() = default;

void Application::Run() {
    while (m_running) {
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_window->SwapBuffers();
        m_window->PollEvents();
    }
}

void Application::OnEvent(Event& e) {
    SEED_CORE_TRACE("{}", e.ToString());

    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(SEED_BIND_EVENT_FN(Application::OnWindowClose));
}

bool Application::OnWindowClose(WindowCloseEvent& /*e*/) {
    m_running = false;
    return true;
}

}  // namespace seed
