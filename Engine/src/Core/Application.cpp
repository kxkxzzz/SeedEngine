// Application.cpp
#include "Seed/Core/Application.h"

#include <GLFW/glfw3.h>

#include "Seed/Core/Log.h"
#include "Seed/Events/Event.h"
#include "Seed/Renderer/Renderer.h"

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
    // 设置事件回调函数，GLFW 回调触发后上抛给 Application
    m_window->SetEventCallback(SEED_BIND_EVENT_FN(Application::OnEvent));

    // Renderer 依赖 GL 上下文（窗口创建时已 MakeContextCurrent），必须在窗口创建后初始化
    Renderer::Init();
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

        // 清屏与绘制都交给 Layer 通过 RHI 完成
        for (auto* layer : m_layerStack)
            layer->OnUpdate(ts);

        m_window->SwapBuffers();
        m_window->PollEvents();
    }
}

// 触发事件时的回调函数
void Application::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(SEED_BIND_EVENT_FN(Application::OnWindowClose));

    // 从栈顶向下分发，某层处理后设 Handled=true 则停止传递
    for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
        if (e.Handled)
            break;
        (*it)->OnEvent(e);
    }
}

bool Application::OnWindowClose(WindowCloseEvent& /*e*/) {
    m_running = false;
    return true;
}

}  // namespace seed
