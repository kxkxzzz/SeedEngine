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
#include <Seed/Renderer/Model.h>
#include <Seed/Renderer/Renderer.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    ExampleLayer() : m_cameraController(1280.0f / 720.0f) {
    }

    void OnAttach() override {
        m_model = std::make_unique<Model>("Sandbox/assets/models/BoxTextured/BoxTextured.gltf");

        std::string vertexSrc = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Normal;
            layout(location = 2) in vec2 a_TexCoords;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Transform;

            out vec2 v_TexCoords;

            void main() {
                v_TexCoords = a_TexCoords;
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 450 core
            in vec2 v_TexCoords;
            out vec4 color;

            uniform sampler2D u_DiffuseTexture;
            uniform int u_HasTexture;

            void main() {
                if (u_HasTexture == 1)
                    color = texture(u_DiffuseTexture, v_TexCoords);
                else
                    color = vec4(0.6, 0.6, 0.6, 1.0);
            }
        )";

        m_shader = Shader::Create("TexturedMesh", vertexSrc, fragmentSrc);

        SEED_INFO("ExampleLayer 模型资源已就绪");
    }

    void OnUpdate(Timestep ts) override {
        m_cameraController.OnUpdate(ts.GetSeconds());

        Renderer::SetClearColor({0.1f, 0.15f, 0.2f, 1.0f});
        Renderer::Clear();

        // 让模型自转，便于观察 3D 效果与贴图
        m_rotation += ts.GetSeconds() * 30.0f;  // 每秒转 30 度
        glm::mat4 transform =
            glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.5f, 1.0f, 0.0f));

        Renderer::BeginScene(m_cameraController.GetCamera());
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
