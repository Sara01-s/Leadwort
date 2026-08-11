#pragma once

#include "Leadwort/components/public/IRenderer.h"
#include "Leadwort/rendering/public/DrawCommands.h"
#include "Leadwort/rendering/public/rendergraph/IPass.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"

namespace Leadwort::Rendering::RG::Passes {
    
    class OpaquePass final : public IPass {
    public:
        explicit OpaquePass(RenderTexture& color, RenderTexture& depth) noexcept
            : m_Color(color), m_Depth(depth) {}

        std::string_view GetName() const noexcept override { return "Opaque Pass"; }
        
        void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
        	builder.SetRenderAttachment(*this, m_Color);
        	builder.SetDepthAttachment(*this, m_Depth);
        }

		void Execute(const RenderContext& renderContext) noexcept override {
			DrawCommandBuffer buffer{};

        	for (const auto* renderer : (*renderContext.renderQueues)[RenderQueue::Opaque]) {
        		renderer->EmitDrawCommand(buffer, *renderContext.camera);
        	}

        	buffer.Sort();
        	buffer.Draw();
        }

	private:
        RenderTexture& m_Color;
        RenderTexture& m_Depth;
    };

}