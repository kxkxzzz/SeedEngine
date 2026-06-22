// KeyEvent.h —— 键盘相关事件
#pragma once

#include "Seed/Events/Event.h"

namespace seed {

// 键盘事件基类，持有 keycode
class KeyEvent : public Event {
public:
    int GetKeyCode() const { return m_keyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    explicit KeyEvent(int keyCode) : m_keyCode(keyCode) {}
    int m_keyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    // repeatCount > 0 表示系统的按键重复（长按触发）
    KeyPressedEvent(int keyCode, int repeatCount)
        : KeyEvent(keyCode), m_repeatCount(repeatCount) {}

    int GetRepeatCount() const { return m_repeatCount; }

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyPressedEvent: " << m_keyCode << " (repeat=" << m_repeatCount << ")";
        return oss.str();
    }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    int m_repeatCount;
};

class KeyReleasedEvent : public KeyEvent {
public:
    explicit KeyReleasedEvent(int keyCode) : KeyEvent(keyCode) {}

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyReleasedEvent: " << m_keyCode;
        return oss.str();
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

// 文字输入事件（处理 Unicode 字符，与 KeyPressed 分开）
class KeyTypedEvent : public KeyEvent {
public:
    explicit KeyTypedEvent(unsigned int keyCode) : KeyEvent(static_cast<int>(keyCode)) {}

    std::string ToString() const override {
        std::ostringstream oss;
        oss << "KeyTypedEvent: " << m_keyCode;
        return oss.str();
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

}  // namespace seed
