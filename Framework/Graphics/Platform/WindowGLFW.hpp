// WindowGLFW.hpp
#pragma once

#include "IWindow.hpp"

struct GLFWwindow;

namespace kx {
class WindowGLFW : public IWindow {
public:
    WindowGLFW();
    virtual ~WindowGLFW();

    // 创建&销毁 窗口
    virtual bool Create(const WindowCreateInfo& createInfo) override;
    virtual void Destroy() override;

    // 事件处理
    virtual void PollEvents() override;
    virtual bool ShouldClose() override;

    // 获取窗口属性
    virtual void* GetNativeWindow() const override;
    virtual uint32_t GetWidth() const override;
    virtual uint32_t GetHeight() const override;
    virtual std::string_view GetTitle() const override;
    virtual bool IsResizable() const override;
    virtual bool IsFullscreen() const override;

    // 设置窗口属性
    virtual void SetTitle(std::string_view title) override;
    virtual void SetSize(uint32_t width, uint32_t height) override;
    virtual void SetFullscreen(bool fullscreen) override;

private:
    GLFWwindow* m_window = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_fullscreen = false;
    bool m_initialized = false;

    // 内部辅助函数
    void SetupWindowHints(const WindowCreateInfo& createInfo);
    void SetupCallbacks();
    static void OnWindowResize(GLFWwindow* window, int width, int height);
    static void OnWindowClose(GLFWwindow* window);
};
}  // namespace kx