// Texture.cpp —— Texture2D
#include "Seed/RHI/Texture.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace seed {

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTexture2D>(path);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTexture2D>(width, height);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
