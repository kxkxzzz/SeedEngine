#pragma once

#include "Seed/Core/TimeStep.h"
#include "Seed/Events/Event.h"

namespace seed {

// Layer 是一个抽象类，定义了层的接口
// 客户端通过继承 Layer 来实现自己的层，重写其中的虚函数
class Layer {
public:
    // 被 push 进栈时调用，做初始化
    virtual void OnAttach() {
    }
    // 被移出栈时调用，做清理
    virtual void OnDetach() {
    }
    // 每帧逻辑
    virtual void OnUpdate(Timestep ts) {
    }
    // 接收事件
    virtual void OnEvent(Event& e) {
    }
    // ImGui 阶段渲染
    virtual void OnImGuiRender() {
    }
};
}  // namespace seed