// Material.h —— Blinn-Phong 材质定义
#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "Seed/RHI/Texture.h"

namespace seed {

// Blinn-Phong 材质：支持纯参数或贴图两种工作流
struct Material {
    // Blinn-Phong 三个分量
    glm::vec3 Ambient{0.2f};  // 环境光反射率
    glm::vec3 Diffuse{0.8f};  // 漫反射率（也可用 diffuse 贴图替代）
    glm::vec3 Specular{0.5f};  // 镜面反射率（也可用 specular 贴图替代）
    float Shininess{32.0f};  // 高光指数（越大越锐利）

    // 贴图（可选，有贴图时覆盖对应的 vec3 值）
    std::shared_ptr<Texture2D> DiffuseMap;  // 漫反射贴图
    std::shared_ptr<Texture2D> SpecularMap;  // 镜面反射贴图
    std::shared_ptr<Texture2D> NormalMap;  // 法线贴图（切线空间）
};

}  // namespace seed
