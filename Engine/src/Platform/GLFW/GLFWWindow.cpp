// GLFWWindow.cpp
#include "Platform/GLFW/GLFWWindow.h"

#include <glad/gl.h>  // 必须在 glfw3.h 之前包含
#include <GLFW/glfw3.h>

#include "Seed/Core/Log.h"
#include "Seed/Events/KeyEvent.h"
#include "Seed/Events/MouseEvent.h"
#include "Seed/Events/WindowEvent.h"

namespace seed {

int GLFWWindow::s_glfwWindowCount = 0;

// Window::Create 工厂在此实现：当前平台固定返回 GLFW 实现
Window* Window::Create(const WindowCreateInfo& info) {
    return new GLFWWindow(info);
}

GLFWWindow::GLFWWindow(const WindowCreateInfo& info) {
    Init(info);
}

GLFWWindow::~GLFWWindow() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        --s_glfwWindowCount;
    }
    if (s_glfwWindowCount == 0) {
        glfwTerminate();
    }
}

bool GLFWWindow::Init(const WindowCreateInfo& info) {
    if (s_glfwWindowCount == 0) {
        if (!glfwInit()) {
            SEED_CORE_CRITICAL("GLFW 初始化失败");
            return false;
        }
    }

    SetupWindowHints(info);

    m_data.Title  = std::string(info.title);
    m_data.Width  = info.width;
    m_data.Height = info.height;
    m_resizable   = info.resizable;
    m_fullscreen  = info.fullscreen;

    GLFWmonitor* monitor = info.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_window = glfwCreateWindow(static_cast<int>(info.width),
                                static_cast<int>(info.height),
                                m_data.Title.c_str(), monitor, nullptr);
    if (!m_window) {
        SEED_CORE_CRITICAL("GLFW 窗口创建失败");
        return false;
    }
    ++s_glfwWindowCount;

    // 传 WindowData 而非 this，回调里只需访问数据，不依赖对象布局
    glfwSetWindowUserPointer(m_window, &m_data);

    if (!info.headless) {
        glfwMakeContextCurrent(m_window);

        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0) {
            SEED_CORE_CRITICAL("glad 初始化失败（OpenGL 加载器）");
            return false;
        }
        SEED_CORE_INFO("OpenGL {}.{} 已加载", GLAD_VERSION_MAJOR(version),
                       GLAD_VERSION_MINOR(version));

        glfwSwapInterval(1);
    }

    SetupCallbacks();
    return true;
}

void GLFWWindow::PollEvents() { glfwPollEvents(); }

bool GLFWWindow::ShouldClose() {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void GLFWWindow::SwapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

void GLFWWindow::SetTitle(std::string_view title) {
    m_data.Title = std::string(title);
    if (m_window) glfwSetWindowTitle(m_window, m_data.Title.c_str());
}

void GLFWWindow::SetSize(uint32_t width, uint32_t height) {
    m_data.Width  = width;
    m_data.Height = height;
    if (m_window)
        glfwSetWindowSize(m_window, static_cast<int>(width), static_cast<int>(height));
}

void GLFWWindow::SetFullscreen(bool fullscreen) {
    m_fullscreen = fullscreen;
}

void GLFWWindow::SetupWindowHints(const WindowCreateInfo& info) {
    glfwDefaultWindowHints();

    if (!info.headless) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    } else {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    glfwWindowHint(GLFW_RESIZABLE, info.resizable ? GLFW_TRUE : GLFW_FALSE);
}

void GLFWWindow::SetupCallbacks() {
    glfwSetFramebufferSizeCallback(m_window, OnFramebufferResize);
    glfwSetWindowCloseCallback(m_window, OnWindowClose);
    glfwSetKeyCallback(m_window, OnKey);
    glfwSetCharCallback(m_window, OnChar);
    glfwSetMouseButtonCallback(m_window, OnMouseButton);
    glfwSetCursorPosCallback(m_window, OnCursorPos);
    glfwSetScrollCallback(m_window, OnScroll);
}

// --- 静态回调实现 ---

void GLFWWindow::OnFramebufferResize(GLFWwindow* window, int width, int height) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    data.Width  = static_cast<uint32_t>(width);
    data.Height = static_cast<uint32_t>(height);
    glViewport(0, 0, width, height);

    WindowResizeEvent event(data.Width, data.Height);
    if (data.EventCallback) data.EventCallback(event);
}

void GLFWWindow::OnWindowClose(GLFWwindow* window) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    WindowCloseEvent event;
    if (data.EventCallback) data.EventCallback(event);
}

void GLFWWindow::OnKey(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    if (!data.EventCallback) return;

    switch (action) {
        case GLFW_PRESS: {
            KeyPressedEvent event(key, 0);
            data.EventCallback(event);
            break;
        }
        case GLFW_REPEAT: {
            KeyPressedEvent event(key, 1);
            data.EventCallback(event);
            break;
        }
        case GLFW_RELEASE: {
            KeyReleasedEvent event(key);
            data.EventCallback(event);
            break;
        }
    }
}

void GLFWWindow::OnChar(GLFWwindow* window, unsigned int codepoint) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    KeyTypedEvent event(codepoint);
    if (data.EventCallback) data.EventCallback(event);
}

void GLFWWindow::OnMouseButton(GLFWwindow* window, int button, int action, int /*mods*/) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    if (!data.EventCallback) return;

    if (action == GLFW_PRESS) {
        MouseButtonPressedEvent event(button);
        data.EventCallback(event);
    } else if (action == GLFW_RELEASE) {
        MouseButtonReleasedEvent event(button);
        data.EventCallback(event);
    }
}

void GLFWWindow::OnCursorPos(GLFWwindow* window, double xPos, double yPos) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
    if (data.EventCallback) data.EventCallback(event);
}

void GLFWWindow::OnScroll(GLFWwindow* window, double xOffset, double yOffset) {
    auto& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
    MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
    if (data.EventCallback) data.EventCallback(event);
}

}  // namespace seed
