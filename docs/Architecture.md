# SeedEngine 架构文档

> 本文档记录引擎的结构、各文件作用、运行时数据流与关键设计取舍。
> 随代码演进持续维护——每完成一个阶段（见 [ROADMAP.md](../ROADMAP.md)）就更新对应章节。
>
> 当前进度：**阶段 G 完成**（Mesh + Texture2D + assimp 模型加载，FPS 飞行相机看带贴图的 3D 模型）。

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
│   │   ├── Core/
│   │   │   ├── Window.h        # 窗口抽象接口 + 工厂
│   │   │   ├── Application.h   # 应用基类（引擎脊柱）
│   │   │   ├── EntryPoint.h    # 提供 main()
│   │   │   ├── Log.h           # 日志系统（封装 spdlog）+ 日志宏
│   │   │   ├── Assert.h        # 断言宏（Debug 下打日志并中断）
│   │   │   ├── Layer.h         # Layer 抽象基类
│   │   │   ├── LayerStack.h    # Layer 容器，普通层/Overlay 分区管理
│   │   │   ├── TimeStep.h      # 帧间隔时间封装
│   │   │   ├── Input.h         # 输入状态轮询接口
│   │   │   ├── KeyCode.h       # 引擎按键码（对齐 GLFW 数值）
│   │   │   └── MouseCode.h     # 引擎鼠标按键码
│   │   ├── Events/              # Event 基类 + Window/Key/Mouse 具体事件
│   │   ├── RHI/                 # 渲染硬件接口抽象（见第 8 节）
│   │   │   └── Texture.h            # Texture/Texture2D 抽象 + 工厂（阶段 G）
│   │   └── Renderer/
│   │       ├── PerspectiveCamera.h  # 透视相机：view/projection 矩阵
│   │       ├── Renderer.h           # BeginScene/Submit/EndScene
│   │       ├── CameraController.h   # FPS 飞行相机控制器
│   │       ├── Mesh.h                # Vertex/MeshTexture/Mesh（阶段 G）
│   │       └── Model.h               # assimp 模型加载器（阶段 G）
│   └── src/                # 私有实现：客户端看不到
│       ├── Core/
│       │   ├── Application.cpp
│       │   ├── Log.cpp
│       │   └── LayerStack.cpp
│       ├── Platform/GLFW/
│       │   ├── GLFWWindow.h/.cpp    # Window 的 GLFW 实现
│       │   └── GLFWInput.cpp       # Input 的 GLFW 实现
│       ├── Renderer/
│       │   ├── Renderer.cpp
│       │   ├── PerspectiveCamera.cpp
│       │   ├── CameraController.cpp
│       │   ├── Mesh.cpp             # 建 VertexArray + 绑贴图 + Renderer::Submit（阶段 G）
│       │   └── Model.cpp            # assimp 递归解析场景 → Mesh 列表（阶段 G）
│       └── RHI/                    # OpenGL 后端实现（见第 8 节），含 OpenGL/OpenGLTexture.h/.cpp（阶段 G）
│
├── Sandbox/                # 客户端测试程序 → 可执行 Sandbox
│   ├── CMakeLists.txt
│   ├── assets/models/       # 测试模型资产（如 BoxTextured，CC-BY 4.0）
│   └── src/SandboxApp.cpp
│
└── ThirdParty/            # 第三方依赖
    ├── glfw   (submodule, 3.4)     # 窗口/输入
    ├── glm    (submodule, 1.0.1)   # 数学
    ├── spdlog (submodule, 1.15.1)  # 日志
    ├── glad   (本地生成, GL 4.6)    # OpenGL 函数加载
    ├── stb    (submodule)          # stb_image.h，解码图片
    └── assimp (submodule, v6.0.5)  # 解析 .obj/.gltf 等模型格式
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

#### `Log.h` —— 日志系统
封装 spdlog，提供双 logger 与一组日志宏。
- 两个 logger：`CoreLogger`（引擎内部，输出前缀 `SEED`）和 `ClientLogger`（客户端，前缀 `APP`），便于区分日志来源。
- `Log::Init()`：创建两个彩色控制台 logger，设置格式 `[时间] 名称: 消息`，按等级着色。
- 日志宏分两套：`SEED_CORE_*`（引擎用）和 `SEED_*`（客户端用），各含 TRACE/INFO/WARN/ERROR/CRITICAL。变参转发给 spdlog，支持 `{}` 占位格式化。
- **取舍**：公开头直接 include `spdlog.h`（较重）。因为日志是基础设施、到处都用，这个传染是可接受的（Hazel 同款做法）。

