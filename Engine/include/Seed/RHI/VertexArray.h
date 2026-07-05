// VertexArray.h —— 顶点数组对象
#pragma once

#include <memory>
#include <vector>

#include "Seed/RHI/Buffer.h"

namespace seed {

// VAO: 把若干 VertexBuffer 和一个 IndexBuffer 绑成一个可绘制单元
// 它记录了顶点数据的布局，绘制时只需 Bind 这一个对象
// 对应 OpenGL 的 VAO
// 对应 Vulkan 中 pipeline 的 vertex input state
class VertexArray {
public:
    virtual ~VertexArray() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    // 添加顶点缓冲，会按其 BufferLayout 配置顶点属性指针
    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) = 0;

    virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const = 0;
    virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;

    static std::shared_ptr<VertexArray> Create();
};

}  // namespace seed
