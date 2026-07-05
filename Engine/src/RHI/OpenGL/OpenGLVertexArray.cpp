// OpenGLVertexArray.cpp
#include "RHI/OpenGL/OpenGLVertexArray.h"

#include <glad/gl.h>

#include "Seed/Core/Assert.h"

namespace seed {

// 把引擎的 ShaderDataType 映射到 OpenGL 的基础类型枚举
// glVertexAttribPointer 需要这个 GL 类型（如 GL_FLOAT）
static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
            return GL_FLOAT;
        case ShaderDataType::Float2:
            return GL_FLOAT;
        case ShaderDataType::Float3:
            return GL_FLOAT;
        case ShaderDataType::Float4:
            return GL_FLOAT;
        case ShaderDataType::Mat3:
            return GL_FLOAT;
        case ShaderDataType::Mat4:
            return GL_FLOAT;
        case ShaderDataType::Int:
            return GL_INT;
        case ShaderDataType::Int2:
            return GL_INT;
        case ShaderDataType::Int3:
            return GL_INT;
        case ShaderDataType::Int4:
            return GL_INT;
        case ShaderDataType::Bool:
            return GL_BOOL;
        default:
            return 0;
    }
}

OpenGLVertexArray::OpenGLVertexArray() {
    glCreateVertexArrays(1, &m_rendererID);
}

OpenGLVertexArray::~OpenGLVertexArray() {
    glDeleteVertexArrays(1, &m_rendererID);
}

void OpenGLVertexArray::Bind() const {
    glBindVertexArray(m_rendererID);
}

void OpenGLVertexArray::Unbind() const {
    glBindVertexArray(0);
}

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
    // 顶点缓冲必须有布局，否则不知道数据怎么解释
    SEED_CORE_ASSERT(!vertexBuffer->GetLayout().GetElements().empty(), "VertexBuffer 没有 Layout");

    glBindVertexArray(m_rendererID);
    vertexBuffer->Bind();
    for (const auto& element : vertexBuffer->GetLayout().GetElements()) {
        glEnableVertexAttribArray(m_vertexBufferIndex);
        glVertexAttribPointer(
            m_vertexBufferIndex, element.GetComponentCount(),
            ShaderDataTypeToOpenGLBaseType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE,
            vertexBuffer->GetLayout().GetStride(), (const void*)(uintptr_t)element.Offset);
        ++m_vertexBufferIndex;
    }
    m_vertexBuffers.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
    glBindVertexArray(m_rendererID);
    indexBuffer->Bind();
    m_indexBuffer = indexBuffer;
}

}  // namespace seed
