#pragma once

#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class AlphaTestPass final : public IPass {
public:
	explicit AlphaTestPass(RenderTexture& color, RenderTexture& depth) noexcept
		: m_Color(color), m_Depth(depth) {}

	std::string_view GetName() const noexcept override { return "Alpha Test Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.SetRenderAttachment(*this, m_Color);
		builder.SetDepthAttachment(*this, m_Depth);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		DrawCommandBuffer buffer{};
		GLStateCache::Get().ApplyState(RenderPipelineState::AlphaTest());

		for (const auto* renderer : (*renderContext.RenderQueues)[RenderQueue::AlphaTest]) {
			renderer->EmitDrawCommand(buffer, *renderContext.Camera);
		}

		buffer.Sort();
		buffer.Draw();
	}

private:
	RenderTexture& m_Color;
	RenderTexture& m_Depth;
};

}