#### `Assert.h` —— 断言宏
- `SEED_ASSERT(cond, ...)` / `SEED_CORE_ASSERT(cond, ...)`：条件为假时打错误日志（含 `__FILE__:__LINE__`）并触发调试中断。
- `SEED_DEBUGBREAK()`：clang-cl/MSVC 用 `__debugbreak()`，GCC/Clang 用 `raise(SIGTRAP)`。
- 只在定义了 `SEED_DEBUG`（Debug 配置）时生效；Release 下宏展开为空，**零开销**。
- `SEED_DEBUG`/`SEED_RELEASE` 由 CMake 按构建配置定义（见 Engine/CMakeLists.txt）。

#### `TimeStep.h` —— 帧间隔时间
封装每帧的 delta time，避免各处裸传 `float`。
- `GetSeconds()` / `GetMilliseconds()`：两种精度访问。
- `operator float()`：隐式转换，方便直接传给需要 float 的函数。
- header-only，无对应 .cpp。

#### `Layer.h` —— Layer 抽象基类
定义"一个层应该能做什么"，客户端继承并重写虚函数。
- `OnAttach()`：被 push 进栈时调用，做初始化。
- `OnDetach()`：被移出栈时调用，做清理。
- `OnUpdate(Timestep ts)`：每帧逻辑，接收帧间隔。
- `OnEvent(Event& e)`：接收事件，可设 `e.Handled = true` 阻止向下传递。
- `OnImGuiRender()`：预留给 ImGui 阶段（阶段 H）。
- 所有方法默认空实现，子类按需重写。header-only。

#### `LayerStack.h / LayerStack.cpp` —— Layer 容器
用**单个 vector + 插入指针**管理普通层和 Overlay，保证遍历时天然有序。
- `PushLayer`：插入到 `m_layerInsertIndex` 位置，`++m_layerInsertIndex`，调用 `OnAttach`。
- `PushOverlay`：`push_back` 到末尾，调用 `OnAttach`。
- `PopLayer` / `PopOverlay`：找到后调 `OnDetach`，erase，Pop Layer 时 `--m_layerInsertIndex`。
- 提供 `begin/end/rbegin/rend`：Update 用正向遍历，Event 用反向遍历。

### 私有实现（src/）

#### `Application.cpp`
- `s_instance = nullptr`：静态成员须在 .cpp 定义一次（C++ 规则）。
- 构造：设单例 → `Log::Init()` → 创建窗口 → 设事件回调 → `Renderer::Init()`。**全程没有一个 `glfw`/`gl` 字**，体现抽象价值。
- `Run()`：主循环 = 计算 Timestep → 遍历 Layer `OnUpdate` → SwapBuffers → PollEvents。清屏和绘制完全交给 Layer 通过 `Renderer`/RHI 完成，`Application` 不直接碰任何 GL 调用。

#### `Input.h / GLFWInput.cpp` —— 输入状态轮询
与事件系统互补：事件是被动接收（发生时通知），Input 是主动查询（此刻问"W 按住了吗"）。
- 静态接口 `IsKeyPressed` / `IsMouseButtonPressed` / `GetMousePosition`，客户端只认 `seed::Key::W` 这类引擎按键码，不碰 GLFW 宏。
- `GLFWInput.cpp` 通过 `Application::Get().GetWindow().GetNativeWindow()` 拿到 `GLFWwindow*`，转发给 `glfwGetKey` 等。
- `KeyCode.h` / `MouseCode.h`：数值特意与 GLFW 对齐，映射零开销，但类型是 `seed::KeyCode`，不是 GLFW 类型。

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
         ThirdParty: glfw / glad / glm / spdlog / stb / assimp
