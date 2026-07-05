// Framebuffer.h —— 帧缓冲
#pragma once

#include <cstdint>
#include <memory>

namespace seed {

struct FramebufferSpecification {
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t Samples = 1;  // 多重采样数，1 表示不开 MSAA
};

// 帧缓冲：把渲染结果画到纹理而非屏幕
// 编辑器把场景渲染进 Framebuffer，再作为纹理显示在 ImGui 的 Viewport 面板里
class Framebuffer {
public:
    virtual ~Framebuffer() = default;

    virtual void Bind() = 0;
    virtual void Unbind() = 0;

    // 窗口尺寸变化时重建附件
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    // 颜色附件的纹理 ID，供 ImGui::Image 显示
    virtual uint32_t GetColorAttachmentRendererID() const = 0;

    virtual const FramebufferSpecification& GetSpecification() const = 0;

    static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
};

}  // namespace seed
