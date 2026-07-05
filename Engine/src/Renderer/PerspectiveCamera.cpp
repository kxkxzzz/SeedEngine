// PerspectiveCamera.cpp
#include "Seed/Renderer/PerspectiveCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace seed {

PerspectiveCamera::PerspectiveCamera(float fovDegrees, float aspectRatio, float nearClip,
                                     float farClip)
    : m_fov(fovDegrees), m_aspectRatio(aspectRatio), m_nearClip(nearClip), m_farClip(farClip) {
    RecalculateProjectionMatrix();
    RecalculateViewMatrix();
}

void PerspectiveCamera::RecalculateProjectionMatrix() {
    // glm::perspective 生成透视投影矩阵
    // 参数：视野角(弧度)、宽高比、近裁剪面、远裁剪面
    m_projectionMatrix =
        glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearClip, m_farClip);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void PerspectiveCamera::RecalculateViewMatrix() {
    // 由欧拉角(yaw/pitch)算出相机朝向向量 forward
    //    yaw 绕 Y 轴（左右转），pitch 绕 X 轴（上下看）
    glm::vec3 forward;
    forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    forward.y = sin(glm::radians(m_pitch));
    forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_forward = glm::normalize(forward);

    // 由 forward 和世界上方向 算出 right 和 up
    //     right = forward × worldUp
    const glm::vec3 worldUp{0.0f, 1.0f, 0.0f};
    m_right = glm::normalize(glm::cross(m_forward, worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_forward));

    // glm::lookAt(相机位置, 目标点, 上方向) 计算 view 矩阵
    //     目标点 = 位置 + 朝向，表示"从 position 沿 forward 看过去"
    m_viewMatrix = glm::lookAt(m_position, m_position + m_forward, m_up);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

}  // namespace seed
