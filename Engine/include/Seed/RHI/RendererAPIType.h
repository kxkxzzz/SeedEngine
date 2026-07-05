// RendererAPIType.h
#pragma once

namespace seed {

// 当前使用的渲染后端
// 所有资源的 Create() 工厂会根据它决定返回哪个后端的实现
// 现在只实现 OpenGL，Vulkan 预留位
enum class RendererAPIType
{
    None = 0,
    OpenGL,
    Vulkan
};

}  // namespace seed
