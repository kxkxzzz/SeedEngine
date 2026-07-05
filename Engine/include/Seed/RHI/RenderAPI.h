// RenderAPI.h —— 渲染命令抽象接口
#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

#include "Seed/RHI/RendererAPIType.h"
#include "Seed/RHI/VertexArray.h"

namespace seed {

// 封装清屏、视口、绘制等最底层操作，OpenGL/Vulkan 各自实现一份
// 同时持有"当前用哪个后端"的全局状态，供所有资源的 Create() 工厂查询
class RenderAPI {
public:
    virtual ~RenderAPI() = default;

    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;

    // 按索引绘制一个 VertexArray，indexCount=0 表示用其 IndexBuffer 的全部索引
    virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray,
                             uint32_t indexCount = 0) = 0;

    // 当前后端类型，所有资源工厂据此选择实现
    static RendererAPIType GetAPI() {
        return s_api;
    }

    // 创建当前后端的 RenderAPI 实例
    static std::unique_ptr<RenderAPI> Create();

private:
    static RendererAPIType s_api;
};

}  // namespace seed
