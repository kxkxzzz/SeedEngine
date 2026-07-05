// VertexArray.cpp —— VertexArray 工厂
#include "Seed/RHI/VertexArray.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLVertexArray.h"

namespace seed {

std::shared_ptr<VertexArray> VertexArray::Create() {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLVertexArray>();
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
