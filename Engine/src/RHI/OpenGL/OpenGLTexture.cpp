// OpenGLTexture.cpp
#include "RHI/OpenGL/OpenGLTexture.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "Seed/Core/Assert.h"
#include "Seed/Core/Log.h"

namespace seed {

OpenGLTexture2D::OpenGLTexture2D(const std::string& path) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    SEED_CORE_ASSERT(data, "贴图加载失败: {}", path);
    if (!data) {
        SEED_CORE_ERROR("贴图加载失败: {}", path);
        return;
    }

    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);

    if (channels == 4) {
        m_internalFormat = GL_RGBA8;
        m_dataFormat = GL_RGBA;
    }
    else if (channels == 3) {
        m_internalFormat = GL_RGB8;
        m_dataFormat = GL_RGB;
    }
    else {
        SEED_CORE_ASSERT(false, "不支持的贴图通道数: {}", channels);
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, 1, m_internalFormat, width, height);

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureSubImage2D(m_rendererID, 0, 0, 0, width, height, m_dataFormat, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(m_rendererID);

    stbi_image_free(data);
}

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
    : m_width(width), m_height(height), m_internalFormat(GL_RGBA8), m_dataFormat(GL_RGBA) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, 1, m_internalFormat, width, height);

    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

OpenGLTexture2D::~OpenGLTexture2D() {
    glDeleteTextures(1, &m_rendererID);
}

void OpenGLTexture2D::Bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_rendererID);
}

void OpenGLTexture2D::SetData(void* data, uint32_t size) {
    uint32_t bytesPerPixel = m_dataFormat == GL_RGBA ? 4 : 3;
    SEED_CORE_ASSERT(size == m_width * m_height * bytesPerPixel, "SetData 的数据大小与贴图尺寸不匹配");
    glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);
}

}  // namespace seed