```

**核心原则**：依赖永远指向抽象，而非具体。要换成 SDL，只需加 `SDLWindow.cpp`，`Application` 一行不改。

**CMake 依赖可见性**：
- glfw / glad / stb_image / assimp → `PRIVATE`（客户端不该直接看到）
- glm / spdlog → `PUBLIC`（后续公开头会用到数学类型和日志）

---

## 5. 运行时数据流（一帧发生了什么）

```
程序启动
  └─ main() [EntryPoint.h]
       └─ CreateApplication() [Sandbox 实现] → new Sandbox()
            └─ Application 构造 [Application.cpp]
                 ├─ s_instance = this
                 ├─ Log::Init()  ← 日志最先初始化，后续模块都依赖它
                 └─ Window::Create(info) → new GLFWWindow(info)
                      └─ GLFWWindow::Init()
                           ├─ glfwInit() (首个窗口)
                           ├─ 设 GL 4.6 Core hints
                           ├─ glfwCreateWindow()
                           ├─ glfwMakeContextCurrent() + gladLoadGL()  ← OpenGL 4.6 loaded
                           └─ 注册 resize/close/键盘/鼠标 回调
       └─ Renderer::Init()  ← 创建全局 RenderAPI，Application 构造最后一步
       └─ app->Run()  ← 主循环开始
            每帧:
              ① 计算 Timestep = now - lastFrameTime
              ② for layer in layerStack: layer->OnUpdate(ts)   从底向上
                  └─ Layer 内部：
                       ├─ CameraController::OnUpdate() 轮询 Input，移动/旋转相机
                       ├─ Renderer::Clear()
                       ├─ Renderer::BeginScene(camera)  记录 ViewProjection 矩阵
                       ├─ Renderer::Submit(shader, vao, transform)  绘制一个物体
                       └─ Renderer::EndScene()
              ③ SwapBuffers()       显示这一帧
              ④ PollEvents()        处理输入/窗口事件 → 触发回调
                  └─ OnEvent(e)
                       ├─ Dispatch<WindowCloseEvent> → OnWindowClose → m_running=false
                       └─ for layer in layerStack.rbegin(): layer->OnEvent(e)  从顶向下
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
| glfwGetTime() 直接在 Application 里调用 | 后续换成平台无关的时间接口 |
| `Renderer::Submit` 每次都全量设 uniform，无渲染队列/排序 | 后续按需扩展（当前物体数量少，非瓶颈） |
| 只打通 diffuse 贴图，无光照 | 阶段 H：Blinn-Phong / PBR，用上 Mesh 已提取的法线 |

这套结构的好处：每个后续阶段都是往骨架里"填肉"，不用再动骨架。

---

## 8. RHI 抽象层（阶段 E）

后端无关的渲染硬件接口。上层只认抽象接口，OpenGL 是其中一个实现，未来可扩 Vulkan。

### 分层

```
include/Seed/RHI/           # 抽象接口（公开）
    RendererAPIType.h        # enum 当前后端（None/OpenGL/Vulkan）
    RenderAPI.h              # 渲染命令 Init/Clear/DrawIndexed + 持有当前后端类型
    Buffer.h                 # VertexBuffer/IndexBuffer/BufferLayout
    VertexArray.h            # 把 VBO+IBO 绑成可绘制单元
    Shader.h                 # 编译 GLSL + 设 uniform
    Texture.h / Framebuffer.h # 贴图 / 离屏渲染目标（接口先行，实现在后续阶段）

src/RHI/                    # 工厂（私有）
    RenderAPI.cpp / Buffer.cpp / VertexArray.cpp / Shader.cpp
        每个抽象类的 Create() 在此 switch(当前后端) 返回对应实现

src/RHI/OpenGL/             # OpenGL 后端实现（私有）
    OpenGLRenderAPI / OpenGLBuffer / OpenGLVertexArray / OpenGLShader
```

### 核心设计

- **工厂模式解耦**：`VertexBuffer::Create(...)` 内部按 `RenderAPI::GetAPI()` 返回 `OpenGLVertexBuffer`，上层拿到的是接口指针，不知道后端。
- **BufferLayout 自动算偏移/步长**：声明"顶点 = Float3 位置 + Float3 颜色"，构造时自动算出每个属性 Offset 和总 Stride。`OpenGLVertexArray::AddVertexBuffer` 再把它翻译成 `glVertexAttribPointer`——把手动数字节的活自动化。
- **RAII 封装 GPU 资源**：每个 OpenGL 对象（VBO/VAO/Program）的句柄存在 `m_rendererID`，构造时创建、析构时删除。

