// RenderAPI.cpp —— 后端选择
#include "Seed/RHI/RenderAPI.h"

#include "Seed/Core/Assert.h"
#include "RHI/OpenGL/OpenGLRenderAPI.h"

namespace seed {

// 全局默认后端：当前固定 OpenGL
RendererAPIType RenderAPI::s_api = RendererAPIType::OpenGL;

std::unique_ptr<RenderAPI> RenderAPI::Create() {
    switch (s_api) {
        case RendererAPIType::OpenGL:
            return std::make_unique<OpenGLRenderAPI>();
        case RendererAPIType::None:
        case RendererAPIType::Vulkan:
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
