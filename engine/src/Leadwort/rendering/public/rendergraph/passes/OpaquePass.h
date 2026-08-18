#pragma once

#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/GlobalTextureSlots.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"
#include "ShadowMapPass.h"

namespace Leadwort::Rendering::RG::Passes {

class OpaquePass final : public IPass {

	public:
		explicit OpaquePass(RenderTexture& color, RenderTexture& depth, RenderTexture& shadowMap) noexcept
		  : m_Color(color), m_Depth(depth), m_ShadowMap(shadowMap) {}

		std::string_view GetName() const noexcept override { return "Opaque Pass"; }

		void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
			builder.SetRenderAttachment(*this, m_Color);
			builder.SetDepthAttachment(*this, m_Depth);
		}

		void Execute(const RenderContext& renderContext) noexcept override {
			glViewport(0, 0, m_Color.GetWidth(), m_Color.GetHeight());

			GLStateCache::Get().ApplyState(RenderPipelineState::Opaque());

			DrawCommandBuffer buffer{};

			for (const auto* renderer : (*renderContext.RenderQueues)[RenderQueue::Opaque]) {
				renderer->EmitDrawCommand(buffer, *renderContext.Camera);
			}

			buffer.Sort();
			m_ShadowMap.BindAsInput(GlobalTextureSlots::ShadowMapSlot);
			buffer.Draw();
		}

	private:
		RenderTexture& m_Color;
		RenderTexture& m_Depth;
		RenderTexture& m_ShadowMap;
	};

}