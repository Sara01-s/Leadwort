#pragma once

#include "engine/rendering/bindables/public/Material.h"
#include "engine/rendering/bindables/public/Mesh.h"
#include "engine/rendering/bindables/public/Shader.h"
#include "engine/rendering/public/GLStateCache.h"
#include "engine/rendering/public/RenderPipelineState.h"

#include <glad/glad.h>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace Engine::Rendering {

struct DrawCommand {
    // Non-owning resources
    const Bindables::Shader*   shader   { nullptr };
    const Bindables::Material* material { nullptr };
    const Bindables::Mesh*     mesh     { nullptr };

    RenderPipelineState pipelineState { RenderPipelineState::Opaque() };
    Mat4 modelMatrix{};
	Mat3 normalMatrix{};

    uint64_t sortKey { 0ULL };

	static DrawCommand Create(
		const Bindables::Mesh& mesh,
		const Mat4& modelMatrix,
		const float linearDepth
	) noexcept {
		const auto& material = *mesh.GetMaterial();
		const auto& shader = material.GetShader();
		const bool invertDepth =
			material.pipelineState.blendMode == BlendMode::AlphaBlend ||
			material.pipelineState.blendMode == BlendMode::PremultipliedAlpha;

		return DrawCommand {
			.shader        = &shader,
			.material      = &material,
			.mesh          = &mesh,
			.pipelineState = material.pipelineState,
			.modelMatrix   = modelMatrix,
			.normalMatrix  = Transpose(Inverse(modelMatrix.ToMat3())),
			.sortKey = BuildKey(
				static_cast<uint16_t>(shader.GetGpuID()),
				static_cast<uint16_t>(material.GetGpuID()),
				linearDepth,
				invertDepth
			)
		};
	}

private:

	// ─────────────────────────────────────────────
	//  Sort key layout (64 bits)
	//
	//  [63..48] shader GPU ID   (16 bits) - minimize program changes
	//  [47..32] material GPU ID (16 bits) - minimize textures/uniforms changes
	//  [31.. 8] depth			 (24 bits) - front-to-back for opaques (inverted for transparents)
	//  [ 7.. 0] reserved        ( 8 bits)
	// ─────────────────────────────────────────────
    static uint64_t BuildKey(
    	const uint16_t shaderID,
    	const uint16_t materialID,
    	const float depth,
		const bool invertDepth
	) noexcept {
        // Normalize depth [0, 1] to 24 bits
        const float    clampedDepth = Clamp01(depth);
        const uint32_t depthBits    = static_cast<uint32_t>(clampedDepth * 0x00FFFFFFu);
        const uint32_t sortedDepth  = invertDepth ? (0x00FFFFFFu - depthBits) : depthBits;

        return (static_cast<uint64_t>(shaderID)    << 48)
             | (static_cast<uint64_t>(materialID)  << 32)
             | (static_cast<uint64_t>(sortedDepth) << 8);
    }
};

// ─────────────────────────────────────────────
//  DrawCommandBuffer
// ─────────────────────────────────────────────

class DrawCommandBuffer {
public:
	void Add(DrawCommand&& cmd) noexcept {
		m_DrawCommands.emplace_back(std::move(cmd));
	}

    void Sort() noexcept {
        std::ranges::sort(m_DrawCommands, [](const DrawCommand& a, const DrawCommand& b) {
			return a.sortKey < b.sortKey;
        });
    }

    void Draw() const noexcept {
        for (const auto& cmd : m_DrawCommands) {
            GLStateCache::Get().ApplyState(cmd.pipelineState);

            cmd.shader->Bind();
            cmd.shader->SetUniform("_ModelMatrix", cmd.modelMatrix);
        	cmd.shader->SetUniform("_NormalMatrix", cmd.normalMatrix);

            cmd.material->Bind();

            cmd.mesh->Bind();
            glDrawElements(cmd.mesh->GetTopology(), cmd.mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            cmd.mesh->Unbind();
        }
    }

    void Clear() noexcept {
	    m_DrawCommands.clear();
    }

    [[nodiscard]] bool IsEmpty() const noexcept { return m_DrawCommands.empty(); }
    [[nodiscard]] std::size_t Size() const noexcept { return m_DrawCommands.size(); }

private:
    std::vector<DrawCommand> m_DrawCommands{};
};

} // namespace Engine::Rendering