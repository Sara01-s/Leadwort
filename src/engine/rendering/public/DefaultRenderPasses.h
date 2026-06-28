#pragma once

#include "engine/components/public/Renderer.h"
#include "engine/components/public/Transform.h"
#include "engine/core/public/Entity.h"
#include "engine/rendering/public/GLStateCache.h"
#include "engine/rendering/public/RenderPass.h"
#include "engine/rendering/public/RenderPipelineState.h"
#include "engine/utils/public/Visit.h"

#include <glad/glad.h>
#include <variant>

#undef near
#undef far

namespace Engine::Rendering::Passes {

// ─────────────────────────────────────────────
//  Background
// ─────────────────────────────────────────────

class BackgroundPass final : public RenderPass {
public:
    void Execute(const RenderContext& renderContext) override {
        std::visit(overloaded {
            [](const Components::Camera::SkyBox& sky) {
                sky.skybox->Render();
            },
            [](const Components::Camera::SolidColor& solid) {
                const auto& bg = solid.color;
                glClearColor(bg.r, bg.g, bg.b, bg.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
        }, renderContext.camera->background);
    }

    constexpr std::string_view GetName() const override { return "Background"; }
};

// ─────────────────────────────────────────────
//  Opaque
// ─────────────────────────────────────────────

class OpaquePass final : public RenderPass {
public:
    void Execute(const RenderContext& renderContext) override {
        GLStateCache::Get().ApplyState(RenderPipelineState::Opaque());

        for (auto* renderer : (*renderContext.renderQueues)[static_cast<std::size_t>(RenderQueue::Opaque)]) {
            renderer->Render(renderContext.camera);
        }
    }

    constexpr std::string_view GetName() const override { return "Opaque"; }
};

// ─────────────────────────────────────────────
//  AlphaTest
// ─────────────────────────────────────────────

class AlphaTestPass final : public RenderPass {
public:
    void Execute(const RenderContext& renderContext) override {
        GLStateCache::Get().ApplyState(RenderPipelineState::AlphaTest());

        for (auto* renderer : (*renderContext.renderQueues)[static_cast<std::size_t>(RenderQueue::AlphaTest)]) {
            renderer->Render(renderContext.camera);
        }
    }

    constexpr std::string_view GetName() const override { return "AlphaTest"; }
};

// ─────────────────────────────────────────────
//  Grid
// ─────────────────────────────────────────────

class GridPass final : public RenderPass {
public:
	GridPass() {
		glGenVertexArrays(1, &m_EmptyVAO);
	}

    void Execute(const RenderContext& renderContext) override {
        if ((renderContext.camera->cullingMask & Utils::Layers::SCENE) == 0) {
            return;
        }

        GLStateCache::Get().ApplyState(RenderPipelineState::Grid());

        m_GridShader->Bind();
        m_GridShader->SetUniform("_InvProjectionMatrix", Inverse(renderContext.camera->GetProjectionMatrix()));
        m_GridShader->SetUniform("_Resolution",          renderContext.resolution);

        glBindVertexArray(m_EmptyVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_GridShader->Unbind();
        glBindVertexArray(0);
    }

    constexpr std::string_view GetName() const override { return "Grid"; }

private:
	Shared<Bindables::Shader> m_GridShader { AssetManagement::EngineAssets::GetShader("shaders/shd_grid.glsl") };
    GLuint m_EmptyVAO { 0 };
};

// ─────────────────────────────────────────────
//  Transparent
// ─────────────────────────────────────────────

class TransparentPass final : public RenderPass {
public:
    void Execute(const RenderContext& renderContext) override {
        GLStateCache::Get().ApplyState(RenderPipelineState::Transparent());

        auto& transparents { (*renderContext.renderQueues)[static_cast<std::size_t>(RenderQueue::Transparent)] };
        const Vec3 camPos { renderContext.camera->GetEntity().GetTransform().GetWorldPosition() };

        SortBackToFront(transparents, camPos);

        for (auto* renderer : transparents) {
            renderer->Render(renderContext.camera);
        }
    }

    constexpr std::string_view GetName() const override { return "Transparent"; }

private:
    static void SortBackToFront(std::vector<Components::Renderer*>& renderers, const Vec3& camPos) {
        for (int i = 1; i < static_cast<int>(renderers.size()); ++i) {
            Components::Renderer* key { renderers[i] };
            const float keyDist { Distance(camPos, key->GetEntity().GetTransform().GetWorldPosition()) };
            int j { i - 1 };

            while (j >= 0 && Distance(camPos, renderers[j]->GetEntity().GetTransform().GetWorldPosition()) < keyDist) {
                renderers[j + 1] = renderers[j];
                --j;
            }

            renderers[j + 1] = key;
        }
    }
};

// ─────────────────────────────────────────────
//  Post Process
// ─────────────────────────────────────────────

class PostProcessPass final : public RenderPass {
public:
	PostProcessPass(const Bindables::RenderTarget* src, const Bindables::RenderTarget* dst)
		: m_Src(src)
		, m_Dst(dst)
		, m_PostProcess(CreateUnique<PostProcess>(
			AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
		  ))
	{}

	void Execute(const RenderContext& ctx) override {
		CORE_ASSERT(m_PostProcess, "PostProcessPass: PostProcess not initialized.");
		CORE_ASSERT(m_Src,         "PostProcessPass: Source render target is null.");
		CORE_ASSERT(m_Dst,         "PostProcessPass: Destination render target is null.");

		m_Dst->Bind();
		glViewport(0, 0, m_Dst->GetWidth(), m_Dst->GetHeight());
		glClear(GL_COLOR_BUFFER_BIT);

		GLStateCache::Get().ApplyState(RenderPipelineState::PostProcess());
		m_PostProcess->Render(m_Src->GetTextureGpuID(), m_Src->GetWidth(), m_Src->GetHeight());

		m_Dst->Unbind();
	}

	constexpr std::string_view GetName() const override { return "PostProcess"; }

private:
	const Bindables::RenderTarget* m_Src { nullptr };
	const Bindables::RenderTarget* m_Dst { nullptr };
	Unique<PostProcess>            m_PostProcess;
};

} // namespace Engine::Rendering::Passes