#pragma once
#include "RenderQueue.h"
#include "engine/components/public/Camera.h"
#include "engine/rendering/bindables/public/RenderTarget.h"

#include <string_view>

namespace Engine::Rendering {

struct RenderContext {
	Components::Camera* camera { nullptr };
	const Bindables::RenderTarget* renderTarget { nullptr };
	RenderQueues* renderQueues { nullptr };
	Vec2 resolution { Vec2() };
};

class RenderPass {
public:
	virtual ~RenderPass() = default;
	virtual void Execute(const RenderContext& renderContext) = 0;
	virtual std::string_view GetName() const = 0;
};

}