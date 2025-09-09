#pragma once

#include "Interface.hpp"
#include "IRuntimeModule.hpp"

namespace kx {
class IApplication : public IRuntimeModule {
public:
    virtual int Initialize() = 0;
    virtual void Tick() = 0;
    virtual void Shutdown() = 0;

    virtual bool IsQuitting() = 0;
};
}