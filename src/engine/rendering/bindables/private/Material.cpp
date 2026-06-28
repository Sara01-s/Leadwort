#include "../public/Material.h"

#include "engine/asset-management/private/AssetKey.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/asset-management/public/DefaultAssets.h"
#include "engine/core/math/public/Vec4.h"
#include "engine/rendering/bindables/public/CubeMap.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Logger.h"

#include <memory>

namespace Engine::Rendering::Bindables {

using Color = Utils::Color;

Material::Material(const Shared<Shader>& shader, AssetManagement::AssetKey<Material>) : m_Shader(shader) {
	CORE_ASSERT(m_Shader, "Material::Material: Provided shader is null!");
	SetColor4("_Color", Color::White());
}

// ─────────────────────────────────────────────
//  Setters
// ─────────────────────────────────────────────

void Material::SetInt(const std::string& name, const int value)     { m_Ints[name]    = value; }
void Material::SetFloat(const std::string& name, const float value) { m_Floats[name]  = value; }
void Material::SetVec3(const std::string& name, const Vec3& value)  { m_Vec3s[name]   = value; }
void Material::SetVec4(const std::string& name, const Vec4& value)  { m_Vec4s[name]   = value; }
void Material::SetMat3(const std::string& name, const Mat3& value)  { m_Mat3s[name]   = value; }
void Material::SetMat4(const std::string& name, const Mat4& value)  { m_Mat4s[name]   = value; }

void Material::SetTexture(const std::string& name, const Shared<Texture>& texture) {
    m_Textures[name] = TextureSlot { texture, 0, GL_TEXTURE_2D, -1 };
}

void Material::SetTexture(const std::string& name, const Shared<Texture>& texture, const int slot) {
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
    SetVec3(name, Vec3(color.r, color.g, color.b));
}

void Material::SetColor4(const std::string& name, const Color& color) {
    SetVec4(name, Vec4(color.r, color.g, color.b, color.a));
}

void Material::SetMainColor(const Color& color) { SetColor4("_Color", color); }
void Material::SetMainTexture(const Shared<Texture>& texture) { SetTexture("_MainTex", texture, 0); }

// ─────────────────────────────────────────────
//  Bind
// ─────────────────────────────────────────────


void Material::Bind() const noexcept {
	CORE_ASSERT(m_Shader, "Material::Bind: m_Shader is null.");

	m_Shader->Bind();

	if (m_Shader->GetVersion() != m_LastShaderVersion) {
		for (const auto& [name, value] : m_Ints)   m_Shader->SetUniform(name, value);
		for (const auto& [name, value] : m_Floats) m_Shader->SetUniform(name, value);
		for (const auto& [name, value] : m_Vec3s)  m_Shader->SetUniform(name, value);
		for (const auto& [name, value] : m_Vec4s)  m_Shader->SetUniform(name, value);
		for (const auto& [name, value] : m_Mat3s)  m_Shader->SetUniform(name, value);
		for (const auto& [name, value] : m_Mat4s)  m_Shader->SetUniform(name, value);

		m_LastShaderVersion = m_Shader->GetVersion();
	}

	static const Shared<Texture> s_Fallback = AssetManagement::DefaultAssets::GetTexture();

	int slot = 0;
	for (const auto& [name, samplerInfo] : m_Shader->GetSamplers()) {
		glActiveTexture(GL_TEXTURE0 + slot);

		const auto it = m_Textures.find(name);

		if (it != m_Textures.end()) {
			const auto& textureSlot = it->second;

			if (textureSlot.texture) {
				glBindTexture(textureSlot.target, textureSlot.texture->GetGpuID());
			}
			else if (textureSlot.gpuID != 0) {
				glBindTexture(textureSlot.target, textureSlot.gpuID);
			}
			else {
				CORE_ASSERT(false, "Material::Bind: Invalid TextureSlot for: " + name);
			}
		}
		else {
			// Fallback texture.
			const GLenum target = samplerInfo.type == GL_SAMPLER_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
			glBindTexture(target, s_Fallback->GetGpuID());
		}

		m_Shader->SetUniform(name, slot);
		slot++;
	}

	glActiveTexture(GL_TEXTURE0);
}

// ─────────────────────────────────────────────
//  Clone
// ─────────────────────────────────────────────

Shared<Material> Material::Clone() const {
	auto material = AssetManagement::EngineAssets::CreateMaterial(m_Shader);

    material->m_Ints     = m_Ints;
    material->m_Floats   = m_Floats;
    material->m_Vec3s    = m_Vec3s;
    material->m_Vec4s    = m_Vec4s;
    material->m_Mat3s    = m_Mat3s;
    material->m_Mat4s    = m_Mat4s;
    material->m_Textures = m_Textures;

    return material;
}

void Material::Unbind() const noexcept {
	for (const auto& slot: m_Textures | std::views::values) {
		const int targetSlot = slot.slot >= 0 ? slot.slot : 0;

		glActiveTexture(GL_TEXTURE0 + targetSlot);
		glBindTexture(slot.target, 0);
	}

	m_Shader->Unbind();
}

} // namespace Engine::Rendering::Bindables