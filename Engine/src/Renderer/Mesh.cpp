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

    // TODO 9 简化版：硬编码材质参数（后续改成从外部传入）
    glm::vec3 materialAmbient{0.2f};
    glm::vec3 materialDiffuse{0.8f};
    glm::vec3 materialSpecular{0.5f};
    float materialShininess = 32.0f;

    // 从 m_textures 找 diffuse/specular/normal 贴图
    const MeshTexture* diffuse = nullptr;
    const MeshTexture* specular = nullptr;
    const MeshTexture* normal = nullptr;
    for (const auto& texture : m_textures) {
        if (texture.Type == "diffuse")
            diffuse = &texture;
        else if (texture.Type == "specular")
            specular = &texture;
        else if (texture.Type == "normal")
            normal = &texture;
    }

    // 绑定贴图到对应槽位
    if (diffuse) {
        diffuse->TextureRef->Bind(0);
        shader->SetInt("u_Material_DiffuseMap", 0);
        shader->SetInt("u_Material_HasDiffuseMap", 1);
    }
    else {
        shader->SetInt("u_Material_HasDiffuseMap", 0);
    }

    if (specular) {
        specular->TextureRef->Bind(1);
        shader->SetInt("u_Material_SpecularMap", 1);
        shader->SetInt("u_Material_HasSpecularMap", 1);
    }
    else {
        shader->SetInt("u_Material_HasSpecularMap", 0);
    }

    if (normal) {
        normal->TextureRef->Bind(2);
        shader->SetInt("u_Material_NormalMap", 2);
        shader->SetInt("u_Material_HasNormalMap", 1);
    }
    else {
        shader->SetInt("u_Material_HasNormalMap", 0);
    }

    // 设置材质参数 uniform
    shader->SetFloat3("u_Material_Ambient", materialAmbient);
    shader->SetFloat3("u_Material_Diffuse", materialDiffuse);
    shader->SetFloat3("u_Material_Specular", materialSpecular);
    shader->SetFloat("u_Material_Shininess", materialShininess);

    // 从 Renderer::SceneData 读取光源和相机位置
    const auto* sceneData = Renderer::GetSceneData();
    shader->SetFloat3("u_DirLight_Direction", sceneData->DirLight.Direction);
    shader->SetFloat3("u_DirLight_Color", sceneData->DirLight.Color);
    shader->SetFloat("u_DirLight_Intensity", sceneData->DirLight.Intensity);
    shader->SetFloat3("u_ViewPos", sceneData->CameraPosition);

    // 计算并设置法线变换矩阵
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
    shader->SetMat3("u_NormalMatrix", normalMatrix);

    Renderer::Submit(shader, m_vertexArray, transform);
}

}  // namespace seed
