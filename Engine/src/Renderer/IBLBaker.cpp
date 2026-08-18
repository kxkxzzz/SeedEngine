// IBLBaker.cpp —— IBL 预计算实现
#include "Seed/Renderer/IBLBaker.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Seed/Core/Log.h"
#include "Seed/RHI/Buffer.h"
#include "Seed/RHI/VertexArray.h"
#include "RHI/OpenGL/OpenGLTexture.h"  // 需要访问 OpenGLTextureCube::GetRendererID()

namespace seed {

// 立方体顶点（用于渲染 cubemap 6 个面）
static float s_cubeVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// 全屏四边形顶点（用于 BRDF LUT）
static float s_quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
};

IBLTextures IBLBaker::BakeFromHDR(const std::string& hdrPath, const IBLSpecification& spec) {
    SEED_CORE_INFO("IBL 预计算开始: {}", hdrPath);

    // 1. 加载 HDR 图
    auto hdrTexture = Texture2D::Create(hdrPath, TextureFormat::RGB16F);

    // 2. Equirectangular → Cubemap
    auto envCubemap = ConvertEquirectangularToCubemap(hdrTexture, spec.EnvironmentResolution);

    // 3. 预计算辐照度图
    auto irradianceMap = GenerateIrradianceMap(envCubemap, spec.IrradianceResolution);

    // 4. 预计算预过滤环境图
    auto prefilterMap = GeneratePrefilterMap(envCubemap, spec.PrefilterResolution,
                                             spec.PrefilterMipLevels, spec.EnvironmentResolution);

    // 5. 预计算 BRDF LUT
    auto brdfLUT = GenerateBRDFLUT(spec.BRDFLUTResolution);

    SEED_CORE_INFO("IBL 预计算完成");

    return {irradianceMap, prefilterMap, brdfLUT, envCubemap};
}

