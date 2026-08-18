#include "../public/Material.h"

#include "Leadwort/rendering/public/rendergraph/GlobalTextureSlots.h"

#include <Leadwort/asset-management/private/AssetKey.h>
#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/asset-management/public/DefaultAssets.h>
#include <Leadwort/core/math/public/Vec4.h>
#include <Leadwort/rendering/bindables/public/CubeMap.h>
#include <Leadwort/utils/public/Logger.h>

#include <memory>

namespace Leadwort::Rendering::Bindables {

	Material::Material(const Shared<Shader>& shader, AssetManagement::AssetKey<Material>) : m_Shader(shader) {
		LW_ASSERT(m_Shader, "Material::Material: Provided shader is null!");
		SetColor4("_Color", Color::White());
	}

	// ─────────────────────────────────────────────
	//  Setters
	// ─────────────────────────────────────────────

	void Material::SetInt(const std::string& name, const int value)     { m_Ints[name]    = value; m_UniformsDirty = true; }
	void Material::SetFloat(const std::string& name, const float value) {
		m_Floats[name] = value;
		m_UniformsDirty = true;
	}
	void Material::SetVec3(const std::string& name, const Vec3& value)  { m_Vec3s[name]   = value; m_UniformsDirty = true; }
	void Material::SetVec4(const std::string& name, const Vec4& value)  { m_Vec4s[name]   = value; m_UniformsDirty = true; }
	void Material::SetMat3(const std::string& name, const Mat3& value)  { m_Mat3s[name]   = value; m_UniformsDirty = true; }
	void Material::SetMat4(const std::string& name, const Mat4& value)  { m_Mat4s[name]   = value; m_UniformsDirty = true; }
	void Material::SetTextureST(const std::string& name, const Vec4& st) {
		const auto it = m_Textures.find(name);
		if (it != m_Textures.end()) {
			it->second.ST = st;
		}
		else {
			m_Textures[name] = TextureSlot { nullptr, 0, GL_TEXTURE_2D, -1, st };
		}

		m_UniformsDirty = true;
	}

	void Material::SetTexture(const std::string& name, const Shared<Texture>& texture) {
	    m_Textures[name] = TextureSlot { texture, 0, GL_TEXTURE_2D, -1 };
		m_UniformsDirty = true;
	}

	void Material::SetTexture(const std::string& name, const Shared<Texture>& texture, const int slot) {
	    m_Textures[name] = TextureSlot { texture, 0, GL_TEXTURE_2D, slot };
		m_UniformsDirty = true;
	}

	void Material::SetTexture(const std::string& name, const uint32_t gpuID, const int slot, const uint32_t target) {
	    m_Textures[name] = TextureSlot { nullptr, gpuID, target, slot };
		m_UniformsDirty = true;
	}

	void Material::SetCubeMap(const std::string& name, const CubeMap* cubeMap) {
		m_Textures[name] = TextureSlot { nullptr, cubeMap->GetGpuID(), GL_TEXTURE_CUBE_MAP, -1 };
		m_UniformsDirty = true;
	}

	void Material::SetCubeMap(const std::string& name, const CubeMap* cubeMap, const int slot) {
		m_Textures[name] = TextureSlot { nullptr, cubeMap->GetGpuID(), GL_TEXTURE_CUBE_MAP, slot };
		m_UniformsDirty = true;
	}

	void Material::SetColor3(const std::string& name, const Color& color) {
	    SetVec3(name, Vec3(color.r, color.g, color.b));
		m_UniformsDirty = true;
	}

	void Material::SetColor4(const std::string& name, const Color& color) {
	    SetVec4(name, Vec4(color.r, color.g, color.b, color.a));
		m_UniformsDirty = true;
	}

	void Material::SetMainColor(const Color& color) { SetColor4("_Color", color); }
	void Material::SetMainTexture(const Shared<Texture>& texture) { SetTexture("_MainTex", texture, 0); }

	// ─────────────────────────────────────────────
	//  Bind
	// ─────────────────────────────────────────────


	void Material::Bind() const noexcept {
		LW_ASSERT(m_Shader, "Material::Bind: m_Shader is null.");

		m_Shader->Bind();

		const bool shaderChanged = m_Shader->GetVersion() != m_LastShaderVersion;

		if (shaderChanged || m_UniformsDirty) {
			for (const auto& [name, value] : m_Ints)   m_Shader->SetUniform(name, value);
			for (const auto& [name, value] : m_Floats) m_Shader->SetUniform(name, value);
			for (const auto& [name, value] : m_Vec3s)  m_Shader->SetUniform(name, value);
			for (const auto& [name, value] : m_Vec4s)  m_Shader->SetUniform(name, value);
			for (const auto& [name, value] : m_Mat3s)  m_Shader->SetUniform(name, value);
			for (const auto& [name, value] : m_Mat4s)  m_Shader->SetUniform(name, value);

			m_LastShaderVersion = m_Shader->GetVersion();
			m_UniformsDirty = false;
		}

		static const Shared<Texture> s_Fallback = AssetManagement::DefaultAssets::GetTexture();

		int slot{};
		for (const auto& [name, samplerInfo] : m_Shader->GetSamplers()) {
			const int reservedSlot = GlobalTextureSlots::GetReservedSlotForGlobalSampler(name);
			if (reservedSlot >= 0) {
				continue;
			}

			glActiveTexture(GL_TEXTURE0 + slot);

			const auto it = m_Textures.find(name);
			Vec4 st { 1.0f, 1.0f, 0.0f, 0.0f };

			if (it != m_Textures.end()) {
				const auto& textureSlot = it->second;
				st = textureSlot.ST;

				if (textureSlot.Texture) {
					glBindTexture(textureSlot.Target, textureSlot.Texture->GetGpuID());
				}
				else if (textureSlot.GpuID != 0) {
					glBindTexture(textureSlot.Target, textureSlot.GpuID);
				}
				else {
					const GLenum target = samplerInfo.Type == GL_SAMPLER_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
					glBindTexture(target, s_Fallback->GetGpuID());
				}
			}
			else {
				// Fallback texture.
				const GLenum target = samplerInfo.Type == GL_SAMPLER_CUBE ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
				glBindTexture(target, s_Fallback->GetGpuID());
			}

			m_Shader->SetUniform(name, slot);
			m_Shader->SetUniform(name + "_ST", st);
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
			const int targetSlot { slot.Slot >= 0 ? slot.Slot : 0 };

			glActiveTexture(GL_TEXTURE0 + targetSlot);
			glBindTexture(slot.Target, 0);
		}

		m_Shader->Unbind();
	}

}