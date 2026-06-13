#include "engine/rendering/public/GLStateCache.h"

#include <glad/glad.h>

namespace Engine::Rendering {

void GLStateCache::Invalidate() {
    m_Initialized = false;
}

void GLStateCache::ApplyState(const RenderPipelineState& state) {
    if (!m_Initialized) {
        // First call (or after Invalidate): force every GL call once
        // so the cache and real GL state are guaranteed to match.
        ApplyDepthTest(state.depthTest);
        ApplyDepthWrite(state.depthWrite);
        ApplyDepthFunc(state.depthFunc);
        ApplyCullMode(state.cullMode);
        ApplyBlendMode(state.blendMode);
        ApplyMultisample(state.multisample);

        m_CurrentRenderPipelineState = state;
        m_Initialized = true;

        return;
    }

    if (state.depthTest != m_CurrentRenderPipelineState.depthTest) {
		ApplyDepthTest(state.depthTest);
	}

	if (state.depthWrite != m_CurrentRenderPipelineState.depthWrite) {
		ApplyDepthWrite(state.depthWrite);
	}

	// depthFunc only matters if depth test is enabled, but we still
    // diff it independently to keep the cache accurate.
    if (state.depthFunc != m_CurrentRenderPipelineState.depthFunc) {
		ApplyDepthFunc(state.depthFunc);
	}

	if (state.cullMode != m_CurrentRenderPipelineState.cullMode) {
		ApplyCullMode(state.cullMode);
	}

	if (state.blendMode != m_CurrentRenderPipelineState.blendMode) {
		ApplyBlendMode(state.blendMode);
	}

	if (state.multisample != m_CurrentRenderPipelineState.multisample) {
		ApplyMultisample(state.multisample);
	}

	m_CurrentRenderPipelineState = state;
}

// ─────────────────────────────────────────────
//  Individual appliers
// ─────────────────────────────────────────────

void GLStateCache::ApplyDepthTest(const bool enabled) {
    if (enabled) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
}

void GLStateCache::ApplyDepthWrite(const bool enabled) {
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void GLStateCache::ApplyDepthFunc(const DepthFunc func) {
    switch (func) {
        case DepthFunc::Never:    glDepthFunc(GL_NEVER);    break;
        case DepthFunc::Less:     glDepthFunc(GL_LESS);     break;
        case DepthFunc::Equal:    glDepthFunc(GL_EQUAL);    break;
        case DepthFunc::LEqual:   glDepthFunc(GL_LEQUAL);   break;
        case DepthFunc::Greater:  glDepthFunc(GL_GREATER);  break;
        case DepthFunc::NotEqual: glDepthFunc(GL_NOTEQUAL); break;
        case DepthFunc::GEqual:   glDepthFunc(GL_GEQUAL);   break;
        case DepthFunc::Always:   glDepthFunc(GL_ALWAYS);   break;
    }
}

void GLStateCache::ApplyCullMode(const CullMode mode) {
    if (mode == CullMode::None) {
        glDisable(GL_CULL_FACE);
        return;
    }

    glEnable(GL_CULL_FACE);
    switch (mode) {
        case CullMode::Back:          glCullFace(GL_BACK);  break;
        case CullMode::Front:         glCullFace(GL_FRONT); break;
        case CullMode::FrontAndBack:  glCullFace(GL_FRONT_AND_BACK); break;
        default: break;
    }
}

void GLStateCache::ApplyBlendMode(const BlendMode mode) {
    if (mode == BlendMode::Disabled) {
        glDisable(GL_BLEND);
        return;
    }

    glEnable(GL_BLEND);
    switch (mode) {
        case BlendMode::AlphaBlend:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case BlendMode::Additive:
            glBlendFunc(GL_ONE, GL_ONE);
            break;
        case BlendMode::PremultipliedAlpha:
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
        default: break;
    }
}

void GLStateCache::ApplyMultisample(const bool enabled) {
    if (enabled) {
		glEnable(GL_MULTISAMPLE);
	}
	else {
		glDisable(GL_MULTISAMPLE);
	}
}

} // namespace Engine::Rendering