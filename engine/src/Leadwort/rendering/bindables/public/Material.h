#pragma once

#include "IBindable.h"
#include "Shader.h"
#include "Texture.h"
#include <Leadwort/core/math/public/Color.h>
#include <Leadwort/core/math/public/Mat4.h>
#include <Leadwort/core/math/public/Math.h>
#include <Leadwort/core/public/Core.h>
#include <Leadwort/rendering/public/RenderPipelineState.h>

#include <glad/glad.h>
#include <string>
#include <unordered_map>

#include <memory>

namespace Leadwort::Rendering::Bindables {
	class CubeMap;

	class Material : public IBindable {
	public:
		struct TextureSlot {
			Shared<Texture> texture { nullptr };
			uint32_t gpuID { 0 };
			uint32_t target { GL_TEXTURE_2D };
			int slot { -1 }; // -1 = auto
		};

		RenderPipelineState pipelineState { RenderPipelineState::Opaque() };

	public:
		explicit Material(const Shared<Shader>& shader, AssetManagement::AssetKey<Material>);
		Material() = delete;

		explicit Material(Shader&& shader) = delete;
	    Material(const Material&) = delete;
	    Material& operator=(const Material&) = delete;

	    // Setters
	    void SetFloat(const std::string& name, float value);
	    void SetInt(const std::string& name, int value);
	    void SetVec3(const std::string& name, const Vec3& value);
	    void SetVec4(const std::string& name, const Vec4& value);
	    void SetMat3(const std::string& name, const Mat3& value);
	    void SetMat4(const std::string& name, const Mat4& value);

	    void SetTexture(const std::string& name, const Shared<Texture>& texture);
	    void SetTexture(const std::string& name, const Shared<Texture>& texture, int slot);
	    void SetTexture(const std::string& name, uint32_t gpuID, int slot, uint32_t target = GL_TEXTURE_2D);

		void SetCubeMap(const std::string& name, const CubeMap* cubeMap);
		void SetCubeMap(const std::string& name, const CubeMap* cubeMap, int slot);

	    void SetColor3(const std::string& name, const Color& color);
	    void SetColor4(const std::string& name, const Color& color);

	    void SetMainColor(const Color& color);
	    void SetMainTexture(const Shared<Texture>& texture);

	    void Bind() const noexcept override;
	    void Unbind() const noexcept override;

	    [[nodiscard]] Shared<Material> Clone() const;
	    [[nodiscard]] Shader const& GetShader() const { return *m_Shader; }

		[[nodiscard]] const std::unordered_map<std::string, TextureSlot>& GetTextures() const noexcept { return m_Textures; }
		[[nodiscard]] const std::unordered_map<std::string, float>& GetFloats() const noexcept { return m_Floats; }
		[[nodiscard]] const std::unordered_map<std::string, Vec3>& GetVec3s() const noexcept { return m_Vec3s; }
		[[nodiscard]] const std::unordered_map<std::string, Vec4>& GetVec4s() const noexcept { return m_Vec4s; }
		[[nodiscard]] const std::unordered_map<std::string, int>& GetInts() const noexcept { return m_Ints; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

	private:
		std::string m_Name{};
		Shared<Shader> m_Shader{};
		mutable uint32_t m_LastShaderVersion { IntInfinity };

	    std::unordered_map<std::string, float> m_Floats{};
	    std::unordered_map<std::string, int>   m_Ints{};
	    std::unordered_map<std::string, Vec3>  m_Vec3s{};
	    std::unordered_map<std::string, Vec4>  m_Vec4s{};
	    std::unordered_map<std::string, Mat3>  m_Mat3s{};
	    std::unordered_map<std::string, Mat4>  m_Mat4s{};
	    std::unordered_map<std::string, TextureSlot> m_Textures{};
	};

} // namespace Engine::Rendering::Bindables