### 一次 DrawIndexed 的数据流

```
SandboxApp（只用 RHI 接口，无一句 gl*）
  → VertexArray::Create()  → OpenGLVertexArray（glCreateVertexArrays）
  → VertexBuffer::Create() + SetLayout()  → BufferLayout 算好 offset/stride
  → AddVertexBuffer()  → glVertexAttribPointer 逐属性配置
  → Shader::Create()  → 编译链接 GLSL program
  → 每帧 Clear() + shader->Bind() + DrawIndexed()  → glDrawElements
```

---

## 9. Renderer 与相机（阶段 F）

把"Layer 自己攥着 RenderAPI 画三角形"升级为正规的渲染管线：相机产出矩阵，Renderer 统一提交绘制。

### PerspectiveCamera —— 透视相机

持有 `view` 和 `projection` 两个矩阵，合成 `ViewProjection` 供 shader 做 MVP 变换。

- **projection**：`glm::perspective(fov, aspect, near, far)`，只在 FOV/宽高比/裁剪面变化时重算（如窗口 resize）。
- **view**：由位置 + 欧拉角(yaw/pitch) 算出：
  1. 欧拉角 → `forward` 向量（球坐标转笛卡尔坐标的标准公式）
  2. `right = normalize(cross(forward, worldUp))`，`up = normalize(cross(right, forward))`——叉乘求出与两个向量都垂直的方向
  3. `glm::lookAt(position, position + forward, up)` 生成 view 矩阵
- `SetPosition`/`SetRotation` 改动后立即重算 view，`GetViewProjectionMatrix()` 始终是最新值——不用外部手动同步。

### Renderer —— 静态渲染器

全引擎唯一的静态类，是 Layer 和 RHI 之间的中间层。

- `Init()`：创建全局 `RenderAPI`（在 `Application` 构造、GL 上下文就绪后调用一次）。
- `SetClearColor()` / `Clear()`：转发给 `RenderAPI`，Layer 不必直接持有 `RenderAPI`。
- `BeginScene(camera)`：把相机的 `ViewProjectionMatrix` 存进 `SceneData`（该帧全局共享的场景数据，以后扩展光源/环境贴图都加在这里）。
- `Submit(shader, vertexArray, transform)`：`shader->Bind()` → 设 `u_ViewProjection`/`u_Transform` 两个约定 uniform → `DrawIndexed`。这是引擎和 shader 之间的**契约**——任何 shader 想被 `Renderer::Submit` 正确渲染，必须声明这两个 uniform。

### CameraController —— FPS 飞行相机

持有一个 `PerspectiveCamera`，每帧翻译"输入状态"为"相机变化"。

- **移动**：轮询 `Input::IsKeyPressed`，WASD 沿相机的 `forward`/`right` 平面移动，Space/Shift 沿世界 Y 轴升降。
- **视角旋转**：**右键按住时才旋转**（避免一进窗口视角就乱转）；用**相对上一帧的鼠标位移**（delta）算 yaw/pitch，而非绝对坐标，否则转向会跳变。
- `m_firstMouse` 标志：首次按下右键或每次松开重置，避免因鼠标位置跳变导致视角"猛转一下"。
- `pitch` 限制在 `[-89°, 89°]`：防止到 90° 时 `forward` 与 `worldUp` 平行，叉乘退化为零向量。
- `OnEvent` 里用 `EventDispatcher` 监听 `WindowResizeEvent`，同步更新相机宽高比；返回 `false` 表示不消费事件（其他 Layer 可能也关心 resize）。

---

## 10. Mesh 与模型加载（阶段 G）

把手写立方体顶点换成真实模型文件：assimp 解析场景 → `Mesh` 持有 GPU 资源 → `Texture2D` 提供贴图，三者拼成一条完整的"文件到屏幕"链路。

### Texture2D —— RHI 贴图（OpenGL 实现）

`src/RHI/OpenGL/OpenGLTexture.cpp`，延续项目里的 DSA 风格（`glCreateBuffers`/`glCreateVertexArrays` 已是这个模式）：

