// IBLBaker.h —— IBL 预计算管线（HDR 环境贴图 → 辐照度图 + 预过滤图 + BRDF LUT）
#pragma once

#include <memory>
#include <string>

#include "Seed/RHI/Framebuffer.h"
#include "Seed/RHI/Shader.h"
#include "Seed/RHI/Texture.h"

namespace seed {

// IBL 预计算的分辨率配置
// 各项独立可调：环境图影响 skybox 清晰度，辐照度图/预过滤图影响光照质量
struct IBLSpecification {
    // 环境 cubemap 单面分辨率。skybox 直接采样它，所以它决定背景清晰度
    uint32_t EnvironmentResolution = 512;
    // 辐照度图单面分辨率。漫反射是大范围卷积，32 足够，调大只会变慢
    uint32_t IrradianceResolution = 32;
    // 预过滤图 mip 0 的单面分辨率，逐级减半
    uint32_t PrefilterResolution = 128;
    // 预过滤的 mipmap 级数，对应 roughness 从 0 到 1 采样 N 档
    uint32_t PrefilterMipLevels = 5;
    // BRDF 查找纹理分辨率
    uint32_t BRDFLUTResolution = 512;
};

// IBL 预计算结果：3 张贴图
struct IBLTextures {
    std::shared_ptr<TextureCube> IrradianceMap;   // 辐照度图（漫反射 IBL）
    std::shared_ptr<TextureCube> PrefilterMap;    // 预过滤环境图（镜面反射 IBL），带 mipmap
    std::shared_ptr<Texture2D> BRDFLUT;           // BRDF 查找纹理
    std::shared_ptr<TextureCube> EnvironmentMap;  // 转换后的环境 cubemap，供 skybox 直接使用
};

// IBL 预计算工具类
class IBLBaker {
public:
    // 从 HDR equirectangular 图一次性预计算全部 IBL 贴图
    static IBLTextures BakeFromHDR(const std::string& hdrPath,
                                   const IBLSpecification& spec = {});

    // 1. Equirectangular HDR → Cubemap
    static std::shared_ptr<TextureCube> ConvertEquirectangularToCubemap(
        const std::shared_ptr<Texture2D>& hdrTexture, uint32_t resolution = 512);

    // 2. 预计算辐照度图（卷积）
    static std::shared_ptr<TextureCube> GenerateIrradianceMap(
        const std::shared_ptr<TextureCube>& envCubemap, uint32_t resolution = 32);

    // 3. 预计算预过滤环境图（按 roughness 分 mipmap）
    // envResolution 是源环境图的单面分辨率，shader 用它推算采样时该取哪一级 mip
    static std::shared_ptr<TextureCube> GeneratePrefilterMap(
        const std::shared_ptr<TextureCube>& envCubemap, uint32_t resolution = 128,
        uint32_t mipLevels = 5, uint32_t envResolution = 512);

    // 4. 预计算 BRDF LUT
    static std::shared_ptr<Texture2D> GenerateBRDFLUT(uint32_t resolution = 512);

private:
    // 工具：渲染立方体的 6 个面
    static void RenderCubeFaces(const std::shared_ptr<Shader>& shader,
                                const std::shared_ptr<Framebuffer>& fbo,
                                uint32_t resolution,
                                uint32_t mipLevel = 0);

    // 工具：渲染全屏四边形（用于 BRDF LUT）
    static void RenderQuad();
};

}  // namespace seed
