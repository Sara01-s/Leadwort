#pragma once

#include "IBindable.h"
#include <Leadwort/asset-management/private/AssetKey.h>
#include <Leadwort/core/math/public/Mat3.h>

#include <glad/glad.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace Leadwort::Rendering::Bindables {

	class Shader : public IBindable  {
	public:
		struct SamplerInfo {
			GLenum type; // GL_SAMPLER_2D, GL_SAMPLER_CUBE, etc.
			int location;
		};

		Shader(const std::string& filePath, const std::set<std::string>& defines, AssetManagement::AssetKey<Shader>);
		explicit Shader() = delete;
		~Shader() override;

		Shader& operator=(const Shader&) = delete;

		void Bind() const noexcept override;
		void Unbind() const noexcept override;

		void EnableDefine(const std::string& define);
		void DisableDefine(const std::string& define);
		void SetDefines(const std::set<std::string>& defines);

		void SetUniformBlock(const std::string& blockName, int bindingPoint);

		void SetUniform(std::string_view name, int value) const;
		void SetUniform(std::string_view name, float value) const;
		void SetUniform(std::string_view name, Vec2 value) const;
		void SetUniform(std::string_view name, const Vec3& value) const;
		void SetUniform(std::string_view name, const Vec4& value) const;
		void SetUniform(std::string_view name, const Mat3& value) const;
		void SetUniform(std::string_view name, const Mat4& value) const;
		void ExtractSamplers();

		[[nodiscard]] bool HasUniform(std::string_view name) const;
		[[nodiscard]] std::vector<std::string> GetDependencies() const noexcept;
		[[nodiscard]] std::unordered_map<std::string, SamplerInfo> GetSamplers() const noexcept;
		[[nodiscard]] std::string GetPath() const noexcept { return m_Path; }
		[[nodiscard]] uint32_t GetVersion() const { return m_Version; }

		void Compile();

	private:
		int GetUniformLocation(std::string_view name) const;
		static std::string LoadSource(const std::string& path);
		static std::unordered_map<std::string, std::string> ParseShader(std::string_view source);
		std::string ResolveIncludes(std::string_view source, const std::string& currentDir, std::set<std::string>& visited);
		std::string InjectDefines(std::string_view source) const;

		static uint32_t CompileShader(uint32_t type, const std::string& source);
		static uint32_t LinkProgram(uint32_t vs, uint32_t fs);

	private:
		std::unordered_map<std::string, SamplerInfo> m_Samplers{};
		std::uint32_t m_Version { 0 };

		std::string m_Source{};
		std::string m_Path{};
		std::set<std::string> m_Defines{};
		std::vector<std::string> m_Dependencies{};
		mutable std::unordered_map<std::string, int> m_UniformLocationCache{};
		mutable std::mutex m_CompileMutex{};
	};

} // namespace Engine::Rendering