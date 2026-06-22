# SeedEngine 架构文档

> 本文档记录引擎的结构、各文件作用、运行时数据流与关键设计取舍。
> 随代码演进持续维护——每完成一个阶段（见 [ROADMAP.md](../ROADMAP.md)）就更新对应章节。
>
> 当前进度：**阶段 A 完成**（构建系统 + 窗口 + OpenGL 上下文）。

---

## 1. 设计总原则

1. **依赖倒置**：高层模块依赖抽象，不依赖具体实现。
   `Application` 只认识 `Window` 接口，不知道 GLFW 存在。
2. **接口与实现分离**：`include/` 放对外契约，`src/` 放私有实现。
   客户端只能看到接口，平台细节（GLFW/glad）被锁在 `src/Platform/`。
3. **引擎掌控生命周期**：客户端代码极简，`main`、主循环、资源管理都由引擎接管。
4. **每个阶段往骨架填肉**：后续功能（事件/Layer/RHI/渲染）都嵌入现有结构，尽量不改骨架。

---

## 2. 目录结构

```
SeedEngine/
├── CMakeLists.txt          # 顶层：仅 add_subdirectory
├── ROADMAP.md              # 阶段规划（要做什么）
├── docs/Architecture.md    # 本文档（怎么设计、为什么）
│
├── Engine/                 # ★ 引擎本体 → 静态库 Seed::Engine
│   ├── CMakeLists.txt
│   ├── include/Seed/       # 公开头：客户端 #include <Seed/...>
│   │   └── Core/
│   │       ├── Window.h        # 窗口抽象接口 + 工厂
│   │       ├── Application.h    # 应用基类（引擎脊柱）
│   │       └── EntryPoint.h     # 提供 main()
│   └── src/                # 私有实现：客户端看不到
│       ├── Core/
│       │   └── Application.cpp
│       └── Platform/GLFW/
│           ├── GLFWWindow.h     # Window 的 GLFW 实现（私有头）
│           └── GLFWWindow.cpp   # 实现 + Window::Create 工厂落地
│
├── Sandbox/                # 客户端测试程序 → 可执行 Sandbox
│   ├── CMakeLists.txt
│   └── src/SandboxApp.cpp
│
└── ThirdParty/            # 第三方依赖
    ├── glfw   (submodule, 3.4)     # 窗口/输入
    ├── glm    (submodule, 1.0.1)   # 数学
    ├── spdlog (submodule, 1.15.1)  # 日志
    └── glad   (本地生成, GL 4.6)    # OpenGL 函数加载
```

**核心划分**：`include/` 是"别人调用引擎时需要的东西"，`src/` 是"实现细节，别人不该碰也碰不到"。
客户端代码里出现 `glfw` 字样是不允许的——GLFW 整个被锁在 `src/Platform/`。

---

## 3. 逐文件作用

### 公开头（include/Seed/Core/）

#### `Window.h` —— 窗口抽象接口
定义"一个窗口应该能做什么"，但不关心怎么实现。
- `WindowCreateInfo`：创建参数包（标题/宽高/可调整/全屏/headless）。用 struct 打包参数，加新选项不破坏接口。
- `class Window`：纯虚接口（方法全 `= 0`），是一份"契约"，任何平台实现都得满足。
- `static Window* Create(info)`：**工厂方法**。调用方写 `Window::Create(info)` 拿到窗口，却不知道背后是 GLFW——具体返回什么由 `.cpp` 决定。
- `GetNativeWindow() -> void*`："逃生舱"，以后接 ImGui 等需要底层 `GLFWwindow*` 时用，平时不用。

#### `Application.h` —— 引擎脊柱
代表"一个引擎应用"，持有窗口、跑主循环。
- 持有 `unique_ptr<Window>`：析构自动释放。
- `Run()`：主循环所在。
- `Get()` 单例：全局唯一 Application，任何地方可 `Application::Get()` 访问（构造时 `s_instance = this`）。
- `CreateApplication()`：**声明在引擎，由客户端实现**。控制反转——引擎不知道你的 App 长什么样，只调这个工厂拿实例。

#### `EntryPoint.h` —— main 入口
把 `main()` 藏进引擎，客户端不用自己写。
- 客户端 `#include <Seed/Core/EntryPoint.h>` 即自动获得 `main`。
- `main` 干三件事：`CreateApplication()` → `Run()` → `delete`。
- **为什么放头文件**：`main` 必须编进可执行文件（Sandbox），不能编进静态库（Engine）。放头文件让 Sandbox 的 .cpp 来 include 并生成 main。

### 私有实现（src/）

#### `Application.cpp`
- `s_instance = nullptr`：静态成员须在 .cpp 定义一次（C++ 规则）。
- 构造：设单例 → 填 `WindowCreateInfo` → `Window::Create(info)` 拿窗口。**全程没有一个 `glfw` 字**，体现抽象价值。
- `Run()`：当前主循环 = 清屏 → 交换缓冲 → 处理事件。`glClearColor/glClear` 是阶段 A 临时直接调的 GL（阶段 E 做 RHI 后会被 `RenderCommand::Clear()` 取代）。

