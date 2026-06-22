// EntryPoint.h —— 引擎入口
#pragma once

#include "Seed/Core/Application.h"

// 客户端在其 main 翻译单元里 #include 本文件即可获得 main()。
// 引擎负责 创建 -> 运行 -> 销毁，客户端只需实现 CreateApplication()。
int main(int argc, char** argv) {
    auto* app = seed::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
