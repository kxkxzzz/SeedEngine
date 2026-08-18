// Skybox.cpp
#include "Seed/Renderer/Skybox.h"

#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include "Seed/RHI/Buffer.h"

namespace seed {

// 立方体顶点（天空盒用）
static float s_skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

Skybox::Skybox(const std::shared_ptr<TextureCube>& cubemap) : m_cubemap(cubemap) {
    m_shader = Shader::Create("Sandbox/assets/shaders/Skybox.glsl");

    // 创建立方体 VAO
    auto vbo = VertexBuffer::Create(s_skyboxVertices, sizeof(s_skyboxVertices));
    vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
    m_vao = VertexArray::Create();
    m_vao->AddVertexBuffer(vbo);
}

void Skybox::Draw(const PerspectiveCamera& camera) {
    // 禁用深度写入，让天空盒永远在最远处
    glDepthFunc(GL_LEQUAL);

    m_shader->Bind();

    // 移除 view 矩阵的平移分量（只保留旋转）
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    glm::mat4 viewProjection = camera.GetProjectionMatrix() * view;

    m_shader->SetMat4("u_ViewProjection", viewProjection);
    m_shader->SetInt("u_Skybox", 0);

    m_cubemap->Bind(0);
    m_vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 恢复深度测试
    glDepthFunc(GL_LESS);
}

}  // namespace seed
