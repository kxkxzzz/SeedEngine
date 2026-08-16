// Shader.h —— 着色器程序
#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace seed {

// shader：编译 GLSL 顶点/片段着色器，绑定后用于绘制
// SetXxx(): 向着色器传 uniform 参数（如变换矩阵、颜色）
class Shader {
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
    virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
    virtual void SetMat3(const std::string& name, const glm::mat3& value) = 0;
    virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

    virtual const std::string& GetName() const = 0;

    // 从单个文件加载（文件内用 #type 标记区分vertex shader和fragment shader）
    // #type vertex - vertex shader
    // #type fragment - fragment shader
    static std::shared_ptr<Shader> Create(const std::string& filepath);
    // 直接用源码字符串创建
    static std::shared_ptr<Shader> Create(const std::string& name, const std::string& vertexSrc,
                                          const std::string& fragmentSrc);
};

}  // namespace seed
