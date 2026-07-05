// Buffer.h —— 顶点缓冲、索引缓冲与顶点布局
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Seed/Core/Assert.h"

namespace seed {

// 顶点属性的数据类型
// GPU 顶点数据由多个属性组成（位置/颜色/UV...），每个属性是这些类型之一
enum class ShaderDataType
{
    None = 0,
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Int,
    Int2,
    Int3,
    Int4,
    Bool
};

// 返回某个 ShaderDataType 占用的字节数
inline uint32_t ShaderDataTypeSize(ShaderDataType type) {
    switch (type) {
        case ShaderDataType::Float:
            return 4;
        case ShaderDataType::Float2:
            return 4 * 2;
        case ShaderDataType::Float3:
            return 4 * 3;
        case ShaderDataType::Float4:
            return 4 * 4;
        case ShaderDataType::Mat3:
            return 4 * 3 * 3;
        case ShaderDataType::Mat4:
            return 4 * 4 * 4;
        case ShaderDataType::Int:
            return 4;
        case ShaderDataType::Int2:
            return 4 * 2;
        case ShaderDataType::Int3:
            return 4 * 3;
        case ShaderDataType::Int4:
            return 4 * 4;
        case ShaderDataType::Bool:
            return 1;
        default:
            return 0;
    }
}

// 顶点布局中的一个属性（一列数据）
// e.g. 位置就是一个 Float3 元素，颜色是一个 Float4 元素
struct BufferElement {
    std::string Name;
    ShaderDataType Type = ShaderDataType::None;
    uint32_t Size = 0;  // 本属性字节数
    uint32_t Offset = 0;  // 在单个顶点内的字节偏移，由 BufferLayout 计算
    bool Normalized = false;  // 整数是否归一化到 [0,1]/[-1,1]

    BufferElement() = default;
    BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
        : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Normalized(normalized) {
    }

    // 本属性包含几个分量，如 Float3 -> 3
    uint32_t GetComponentCount() const {
        switch (Type) {
            case ShaderDataType::Float:
                return 1;
            case ShaderDataType::Float2:
                return 2;
            case ShaderDataType::Float3:
                return 3;
            case ShaderDataType::Float4:
                return 4;
            case ShaderDataType::Mat3:
                return 3 * 3;
            case ShaderDataType::Mat4:
                return 4 * 4;
            case ShaderDataType::Int:
                return 1;
            case ShaderDataType::Int2:
                return 2;
            case ShaderDataType::Int3:
                return 3;
            case ShaderDataType::Int4:
                return 4;
            case ShaderDataType::Bool:
                return 1;
            default:
                return 0;
        }
    }
};

// 记录一个顶点由哪些属性按什么顺序组成
// 构造时自动算出每个属性的 Offset 和整体 Stride
class BufferLayout {
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> elements) : m_elements(elements) {
        CalculateOffsetsAndStride();
    }

    uint32_t GetStride() const {
        return m_stride;
    }
    const std::vector<BufferElement>& GetElements() const {
        return m_elements;
    }

    std::vector<BufferElement>::iterator begin() {
        return m_elements.begin();
    }
    std::vector<BufferElement>::iterator end() {
        return m_elements.end();
    }
    std::vector<BufferElement>::const_iterator begin() const {
        return m_elements.begin();
    }
    std::vector<BufferElement>::const_iterator end() const {
        return m_elements.end();
    }

private:
    // 顺序累加每个属性的字节数：前者的末尾即后者的偏移，总和即步长
    void CalculateOffsetsAndStride() {
        uint32_t offset = 0;
        m_stride = 0;
        for (auto& element : m_elements) {
            element.Offset = offset;
            offset += element.Size;
            m_stride += element.Size;
        }
    }

    std::vector<BufferElement> m_elements;
    uint32_t m_stride = 0;  // 单个顶点的总字节数
};

// 存放顶点数据的 GPU 缓冲
// VBO
class VertexBuffer {
public:
    virtual ~VertexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    // 动态更新缓冲数据（批渲染时用）
    virtual void SetData(const void* data, uint32_t size) = 0;

    virtual const BufferLayout& GetLayout() const = 0;
    virtual void SetLayout(const BufferLayout& layout) = 0;

    // 工厂：用已有顶点数据创建（静态网格）
    static std::shared_ptr<VertexBuffer> Create(const float* vertices, uint32_t size);
    // 工厂：只分配空缓冲，之后用 SetData 填（动态）
    static std::shared_ptr<VertexBuffer> Create(uint32_t size);
};

// 索引缓冲：存放顶点索引，让顶点可复用，减少数据量
class IndexBuffer {
public:
    virtual ~IndexBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual uint32_t GetCount() const = 0;  // 索引个数

    static std::shared_ptr<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
};

}  // namespace seed
