// MouseEvent.h —— 鼠标相关事件
#pragma once

#include "Seed/Events/Event.h"

namespace seed {

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : m_mouseX(x), m_mouseY(y) {}

    float GetX() const { return m_mouseX; }
    float GetY() const { return m_mouseY; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
        return oss.str();
    }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float m_mouseX, m_mouseY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset)
        : m_xOffset(xOffset), m_yOffset(yOffset) {}

    float GetXOffset() const { return m_xOffset; }
    float GetYOffset() const { return m_yOffset; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseScrolledEvent: " << m_xOffset << ", " << m_yOffset;
        return oss.str();
    }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float m_xOffset, m_yOffset;
};

class MouseButtonEvent : public Event {
public:
    int GetMouseButton() const { return m_button; }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryMouseButton | EventCategoryInput)

protected:
    explicit MouseButtonEvent(int button) : m_button(button) {}
    int m_button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseButtonPressedEvent: " << m_button;
        return oss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "MouseButtonReleasedEvent: " << m_button;
        return oss.str();
    }

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

}  // namespace seed
