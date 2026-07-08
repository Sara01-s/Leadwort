#include "../public/PostProcess.h"

#include "asset-management/public/AssetManager.h"

#include <glad/glad.h>

#include "utils/public/PrimitiveMeshes.h"

namespace Engine::Rendering {

PostProcess::PostProcess(const Shared<Bindables::Shader>& shader)
	: m_Mesh(Utils::PrimitiveMeshes::Get().Quad())
{
	m_Material = AssetManagement::EngineAssets::CreateMaterial(shader);
}

void PostProcess::Render(const GpuID sceneTextureID) const {
	m_Material->SetTexture("_ScreenTexture", sceneTextureID, 0);
	m_Material->Bind();

	m_Mesh->Bind();
	glDrawElements(m_Mesh->GetTopology(), m_Mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
	m_Mesh->Unbind();
}

} // namespace Engine::Rendering