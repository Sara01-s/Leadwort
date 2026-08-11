#pragma once

#include "Leadwort/asset-management/public/AssetDatabase.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/PostProcess.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {

class PostProcessPass final : public IPass {
public:
	explicit PostProcessPass(RenderTexture& src, RenderTexture& dst) noexcept
		: m_Src(src), m_Dst(dst) {}

	std::string_view GetName() const noexcept override { return "Post-Process Pass"; }

	void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
		builder.UseTexture(*this, m_Src, AccessFlags::Read);
		builder.SetRenderAttachment(*this, m_Dst);
	}

	void Execute(const RenderContext& renderContext) noexcept override {
		GLStateCache::Get().ApplyState(RenderPipelineState::PostProcess());
		m_PostProcess->Render(m_Src.GetGpuID());
	}

private:
	RenderTexture& m_Src;
	RenderTexture& m_Dst;
	Unique<PostProcess> m_PostProcess { CreateUnique<PostProcess>(
		AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
	)};
};

}