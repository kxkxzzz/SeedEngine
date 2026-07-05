// SandboxApp.cpp —— 客户端测试程序
#include <memory>

#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Layer.h>
#include <Seed/Core/Log.h>
#include <Seed/Core/TimeStep.h>
#include <Seed/RHI/Buffer.h>
#include <Seed/RHI/RenderAPI.h>
#include <Seed/RHI/Shader.h>
#include <Seed/RHI/VertexArray.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    void OnAttach() override {
        // 初始化渲染 API（开混合/深度测试）
        m_renderAPI = RenderAPI::Create();
        m_renderAPI->Init();

        // 1. 创建 VertexArray（VAO）
        m_vertexArray = VertexArray::Create();

        // 2. 三角形顶点数据：每个顶点 = 位置(x,y,z) + 颜色(r,g,b)
        float vertices[3 * 6] = {
            // 位置              // 颜色
            -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // 左下 红
             0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // 右下 绿
             0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // 顶部 蓝
        };

        // 3. 创建 VertexBuffer 并声明布局
        auto vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Color" },
        });

        // 4. 把 VertexBuffer 加到 VertexArray（按 layout 配置属性指针）
        m_vertexArray->AddVertexBuffer(vertexBuffer);

        // 5. 索引缓冲：三个顶点组成一个三角形
        uint32_t indices[3] = { 0, 1, 2 };
        auto indexBuffer = IndexBuffer::Create(indices, 3);
        m_vertexArray->SetIndexBuffer(indexBuffer);

        // 6. 着色器：顶点传递位置和颜色，片段直接输出插值后的颜色
        std::string vertexSrc = R"(
            #version 450 core
            layout(location = 0) in vec3 a_Position;
            layout(location = 1) in vec3 a_Color;

            out vec3 v_Color;

            void main() {
                v_Color = a_Color;
                gl_Position = vec4(a_Position, 1.0);
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

        m_shader = Shader::Create("Triangle", vertexSrc, fragmentSrc);

        SEED_INFO("ExampleLayer 三角形资源已就绪");
    }

    void OnUpdate(Timestep ts) override {
        m_renderAPI->SetClearColor({ 0.1f, 0.15f, 0.2f, 1.0f });
        m_renderAPI->Clear();

        m_shader->Bind();
        m_vertexArray->Bind();
        m_renderAPI->DrawIndexed(m_vertexArray);
    }

    void OnEvent(Event& e) override {}

private:
    std::unique_ptr<RenderAPI> m_renderAPI;
    std::shared_ptr<VertexArray> m_vertexArray;
    std::shared_ptr<Shader> m_shader;
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
