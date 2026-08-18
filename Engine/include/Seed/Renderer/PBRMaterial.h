// PBRMaterial.h —— PBR 材质定义（金属度/粗糙度工作流）
#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "Seed/RHI/Shader.h"
#include "Seed/RHI/Texture.h"

namespace seed {

// PBR 材质：基于物理的渲染，使用金属度/粗糙度工作流
struct PBRMaterial {
    // 基础颜色（漫反射颜色 / 金属基色）
    glm::vec3 Albedo{1.0f};
    float Metallic{0.0f};   // 金属度 [0,1]：0=电介质，1=金属
    float Roughness{0.5f};  // 粗糙度 [0,1]：0=完全光滑，1=完全粗糙
    float AO{1.0f};         // 环境光遮蔽 [0,1]

    // 贴图（可选，有贴图时覆盖对应的标量/向量值）
    std::shared_ptr<Texture2D> AlbedoMap;     // 基础颜色贴图（sRGB 空间）
    std::shared_ptr<Texture2D> NormalMap;     // 法线贴图（切线空间）
    std::shared_ptr<Texture2D> MetallicMap;   // 金属度贴图（线性空间）
    std::shared_ptr<Texture2D> RoughnessMap;  // 粗糙度贴图（线性空间）
    std::shared_ptr<Texture2D> AOMap;         // 环境光遮蔽贴图（线性空间）

    // 把五张贴图绑到 shader 的 slot 0-4，并设置对应的 u_Has*Map 开关。
    // 缺失的贴图会退回到上面的标量/向量值
    void Bind(const std::shared_ptr<Shader>& shader) const;

    // 从一个目录加载整套贴图。文件名后缀由调用方给出，因为各素材站命名不统一
    static PBRMaterial LoadFromDirectory(const std::string& directory,
                                        const std::string& basename);
};

}  // namespace seed
