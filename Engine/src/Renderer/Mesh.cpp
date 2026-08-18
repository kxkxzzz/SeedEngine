// Mesh.cpp
#include "Seed/Renderer/Mesh.h"

#include "Seed/RHI/Buffer.h"
#include "Seed/Renderer/Renderer.h"

namespace seed {

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
           std::vector<MeshTexture> textures)
    : m_textures(std::move(textures)) {
    auto vertexBuffer =
        VertexBuffer::Create(reinterpret_cast<float*>(vertices.data()),
                             static_cast<uint32_t>(vertices.size() * sizeof(Vertex)));
    vertexBuffer->SetLayout({
        {ShaderDataType::Float3, "a_Position"},
        {ShaderDataType::Float3, "a_Normal"},
        {ShaderDataType::Float2, "a_TexCoords"},
        {ShaderDataType::Float3, "a_Tangent"},
    });

    auto indexBuffer = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size()));

    m_vertexArray = VertexArray::Create();
    m_vertexArray->AddVertexBuffer(vertexBuffer);
    m_vertexArray->SetIndexBuffer(indexBuffer);
}

void Mesh::Draw(const std::shared_ptr<Shader>& shader, const glm::mat4& transform) const {
    shader->Bind();

    // 法线矩阵：非均匀缩放下法线不能直接乘 model 矩阵，否则不再垂直于表面
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
    shader->SetMat3("u_NormalMatrix", normalMatrix);

    // 材质与光源由调用方负责设置（见 PBRMaterial::Bind）：
    // Mesh 只管几何和变换，不绑定任何材质 uniform，
    // 这样同一份网格能配任意 shader（PBR / Blinn-Phong / 纯色调试）
    Renderer::Submit(shader, m_vertexArray, transform);
}


}  // namespace seed
