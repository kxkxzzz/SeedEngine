// WindowGLFW.cpp
#include "WindowGLFW.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace kx {
WindowGLFW::WindowGLFW() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "[WindowGLFW] Failed to initialize GLFW" << std::endl;
        return;
    }
    m_initialized = true;
}

WindowGLFW::~WindowGLFW() {
    Destroy();
    if (m_initialized) {
        glfwTerminate();
        m_initialized = false;
    }
}

bool WindowGLFW::Create(const WindowCreateInfo& info) {
    if (!m_initialized) {
        std::cerr << "[WindowGLFW] GLFW is not initialized" << std::endl;
        return false;
    }
    if (m_window) {
        std::cerr << "[WindowGLFW] Window already created" << std::endl;
        return false;
    }

    // 设置窗口提示
    SetupWindowHints(info);

    return true;
}
}  // namespace kx