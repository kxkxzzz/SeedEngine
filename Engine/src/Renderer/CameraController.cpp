// CameraController.cpp
#include "Seed/Renderer/CameraController.h"

#include "Seed/Core/Input.h"
#include "Seed/Core/KeyCode.h"
#include "Seed/Core/MouseCode.h"
#include "Seed/Events/Event.h"

namespace seed {

CameraController::CameraController(float aspectRatio) : m_camera(45.0f, aspectRatio, 0.1f, 100.0f) {
}

void CameraController::OnUpdate(float deltaTime) {
    // --- 键盘：沿相机的 forward/right 平面移动 ---
    glm::vec3 position = m_camera.GetPosition();
    float velocity = m_moveSpeed * deltaTime;

    if (Input::IsKeyPressed(Key::W))
        position += m_camera.GetForward() * velocity;
    if (Input::IsKeyPressed(Key::S))
        position -= m_camera.GetForward() * velocity;
    if (Input::IsKeyPressed(Key::A))
        position -= m_camera.GetRight() * velocity;
    if (Input::IsKeyPressed(Key::D))
        position += m_camera.GetRight() * velocity;
    if (Input::IsKeyPressed(Key::Space))
        position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    if (Input::IsKeyPressed(Key::LeftShift))
        position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;

    m_camera.SetPosition(position);

    // --- 鼠标, 右键按住时才旋转视角---
    if (Input::IsMouseButtonPressed(Mouse::ButtonRight)) {
        auto [mouseX, mouseY] = Input::GetMousePosition();

        if (m_firstMouse) {
            m_lastMouseX = mouseX;
            m_lastMouseY = mouseY;
            m_firstMouse = false;
        }

        float deltaX = (mouseX - m_lastMouseX) * m_mouseSensitivity;
        float deltaY = (m_lastMouseY - mouseY) * m_mouseSensitivity;  // 屏幕 Y 向下，取反
        m_lastMouseX = mouseX;
        m_lastMouseY = mouseY;

        float yaw = m_camera.GetYaw() + deltaX;
        float pitch = m_camera.GetPitch() + deltaY;
        // 防止 pitch 到 90 度时叉乘退化
        pitch = glm::clamp(pitch, -89.0f, 89.0f);

        m_camera.SetRotation(yaw, pitch);
    }
    else {
        // 松开右键后重置，下次按下不会因位置跳变而猛转一下
        m_firstMouse = true;
    }
}

void CameraController::OnEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(SEED_BIND_EVENT_FN(CameraController::OnWindowResize));
}

bool CameraController::OnWindowResize(WindowResizeEvent& e) {
    float aspectRatio = static_cast<float>(e.GetWidth()) / static_cast<float>(e.GetHeight());
    m_camera.SetAspectRatio(aspectRatio);
    // 不结束事件，其他 Layer 可能也受 resize 影响
    return false;
}

}  // namespace seed
