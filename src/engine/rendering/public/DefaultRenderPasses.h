#pragma once

#include "DrawCommands.h"
#include "RenderPassBuilder.h"
#include "engine/asset-management/public/AssetManager.h"
#include "engine/components/public/Renderer.h"
#include "engine/rendering/public/GLStateCache.h"
#include "engine/rendering/public/PostProcess.h"
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
    explicit BackgroundPass(RenderTexture* color, RenderTexture* depth) noexcept
        : m_Output(color), m_Depth(depth) {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
    	glDepthMask(GL_TRUE);
        std::visit(overloaded {
            [](const Components::Camera::SkyBox& sky) {
                glClear(GL_DEPTH_BUFFER_BIT);
                sky.skybox->Render();
            },
            [](const Components::Camera::SolidColor& solid) {
                const auto& bg = solid.color;
                glClearColor(bg.r, bg.g, bg.b, bg.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
        }, ctx.camera->background);
    }

    constexpr std::string_view GetName() const noexcept override { return "Background"; }

private:
	RenderTexture* m_Output { nullptr };
    RenderTexture* m_Depth { nullptr };
};

// ─────────────────────────────────────────────
//  Opaque
// ─────────────────────────────────────────────

class OpaquePass final : public RenderPass {
public:
    explicit OpaquePass(RenderTexture* color, RenderTexture* depth) noexcept
        : m_Output(color), m_Depth(depth) {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
        DrawCommandBuffer buffer{};

        for (const auto* renderer : (*ctx.renderQueues)[static_cast<std::size_t>(RenderQueue::Opaque)]) {
            renderer->EmitDrawCommand(buffer, *ctx.camera);
        }

        buffer.Sort();
        buffer.Draw();
    }

    constexpr std::string_view GetName() const noexcept override { return "Opaque"; }

private:
	RenderTexture* m_Output { nullptr };
	RenderTexture* m_Depth { nullptr };
};

// ─────────────────────────────────────────────
//  AlphaTest
// ─────────────────────────────────────────────

class AlphaTestPass final : public RenderPass {
public:
    explicit AlphaTestPass(RenderTexture* output, RenderTexture* depth) noexcept
        : m_Output(output), m_Depth(depth) {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
        DrawCommandBuffer buffer;

        for (const auto* renderer : (*ctx.renderQueues)[static_cast<std::size_t>(RenderQueue::AlphaTest)]) {
            renderer->EmitDrawCommand(buffer, *ctx.camera);
        }

        buffer.Sort();
        buffer.Draw();
    }

    constexpr std::string_view GetName() const noexcept override { return "AlphaTest"; }

private:
    RenderTexture* m_Output { nullptr };
	RenderTexture* m_Depth { nullptr };
};

// ─────────────────────────────────────────────
//  Grid
// ─────────────────────────────────────────────

class GridPass final : public RenderPass {
public:
    explicit GridPass(RenderTexture* output, RenderTexture* depth) noexcept
        : m_Output(output), m_Depth(depth)
    {
        glGenVertexArrays(1, &m_EmptyVAO);
    }

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
        if ((ctx.camera->cullingMask & Utils::Layers::SCENE) == 0) {
            return;
        }

        GLStateCache::Get().ApplyState(RenderPipelineState::Grid());

        m_GridShader->Bind();
        m_GridShader->SetUniform("_InvProjectionMatrix", Inverse(ctx.camera->GetProjectionMatrix()));
        m_GridShader->SetUniform("_Resolution", m_Output->GetResolution());

        glBindVertexArray(m_EmptyVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        m_GridShader->Unbind();
        glBindVertexArray(0);
    }

    constexpr std::string_view GetName() const noexcept override { return "Grid"; }

private:
    RenderTexture* m_Output { nullptr };
	RenderTexture* m_Depth { nullptr };
    Shared<Bindables::Shader> m_GridShader { AssetManagement::EngineAssets::GetShader("shaders/shd_grid.glsl") };
    GLuint m_EmptyVAO { 0 };
};

// ─────────────────────────────────────────────
//  Transparent
// ─────────────────────────────────────────────

class TransparentPass final : public RenderPass {
public:
    explicit TransparentPass(RenderTexture* output, RenderTexture* depth) noexcept
        : m_Output(output), m_Depth(depth) {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
        DrawCommandBuffer buffer{};

        for (const auto* renderer : (*ctx.renderQueues)[static_cast<std::size_t>(RenderQueue::Transparent)]) {
            renderer->EmitDrawCommand(buffer, *ctx.camera);
        }

        buffer.Sort();
        buffer.Draw();
    }

    constexpr std::string_view GetName() const noexcept override { return "Transparent"; }

private:
	RenderTexture* m_Output { nullptr };
	RenderTexture* m_Depth { nullptr };
};

// ─────────────────────────────────────────────
//  Post Process
// ─────────────────────────────────────────────

class PostProcessPass final : public RenderPass {
public:
    PostProcessPass(RenderTexture* src, RenderTexture* dst) noexcept
        : m_Src(src)
        , m_Dst(dst)
    {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Dst);
    }

	void Execute(const RenderContext& ctx) noexcept override {
    	GLStateCache::Get().ApplyState(RenderPipelineState::PostProcess());
    	m_PostProcess->Render(m_Src->GetGpuID());
    }

    constexpr std::string_view GetName() const noexcept override { return "PostProcess"; }

private:
    RenderTexture* m_Src { nullptr };
    RenderTexture* m_Dst { nullptr };
    Unique<PostProcess> m_PostProcess { CreateUnique<PostProcess>(
		AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
	)};
};

} // namespace Engine::Rendering::Passes