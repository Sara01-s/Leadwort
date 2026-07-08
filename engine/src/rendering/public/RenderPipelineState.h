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
    bool      depthTest   { true };
    bool      depthWrite  { true };
    bool      multisample { true };
    DepthFunc depthFunc { DepthFunc::Less };
    CullMode  cullMode  { CullMode::Back };
    BlendMode blendMode { BlendMode::Disabled };

    // Presets
    static constexpr RenderPipelineState Opaque() {
        return RenderPipelineState {
            .depthTest  = true,
            .depthWrite = true,
            .depthFunc  = DepthFunc::Less,
            .cullMode   = CullMode::Back,
            .blendMode  = BlendMode::Disabled,
        };
    }

    static constexpr RenderPipelineState AlphaTest() {
        return RenderPipelineState {
            .depthTest  = true,
            .depthWrite = true,
            .depthFunc  = DepthFunc::Less,
            .cullMode   = CullMode::None,
            .blendMode  = BlendMode::AlphaBlend,
        };
    }

    static constexpr RenderPipelineState Transparent() {
        return RenderPipelineState {
            .depthTest  = true,
            .depthWrite = false,
            .depthFunc  = DepthFunc::Less,
            .cullMode   = CullMode::Back,
            .blendMode  = BlendMode::AlphaBlend,
        };
    }

    static constexpr RenderPipelineState Grid() {
        return RenderPipelineState {
            .depthTest   = true,
            .depthWrite  = true,
            .multisample = true,
            .depthFunc   = DepthFunc::LEqual,
            .cullMode    = CullMode::None,
            .blendMode   = BlendMode::AlphaBlend,
        };
    }

    static constexpr RenderPipelineState PostProcess() {
        return RenderPipelineState {
            .depthTest   = false,
            .depthWrite  = false,
            .multisample = false,
            .cullMode    = CullMode::None,
            .blendMode   = BlendMode::Disabled,
        };
    }

    static constexpr RenderPipelineState Overlay() {
        return RenderPipelineState {
            .depthTest   = false,
            .depthWrite  = false,
            .multisample = true,
            .cullMode    = CullMode::None,
            .blendMode   = BlendMode::AlphaBlend,
        };
    }

    bool operator==(const RenderPipelineState& other) const = default;
};

}