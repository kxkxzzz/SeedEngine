// BasicApplication.hpp
#pragma once

#include "IApplication.hpp"

namespace kx {
class BasicApplication : public IApplication {
public:
    virtual int Initialize();
    virtual void Tick();
    virtual void Shutdown();

    virtual bool IsQuitting();

protected:
    // 标识是否需要退出主循环
    bool m_quitting;
};
}