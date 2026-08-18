// Model.cpp
#include "Seed/Renderer/Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Seed/Core/Assert.h"
#include "Seed/Core/Log.h"

namespace seed {

Model::Model(const std::string& path) {
    LoadModel(path);
}

void Model::Draw(const std::shared_ptr<Shader>& shader, const glm::mat4& transform) const {
    for (const auto& mesh : m_meshes)
        mesh.Draw(shader, transform);
}

void Model::LoadModel(const std::string& path) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
              | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SEED_CORE_ASSERT(false, "assimp 加载模型失败: {} ({})", path, importer.GetErrorString());
        return;
    }

    // 贴图路径要拼在模型文件所在目录下，而不是当前工作目录
    size_t lastSlash = path.find_last_of("/\\");
    m_directory = lastSlash == std::string::npos ? "" : path.substr(0, lastSlash);

    ProcessNode(scene->mRootNode, scene, glm::mat4(1.0f));
    SEED_CORE_INFO("模型加载完成: {} ({} 个 Mesh)", path, m_meshes.size());
}

// assimp 的 aiMatrix4x4 是行主序，glm::mat4 是列主序，转置一次
static glm::mat4 ToGlmMat4(const aiMatrix4x4& m) {
    return glm::mat4{
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4,
    };
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform) {
    // 节点变换沿层级累乘：子节点的世界变换 = 父节点世界变换 × 自身局部变换
    glm::mat4 nodeTransform = parentTransform * ToGlmMat4(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(mesh, scene, nodeTransform));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        ProcessNode(node->mChildren[i], scene, nodeTransform);
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform) {
    // 法线的正确变换是模型矩阵的逆转置（非均匀缩放下才不会偏斜）
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        // 顶点位置烘进节点变换（w=1，平移生效）
        glm::vec3 localPos{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.Position = glm::vec3(transform * glm::vec4(localPos, 1.0f));

        // 法线/切线是方向量，用逆转置矩阵变换且不受平移影响
        vertex.Normal =
            mesh->HasNormals()
                ? glm::normalize(normalMatrix * glm::vec3{mesh->mNormals[i].x, mesh->mNormals[i].y,
                                                          mesh->mNormals[i].z})
                : glm::vec3{0.0f, 0.0f, 0.0f};
        vertex.TexCoords = mesh->mTextureCoords[0]
                                ? glm::vec2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}
                                : glm::vec2{0.0f, 0.0f};
        vertex.Tangent =
            mesh->HasTangentsAndBitangents()
                ? glm::normalize(normalMatrix * glm::vec3{mesh->mTangents[i].x, mesh->mTangents[i].y,
                                                          mesh->mTangents[i].z})
                : glm::vec3{0.0f, 0.0f, 0.0f};
        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    // assimp 总会给一个有效的材质下标（哪怕模型没显式指定），直接取即可
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // 传统 OBJ/FBX 材质用 DIFFUSE，glTF 的 PBR 材质用 BASE_COLOR，两个都试一下
    std::vector<MeshTexture> textures = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
    if (textures.empty())
        textures = LoadMaterialTextures(material, aiTextureType_BASE_COLOR, "diffuse");

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<MeshTexture> Model::LoadMaterialTextures(aiMaterial* material, int textureType,
                                                     const std::string& typeName) {
    std::vector<MeshTexture> textures;
    auto type = static_cast<aiTextureType>(textureType);
    for (unsigned int i = 0; i < material->GetTextureCount(type); ++i) {
        aiString str;
        material->GetTexture(type, i, &str);

        std::string path = m_directory + "/" + str.C_Str();
        auto it = m_loadedTextures.find(path);
        if (it != m_loadedTextures.end()) {
            textures.push_back({it->second, typeName});
            continue;
        }

        auto texture = Texture2D::Create(path);
        m_loadedTextures[path] = texture;
        textures.push_back({texture, typeName});
    }
    return textures;
}

}  // namespace seed
