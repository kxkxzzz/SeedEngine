// SandboxApp.cpp —— 客户端测试程序
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Layer.h>
#include <Seed/Core/Log.h>
#include <Seed/Core/TimeStep.h>
#include <Seed/RHI/Shader.h>
#include <Seed/Renderer/CameraController.h>
#include <Seed/Renderer/Light.h>
#include <Seed/Renderer/Model.h>
#include <Seed/Renderer/Renderer.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    ExampleLayer() : m_cameraController(1280.0f / 720.0f) {
    }

    void OnAttach() override {
        m_model = std::make_unique<Model>("Sandbox/assets/models/BoxTextured/BoxTextured.gltf");
        m_shader = Shader::Create("Sandbox/assets/shaders/BlinnPhong.glsl");

        SEED_INFO("ExampleLayer 模型资源已就绪");
    }

    void OnUpdate(Timestep ts) override {
        m_cameraController.OnUpdate(ts.GetSeconds());

        Renderer::SetClearColor({0.1f, 0.15f, 0.2f, 1.0f});
        Renderer::Clear();

        // 设置场景光源
        DirectionalLight light;
        light.Direction = glm::vec3(0.3f, -1.0f, -0.5f);
        light.Color = glm::vec3(1.0f, 0.95f, 0.85f);
        light.Intensity = 1.2f;
        Renderer::SetDirectionalLight(light);

        Renderer::BeginScene(m_cameraController.GetCamera());

        // 让模型自转，便于观察 3D 效果与贴图
        m_rotation += ts.GetSeconds() * 30.0f;  // 每秒转 30 度
        glm::mat4 transform =
            glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.5f, 1.0f, 0.0f));

        m_model->Draw(m_shader, transform);
        Renderer::EndScene();
    }

    void OnEvent(Event& e) override {
        m_cameraController.OnEvent(e);
    }

private:
    std::unique_ptr<Model> m_model;
    std::shared_ptr<Shader> m_shader;
    CameraController m_cameraController;
    float m_rotation = 0.0f;
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
