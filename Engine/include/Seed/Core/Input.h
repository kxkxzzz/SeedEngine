// Input.h —— 输入状态轮询
// 与事件系统互补：事件是被动接收（发生时通知），Input 是主动查询（此刻 W 按住了吗）
#pragma once

#include <utility>

#include "Seed/Core/KeyCode.h"
#include "Seed/Core/MouseCode.h"

namespace seed {

// 静态接口，内部转发给平台实现（GLFW）
class Input {
public:
    static bool IsKeyPressed(KeyCode key);
    static bool IsMouseButtonPressed(MouseCode button);
    static std::pair<float, float> GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
};

}  // namespace seed
