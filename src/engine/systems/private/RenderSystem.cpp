#include "engine/systems/public/RenderSystem.h"

#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/Camera.h"
#include "engine/core/public/Window.h"
#include "engine/systems/public/SceneSystem.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Visit.h"
#include "engine/rendering/public/GLStateCache.h"

#include <glad/glad.h>
#include <variant>

#undef near
#undef far

namespace Engine::Systems {

using namespace Rendering;

void RenderSystem::Init() {
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    SetClearColor(Utils::Color::Gray20());

    SceneSystem::Get().OnSceneLoaded.Subscribe(
        [this](const Core::Scene* scene) {
            m_SceneCollector.FindRenderersInScene(scene);
        }, this
    );

    m_CameraUBO.Init();
	m_PostProcess = CreateUnique<PostProcess>(
		AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
	);

    glGenVertexArrays(1, &m_EmptyVAO);
}

// ─────────────────────────────────────────────
//  Render passes
// ─────────────────────────────────────────────

void RenderSystem::RenderGrid(const Components::Camera* camera, const Vec2 resolution) const {
	GLStateCache::Get().ApplyState(RenderPipelineState::Grid());

	m_GridShader->Bind();

	const Mat4 cameraWorld      = camera->GetEntity().GetTransform().GetWorldMatrix();
	const Mat4 cameraProjection = camera->GetProjectionMatrix();

	m_GridShader->SetUniform("_InvViewMatrix",       cameraWorld);
	m_GridShader->SetUniform("_InvProjectionMatrix", Inverse(cameraProjection));
	m_GridShader->SetUniform("_Resolution",          resolution);

	glBindVertexArray(m_EmptyVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	m_GridShader->Unbind();
	glBindVertexArray(0);
}

void RenderSystem::RenderBackground(const Components::Camera* camera, const Vec2 resolution) {
    std::visit(overloaded {
        [](const Components::Camera::SkyBox& sky) {
            sky.skybox->Render();
        },
        [](const Components::Camera::SolidColor& solid) {
            const auto& bg = solid.color;
            glClearColor(bg.r, bg.g, bg.b, bg.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
    }, camera->background);

	if ((camera->cullingMask & Utils::Layers::SCENE) != 0) {
		Get().RenderGrid(camera, resolution);
	}
}


void RenderSystem::RenderOpaque(const RenderQueues& queues, const Components::Camera* camera) {
	const auto& opaqueQueue = queues[static_cast<std::uint8_t>(RenderQueue::Opaque)];

	GLStateCache::Get().ApplyState(RenderPipelineState::Opaque());

	for (auto* renderer : opaqueQueue) {
		renderer->Render(camera);
	}
}

void RenderSystem::RenderAlphaTest(const RenderQueues& queues, const Components::Camera* camera) {
	const auto& alphaTestQueue = queues[static_cast<std::uint8_t>(RenderQueue::AlphaTest)];

	GLStateCache::Get().ApplyState(RenderPipelineState::AlphaTest());

	for (auto* renderer : alphaTestQueue) {
		renderer->Render(camera);
	}
}


void RenderSystem::SortTransparents(std::vector<Components::Renderer*>& transparents, const Vec3& camPos) {
    for (int i = 1; i < static_cast<int>(transparents.size()); ++i) {
        Components::Renderer* key = transparents[i];
        const float keyDist = Distance(camPos, key->GetEntity().GetTransform().GetWorldPosition());
        int j = i - 1;

        while (j >= 0 && Distance(camPos, transparents[j]->GetEntity().GetTransform().GetWorldPosition()) < keyDist) {
            transparents[j + 1] = transparents[j];
            --j;
        }

        transparents[j + 1] = key;
    }
}

void RenderSystem::RenderTransparent(RenderQueues& queues, const Components::Camera* camera) {
	auto& transparents = queues[static_cast<std::uint8_t>(RenderQueue::Transparent)];

	GLStateCache::Get().ApplyState(RenderPipelineState::Transparent());

	SortTransparents(transparents, camera->GetEntity().GetTransform().GetWorldPosition());

	for (auto* renderer : transparents) {
		renderer->Render(camera);
	}
}

// ─────────────────────────────────────────────
//  Main render
// ─────────────────────────────────────────────

void RenderSystem::Render(
    Components::Camera* camera,
    const Bindables::RenderTarget* renderTarget
) const {
    GLStateCache::Get().Invalidate();
	GLStateCache::Get().ApplyState(RenderPipelineState::Opaque());

	camera->aspect = static_cast<float>(renderTarget->GetWidth()) / static_cast<float>(renderTarget->GetHeight());

    renderTarget->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, renderTarget->GetWidth(), renderTarget->GetHeight());

    m_CameraUBO.Update(camera);

    const auto resolution = Vec2(renderTarget->GetWidth(), renderTarget->GetHeight());
    RenderBackground(camera, resolution);

    auto queues = m_SceneCollector.BuildRenderQueues(camera);

    RenderOpaque(queues, camera);
    RenderAlphaTest(queues, camera);
    RenderTransparent(queues, camera);

    renderTarget->Unbind();

    glViewport(0, 0, Core::Window::Get().GetWidth(), Core::Window::Get().GetHeight());
}

// ─────────────────────────────────────────────
//  UI / Overlay
// ─────────────────────────────────────────────

void RenderSystem::RenderPostProcess(const Bindables::RenderTarget* src, const Bindables::RenderTarget* dst) const {
	CORE_ASSERT(m_PostProcess, "Post Process has not been initialized.");

	dst->Bind();
	glViewport(0, 0, dst->GetWidth(), dst->GetHeight());
	glClear(GL_COLOR_BUFFER_BIT);

	GLStateCache::Get().ApplyState(RenderPipelineState::PostProcess());
	m_PostProcess->Render(src->GetTextureGpuID(), src->GetWidth(), src->GetHeight());

	dst->Unbind();
}

void RenderSystem::RenderUI() const {
    for (auto const& callback : m_OverlayCallbacks) {
        callback();
    }
}

void RenderSystem::AddOverlay(std::function<void()> callback) {
    m_OverlayCallbacks.push_back(std::move(callback));
}

void RenderSystem::ClearScreen() {
    glViewport(0, 0, Core::Window::Get().GetWidth(), Core::Window::Get().GetHeight());
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

void RenderSystem::Clear() {
	// TODO: Clear whatever need no be cleared here.
}

RenderSystem::~RenderSystem() {
    Clear();
}

} // namespace Engine::Systems