#pragma once

#include "Bindable.h"
#include "engine/asset-management/private/AssetKey.h"
#include "engine/core/math/public/Mat3.h"

#include <glad/glad.h>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine::Rendering::Bindables {

class Shader : public Bindable  {
public:
	struct SamplerInfo {
		GLenum type; // GL_SAMPLER_2D, GL_SAMPLER_CUBE, etc.
		int location;
	};

	explicit Shader(const std::string& filePath, AssetManagement::AssetKey<Shader>);
	explicit Shader() = delete;
	~Shader() override;

	Shader& operator=(const Shader&) = delete;

	void Bind() const noexcept override;
	void Unbind() const noexcept override;

	void EnableDefine(const std::string& define);
	void DisableDefine(const std::string& define);

	void SetUniformBlock(const std::string& blockName, int bindingPoint);

	void SetUniform(const std::string& name, int value) const;
	void SetUniform(const std::string& name, float value) const;
	void SetUniform(const std::string& name, const Vec2& value) const;
	void SetUniform(const std::string& name, const Vec3& value) const;
	void SetUniform(const std::string& name, const Vec4& value) const;
	void SetUniform(const std::string& name, const Mat3& value) const;
	void SetUniform(const std::string& name, const Mat4& value) const;
	void ExtractSamplers();

	[[nodiscard]] bool HasUniform(const std::string& name) const;
	[[nodiscard]] std::vector<std::string> GetDependencies() const noexcept;
	[[nodiscard]] std::unordered_map<std::string, SamplerInfo> GetSamplers() const noexcept;
	[[nodiscard]] uint32_t GetVersion() const { return m_Version; }

	void Compile();

private:
	std::unordered_map<std::string, SamplerInfo> m_Samplers;
	std::uint32_t m_Version = 0;

	std::string m_Source;
	std::string m_Path;
	std::set<std::string> m_Defines;
	std::vector<std::string> m_Dependencies;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;

	int GetUniformLocation(const std::string& name) const;
	static std::string LoadSource(const std::string& path);
	static std::unordered_map<std::string, std::string> ParseShader(const std::string& source);
	std::string ResolveIncludes(const std::string& source, const std::string& currentDir, std::set<std::string>& visited);
	std::string InjectDefines(const std::string& source) const;

	static uint32_t CompileShader(uint32_t type, const std::string& source);
	static uint32_t LinkProgram(uint32_t vs, uint32_t fs);
};

} // namespace Engine::Rendering