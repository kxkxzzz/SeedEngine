// Application.h —— 引擎应用基类
#pragma once

#include <memory>

#include "Seed/Core/LayerStack.h"
#include "Seed/Core/TimeStep.h"
#include "Seed/Core/Window.h"
#include "Seed/Events/Event.h"
#include "Seed/Events/WindowEvent.h"

namespace seed {

// 引擎应用基类：持有窗口和 Layer 栈，驱动主循环.
// 客户端（如 Sandbox）继承它，并由 CreateApplication() 工厂返回实例.
// 用 Application::Get() 单例访问
class Application {
public:
    Application();
    virtual ~Application();

    void Run();
    void OnEvent(Event& e);

    void PushLayer(Layer* layer);
    void PushOverlay(Layer* overlay);

    Window& GetWindow() { return *m_window; }

    static Application& Get() { return *s_instance; }

private:
    bool OnWindowClose(WindowCloseEvent& e);

    std::unique_ptr<Window> m_window;
    LayerStack m_layerStack;
    float m_lastFrameTime = 0.0f;
    bool m_running = true;

    static Application* s_instance;
};

// 由客户端实现，返回具体的 Application 子类实例.
// EntryPoint.h 提供的 main() 会调用它
Application* CreateApplication();

}  // namespace seed
