#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/components/public/MeshRenderer.h"
#include "Leadwort/rendering/public/CoordinateSystem.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class OutlinePass final : public IPass {
public:
	explicit OutlinePass(RenderTexture& color, RenderTexture& depth) noexcept
		: m_Color(color), m_Depth(depth) {}

	std::string_view GetName() const noexcept override { return "Outline Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.SetRenderAttachment(*this, m_Color);
		builder.SetDepthAttachment(*this, m_Depth);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		if (renderContext.highlightedMeshRenderer == nullptr) {
			return;
		}

		const auto& entity      { renderContext.highlightedMeshRenderer->GetEntity() };
		const auto& mesh        { renderContext.highlightedMeshRenderer->mesh };
		const Mat4  modelMatrix { CoordinateSystem::CalculateModelMatrix(entity.GetTransform()) };

		GLStateCache::Get().ApplyState(RenderPipelineState::OutlineStencilWrite());
		DrawMesh(*mesh, modelMatrix, *m_MaskShader);

		GLStateCache::Get().ApplyState(RenderPipelineState::OutlineDraw());

		m_OutlineShader->Bind();
		m_OutlineShader->SetUniform("_ModelMatrix", modelMatrix);
		m_OutlineShader->SetUniform("_OutlineThickness", 0.3f);
		m_OutlineShader->SetUniform("_OutlineColor", Vec4(0.941f, 0.682f, 0.082f, 1.0f));

		mesh->Bind();
		glDrawElements(mesh->GetTopology(), mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		mesh->Unbind();

		m_OutlineShader->Unbind();
	}

private:
	static void DrawMesh(const Bindables::Mesh& mesh, const Mat4& modelMatrix, const Bindables::Shader& shader) noexcept {
		shader.Bind();
		shader.SetUniform("_ModelMatrix", modelMatrix);
		mesh.Bind();
		glDrawElements(mesh.GetTopology(), mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		mesh.Unbind();
		shader.Unbind();
	}

private:
	RenderTexture& m_Color;
	RenderTexture& m_Depth;
	Shared<Bindables::Shader> m_MaskShader    { AssetManagement::EngineAssets::GetShader("shaders/shd_outline_mask.glsl") };
	Shared<Bindables::Shader> m_OutlineShader { AssetManagement::EngineAssets::GetShader("shaders/shd_outline.glsl") };
};

}