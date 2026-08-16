// Mesh.h —— 网格：一份顶点/索引/贴图数据 + 对应的 GPU 资源
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Seed/RHI/Shader.h"
#include "Seed/RHI/Texture.h"
#include "Seed/RHI/VertexArray.h"

namespace seed {

// 顶点属性：位置 + 法线 + UV
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
};

// 一张贴图及其用途（"diffuse"/"specular"...），Type 目前只用到 diffuse
struct MeshTexture {
    std::shared_ptr<Texture2D> TextureRef;
    std::string Type;
};

// 模型的一个子网格：一份独立的顶点/索引数据 + 用到的贴图
// 对应 assimp 里的一个 aiMesh，一个模型（Model）通常由多个 Mesh 组成
class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
         std::vector<MeshTexture> textures);

    // 绑定贴图后交给 Renderer::Submit 绘制
    void Draw(const std::shared_ptr<Shader>& shader, const glm::mat4& transform) const;

private:
    std::vector<MeshTexture> m_textures;
    std::shared_ptr<VertexArray> m_vertexArray;
};

}  // namespace seed
