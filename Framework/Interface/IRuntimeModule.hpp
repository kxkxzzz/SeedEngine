// IRuntimeModule.hpp
#pragma once

#include "Interface.hpp"

namespace kx {
class IRuntimeModule {
public:
    virtual int Initialize() = 0;
    virtual void Tick() = 0;
    virtual void Shutdown() = 0;
};
}  // namespace kx
