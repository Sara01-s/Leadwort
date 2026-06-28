#include "engine/systems/public/RenderSystem.h"

#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/Camera.h"
#include "engine/core/public/Window.h"
#include "engine/rendering/public/DefaultRenderPasses.h"
#include "engine/rendering/public/GLStateCache.h"
#include "engine/rendering/public/RenderPass.h"
#include "engine/systems/public/LightingSystem.h"
#include "engine/systems/public/SceneSystem.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Visit.h"

#include <glad/glad.h>
#include <variant>

#undef near
#undef far

namespace Engine::Systems {

using namespace Rendering;
using namespace Components;
using namespace Bindables;
using namespace Core;
using namespace Passes;

void RenderSystem::Initialize() {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glFrontFace(GL_CW);

    SetClearColor(Utils::Color::Gray20());

    SceneSystem::Get().OnSceneLoaded.Subscribe(
        [this](const Scene* scene) {
            m_SceneCollector.FindRenderersInScene(*scene);
        }, this
    );

    m_CameraUBO.Initialize();
	m_LightingUBO.Initialize();

	m_RenderPasses.emplace_back(CreateUnique<BackgroundPass>());
	m_RenderPasses.emplace_back(CreateUnique<OpaquePass>());
	m_RenderPasses.emplace_back(CreateUnique<AlphaTestPass>());
	m_RenderPasses.emplace_back(CreateUnique<GridPass>());
	m_RenderPasses.emplace_back(CreateUnique<TransparentPass>());
}

// ─────────────────────────────────────────────
//  Main render
// ─────────────────────────────────────────────

void RenderSystem::Render(Camera& camera, const RenderTarget& renderTarget) const {
	GLStateCache::Get().Invalidate();
	GLStateCache::Get().ApplyState(RenderPipelineState::Opaque());

	camera.aspect = renderTarget.GetAspectRatio();

	renderTarget.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

    auto queues = m_SceneCollector.BuildRenderQueues(camera);
    m_CameraUBO.Update(camera);

	const auto* directionalLight = LightingSystem::Get().GetDirectionalLight();
	m_LightingUBO.Update(directionalLight);

	const RenderContext renderContext {
		.camera = &camera,
		.renderTarget = &renderTarget,
		.renderQueues = &queues,
		.resolution = renderTarget.GetResolution()
	};

	for (const auto& pass : m_RenderPasses) {
		pass->Execute(renderContext);
	}

	renderTarget.Unbind();
	glViewport(0, 0, Window::Get().GetWidth(), Window::Get().GetHeight());
}

// ─────────────────────────────────────────────
//  UI / Overlay
// ─────────────────────────────────────────────

void RenderSystem::RenderUI() const {
    for (auto const& callback : m_OverlayCallbacks) {
        callback();
    }
}

void RenderSystem::AddOverlay(std::function<void()> callback) {
    m_OverlayCallbacks.push_back(std::move(callback));
}

void RenderSystem::ClearScreen() {
    glViewport(0, 0, Window::Get().GetWidth(), Window::Get().GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// ─────────────────────────────────────────────
//  Clear color / cleanup
// ─────────────────────────────────────────────

void RenderSystem::SetClearColor(const Utils::Color& color) {
    SetClearColor(color.r, color.g, color.b, color.a);
}

void RenderSystem::SetClearColor(const float r, const float g, const float b, const float a) {
    glClearColor(r, g, b, a);
}

} // namespace Engine::Systems