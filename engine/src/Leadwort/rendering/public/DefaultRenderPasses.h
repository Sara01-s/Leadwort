#pragma once

#include "CoordinateSystem.h"
#include "DrawCommands.h"
#include "Leadwort/components/public/MeshRenderer.h"
#include "RenderPassBuilder.h"

#include <Leadwort/asset-management/public/AssetManager.h>
#include <Leadwort/components/public/Renderer.h>
#include <Leadwort/rendering/public/GLStateCache.h>
#include <Leadwort/rendering/public/PostProcess.h>
#include <Leadwort/rendering/public/RenderPass.h>
#include <Leadwort/rendering/public/RenderPipelineState.h>
#include <Leadwort/utils/public/Visit.h>

#include <glad/glad.h>
#include <variant>

#undef near
#undef far

namespace Leadwort::Rendering::Passes {

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
    	glStencilMask(0xFF);

        std::visit(overloaded {
            [](const Components::Camera::SkyBox& sky) {
                glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                sky.skybox->Render();
            },
            [](const Components::Camera::SolidColor& solid) {
                const auto& bg = solid.color;
                glClearColor(bg.r, bg.g, bg.b, bg.a);
            	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            }
        }, ctx.camera->background);
    }

    std::string_view GetName() const noexcept override { return "Background"; }

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

	std::string_view GetName() const noexcept override { return "Opaque"; }

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

    std::string_view GetName() const noexcept override { return "AlphaTest"; }

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

    std::string_view GetName() const noexcept override { return "Grid"; }

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

    std::string_view GetName() const noexcept override { return "Transparent"; }

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

    std::string_view GetName() const noexcept override { return "PostProcess"; }

private:
    RenderTexture* m_Src { nullptr };
    RenderTexture* m_Dst { nullptr };
    Unique<PostProcess> m_PostProcess { CreateUnique<PostProcess>(
		AssetManagement::EngineAssets::GetShader("shaders/postprocess/shd_post_process.glsl")
	)};
};

// ─────────────────────────────────────────────
//  Outline (stencil-based selection outline)
// ─────────────────────────────────────────────

class OutlinePass final : public RenderPass {
public:
    explicit OutlinePass(RenderTexture* output, RenderTexture* depth) noexcept
        : m_Output(output), m_Depth(depth) {}

    void DeclareResources(RenderPassBuilder& builder) noexcept override {
        builder.Write(m_Output);
        builder.WriteDepth(m_Depth);
    }

    void Execute(const RenderContext& ctx) noexcept override {
        if (ctx.highlightedMeshRenderer == nullptr) {
            return;
        }

        const auto& entity      { ctx.highlightedMeshRenderer->GetEntity() };
        const auto& mesh        { ctx.highlightedMeshRenderer->mesh };
        const Mat4  modelMatrix { CoordinateSystem::CalculateModelMatrix(entity.GetTransform()) };

        GLStateCache::Get().ApplyState(RenderPipelineState::OutlineStencilWrite());
        DrawMesh(*mesh, modelMatrix, *m_MaskShader);

        GLStateCache::Get().ApplyState(RenderPipelineState::OutlineDraw());

    	m_OutlineShader->Bind();
    	m_OutlineShader->SetUniform("_ModelMatrix", modelMatrix);
    	m_OutlineShader->SetUniform("_OutlineThickness", 0.03f);
    	m_OutlineShader->SetUniform("_OutlineColor", Vec4(0.2f, 0.75f, 0.9f, 1.0f));

    	mesh->Bind();
        glDrawElements(mesh->GetTopology(), mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        mesh->Unbind();

        m_OutlineShader->Unbind();
    }

    std::string_view GetName() const noexcept override { return "Outline"; }

private:
    static void DrawMesh(const Bindables::Mesh& mesh, const Mat4& modelMatrix, const Bindables::Shader& shader) noexcept {
        shader.Bind();
        shader.SetUniform("_ModelMatrix", modelMatrix);
        mesh.Bind();
        glDrawElements(mesh.GetTopology(), mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        mesh.Unbind();
        shader.Unbind();
    }

private:
    RenderTexture* m_Output { nullptr };
    RenderTexture* m_Depth { nullptr };
    Shared<Bindables::Shader> m_MaskShader    { AssetManagement::EngineAssets::GetShader("shaders/shd_outline_mask.glsl") };
    Shared<Bindables::Shader> m_OutlineShader { AssetManagement::EngineAssets::GetShader("shaders/shd_outline.glsl") };
};

} // namespace Engine::Rendering::Passes