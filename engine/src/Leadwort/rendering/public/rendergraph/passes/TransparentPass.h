#pragma once

#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class TransparentPass final : public IPass {
public:
	explicit TransparentPass(RenderTexture& color, RenderTexture& depth, const IBLBaker::Result& ibl) noexcept
		: m_Color(color), m_Depth(depth), m_IBL(ibl) {}

	std::string_view GetName() const noexcept override { return "Transparent Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.SetRenderAttachment(*this, m_Color);
		builder.SetDepthAttachment(*this, m_Depth);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		DrawCommandBuffer buffer{};
		GLStateCache::Get().ApplyState(RenderPipelineState::Transparent());

		for (const auto* renderer : (*renderContext.RenderQueues)[RenderQueue::Transparent]) {
			renderer->EmitDrawCommand(buffer, *renderContext.Camera);
		}

		buffer.Sort();

		m_IBL.IrradianceMap->BindAsInput(TextureSlots::IBLIrradianceSlot);
		m_IBL.PrefilterMap->BindAsInput(TextureSlots::IBLPrefilterSlot);
		glActiveTexture(GL_TEXTURE0 + TextureSlots::IBLBrdfLUTSlot);
		glBindTexture(GL_TEXTURE_2D, m_IBL.BrdfLUT);

		buffer.Draw();
	}

private:
	RenderTexture& m_Color;
	RenderTexture& m_Depth;
	const IBLBaker::Result& m_IBL;
};

}