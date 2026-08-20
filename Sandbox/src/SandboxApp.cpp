// SandboxApp.cpp —— PBR + IBL + Skybox 测试
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Layer.h>
#include <Seed/Core/Log.h>
#include <Seed/Core/KeyCode.h>
#include <Seed/Core/TimeStep.h>
#include <Seed/Events/KeyEvent.h>
#include <Seed/Events/WindowEvent.h>
#include <Seed/RHI/Shader.h>
#include <Seed/Renderer/CameraController.h>
#include <Seed/Renderer/IBLBaker.h>
#include <Seed/Renderer/Light.h>
#include <Seed/Renderer/Model.h>
#include <Seed/Renderer/PBRMaterial.h>
#include <Seed/Renderer/Renderer.h>
#include <Seed/Renderer/Skybox.h>

namespace seed {

class ExampleLayer : public Layer {
public:
    ExampleLayer() : m_cameraController(1280.0f / 720.0f) {
    }

private:
    static constexpr float kSpacing = 2.5f;
    static constexpr const char* kSphereModelPath = "Sandbox/assets/models/Sphere/sphere.obj";

public:
    void OnAttach() override {
        // 球体是展示 PBR 最好的形状：法线连续变化，能一眼看到菲涅尔和高光的完整分布
        m_model = std::make_unique<Model>(kSphereModelPath);
        m_shader = Shader::Create("Sandbox/assets/shaders/PBR.glsl");

        // 五个球横排一行，z=9 刚好装下
        m_cameraController.GetCamera().SetPosition({0.0f, 0.0f, 9.0f});

        LoadMaterials();

        // IBL 预计算：环境图开到 2K 让 skybox 更锐利，
        // 辐照度/预过滤图保持默认（它们是卷积结果，调大只会变慢不会更准）
        m_iblSpec.EnvironmentResolution = 512;
        m_iblTextures =
            IBLBaker::BakeFromHDR("Sandbox/assets/hdri/cowboy_town_saloon_2k.hdr", m_iblSpec);
        m_useIBL = true;

        // skybox 直接采样环境 cubemap
        m_skybox = std::make_unique<Skybox>(m_iblTextures.EnvironmentMap);

        SEED_INFO("ExampleLayer 资源已就绪");
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

        m_shader->Bind();

        // 逐帧只需设一次的 uniform：光源、相机、IBL 贴图
        m_shader->SetFloat3("u_LightDirection", light.Direction);
        m_shader->SetFloat3("u_LightColor", light.Color);
        m_shader->SetFloat("u_LightIntensity", light.Intensity);
        m_shader->SetFloat3("u_ViewPos", m_cameraController.GetCamera().GetPosition());

        if (m_useIBL && m_iblTextures.IrradianceMap) {
            m_shader->SetInt("u_UseIBL", 1);
            m_iblTextures.IrradianceMap->Bind(5);
            m_iblTextures.PrefilterMap->Bind(6);
            m_iblTextures.BRDFLUT->Bind(7);
            m_shader->SetInt("u_IrradianceMap", 5);
            m_shader->SetInt("u_PrefilterMap", 6);
            m_shader->SetInt("u_BRDFLUT", 7);
            m_shader->SetFloat("u_PrefilterMaxLod", (float)(m_iblSpec.PrefilterMipLevels - 1));
        }
        else {
            m_shader->SetInt("u_UseIBL", 0);
        }
        m_shader->SetInt("u_DebugMode", m_debugMode);

        // 五个球横排，每个球绑一整套真实 PBR 贴图
        for (size_t i = 0; i < m_materials.size(); ++i) {
            m_materials[i].Bind(m_shader);
            glm::vec3 position{
                ((float)i - (m_materials.size() - 1) * 0.5f) * kSpacing,
                0.0f,
                0.0f,
            };
            m_model->Draw(m_shader, glm::translate(glm::mat4(1.0f), position));
        }

        // 天空盒最后画：深度函数用 GL_LEQUAL，被物体挡住的部分会被剔除，省下无用的片元着色
        if (m_skybox) {
            m_skybox->Draw(m_cameraController.GetCamera());
        }

        Renderer::EndScene();
    }

    void OnEvent(Event& e) override {
        m_cameraController.OnEvent(e);

        // 数字键 0-5 切换 IBL 调试通道
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& event) {
            int key = event.GetKeyCode();
            if (key >= Key::D0 && key <= Key::D5) {
                m_debugMode = key - Key::D0;
                static const char* names[] = {
                    "正常",       "prefilteredColor", "BRDF LUT",
                    "irradiance", "specular_IBL",     "prefilter(mip0)",
                };
                SEED_INFO("调试通道 {} = {}", m_debugMode, names[m_debugMode]);
            }
            return false;
        });
    }

private:
    // 各素材站的文件名前缀不统一，所以目录名和 basename 分开给
    void LoadMaterials() {
        struct Entry {
            const char* dir;
            const char* base;
        };
        static constexpr Entry kEntries[] = {
            {"light-gold", "lightgold"},
            {"titanium-scuffed", "Titanium-Scuffed"},
            {"painted-worn-concrete-bl", "painted-worn-concrete"},
            {"hungarian-point-flooring", "hungarian-point-flooring"},
            {"white-quilted-diamond-bl", "white-quilted-diamond"},
        };

        for (const Entry& e : kEntries) {
            std::string dir = std::string("Sandbox/assets/textures/") + e.dir;
            m_materials.push_back(PBRMaterial::LoadFromDirectory(dir, e.base));
        }
        SEED_INFO("已加载 {} 套 PBR 材质", m_materials.size());
    }

    std::unique_ptr<Model> m_model;
    std::shared_ptr<Shader> m_shader;
    CameraController m_cameraController;
    std::vector<PBRMaterial> m_materials;

    IBLSpecification m_iblSpec;
    IBLTextures m_iblTextures;
    int m_debugMode = 0;
    std::unique_ptr<Skybox> m_skybox;
    bool m_useIBL = false;
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
