#include "../public/PostProcess.h"

#include "engine/core/public/Window.h"
#include "engine/utils/public/PrimitiveMeshes.h"

namespace Engine::Rendering {

PostProcess::PostProcess(const std::shared_ptr<Bindables::Shader>& shader)
	: m_Mesh(Utils::PrimitiveMeshes::Get().Quad()), m_Material(shader)
{}

void PostProcess::Render(const unsigned int sceneTextureID) {
	glViewport(0, 0, Core::Window::Get().GetWidth(), Core::Window::Get().GetHeight());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	m_Material.SetTexture("_ScreenTexture", sceneTextureID, 0);
	m_Material.Bind();

	m_Mesh->Bind();
	glDrawElements(m_Mesh->GetTopology(), m_Mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
	m_Mesh->Unbind();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
}

} // namespace Engine::Rendering