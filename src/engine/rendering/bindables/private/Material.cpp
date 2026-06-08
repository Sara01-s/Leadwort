#include "../public/Material.h"

#include "engine/rendering/bindables/public/CubeMap.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Logger.h"

#include <memory>

namespace Engine::Rendering::Bindables {

using Color = Utils::Color;

Material::Material(const std::shared_ptr<Shader>& shader) : m_Shader(shader) {
    SetColor4("_Color", Color::White());
}

Material::Material(Shader&& shader) : m_Shader(&shader) {
	SetColor3("_Color", Color::White());
}

// ─────────────────────────────────────────────
//  Setters
// ─────────────────────────────────────────────

void Material::SetInt(const std::string& name, const int value)          { m_Ints[name]    = value; }
void Material::SetFloat(const std::string& name, const float value)      { m_Floats[name]  = value; }
void Material::SetVec3(const std::string& name, const glm::vec3& value)  { m_Vec3s[name]   = value; }
void Material::SetVec4(const std::string& name, const glm::vec4& value)  { m_Vec4s[name]   = value; }
void Material::SetMat3(const std::string& name, const glm::mat3& value)  { m_Mat3s[name]   = value; }
void Material::SetMat4(const std::string& name, const glm::mat4& value)  { m_Mat4s[name]   = value; }

void Material::SetTexture(const std::string& name, Texture* texture) {
    m_Textures[name] = TextureSlot { texture, 0, GL_TEXTURE_2D, -1 };
}

void Material::SetTexture(const std::string& name, Texture* texture, const int slot) {
    m_Textures[name] = TextureSlot { texture, 0, GL_TEXTURE_2D, slot };
}

void Material::SetTexture(const std::string& name, const uint32_t gpuID, const int slot, const uint32_t target) {
    m_Textures[name] = TextureSlot { nullptr, gpuID, target, slot };
}

void Material::SetCubeMap(const std::string& name, const CubeMap* cubeMap) {
	m_Textures[name] = TextureSlot { nullptr, cubeMap->GetGpuID(), GL_TEXTURE_CUBE_MAP, -1 };
}

void Material::SetCubeMap(const std::string& name, const CubeMap* cubeMap, const int slot) {
	m_Textures[name] = TextureSlot { nullptr, cubeMap->GetGpuID(), GL_TEXTURE_CUBE_MAP, slot };
}

void Material::SetColor3(const std::string& name, const Color& color) {
    SetVec3(name, { color.r, color.g, color.b });
}

void Material::SetColor4(const std::string& name, const Color& color) {
    SetVec4(name, { color.r, color.g, color.b, color.a });
}

void Material::SetMainColor(const Color& color) { SetColor4("_Color", color); }
void Material::SetMainTexture(Texture* texture) { SetTexture("_MainTex", texture, 0); }

// ─────────────────────────────────────────────
//  Bind
// ─────────────────────────────────────────────

void Material::Bind() const noexcept {
    CORE_ASSERT(m_Shader, "Material::Bind: m_Shader is null.");
    m_Shader->Bind();

    for (const auto& [name, v] : m_Ints)   m_Shader->SetUniform(name, v);
    for (const auto& [name, v] : m_Floats) m_Shader->SetUniform(name, v);
    for (const auto& [name, v] : m_Vec3s)  m_Shader->SetUniform(name, v);
    for (const auto& [name, v] : m_Vec4s)  m_Shader->SetUniform(name, v);
    for (const auto& [name, v] : m_Mat3s)  m_Shader->SetUniform(name, v);
    for (const auto& [name, v] : m_Mat4s)  m_Shader->SetUniform(name, v);

    if (m_Textures.empty()) {
        if (m_Shader->HasUniform("_MainTex")) {
            static Texture* s_White = Texture::CreateSolid(Color::White());
            CORE_ASSERT(s_White, "Material::Bind: Failed to create default white texture.");
            s_White->Bind(0);
            m_Shader->SetUniform("_MainTex", 0);
        }
        return;
    }

    int autoSlot = 0;
    for (const auto& [name, slot] : m_Textures) {
        if (!m_Shader->HasUniform(name)) {
            continue;
        }

        const int targetSlot = (slot.slot >= 0) ? slot.slot : autoSlot++;

        CORE_ASSERT(targetSlot >= 0 && targetSlot < 32, "Material::Bind: Texture slot out of range.");

        if (slot.texture != nullptr) {
            slot.texture->Bind(targetSlot);
            m_Shader->SetUniform(name, targetSlot);
        }
        else if (slot.gpuID != 0) {
            CORE_ASSERT(glGetError() == GL_NO_ERROR, "Material::Bind: OpenGL error detected before glBindTexture.");

            glActiveTexture(GL_TEXTURE0 + targetSlot);
            glBindTexture(slot.target, slot.gpuID);
            m_Shader->SetUniform(name, targetSlot);
        }
        else {
            CORE_ASSERT(false, "Material::Bind: Invalid TextureSlot (null texture and missing GPU ID).");
        }

        if (targetSlot >= autoSlot) {
            autoSlot = targetSlot + 1;
        }
    }
}

// ─────────────────────────────────────────────
//  Clone
// ─────────────────────────────────────────────

std::unique_ptr<Material> Material::Clone() const {
	auto material = std::make_unique<Material>(m_Shader);

    material->m_Ints     = m_Ints;
    material->m_Floats   = m_Floats;
    material->m_Vec3s    = m_Vec3s;
    material->m_Vec4s    = m_Vec4s;
    material->m_Mat3s    = m_Mat3s;
    material->m_Mat4s    = m_Mat4s;
    material->m_Textures = m_Textures;

    return material;
}

void Material::Unbind() const noexcept { m_Shader->Unbind(); }

} // namespace Engine::Rendering::Bindables