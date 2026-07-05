// GLFWInput.cpp —— Input 的 GLFW 实现
#include "Seed/Core/Input.h"

#include <GLFW/glfw3.h>

#include "Seed/Core/Application.h"

namespace seed {

// 从 Application 拿到当前 GLFW 窗口句柄
static GLFWwindow* GetGLFWWindow() {
    return static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
}

bool Input::IsKeyPressed(KeyCode key) {
    int state = glfwGetKey(GetGLFWWindow(), static_cast<int>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    int state = glfwGetMouseButton(GetGLFWWindow(), static_cast<int>(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::GetMousePosition() {
    double x, y;
    glfwGetCursorPos(GetGLFWWindow(), &x, &y);
    return {static_cast<float>(x), static_cast<float>(y)};
}

float Input::GetMouseX() {
    return GetMousePosition().first;
}
float Input::GetMouseY() {
    return GetMousePosition().second;
}

}  // namespace seed
