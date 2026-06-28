#include "../public/Shader.h"

#include "engine/asset-management/public/AssetManager.h"
#include "engine/core/math/public/Mat4.h"
#include "engine/core/math/public/Vec4.h"
#include "engine/systems/public/ShaderWatcher.h"
#include "engine/utils/public/Logger.h"

#include <filesystem>
#include <sstream>

namespace Engine::Rendering::Bindables {

namespace fs = std::filesystem;

// ─────────────────────────────────────────────
//  Lifecycle
// ─────────────────────────────────────────────
Shader::Shader(const std::string& filePath, const std::set<std::string>& defines, AssetManagement::AssetKey<Shader>)
	: m_Path(filePath), m_Defines(defines)
{
	m_Source = AssetManagement::EngineAssets::LoadText(m_Path);
	Compile();

	const fs::path sourceRoot = fs::path(PROJECT_SOURCE_DIR) / "src" / "engine" / "engine-assets";
	const fs::path sourcePath = (sourceRoot / m_Path).lexically_normal();

	Systems::ShaderWatcher::Get().RegisterShader(this, sourcePath, sourceRoot);
}

Shader::~Shader() {
	if (m_GpuID != 0) {
		CORE_LOG("Destroying Shader with path: ", m_Path, " and GPU ID: ", m_GpuID);
		glDeleteProgram(m_GpuID);
	}
}

void Shader::Bind() const noexcept {
	CORE_ASSERT(m_GpuID != 0, "Shader: Cannot bind, GPU ID is 0.");
	glUseProgram(m_GpuID);
}

void Shader::Unbind() const noexcept {
	glUseProgram(0);
}

// ─────────────────────────────────────────────
//  Defines
// ─────────────────────────────────────────────

void Shader::EnableDefine(const std::string& define) {
	if (m_Defines.insert(define).second) {
		Compile();
	}
}

void Shader::DisableDefine(const std::string& define) {
	if (m_Defines.erase(define)) {
		Compile();
	}
}

void Shader::SetDefines(const std::set<std::string>& defines) {
	if (m_Defines == defines) {
		return;
	}

	m_Defines = defines;
	Compile();
}

// ─────────────────────────────────────────────
//  UBO
// ─────────────────────────────────────────────

void Shader::SetUniformBlock(const std::string& blockName, int bindingPoint) {
	if (m_GpuID == 0) {
		return;
	}

	if (const uint32_t blockIndex = glGetUniformBlockIndex(m_GpuID, blockName.c_str());
		blockIndex != GL_INVALID_INDEX) {
		glUniformBlockBinding(m_GpuID, blockIndex, bindingPoint);
	}
	else {
		CORE_WARN("Shader: Block '", blockName, "' not found in shader '", m_Path, "'");
	}
}

// ─────────────────────────────────────────────
//  Compilation pipeline
// ─────────────────────────────────────────────
void Shader::Compile() {
    std::lock_guard lock(m_CompileMutex);
    std::vector<std::string> backupDependencies = m_Dependencies;

    try {
        m_Dependencies.clear();

        if (!m_Path.empty()) {
			m_Source = LoadSource(m_Path);
		}

		const auto shaderSources = ParseShader(m_Source);
        const std::string baseDir = m_Path.empty() ? "" : m_Path.substr(0, m_Path.find_last_of('/'));

        auto resolveAndInject = [&](const std::string& key) -> std::string {
            const auto it = shaderSources.find(key);
            if (it == shaderSources.end())
                throw std::runtime_error("Missing " + key + " shader in: " + m_Path);

            std::set<std::string> visited;
            return InjectDefines(ResolveIncludes(it->second, baseDir, visited));
        };

        const std::string vertexSrc   = resolveAndInject("vertex");
        const std::string fragmentSrc = resolveAndInject("fragment");

        const uint32_t vs = CompileShader(GL_VERTEX_SHADER,   vertexSrc);
        const uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

        const uint32_t newProgram = LinkProgram(vs, fs);
        CORE_ASSERT(newProgram != 0, "Shader: Failed to create a program during compilation.");

    	if (const uint32_t cameraBlock = glGetUniformBlockIndex(newProgram, "CameraData"); cameraBlock != GL_INVALID_INDEX) {
    		glUniformBlockBinding(newProgram, cameraBlock, 0);
		}

    	if (const uint32_t lightBlock = glGetUniformBlockIndex(newProgram, "LightingData"); lightBlock != GL_INVALID_INDEX) {
    		glUniformBlockBinding(newProgram, lightBlock, 1);
		}

        if (m_GpuID != 0) {
			glDeleteProgram(m_GpuID);
		}

		m_GpuID = newProgram;
        m_Version++;
        m_UniformLocationCache.clear();
    	m_Samplers.clear();

        CORE_LOG("Shader: Compiled '", m_Path, "' new version=", m_Version, " gpuID=", m_GpuID);

        ExtractSamplers();
    }
    catch (const std::exception& e) {
        CORE_ERROR("Shader: Error while compiling '", m_Path, "':\n", e.what());
        m_Dependencies = std::move(backupDependencies);
    }
}

// ─────────────────────────────────────────────
//  Path helpers
// ─────────────────────────────────────────────

static std::string NormalizePath(const std::string& path) {
	std::string p = path;
	std::ranges::replace(p, '\\', '/');

	if (!p.empty() && p[0] == '/') {
		p.erase(0, 1);
	}

	// Collapse redundant segments (e.g. "shaders/../common/foo" → "common/foo")
	return std::filesystem::path(p).lexically_normal().generic_string();
}

// ─────────────────────────────────────────────
//  Source loading
// ─────────────────────────────────────────────

std::string Shader::LoadSource(const std::string& path) {
	return AssetManagement::EngineAssets::LoadText(path);
}

// ─────────────────────────────────────────────
//  #type parsing  (splits one file into vertex / fragment)
// ─────────────────────────────────────────────

std::unordered_map<std::string, std::string> Shader::ParseShader(const std::string& source) {
	std::unordered_map<std::string, std::string> shaders;
	std::istringstream stream(source);
	std::string line;
	std::string currentType;
	std::ostringstream builder;

	while (std::getline(stream, line)) {
		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (line.rfind("#type", 0) == 0) {
			if (!currentType.empty()) {
				shaders[currentType] = builder.str();
				builder.str("");
				builder.clear();
			}

			currentType = line.substr(5);
			currentType.erase(0, currentType.find_first_not_of(" \t"));
			currentType.erase(currentType.find_last_not_of(" \t") + 1);
		}
		else {
			builder << line << "\n";
		}
	}

	if (!currentType.empty()) {
		shaders[currentType] = builder.str();
	}

	return shaders;
}

// ─────────────────────────────────────────────
//  #include resolution
// ─────────────────────────────────────────────

std::string Shader::ResolveIncludes(
	const std::string& source,
	const std::string& currentDir,
	std::set<std::string>& visited
) {
	std::ostringstream result;
	std::istringstream stream(source);
	std::string line;

	while (std::getline(stream, line)) {
		const std::string trimmed = [&] {
			const size_t s = line.find_first_not_of(" \t");
			return s == std::string::npos ? line : line.substr(s);
		}();

		if (trimmed.rfind("#include", 0) == 0) {
			const size_t q1 = trimmed.find('"');
			const size_t q2 = trimmed.find('"', q1 + 1);
			if (q1 == std::string::npos || q2 == std::string::npos) {
				throw std::runtime_error("Shader: Malformed #include: " + line);
			}

			const std::string includePath = trimmed.substr(q1 + 1, q2 - q1 - 1);
			const std::string fullPath = currentDir.empty() ? includePath : currentDir + "/" + includePath;

			if (const std::string normalized = NormalizePath(fullPath); visited.insert(normalized).second) {
				const std::string includeDir = normalized.substr(0, normalized.find_last_of('/'));

				m_Dependencies.emplace_back(normalized);

				result << ResolveIncludes(LoadSource(normalized), includeDir, visited) << '\n';
			}
		}
		else {
			result << line << '\n';
		}
	}

	return result.str();
}

// ─────────────────────────────────────────────
//  #define injection (after #version, before everything else)
// ─────────────────────────────────────────────

std::string Shader::InjectDefines(const std::string& source) const {
	if (m_Defines.empty()) {
		return source;
	}

	std::istringstream stream(source);
	std::ostringstream builder;
	std::string line;
	bool injected = false;

	if (std::getline(stream, line)) {
		builder << line << '\n';

		if (line.rfind("#version", 0) == 0) {
			for (const auto& define: m_Defines) {
				builder << "#define " << define << '\n';
			}
			injected = true;
		}
	}

	if (!injected) {
		std::ostringstream prefix;
		for (const auto& define: m_Defines) {
			prefix << "#define " << define << '\n';
		}

		std::string rest;

		while (std::getline(stream, rest)) {
			builder << rest << '\n';
		}

		return prefix.str() + builder.str();
	}

	while (std::getline(stream, line)) {
		builder << line << '\n';
	}

	return builder.str();
}

// ─────────────────────────────────────────────
//  GL helpers
// ─────────────────────────────────────────────

uint32_t Shader::CompileShader(const uint32_t type, const std::string& source) {
	const uint32_t id = glCreateShader(type);
	CORE_ASSERT(id != 0, "Shader: Failed to create shader object.");

	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (!success) {
		char infoLog[1024];
		glGetShaderInfoLog(id, 1024, nullptr, infoLog);
		glDeleteShader(id);
		throw std::runtime_error("Shader compilation error: " + std::string(infoLog));
	}

	return id;
}

uint32_t Shader::LinkProgram(const uint32_t vs, const uint32_t fs) {
	const uint32_t program = glCreateProgram();
	CORE_ASSERT(program != 0, "Shader: Failed to create linking program.");

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success) {
		char infoLog[1024];
		glGetProgramInfoLog(program, 1024, nullptr, infoLog);
		glDeleteProgram(program);
		throw std::runtime_error("Shader linking error: " + std::string(infoLog));
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

// ─────────────────────────────────────────────
//  Uniform location cache
// ─────────────────────────────────────────────

int Shader::GetUniformLocation(const std::string& name) const {
	CORE_ASSERT(m_GpuID != 0, "Shader: Cannot get uniform location, GPU ID is 0.");

	if (const auto it = m_UniformLocationCache.find(name); it != m_UniformLocationCache.end()) {
		return it->second;
	}

	const int location = glGetUniformLocation(m_GpuID, name.c_str());
	m_UniformLocationCache[name] = location;
	return location;
}

// ─────────────────────────────────────────────
//  SetUniform overloads
// ─────────────────────────────────────────────

void Shader::SetUniform(const std::string& name, const int value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniform1i(loc, value);
	}
}

void Shader::SetUniform(const std::string& name, const float value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniform1f(loc, value);
	}
}

