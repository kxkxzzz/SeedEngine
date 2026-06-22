// SandboxApp.cpp —— 客户端测试程序
#include <Seed/Core/Application.h>
#include <Seed/Core/EntryPoint.h>
#include <Seed/Core/Log.h>

namespace seed {

// 阶段 A：暂不加自定义逻辑，仅依赖引擎开窗 + 清屏。
class Sandbox : public Application {
public:
    Sandbox() { SEED_INFO("Sandbox 客户端已创建"); }
    ~Sandbox() override = default;
};

Application* CreateApplication() {
    return new Sandbox();
}

}  // namespace seed
