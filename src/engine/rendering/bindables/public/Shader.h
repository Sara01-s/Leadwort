#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Bindable.h"

namespace Engine::Rendering::Bindables {

class Shader : public Bindable {
public:
	Shader(std::string source, std::string path);
	~Shader() override;

	Shader& operator=(const Shader&) = delete;

	void Bind() const noexcept override;
	void Unbind() const noexcept override;

	void EnableDefine(const std::string& define);
	void DisableDefine(const std::string& define);

	void SetUniformBlock(const std::string& blockName, int bindingPoint);

	void SetUniform(const std::string& name, int value) const;
	void SetUniform(const std::string& name, float value) const;
	void SetUniform(const std::string& name, const glm::vec2& value) const;
	void SetUniform(const std::string& name, const glm::vec3& value) const;
	void SetUniform(const std::string& name, const glm::vec4& value) const;
	void SetUniform(const std::string& name, const glm::mat3& value) const;
	void SetUniform(const std::string& name, const glm::mat4& value) const;

	[[nodiscard]] bool HasUniform(const std::string& name) const;
	[[nodiscard]] std::vector<std::string> GetDependencies() const noexcept;

	void Compile();

private:
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