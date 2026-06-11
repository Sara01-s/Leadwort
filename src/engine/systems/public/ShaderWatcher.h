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
        std::string                   mainPath;
        fs::path                      assetRoot;
        std::vector<fs::path>         watchedPaths;
        fs::file_time_type            lastCheckTime;
    };

    static ShaderWatcher& Get() {
        static ShaderWatcher instance;
        return instance;
    }

    ShaderWatcher(const ShaderWatcher&)            = delete;
    ShaderWatcher& operator=(const ShaderWatcher&) = delete;

    void RegisterShader(Rendering::Bindables::Shader* shader, const fs::path& absolutePath, const fs::path& assetRoot) {
    	CORE_LOG("ShaderWatcher: REGISTRANDO shader: ", absolutePath.string());

    	if (!fs::exists(absolutePath)) {
    		CORE_ERROR("ShaderWatcher: ERROR. El archivo NO existe en la ruta registrada: ", absolutePath.string());
    		return;
    	}

        CORE_LOG("ShaderWatcher: Registering shader at: ", absolutePath.string());

        ShaderEntry entry;
        entry.shader    = shader;
        entry.mainPath  = absolutePath.string();
        entry.assetRoot = assetRoot;

        RefreshWatchedPaths(entry);
    	entry.lastCheckTime = fs::file_time_type::min();
        m_ActiveShaders.push_back(std::move(entry));
    }

	void Update() {
    	for (auto& entry : m_ActiveShaders) {
    		// AQUÍ ESTÁ EL CAMBIO: Quitamos la fuerza y usamos la detección real
    		if (HasAnyChanged(entry)) {
    			CORE_LOG("ShaderWatcher: CAMBIO DETECTADO. Recargando: ", entry.mainPath);

    			try {
    				// 1. Recompilamos
    				entry.shader->Compile();

    				// 2. Actualizamos el tiempo para no detectar el mismo cambio infinitamente
    				entry.lastCheckTime = MaxWriteTime(entry.watchedPaths);

    				CORE_LOG("ShaderWatcher: Recompile exitoso.");
    			}
    			catch (const std::exception& e) {
    				CORE_ERROR("ShaderWatcher: Error al recompilar: ", e.what());
    			}
    		}
    	}
    }

    const std::vector<ShaderEntry>& GetActiveShaderEntries() const noexcept { return m_ActiveShaders; }

private:
    ShaderWatcher() = default;

    static void RefreshWatchedPaths(ShaderEntry& entry) {
        entry.watchedPaths.clear();
        entry.watchedPaths.push_back(fs::path(entry.mainPath));

        for (const auto& dep : entry.shader->GetDependencies()) {
            const fs::path p = fs::path(dep).is_absolute()
                ? fs::path(dep).lexically_normal()
                : (entry.assetRoot / dep).lexically_normal();

            if (fs::exists(p)) {
                entry.watchedPaths.push_back(p);
            }
        	else {
                CORE_WARN("ShaderWatcher: Dependency not found, skipping: ", p.string());
            }
        }
    }

    static fs::file_time_type MaxWriteTime(const std::vector<fs::path>& paths) {
        fs::file_time_type latest{};
        for (const auto& p : paths) {
            try {
                if (fs::exists(p))
                    latest = std::max(latest, fs::last_write_time(p));
            }
        	catch (const fs::filesystem_error&) {}
        }

        return latest;
    }

	static bool HasAnyChanged(const ShaderEntry& entry) {
    	for (const auto& path : entry.watchedPaths) {
    		if (!fs::exists(path)) continue;

    		auto lastWrite = fs::last_write_time(path);

    		if (lastWrite > entry.lastCheckTime) {
    			return true;
    		}
    	}

    	return false;
    }

    std::vector<ShaderEntry> m_ActiveShaders;
};

} // namespace Engine::Systems