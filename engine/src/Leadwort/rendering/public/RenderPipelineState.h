#pragma once

#include <cstdint>

namespace Leadwort::Rendering {

	enum class DepthFunc : std::uint8_t {
		Never, Less, Equal, LEqual, Greater, NotEqual, GEqual, Always
	};

	enum class StencilOp : std::uint8_t {
		Keep, Zero, Replace, Incr, IncrWrap, Decr, DecrWrap, Invert
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

		bool      stencilTest      { false };
		DepthFunc stencilFunc      { DepthFunc::Always };
		uint8_t   stencilRef       { 0 };
		uint8_t   stencilReadMask  { 0xFF };
		uint8_t   stencilWriteMask { 0xFF };
		StencilOp stencilFailOp          { StencilOp::Keep };
		StencilOp stencilPassDepthFailOp { StencilOp::Keep };
		StencilOp stencilPassDepthPassOp { StencilOp::Keep };
		bool      colorWrite { true };

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
	            .depthWrite  = false,
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

	    static constexpr RenderPipelineState OutlineStencilWrite() {
	        return RenderPipelineState {
	            .depthTest   = false,
	            .depthWrite  = false,
        		.multisample = false,
	            .cullMode    = CullMode::Back,
	            .blendMode   = BlendMode::Disabled,
	            .stencilTest      = true,
	            .stencilFunc      = DepthFunc::Always,
	            .stencilRef       = 1,
	            .stencilReadMask  = 0xFF,
	            .stencilWriteMask = 0xFF,
	            .stencilFailOp          = StencilOp::Keep,
	            .stencilPassDepthFailOp = StencilOp::Replace,
	            .stencilPassDepthPassOp = StencilOp::Replace,
	            .colorWrite = false,
	        };
	    }

	    static constexpr RenderPipelineState OutlineDraw() {
	        return RenderPipelineState {
	            .depthTest   = false,
	            .depthWrite  = false,
        		.multisample = false,
        		.depthFunc  = DepthFunc::LEqual,
	            .cullMode   = CullMode::Back,
	            .blendMode  = BlendMode::Disabled,
	            .stencilTest      = true,
	            .stencilFunc      = DepthFunc::NotEqual,
	            .stencilRef       = 1,
	            .stencilReadMask  = 0xFF,
	            .stencilWriteMask = 0x00,
	            .colorWrite = true,
	        };
	    }

		static constexpr RenderPipelineState ShadowDepth() {
	    	return RenderPipelineState {
	    		.depthTest   = true,
				.depthWrite  = true,
				.multisample = false,
				.depthFunc   = DepthFunc::Less,
				.cullMode    = CullMode::Front,
				.blendMode   = BlendMode::Disabled,
				.colorWrite  = false,
			};
	    }

	    bool operator==(const RenderPipelineState& other) const = default;
	};

}