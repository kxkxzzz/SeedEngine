// SandboxApp.cpp —— 客户端测试程序
#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>

namespace seed {

// 阶段 A：暂不加自定义逻辑，仅依赖引擎开窗 + 清屏。
class Sandbox : public Application {
public:
    Sandbox() = default;
    ~Sandbox() override = default;
};

Application* CreateApplication() {
    return new Sandbox();
}

}  // namespace seed
