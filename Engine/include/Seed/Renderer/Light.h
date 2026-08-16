// Light.h —— 光源类型定义
#pragma once

#include <glm/glm.hpp>

namespace seed {

// 光源类型枚举
enum class LightType
{
    Directional,
    Point,
    Spot
};

// 基础光源属性（所有光源共有）
struct LightBase {
    glm::vec3 Color{1.0f};  // 光源颜色
    float Intensity{1.0f};  // 强度
};

// 方向光：只有方向，无位置（如太阳）
struct DirectionalLight : LightBase {
    glm::vec3 Direction{0.0f, -1.0f, 0.0f};
};

// 点光源：从一点向四周均匀发光，有衰减
struct PointLight : LightBase {
    glm::vec3 Position{0.0f};
    float Constant{1.0f};  // 衰减常数项
    float Linear{0.09f};  // 衰减一次项
    float Quadratic{0.032f};  // 衰减二次项
};

// 聚光灯：点光源+方向锥体，有内外切角
struct SpotLight : LightBase {
    glm::vec3 Position{0.0f};
    glm::vec3 Direction{0.0f, -1.0f, 0.0f};
    float InnerCutOff{glm::cos(glm::radians(12.5f))};  // 内锥角余弦
    float OuterCutOff{glm::cos(glm::radians(17.5f))};  // 外锥角余弦
    float Constant{1.0f};
    float Linear{0.09f};
    float Quadratic{0.032f};
};

}  // namespace seed