void Shader::SetUniform(const std::string& name, const Vec2& value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniform2f(loc, value.x, value.y);
	}
}

void Shader::SetUniform(const std::string& name, const Vec3& value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniform3f(loc, value.x, value.y, value.z);
	}
}

void Shader::SetUniform(const std::string& name, const Vec4& value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniform4f(loc, value.x, value.y, value.z, value.w);
	}
}

void Shader::SetUniform(const std::string& name, const Mat3& value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniformMatrix3fv(loc, 1, GL_FALSE, value.ToPtr());
	}
}

void Shader::SetUniform(const std::string& name, const Mat4& value) const {
	if (const int loc = GetUniformLocation(name); loc != -1) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, value.ToPtr());
	}
}

void Shader::ExtractSamplers() {
	int uniformCount = 0;

	glGetProgramiv(m_GpuID, GL_ACTIVE_UNIFORMS, &uniformCount);

	for (int i = 0; i < uniformCount; i++) {
		char name[256];
		int size;
		GLenum type;
		glGetActiveUniform(m_GpuID, i, sizeof(name), nullptr, &size, &type, name);

		if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE || type == GL_SAMPLER_2D_SHADOW || type == GL_SAMPLER_3D) {
			const int location = glGetUniformLocation(m_GpuID, name);
			m_Samplers[std::string(name)] = SamplerInfo{type, location};
		}
	}
}

bool Shader::HasUniform(const std::string& name) const {
	return m_GpuID != 0 && GetUniformLocation(name) != -1;
}

std::vector<std::string> Shader::GetDependencies() const noexcept {
	return std::vector(m_Dependencies.begin(), m_Dependencies.end());
}

std::unordered_map<std::string, Shader::SamplerInfo> Shader::GetSamplers() const noexcept {
	return m_Samplers;
}

} // namespace Engine::Rendering::Bindables