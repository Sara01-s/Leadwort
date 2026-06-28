#pragma once

#include "engine/rendering/bindables/public/Shader.h"
#include "engine/systems/public/FileWatcherSystem.h"
#include "engine/utils/public/Logger.h"

#include <filesystem>
#include <list>
#include <mutex>
#include <unordered_set>

namespace Engine::Systems {

namespace fs = std::filesystem;

class ShaderWatcher {
public:
    struct ShaderEntry {
        Rendering::Bindables::Shader* shader { nullptr };
        std::string mainPath{};
        fs::path assetRoot{};
        std::vector<fs::path> watchedPaths{};
        int subscriptionId { -1 };
    };

    static ShaderWatcher& Get() {
        static ShaderWatcher instance;
        return instance;
    }

    ShaderWatcher(const ShaderWatcher&) = delete;
    ShaderWatcher& operator=(const ShaderWatcher&) = delete;

    void RegisterShader(Rendering::Bindables::Shader* shader, const fs::path& absolutePath, const fs::path& assetRoot) {
        CORE_LOG("ShaderWatcher [Register]: absolutePath='", absolutePath.string(), "'");
        CORE_LOG("ShaderWatcher [Register]: assetRoot='", assetRoot.string(), "'");

        if (!fs::exists(absolutePath)) {
            CORE_ERROR("ShaderWatcher [Register]: FAILED. File does not exist at: ", absolutePath.string());
            return;
        }

        ShaderEntry entry;
        entry.shader = shader;
        entry.mainPath = absolutePath.string();
        entry.assetRoot = assetRoot;
        RefreshWatchedPaths(entry);

        CORE_LOG("ShaderWatcher [Register]: watched paths after RefreshWatchedPaths (",
                 entry.watchedPaths.size(), "):");
        for (const auto& wp : entry.watchedPaths) {
            CORE_LOG("  -> '", wp.string(), "'");
        }

        m_ActiveShaders.push_back(std::move(entry));
        ShaderEntry& stored = m_ActiveShaders.back();

        const int subId = FileWatcherSystem::Get().Watch(assetRoot, [this, &stored](const FileChangeEvent& ev) {
            OnFileChanged(stored, ev);
        });

        stored.subscriptionId = subId;
        CORE_LOG("ShaderWatcher [Register]: done. subId=", subId,
                 " total registered shaders=", m_ActiveShaders.size());
    }

    void UnregisterShader(Rendering::Bindables::Shader* shader) {
        const auto it = std::ranges::find_if(m_ActiveShaders, [shader](const ShaderEntry& entry) {
            return entry.shader == shader;
        });

        if (it == m_ActiveShaders.end()) {
            CORE_WARN("ShaderWatcher [Unregister]: shader not found, ignoring.");
            return;
        }

        CORE_LOG("ShaderWatcher [Unregister]: removing '", it->mainPath, "'");

        if (it->subscriptionId >= 0) {
            FileWatcherSystem::Get().Unwatch(it->subscriptionId);
        }

        m_ActiveShaders.erase(it);
    }

    void ProcessPendingRecompiles() {
        std::unordered_set<Rendering::Bindables::Shader*> pending;

        {
            std::lock_guard lock(m_PendingMutex);
            std::swap(pending, m_PendingRecompiles);
        }

        if (pending.empty()) {
            return;
        }

        CORE_LOG("ShaderWatcher [Main]: processing ", pending.size(), " pending recompile(s).");

        for (auto* shader : pending) {
            auto it = std::ranges::find_if(m_ActiveShaders, [shader](const ShaderEntry& e) {
                return e.shader == shader;
            });

            if (it == m_ActiveShaders.end()) {
                CORE_WARN("ShaderWatcher [Main]: shader ptr=",
                          reinterpret_cast<uintptr_t>(shader),
                          " not found in active list, skipping.");
                continue;
            }

            CORE_LOG("ShaderWatcher [Main]: recompiling '", it->mainPath, "'");
            try {
                shader->Compile();
                RefreshWatchedPaths(*it);
                CORE_LOG("ShaderWatcher [Main]: recompile OK. ptr=",
                         reinterpret_cast<uintptr_t>(shader),
                         " version=", shader->GetVersion());
                CORE_LOG("ShaderWatcher [Main]: updated watched paths (",
                         it->watchedPaths.size(), "):");
                for (const auto& wp : it->watchedPaths) {
                    CORE_LOG("  -> '", wp.string(), "'");
                }
            }
            catch (const std::exception& e) {
                CORE_ERROR("ShaderWatcher [Main]: recompile FAILED for '",
                           it->mainPath, "': ", e.what());
            }
        }
    }

private:
    ShaderWatcher() = default;

    static void RefreshWatchedPaths(ShaderEntry& entry) {
        entry.watchedPaths.clear();
        entry.watchedPaths.push_back(fs::path(entry.mainPath).lexically_normal());

        for (const auto& dep : entry.shader->GetDependencies()) {
            const fs::path p = fs::path(dep).is_absolute()
                ? fs::path(dep).lexically_normal()
                : (entry.assetRoot / dep).lexically_normal();

            if (fs::exists(p)) {
                entry.watchedPaths.push_back(p);
            }
            else {
                CORE_WARN("ShaderWatcher [RefreshWatchedPaths]: dependency does not exist: '", p.string(), "'");
            }
        }
    }

    static bool IsWatchedPath(const ShaderEntry& entry, const fs::path& changedPath) {
        const fs::path normalized = changedPath.lexically_normal();

        for (const auto& watched : entry.watchedPaths) {
            if (watched == normalized) {
                return true;
            }
        }

        return false;
    }

    void OnFileChanged(ShaderEntry& entry, const FileChangeEvent& ev) {
        CORE_LOG("ShaderWatcher [OnFileChanged]: entry='",
                 fs::path(entry.mainPath).filename().string(),
                 "' ev.path='", ev.path.string(),
                 "' type=", static_cast<int>(ev.type));

        if (ev.type != FileChangeType::Modified && ev.type != FileChangeType::Added) {
            CORE_LOG("ShaderWatcher [OnFileChanged]: skipped (type=",
                     static_cast<int>(ev.type), " is not Modified/Added).");
            return;
        }

        CORE_LOG("ShaderWatcher [OnFileChanged]: watched paths for this entry (",
                 entry.watchedPaths.size(), "):");
        for (const auto& wp : entry.watchedPaths) {
            CORE_LOG("  -> '", wp.string(), "'");
        }

        if (!IsWatchedPath(entry, ev.path)) {
            CORE_LOG("ShaderWatcher [OnFileChanged]: '", ev.path.string(),
                     "' not in watch list, skipping.");
            return;
        }

        CORE_LOG("ShaderWatcher [Worker]: change detected on '",
                 ev.path.filename().string(),
                 "'. Queuing recompile for: ", entry.mainPath);

        std::lock_guard lock(m_PendingMutex);
        m_PendingRecompiles.insert(entry.shader);
    }

    std::list<ShaderEntry>                             m_ActiveShaders;
    std::mutex                                         m_PendingMutex;
    std::unordered_set<Rendering::Bindables::Shader*>  m_PendingRecompiles;
};

} // namespace Engine::Systems