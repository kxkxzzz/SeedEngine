// GLFWWindow.cpp
#include "Platform/GLFW/GLFWWindow.h"

#include <glad/gl.h>  // 必须在 glfw3.h 之前包含
#include <GLFW/glfw3.h>

#include "Seed/Core/Log.h"

namespace seed {

int GLFWWindow::s_glfwWindowCount = 0;

// Window::Create 工厂在此实现：当前平台固定返回 GLFW 实现。
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
    // 首个窗口负责初始化 GLFW
    if (s_glfwWindowCount == 0) {
        if (!glfwInit()) {
            SEED_CORE_CRITICAL("GLFW 初始化失败");
            return false;
        }
    }

    SetupWindowHints(info);

    m_title = std::string(info.title);
    m_width = info.width;
    m_height = info.height;
    m_resizable = info.resizable;
    m_fullscreen = info.fullscreen;

    GLFWmonitor* monitor = info.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_window = glfwCreateWindow(static_cast<int>(info.width),
                                static_cast<int>(info.height), m_title.c_str(),
                                monitor, nullptr);
    if (!m_window) {
        SEED_CORE_CRITICAL("GLFW 窗口创建失败");
        return false;
    }
    ++s_glfwWindowCount;

    glfwSetWindowUserPointer(m_window, this);

    // headless 模式不创建 GL 上下文，跳过 glad 加载
    if (!info.headless) {
        glfwMakeContextCurrent(m_window);

        int version = gladLoadGL(glfwGetProcAddress);
        if (version == 0) {
            SEED_CORE_CRITICAL("glad 初始化失败（OpenGL 加载器）");
            return false;
        }
        SEED_CORE_INFO("OpenGL {}.{} 已加载", GLAD_VERSION_MAJOR(version),
                       GLAD_VERSION_MINOR(version));

        glfwSwapInterval(1);  // 垂直同步
    }

    SetupCallbacks();
    return true;
}

void GLFWWindow::PollEvents() {
    glfwPollEvents();
}

bool GLFWWindow::ShouldClose() {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void GLFWWindow::SwapBuffers() {
    if (m_window) {
        glfwSwapBuffers(m_window);
    }
}

void GLFWWindow::SetTitle(std::string_view title) {
    m_title = std::string(title);
    if (m_window) {
        glfwSetWindowTitle(m_window, m_title.c_str());
    }
}

void GLFWWindow::SetSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    if (m_window) {
        glfwSetWindowSize(m_window, static_cast<int>(width),
                          static_cast<int>(height));
    }
}

void GLFWWindow::SetFullscreen(bool fullscreen) {
    // 阶段 A 暂不支持运行时切换全屏，仅记录状态。
    m_fullscreen = fullscreen;
}

void GLFWWindow::SetupWindowHints(const WindowCreateInfo& info) {
    glfwDefaultWindowHints();

    if (!info.headless) {
        // 请求 OpenGL 4.6 Core Profile（与 glad 生成版本一致）
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
}

void GLFWWindow::OnFramebufferResize(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_width = static_cast<uint32_t>(width);
        self->m_height = static_cast<uint32_t>(height);
    }
    glViewport(0, 0, width, height);
}

void GLFWWindow::OnWindowClose(GLFWwindow* window) {
    // 阶段 C 接入事件系统后，这里会上抛 WindowCloseEvent。
    // 目前 ShouldClose() 已能反映关闭状态。
    (void)window;
}

}  // namespace seed
