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
        path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        SEED_CORE_ASSERT(false, "assimp 加载模型失败: {} ({})", path, importer.GetErrorString());
        return;
    }

    // 贴图路径要拼在模型文件所在目录下，而不是当前工作目录
    size_t lastSlash = path.find_last_of("/\\");
    m_directory = lastSlash == std::string::npos ? "" : path.substr(0, lastSlash);

    ProcessNode(scene->mRootNode, scene);
    SEED_CORE_INFO("模型加载完成: {} ({} 个 Mesh)", path, m_meshes.size());
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(ProcessMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        ProcessNode(node->mChildren[i], scene);
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        vertex.Normal = mesh->HasNormals()
                            ? glm::vec3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z}
                            : glm::vec3{0.0f, 0.0f, 0.0f};
        vertex.TexCoords = mesh->mTextureCoords[0]
                                ? glm::vec2{mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y}
                                : glm::vec2{0.0f, 0.0f};
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
