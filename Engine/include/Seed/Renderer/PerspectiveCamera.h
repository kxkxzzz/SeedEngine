// PerspectiveCamera.h —— 透视相机
#pragma once

#include <glm/glm.hpp>

namespace seed {

// 透视相机：持有 view 和 projection 矩阵
// view 由 位置+朝向 决定，projection 由 FOV+宽高比+裁剪面 决定
// 对外提供合成好的 ViewProjection 矩阵供 shader 做 MVP 变换
class PerspectiveCamera {
public:
    PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip, float farClip);

    // 相机位置
    const glm::vec3& GetPosition() const {
        return m_position;
    }
    void SetPosition(const glm::vec3& position) {
        m_position = position;
        RecalculateViewMatrix();
    }

    // 朝向：欧拉角（度）。yaw 偏航（左右），pitch 俯仰（上下）
    float GetYaw() const {
        return m_yaw;
    }
    float GetPitch() const {
        return m_pitch;
    }
    void SetRotation(float yaw, float pitch) {
        m_yaw = yaw;
        m_pitch = pitch;
        RecalculateViewMatrix();
    }

    // 窗口尺寸变化时更新投影
    void SetAspectRatio(float aspectRatio) {
        m_aspectRatio = aspectRatio;
        RecalculateProjectionMatrix();
    }

    // 相机的三个方向向量，供控制器做 WASD 移动
    const glm::vec3& GetForward() const {
        return m_forward;
    }
    const glm::vec3& GetRight() const {
        return m_right;
    }
    const glm::vec3& GetUp() const {
        return m_up;
    }

    const glm::mat4& GetViewMatrix() const {
        return m_viewMatrix;
    }
    const glm::mat4& GetProjectionMatrix() const {
        return m_projectionMatrix;
    }
    const glm::mat4& GetViewProjectionMatrix() const {
        return m_viewProjectionMatrix;
    }

private:
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

    glm::mat4 m_projectionMatrix{1.0f};
    glm::mat4 m_viewMatrix{1.0f};
    glm::mat4 m_viewProjectionMatrix{1.0f};

    glm::vec3 m_position{0.0f, 0.0f, 3.0f};

    // 朝向：由欧拉角算出 forward/right/up 三个向量
    float m_yaw = -90.0f;  // 初始朝 -Z 方向看
    float m_pitch = 0.0f;
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};

    // 投影参数
    float m_fov = 45.0f;
    float m_aspectRatio = 16.0f / 9.0f;
    float m_nearClip = 0.1f;
    float m_farClip = 100.0f;
};

}  // namespace seed
