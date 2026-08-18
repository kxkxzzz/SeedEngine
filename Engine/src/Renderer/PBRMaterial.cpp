// PBRMaterial.cpp
#include "Seed/Renderer/PBRMaterial.h"

#include <filesystem>
#include <initializer_list>

#include "Seed/Core/Log.h"

namespace seed {

namespace {

std::shared_ptr<Texture2D> TryLoad(const std::string& dir, const std::string& base,
        std::initializer_list<const char*> suffixes, TextureFormat fmt) {
    for (const char* suffix : suffixes) {
        for (const char* ext : {".png", ".jpg", ".tga"}) {
            std::string p = dir + "/" + base + suffix + ext;
            if (std::filesystem::exists(p))
                return Texture2D::Create(p, fmt);
        }
    }
    return nullptr;
}

}  // namespace

PBRMaterial PBRMaterial::LoadFromDirectory(const std::string& dir, const std::string& base) {
    PBRMaterial m;

    // albedo is a color texture: must decode as sRGB. The rest are data
    // textures and must stay linear, otherwise values get gamma-skewed.
    m.AlbedoMap = TryLoad(dir, base, {"_albedo", "_basecolor", "_baseColor"},
                          TextureFormat::SRGBA8);
    m.NormalMap = TryLoad(dir, base, {"_normal-ogl", "_normal"}, TextureFormat::RGB8);
    m.MetallicMap = TryLoad(dir, base, {"_metallic"}, TextureFormat::RGB8);
    m.RoughnessMap = TryLoad(dir, base, {"_roughness"}, TextureFormat::RGB8);
    m.AOMap = TryLoad(dir, base, {"_ao"}, TextureFormat::RGB8);

    return m;
}

void PBRMaterial::Bind(const std::shared_ptr<Shader>& shader) const {
    // Scalar fallbacks, used wherever a map is absent.
    shader->SetFloat3("u_Albedo", Albedo);
    shader->SetFloat("u_Metallic", Metallic);
    shader->SetFloat("u_Roughness", Roughness);
    shader->SetFloat("u_AO", AO);

    auto bindMap = [&shader](const std::shared_ptr<Texture2D>& tex, uint32_t slot,
                             const char* sampler, const char* flag) {
        if (tex) {
            tex->Bind(slot);
            shader->SetInt(sampler, static_cast<int>(slot));
            shader->SetInt(flag, 1);
        }
        else {
            shader->SetInt(flag, 0);
        }
    };

    // Slots 0-4; the IBL maps live at 5-7 (see SandboxApp).
    bindMap(AlbedoMap, 0, "u_AlbedoMap", "u_HasAlbedoMap");
    bindMap(NormalMap, 1, "u_NormalMap", "u_HasNormalMap");
    bindMap(MetallicMap, 2, "u_MetallicMap", "u_HasMetallicMap");
    bindMap(RoughnessMap, 3, "u_RoughnessMap", "u_HasRoughnessMap");
    bindMap(AOMap, 4, "u_AOMap", "u_HasAOMap");
}

}  // namespace seed
