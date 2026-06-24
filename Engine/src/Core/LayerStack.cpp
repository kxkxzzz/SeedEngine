// LayerStack.cpp
#include "Seed/Core/LayerStack.h"

#include <algorithm>

namespace seed {

void LayerStack::PushLayer(Layer* layer) {
    // 插入到普通层区域末尾，Overlay 区域之前
    m_layers.emplace(m_layers.begin() + m_layerInsertIndex, layer);
    ++m_layerInsertIndex;
    layer->OnAttach();
}

void LayerStack::PopLayer(Layer* layer) {
    auto it = std::find(m_layers.begin(), m_layers.begin() + m_layerInsertIndex, layer);
    if (it != m_layers.begin() + m_layerInsertIndex) {
        layer->OnDetach();
        m_layers.erase(it);
        --m_layerInsertIndex;
    }
}

void LayerStack::PushOverlay(Layer* overlay) {
    m_layers.emplace_back(overlay);
    overlay->OnAttach();
}

void LayerStack::PopOverlay(Layer* overlay) {
    auto it = std::find(m_layers.begin() + m_layerInsertIndex, m_layers.end(), overlay);
    if (it != m_layers.end()) {
        overlay->OnDetach();
        m_layers.erase(it);
    }
}

}  // namespace seed
