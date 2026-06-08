#pragma once

#include "engine/rendering/bindables/public/Shader.h"
#include "engine/utils/public/Logger.h"

#include <filesystem>
#include <vector>
#include <string>

namespace Engine::Systems {

namespace fs = std::filesystem;

class ShaderWatcher {
public:
	struct ShaderEntry {
		Rendering::Bindables::Shader* shader;
		std::string mainPath;
		std::vector<std::string> dependencies;
		fs::file_time_type lastCheckTime;
	};

    static ShaderWatcher& Get() {
       static ShaderWatcher instance;
       return instance;
    }

    ~ShaderWatcher() = default;

    ShaderWatcher(const ShaderWatcher&) = delete;
    ShaderWatcher& operator=(const ShaderWatcher&) = delete;

    void Update() {
       for (auto& [shader, mainPath, dependencies, lastCheckTime] : m_ActiveShaders) {
          bool changed = false;

          for (const auto& dependency : dependencies) {
             if (const fs::path path(dependency); fs::exists(path)) {
                if (const auto lastWrite = fs::last_write_time(path); lastWrite > lastCheckTime) {
                   CORE_LOG("Change detected in: ", path.filename().string());
                   lastCheckTime = lastWrite;
                   changed = true;
                }
             }
          }

          if (changed) {
             CORE_LOG("Recompiling shader: ", mainPath);
             try {
                shader->Compile();
             	lastCheckTime = fs::file_time_type::clock::now(); // Avoids recompiling twice.
                CORE_LOG("Shader recompiled successfully.");
             }
             catch (const std::exception& e) {
                CORE_ERROR("Failed to recompile shader: ", e.what());
             }
          }
       }
    }

	void RegisterShader(Rendering::Bindables::Shader* shader, const std::string& path) {
		const fs::path exeDir = fs::current_path();
		const fs::path projectRoot = exeDir.parent_path();
		const fs::path fullPath = projectRoot / "src" / "engine" / "engine-assets" / path;

    	if (!fs::exists(fullPath)) {
    		CORE_ERROR("Shader file not found at: ", fullPath.string());
    		return;
    	}

    	CORE_LOG("Registering shader at: ", fullPath.string());

    	ShaderEntry entry;
    	entry.shader = shader;
    	entry.mainPath = fullPath.string();
    	entry.dependencies = shader->GetDependencies();

    	std::vector<std::string> absoluteDeps;

    	for (auto& dep : entry.dependencies) {
    		fs::path depPath = projectRoot / "engine-assets" / dep;
    		absoluteDeps.push_back(depPath.string());
    	}

    	entry.dependencies = absoluteDeps;
    	entry.dependencies.push_back(fullPath.string());

    	entry.lastCheckTime = fs::last_write_time(fullPath);
    	m_ActiveShaders.push_back(entry);
    }

	std::vector<ShaderEntry> const& GetActiveShaderEntries() const noexcept { return m_ActiveShaders; }

private:
    ShaderWatcher() = default;

    std::vector<ShaderEntry> m_ActiveShaders;
};

} // namespace Engine::Systems