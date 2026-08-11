#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class GridPass final : public IPass {
public:
	explicit GridPass(RenderTexture& color, RenderTexture& depth) noexcept
		: m_Color(color), m_Depth(depth)
	{
		glGenVertexArrays(1, &m_EmptyVAO);
	}

	std::string_view GetName() const noexcept override { return "Grid Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.SetRenderAttachment(*this, m_Color);
		builder.SetDepthAttachment(*this, m_Depth);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		if ((renderContext.camera->cullingMask & Utils::Layers::SCENE) == 0) {
			return;
		}

		GLStateCache::Get().ApplyState(RenderPipelineState::Grid());

		m_GridShader->Bind();
		m_GridShader->SetUniform("_InvProjectionMatrix", Inverse(renderContext.camera->GetProjectionMatrix()));
		m_GridShader->SetUniform("_Resolution", m_Color.GetResolution());

		glBindVertexArray(m_EmptyVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		m_GridShader->Unbind();
		glBindVertexArray(0);
	}

private:
	RenderTexture& m_Color;
	RenderTexture& m_Depth;
	Shared<Bindables::Shader> m_GridShader { AssetManagement::EngineAssets::GetShader("shaders/shd_grid.glsl") };
	GLuint m_EmptyVAO { 0 };
};

}