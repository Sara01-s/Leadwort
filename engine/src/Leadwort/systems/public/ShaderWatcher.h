#pragma once

#include <Leadwort/rendering/bindables/public/Shader.h>
#include <Leadwort/utils/public/Logger.h>

#include <algorithm>
#include <vector>

namespace Leadwort::Systems {

class ShaderWatcher {
public:
    static ShaderWatcher& Get() {
        static ShaderWatcher instance;
        return instance;
    }

    ShaderWatcher(const ShaderWatcher&) = delete;
    ShaderWatcher& operator=(const ShaderWatcher&) = delete;

    void RegisterShader(Rendering::Bindables::Shader* shader) {
        if (std::ranges::find(m_ActiveShaders, shader) != m_ActiveShaders.end()) {
            LW_WARN("ShaderWatcher [Register]: shader already registered, ignoring. ptr=",
                     reinterpret_cast<uintptr_t>(shader));
            return;
        }

        m_ActiveShaders.push_back(shader);
        LW_LOG("ShaderWatcher [Register]: shader registered. total=", m_ActiveShaders.size());
    }

    void UnregisterShader(Rendering::Bindables::Shader* shader) {
        const auto it = std::ranges::find(m_ActiveShaders, shader);

        if (it == m_ActiveShaders.end()) {
            LW_WARN("ShaderWatcher [Unregister]: shader not found, ignoring.");
            return;
        }

        m_ActiveShaders.erase(it);
        LW_LOG("ShaderWatcher [Unregister]: shader removed. total=", m_ActiveShaders.size());
    }

    void MarkPending(Rendering::Bindables::Shader* shader) {
        if (std::ranges::find(m_ActiveShaders, shader) == m_ActiveShaders.end()) {
            LW_WARN("ShaderWatcher [MarkPending]: shader is not registered, ignoring.");
            return;
        }

        if (std::ranges::find(m_PendingShaders, shader) != m_PendingShaders.end()) {
            return; // already pending
        }

        m_PendingShaders.push_back(shader);
    }

    void MarkAllShaderAsPending() {
        m_PendingShaders = m_ActiveShaders;
    }

    void RecompilePendingShaders() {
        if (m_PendingShaders.empty()) {
            return;
        }

        LW_LOG("ShaderWatcher [Recompile]: recompiling ", m_PendingShaders.size(), " shader(s).");

        for (auto* shader : m_PendingShaders) {
            try {
                shader->Compile();
                LW_LOG("ShaderWatcher [Recompile]: OK. ptr=", reinterpret_cast<uintptr_t>(shader),
                         " version=", shader->GetVersion());
            }
            catch (const std::exception& e) {
                LW_ERROR("ShaderWatcher [Recompile]: FAILED. ptr=", reinterpret_cast<uintptr_t>(shader),
                           " error=", e.what());
            }
        }

        m_PendingShaders.clear();
    }

    [[nodiscard]] bool HasPendingShaders() const noexcept {
        return !m_PendingShaders.empty();
    }

private:
    ShaderWatcher() = default;

    std::vector<Rendering::Bindables::Shader*> m_ActiveShaders{};
    std::vector<Rendering::Bindables::Shader*> m_PendingShaders{};
};

} // namespace Leadwort::Systems