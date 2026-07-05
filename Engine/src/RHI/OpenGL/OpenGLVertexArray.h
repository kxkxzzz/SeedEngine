// OpenGLVertexArray.h —— VertexArray 的 OpenGL 实现
#pragma once

#include "Seed/RHI/VertexArray.h"

namespace seed {

class OpenGLVertexArray : public VertexArray {
public:
    OpenGLVertexArray();
    ~OpenGLVertexArray() override;

    void Bind() const override;
    void Unbind() const override;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;

    const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override {
        return m_vertexBuffers;
    }
    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const override {
        return m_indexBuffer;
    }

private:
    // VAO 句柄
    uint32_t m_rendererID = 0;
    // 下一个顶点属性的 location
    uint32_t m_vertexBufferIndex = 0;
    std::vector<std::shared_ptr<VertexBuffer>> m_vertexBuffers;
    std::shared_ptr<IndexBuffer> m_indexBuffer;
};

}  // namespace seed
