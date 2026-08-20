// OpenGLTexture.h —— Texture 的 OpenGL 实现
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Seed/RHI/Texture.h"

namespace seed {

class OpenGLTexture2D : public Texture2D {
public:
    OpenGLTexture2D(const std::string& path, TextureFormat format);
    OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format);
    ~OpenGLTexture2D() override;

    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }

    void Bind(uint32_t slot = 0) const override;
    void SetData(void* data, uint32_t size) override;

    // 供 IBL 预计算把纹理挂到 FBO 附件上使用
    uint32_t GetRendererID() const { return m_rendererID; }

private:
    uint32_t m_rendererID = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_internalFormat = 0;
    uint32_t m_dataFormat = 0;
    uint32_t m_dataType = 0;  // GL_UNSIGNED_BYTE 或 GL_FLOAT
    uint32_t m_mipLevels = 1;  // 1 表示无 mip 链（HDR 环境图、离屏渲染目标）
};

class OpenGLTextureCube : public TextureCube {
public:
    explicit OpenGLTextureCube(const std::vector<std::string>& faces);
    explicit OpenGLTextureCube(const std::string& equirectangularPath);
    // 创建空 HDR cubemap（用于预计算）
    OpenGLTextureCube(uint32_t resolution, bool isHDR, bool generateMipmaps = false);
    ~OpenGLTextureCube() override;

    void Bind(uint32_t slot = 0) const override;

    uint32_t GetRendererID() const { return m_rendererID; }

private:
    void LoadFromFaces(const std::vector<std::string>& faces);
    void LoadFromEquirectangular(const std::string& path);

    uint32_t m_rendererID = 0;
};

}  // namespace seed
