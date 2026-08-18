// Texture.cpp —— Texture2D & TextureCube 工厂
#include "Seed/RHI/Texture.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLTexture.h"

namespace seed {

std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path, TextureFormat format) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTexture2D>(path, format);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, TextureFormat format) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTexture2D>(width, height, format);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<TextureCube> TextureCube::Create(const std::vector<std::string>& faces) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTextureCube>(faces);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<TextureCube> TextureCube::CreateFromEquirectangular(const std::string& path) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTextureCube>(path);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<TextureCube> TextureCube::CreateEmpty(uint32_t resolution, bool isHDR, bool generateMipmaps) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLTextureCube>(resolution, isHDR, generateMipmaps);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
