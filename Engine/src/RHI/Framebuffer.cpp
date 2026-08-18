// Framebuffer.cpp —— 帧缓冲工厂
#include "Seed/RHI/Framebuffer.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLFramebuffer.h"

namespace seed {

std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLFramebuffer>(spec);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
