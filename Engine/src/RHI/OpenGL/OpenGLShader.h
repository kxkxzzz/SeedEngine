// OpenGLShader.h —— Shader 的 OpenGL 实现
#pragma once

#include <string>
#include <unordered_map>

#include "Seed/RHI/Shader.h"

namespace seed {

class OpenGLShader : public Shader {
public:
    explicit OpenGLShader(const std::string& filepath);
    OpenGLShader(const std::string& name, const std::string& vertexSrc,
                 const std::string& fragmentSrc);
    ~OpenGLShader() override;

    void Bind() const override;
    void Unbind() const override;

    void SetInt(const std::string& name, int value) override;
    void SetFloat(const std::string& name, float value) override;
    void SetFloat3(const std::string& name, const glm::vec3& value) override;
    void SetFloat4(const std::string& name, const glm::vec4& value) override;
    void SetMat4(const std::string& name, const glm::mat4& value) override;

    const std::string& GetName() const override {
        return m_name;
    }

private:
    // 读文件为字符串
    std::string ReadFile(const std::string& filepath);
    // 按 #type 标记把源码拆成 vertex/fragment 两段
    std::unordered_map<uint32_t, std::string> PreProcess(const std::string& source);
    // 编译并链接各着色器段
    void Compile(const std::unordered_map<uint32_t, std::string>& shaderSources);
    // 查询 uniform location（可加缓存）
    int GetUniformLocation(const std::string& name);

    uint32_t m_rendererID = 0;  // OpenGL program 句柄
    std::string m_name;
    std::unordered_map<std::string, int> m_uniformLocationCache;
};

}  // namespace seed
