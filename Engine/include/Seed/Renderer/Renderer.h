// Renderer.h —— 渲染器：串联相机、物体变换与 RHI 绘制
#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "Seed/RHI/RenderAPI.h"
#include "Seed/RHI/Shader.h"
#include "Seed/RHI/VertexArray.h"
#include "Seed/Renderer/Light.h"
#include "Seed/Renderer/PerspectiveCamera.h"

namespace seed {

// 全引擎唯一的渲染器：BeginScene 记相机矩阵，Submit 提交物体绘制
// Renderer::Init() 在 Application 启动时调用一次，创建底层 RenderAPI
class Renderer {
public:
    static void Init();

    // 窗口尺寸变化时同步给 RenderAPI（视口）
    static void OnWindowResize(uint32_t width, uint32_t height);

    // 转发给底层 RenderAPI，Layer 不必直接持有 RenderAPI
    static void SetClearColor(const glm::vec4& color);
    static void Clear();

    static void BeginScene(const PerspectiveCamera& camera);
    static void EndScene();

    // 设置场景光源（在 BeginScene 前或后调用）
    static void SetDirectionalLight(const DirectionalLight& light);

    // 提交一个物体：绑定 shader，传入 VP 矩阵和物体自身的 transform，再绘制
    static void Submit(const std::shared_ptr<Shader>& shader,
                       const std::shared_ptr<VertexArray>& vertexArray,
                       const glm::mat4& transform = glm::mat4(1.0f));

    // 跨 BeginScene/Submit 共享的帧内数据
    struct SceneData {
        glm::mat4 ViewProjectionMatrix{1.0f};
        glm::vec3 CameraPosition{0.0f, 0.0f, 0.0f};
        DirectionalLight DirLight;
    };

    // 暴露给 Mesh::Draw 读取光源/相机
    static const SceneData* GetSceneData() { return s_sceneData; }

private:
    static SceneData* s_sceneData;
    static std::unique_ptr<RenderAPI> s_renderAPI;
};

}  // namespace seed
