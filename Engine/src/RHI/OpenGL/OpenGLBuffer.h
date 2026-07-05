// OpenGLBuffer.h —— VertexBuffer / IndexBuffer 的 OpenGL 实现
#pragma once

#include "Seed/RHI/Buffer.h"

namespace seed {

class OpenGLVertexBuffer : public VertexBuffer {
public:
    OpenGLVertexBuffer(const float* vertices, uint32_t size);
    explicit OpenGLVertexBuffer(uint32_t size);
    ~OpenGLVertexBuffer() override;

    void Bind() const override;
    void Unbind() const override;
    void SetData(const void* data, uint32_t size) override;

    const BufferLayout& GetLayout() const override {
        return m_layout;
    }
    void SetLayout(const BufferLayout& layout) override {
        m_layout = layout;
    }

private:
    uint32_t m_rendererID = 0;  // OpenGL 缓冲对象句柄
    BufferLayout m_layout;
};

class OpenGLIndexBuffer : public IndexBuffer {
public:
    OpenGLIndexBuffer(const uint32_t* indices, uint32_t count);
    ~OpenGLIndexBuffer() override;

    void Bind() const override;
    void Unbind() const override;

    uint32_t GetCount() const override {
        return m_count;
    }

private:
    uint32_t m_rendererID = 0;
    uint32_t m_count = 0;
};

}  // namespace seed
