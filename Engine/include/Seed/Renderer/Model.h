// Model.h —— 模型：递归加载 assimp 场景里的所有 Mesh
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "Seed/Renderer/Mesh.h"

// 只前向声明 assimp 类型，公开头不拖 assimp 依赖（同 GLFWWindow.h 前向声明 GLFWwindow 的做法）
struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace seed {

// 一个模型 = 多个 Mesh（对应 assimp 场景里的多个 aiMesh 节点）
class Model {
public:
    explicit Model(const std::string& path);

    void Draw(const std::shared_ptr<Shader>& shader, const glm::mat4& transform) const;

private:
    void LoadModel(const std::string& path);
    // parentTransform 沿节点层级累乘，把各节点的局部变换烘进顶点
    void ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform);
    // textureType 对应 aiTextureType，这里用 int 是为了不在头文件里 include assimp 的枚举
    std::vector<MeshTexture> LoadMaterialTextures(aiMaterial* material, int textureType,
                                                  const std::string& typeName);

    std::vector<Mesh> m_meshes;
    std::string m_directory;
    // 按贴图完整路径去重缓存，避免同一张贴图被不同 Mesh 重复上传到 GPU
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_loadedTextures;
};

}  // namespace seed
