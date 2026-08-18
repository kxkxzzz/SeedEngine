// Texture.h —— 纹理
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace seed {

// 纹理内部格式（决定 GPU 如何存储和解释数据）
enum class TextureFormat {
    None = 0,
    RGB8,         // 8-bit LDR，线性空间（用于 metallic/roughness/ao）
    RGBA8,        // 8-bit LDR，线性空间
    SRGB8,        // 8-bit LDR，sRGB 空间（用于 albedo）
    SRGBA8,       // 8-bit LDR，sRGB 空间
    RGB16F,       // 16-bit HDR，线性空间（用于环境贴图）
    RGBA16F,      // 16-bit HDR，线性空间
};

// 纹理基类
class Texture {
public:
    virtual ~Texture() = default;

    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;

    // 绑定到指定纹理槽，供着色器采样
    virtual void Bind(uint32_t slot = 0) const = 0;
};

// 2D 纹理
class Texture2D : public Texture {
public:
    // 动态更新纹理像素数据
    virtual void SetData(void* data, uint32_t size) = 0;

    // 工厂：从图片文件加载（自动检测 sRGB/线性）
    static std::shared_ptr<Texture2D> Create(const std::string& path, TextureFormat format = TextureFormat::None);
    // 工厂：创建空白纹理
    static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8);
};

// 立方体贴图（天空盒 + IBL 环境贴图）
class TextureCube : public Texture {
public:
    uint32_t GetWidth() const override { return 0; }   // Cubemap 没有单一宽高概念
    uint32_t GetHeight() const override { return 0; }

    // 工厂：从 6 张图片加载（+X, -X, +Y, -Y, +Z, -Z）
    static std::shared_ptr<TextureCube> Create(const std::vector<std::string>& faces);
    // 工厂：从单张 equirectangular HDR 图转换（PBR 环境贴图常用这种格式）
    static std::shared_ptr<TextureCube> CreateFromEquirectangular(const std::string& path);
    // 工厂：创建空 cubemap（用于预计算）
    static std::shared_ptr<TextureCube> CreateEmpty(uint32_t resolution, bool isHDR, bool generateMipmaps = false);
};

}  // namespace seed
