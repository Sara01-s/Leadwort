#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"
#include "Leadwort/systems/public/CameraSystem.h"
#include "Leadwort/utils/public/PrimitiveMeshes.h"

namespace Leadwort::Rendering::RG::Passes {

class PostProcessPass final : public IPass {
public:
	explicit PostProcessPass(RenderTexture& src, RenderTexture& dst, RenderTexture& depth) noexcept
		: m_Src(src), m_Dst(dst), m_Depth(depth)
	{
		const Shared<Bindables::Shader> shader {
			AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
		};

		m_PostProcessMaterial = AssetManagement::EngineAssets::CreateMaterial(shader);
	}

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.UseTexture(*this, m_Src, AccessFlags::Read);
		builder.UseTexture(*this, m_Depth, AccessFlags::Read);
		builder.SetRenderAttachment(*this, m_Dst);
	}

	std::string_view GetName() const noexcept override { return "Post-Process Pass"; }

	void Execute(const RenderContext& renderContext) noexcept override {
		GLStateCache::Get().ApplyState(RenderPipelineState::PostProcess());

		const Vec2 resolution { m_Src.GetResolution() };
		const float nearPane { Systems::CameraSystem::Get().GetMainCamera()->NearPlane };
		const float farPlane { Systems::CameraSystem::Get().GetMainCamera()->FarPlane };
		const Vec4 screenParams { resolution.x, resolution.y, nearPane, farPlane };

		LW_ASSERT(m_Src.IsValid() && m_Depth.IsValid(), "PostProcessPass: input texture has GpuID 0");

		m_PostProcessMaterial->SetTexture("_ScreenTexture", m_Src.GetGpuID(), -1, GL_TEXTURE_2D);
		m_PostProcessMaterial->SetTexture("_DepthTexture", m_Depth.GetGpuID(), -1, GL_TEXTURE_2D);
		m_PostProcessMaterial->SetVec4("_ScreenParams", screenParams);
		m_PostProcessMaterial->Bind();

		m_FullScreenQuadMesh->Bind();
		glDrawElements(m_FullScreenQuadMesh->GetTopology(), m_FullScreenQuadMesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
		m_FullScreenQuadMesh->Unbind();
	}

private:
	RenderTexture& m_Src;
	RenderTexture& m_Dst;
	RenderTexture& m_Depth;

	Shared<Bindables::Mesh> m_FullScreenQuadMesh { Utils::PrimitiveMeshes::Get().Quad() };
	Shared<Bindables::Material> m_PostProcessMaterial;
};

}