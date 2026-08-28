#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class ShadowMapPass final : public IPass {
public:
	explicit ShadowMapPass(RenderTexture& shadowDepth) noexcept
		: m_ShadowDepth(shadowDepth)
	{
		m_ShadowShader = AssetManagement::EngineAssets::GetShader("shaders/shadow/shd_shadow_depth.glsl");
	}

	std::string_view GetName() const noexcept override { return "Shadow Map Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.SetDepthAttachment(*this, m_ShadowDepth);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		GLStateCache::Get().ApplyState(RenderPipelineState::ShadowDepth());

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glViewport(0, 0, m_ShadowDepth.GetWidth(), m_ShadowDepth.GetHeight());
		glClear(GL_DEPTH_BUFFER_BIT);

		m_ShadowShader->Bind();
		m_ShadowShader->SetUniform("_LightSpaceMatrix", renderContext.LightSpaceMatrix);

		DrawCommandBuffer buffer{};

		// Alpha-tested geometry casts shadows as well: the depth shader discards the same
		// texels the lit pass does. Transparent (glTF BLEND) geometry is left out on
		// purpose, it has no meaningful opaque shadow.
		for (const RenderQueue queue : { RenderQueue::Opaque, RenderQueue::AlphaTest }) {
			for (const auto* renderer : (*renderContext.RenderQueues)[queue]) {
				renderer->EmitDrawCommand(buffer, *renderContext.Camera);
			}
		}
		buffer.DrawShadowCasters(*m_ShadowShader);

		m_ShadowShader->Unbind();
	}

	[[nodiscard]] const RenderTexture& GetShadowDepth() const noexcept { return m_ShadowDepth; }

private:
	RenderTexture& m_ShadowDepth;
	Shared<Bindables::Shader> m_ShadowShader;
};

}