- `Create(path)`：`stbi_load` 读文件解码 → 按 channel 数选 `GL_RGB8`/`GL_RGBA8` → `glCreateTextures` + `glTextureStorage2D` + `glTextureSubImage2D` → `glGenerateTextureMipmap`。读取失败时 `SEED_CORE_ASSERT`，不静默吞错误。
- `Bind(slot)`：`glBindTextureUnit(slot, id)`，比传统 `glActiveTexture` + `glBindTexture` 两步少一步。
- 工厂 `Texture.cpp` 与 `Buffer.cpp`/`Shader.cpp` 完全同构：按 `RenderAPI::GetAPI()` switch 返回对应后端实现。

### Mesh —— 一份可绘制的几何 + 贴图

`Vertex { Position, Normal, TexCoords }`：字段顺序对应 shader 里 `layout(location = 0/1/2)`。构造时用 `VertexBuffer::Create` + `SetLayout({Float3, Float3, Float2})` + `IndexBuffer::Create` 建好 `VertexArray`——`BufferLayout` 自动算 offset/stride 的机制（第 8 节）直接复用，不用为新顶点结构手写 `glVertexAttribPointer`。

`Draw()`：从 `m_textures` 里找第一个 `Type == "diffuse"` 的贴图 `Bind(0)`，设 `u_DiffuseTexture`/`u_HasTexture` 两个 uniform（无贴图时退化成固定灰色，不崩不留空洞），再调用 `Renderer::Submit`——**不重复 Renderer 已有的"设 ViewProjection/Transform + DrawIndexed"逻辑**，`Mesh` 只管贴图这一件 `Renderer::Submit` 不知道的事。

法线贴图 / 多光照贴图槽（specular/normal）先不接，留给阶段 H 的光照。

### Model —— assimp 加载器

`Model.h` 只前向声明 `aiNode`/`aiMesh`/`aiScene`/`aiMaterial`，不 `#include <assimp/...>`——与 `GLFWWindow.h` 前向声明 `GLFWwindow` 同样的做法，公开头不拖第三方依赖。`LoadMaterialTextures` 的纹理类型参数用 `int` 而非 `aiTextureType`，避免头文件依赖 assimp 的枚举定义。

加载流程（`Model.cpp`）：

```
Assimp::Importer::ReadFile(path, Triangulate | GenSmoothNormals | FlipUVs)
  → ProcessNode(root)  递归遍历场景节点
       └─ ProcessMesh(aiMesh)
            ├─ 提取 position / normal / texcoord（mTextureCoords[0] 判空）→ vector<Vertex>
            ├─ 展开 mFaces[i].mIndices → vector<uint32_t>
            └─ 材质贴图：先试 aiTextureType_DIFFUSE，为空再试 aiTextureType_BASE_COLOR
                （传统 OBJ/FBX 用前者，glTF PBR 材质在 assimp 里映射到后者）
                贴图路径 = m_directory + "/" + 文件名，按路径存进 m_loadedTextures 去重，
                避免同一张贴图（如多个 Mesh 共用同一贴图）被重复解码、重复传 GPU
  → 每个 aiMesh 对应一个 seed::Mesh，汇总进 m_meshes
```

`Model::Draw()` 只是遍历 `m_meshes` 逐个调用 `Mesh::Draw`——`Model` 本身不碰任何 GPU 状态。

### 测试资产与验收

Sandbox 用 Khronos 官方 [glTF-Sample-Assets](https://github.com/KhronosGroup/glTF-Sample-Assets) 的 `BoxTextured`（CC-BY 4.0，署名 Cesium，见 `Sandbox/assets/models/BoxTextured/LICENSE.md`），选分体文件版（`.gltf` + `.bin` + `.png`）而非单文件 `.glb`——因为分体版的纹理引用是外部文件路径，走标准 `Texture2D::Create(path)` 即可，不需要额外实现从内存解码贴图（`stbi_load_from_memory`）的路径。

验收：`SandboxApp` 用 `Model` 替换手写立方体顶点，运行 `./out/bin/Sandbox.exe` 可见一个贴着 Cesium 贴图的立方体持续自转，日志打印 `模型加载完成: ... (1 个 Mesh)`。

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
