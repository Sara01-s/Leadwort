#include <Leadwort/systems/public/RenderSystem.h>

#include "../../core/math/public/Color.h"
#include "Leadwort/components/behaviours/public/Light.h"
#include "Leadwort/core/public/Time.h"

#include <Leadwort/asset-management/public/AssetDatabase.h>
#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/rendering/public/GLStateCache.h>
#include <Leadwort/systems/public/LightingSystem.h>
#include <Leadwort/systems/public/SceneSystem.h>

#include <glad/glad.h>

#undef near
#undef far

namespace Leadwort::Systems {

using namespace Rendering;
using namespace Components;
using namespace Bindables;
using namespace Core;

void RenderSystem::Initialize() {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glFrontFace(GL_CW);

    SetClearColor(Color::Gray20());

	m_OnSceneLoadedToken = SceneSystem::Get().OnSceneLoaded.Subscribe(
		[this](const IScene* scene) {
			m_SceneCollector.FindRenderersInScene(*scene);
			m_HighlightedEntity = nullptr;
		}
	);

    m_CameraUBO.Initialize();
	m_LightingUBO.Initialize();
	m_TimeUBO.Initialize();
}

// ----------------------------------------------
//  Main render
// ----------------------------------------------
Mat4 FindAndCalculateLightSpaceMatrix(const LightingSystem::Lights& lights) {
	for (const auto* light : lights) {
		if (light && light->Type == Behaviours::Light::LightType::Directional) {
			return Behaviours::Light::CalculateLightSpaceMatrix(*light);
		}
	}

	return Mat4::Identity(); // no directional light in scene
}

void RenderSystem::Render(Camera& camera, const RG::RenderGraph& graph) const {
	GLStateCache::Get().Invalidate();

	auto queues { m_SceneCollector.BuildRenderQueues(camera) };
	m_CameraUBO.Update(camera);

	const auto lights = LightingSystem::Get().GetLights();
	const Mat4 lightSpaceMatrix = FindAndCalculateLightSpaceMatrix(lights);

	m_LightingUBO.Update(lights, lightSpaceMatrix);
	m_TimeUBO.Update(Time::GetTime(), Time::GetDeltaTime());

	MeshRenderer* highlightedMeshRenderer {
		m_HighlightedEntity == nullptr ? nullptr : m_HighlightedEntity->GetComponent<MeshRenderer>()
	};

	graph.Execute(camera, queues, highlightedMeshRenderer, lightSpaceMatrix);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, Window::Get().GetWidth(), Window::Get().GetHeight());
}

void RenderSystem::ClearScreen() {
    glViewport(0, 0, Window::Get().GetWidth(), Window::Get().GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::SetClearColor(const Color color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void RenderSystem::Clear() const noexcept {
	SceneSystem::Get().OnSceneLoaded.Unsubscribe(m_OnSceneLoadedToken);
}

} // namespace Engine::Systems