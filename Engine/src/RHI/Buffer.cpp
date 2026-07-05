// Buffer.cpp —— VertexBuffer / IndexBuffer
#include "Seed/RHI/Buffer.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLBuffer.h"

namespace seed {

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const float* vertices, uint32_t size) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(vertices, size);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLVertexBuffer>(size);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLIndexBuffer>(indices, count);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
