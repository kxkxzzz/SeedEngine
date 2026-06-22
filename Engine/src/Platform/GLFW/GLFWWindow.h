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

    void SetEventCallback(const EventCallbackFn& callback) override {
        m_data.EventCallback = callback;
    }

    void PollEvents() override;
    bool ShouldClose() override;
    void SwapBuffers() override;

    void* GetNativeWindow() const override { return m_window; }
    uint32_t GetWidth() const override { return m_data.Width; }
    uint32_t GetHeight() const override { return m_data.Height; }
    std::string_view GetTitle() const override { return m_data.Title; }
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
    static void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void OnChar(GLFWwindow* window, unsigned int codepoint);
    static void OnMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void OnCursorPos(GLFWwindow* window, double xPos, double yPos);
    static void OnScroll(GLFWwindow* window, double xOffset, double yOffset);

    // 传给 glfwSetWindowUserPointer 的数据包
    // 静态回调里只需访问这些数据，不需要整个 GLFWWindow 对象
    struct WindowData {
        std::string Title;
        uint32_t Width = 0, Height = 0;
        EventCallbackFn EventCallback;
    };

    GLFWwindow* m_window = nullptr;
    WindowData m_data;
    bool m_resizable = true;
    bool m_fullscreen = false;

    // GLFW 全局初始化引用计数：多窗口时只 init/terminate 一次
    static int s_glfwWindowCount;
};

}  // namespace seed
