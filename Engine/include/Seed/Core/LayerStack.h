#pragma once

#include <vector>

#include "Seed/Core/Layer.h"

namespace seed {

// LayerStack 管理 Layer 的容器，提供 Push/Pop 接口
// LayerStack 维护一个 Layer* 的 vector，PushLayer 插入普通层，PushOverlay 插入覆盖层
class LayerStack {
    std::vector<Layer*> m_layers;
    unsigned int m_layerInsertIndex = 0;  // 普通层插入位置

public:
    void PushLayer(Layer* layer);
    void PopLayer(Layer* layer);
    void PushOverlay(Layer* overlay);
    void PopOverlay(Layer* overlay);

    // 支持范围 for 循环
    auto begin() {
        return m_layers.begin();
    }
    auto end() {
        return m_layers.end();
    }
    auto rbegin() {
        return m_layers.rbegin();
    }
    auto rend() {
        return m_layers.rend();
    }
};
}  // namespace seed