// OpenGLShader.cpp
#include "RHI/OpenGL/OpenGLShader.h"

#include <cstring>
#include <fstream>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "Seed/Core/Assert.h"
#include "Seed/Core/Log.h"

namespace seed {

// 把 "#type vertex" / "#type fragment" 里的类型名转成 GL 着色器类型
static GLenum ShaderTypeFromString(const std::string& type) {
    if (type == "vertex")
        return GL_VERTEX_SHADER;
    if (type == "fragment" || type == "pixel")
        return GL_FRAGMENT_SHADER;
    SEED_CORE_ASSERT(false, "未知的 shader 类型");
    return 0;
}

OpenGLShader::OpenGLShader(const std::string& filepath) {
    std::string source = ReadFile(filepath);
    auto shaderSources = PreProcess(source);
    Compile(shaderSources);

    m_name = filepath.substr(filepath.find_last_of("/\\") + 1);
    m_name = m_name.substr(0, m_name.find_last_of("."));
}

OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc,
                           const std::string& fragmentSrc)
    : m_name(name) {
    auto shaderSources = std::unordered_map<uint32_t, std::string>{
        {GL_VERTEX_SHADER, vertexSrc},
        {GL_FRAGMENT_SHADER, fragmentSrc},
    };
    Compile(shaderSources);
}

OpenGLShader::~OpenGLShader() {
    glDeleteProgram(m_rendererID);
}

void OpenGLShader::Bind() const {
    glUseProgram(m_rendererID);
}

void OpenGLShader::Unbind() const {
    glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void OpenGLShader::SetFloat(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) {
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

// ---- 辅助函数 ----

std::string OpenGLShader::ReadFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        SEED_CORE_ERROR("Failed to open file: {}", filepath);
        return {};
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
}

std::unordered_map<uint32_t, std::string> OpenGLShader::PreProcess(const std::string& source) {
    std::unordered_map<uint32_t, std::string> shaderSources;

    const char* typeToken = "#type";
    size_t typeTokenLength = strlen(typeToken);

    // 从头找第一个 #type
    size_t pos = source.find(typeToken, 0);
    while (pos != std::string::npos) {
        // 本行结尾（#type 这一行的换行位置）
        size_t eol = source.find_first_of("\r\n", pos);
        SEED_CORE_ASSERT(eol != std::string::npos, "Shader 语法错误：#type 后缺少内容");

        // 取 #type 后面的类型名，如 "vertex"
        size_t begin = pos + typeTokenLength + 1;  // +1 跳过空格
        std::string type = source.substr(begin, eol - begin);
        SEED_CORE_ASSERT(ShaderTypeFromString(type), "无效的 shader 类型");

        // 该段源码从本行的下一行开始
        size_t nextLinePos = source.find_first_not_of("\r\n", eol);
        // 找下一个 #type，作为本段的结束
        pos = source.find(typeToken, nextLinePos);

        // 截取 [nextLinePos, 下一个#type) 之间的内容作为本段源码
        shaderSources[ShaderTypeFromString(type)] =
            (nextLinePos == std::string::npos)
                ? std::string()
                : source.substr(nextLinePos, (pos == std::string::npos)
                                                 ? source.size() - nextLinePos
                                                 : pos - nextLinePos);
    }

    return shaderSources;
}

void OpenGLShader::Compile(const std::unordered_map<uint32_t, std::string>& shaderSources) {
    GLuint program = glCreateProgram();
    std::vector<GLuint> shaders;
    for (auto& [type, src] : shaderSources) {
        GLuint shader = glCreateShader(type);
        const char* c = src.c_str();
        glShaderSource(shader, 1, &c, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            SEED_CORE_ERROR("Shader compilation failed: {}", infoLog);
        }

        glAttachShader(program, shader);
        shaders.emplace_back(shader);
    }
    glLinkProgram(program);
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        SEED_CORE_ERROR("Shader linking failed: {}", infoLog);
    }

    // 删除无用的 shader
    for (auto shader : shaders) {
        glDetachShader(program, shader);
        glDeleteShader(shader);
    }
    m_rendererID = program;
}

int OpenGLShader::GetUniformLocation(const std::string& name) {
    auto it = m_uniformLocationCache.find(name);
    if (it != m_uniformLocationCache.end()) {
        return it->second;
    }
    m_uniformLocationCache[name] = glGetUniformLocation(m_rendererID, name.c_str());
    return m_uniformLocationCache[name];
}

}  // namespace seed
