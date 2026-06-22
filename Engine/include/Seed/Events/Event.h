// Event.h —— 事件系统基础定义
#pragma once

#include <functional>
#include <sstream>
#include <string>

namespace seed {

// 所有事件类型枚举
// 新增事件时在此处添加，并实现对应的 Event 子类
enum class EventType {
    None = 0,
    // 窗口
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    // 键盘
    KeyPressed, KeyReleased, KeyTyped,
    // 鼠标
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

// 事件类别位掩码：一个事件可同时属于多个类别，便于按类过滤
enum EventCategory {
    EventCategoryNone        = 0,
    EventCategoryApplication = 1 << 0,
    EventCategoryInput       = 1 << 1,
    EventCategoryKeyboard    = 1 << 2,
    EventCategoryMouse       = 1 << 3,
    EventCategoryMouseButton = 1 << 4,
};

// 子类实现三个接口的样板宏，避免重复代码
// GetStaticType()  静态方法，供 EventDispatcher 模板匹配用
// GetEventType()   虚方法，运行时获取类型
// GetName()        调试用名称字符串
#define EVENT_CLASS_TYPE(type)                                          \
    static EventType GetStaticType() { return EventType::type; }       \
    EventType GetEventType() const override { return GetStaticType(); } \
    const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    int GetCategoryFlags() const override { return category; }

// 事件基类
class Event {
public:
    virtual ~Event() = default;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & category;
    }

    // 标记事件已被处理，阻止继续向下传递
    bool Handled = false;
};

// 按类型分发事件的工具类
// 用法：
//   EventDispatcher dispatcher(event);
//   dispatcher.Dispatch<WindowCloseEvent>(SEED_BIND_EVENT_FN(OnWindowClose));
class EventDispatcher {
public:
    explicit EventDispatcher(Event& event) : m_event(event) {}

    // T 是目标事件类型，F 是处理函数 bool(T&)
    // 类型匹配时调用 fn，fn 返回 true 表示事件已处理
    template<typename T, typename F>
    bool Dispatch(const F& fn) {
        if (m_event.GetEventType() == T::GetStaticType()) {
            m_event.Handled |= fn(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

// 将成员函数绑定为事件回调，省去每次写 lambda 的麻烦
// 用法：SEED_BIND_EVENT_FN(Application::OnWindowClose)
#define SEED_BIND_EVENT_FN(fn) \
    [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

// 方便 Log 输出 Event（operator<< 转发 ToString）
inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.ToString();
}

}  // namespace seed
