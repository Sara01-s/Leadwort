#include "engine/systems/public/RenderSystem.h"

#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/Camera.h"
#include "engine/core/public/Window.h"
#include "engine/systems/public/SceneSystem.h"
#include "engine/utils/public/Color.h"
#include "engine/utils/public/Visit.h"

#include <glad/glad.h>
#include <variant>

#undef near
#undef far

namespace Engine::Systems {

// ─────────────────────────────────────────────
//  Init
// ─────────────────────────────────────────────

void RenderSystem::Init() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

    SetClearColor(Utils::Color::Gray20());

    SceneSystem::Get().OnSceneLoaded.Subscribe(
        [this](const Core::Scene* scene) {
            m_SceneCollector.FindRenderersInScene(scene);
        }, this
    );

    m_CameraUBO.Init();
	m_PostProcess = CreateUnique<Rendering::PostProcess>(
		AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
	);

    glGenVertexArrays(1, &m_EmptyVAO);
}

// ─────────────────────────────────────────────
//  Render passes
// ─────────────────────────────────────────────

void RenderSystem::RenderGrid(const Components::Camera* camera, const Vec2 resolution) const {
	glDisable(GL_CULL_FACE);
	m_GridShader->Bind();

	const Mat4 cameraWorld = camera->GetEntity().GetTransform().GetWorldMatrix();
	const Mat4 cameraProjection  = camera->GetProjectionMatrix();

	m_GridShader->SetUniform("_InvViewMatrix",       cameraWorld);
	m_GridShader->SetUniform("_InvProjectionMatrix", Inverse(cameraProjection));
	m_GridShader->SetUniform("_Resolution",          resolution);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(m_EmptyVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_GridShader->Unbind();
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
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

void RenderSystem::RenderOpaque(const Rendering::RenderQueues& queues, const Components::Camera* camera) {
    const auto& opaqueQueue = queues[static_cast<std::uint8_t>(Rendering::RenderQueue::Opaque)];

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    for (auto* renderer : opaqueQueue) {
        renderer->Render(camera);
    }
}

void RenderSystem::RenderAlphaTest(const Rendering::RenderQueues& queues, const Components::Camera* camera) {
    const auto& alphaTestQueue = queues[static_cast<std::uint8_t>(Rendering::RenderQueue::AlphaTest)];

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);

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

void RenderSystem::RenderTransparent(Rendering::RenderQueues& queues, const Components::Camera* camera) {
    auto& transparents = queues[static_cast<std::uint8_t>(Rendering::RenderQueue::Transparent)];

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    SortTransparents(transparents, camera->GetEntity().GetTransform().GetWorldPosition());

    for (auto* renderer : transparents) {
        renderer->Render(camera);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

// ─────────────────────────────────────────────
//  Main render
// ─────────────────────────────────────────────

static void ResetGLState() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
}

void RenderSystem::Render(
    Components::Camera* camera,
    const Rendering::Bindables::RenderTarget* renderTarget
) const {
    ResetGLState();

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

void RenderSystem::RenderPostProcess(
	const Rendering::Bindables::RenderTarget* src,
	const Rendering::Bindables::RenderTarget* dst
) const {
	CORE_ASSERT(m_PostProcess, "Post Process has not been initialized.");

	dst->Bind();
	glViewport(0, 0, dst->GetWidth(), dst->GetHeight());
	glClear(GL_COLOR_BUFFER_BIT);

	ApplyPostProcessState();
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

// ─────────────────────────────────────────────
//  GL State
// ─────────────────────────────────────────────

void RenderSystem::ClearScreen() {
    glViewport(0, 0, Core::Window::Get().GetWidth(), Core::Window::Get().GetHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::ApplySceneState() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}

void RenderSystem::ApplyPostProcessState() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
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