std::shared_ptr<TextureCube> IBLBaker::ConvertEquirectangularToCubemap(
    const std::shared_ptr<Texture2D>& hdrTexture, uint32_t resolution) {

    SEED_CORE_INFO("转换 Equirectangular → Cubemap ({0}x{0})", resolution);

    // 带 mipmap 存储：预过滤阶段要按 roughness 采样不同 mip 级别
    auto envCubemap = TextureCube::CreateEmpty(resolution, true, true);

    // 创建 FBO
    FramebufferSpecification spec;
    spec.Width = resolution;
    spec.Height = resolution;
    auto fbo = Framebuffer::Create(spec);

    // 加载转换 shader
    auto shader = Shader::Create("Sandbox/assets/shaders/EquirectangularToCubemap.glsl");

    // Cubemap 6 个面的 view 矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // 创建立方体 VAO
    auto vbo = VertexBuffer::Create(s_cubeVertices, sizeof(s_cubeVertices));
    vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
    auto vao = VertexArray::Create();
    vao->AddVertexBuffer(vbo);

    shader->Bind();
    shader->SetInt("u_EquirectangularMap", 0);
    shader->SetMat4("u_Projection", captureProjection);
    hdrTexture->Bind(0);

    fbo->Bind();
    glViewport(0, 0, resolution, resolution);

    // 渲染到 6 个面
    for (uint32_t i = 0; i < 6; ++i) {
        shader->SetMat4("u_View", captureViews[i]);

        // 绑定 cubemap 的第 i 个面到 FBO
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               static_cast<OpenGLTextureCube*>(envCubemap.get())->GetRendererID(), 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    fbo->Unbind();

    // 给环境图生成 mipmap：预过滤时高 roughness 档的重要性采样会命中大范围方向，
    // 缺 mip 会因采样点过于稀疏而产生明显噪点
    uint32_t envID = static_cast<OpenGLTextureCube*>(envCubemap.get())->GetRendererID();
    glBindTexture(GL_TEXTURE_CUBE_MAP, envID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return envCubemap;
}

std::shared_ptr<TextureCube> IBLBaker::GenerateIrradianceMap(
    const std::shared_ptr<TextureCube>& envCubemap, uint32_t resolution) {

    SEED_CORE_INFO("预计算辐照度图 ({0}x{0})", resolution);

    auto irradianceMap = TextureCube::CreateEmpty(resolution, true, false);

    // 创建 FBO
    FramebufferSpecification spec;
    spec.Width = resolution;
    spec.Height = resolution;
    auto fbo = Framebuffer::Create(spec);

    // 加载卷积 shader
    auto shader = Shader::Create("Sandbox/assets/shaders/IrradianceConvolution.glsl");

    // Cubemap 6 个面的 view 矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // 创建立方体 VAO
    auto vbo = VertexBuffer::Create(s_cubeVertices, sizeof(s_cubeVertices));
    vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
    auto vao = VertexArray::Create();
    vao->AddVertexBuffer(vbo);

    shader->Bind();
    shader->SetInt("u_EnvironmentMap", 0);
    shader->SetMat4("u_Projection", captureProjection);
    envCubemap->Bind(0);

    fbo->Bind();
    glViewport(0, 0, resolution, resolution);

    // 渲染到 6 个面
    for (uint32_t i = 0; i < 6; ++i) {
        shader->SetMat4("u_View", captureViews[i]);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               static_cast<OpenGLTextureCube*>(irradianceMap.get())->GetRendererID(), 0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    fbo->Unbind();

    return irradianceMap;
}

std::shared_ptr<TextureCube> IBLBaker::GeneratePrefilterMap(
    const std::shared_ptr<TextureCube>& envCubemap, uint32_t resolution, uint32_t mipLevels,
    uint32_t envResolution) {

    SEED_CORE_INFO("预计算预过滤环境图 ({0}x{0}, {1} mipmap)", resolution, mipLevels);

    auto prefilterMap = TextureCube::CreateEmpty(resolution, true, true);

    // 加载预过滤 shader
    auto shader = Shader::Create("Sandbox/assets/shaders/PrefilterEnvMap.glsl");

    // Cubemap 6 个面的 view 矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // 创建立方体 VAO
    auto vbo = VertexBuffer::Create(s_cubeVertices, sizeof(s_cubeVertices));
    vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
    auto vao = VertexArray::Create();
    vao->AddVertexBuffer(vbo);

    shader->Bind();
    shader->SetInt("u_EnvironmentMap", 0);
    shader->SetMat4("u_Projection", captureProjection);
    // 预过滤 shader 需要知道源环境图的分辨率来推算该采哪一级 mip
    shader->SetFloat("u_EnvironmentResolution", (float)envResolution);
    envCubemap->Bind(0);

    // 逐级 mipmap 预过滤，mip 越高 roughness 越大
    for (uint32_t mip = 0; mip < mipLevels; ++mip) {
        // 每升一级 mip 分辨率减半（用整数位移，避免浮点 pow 的精度问题）
        uint32_t mipWidth = resolution >> mip;
        uint32_t mipHeight = resolution >> mip;

        // 创建对应分辨率的 FBO
        FramebufferSpecification spec;
        spec.Width = mipWidth;
        spec.Height = mipHeight;
        auto fbo = Framebuffer::Create(spec);

        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(mipLevels - 1);
        shader->SetFloat("u_Roughness", roughness);

        fbo->Bind();
        for (uint32_t i = 0; i < 6; ++i) {
            shader->SetMat4("u_View", captureViews[i]);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   static_cast<OpenGLTextureCube*>(prefilterMap.get())->GetRendererID(), mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            vao->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        fbo->Unbind();
    }

    return prefilterMap;
}

std::shared_ptr<Texture2D> IBLBaker::GenerateBRDFLUT(uint32_t resolution) {
    SEED_CORE_INFO("预计算 BRDF LUT ({0}x{0})", resolution);

    auto brdfLUT = Texture2D::Create(resolution, resolution,
                                     TextureFormat::RGBA16F);  // 暂用 RGBA，TODO: 加 RG16F 格式

    // LUT 是查找表，两轴分别是 NdotV 和 roughness，边缘必须 clamp。
    // 默认的 GL_REPEAT 会让 NdotV 接近 1 处绕回采到 NdotV 接近 0 的值
    uint32_t lutID = static_cast<OpenGLTexture2D*>(brdfLUT.get())->GetRendererID();
    glTextureParameteri(lutID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(lutID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 创建 FBO
    FramebufferSpecification spec;
    spec.Width = resolution;
    spec.Height = resolution;
    auto fbo = Framebuffer::Create(spec);

    // 加载 BRDF LUT shader
    auto shader = Shader::Create("Sandbox/assets/shaders/BRDFLUT.glsl");

    // 渲染全屏四边形到纹理
    fbo->Bind();
    glViewport(0, 0, resolution, resolution);

    // 把 brdfLUT 挂到 FBO 的颜色附件上，否则渲染结果会进 FBO 自带的附件，
    // brdfLUT 始终是空纹理（表现为金属球全黑：specular_IBL 恒等于 0）
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lutID, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Bind();
    RenderQuad();
    fbo->Unbind();

    return brdfLUT;
}

void IBLBaker::RenderCubeFaces(const std::shared_ptr<Shader>& shader,
                               const std::shared_ptr<Framebuffer>& fbo,
                               uint32_t resolution,
                               uint32_t mipLevel) {

    // Cubemap 6 个面的 view 矩阵
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    shader->Bind();
    shader->SetMat4("u_Projection", captureProjection);

    // 创建立方体 VAO
    auto vbo = VertexBuffer::Create(s_cubeVertices, sizeof(s_cubeVertices));
    vbo->SetLayout({{ShaderDataType::Float3, "a_Position"}});
    auto vao = VertexArray::Create();
    vao->AddVertexBuffer(vbo);

    fbo->Bind();
    glViewport(0, 0, resolution, resolution);

    for (uint32_t i = 0; i < 6; ++i) {
        shader->SetMat4("u_View", captureViews[i]);

        // 绑定 FBO 到 cubemap 的第 i 个面
        // TODO: 需要 glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        //                                     GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapID, mipLevel);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vao->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    fbo->Unbind();
}

void IBLBaker::RenderQuad() {
    static std::shared_ptr<VertexArray> s_quadVAO = nullptr;

    if (!s_quadVAO) {
        auto vbo = VertexBuffer::Create(s_quadVertices, sizeof(s_quadVertices));
        vbo->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoords"}
        });
        s_quadVAO = VertexArray::Create();
        s_quadVAO->AddVertexBuffer(vbo);
    }

    s_quadVAO->Bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

}  // namespace seed
