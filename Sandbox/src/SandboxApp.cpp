// SandboxApp.cpp —— 客户端测试程序
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Layer.h>
#include <Seed/Core/Log.h>
#include <Seed/Core/TimeStep.h>
#include <Seed/RHI/Buffer.h>
#include <Seed/RHI/Shader.h>
#include <Seed/RHI/VertexArray.h>
#include <Seed/Renderer/CameraController.h>
#include <Seed/Renderer/Renderer.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    ExampleLayer() : m_cameraController(1280.0f / 720.0f) {
    }

    void OnAttach() override {
        m_vertexArray = VertexArray::Create();

        // 立方体 8 个顶点：位置(xyz) + 颜色(rgb)，每个顶点一个独特颜色方便观察朝向
        float vertices[8 * 6] = {
            // 位置                颜色
            -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  // 0
            0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // 1
            0.5f,  0.5f,  -0.5f, 0.0f, 0.0f, 1.0f,  // 2
            -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, 0.0f,  // 3
            -0.5f, -0.5f, 0.5f,  1.0f, 0.0f, 1.0f,  // 4
            0.5f,  -0.5f, 0.5f,  0.0f, 1.0f, 1.0f,  // 5
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  // 6
            -0.5f, 0.5f,  0.5f,  0.3f, 0.3f, 0.3f,  // 7
        };

        auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Color"},
        });
        m_vertexArray->AddVertexBuffer(vertexBuffer);

        // 6 个面 × 2 个三角形 = 12 个三角形，36 个索引
        uint32_t indices[36] = {
            0, 1, 2, 2, 3, 0,  // 背面 (z=-0.5)
            4, 5, 6, 6, 7, 4,  // 正面 (z=+0.5)
            4, 0, 3, 3, 7, 4,  // 左面 (x=-0.5)
            1, 5, 6, 6, 2, 1,  // 右面 (x=+0.5)
            4, 5, 1, 1, 0, 4,  // 底面 (y=-0.5)
            3, 2, 6, 6, 7, 3,  // 顶面 (y=+0.5)
        };
        auto indexBuffer = IndexBuffer::Create(indices, 36);
        m_vertexArray->SetIndexBuffer(indexBuffer);

        std::string vertexSrc = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Color;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Transform;

            out vec3 v_Color;

            void main() {
                v_Color = a_Color;
                gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
            }
        )";

        std::string fragmentSrc = R"(
            #version 450 core
            in vec3 v_Color;
            out vec4 color;

            void main() {
                color = vec4(v_Color, 1.0);
            }
        )";

        m_shader = Shader::Create("Cube", vertexSrc, fragmentSrc);

        SEED_INFO("ExampleLayer 立方体资源已就绪");
    }

    void OnUpdate(Timestep ts) override {
        m_cameraController.OnUpdate(ts.GetSeconds());

        Renderer::SetClearColor({0.1f, 0.15f, 0.2f, 1.0f});
        Renderer::Clear();

        // 让立方体自转，便于观察 3D 效果与深度遮挡
        m_rotation += ts.GetSeconds() * 30.0f;  // 每秒转 30 度
        glm::mat4 transform =
            glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation), glm::vec3(0.5f, 1.0f, 0.0f));

        Renderer::BeginScene(m_cameraController.GetCamera());
        Renderer::Submit(m_shader, m_vertexArray, transform);
        Renderer::EndScene();
    }

    void OnEvent(Event& e) override {
        m_cameraController.OnEvent(e);
    }

private:
    std::shared_ptr<VertexArray> m_vertexArray;
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
