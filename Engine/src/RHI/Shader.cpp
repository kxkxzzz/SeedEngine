// Shader.cpp —— Shader
#include "Seed/RHI/Shader.h"

#include "Seed/Core/Assert.h"
#include "Seed/RHI/RenderAPI.h"
#include "RHI/OpenGL/OpenGLShader.h"

namespace seed {

std::shared_ptr<Shader> Shader::Create(const std::string& filepath) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLShader>(filepath);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

std::shared_ptr<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc,
                                       const std::string& fragmentSrc) {
    switch (RenderAPI::GetAPI()) {
        case RendererAPIType::OpenGL:
            return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
        default:
            SEED_CORE_ASSERT(false, "暂不支持的 RendererAPIType");
            return nullptr;
    }
}

}  // namespace seed
