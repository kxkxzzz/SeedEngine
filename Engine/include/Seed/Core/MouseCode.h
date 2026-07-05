// MouseCode.h —— 引擎鼠标按键码
// 数值与 GLFW 对齐
#pragma once

#include <cstdint>

namespace seed {

using MouseCode = uint16_t;

namespace Mouse {
enum : MouseCode {
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,

    ButtonLeft   = Button0,
    ButtonRight  = Button1,
    ButtonMiddle = Button2,
};
}  // namespace Mouse

}  // namespace seed
