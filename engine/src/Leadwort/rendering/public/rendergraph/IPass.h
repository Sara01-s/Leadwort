#pragma once

#include "Leadwort/rendering/public/RenderContext.h"

#include <string>
#include <string_view>

namespace Leadwort::Rendering::RG {

	class FrameBuffer;
    class RenderGraphBuilder;

    class IPass {
    public:
        IPass() = default;
        virtual ~IPass() = default;

        virtual void RecordToRenderGraph(RenderGraphBuilder& renderGraphBuilder) noexcept = 0;
    	virtual void Execute(const RenderContext& renderContext) noexcept = 0;
        [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

        [[nodiscard]] std::uint32_t GetID() const noexcept { return m_ID; }

    	// Injected by RenderGraph::Compile(). Non-owning. RenderGraph owns the lifetime.
    	// RenderGraph::Execute binds this automatically before calling Execute();
    	// passes never need to bind it themselves :)
    	void SetFrameBuffer(FrameBuffer* frameBuffer) noexcept { m_FrameBuffer = frameBuffer; }

    	// Non-null only if this pass declared at least one Write/WriteDepth resource.
    	[[nodiscard]] FrameBuffer* GetFrameBuffer() const noexcept { return m_FrameBuffer; }

    private:
        static inline std::uint32_t s_NextPassID { 0 };

        std::uint32_t m_ID { s_NextPassID++ };
        std::string m_Name{};
    	FrameBuffer* m_FrameBuffer { nullptr };
    };

}
