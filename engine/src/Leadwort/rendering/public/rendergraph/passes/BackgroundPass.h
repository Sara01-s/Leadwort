#pragma once

#include "Leadwort/components/public/Camera.h"
#include "Leadwort/rendering/public/GLStateCache.h"
#include "Leadwort/rendering/public/rendergraph/RenderGraphBuilder.h"
#include "Leadwort/rendering/public/rendergraph/RenderTexture.h"
#include "Leadwort/utils/public/Visit.h"

namespace Leadwort::Rendering::RG::Passes {
    
    class BackgroundPass final : public IPass {
    public:
		BackgroundPass(RenderTexture& texture, RenderTexture& depth) noexcept
            : m_Output(&texture), m_Depth(&depth) {}

        std::string_view GetName() const noexcept override { return "Background Pass"; }
        
        void RecordToRenderGraph(RenderGraphBuilder& builder) noexcept override {
            builder.SetRenderAttachment(*this, *m_Output);
            builder.SetDepthAttachment(*this, *m_Depth);
        }

    	void Execute(const RenderContext& renderContext) noexcept override {
			GLStateCache::Get().Invalidate();

			if (m_Output) {
				glViewport(0, 0, m_Output->GetWidth(), m_Output->GetHeight());
			}

			glStencilMask(0xFF);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask(GL_TRUE);

			std::visit(overloaded {
				[](const Components::Camera::SkyBox& sky) {
					glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
					sky.Sky->Render();
				},
				[](const Components::Camera::SolidColor& solid) {
					const auto& bg { solid.Color };
					glClearColor(bg.r, bg.g, bg.b, bg.a);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
				}
			}, renderContext.Camera->Background);
		}

    private:
		RenderTexture* m_Output { nullptr };
		RenderTexture* m_Depth { nullptr };
    };

}