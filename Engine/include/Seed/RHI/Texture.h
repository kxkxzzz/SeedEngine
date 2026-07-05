// Texture.h —— 纹理
#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace seed {

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

    // 工厂：从图片文件加载
    static std::shared_ptr<Texture2D> Create(const std::string& path);
    // 工厂：创建空白纹理（之后用 SetData 填，常用于纯色或字体图集）
    static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height);
};

}  // namespace seed
