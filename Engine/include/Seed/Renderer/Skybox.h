// Skybox.h —— 天空盒渲染器
#pragma once

#include <memory>

#include "Seed/RHI/Shader.h"
#include "Seed/RHI/Texture.h"
#include "Seed/RHI/VertexArray.h"
#include "Seed/Renderer/PerspectiveCamera.h"

namespace seed {

// 天空盒：渲染环境 cubemap 作为背景
class Skybox {
public:
    explicit Skybox(const std::shared_ptr<TextureCube>& cubemap);

    // 渲染天空盒（在所有不透明物体之后调用）
    void Draw(const PerspectiveCamera& camera);

private:
    std::shared_ptr<TextureCube> m_cubemap;
    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<VertexArray> m_vao;
};

}  // namespace seed
