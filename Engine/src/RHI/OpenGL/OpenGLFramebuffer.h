// OpenGLFramebuffer.h —— Framebuffer 的 OpenGL 实现
#pragma once

#include "Seed/RHI/Framebuffer.h"

namespace seed {

class OpenGLFramebuffer : public Framebuffer {
public:
    explicit OpenGLFramebuffer(const FramebufferSpecification& spec);
    ~OpenGLFramebuffer() override;

    void Bind() override;
    void Unbind() override;

    void Resize(uint32_t width, uint32_t height) override;

    uint32_t GetColorAttachmentRendererID() const override { return m_colorAttachment; }
    const FramebufferSpecification& GetSpecification() const override { return m_specification; }

private:
    void Invalidate();  // 创建/重建 FBO 及其附件

    uint32_t m_rendererID = 0;
    uint32_t m_colorAttachment = 0;
    uint32_t m_depthAttachment = 0;
    FramebufferSpecification m_specification;
};

}  // namespace seed
