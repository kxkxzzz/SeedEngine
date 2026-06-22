// GLFWWindow.h —— Window 的 GLFW 实现（引擎内部私有头，不对客户端暴露）
#pragma once

#include <string>

#include "Seed/Core/Window.h"

struct GLFWwindow;

namespace seed {

class GLFWWindow : public Window {
public:
    explicit GLFWWindow(const WindowCreateInfo& info);
    ~GLFWWindow() override;

    void PollEvents() override;
    bool ShouldClose() override;
    void SwapBuffers() override;

    void* GetNativeWindow() const override { return m_window; }
    uint32_t GetWidth() const override { return m_width; }
    uint32_t GetHeight() const override { return m_height; }
    std::string_view GetTitle() const override { return m_title; }
    bool IsResizable() const override { return m_resizable; }
    bool IsFullscreen() const override { return m_fullscreen; }

    void SetTitle(std::string_view title) override;
    void SetSize(uint32_t width, uint32_t height) override;
    void SetFullscreen(bool fullscreen) override;

private:
    bool Init(const WindowCreateInfo& info);
    void SetupWindowHints(const WindowCreateInfo& info);
    void SetupCallbacks();

    static void OnFramebufferResize(GLFWwindow* window, int width, int height);
    static void OnWindowClose(GLFWwindow* window);

    GLFWwindow* m_window = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::string m_title;
    bool m_resizable = true;
    bool m_fullscreen = false;

    // GLFW 全局初始化引用计数：多窗口时只 init/terminate 一次
    static int s_glfwWindowCount;
};

}  // namespace seed
