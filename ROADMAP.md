# SeedEngine 实现路线

> 定位：基于 OpenGL 的迷你游戏引擎，主线 = **架构 + 渲染 + 编辑器**。
> 参考：Hazel（架构/Layer/RHI）、Games104（理念/模块边界）、GameEngineFromScratch（你现有的 IRuntimeModule 骨架）。
>
> 已确认的三个方向：
> 1. **RHI 抽象层**：按 Vulkan 心智模型设计接口，先只实现 OpenGL 后端。
> 2. **优先架构地基**：先做 Layer 栈、事件、日志/断言、RHI 抽象，再做渲染画面。
> 3. 物理/音频/网络只做 demo，不投入主线时间。

---

## 0. 命名与目录目标形态

**命名空间**：统一用 `seed`（全小写，与项目名一致；符合 std/glm/entt 等现代库惯例）。
原 `namespace kx` 已全部替换。地基期保持单层 `seed::`，RHI 阶段视情况再加 `seed::rhi::` 等二级。

```
SeedEngine/
├── Framework/
│   ├── Core/          # Application、Layer/LayerStack、EntryPoint、Timestep、Log、Assert
│   ├── Event/         # Event 基类 + Window/Key/Mouse 事件
│   ├── Platform/      # Window(GLFW)、Input、OpenGLContext
│   ├── RHI/           # 渲染硬件接口抽象（RendererAPI / Buffer / Shader / Texture / Pipeline）
│   │   └── OpenGL/    # OpenGL 后端实现
│   ├── Renderer/      # 高层：Renderer / Renderer2D / Camera / Mesh / Material / Light
│   ├── Scene/         # ECS(entt) / Components / SceneSerializer
│   ├── Resource/      # Texture/Shader/Model 加载（stb_image / assimp）
│   └── ImGui/         # ImGuiLayer 集成
├── Editor/            # 编辑器（Viewport / Hierarchy / Inspector 面板）
├── Sandbox/           # 现在的 Empty 改名，做测试场景
└── ThirdParty/        # git submodule: glfw glad glm imgui entt spdlog stb assimp
```

你现有 `Framework/Common` + `Framework/Interface` + `Framework/Graphics` 会逐步迁移：
- `IRuntimeModule`（Initialize/Tick/Shutdown）思想**保留**，用于各 Manager。
- 主循环从 `g_pApp` 全局指针 + 裸 while，升级为 **Application + Layer 栈**。
- `WindowGLFW` 已经是对的方向，迁到 `Platform/`，补全 `Create` 里真正的窗口创建。

---

## 1. 第三方库（一次性配好，后面省心）

用 git submodule 统一放 `ThirdParty/`。优先选 header-only / CMake 友好的：

| 库 | 用途 | 接入方式 |
|---|---|---|
| glfw | 窗口/输入 | submodule，`add_subdirectory` |
| glad | OpenGL 函数加载 | 生成一份 OpenGL 4.5 Core 的源码丢进去编译 |
| glm | 数学（向量/矩阵） | header-only submodule |
| spdlog | 日志 | header-only |
| imgui | 编辑器 UI | 源码直接编进一个 lib，用 docking 分支 |
| entt | ECS | header-only single header |
| stb_image | 贴图加载 | 单头文件 |
| assimp | 模型加载（3D 阶段才需要） | submodule，编译较慢，放后面 |

CMake 建议：根 `CMakeLists.txt` 用 `option()` 控制是否启用 assimp 等重库；把 `Framework` 编成静态库 `SeedEngine`，`Editor`/`Sandbox` 链接它。

---

## 2. 实现阶段（按你"优先架构地基"的选择排序）

每个阶段给出：**做什么 / 关键类型 / 验收标准 / 面试点**。

### 阶段 A — 构建系统与窗口（地基，约 2-3 天）
- **做什么**：理顺 CMake；接 glfw + glad；`WindowGLFW::Create` 真正创建窗口和 OpenGL Context；清屏成一个颜色。
- **关键类型**：`Window`（你已有 `IWindow`/`WindowGLFW`）、`GraphicsContext`（封装 glad 初始化 + SwapBuffers）。
- **验收**：跑 Sandbox 出现一个 1280×720 蓝色窗口，能关闭。
- **面试点**：能讲清窗口/上下文/交换链的关系。

