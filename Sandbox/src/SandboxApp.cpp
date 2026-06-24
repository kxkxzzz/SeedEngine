// SandboxApp.cpp —— 客户端测试程序
#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Layer.h>
#include <Seed/Core/Log.h>
#include <Seed/Core/TimeStep.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    void OnAttach() override {
        SEED_INFO("ExampleLayer::OnAttach");
    }

    void OnDetach() override {
        SEED_INFO("ExampleLayer::OnDetach");
    }

    void OnUpdate(Timestep ts) override {
        SEED_INFO("ExampleLayer::OnUpdate  dt={:.2f}ms", ts.GetMilliseconds());
    }

    void OnEvent(Event& e) override {
        SEED_INFO("ExampleLayer::OnEvent  {}", e.ToString());
    }
};

class Sandbox : public Application {
public:
    Sandbox() {
        PushLayer(new ExampleLayer());
    }
    ~Sandbox() override = default;
};

Application* CreateApplication() {
    return new Sandbox();
}

}  // namespace seed
