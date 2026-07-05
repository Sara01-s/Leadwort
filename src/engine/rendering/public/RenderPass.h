#pragma once

#include "RenderContext.h"

#include <string_view>

namespace Engine::Rendering {

class RenderPass {
public:
	virtual ~RenderPass() = default;
	virtual void DeclareResources(RenderPassBuilder& builder) noexcept = 0;
	virtual void Execute(const RenderContext& ctx) noexcept = 0;
	virtual std::string_view GetName() const noexcept = 0;
};

} // namespace Engine::Rendering