### 阶段 B — Log + Assert（半天）
- **做什么**：spdlog 包一层 `KX_CORE_INFO` / `KX_INFO` 宏；`KX_ASSERT` 宏（Debug 下 break）。
- **验收**：日志带颜色和分类（Core/Client）。
- **面试点**：小，但体现工程习惯。

### 阶段 C — 事件系统（1-2 天，Hazel 核心）
- **做什么**：`Event` 基类 + `EventType`/`EventCategory`；`WindowCloseEvent`/`WindowResizeEvent`/`KeyPressed`/`MouseMoved` 等；`EventDispatcher`。
- **关键设计**：GLFW 回调里**构造 Event 对象**，通过 `Window` 上设置的回调 `std::function<void(Event&)>` 上抛给 Application。
- **验收**：窗口 resize / 关闭 / 按键都打印对应事件日志。
- **面试点**：解耦平台层和应用层的标准做法，必讲。

### 阶段 D — Application + Layer 栈（1-2 天，整个引擎的脊柱）
- **做什么**：`Application` 持有 Window、LayerStack；主循环 = 算 `Timestep` → 遍历 Layer `OnUpdate` → Window `OnUpdate`；事件从 Window 流入 Application，再**从栈顶向下**分发给 Layer。
- **关键类型**：`Layer`（OnAttach/OnDetach/OnUpdate/OnEvent/OnImGuiRender）、`LayerStack`、`Timestep`、`EntryPoint`（提供 `main`，调用客户端 `CreateApplication()`）。
- **取舍**：用 `Application::Get()` 单例替代你现在的 `g_pApp` extern。客户端只写一个 `class Sandbox : public Application` 和 `CreateApplication()`。
- **验收**：Sandbox 里 push 一个 `ExampleLayer`，每帧打印 dt。
- **面试点**：Layer 栈让 Editor/ImGui/游戏逻辑互不耦合，是 Hazel 架构精髓。

### 阶段 E — RHI 抽象层（2-4 天，你选的加分项核心）
- **做什么**：定义后端无关接口，OpenGL 作为一个实现。按 Vulkan 心智设计：资源创建与绑定分离、显式状态。
- **关键接口**（先做这些，够用且能扩 Vulkan）：
  - `RendererAPI`：`Init / SetViewport / SetClearColor / Clear / DrawIndexed`，带 `enum class API { None, OpenGL, Vulkan }`。
  - `VertexBuffer` / `IndexBuffer` + `BufferLayout`（元素类型、stride、offset，自动算）。
  - `VertexArray`（OpenGL 是 VAO；Vulkan 阶段对应 pipeline 的 vertex input state）。
  - `Shader`：从文件/字符串编译，`SetUniform*`（OpenGL）；预留 `ShaderReflection`。
  - `Texture2D`：`Create(width,height)` / `Create(path)` / `Bind(slot)`。
  - `Framebuffer`：颜色/深度附件——**编辑器 Viewport 渲染到纹理要用**。
  - （预留 Vulkan 的关键：`Pipeline` / `RenderPass` 接口可以先定义空壳，OpenGL 后端用状态机模拟。）
- **关键设计**：每个接口提供静态 `Create(...)`，内部按当前 `RendererAPI::API` switch 返回对应后端的实现（`std::unique_ptr`）。
- **验收**：用 RHI 接口画出一个彩色三角形（不直接调任何 `gl*`，全走抽象）。
- **面试点**：这是"懂引擎"和"会 OpenGL"的分水岭。讲清"为什么 BufferLayout 自动算 offset""为什么资源创建/绑定分离""OpenGL 后端如何模拟 Vulkan 的显式 pipeline"。

### 阶段 F — Renderer + Camera（2-3 天）
- **做什么**：`Renderer` 持有"场景级"数据（view-projection 矩阵），`BeginScene/Submit/EndScene`；`OrthographicCamera`（先正交）+ `CameraController`（WASD/缩放）。
- **验收**：相机能移动，三角形随相机变换。
- **面试点**：渲染指令的收集与提交模型。

