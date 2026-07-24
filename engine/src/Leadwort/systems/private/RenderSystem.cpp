#include <Leadwort/systems/public/RenderSystem.h>

#include "../../core/math/public/Color.h"
#include <Leadwort/asset-management/public/AssetManager.h>
#include <Leadwort/components/public/Camera.h>
#include <Leadwort/core/public/Window.h>
#include <Leadwort/rendering/public/DefaultRenderPasses.h>
#include <Leadwort/rendering/public/GLStateCache.h>
#include <Leadwort/rendering/public/RenderPass.h>
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
using namespace Passes;

void RenderSystem::Initialize() {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glFrontFace(GL_CW);

    SetClearColor(Color::Gray20());

    SceneSystem::Get().OnSceneLoaded.Subscribe(
        [this](const Scene* scene) {
            m_SceneCollector.FindRenderersInScene(*scene);
        }, this
    );

    m_CameraUBO.Initialize();
	m_LightingUBO.Initialize();
}

// ----------------------------------------------
//  Main render
// ----------------------------------------------

void RenderSystem::Render(Camera& camera, const RenderGraph& graph) const {
	GLStateCache::Get().Invalidate();

	auto queues = m_SceneCollector.BuildRenderQueues(camera);
	m_CameraUBO.Update(camera);
	m_LightingUBO.Update(LightingSystem::Get().GetDirectionalLight());

	graph.Execute(camera, queues, m_HighlightedEntity);

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

} // namespace Engine::Systems