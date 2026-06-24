// Application.cpp
#include "Seed/Core/Application.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
    m_window->SetEventCallback(SEED_BIND_EVENT_FN(Application::OnEvent));
}

Application::~Application() = default;

void Application::PushLayer(Layer* layer) {
    m_layerStack.PushLayer(layer);
}

void Application::PushOverlay(Layer* overlay) {
    m_layerStack.PushOverlay(overlay);
}

void Application::Run() {
    while (m_running) {
        float time = static_cast<float>(glfwGetTime());
        Timestep ts = time - m_lastFrameTime;
        m_lastFrameTime = time;

        // 从底向上逐层 Update
        for (auto* layer : m_layerStack)
            layer->OnUpdate(ts);

        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_window->SwapBuffers();
        m_window->PollEvents();
    }
}

void Application::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(SEED_BIND_EVENT_FN(Application::OnWindowClose));

    // 从栈顶向下分发，某层处理后设 Handled=true 则停止传递
    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
        if (e.Handled) break;
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& /*e*/) {
    m_running = false;
    return true;
}

}  // namespace seed
