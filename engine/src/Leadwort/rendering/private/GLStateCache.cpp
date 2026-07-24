#include <Leadwort/rendering/public/GLStateCache.h>

#include <glad/glad.h>

namespace Leadwort::Rendering {

namespace {
    GLenum ToGLDepthFunc(const DepthFunc func) {
        switch (func) {
            case DepthFunc::Never:    return GL_NEVER;
            case DepthFunc::Less:     return GL_LESS;
            case DepthFunc::Equal:    return GL_EQUAL;
            case DepthFunc::LEqual:   return GL_LEQUAL;
            case DepthFunc::Greater:  return GL_GREATER;
            case DepthFunc::NotEqual: return GL_NOTEQUAL;
            case DepthFunc::GEqual:   return GL_GEQUAL;
            case DepthFunc::Always:   return GL_ALWAYS;
        }
        return GL_LESS;
    }

    GLenum ToGLStencilOp(const StencilOp op) {
        switch (op) {
            case StencilOp::Keep:     return GL_KEEP;
            case StencilOp::Zero:     return GL_ZERO;
            case StencilOp::Replace:  return GL_REPLACE;
            case StencilOp::Incr:     return GL_INCR;
            case StencilOp::IncrWrap: return GL_INCR_WRAP;
            case StencilOp::Decr:     return GL_DECR;
            case StencilOp::DecrWrap: return GL_DECR_WRAP;
            case StencilOp::Invert:   return GL_INVERT;
        }
        return GL_KEEP;
    }
}

void GLStateCache::Invalidate() {
    m_Initialized = false;
}

void GLStateCache::ApplyState(const RenderPipelineState& state) {
    if (!m_Initialized) {
        ApplyDepthTest(state.depthTest);
        ApplyDepthWrite(state.depthWrite);
        ApplyDepthFunc(state.depthFunc);
        ApplyCullMode(state.cullMode);
        ApplyBlendMode(state.blendMode);
        ApplyMultisample(state.multisample);

        ApplyStencilTest(state.stencilTest);
        ApplyStencilFunc(state.stencilFunc, state.stencilRef, state.stencilReadMask);
        ApplyStencilOp(state.stencilFailOp, state.stencilPassDepthFailOp, state.stencilPassDepthPassOp);
        ApplyStencilWriteMask(state.stencilWriteMask);
        ApplyColorWrite(state.colorWrite);

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

    if (state.stencilTest != m_CurrentRenderPipelineState.stencilTest) {
       ApplyStencilTest(state.stencilTest);
    }

    if (state.stencilFunc != m_CurrentRenderPipelineState.stencilFunc ||
        state.stencilRef != m_CurrentRenderPipelineState.stencilRef ||
        state.stencilReadMask != m_CurrentRenderPipelineState.stencilReadMask
    ) {
       ApplyStencilFunc(state.stencilFunc, state.stencilRef, state.stencilReadMask);
    }

    if (state.stencilFailOp != m_CurrentRenderPipelineState.stencilFailOp ||
        state.stencilPassDepthFailOp != m_CurrentRenderPipelineState.stencilPassDepthFailOp ||
        state.stencilPassDepthPassOp != m_CurrentRenderPipelineState.stencilPassDepthPassOp
    ) {
       ApplyStencilOp(state.stencilFailOp, state.stencilPassDepthFailOp, state.stencilPassDepthPassOp);
    }

    if (state.stencilWriteMask != m_CurrentRenderPipelineState.stencilWriteMask) {
       ApplyStencilWriteMask(state.stencilWriteMask);
    }

    if (state.colorWrite != m_CurrentRenderPipelineState.colorWrite) {
       ApplyColorWrite(state.colorWrite);
    }

    m_CurrentRenderPipelineState = state;
}

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
    glDepthFunc(ToGLDepthFunc(func));
}

void GLStateCache::ApplyCullMode(const CullMode mode) {
    if (mode == CullMode::None) {
        glDisable(GL_CULL_FACE);
        return;
    }

    glEnable(GL_CULL_FACE);
    switch (mode) {
        case CullMode::Back:         glCullFace(GL_BACK);  break;
        case CullMode::Front:        glCullFace(GL_FRONT); break;
        case CullMode::FrontAndBack: glCullFace(GL_FRONT_AND_BACK); break;
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
        case BlendMode::AlphaBlend:          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); break;
        case BlendMode::Additive:            glBlendFunc(GL_ONE, GL_ONE); break;
        case BlendMode::PremultipliedAlpha:  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); break;
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

void GLStateCache::ApplyStencilTest(const bool enabled) {
    if (enabled) {
	    glEnable(GL_STENCIL_TEST);
    }
    else {
	    glDisable(GL_STENCIL_TEST);
    }
}

void GLStateCache::ApplyStencilFunc(const DepthFunc func, const uint8_t ref, const uint8_t readMask) {
    glStencilFunc(ToGLDepthFunc(func), static_cast<GLint>(ref), static_cast<GLuint>(readMask));
}

void GLStateCache::ApplyStencilOp(const StencilOp fail, const StencilOp passDepthFail, const StencilOp passDepthPass) {
    glStencilOp(ToGLStencilOp(fail), ToGLStencilOp(passDepthFail), ToGLStencilOp(passDepthPass));
}

void GLStateCache::ApplyStencilWriteMask(const uint8_t mask) {
    glStencilMask(static_cast<GLuint>(mask));
}

void GLStateCache::ApplyColorWrite(const bool enabled) {
    const GLboolean v = enabled ? GL_TRUE : GL_FALSE;
    glColorMask(v, v, v, v);
}

} // Leadwort::Rendering