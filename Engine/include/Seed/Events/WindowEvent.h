// WindowEvent.h —— 窗口相关事件
#pragma once

#include "Seed/Events/Event.h"

namespace seed {

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(uint32_t width, uint32_t height) : m_width(width), m_height(height) {
    }

    uint32_t GetWidth() const {
        return m_width;
    }
    uint32_t GetHeight() const {
        return m_height;
    }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "WindowResizeEvent: " << m_width << "x" << m_height;
        return oss.str();
    }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
    uint32_t m_width, m_height;
};

}  // namespace seed
