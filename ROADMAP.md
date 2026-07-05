# SeedEngine 实现路线

> 定位：基于 OpenGL 的迷你游戏引擎，主线 = **架构 + 3D 渲染（PBR）+ 编辑器**。
> 参考：Hazel（架构/Layer/RHI）、LearnOpenGL（3D/PBR 技术）、Games104（理念/模块边界）。
>
> 已确认的方向：
> 1. **RHI 抽象层**：按 Vulkan 心智模型设计接口，先只实现 OpenGL 后端。
> 2. **优先架构地基**：先做 Layer 栈、事件、日志/断言、RHI 抽象，再做渲染画面。
> 3. **重心放在 3D 渲染**：尽可能多实现 3D 渲染技术，目标做到 PBR + IBL + 阴影 + 后处理。
> 4. Renderer2D 批渲染、物理/音频/网络**跳过或只做 demo**，不投入主线时间。
>
> **进度**：阶段 A-D 已完成，阶段 E（RHI）进行中。

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

### 阶段 F — Renderer + 3D 相机（2-3 天）
- **做什么**：`Renderer` 持有场景级数据（view-projection 矩阵），`BeginScene/Submit/EndScene`；直接做 `PerspectiveCamera`（透视）+ `CameraController`（轨道/FPS 风格，鼠标+WASD）。
- **验收**：透视相机能自由移动/旋转，三角形/立方体随相机变换，深度测试正确。
- **面试点**：渲染指令的收集与提交模型、MVP 矩阵、透视投影。

> **跳过原 Renderer2D 批渲染阶段**——那是 2D 专属优化，不在 3D/PBR 主线上。如以后想补充，可作为独立分支。

### 阶段 G — Mesh + 模型加载（2-3 天）
- **做什么**：`Mesh`（顶点含 位置/法线/UV/切线 + 索引）；`Texture2D` 用 stb_image 加载图片；assimp 加载 .obj/.gltf 模型转成 `Mesh`。
- **验收**：加载一个带贴图的 3D 模型（如经典的 backpack/nanosuit）并显示。
- **面试点**：顶点属性布局、模型数据组织、资源加载管线。

### 阶段 H — 光照（Blinn-Phong）（2-3 天）
- **做什么**：`Light`（方向光/点光源/聚光灯）；Blinn-Phong 着色（环境+漫反射+高光）；多光源；法线贴图（切线空间）。
- **验收**：模型在多光源下有正确的明暗、高光、法线细节。
- **面试点**：光照方程、切线空间、为什么 Blinn-Phong 优于 Phong。这是理解 PBR 的必经之路。

### 阶段 I — PBR（核心目标，4-6 天）
- **做什么**：
  - Cook-Torrance BRDF（金属度/粗糙度工作流）
  - 材质系统：Albedo/Metallic/Roughness/AO/Normal 贴图组合
  - IBL 环境光照：HDR 环境贴图 → 辐照度图（漫反射）+ 预过滤环境图 + BRDF LUT（镜面）
  - 天空盒（立方体贴图）
- **验收**：金属/非金属材质球阵列在 HDR 环境下有真实的反射和光照。
- **面试点**：PBR 是图形岗位的核心考点，能讲 BRDF、能量守恒、IBL 预计算。**这是简历的最大亮点。**

### 阶段 J — 阴影（2-4 天）
- **做什么**：Shadow Mapping（方向光深度图）→ PCF 软阴影 →（进阶）CSM 级联阴影。
- **验收**：物体投射出带软边缘的阴影。
- **面试点**：深度图、阴影失真（bias）、PCF 采样、级联划分。

### 阶段 K — 后处理管线（2-4 天）
- **做什么**：`Framebuffer` 离屏渲染 → HDR + Tonemapping + Gamma 校正 → Bloom（泛光）→（进阶）SSAO / FXAA。
- **验收**：画面有 HDR 高光溢出（Bloom）、正确的色调映射，观感明显提升。
- **面试点**：HDR 管线、后处理链、屏幕空间技术。

### 阶段 L — ImGui 编辑器（2-3 天）
- **做什么**：`ImGuiLayer`（imgui docking 分支）；场景渲染到 `Framebuffer`，在 ImGui `Image` 里显示为 Viewport；材质/光照参数面板实时调节。
- **验收**：可停靠面板的编辑器，Viewport 显示 3D 场景，右侧面板能实时改材质/光照参数看效果。
- **面试点**：渲染到纹理 + 编辑器 = 最直观的"这是个引擎"证据，**截图放简历**。

### 阶段 M — ECS 场景系统（可选，2-4 天）
- **做什么**：entt 封装 `Scene` + `Entity`；`Transform`/`MeshRenderer`/`Light`/`Camera` 组件；`Scene::OnUpdate` 遍历实体提交渲染；Hierarchy + Inspector 面板；场景序列化。
- **验收**：编辑器里增删 3D 实体、改 Transform/材质、保存加载场景。
- **面试点**：ECS 数据布局、组件化设计、序列化。

### 进阶技术池（有精力再冲，都是简历亮点）
- **延迟渲染（Deferred Shading）** — G-Buffer，支持大量光源
- **SSR 屏幕空间反射**
- **体积光 / 大气散射**
- **GPU 实例化** — 大量相同物体一次绘制
- **视锥剔除（Frustum Culling）** — 性能优化

---

## 3. 里程碑节奏建议

- **M1（地基可演示）✅**：阶段 A-D 完成 → 窗口、日志、Layer 栈、事件。
- **M2（渲染起步）**：阶段 E-G 完成 → RHI 画三角形 → 3D 相机 → 加载显示模型。
- **M3（光照与 PBR）**：阶段 H-I 完成 → Blinn-Phong → **PBR + IBL**。此时简历已有强力亮点。
- **M4（画面完整度）**：阶段 J-L 完成 → 阴影 + 后处理 + 编辑器。**截图非常出彩。**
- **M5（可选深化）**：阶段 M（ECS）或进阶技术池挑 1-2 项。

求职最低交付建议做到 **M3（PBR）**，理想到 **M4**。

---

## 4. 给面试官讲故事的主线（提前想好）

1. "我用 Layer 栈解耦了引擎层、编辑器层、ImGui 层" —— 架构。
2. "我做了后端无关的 RHI，OpenGL 是其中一个实现，接口按显式管线设计以便扩 Vulkan" —— 抽象能力。
3. "我实现了完整的 PBR 管线：Cook-Torrance BRDF + IBL 环境光照（辐照度图/预过滤图/BRDF LUT）" —— **图形深度核心**。
4. "阴影用 Shadow Mapping + PCF，后处理有 HDR/Tonemapping/Bloom" —— 渲染完整度。
5. "场景用 ECS，材质光照可在编辑器里实时调节" —— 工程完整度。

---

## 5. 立即可做的第一步（等你发话）

最小改造顺序（阶段 A + 局部 D）：
1. 配 `ThirdParty/`：glfw、glad、glm、spdlog 四个先到位。
2. 理顺根 CMake：`Framework` → 静态库；新增 `Core/` 目录放 `Application`/`Layer`/`EntryPoint`/`Log`。
3. 补全 `WindowGLFW::Create`（真正 `glfwCreateWindow` + 设回调 + glad 加载）。
4. 写最小 `Application` + `EntryPoint`，Sandbox 出一个清屏窗口。

> 当你准备好，我就从这第一步开始动手改代码。
