#pragma once

#include "engine/rendering/public/RenderPipelineState.h"

namespace Engine::Rendering {

// Caches the currently-applied GL state and only issues
// glEnable/glDisable/etc calls when something actually changes.
//
// Single source of truth: every pass calls ApplyState(...)
// instead of poking GL directly.
class GLStateCache {
public:
	static GLStateCache& Get() {
		static GLStateCache instance;
		return instance;
	}

	// Forces the next ApplyState to re-issue everything.
	// Call this once at frame start (or after external code
	// that pokes GL directly, e.g. ImGui).
	void Invalidate();
	void ApplyState(const RenderPipelineState& state);

	[[nodiscard]] const RenderPipelineState& CurrentRenderPipelineState() const { return m_CurrentRenderPipelineState; }

private:
	GLStateCache() = default;

	static void ApplyDepthTest(bool enabled);
	static void ApplyDepthWrite(bool enabled);
	static void ApplyDepthFunc(DepthFunc func);
	static void ApplyCullMode(CullMode mode);
	static void ApplyBlendMode(BlendMode mode);
	static void ApplyMultisample(bool enabled);

	RenderPipelineState m_CurrentRenderPipelineState{};
	bool m_Initialized { false };
};

} // namespace Engine::Rendering
