// OpenGLBuffer.cpp
#include "RHI/OpenGL/OpenGLBuffer.h"

#include <glad/gl.h>

namespace seed {

// ============ OpenGLVertexBuffer ============

OpenGLVertexBuffer::OpenGLVertexBuffer(const float* vertices, uint32_t size) {
    glCreateBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

// Dynamic draw
OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size) {
    glCreateBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
    glDeleteBuffers(1, &m_rendererID);
}

void OpenGLVertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void OpenGLVertexBuffer::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLVertexBuffer::SetData(const void* data, uint32_t size) {
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

// ============ OpenGLIndexBuffer ============

OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t* indices, uint32_t count) : m_count(count) {
    glCreateBuffers(1, &m_rendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
    glDeleteBuffers(1, &m_rendererID);
}

void OpenGLIndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID);
}

void OpenGLIndexBuffer::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}  // namespace seed
