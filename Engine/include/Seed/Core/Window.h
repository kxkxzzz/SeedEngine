// Window.h —— 平台无关的窗口抽象接口
#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace seed {

struct WindowCreateInfo {
    std::string_view title = "SeedEngine";

    uint32_t width = 1280;
    uint32_t height = 720;

    bool resizable = true;
    bool fullscreen = false;

    // 无窗口模式：用于 Vulkan/D3D12/Metal，不创建 OpenGL 上下文
    bool headless = false;
};

// 窗口抽象：GLFW 是当前唯一实现（GLFWWindow）.
// 通过 Window::Create(info) 工厂创建具体平台实例
class Window {
public:
    virtual ~Window() = default;

    // 工厂：根据平台返回具体实现（当前固定为 GLFWWindow）
    static Window* Create(const WindowCreateInfo& info);

    // 事件处理
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() = 0;

    // 交换前后缓冲（双缓冲）
    virtual void SwapBuffers() = 0;

    // 查询属性
    virtual void* GetNativeWindow() const = 0;  // GLFWwindow* / NSWindow* ...
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual std::string_view GetTitle() const = 0;
    virtual bool IsResizable() const = 0;
    virtual bool IsFullscreen() const = 0;

    // 设置属性
    virtual void SetTitle(std::string_view title) = 0;
    virtual void SetSize(uint32_t width, uint32_t height) = 0;
    virtual void SetFullscreen(bool fullscreen) = 0;
};

}  // namespace seed
