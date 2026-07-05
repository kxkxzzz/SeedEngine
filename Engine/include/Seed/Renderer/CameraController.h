// CameraController.h —— FPS 风格相机控制器
#pragma once

#include "Seed/Events/Event.h"
#include "Seed/Events/MouseEvent.h"
#include "Seed/Events/WindowEvent.h"
#include "Seed/Renderer/PerspectiveCamera.h"

namespace seed {

// 持有一个 PerspectiveCamera，每帧根据 WASD 和鼠标移动更新它
// WASD 沿相机的 forward/right 平面移动，鼠标控制 yaw/pitch
class CameraController {
public:
    CameraController(float aspectRatio);

    void OnUpdate(float deltaTime);
    void OnEvent(Event& e);

    PerspectiveCamera& GetCamera() { return m_camera; }
    const PerspectiveCamera& GetCamera() const { return m_camera; }

    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    void SetMouseSensitivity(float sensitivity) { m_mouseSensitivity = sensitivity; }

private:
    bool OnWindowResize(WindowResizeEvent& e);

    PerspectiveCamera m_camera;

    float m_moveSpeed        = 3.0f;   // 每秒移动的世界单位
    float m_mouseSensitivity = 0.1f;   // 鼠标每像素位移对应的角度

    bool m_firstMouse   = true;  // 首帧没有"上一次鼠标位置"，跳过一次 delta 计算
    float m_lastMouseX  = 0.0f;
    float m_lastMouseY  = 0.0f;
};

}  // namespace seed
