// OpenGLTexture.cpp
#include "RHI/OpenGL/OpenGLTexture.h"

#include <glad/gl.h>
#include <stb_image.h>

#include "Seed/Core/Assert.h"
#include "Seed/Core/Log.h"

namespace seed {

OpenGLTexture2D::OpenGLTexture2D(const std::string& path, TextureFormat format) {
    int width, height, channels;
    stbi_uc* data = nullptr;
    float* hdrData = nullptr;

    // 检测是否是 HDR 格式（.hdr/.exr）
    bool isHDR = path.find(".hdr") != std::string::npos || path.find(".exr") != std::string::npos;

    if (isHDR) {
        // HDR 环境图按 equirectangular 映射采样，需要翻转成 OpenGL 的下-上行序
        stbi_set_flip_vertically_on_load(1);
        hdrData = stbi_loadf(path.c_str(), &width, &height, &channels, 0);
        SEED_CORE_ASSERT(hdrData, "HDR 贴图加载失败: {}", path);
        if (!hdrData) {
            SEED_CORE_ERROR("HDR 贴图加载失败: {}", path);
            return;
        }
        m_dataType = GL_FLOAT;
    }
    else {
        // 模型贴图不在这里翻转：assimp 的 aiProcess_FlipUVs 已经翻过 UV 一次，
        // 这里再翻会互相抵消，等于没翻
        stbi_set_flip_vertically_on_load(0);
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        SEED_CORE_ASSERT(data, "贴图加载失败: {}", path);
        if (!data) {
            SEED_CORE_ERROR("贴图加载失败: {}", path);
            return;
        }
        m_dataType = GL_UNSIGNED_BYTE;
    }

    m_width = static_cast<uint32_t>(width);
    m_height = static_cast<uint32_t>(height);

    // 根据 format 参数或自动推断设置内部格式
    if (format == TextureFormat::None) {
        // 自动推断：HDR 用 RGB16F，LDR 根据通道数决定
        if (isHDR) {
            m_internalFormat = (channels == 4) ? GL_RGBA16F : GL_RGB16F;
        }
        else {
            // LDR 默认假设是线性空间（metallic/roughness/normal），除非显式指定 sRGB
            m_internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;
        }
    }
    else {
        // 显式指定格式
        switch (format) {
            case TextureFormat::RGB8:      m_internalFormat = GL_RGB8; break;
            case TextureFormat::RGBA8:     m_internalFormat = GL_RGBA8; break;
            case TextureFormat::SRGB8:     m_internalFormat = GL_SRGB8; break;
            case TextureFormat::SRGBA8:    m_internalFormat = GL_SRGB8_ALPHA8; break;
            case TextureFormat::RGB16F:    m_internalFormat = GL_RGB16F; break;
            case TextureFormat::RGBA16F:   m_internalFormat = GL_RGBA16F; break;
            default: SEED_CORE_ASSERT(false, "未知的 TextureFormat");
        }
    }

    m_dataFormat = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;

    // HDR 环境图是 equirectangular 全景，只被 IBL 烘焙一次性采样，不需要 mip 链；
    // 材质贴图会以任意缩放贴在网格上，缺 mipmap 会在缩小处严重走样
    m_mipLevels = 1;
    if (!isHDR) {
        uint32_t size = (m_width > m_height) ? m_width : m_height;
        while (size > 1) {
            size >>= 1;
            ++m_mipLevels;
        }
    }

    glCreateTextures(GL_TEXTURE_2D, 1, &m_rendererID);
    glTextureStorage2D(m_rendererID, m_mipLevels, m_internalFormat, m_width, m_height);

    // 缩小时用三线性过滤（mip 内 + mip 间都插值），放大时双线性
    glTextureParameteri(m_rendererID, GL_TEXTURE_MIN_FILTER,
                        m_mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_rendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (isHDR) {
        glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_FLOAT, hdrData);
        stbi_image_free(hdrData);
    }
    else {
        glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    if (m_mipLevels > 1) {
        // mip 链要在像素上传之后生成，否则下采样的是未初始化的内容。
        // sRGB 纹理的下采样由驱动在线性空间做，不会有 gamma 偏差
        glGenerateTextureMipmap(m_rendererID);

        // 各向异性过滤：球面/地板这类以掠射角看过去的表面，各向同性的三线性
        // 会沿压缩方向过度模糊，AF 沿该方向多取几个样本，兼顾清晰与抗闪
        GLfloat maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
        if (maxAniso > 1.0f)
            glTextureParameterf(m_rendererID, GL_TEXTURE_MAX_ANISOTROPY,
                                maxAniso < 8.0f ? maxAniso : 8.0f);
    }
}

OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format)
    : m_width(width), m_height(height) {

    switch (format) {
        case TextureFormat::RGB8:      m_internalFormat = GL_RGB8; m_dataFormat = GL_RGB; break;
        case TextureFormat::RGBA8:     m_internalFormat = GL_RGBA8; m_dataFormat = GL_RGBA; break;
        case TextureFormat::SRGB8:     m_internalFormat = GL_SRGB8; m_dataFormat = GL_RGB; break;
        case TextureFormat::SRGBA8:    m_internalFormat = GL_SRGB8_ALPHA8; m_dataFormat = GL_RGBA; break;
        case TextureFormat::RGB16F:    m_internalFormat = GL_RGB16F; m_dataFormat = GL_RGB; break;
        case TextureFormat::RGBA16F:   m_internalFormat = GL_RGBA16F; m_dataFormat = GL_RGBA; break;
        default: m_internalFormat = GL_RGBA8; m_dataFormat = GL_RGBA;
    }

    m_dataType = (format == TextureFormat::RGB16F || format == TextureFormat::RGBA16F)
                      ? GL_FLOAT : GL_UNSIGNED_BYTE;

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
    if (m_dataType == GL_FLOAT) {
        bytesPerPixel *= 4;  // float 是 4 字节
    }
    SEED_CORE_ASSERT(size == m_width * m_height * bytesPerPixel, "SetData 的数据大小与贴图尺寸不匹配");
    glTextureSubImage2D(m_rendererID, 0, 0, 0, m_width, m_height, m_dataFormat, m_dataType, data);

    // 只写了 mip 0，有 mip 链时其余各级会残留旧内容，缩小采样时就会采到过期像素
    if (m_mipLevels > 1)
        glGenerateTextureMipmap(m_rendererID);
}

// ========== TextureCube ==========

OpenGLTextureCube::OpenGLTextureCube(const std::vector<std::string>& faces) {
    LoadFromFaces(faces);
}

OpenGLTextureCube::OpenGLTextureCube(const std::string& equirectangularPath) {
    LoadFromEquirectangular(equirectangularPath);
}

OpenGLTextureCube::OpenGLTextureCube(uint32_t resolution, bool isHDR, bool generateMipmaps) {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);