### 阶段 G — Renderer2D 批渲染（2-3 天，性价比极高）
- **做什么**：`Renderer2D::DrawQuad`，内部做 **batching**（一个大 VBO 累积顶点，纹理 slot 数组，凑满或 EndScene 时一次 DrawIndexed）。
- **验收**：屏幕上几千个带纹理的 quad，帧率正常；统计 draw call 数。
- **面试点**：批渲染是 2D 引擎性能必考点，能讲 draw call 合并、纹理槽管理。

### 阶段 H — ImGui 集成 + 编辑器雏形（2-3 天）
- **做什么**：`ImGuiLayer`（接 imgui glfw+opengl3 backend，开 docking）；Editor 里把场景渲染到 `Framebuffer`，在 ImGui `Image` 控件里显示为 Viewport 面板。
- **验收**：编辑器有可停靠面板，Viewport 显示渲染结果，有个统计面板显示帧率/draw call。
- **面试点**：渲染到纹理 + 编辑器 = 最直观的"这是个引擎"证据，截图放简历。

### 阶段 I — ECS 场景系统（2-4 天，Games104 理念落地）
- **做什么**：entt 封装 `Scene` + `Entity`；`TransformComponent`/`SpriteRendererComponent`/`TagComponent`/`CameraComponent`；`Scene::OnUpdate` 遍历有 Transform+Sprite 的实体提交渲染。
- **进阶**：`SceneHierarchyPanel`（左侧实体列表）+ `Inspector`（右侧改组件属性）；`SceneSerializer`（yaml 存读场景）。
- **验收**：编辑器里增删实体、拖动 Transform 实时看到 quad 移动、保存/加载场景文件。
- **面试点**：ECS 的数据布局优势（cache 友好）、为什么用 entt、组件序列化。

### 阶段 J — 3D 渲染（按精力扩展，最能体现图形深度）
顺序：`PerspectiveCamera` → assimp 加载 mesh → Blinn-Phong 光照 → 方向光阴影（shadow map）→ PBR（金属/粗糙度 + IBL）→ 后处理（HDR/Bloom/Tonemapping）。
每加一项都能在编辑器里截图，且都是图形面试高频题。

---

## 3. 里程碑节奏建议

- **M1（地基可演示）**：阶段 A-D 完成 → 有窗口、有日志、有 Layer 栈、事件能跑。
- **M2（看得见的引擎）**：阶段 E-H 完成 → RHI 画三角形 → 2D 批渲染 → ImGui 编辑器 Viewport。**此时已能写进简历并截图。**
- **M3（像个引擎）**：阶段 I 完成 → ECS + 场景编辑/序列化。
- **M4（图形深度）**：阶段 J 挑 2-3 个做（推荐：模型加载 + Blinn-Phong + 阴影，或直接冲 PBR）。

求职最低交付建议做到 **M2**，理想到 **M3 + M4 的一两项**。

---

## 4. 给面试官讲故事的主线（提前想好）

1. "我用 Layer 栈解耦了引擎层、编辑器层、ImGui 层" —— 架构。
2. "我做了后端无关的 RHI，OpenGL 是其中一个实现，接口按显式管线设计以便扩 Vulkan" —— 抽象能力。
3. "2D 渲染器用批处理把 N 个 quad 合并成一次 draw call" —— 性能。
4. "场景用 ECS，组件可序列化，编辑器实时编辑" —— 工程完整度。
5. "3D 部分实现了 XXX（阴影/PBR）" —— 图形深度。

---

## 5. 立即可做的第一步（等你发话）

最小改造顺序（阶段 A + 局部 D）：
1. 配 `ThirdParty/`：glfw、glad、glm、spdlog 四个先到位。
2. 理顺根 CMake：`Framework` → 静态库；新增 `Core/` 目录放 `Application`/`Layer`/`EntryPoint`/`Log`。
3. 补全 `WindowGLFW::Create`（真正 `glfwCreateWindow` + 设回调 + glad 加载）。
4. 写最小 `Application` + `EntryPoint`，Sandbox 出一个清屏窗口。

> 当你准备好，我就从这第一步开始动手改代码。