#### `GLFWWindow.h / .cpp`
- `class GLFWWindow : public Window`：实现那份契约。
- `struct GLFWwindow;`（前向声明）：头文件**不 include glfw3.h**，只声明指针类型。包含本头的文件不会被迫拖进整个 GLFW 头，编译更快、依赖更干净。真正 `#include <GLFW/glfw3.h>` 只在 .cpp。
- `static int s_glfwWindowCount`：引用计数。首个窗口 `glfwInit()`，最后一个窗口 `glfwTerminate()`。
- 静态回调 `OnFramebufferResize/OnWindowClose`：GLFW 是 C 库，回调必须是普通函数指针，不能是成员函数。靠 `glfwSetWindowUserPointer(this)` 存入 this，回调里 `glfwGetWindowUserPointer` 取回，桥接到对象。

### 客户端（Sandbox/src/）

#### `SandboxApp.cpp`
整个客户端就这么点：继承 `Application`、实现 `CreateApplication()` 返回它。
**没有 main、没有 GLFW、没有循环**——全被引擎接管。这是好引擎该有的样子。

---

## 4. 依赖结构

```
        ┌─────────────────────────────────────────┐
        │  Sandbox (可执行)                         │
        │  - 只 #include <Seed/Core/...>            │
        │  - 继承 Application，实现 CreateApplication│
        └────────────────┬────────────────────────┘
                         │ 链接 Seed::Engine
                         ▼
   ┌──────────────────────────────────────────────────┐
   │  Engine (静态库)                                   │
   │  include/Seed/Core/  ← 公开契约（抽象）             │
   │    Window (接口) ◄───────┐                         │
   │    Application           │ 依赖抽象，不依赖具体      │
   │    EntryPoint            │                         │
   │  src/  ← 私有实现          │                         │
   │    Application.cpp ───────┘                         │
   │    Platform/GLFW/GLFWWindow ──► 实现 Window 接口     │
   │                              └─► 这里才碰 glfw/glad  │
   └──────────────────────────────────────────────────┘
                         │
                         ▼
         ThirdParty: glfw / glad / glm / spdlog
```

**核心原则**：依赖永远指向抽象，而非具体。要换成 SDL，只需加 `SDLWindow.cpp`，`Application` 一行不改。

**CMake 依赖可见性**：
- glfw / glad → `PRIVATE`（客户端不该直接看到）
- glm / spdlog → `PUBLIC`（后续公开头会用到数学类型和日志）

---

## 5. 运行时数据流（一帧发生了什么）

```
程序启动
  └─ main() [EntryPoint.h]
       └─ CreateApplication() [Sandbox 实现] → new Sandbox()
            └─ Application 构造 [Application.cpp]
                 ├─ s_instance = this
                 └─ Window::Create(info) → new GLFWWindow(info)
                      └─ GLFWWindow::Init()
                           ├─ glfwInit() (首个窗口)
                           ├─ 设 GL 4.6 Core hints
                           ├─ glfwCreateWindow()
                           ├─ glfwMakeContextCurrent() + gladLoadGL()  ← OpenGL 4.6 loaded
                           └─ 注册 resize/close 回调
       └─ app->Run()  ← 主循环开始
            每帧:
              ① glClear()           清屏
              ② SwapBuffers()       显示这一帧
              ③ PollEvents()        处理输入/窗口事件 → 触发回调
            直到 ShouldClose()==true (用户点 X)
       └─ delete app
            └─ ~GLFWWindow: glfwDestroyWindow + glfwTerminate(最后一个)
```

---

## 6. 关键设计取舍

| 设计 | 为什么这么做 | 替代方案及缺点 |
|---|---|---|
| 抽象 Window + 工厂 | Application 不依赖 GLFW，可换后端 | 直接用 GLFWWindow → 锁死平台 |
| include/src 分离 | 客户端只见接口，实现可自由改 | 头实现同目录 → 暴露细节、易误用 |
| EntryPoint 提供 main | 客户端代码极简，引擎控生命周期 | 客户端自己写 main → 重复样板 |
| 前向声明 GLFWwindow | 公开头不拖 GLFW，编译快 | 直接 include → 传染式依赖 |
| 引用计数 init GLFW | 多窗口安全 | 每对象 init/terminate → 多窗口崩 |
| 单例 Application::Get() | 全局访问点 | 到处传指针 → 参数污染 |

---

## 7. 当前有意为之的简化（属于后续阶段，非疏漏）

| 现状 | 将在哪个阶段完善 |
|---|---|
| `Run()` 里直接调 `glClear` | 阶段 E（RHI）抽象成渲染命令 |
| `OnWindowClose` 回调是空的 | 阶段 C（事件系统）上抛 `WindowCloseEvent` |
| 没有 Layer 栈 | 阶段 D |
| 没有日志 | 阶段 B（下一步） |

这套结构的好处：每个后续阶段都是往骨架里"填肉"，不用再动骨架。

---

## 构建与运行

```bash
# 配置（首次，clang-cl + Ninja）
cmake -S . -B out -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
# 编译
cmake --build out
# 运行
./out/bin/Sandbox.exe
```