    GLenum internalFormat = isHDR ? GL_RGB16F : GL_RGB8;
    GLenum format = GL_RGB;
    GLenum type = isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;

    // 需要 mipmap 时先算出层级数，为每一级都分配存储。
    // 注意不能靠 glGenerateMipmap 来"顺便分配"——那时内容还是空的，
    // 而且后续预过滤是逐级手动渲染写入，只需要存储就位，不需要 GL 帮我们下采样
    uint32_t mipCount = 1;
    if (generateMipmaps) {
        uint32_t size = resolution;
        while (size > 1) {
            size >>= 1;
            ++mipCount;
        }
    }

    for (uint32_t mip = 0; mip < mipCount; ++mip) {
        uint32_t mipSize = resolution >> mip;
        for (uint32_t i = 0; i < 6; ++i) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, internalFormat, mipSize, mipSize,
                         0, format, type, nullptr);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    generateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
}

OpenGLTextureCube::~OpenGLTextureCube() {
    glDeleteTextures(1, &m_rendererID);
}

void OpenGLTextureCube::Bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_rendererID);
}

void OpenGLTextureCube::LoadFromFaces(const std::vector<std::string>& faces) {
    SEED_CORE_ASSERT(faces.size() == 6, "TextureCube 需要 6 张贴图");

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_rendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_rendererID);

    int width, height, channels;
    for (uint32_t i = 0; i < 6; ++i) {
        stbi_uc* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
        if (!data) {
            SEED_CORE_ERROR("TextureCube 加载失败: {}", faces[i]);
            continue;
        }

        GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void OpenGLTextureCube::LoadFromEquirectangular(const std::string& path) {
    // TODO: 从 equirectangular HDR 转 cubemap 需要离屏渲染 + shader
    // 暂时留空，后续在 IBL 预计算管线里实现
    SEED_CORE_ASSERT(false, "LoadFromEquirectangular 暂未实现，请先用 6 张面加载");
}

}  // namespace seed
