// IWindow.hpp
#pragma once

#include <string_view>

namespace kx {

struct WindowCreateInfo {
    std::string_view title = "SeedEngine";

    uint32_t width = 1280;
    uint32_t height = 720;

    bool resizable = true;

    bool fullscreen = false;

    // 无窗口模式, 用于 Vulkan/D3D12/Metal，不创建 OpenGL 上下文
    bool headless = false;
};

class IWindow {
public:
    virtual ~IWindow() {};

    // 创建&销毁 窗口
    virtual bool Create(const WindowCreateInfo& createInfo) = 0;
    virtual void Destroy() = 0;

    // 事件处理
    virtual void PollEvents() = 0;
    virtual bool ShouldClose() = 0;

    // 获取窗口属性
    // GLFWwindow* NSWindow*
    virtual void* GetNativeWindow() const = 0;
    virtual uint32_t GetWidth() const = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual std::string_view GetTitle() const = 0;
    virtual bool IsResizable() const = 0;
    virtual bool IsFullscreen() const = 0;

    // 设置窗口属性
    virtual void SetTitle(std::string_view title) = 0;
    virtual void SetSize(uint32_t width, uint32_t height) = 0;
    virtual void SetFullscreen(bool fullscreen) = 0;
};
}  // namespace kx