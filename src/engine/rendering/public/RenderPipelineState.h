#pragma once

#include <cstdint>

namespace Engine::Rendering {
enum class DepthFunc : std::uint8_t {
	Never, Less, Equal, LEqual, Greater, NotEqual, GEqual, Always
};

enum class CullMode : std::uint8_t {
	None, Back, Front, FrontAndBack
};

enum class BlendMode : std::uint8_t {
	Disabled,
	AlphaBlend,     // SRC_ALPHA, ONE_MINUS_SRC_ALPHA
	Additive,       // ONE, ONE
	PremultipliedAlpha
};

struct RenderPipelineState {
    bool      depthTest   = true;
    bool      depthWrite  = true;
    bool      multisample = true;
    DepthFunc depthFunc  = DepthFunc::Less;
    CullMode  cullMode   = CullMode::Back;
    BlendMode blendMode  = BlendMode::Disabled;

    // Presets
    static constexpr RenderPipelineState Opaque() {
        RenderPipelineState state;
        state.depthTest  = true;
        state.depthWrite = true;
        state.depthFunc  = DepthFunc::Less;
        state.cullMode   = CullMode::Back;
        state.blendMode  = BlendMode::Disabled;
        return state;
    }

    static constexpr RenderPipelineState AlphaTest() {
        RenderPipelineState state;
        state.depthTest  = true;
        state.depthWrite = true;
        state.depthFunc  = DepthFunc::Less;
        state.cullMode   = CullMode::None;
        state.blendMode  = BlendMode::AlphaBlend;
        return state;
    }

    static constexpr RenderPipelineState Transparent() {
        RenderPipelineState state;
        state.depthTest  = true;
        state.depthWrite = false;
        state.depthFunc  = DepthFunc::Less;
        state.cullMode   = CullMode::Back;
        state.blendMode  = BlendMode::AlphaBlend;
        return state;
    }

    static constexpr RenderPipelineState Grid() {
        RenderPipelineState state;
        state.depthTest  = true;
        state.depthWrite = false;
        state.depthFunc  = DepthFunc::LEqual;
        state.cullMode   = CullMode::None;
        state.blendMode  = BlendMode::AlphaBlend;
        return state;
    }

    static constexpr RenderPipelineState PostProcess() {
        RenderPipelineState state;
        state.depthTest  = false;
        state.depthWrite = false;
        state.cullMode   = CullMode::None;
        state.blendMode  = BlendMode::Disabled;
        state.multisample = false;
        return state;
    }

    static constexpr RenderPipelineState Overlay() {
        RenderPipelineState state;
        state.depthTest  = false;
        state.depthWrite = false;
        state.cullMode   = CullMode::None;
        state.blendMode  = BlendMode::AlphaBlend;
        state.multisample = true;
        return state;
    }

    bool operator==(const RenderPipelineState& other) const = default;
};

}