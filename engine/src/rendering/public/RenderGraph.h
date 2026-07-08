#pragma once

#include "../../core/public/Core.h"
#include "FrameBuffer.h"
#include "RenderContext.h"
#include "RenderPass.h"
#include "RenderPassBuilder.h"

namespace Engine::Rendering {

class RenderGraph final {
private:
	struct PassEntry {
		Unique<RenderPass> pass{};
		Unique<FrameBuffer> frameBuffer{};
		std::vector<RenderTexture*> inputs{};
	};

public:
	void AddPass(Unique<RenderPass> pass) noexcept {
		m_IsCompiled = false;
		m_Passes.push_back(PassEntry { std::move(pass), nullptr, {} });
	}

	// Registers declared inputs/outputs per pass.
	// Future: topological sort + render target aliasing.
	void Compile() noexcept {
		for (auto& [pass, frameBuffer, inputs] : m_Passes) {
			RenderPassBuilder builder{};
			pass->DeclareResources(builder);

			std::span outputs { builder.GetOutputs() };
			RenderTexture* depth { builder.GetDepth() };

			std::span inputsSpan { builder.GetInputs() };
			inputs.assign(inputsSpan.begin(), inputsSpan.end());

			if (!outputs.empty() || depth != nullptr) {
				frameBuffer = CreateUnique<FrameBuffer>(outputs, depth);
			}
		}

		m_IsCompiled = true;
	}

	void Execute(Components::Camera& camera, RenderQueues& queues) const noexcept {
		const RenderContext renderContext { &camera, &queues };

		for (const auto& [pass, frameBuffer, inputs] : m_Passes) {
			if (frameBuffer) {
				frameBuffer->Bind();
				glViewport(0, 0, frameBuffer->GetWidth(), frameBuffer->GetHeight());
			}
			else {
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}

			// Pass inputs as samplers.
			for (GLint slot = 0; slot < static_cast<GLint>(inputs.size()); ++slot) {
				inputs[slot]->BindAsInput(slot);
			}

			pass->Execute(renderContext);
		}
	}

	void Clear() noexcept {
		m_IsCompiled = false;
		m_Passes.clear();
	}

	[[nodiscard]] std::size_t GetPassCount() const noexcept { return m_Passes.size(); }

private:
	bool m_IsCompiled { false };
	std::vector<PassEntry> m_Passes{};
};

} // namespace Engine::Rendering