// Renderer.cpp
#include "Seed/Renderer/Renderer.h"

namespace seed {

Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData();
std::unique_ptr<RenderAPI> Renderer::s_renderAPI = nullptr;

void Renderer::Init() {
    s_renderAPI = RenderAPI::Create();
    s_renderAPI->Init();
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
    s_renderAPI->SetViewport(0, 0, width, height);
}

void Renderer::SetClearColor(const glm::vec4& color) {
    s_renderAPI->SetClearColor(color);
}

void Renderer::Clear() {
    s_renderAPI->Clear();
}

void Renderer::BeginScene(const PerspectiveCamera& camera) {
    s_sceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
    s_sceneData->CameraPosition = camera.GetPosition();
}

void Renderer::EndScene() {
    // 留空
}

void Renderer::SetDirectionalLight(const DirectionalLight& light) {
    s_sceneData->DirLight = light;
}

void Renderer::Submit(const std::shared_ptr<Shader>& shader,
                      const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
    shader->Bind();
    shader->SetMat4("u_ViewProjection", s_sceneData->ViewProjectionMatrix);
    shader->SetMat4("u_Transform", transform);
    shader->SetFloat3("u_CameraPosition", s_sceneData->CameraPosition);

    vertexArray->Bind();
    s_renderAPI->DrawIndexed(vertexArray);
}

}  // namespace seed
