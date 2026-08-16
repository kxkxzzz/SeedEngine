// OpenGLTexture.h —— Texture2D 的 OpenGL 实现
#pragma once

#include <cstdint>
#include <string>

#include "Seed/RHI/Texture.h"

namespace seed {

class OpenGLTexture2D : public Texture2D {
public:
    explicit OpenGLTexture2D(const std::string& path);
    OpenGLTexture2D(uint32_t width, uint32_t height);
    ~OpenGLTexture2D() override;

    uint32_t GetWidth() const override {
        return m_width;
    }
    uint32_t GetHeight() const override {
        return m_height;
    }

    void Bind(uint32_t slot = 0) const override;
    void SetData(void* data, uint32_t size) override;

private:
    uint32_t m_rendererID = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_internalFormat = 0;  // 存储格式，如 GL_RGB8/GL_RGBA8
    uint32_t m_dataFormat = 0;      // 上传数据格式，如 GL_RGB/GL_RGBA
};

}  // namespace seed
