#pragma once

#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/IBLBaker.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/GlobalSlots.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class OpaquePass final : public IPass {

	public:
	   explicit OpaquePass(
	       RenderTexture& color,
	       RenderTexture& depth,
	       RenderTexture& shadowMap,
	       const IBLBaker::Result& ibl
	   ) noexcept
	     : m_Color(color), m_Depth(depth), m_ShadowMap(shadowMap), m_IBL(ibl) {}

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

			m_ShadowMap.BindAsInput(TextureSlots::ShadowMapSlot);

			m_IBL.IrradianceMap->BindAsInput(TextureSlots::IBLIrradianceSlot);
			m_IBL.PrefilterMap->BindAsInput(TextureSlots::IBLPrefilterSlot);
			glActiveTexture(GL_TEXTURE0 + TextureSlots::IBLBrdfLUTSlot);
			glBindTexture(GL_TEXTURE_2D, m_IBL.BrdfLUT);

			buffer.Draw();
		}

    private:
		RenderTexture& m_Color;
		RenderTexture& m_Depth;
		RenderTexture& m_ShadowMap;
		const IBLBaker::Result& m_IBL;
    };

}