#include "../public/RenderGraph.h"

#include <Leadwort/components/public/MeshRenderer.h>
#include <Leadwort/core/public/Entity.h>

namespace Leadwort::Rendering {

void RenderGraph::Compile() noexcept {
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

void RenderGraph::Execute(Components::Camera& camera, RenderQueues& queues, const Core::Entity* highlightedEntity) const noexcept {
	const RenderContext renderContext { &camera, &queues, highlightedEntity == nullptr ? nullptr : highlightedEntity->GetComponent<Components::MeshRenderer>() };

	for (const auto& [pass, frameBuffer, inputs] : m_Passes) {
		if (frameBuffer) {
			frameBuffer->Bind();
			glViewport(0, 0, frameBuffer->GetWidth(), frameBuffer->GetHeight());
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		for (GLint slot = 0; slot < static_cast<GLint>(inputs.size()); ++slot) {
			inputs[slot]->BindAsInput(slot);
		}

		pass->Execute(renderContext);
	}
}

} // namespace Leadwort::Rendering
